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

#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdarg.h>
#include <errno.h>
#include <sysexits.h>
#include <unistd.h>
#include "faxrecv.h"
#include "ConfPars.h"
#include "aString.h"
#include "osImport.h"
#include "misc.h"

/*---------------------------------------------------------------------------*\
    from capiutils.h:
\*---------------------------------------------------------------------------*/

extern "C" char *capi_info2str(c_word reason);

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define FAX_FILTER_BUFFER_SIZE      1000
#define CAPI_BUFFER_SIZE            CapiMaxDataSize

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

//#define GENERATE_DEBUGSFFDATAFILE

#ifdef GENERATE_DEBUGSFFDATAFILE
static FILE *DebugSffDataFile = 0;
#endif

/*===========================================================================*\
\*===========================================================================*/

CFaxReceive::CFaxReceive (CFaxReceiveDevice *FaxRecvDevice, FaxFormatType faxtype)
  : jobNr (0),
    quitRecv (0),
    hJobFile (0) {

    dhead ("CFaxReceive - Constructor", DCON_CFaxReceive);
    FaxDevice = FaxRecvDevice;
    SetFormat (faxtype);
    SetReceiveID (&ONumber);
}

/*===========================================================================*\
\*===========================================================================*/

CFaxReceive::~CFaxReceive (void) {
    dhead ("CFaxReceive - Destructor", DCON_CFaxReceive);
    CloseRecv();
    if (hJobFile > 0) {
        fclose (hJobFile);
        hJobFile = 0;
    }
}


/*===========================================================================*\
\*===========================================================================*/

