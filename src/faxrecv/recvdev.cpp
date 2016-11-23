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

#include "recvdev.h"
#include "faxrecv.h"
#include "defaults.h"
#include "ConfAssi.h"
#include "PathName.h"
#include "osImport.h"
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <syslog.h>

/*===========================================================================*\
\*===========================================================================*/

void closeFIFO (int devinfo, const char * fifoName = 0) {
    if (devinfo != -1) {
        close (devinfo);
    }
    if (fifoName != 0) {
        unlink (fifoName);
    }
}

int openFIFO(const char* fifoName, int mode) {
    closeFIFO (-1, fifoName);
    if (  (access (fifoName, F_OK) == 0)
       || (mkfifo (fifoName, mode & 0777) < 0)) {
        return -1;
    }
    int fd = open (fifoName, O_RDWR | O_NDELAY, mode & 0777);
    return (fd);
}

/*===========================================================================*\
\*===========================================================================*/

CFaxReceiveDevice::CFaxReceiveDevice (CFaxReceiveMain *MainFaxClass, FaxFormatType faxtype) {
    dhead ("CFaxReceiveDevice-Constructor", DCON_CFaxReceiveDevice);
    DeviceName.Set (DEFAULT_DEVICE_NAME);
    MainClass      = MainFaxClass;
    format         = faxtype;
    RecvFileMode   = DEFAULT_FILE_MODE;
    CountThreads   = 2;
    hFIFOfile      = -1;
    hStatusFile    = 0;
    Starting       = vFalse;
}

/*===========================================================================*\
\*===========================================================================*/

