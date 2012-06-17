/*---------------------------------------------------------------------------*\

    Copyright (C) 2000 AVM GmbH. All rights reserved.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, and WITHOUT
    ANY LIABILITY FOR ANY DAMAGES arising out of or in connection
    with the use or performance of this software. See the
    GNU General Public License for further details.

\*---------------------------------------------------------------------------*/

#include "BasicFax.h"
#include "capi2def.h"
#include "misc.h"
#include "aFlags.h"
#include "osImport.h"
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define FAX_FILTER_BUFFER_SIZE       1000
#define CAPI_BUFFER_SIZE             CAPI_MAX_DATA_SIZE

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void fmtTime (CDynamicString *pString, time_t t) {
    time_t v;
    char  *fmt = "%u:%02u";
    if (t < 0) {
        pString->Append ("0:00", 4);
    } else if (t > 99*60*60) {
        pString->Append ("??:??:??", 8);
    } else {
        v = t / 3600;
        if (v > 0) {
            pString->PrintAppend ("%u:", v);
            t  -= v * 3600;
            fmt = "%02u:%02u";
        }
        v  = t / 60;
        t -= v * 60;
        pString->PrintAppend (fmt, v, t);
    }
}


/*===========================================================================*\
\*===========================================================================*/

CBasicFaxConversion::CBasicFaxConversion (void)
  : m_format (FaxFormat_Hylafax),
    m_pCurSendFile (0),
    m_pRecvString (0),
    m_PageCount (0),
    m_BaudRate (0),
    m_VResolution (0),
    m_finishedFaxFile (0),
    m_recvState (0),
    m_faxFileNr (0),
    m_FaxFileD (-1),
    m_RecvFile (-1),
    m_StartTime (0),
    m_ConnectStartTime (0),
    m_StartPageTime (0),
    m_StopTime (0) {

    dhead ("CBasicFaxConversion-Constructor", DCON_CBasicFaxConversion);
}


/*===========================================================================*\
\*===========================================================================*/

CBasicFaxConversion::~CBasicFaxConversion (void) {
    dhead ("CBasicFaxConversion-Destructor", DCON_CBasicFaxConversion);
    m_finishedFaxFile = 0;
    CloseSend();
    CloseRecv();
}


/*===========================================================================*\
\*===========================================================================*/

void CBasicFaxConversion::ConnectLog (cp_ncpi_all *pNCPI) {
    dhead ("CBasicFaxConversion::ConnectLog", DCON_CBasicFaxConversion);
    WriteLog (LOG_INFO, "Connection established.\n");
    m_StartPageTime = m_ConnectStartTime = m_StopTime = time (0);
    if (pNCPI) {
        dassert (pNCPI->pReceiveID != 0);
        if (pNCPI->pReceiveID->GetLen()) {
            dassert (GetReceiveID() != 0);
            GetReceiveID()->Set (pNCPI->pReceiveID->GetData(), pNCPI->pReceiveID->GetLen());
            WriteLog (LOG_INFO, "\tStationID = %s\n" , GetReceiveID()->GetPointer());
        }
        if (pNCPI->Rate) {
            m_BaudRate = pNCPI->Rate;
            WriteLog (LOG_INFO, "\tBaudRate  = %u\n", pNCPI->Rate);
        }
        // Didn't we request SFF?
        dassert (pNCPI->Format == 0);
        CDynamicString dsFlags;
        if (CheckOneFlag (pNCPI->Options, 0x0)) {
            dsFlags.Append ("HighRes, ");
            if (!m_VResolution) {
                m_VResolution = 196;
            }
        } else {
            if (!m_VResolution) {
                m_VResolution = 98;
            }
            //UseOnlyLowRes = vTrue;
        }
        if (CheckOneFlag (pNCPI->Options, 0x1)) {
            dsFlags.Append ("Polling, ");
        }
        if (CheckOneFlag (pNCPI->Options, 0x2)) {
            dsFlags.Append ("PollAfter, ");
        }
        if (CheckOneFlag (pNCPI->Options, 0xA)) {
            dsFlags.Append ("JPEG, ");
        }
        if (CheckOneFlag (pNCPI->Options, 0xB)) {
            dsFlags.Append ("JBIG, ");
        }
        if (CheckOneFlag (pNCPI->Options, 0xC)) {
            dsFlags.Append ("JBIG_progr, ");
        }
        if (CheckOneFlag (pNCPI->Options, 0xD)) {
            dsFlags.Append ("MR_compr, ");
        }
        if (CheckOneFlag (pNCPI->Options, 0xE)) {
            dsFlags.Append ("MMR_compr, ");
        }
        if (CheckOneFlag (pNCPI->Options, 0xF)) {
            dsFlags.Append ("No_ECM, ");
        }
        if (dsFlags.IsEmpty() == vFalse) {
            dassert (dsFlags.GetLen() > 2);
            dsFlags.CutLen (2); // delete last ", "
            WriteLog (LOG_INFO, "\tFlags     = %s\n", dsFlags.GetPointer());
        }
    }
}