tBool CFaxReceive::StartReceive (void) {
    dhead ("CFaxReceive::StartReceive", DCON_CFaxReceive);
    cp_b3configuration_faxext B3Config (b3_faxextopt_highres | b3_faxextopt_jbig, &FaxDevice->FaxNumber);
    SetB1Parameters (b1prot_G3Fax);
    SetB2Parameters (b2prot_G3Fax);
    SetB3Parameters (b3prot_G3Fax_extended, &B3Config);
    SetNCPI (0);
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::StopReceive (void) {
    dhead ("CFaxReceive::StopReceive", DCON_CFaxReceive);
    quitRecv = 1;
    StopRecv();
    ResetChannel();
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::ListenError (tUInt Controller, c_info Info) {
    dhead ("CFaxReceive::ListenError", DCON_CFaxReceive);
    if (Controller) {
        WriteLog (LOG_ERR, "Try to listen for call on Controller %d, but got error code 0x%X.\n", Controller, Info);
    } else if (CapiInfoIsRegError (Info)) {
        // Get a Register-Errror as Reason => CAPI not installed, terminate Programm
        dprint ("\nCAPI not installed! ");
        WriteLog (LOG_ERR, "CAPI not installed, started or have no access rights on it!\n");
        FaxDevice->WriteStatus ("CAPI not installed, started or have no access rights on it!!");
    } else {
        WriteLog (LOG_ERR, "Try to listen for calls but got error code 0x%X.\n", Info);
    }
    NotifyFIFO ('W');
    StopReceive();
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::IncomingCall (c_cipvalue CIPValue, CConstString *pOppositeNumber, CConstString *pMyCalledNumber) {
    dhead ("CFaxReceive::IncomingCall", DCON_CFaxReceive);
    dassert (pOppositeNumber != 0);
    dassert (pMyCalledNumber != 0);
    dparams ("%x,%s,%s", CIPValue, pOppositeNumber->GetPointer(), pMyCalledNumber->GetPointer());
    CDynamicString InfoText ("Incoming ");
    ConnectionInd();
    jobNr = 0;
    switch (CIPValue) {
    case cipvalue_Speech:
        InfoText.Append ("speech");
        break;
    case cipvalue_3_1kHzAudio:
        InfoText.Append ("analog");
        break;
    default:
        InfoText.Append ("fax");
        break;
    }
    InfoText.PrintAppend (" call on controller %d", GetActiveController());
    if (!pOppositeNumber->IsEmpty()) {
        ONumber.Set (pOppositeNumber);
        InfoText.PrintAppend (" from %S", pOppositeNumber);
    } else {
        ONumber.RemoveAll();
    }
    if (!pMyCalledNumber->IsEmpty()) {
        MyNumber.Set (pMyCalledNumber);
        InfoText.PrintAppend (" to %S", pMyCalledNumber);
    } else {
        MyNumber.RemoveAll();
    }
    if (GetFormat() == FaxFormat_Hylafax) {
        if (GetJobFile() == vFalse) {
            WriteLog (LOG_ERR, "Can't create log file for incomming call!\n");
            // TODO: better disconnect the call?
        }
        WriteLog (LOG_INFO, "SESSION BEGIN %09u %s\n", jobNr, FaxDevice->FaxNumber.GetPointer());
    }
    if (InfoText.ErrorDetected()) {
        WriteLog (LOG_ERR, "Memory Full during generation of Log output!\n");
    } else {
        dassert (InfoText.IsEmpty() == vFalse);
        dassert (InfoText.GetPointer() != 0);
        WriteLog (LOG_INFO, "%s.\n", InfoText.GetPointer());
    }
    FaxDevice->WriteStatus ("Answering the phone");
    AnswerCall (creject_Accept);
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::DoConnecting (void) {
    dhead ("CFaxReceive::DoConnecting", DCON_CFaxReceive);
    // Nothing to do!
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::ConnectProceeding (CConstString *) {
    dhead ("CFaxReceive::ConnectProceeding", DCON_CFaxReceive);
    ConnectionStarted();
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::IsConnected (cp_ncpi_all *pNCPI) {
    dhead ("CFaxReceive::IsConnected", DCON_CFaxReceive);
    ConnectLog (pNCPI);
    if (  (pNCPI)
       && (  (pNCPI->Format != 0)          // no SFF format
          || (pNCPI->Options & 0x2))) {     // fax polling not implemented yet
        dwarning (0);
        WriteLog (LOG_INFO, "Could not handle this kind of fax! Disconnect now.\n");
        Disconnect (DisconnectKind_Fast);
        return;
    }
    if (infoError (OpenRecv())) {
        WriteLog (LOG_ERR, "Can't create file for the fax. Disconnecting now!\n");
        Disconnect (DisconnectKind_Fast);
    } else {
        #ifdef GENERATE_DEBUGSFFDATAFILE
            dwarning (DebugSffDataFile == 0);
            if (!DebugSffDataFile) {
                DebugSffDataFile = fopen ("/tmp/c2faxrecv_dbgdatafile.sff", "w");
            }
        #endif
        NotifyFIFO ('I');  // Comm id
        NotifyFIFO ('B');  // Busy + inbound call
        FaxDevice->WriteStatus ("Receiving facsimile");
        NotifyFIFO ('S');
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::PutDataComplete (void *, c_info) {
    dhead ("CFaxReceive::PutDataComplete", DCON_CFaxReceive);
    dwarning (0); // Should never been called!
    // Nothing to do!
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::GetData (tUByte *Data, tUShort DataLength, tUShort DataHandle) {
    dhead ("CFaxReceive::GetData", DCON_CFaxReceive);
    dassert (Data != 0);
    dwarning (DataLength > 0);
    #ifdef GENERATE_DEBUGSFFDATAFILE
        if (DebugSffDataFile) {
            fwrite (Data, DataLength, 1, DebugSffDataFile);
        }
    #endif
    if (infoError (RecvSFF (Data, DataLength))) {
        Disconnect (DisconnectKind_Fast);
    }
    GetDataComplete (DataHandle);
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::DisconnectInquiry (c_info, cp_ncpi_all *pNCPI) {
    dhead ("CFaxReceive::DisconnectInquiry", DCON_CFaxReceive);
    dwarning (pNCPI != 0);
    StopRecv();
    if (pNCPI) {
        if (GetFormat() == FaxFormat_SFF) {
            SetPageCount (pNCPI->Pages);
        } else if (GetPageCount() != pNCPI->Pages) {
            WriteLog (LOG_ERR, "Discrepance between CAPI's (%d) and internal (%d) number of pages!\n",
                      pNCPI->Pages, GetPageCount());
            //SetPageCount (pNCPI->Pages);
        }
    }
    DisconnectConfirm();
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::IsDisconnected (c_info Reason) {
    dhead ("CFaxReceive::IsDisconnected", DCON_CFaxReceive);
    dparams ("%x", Reason);
    CloseRecv();
    #ifdef GENERATE_DEBUGSFFDATAFILE
        if (DebugSffDataFile) {
            fclose (DebugSffDataFile);
            DebugSffDataFile = 0;
        }
    #endif

    if (GetRecvFiles() && (GetRecvFiles()->IsEmpty() == vFalse)) {
        tUInt CountIllegalLines = 0;
        tUInt CountAllLines = 0;
        if ((GetFormat() != FaxFormat_SFF) && (GetRecvIllegalLines (&CountIllegalLines, &CountAllLines))) {
            if (!CountAllLines) {
                WriteLog (LOG_ERR, "No fax data was received!\n");
            } else {
                tUInt percent = CountIllegalLines * 100 / CountAllLines;
                if (percent > 10) {
                    WriteLog (LOG_ERR, "Around %d%% of the received fax is corrupt (%d lines)!\n", percent,
                                       CountIllegalLines);
                } else {
                    WriteLog (LOG_INFO, "Received some illegal lines (%d lines = %d%% of fax),\n", CountIllegalLines,
                                        percent);
                }
            }
        }
        if (GetReceiveID()->IsEmpty()) {
            dwarning (0);
            GetReceiveID()->Set ("-");
        }
        if (FaxDevice->MainClass->FaxRcvdCmd.IsEmpty() == vFalse) {
            CDynamicString executeCommand;
            executeCommand.Set (&FaxDevice->MainClass->FaxRcvdCmd);
            char *StateText = "";

            if (GetFormat() == FaxFormat_Hylafax) {
                switch (GetRecvState()) {
                default:
                case 0:
                    StateText = "Can't build up connection for incoming call.";
                    break;
                case 3:
                    if (CountAllLines != 0) {
                        if (CountIllegalLines * 100 / CountAllLines > 50) {
                            StateText = "More than half of the fax is corrupted!";
                        }
                        break;
                    }
                    // no break!
                case 1:
                    StateText = "Connection was established but no fax data was received.";
                    break;
                case 2:
                    StateText = "Connection droped unexpected during fax receiving.";
                    break;
                }

                // Hylafax: <qfile> <ModemDeviceID> <CommID> <Reason> <CIDNumber> <CIDName> <destination>
                executeCommand.PrintAppend (" \"%eS\" \"%eS\" \"%09u\" \"%es\" \"%eS\" \"\" \"%eS\"",
                                            GetRecvFiles()->GetFirst(), &FaxDevice->DeviceName, jobNr,
                                            StateText, GetReceiveID(), &MyNumber);
                WriteXferLog ("RECV", jobNr, 0, 0, (char *)FaxDevice->DeviceName.GetPointer(), 0,
                              &MyNumber, StateText);
            } else  {
                // mgetty:  <RecvStatus> <Hangup Code> "<sender id>" "<receiver id>" <nr of pages> <file(s)>
                tUInt recvStatus = 0;
                switch (GetRecvState()) {
                default:
                case 0:
                    recvStatus = 6;
                    break;
                case 3:
                    if (CountAllLines != 0) {
                        if (CountIllegalLines * 100 / CountAllLines > 50) {
                            recvStatus = 12;
                        }
                        break;
                    }
                    // no break!
                case 1:
                    recvStatus = 7;
                    break;
                case 2:
                    recvStatus = 12;
                    break;
                }

                executeCommand.PrintAppend (" %u 0x%X \"", recvStatus, Reason);
                executeCommand.PrintAppend ("%eS", GetReceiveID());
                executeCommand.Append ("\" \"");
                if (!MyNumber.IsEmpty()) {
                    executeCommand.PrintAppend ("%eS", &MyNumber);
                }
                executeCommand.PrintAppend ("\" %u", GetPageCount());
                for (COneMultiString *pLauf = GetRecvFiles()->GetFirst(); (pLauf != 0); pLauf = pLauf->GetNext()) {
                    executeCommand.PrintAppend (" \"%eS\"", pLauf);
                }
            }

            if (executeCommand.ErrorDetected() == vTrue) {
                WriteLog (LOG_INFO, "Fax received and stored under %s.\n",
                        GetRecvFiles()->GetFirst()->GetPointer());
                WriteLog (LOG_ERR, "Allocating memory error! Can't notify new received fax!\n");
                FaxDevice->WriteStatus ("Allocating memory error! Can't notify new received fax!");
                NotifyFIFO ('W');
                quitRecv = 1;
            } else {
                WriteLog (LOG_INFO, "Fax received and calling '%s'.\n", executeCommand.GetPointer());
                fflush (0);
                runCmd ((char *)executeCommand.GetPointer(), vTrue);
            }
        } else {
            WriteLog (LOG_INFO, "Fax received and stored under %s.\n", GetRecvFiles()->GetFirst()->GetPointer());
        }
        GetRecvFiles()->RemoveAndDeleteAll();
    }
    WriteLog (LOG_INFO, "Connection is droped with reason 0x%X (%s).\n", Reason, capi_info2str (Reason));
    if (hJobFile > 0) {
        WriteLog (LOG_INFO, "SESSION END\n");
        fclose (hJobFile);
        hJobFile = 0;
    }
    jobNr       = 0;
    ONumber.RemoveAll();
    MyNumber.RemoveAll();
    FaxDevice->WriteStatus ("Running and idle");
    NotifyFIFO ('I');
    NotifyFIFO ('R');
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::IsReseted (void) {
    dhead ("CFaxReceive::IsReseted", DCON_CFaxReceive);
    //dassert (CapiInfoIsSuccess (Info));
    if (quitRecv == 0) {
        WriteLog (LOG_ERR, "Can't register at CAPI! Is CAPI loaded and accessible from this program?\n");
    } else {
        quitRecv = 0;
    }
    CFaxReceiveDevice *fd = FaxDevice;
    delete this;
    fd->ReceiveStoped();
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::PageEndReached (tBool /*IsLastPage*/) {
    dhead ("CFaxReceive::PageEndReached", DCON_CFaxReceive);
    NotifyFIFO ('P');
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceive::WriteLog (tSInt priority, char *text, ...) {
    dhead ("CFaxReceive::WriteLog", DCON_CFaxReceive);
    char    timebuf[MAX_STRING_SIZE];
    timeval tv;
    va_list params;
    va_start (params, text);

    gettimeofday (&tv, 0);
    strftime (timebuf, MAX_STRING_SIZE, "%h %d %T", localtime((time_t*) &tv.tv_sec));
    sprintf (timebuf + strlen(timebuf), ".%02lu: [%5d]: ", tv.tv_usec / 10000, getpid());

    if (hJobFile) {
        fprintf (hJobFile, "%s", timebuf);
        vfprintf (hJobFile, text, params);
        va_end (params);
        va_start (params, text);
    }
    FaxDevice->vWriteLog (priority , text, params);
    va_end (params);
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CFaxReceive::GetRecvFileMode (void) {
    dhead ("CFaxReceive::GetRecvFileMode", DCON_CFaxReceive);
    return FaxDevice->RecvFileMode;
}

/*===========================================================================*\
\*===========================================================================*/

CDynamicString *CFaxReceive::GetSpoolDir (void) {
    dhead ("CFaxReceive::GetSpoolDir", DCON_CFaxReceive);
    return &FaxDevice->MainClass->SpoolDir;
}

/*===========================================================================*\
\*===========================================================================*/

tBool CFaxReceive::NotifyFIFO (char Type) {
    dhead ("CFaxReceive::NotifyFIFO", DCON_CFaxReceive);
    dparams ("%c", Type);
    if (GetFormat() != FaxFormat_Hylafax) {
        return vTrue;
    }
    CDynamicString FaxInfo;
    tBool  recv = vFalse;
    char  *text = 0;
    switch (Type) {
    // Modem part
    case 'I':
        if (jobNr > 0) {
            FaxInfo.Print ("%09u", jobNr);
            text = (char *)FaxInfo.GetPointer();
        }
        break;
    case 'C':
        // TODO? MSN eintragen?
        text = (char *)ONumber.GetPointer();
        break;

    // Receive part
    case 'S':
    case 'P':
    case 'D':
        if (GetRecvFiles() && (GetRecvFiles()->IsEmpty() == vFalse)) {
            dassert (GetRecvFiles()->GetFirst()->IsEmpty() == vFalse);
            FaxInfo. Print ("%lx,%x,%x,%S,%09u,\"%S\",\"%S\",\"%s\"", (tUInt64)(time (0) - GetStartTime()),
                            GetPageCount(), FaxPropEncode (GetResolution(), GetBaudRate()),
                            GetRecvFiles()->GetFirst(), jobNr, GetReceiveID(), &ONumber, "");
            text = (char *)FaxInfo.GetPointer();
            recv = vTrue;
        } else {
            dassert (0);
            return vFalse;
        }
        break;
    default:
        return FaxDevice->NotifyFIFO (Type);
    }
    return FaxDevice->NotifyFIFO (recv, Type, text);
}

/*===========================================================================*\
\*===========================================================================*/

tBool CFaxReceive::GetJobFile (void) {
    dhead ("CFaxReceive::GetJobFile", DCON_CFaxReceive);
    dassert (GetFormat() == FaxFormat_Hylafax);
    jobNr = 0;
    if (hJobFile != 0) {
        fclose (hJobFile);
        hJobFile = 0;
    }
    char  astring[MAX_STRING_SIZE];
    tSInt len;
    tUInt tmpJobFileNr = 1;
    tSInt tmpFileD     = -1;
    tSInt SeqFile      = open (HYLAFAX_LOG_DIR HYLAFAX_SEQF_FILE, O_CREAT | O_RDWR, 0644);
    if (SeqFile >= 0) {
        flock (SeqFile, LOCK_EX);
        len = read (SeqFile, astring, MAX_STRING_SIZE);
        astring[(len <= 0) ? 0 : len] = '\0';
        tmpJobFileNr = atoi (astring) + 1;
    }
    for (tUInt trys = MAX_TRY_COUNT; trys > 0; trys--, tmpJobFileNr++) {
        if (tmpJobFileNr < 1 || tmpJobFileNr > MAX_SEQ_NUMBER) {
            tmpJobFileNr = 1;
        }
        sprintf (astring, HYLAFAX_LOG_DIR "c%09u", tmpJobFileNr);
        tmpFileD = open (astring, O_RDWR | O_CREAT | O_EXCL, FaxDevice->MainClass->LogFileMode);
        dprint ("try(%s,%u,%d) ", astring, strlen (astring), tmpFileD);
        if (tmpFileD >= 0) {
            flock (tmpFileD, LOCK_EX | LOCK_NB);
            hJobFile = fdopen (tmpFileD, "w");
            dprint ("fdopen(%x,%d) ", hJobFile, tmpFileD);
            if (hJobFile != 0) {
                jobNr = tmpJobFileNr;
                if (SeqFile >= 0) {
                    sprintf (astring , "%u", tmpJobFileNr);
                    len = strlen (astring);
                    lseek (SeqFile, 0, SEEK_SET);
                    write (SeqFile, astring, len);
                    ftruncate (SeqFile, len);
                }
                break;
            }
            close (tmpFileD);
            tmpFileD = -1;
            continue;
        }
        if (errno != EEXIST) {
            break;
        }
    }
    if (SeqFile >= 0) {
        close (SeqFile);
    }
    return (hJobFile != 0);
}

/*===========================================================================*\
\*===========================================================================*/