CFaxReceiveDevice::~CFaxReceiveDevice (void) {
    dhead ("CFaxReceiveDevice-Destructor", DCON_CFaxReceiveDevice);
    if (hFIFOfile >= 0) {
        closeFIFO (hFIFOfile);
        hFIFOfile = -1;
    }
    if (hStatusFile != 0) {
        fclose (hStatusFile);
        hStatusFile = 0;
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveDevice::Config (CConfigParserSection *section) {
    dhead ("CFaxReceiveDevice::Config", DCON_CFaxReceiveDevice);
    dassert (section != 0);
    CDynamicString LocalIdentifier;
    ConfigAssignment_Item Assignments[] = {
        { "HylafaxDeviceName",  ConfigAssignment_String, &DeviceName },
        { "FAXNumber",          ConfigAssignment_String, &FaxNumber },
        { "LocalIdentifier",    ConfigAssignment_String, &LocalIdentifier },
        { "RecvFileMode",       ConfigAssignment_UInt,   &RecvFileMode },
        { "MaxConcurrentRecvs", ConfigAssignment_UInt,   &CountThreads },
        { 0,                    ConfigAssignment_UInt,   0 }
    };
    ConfigValuesAssignment (&section->ValueList, &Assignments[0]);
    if (format == FaxFormat_Hylafax) {
        // Generate special Config File
        MainClass->GenerateHylafaxConfigFile (&DeviceName, &FaxNumber, &LocalIdentifier);
    }

    tUInt          tmpController;
    tUInt          tmpUseDDI;
    tUInt          tmpDDILen;
    CDynamicString tmpDDIPrefix;
    tUInt          tmpSpeech;
    tUInt          tmpCIPMask;
    tUInt          tmpGlobalCall;
    CMultiString   tmpDDIs;
    CMultiString   tmpMSNs;
    CMultiString  *pMSNs;
    tUInt          FindPos;

    ConfigAssignment_Item CntrlAssignments[] = {
        { "Controller",       ConfigAssignment_UInt,       &tmpController },
        { "AcceptSpeech",     ConfigAssignment_UInt,       &tmpSpeech },
        { "UseDDI",           ConfigAssignment_UInt,       &tmpUseDDI },
        { "DDILength",        ConfigAssignment_UInt,       &tmpDDILen },
        { "DDIOffset",        ConfigAssignment_String,     &tmpDDIPrefix },
        { "IncomingDDIs",     ConfigAssignment_StringList, &tmpDDIs },
        { "AcceptGlobalCall", ConfigAssignment_UInt,       &tmpGlobalCall },
        { "IncomingMSNs",     ConfigAssignment_StringList, &tmpMSNs },
        { 0,                  ConfigAssignment_UInt,       0 }
    };

    for (CConfigParserSection *laufSec = (CConfigParserSection *)section->SectionList.GetFirst();
         laufSec; laufSec = (CConfigParserSection *)laufSec->GetNext()) {

        tmpController = 1;
        tmpUseDDI     = 0;
        tmpGlobalCall = 1;
        tmpDDILen     = 0;
        tmpSpeech     = 0;
        tmpCIPMask    = cipmask_3_1kHzAudio | cipmask_G3Fax;
        tmpDDIPrefix.RemoveAll();
        tmpDDIs.RemoveAndDeleteAll();
        tmpMSNs.RemoveAndDeleteAll();
        ConfigValuesAssignment (&laufSec->ValueList, &CntrlAssignments[0]);
        if ((tmpController != 0) && (CIPMSNList.ExistController (tmpController) == vFalse)) {
            if (tmpSpeech) {
                tmpCIPMask |= cipmask_Speech;
            }
            CIPMSNList.Set (tmpController, 0, tmpCIPMask);
            CIPMSNList.SetGlobalCallOpt(tmpGlobalCall == 1);
            if (tmpUseDDI != 0) {
                CIPMSNList.SetDDI (tmpController, &tmpDDIPrefix, tmpDDILen);
                pMSNs = &tmpDDIs;
            } else {
                pMSNs = &tmpMSNs;
            }
            CIPMSNList.SetMSN (tmpController, 0);
            if (pMSNs->IsEmpty() == vFalse) {
                COneMultiString *MSNlauf = pMSNs->GetFirst();
                dassert (MSNlauf != 0);
                do {
                    dassert (MSNlauf->GetPointer() != 0);
                    MSNlauf->RemoveChars ((tChar *)" \t_()[]{}\"'`TtPp,.:;");
                    if (MSNlauf->IsEmpty() == vFalse) {
                        FindPos = MSNlauf->FindChar ('-');
                        if (FindPos == MAXVAL_tUInt) {
                            dprint ("Add MSN-Value=(%s) ", MSNlauf->GetPointer());
                            switch (CIPMSNList.AddMSN (tmpController, MSNlauf)) {
                            case iErr_OutOfMemory:
                                WriteLog (LOG_ERR, "config-file: No memory to add incoming number (%s).\n",
                                          MSNlauf->GetPointer());
                                break;
                            case iErr_MSNList_MSNOverlap:
                            case iErr_MSNList_MSNEqual:
                                WriteLog (LOG_WARNING, "config-file: Two equal incoming numbers (%s) exist.\n",
                                          MSNlauf->GetPointer());
                                break;
                            case iErr_MSNList_InvalidValue:
                                WriteLog (LOG_ERR, "config-file: Invalid incoming number (%s) detected.\n",
                                          MSNlauf->GetPointer());
                                break;
                            }
                        } else if (FindPos == 0) {
                            WriteLog (LOG_ERR, "config-file: An invalid range (%s), with a missing begin is"
                                      " specified in %s\n", MSNlauf->GetPointer(),
                                      (tmpUseDDI) ? "IncomingDDIs" : "IncomingMSNs");
                        } else if (FindPos + 1 >= MSNlauf->GetLen()) {
                            dassert (FindPos + 1 > MSNlauf->GetLen());
                            WriteLog (LOG_ERR, "config-file: An invalid range (%s) with a missing end is"
                                      " specified in %s\n", MSNlauf->GetPointer(),
                                      (tmpUseDDI) ? "IncomingDDIs" : "IncomingMSNs");
                        } else {
                            MSNlauf->Fill ('\0', 1, FindPos);
                            dassert (MSNlauf->GetLen() > FindPos);
                            CConstString aStr (MSNlauf->GetPointer(), FindPos);
                            CConstString bStr (MSNlauf->GetPointer() + FindPos + 1,
                                               MSNlauf->GetLen() - FindPos - 1);
                            dprint ("Add MSN-Rang=(%s(%x)-%s(%x)) ",
                                    aStr.GetPointer(), aStr.GetLen(),
                                    bStr.GetPointer(), bStr.GetLen());
                            switch (CIPMSNList.AddMSN (tmpController, &aStr, &bStr)) {
                            case iErr_OutOfMemory:
                                WriteLog (LOG_ERR, "config-file: No memory to add range (%s-%s).\n",
                                          MSNlauf->GetPointer(), MSNlauf->GetPointer() + FindPos + 1);
                                break;
                            case iErr_MSNList_MSNOverlap:
                                WriteLog (LOG_ERR, "config-file: Overlapping ranges are invalid, so range"
                                          " (%s-%s) wasn't added.\n",
                                          MSNlauf->GetPointer(), MSNlauf->GetPointer() + FindPos + 1);
                                break;
                            case iErr_MSNList_MSNEqual:
                                WriteLog (LOG_WARNING, "config-file: Two equal ranges (%s-%s) exist.\n",
                                          MSNlauf->GetPointer(), MSNlauf->GetPointer() + FindPos + 1);
                                break;
                            case iErr_MSNList_InvalidValue:
                                WriteLog (LOG_ERR, "config-file: Invalid range (%s-%s) detected.\n",
                                          MSNlauf->GetPointer(), MSNlauf->GetPointer() + FindPos + 1);
                                break;
                            }
                        }
                    }
                } while ((MSNlauf = MSNlauf->GetNext()) != 0);
            }
        }
    }
}

/*===========================================================================*\
\*===========================================================================*/

tBool CFaxReceiveDevice::StartReceive (void) {
    dhead ("CFaxReceiveDevice::StartReceive", DCON_CFaxReceiveDevice);
    tUInt i;
    Starting = vTrue;
    if (format == FaxFormat_Hylafax) {
        // Update Status File
        CPathName StatusFileName (HYLAFAX_STATUS_DIR);
        StatusFileName.SetRelative (&DeviceName);
        dassert (StatusFileName.IsEmpty() == vFalse);
        hStatusFile = fopen ((char *)StatusFileName.GetPointer(), "w");
        if (hStatusFile) {
            fchmod (fileno (hStatusFile), MainClass->LogFileMode);
            WriteStatus ("Running and idle");
        } else {
            WriteLog (LOG_ERR, "Failed to create status file \"%s\"!\n", StatusFileName.GetPointer());
        }

        if (RecreateFIFO() == vFalse) {
            WriteLog (LOG_ERR, "Failed to create file \"FIFO.%s\"!\n", DeviceName.GetPointer());
        }
    }

    if ((CountThreads == 0) || (CIPMSNList.IsEmpty() == vTrue)) {
        dassert (DeviceName.IsEmpty() == vFalse);
        WriteLog (LOG_WARNING, "Device \"%s\" is not configured for receiving faxes.\n", DeviceName.GetPointer());
        Starting = vFalse;
        return vFalse;
    }

    for (i = CountThreads; i > 0; i--) {
        CFaxReceive *pFR = new CFaxReceive (this, format);
        if (pFR) {
            FaxThreads.AddLast (pFR);
            pFR->SetMSNList (&CIPMSNList);
            pFR->StartReceive();
            usleep(100000);
        }
    }
    if (FaxThreads.IsEmpty() == vTrue) {
        dwarning (0);
        WriteLog (LOG_WARNING, "\nDevice \"%s\" can't start any threads needed for receiving faxes!\n",
                  DeviceName.GetPointer());
        Starting = vFalse;
        return vFalse;
    }

    WriteLog (LOG_INFO, "\nDevice \"%s\" uses %d receive thread(s) with the following config:\n", DeviceName.GetPointer(),
              FaxThreads.CountElements());
    CIPMSNList.ResetGetNextMask();
    CDynamicString  line;
    CDynamicString *pOffset;
    CMultiString   *pMSNs;
    CMSNRange      *pCurMSN;
    tUInt           cntrl;
    tUInt           info;
    tUInt           cip;
    while (CIPMSNList.GetNextMask (&cntrl, &info, &cip) == vTrue) {
        pCurMSN = 0;
        pMSNs   = CIPMSNList.GetAllMSNs (cntrl);
        if (pMSNs) {
            pCurMSN = (CMSNRange *)pMSNs->GetFirst();
        }
        line.Print ("Controller=%d", cntrl);
        if (CheckBitsOne (info, infomask_CalledPartyNum)) {
            pOffset = CIPMSNList.GetDDIOffset (cntrl);
            if (pOffset && (pOffset->IsEmpty() == vFalse)) {
                line.PrintAppend (" : DDIOffset=%S", pOffset);
            }
            if (pCurMSN) {
                line.Append (" : IncomingDDIs=");
            } else {
                line.PrintAppend (" : DDILen=%d", CIPMSNList.GetDDILen (cntrl));
            }
        } else if (pCurMSN) {
            line.Append (" : IncomingMSNs=");
        } else {
            line.Append (" : accept all");
        }
        while (pCurMSN) {
            dassert (pCurMSN->IsEmpty() == vFalse);
            line.Append (pCurMSN);
            if (pCurMSN->m_End.IsEmpty() == vFalse) {
                line.PrintAppend ("-%S", &pCurMSN->m_End);
            }
            pCurMSN = (CMSNRange *)pCurMSN->GetNext();
            if (pCurMSN) {
                line.Append (",");
            }
        }
        if (!line.ErrorDetected()) {
            WriteLog (LOG_INFO, "%s\n", line.GetPointer());
        } else {
            dwarning (0);
        }
    }
    Starting = vFalse;
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveDevice::StopReceive (void) {
    dhead ("CFaxReceiveDevice::StopReceive", DCON_CFaxReceiveDevice);
    for (CFaxReceive *lauf = (CFaxReceive *)FaxThreads.GetFirst(); lauf != 0; lauf = (CFaxReceive *)lauf->GetNext()) {
        lauf->StopReceive();
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveDevice::ReceiveStoped (void) {
    dhead ("CFaxReceiveDevice::ReceiveStoped", DCON_CFaxReceiveDevice);
    WriteStatus ("Terminated");
    NotifyFIFO ('Q');

    if ((Starting == vFalse) && FaxThreads.IsEmpty()) {
        delete this;
    }
}

/*===========================================================================*\
\*===========================================================================*/

tBool CFaxReceiveDevice::RecreateFIFO (void) {
    dhead ("CFaxReceiveDevice::RecreateFIFO", DCON_CFaxReceiveDevice);
    if (hFIFOfile >= 0) {
        closeFIFO (hFIFOfile);
        hFIFOfile = -1;
    }
    char FifoName[MAX_STRING_SIZE];
    sprintf (FifoName, READ_FIFO_PREFIX "%s", DeviceName.GetPointer());
    hFIFOfile = openFIFO (FifoName, 0666);
    if (hFIFOfile < 0) {
        return vFalse;
    }
    NotifyFIFO ('R');
    NotifyFIFO ('N');
    NotifyFIFO ('R');
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveDevice::FIFOchanged (void) {
    dhead ("CFaxReceiveDevice::FIFOchanged", DCON_CFaxReceiveDevice);
    dprint ("\n");
    char  ReadCmd[MAX_STRING_SIZE] = "";
    tUInt len                      = read (hFIFOfile, ReadCmd, MAX_STRING_SIZE);
    dprint ("select-read(%s,%x)\n", ReadCmd, len);
    if (len && (len + 1)) {
        switch (*ReadCmd) {
        //case 'A':
        //    NotifyFIFO ('I');
        //    AnswerCall (vTrue);
        //    break;
        //case 'C':
        //    SetConfigItem (ReadCmd + 1, ConfigParamsCount, ConfigParams);
        //    break;
        case 'H':
            NotifyFIFO ('N');
            NotifyFIFO ('R');
            break;
        case 'S':
            switch (ReadCmd[1]) {
            case 'R':
                // Start Listen
                break;
            case 'B':
                // Stopp Listen
                break;
            case 'D':
                dprint ("faxstate-down -> terminate device! ");
                StopReceive();
                break;
            default:
                dprintWrn ("Wrong faxstate value! ");
                break;
            }
            break;
        case 'Q':
            StopReceive();
            break;
        //case 'Z':
        //    Disconnect();
        //    break;
        }
    }
}

/*===========================================================================*\
\*===========================================================================*/

tBool CFaxReceiveDevice::NotifyFIFO (tBool Receive, const char Type, const char *Data) {
    dhead ("CFaxReceiveDevice::NotifyFIFO", DCON_CFaxReceive);
    tBool fret = vFalse;
    if (format != FaxFormat_Hylafax) {
        fret = vTrue;
    } else {
        int faxqfifo = open (WRITE_FIFO_FILE, O_WRONLY | O_NDELAY);
        if (faxqfifo >= 0) {
            char msg[MAX_STRING_SIZE];
            sprintf (msg, "%c%s:%c", (Receive == vFalse) ? '+' : '@', DeviceName.GetPointer(), Type);
            if (Data && *Data) {
                strcat (msg, Data);
            }
            tUInt len = strlen (msg) + 1;
            if ((tSInt)len <= write (faxqfifo, msg, len)) {
                fret = vTrue;
            } else {
                WriteLog (LOG_WARNING, "Can't write to Hylafax FIFOs!\n");
            }
            close (faxqfifo);
        }
    }
    RETURN ('x', fret);
}

/*===========================================================================*\
\*===========================================================================*/

tBool CFaxReceiveDevice::NotifyFIFO (char Type) {
    dhead ("CFaxReceiveDevice::NotifyFIFO", DCON_CFaxReceive);
    if (format != FaxFormat_Hylafax) {
        return vTrue;
    }
    tBool  recv = vFalse;
    const char  *text = 0;
    switch (Type) {
    case 'N':
        text = (const char *)FaxNumber.GetPointer();
        break;
    case 'R':
        recv = vFalse;
        text = "P0c3dff01:ff";
        break;
    case 'Q':
        Type = 'D';
        break;
    case 'B':
    case 'E':
        recv = vTrue;
        break;
    }
    return NotifyFIFO (recv, Type, text);
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveDevice::WriteLog (tSInt priority, const char *text, ...) {
    dhead ("CFaxReceiveDevice::WriteLog", DCON_CFaxReceive);
    va_list params;
    va_start (params, text);
    vWriteLog (priority, text, params);
    va_end (params);
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveDevice::vWriteLog (tSInt priority, const char *text, va_list args) {
    dhead ("CFaxReceiveDevice::WriteLog", DCON_CFaxReceive);
    MainClass->vWriteLog (priority, text, args);
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveDevice::WriteStatus (const char *text) {
    dhead ("CFaxReceiveDevice::WriteStatus", DCON_CFaxReceive);
    if (hStatusFile == 0) {
        return;
    }
    flock (fileno (hStatusFile), LOCK_EX);
    rewind (hStatusFile);
    fprintf (hStatusFile, "%s\n", text);
    fflush (hStatusFile);
    ftruncate (fileno (hStatusFile), ftell (hStatusFile));
    flock (fileno (hStatusFile), LOCK_UN);
}

/*===========================================================================*\
\*===========================================================================*/