/*===========================================================================*\
\*===========================================================================*/

void CBasicFaxConversion::WriteXferLog (const char *cmd, tUInt commid, tUChar *jobtag, tUInt *pJobid, char *devname,
                                        CDynamicString *pUser, CDynamicString *pDestination, char *statustext) {
    dhead ("CBasicFaxConversion::WriteXferLog", DCON_CBasicFaxConversion);
    dassert (cmd != 0);
    dassert (devname != 0);
    dassert (pDestination != 0);
    dassert (statustext != 0);
    if (GetFormat() != FaxFormat_Hylafax) {
        return;
    }
    char buf[80];
    char *timestrf = "%D %H:%M";
    strftime (buf, sizeof (buf), timestrf, localtime (&m_StartTime));
    CDynamicString xferline (200);
    xferline.Print ("%s\t%s\t%09u\t%s", buf, cmd, commid, devname);
    if (pJobid) {
        xferline.PrintAppend ("\t%u\t\"", *pJobid);
    } else {
        xferline.Append ("\t\t\"");
    }
    if (jobtag) {
        for (; *jobtag != '\0'; jobtag++) {
            switch (*jobtag) {
            default:
                xferline.Append (jobtag, 1);
                break;
            case '\t':
                xferline.Append (" ", 1);
                break;
            case '\\':
                if (jobtag[1] != '"') {
                    xferline.Append (jobtag, 1);
                    break;
                }
                jobtag++;
                // no break
            case '"':
                xferline.Append ("\\\"", 2);
                break;
            }
        }
    }
    if (pUser) {
        xferline.PrintAppend ("\"\t%S", pUser);
    } else {
        xferline.Append ("\"\tfax", 5);
    }
    xferline.PrintAppend ("\t\"%S\"\t\"%S\"\t%u\t%d\t", pDestination, GetReceiveID(),
                          FaxPropEncode (GetResolution(), GetBaudRate()), GetPageCount());
    fmtTime (&xferline, GetStopTime() - GetStartTime());
    xferline.Append ("\t", 1);
    fmtTime (&xferline, GetConnectionTime());
    xferline.PrintAppend ("\t\"%s\"\n", statustext);

    if (xferline.ErrorDetected() == vTrue) {
        dwarning (0);
        WriteLog (LOG_ERR, "Memory error while generating xferfaxlog accounting record (dest=%s)!",
                  pDestination->GetPointer());
    } else {
        tSInt fd = open (HYLAFAX_XFERLOG, O_RDWR|O_CREAT|O_APPEND, 0644);
        if (fd < 0) {
            WriteLog (LOG_ERR, "Can't open xferfaxlog for new accounting record (dest=%s)!",
                      pDestination->GetPointer());
        } else {
            flock (fd, LOCK_EX);
            if ((tUInt)write (fd, xferline.GetPointer(), xferline.GetLen()) != xferline.GetLen()) {
                WriteLog (LOG_ERR, "Can't write xferfaxlog accounting record (dest=%s)!",
                          pDestination->GetPointer());
            }
            close (fd);
        }
    }
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CBasicFaxConversion::OpenSend (CMultiString *pSendFileList, tUInt Resolution) {
    dhead ("CBasicFaxConversion::OpenSend", DCON_CBasicFaxConversion);
    CloseSend();
    m_pCurSendFile        = pSendFileList->GetFirst();
    m_VResolution         = Resolution;
    m_finishedFaxFile     = 0;
    m_PageCount           = 0;
    m_StartTime           = time (0);
    m_StartPageTime       = m_StartTime;
    m_StopTime            = m_StartTime;
    m_ConnectStartTime    = 0;
    m_BaudRate            = 0;
    char *ModeText        = "SFF";
    switch (m_format) {
    default:
        dassert (0);
        return iErr_Filter_OpenFailed;
    case FaxFormat_Hylafax:
    case FaxFormat_TIFF:
        if (m_sTifFilter.Open (m_pCurSendFile->GetPointer()) == vFalse) {
            WriteLog (LOG_ERR, "Can't open TIF-File %s!\n", m_pCurSendFile->GetPointer());
            return iErr_File_NotExist;
        }
        if (m_sTifFilter.CheckForCorrectFaxFile() == vFalse) {
            WriteLog (LOG_ERR, "TIF-File %s doesn't look like a 1d G3 Fax!\n", m_pCurSendFile->GetPointer());
            return iErr_File_OpenFailed;
        }
        Resolution = (m_VResolution) ? m_VResolution : (m_sTifFilter.GetYRes()) ? m_sTifFilter.GetYRes() : 196;
        if (m_sSffFilter.Open ((Resolution >= 196) ? 1 : 0, 0) == vFalse) {
            WriteLog (LOG_ERR, "Can't allocate memory for data conversion to the SFF format!\n");
            return iErr_Filter_OpenFailed;
        }
        ModeText = "TIFF";
        break;
    case FaxFormat_G3:
        if (!m_VResolution) {
            m_VResolution = Resolution = 196;
        }
        if (m_sSffFilter.Open ((Resolution >= 196) ? 1 : 0, 0) == vFalse) {
            WriteLog (LOG_ERR, "Can't allocate memory for data conversion to the SFF format!\n");
            return iErr_Filter_OpenFailed;
        }
        ModeText = "G3";
        // no break!
    case FaxFormat_SFF:
        m_FaxFileD = open ((char *)m_pCurSendFile->GetPointer(), O_RDONLY);
        if (m_FaxFileD < 0) {
            WriteLog (LOG_ERR, "Can't open File %s!\n", m_pCurSendFile->GetPointer());
            m_sSffFilter.Close();
            return iErr_File_NotExist;
        }
        break;
    }
    WriteLog (LOG_INFO, "Dial and starting transfer of %s-File %s with %s resolution.\n", ModeText,
              m_pCurSendFile->GetPointer(), (Resolution >= 196) ? "fine" : "normal");
    m_pCurSendFile = m_pCurSendFile->GetNext();
    return i_Okay;
}


/*===========================================================================*\
\*===========================================================================*/

void CBasicFaxConversion::StopSend (void) {
    dhead ("CBasicFaxConversion::StopSend", DCON_CBasicFaxConversion);
    m_finishedFaxFile = 2;
}


/*===========================================================================*\
\*===========================================================================*/

void CBasicFaxConversion::CloseSend (void) {
    dhead ("CBasicFaxConversion::CloseSend", DCON_CBasicFaxConversion);
    StopSend();
    m_sTifFilter.Close();
    m_sSffFilter.Close();

    m_StopTime = time (0);
    if (m_ConnectStartTime == 0) {
        m_ConnectStartTime = m_StopTime;
    }
    if (m_FaxFileD >= 0) {
        close (m_FaxFileD);
        m_FaxFileD = -1;
    }
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CBasicFaxConversion::SendSFF (tUByte *buffer, tUInt maxLen, tUInt *pLen) {
    dhead ("CBasicFaxConversion::SendSFF", DCON_CBasicFaxConversion);
    dassert (pLen != 0);
    tSInfo fret = i_Okay;
    *pLen       = 0;
    if (m_finishedFaxFile < 2) {
        if (m_format == FaxFormat_SFF) {
            *pLen = read (m_FaxFileD, buffer, maxLen);
            switch (*pLen) {
            case 0:
                m_finishedFaxFile = 2;
                break;
            case -1:
                WriteLog (LOG_ERR, "Failed to read data from file!\n");
                *pLen = 0;
                break;
            }
        } else {
            fret = FillSffFilter();
            if (m_sSffFilter.CurSize()) {
                *pLen = m_sSffFilter.Get (buffer, maxLen);
            }
            if ((m_finishedFaxFile == 1) && (m_sSffFilter.CurSize() == 0)) {
                m_finishedFaxFile = 2;
            }
        }
        //if ((UseOnlyLowRes == vTrue) && len) {
            //TODO: eliminate every second line on fine faxes
            //SffLowResFilter.Change (buffer, len);
        //}
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CBasicFaxConversion::OpenRecv (void) {
    dhead ("CBasicFaxConversion::OpenRecv", DCON_CBasicFaxConversion);
    if ((m_format != FaxFormat_SFF) && (m_rSffFilter.Open (1) == vFalse)) {
        return iErr_Filter_OpenFailed;
    }
    m_recvState        = (m_format == FaxFormat_G3) ? 2 : 1;
    m_ConnectStartTime = time (0);
    m_StartPageTime    = m_ConnectStartTime;
    m_finishedFaxFile  = 0;
    m_PageCount        = 0;
    if (GetRecvFile() < 0) {
        m_RecvFileList.RemoveAndDeleteAll();
        return iErr_Filter_OpenFailed;
    }
    if (  (m_format == FaxFormat_Hylafax)
       || (m_format == FaxFormat_TIFF)) {
        if (m_rTifFilter.Open (m_RecvFile, m_RecvFileList.GetFirst()->GetPointer()) == vFalse) {
            close (m_RecvFile);
            remove ((char *)m_RecvFileList.GetFirst()->GetPointer());
            m_RecvFile = -1;
            m_RecvFileList.RemoveAndDeleteAll();
            return iErr_Filter_OpenFailed;
        }
        // file descriptor is handled by libtiff now
        m_RecvFile = -1;
    }
    dassert (m_RecvFileList.GetLast() != 0);
    WriteLog (LOG_INFO, "Write fax in path %s to file %s.\n", GetSpoolDir()->GetPointer(),
              m_RecvFileList.GetFirst()->GetPointer());
    m_recvState = (m_format == FaxFormat_G3) ? 2 : 1;
    return i_Okay;
}


/*===========================================================================*\
\*===========================================================================*/

void CBasicFaxConversion::StopRecv (void) {
    dhead ("CBasicFaxConversion::StopRecv", DCON_CBasicFaxConversion);
    m_finishedFaxFile = 2;
}


/*===========================================================================*\
\*===========================================================================*/

void CBasicFaxConversion::CloseRecv (void) {
    dhead ("CBasicFaxConversion::CloseRecv", DCON_CBasicFaxConversion);
    StopRecv();
    m_rTifFilter.Close();
    m_rSffFilter.Close();
    if (m_RecvFile >= 0) {
        close (m_RecvFile);
    }
    m_RecvFile = -1;
    m_StopTime = time (0);
    if (m_ConnectStartTime == 0) {
        m_ConnectStartTime = m_StopTime;
    }
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CBasicFaxConversion::RecvSFF (tUByte *buffer, tUInt len) {
    dhead ("CBasicFaxConversion::RecvSFF", DCON_CBasicFaxConversion);
    if (m_finishedFaxFile >= 2) {
        dwarning (0);
        return i_Okay;
    }
    if (m_format == FaxFormat_SFF) {
        write (m_RecvFile, buffer, len);
        return i_Okay;
    }

    tSInfo fret = i_Okay;
    do {
        switch (m_recvState) {
        default:
            return i_Okay;
        case 1: {
            CSFilter_Sff2Huf::ctPageInfo PageInfo;
            for (; len > 0; len--, buffer++) {
                if (m_rSffFilter.PutChar (*buffer) == CSFilter_Sff2Huf::i_QueueFull) {
                    dwarning (0);
                    WriteLog (LOG_WARNING, "CFaxReceive::GetData: Overflow in SffFilter.\n");
                    break;
                }
            }
            if ((m_recvState == 1) && (m_rSffFilter.GetCurPageInfo (&PageInfo) == vTrue)) {
                tChar hostname[MAX_STRING_SIZE];
                gethostname ((char *)hostname, MAX_STRING_SIZE);
                m_VResolution = PageInfo.ResolutionVertical ? 196 : 98;
                m_rTifFilter.PutPageHeader (m_VResolution, 203,
                                            PageInfo.PageLength ? PageInfo.PageLength - 1 : 0,
                                            PageInfo.LineLength,
                                            FaxPropEncode (m_VResolution, m_BaudRate),
                                            GetReceiveID()->GetPointer(),
                                            (tUChar *)"AVM GmbH", (tUChar *)NAME_OF_MODEL, hostname,
                                            (tUChar *)NAME_OF_SOFTWARE);
                m_recvState = 2;
            }
        }
        case 2: {
            for (; len > 0; len--, buffer++) {
                if (m_rSffFilter.PutChar (*buffer) == CSFilter_Sff2Huf::i_QueueFull) {
                    dwarning (0);
                    WriteLog (LOG_WARNING, "CFaxReceive::GetData: Overflow in SffFilter.\n");
                    break;
                }
            }
            break;
        }
        }

        CSFilter_Sff2Huf::ctInfo info;
        tUInt tmplen;
        tUChar tmpbuf[1000];
        do {
            tmplen = m_rSffFilter.GetToPageEnd (tmpbuf, 1000, &info);
            if (m_format == FaxFormat_G3) {
                write (m_RecvFile, tmpbuf, tmplen);
                switch (info) {
                case CSFilter_Sff2Huf::i_NewPage:
                    m_PageCount++;
                    WriteLog (LOG_INFO, "Page %d was received.\n", m_PageCount);
                    if (GetNextRecvFile() < 0) {
                        WriteLog (LOG_ERR, "Can't create new file for next fax page! Disconnect now.\n");
                        // TODO: How to handle this? Set quitRecv?
                        m_recvState = 3;
                        tmplen      = 0;  // stop both loops
                        len         = 0;
                        fret        = iErr_File_CreateFailed;
                    }
                    break;
                case CSFilter_Sff2Huf::i_LastPage:
                    m_PageCount++;
                    close (m_RecvFile);
                    WriteLog (LOG_INFO, "Page %d was received. Last Page!\n", m_PageCount);
                    m_RecvFile  = -1;
                    m_recvState = 3;
                    tmplen      = 0;      // stop both loops
                    len         = 0;
                    break;
                default:
                    // nothing to do
                    break;
                }
            } else {
                if (tmplen) {
                    m_rTifFilter.Put (tmpbuf, tmplen);
                }
                switch (info) {
                case CSFilter_Sff2Huf::i_NewPage: {
                    CSFilter_Sff2Huf::ctPageInfo PageInfo;
                    m_rSffFilter.GetLastPageInfo (&PageInfo);
                    time_t t = time (0);
                    m_rTifFilter.PutPageEnd (vFalse, PageInfo.PageLength ? PageInfo.PageLength - 1 : 0,
                                             t - m_StartPageTime);
                    m_StartPageTime = t;
                    m_rSffFilter.GetCurPageInfo (&PageInfo);
                    m_PageCount++;
                    WriteLog (LOG_INFO, "Page %d was received.\n", m_PageCount);
                    PageEndReached (vFalse);
                    m_VResolution = PageInfo.ResolutionVertical ? 196 : 98;
                    m_rTifFilter.PutPageHeader (m_VResolution, 203,
                                                PageInfo.PageLength ? PageInfo.PageLength - 1 : 0,
                                                PageInfo.LineLength, 0, 0, 0, 0, 0, 0);
                    tmplen = 1; // to continue the inner loop
                    break;
                }
                case CSFilter_Sff2Huf::i_LastPage: {
                    m_PageCount++;
                    WriteLog (LOG_INFO, "Page %d was received. - Last Page!\n", m_PageCount);
                    PageEndReached (vTrue);
                    CSFilter_Sff2Huf::ctPageInfo PageInfo;
                    m_rSffFilter.GetLastPageInfo (&PageInfo);
                    m_StopTime = time (0);
                    m_rTifFilter.PutPageEnd (vTrue, PageInfo.PageLength ? PageInfo.PageLength - 1 : 0,
                                             m_StopTime - m_StartPageTime);
                    m_recvState = 3;
                    tmplen      = 0; // stop both loops
                    len         = 0;
                    break;
                }
                default:
                    // nothing to do
                    break;
                }
            }
        } while (tmplen);
    } while (len);
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

CMultiString *CBasicFaxConversion::GetRecvFiles (void) {
    return &m_RecvFileList;
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CBasicFaxConversion::FillSffFilter (void) {
    dhead ("CBasicFaxConversion::FillSffFilter", DCON_CBasicFaxConversion);
    tSInfo info;
    tUChar buffer[FAX_FILTER_BUFFER_SIZE];
    tUInt  len;
    tSInfo fret = i_Okay;
    if (m_finishedFaxFile > 0) {
        return fret;
    }
    while (m_sSffFilter.CurSize() < CAPI_BUFFER_SIZE) {
        if (m_format == FaxFormat_G3) {
            len  = read (m_FaxFileD, buffer, FAX_FILTER_BUFFER_SIZE);
            switch (len) {
            case 0:
                info = iWrn_Filter_LastPageEnd;
                break;
            case -1:
                len = 0;
                WriteLog (LOG_ERR, "Failed to read data from file!\n");
                // TODO: Handle errors
                // no break!
            default:
                info = i_Okay;
                break;
            }
        } else {
            len = m_sTifFilter.GetToPageEnd (buffer, FAX_FILTER_BUFFER_SIZE, &info);
        }
        for (tUInt i = 0; i < len; i++) {
            m_sSffFilter.PutChar (buffer[i]);
        }
        switch (info) {
        case iWrn_Filter_PageEnd:
            m_PageCount++;
            m_sSffFilter.PutPageHeader (vFalse);
            WriteLog (LOG_INFO, "Page %d was sent.\n", m_PageCount);
            break;
        case iWrn_Filter_LastPageEnd: {
            tBool isFinish = vTrue;
            m_sTifFilter.Close();
            m_PageCount++;
            if (m_pCurSendFile) {
                dassert (m_pCurSendFile->IsEmpty() == vFalse);
                dprint ("FileName=%s ", m_pCurSendFile->GetPointer());
                if (m_format == FaxFormat_G3) {
                    close (m_FaxFileD);
                    m_FaxFileD = open ((char *)m_pCurSendFile->GetPointer(), O_RDONLY);
                    if (m_FaxFileD >= 0) {
                        WriteLog (LOG_INFO, "Start transfer of G3-File %s.\n", m_pCurSendFile->GetPointer());
                        m_pCurSendFile = m_pCurSendFile->GetNext();
                        isFinish       = vFalse;
                    } else {
                        WriteLog (LOG_ERR, "Can't open next G3-File %s!\n", m_pCurSendFile->GetPointer());
                        fret = iErr_File_NotExist;
                    }
                } else if (m_sTifFilter.Open (m_pCurSendFile->GetPointer()) == vTrue) {
                    if (m_sTifFilter.CheckForCorrectFaxFile() == vFalse) {
                        WriteLog (LOG_ERR, "TIF-File %s doesn't look like a G3 Fax!\n", m_pCurSendFile->GetPointer());
                        fret = iErr_File_OpenFailed;
                    } else {
                        WriteLog (LOG_INFO, "Page %d was sent.\n", m_PageCount);
                        tUInt Resolution = (m_VResolution) ? m_VResolution :
                                        (m_sTifFilter.GetYRes()) ? m_sTifFilter.GetYRes() : 196;
                        m_sSffFilter.SetVRes ((Resolution >= 196) ? 1 : 0);
                        WriteLog (LOG_INFO, "Start transfer of TIFF-File %s with %s resolution.\n",
                                m_pCurSendFile->GetPointer(), (Resolution >= 196) ? "fine" : "normal");
                        isFinish = vFalse;
                    }
                } else {
                    WriteLog (LOG_ERR, "Can't open TIFF-File %s.\n", m_pCurSendFile->GetPointer());
                    fret = iErr_File_NotExist;
                }
                m_pCurSendFile = m_pCurSendFile->GetNext();
            }
            m_sSffFilter.PutPageHeader (isFinish);
            dprint ("i_LastPageEnd-Recv (isFinish=%x) ", isFinish);
            if (isFinish == vTrue) {
                if (m_format != FaxFormat_G3) {
                    WriteLog (LOG_INFO, "Page %d was sent. - Last Page!\n", m_PageCount);
                }
                m_finishedFaxFile++;
                return fret;
            }
            break;
        }
        case 0:
            break;
        default:
            dassert (0);
            // TODO: handle errors!
            break;
        }
    }
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/

tSInt CBasicFaxConversion::GetRecvFile (void) {
    dhead ("CBasicFaxConversion::GetRecvFile", DCON_CFaxReceive);
    dassert (m_RecvFile == -1);
    tUInt          tmpfaxFileNr  =  1;
    tSInt          tmpFileD      = -1;
    tSInt          SeqFile       = -1;
    CDynamicString NewNameFormat;
    CDynamicString fline;
    CDynamicString NewName (MAX_STRING_SIZE);
    m_RecvFileList.RemoveAndDeleteAll();
    m_RecvFile = -1;
    if (m_format == FaxFormat_Hylafax) {
        NewNameFormat.Set (HYLAFAX_RECV_DIR);
        fline.Set (&NewNameFormat);
    }
    fline.Append (HYLAFAX_SEQF_FILE);
    if (NewName.ErrorDetected() || NewNameFormat.ErrorDetected() || fline.ErrorDetected()) {
        WriteLog (LOG_ERR, "Not enough memory for generating file names\n");
        return m_RecvFile;
    }
    SeqFile = open ((char *)fline.GetPointer(), O_CREAT | O_RDWR, 0644);
    if (SeqFile >= 0) {
        flock (SeqFile, LOCK_EX);
        char dummybuf [MAX_STRING_SIZE];
        tSInt len = read (SeqFile, dummybuf, MAX_STRING_SIZE);
        dummybuf[(len < 0) ? 0 : len] = '\0';
        tmpfaxFileNr = atoi (dummybuf) + 1;
    }
    if (m_format == FaxFormat_Hylafax) {
        NewNameFormat.PrintAppend ("fax%%09u" FILE_EXTENSION_TIFF);
    } else {
        NewNameFormat.PrintAppend ("cf%c%%07u", (m_VResolution > 98) ? 'f' : 'n');
        if (GetReceiveID()->IsEmpty() == vFalse) {
            dassert (GetReceiveID()->GetPointer() != 0);
            NewNameFormat.PrintAppend ("-%S", GetReceiveID());
        }
        switch (m_format) {
        case FaxFormat_TIFF:
            NewNameFormat.Append (FILE_EXTENSION_TIFF);
            break;
        case FaxFormat_SFF:
            NewNameFormat.Append (FILE_EXTENSION_SFF);
            break;
        case FaxFormat_G3:
            NewNameFormat.Append (".01" FILE_EXTENSION_G3);
            break;
        default:
            // nothing to do
            break;
        }
    }
    // One Test to see if we have enough space
    NewName.Print ((char *)NewNameFormat.GetPointer(), 1234567890);
    if (NewNameFormat.ErrorDetected() || NewName.ErrorDetected()) {
        WriteLog (LOG_ERR, "Not enough memory for generating file names\n");
        return m_RecvFile;
    }
    for (tUInt trys = MAX_TRY_COUNT; trys > 0; trys--, tmpfaxFileNr++) {
        if (tmpfaxFileNr < 1 || tmpfaxFileNr > MAX_SEQ_NUMBER) {
            tmpfaxFileNr = 1;
        }
        NewName.Print ((char *)NewNameFormat.GetPointer(), tmpfaxFileNr);
        tmpFileD = open ((char *)NewName.GetPointer(), O_RDWR | O_CREAT | O_EXCL, GetRecvFileMode());
        dprint ("try(%s,%u,%d) ", NewName.GetPointer(), NewName.GetLen(), tmpFileD);
        if (tmpFileD >= 0) {
            flock (tmpFileD, LOCK_EX | LOCK_NB);
            COneMultiString *NewLE = new COneMultiString;
            if (!NewLE || (NewLE->Set (&NewName) == vFalse)) {
                close (tmpFileD);
                break;
            }
            m_RecvFileList.AddLast (NewLE);
            m_faxFileNr = tmpfaxFileNr;
            m_RecvFile  = tmpFileD;
            if (SeqFile >= 0) {
                fline.Print ("%u", m_faxFileNr);
                lseek (SeqFile, 0, SEEK_SET);
                write (SeqFile, fline.GetPointer(), fline.GetLen());
                ftruncate (SeqFile, fline.GetLen());
            }
            break;
        }
        if (errno != EEXIST) {
            if (GetSpoolDir() && !GetSpoolDir()->IsEmpty()) {
                NewName.Set (GetSpoolDir());
            } else {
                NewName.Set (".");
            }
            if (m_format == FaxFormat_Hylafax) {
                NewName.Append ("/");
                NewName.Append (HYLAFAX_RECV_DIR);
            }
            WriteLog (LOG_ERR, "It is not possible to create a File in Directiory %s.\n", NewName.GetPointer());
            break;
        }
    }
    if (SeqFile >= 0) {
        close (SeqFile);
    }
    return m_RecvFile;
}


/*===========================================================================*\
\*===========================================================================*/

tSInt CBasicFaxConversion::GetNextRecvFile (void) {
    dhead ("CBasicFaxConversion::GetNextRecvFile", DCON_CFaxReceive);
    dassert (m_format == FaxFormat_G3);
    dassert (m_faxFileNr > 0);
    dassert (m_RecvFileList.IsEmpty() == vFalse);
    tSInt          tmpFileD = -1;
    CDynamicString NewNameFormat;
    CDynamicString NewName (MAX_STRING_SIZE);

    if (m_RecvFile >= 0) {
        close (m_RecvFile);
        m_RecvFile = -1;
    }
    if (m_format == FaxFormat_Hylafax) {
        NewNameFormat.Set (HYLAFAX_RECV_DIR);
    }
    //sprintf (NewNameFormat + strlen (NewNameFormat), "cf%c%07u", (VResolution) ? 'f' : 'n', m_faxFileNr);
    NewNameFormat.PrintAppend ("cf%07u", m_faxFileNr);
    if (GetReceiveID()->IsEmpty() == vFalse) {
        dassert (GetReceiveID()->GetPointer() != 0);
        NewNameFormat.Append ("-");
        NewNameFormat.Append (GetReceiveID());
    }
    NewNameFormat.Append (".%02u");
    NewNameFormat.Append (FILE_EXTENSION_G3);
    // TODO?: can we try to open only the first specified file?

    // One Test to see if we have enough space
    NewName.Print ((char *)NewNameFormat.GetPointer(), 1234567890);
    if (NewNameFormat.ErrorDetected() || NewName.ErrorDetected()) {
        WriteLog (LOG_ERR, "Not enough memory for generating file names\n");
        return m_RecvFile;
    }
    for (tUInt nr = m_PageCount + 1; nr < m_PageCount + MAX_TRY_COUNT; nr++) {
        NewName.Print ((char *)NewNameFormat.GetPointer(), nr);
        tmpFileD = open ((char *)NewName.GetPointer(), O_RDWR | O_CREAT | O_EXCL, GetRecvFileMode());
        if (tmpFileD >= 0) {
            flock (tmpFileD, LOCK_EX | LOCK_NB);
            COneMultiString *NewLE = new COneMultiString;
            if (!NewLE || (NewLE->Set (&NewName) == vFalse)) {
                close (tmpFileD);
                break;
            }
            m_RecvFileList.AddLast (NewLE);
            m_RecvFile = tmpFileD;
            WriteLog (LOG_INFO, "File for next fax page is %s.\n", NewName.GetPointer());
            break;
        }
        if (errno != EEXIST) {
            if (GetSpoolDir() && !GetSpoolDir()->IsEmpty()) {
                NewName.Set (GetSpoolDir());
            } else {
                NewName.Set (".");
            }
            if (m_format == FaxFormat_Hylafax) {
                NewName.Append ("/");
                NewName.Append (HYLAFAX_RECV_DIR);
            }
            WriteLog (LOG_ERR, "It is not possible to create a File in Directiory %s.\n", NewName.GetPointer());
            break;
        }
    }
    return m_RecvFile;
}


/*===========================================================================*\
\*===========================================================================*/

