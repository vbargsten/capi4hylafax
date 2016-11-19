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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <time.h>
#include "faxsend.h"
#include "osImport.h"
#include "ConfAssi.h"
#include "FilePars.h"
#include "misc.h"

/*---------------------------------------------------------------------------*\
    from capiutils.h:
\*---------------------------------------------------------------------------*/

extern "C" char *capi_info2str(c_word reason);

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define FAX_FILTER_BUFFER_SIZE       1000
#define CAPI_BUFFER_SIZE             CAPI_MAX_DATA_SIZE

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

//#define GENERATE_DEBUGSFFDATAFILE

#ifdef GENERATE_DEBUGSFFDATAFILE
static FILE *DebugSffDataFile = 0;
#endif

/*===========================================================================*\
\*===========================================================================*/

CFaxSend::CFaxSend (void (*FinishedFunc)(void))
  : exitState (0),
    finishFunc (FinishedFunc),
    SpoolDir (DEFAULT_SPOOL_DIRECTORY),
    PollRcvdCmd (HYLAFAX_POLL_CMD),
    LogFileName (DEFAULT_C4H_LOGFILENAME),
    LogLevel (0),
    LogFileMode (DEFAULT_FILE_MODE),
    SuppressMSN (vFalse),
    UseFaxCIPValue (0),
    RecvFileMode (DEFAULT_FILE_MODE),
    m_commID (0),
    hLogFile (0),
    hJobFile (0),
    enableEcho (vFalse),
    SendDataInUse (UNIQUE_INIT_VALUE) {

    dhead ("CFaxSend - Constructor", DCON_CFaxSend);
    SetOutController (1);
    SetReceiveID (&TelNumber);
}

/*===========================================================================*\
\*===========================================================================*/

CFaxSend::~CFaxSend (void) {
    dhead ("CFaxSend - Destructor", DCON_CFaxSend);
    CloseSend();
    CloseRecv();
    CloseJobFile();
    if (hLogFile != 0) {
        fclose (hLogFile);
        hLogFile = 0;
    }
}

/*===========================================================================*\
\*===========================================================================*/

tBool CFaxSend::Config (tChar *confName, char *devName, FaxFormatType faxType, int argc, char **argv) {
    dhead ("CFaxSend::Config", DCON_CFaxSend);
    dparams ("%s,%x,%x", confName, faxType, argc);
    dassert (confName != 0);
    dassert (devName != 0);
    dassert (argv != 0);
    DeviceName.Set (devName);
    SetFormat (faxType);
    optind = 1;

    tUInt                 RingingTimeVal = 0;
    tUInt                 OutController  = 1;
    CConfigFileParser     FileParser;
    CConfigParserSection *mainSect;

    ConfigAssignment_Item MainAssignments[] = {
        { "SpoolDir",       ConfigAssignment_String, &SpoolDir },
        { "PollRcvdCmd",    ConfigAssignment_String, &PollRcvdCmd },
        { "LogFile",        ConfigAssignment_String, &LogFileName },
        { "LogFileMode",    ConfigAssignment_UInt,   &LogFileMode },
        { "LogTraceLevel",  ConfigAssignment_UInt,   &LogLevel },
        { 0,                ConfigAssignment_UInt,   0 }
    };
    ConfigAssignment_Item Assignments[] = {
        { "HylafaxDeviceName",  ConfigAssignment_String, &DeviceName },
        { "FAXNumber",          ConfigAssignment_String, &OrgFaxNumber },
        { "LocalIdentifier",    ConfigAssignment_String, &OrgIdentifier },
        { "OutgoingMSN",        ConfigAssignment_String, &OutMSN },
        { "NumberPrefix",       ConfigAssignment_String, &NumberPrefix },
        { "NumberPlusReplacer", ConfigAssignment_String, &NumberPlusReplace },
        { "OutgoingController", ConfigAssignment_UInt,   &OutController },
        { "SuppressMSN",        ConfigAssignment_UInt,   &SuppressMSN },
        { "UseISDNFaxService",  ConfigAssignment_UInt,   &UseFaxCIPValue },
        { "RingingDuration",    ConfigAssignment_UInt,   &RingingTimeVal },
        { "RecvFileMode",       ConfigAssignment_UInt,   &RecvFileMode },
        { 0,                    ConfigAssignment_UInt,   0 }
    };

    printf ("The config file is %s.\n", confName);
    switch (FileParser.ParseFile (confName)) {
    case iErr_File_OpenFailed:
        printf ("Can't open config file \"%s\". Use default values.\n", confName);
        break;
    case i_Okay:
        break;
    case iErr_OutOfMemory:
        dassert (0);
        printf ("Running out of memory during reading of config file!\n");
        return vFalse;
    default:
        dassert (0);
        printf ("Can't read config file \"%s\"!\n", confName);
        return vFalse;
    }
    mainSect = FileParser.GetMainSection();
    if (!mainSect) {
        printf ("Not enough Memory for config file parsing!\n");
        return vFalse;
    }
    ConfigValuesAssignment (&mainSect->ValueList, &MainAssignments[0]);
    for (CConfigParserSection *laufSect = (CConfigParserSection *)mainSect->SectionList.GetFirst();
        laufSect; laufSect = (CConfigParserSection *)laufSect->GetNext()) {

        if (ConfigValuesTestName (&laufSect->ValueList, &Assignments[0]) != 1) {
            ConfigValuesAssignment (&laufSect->ValueList, &Assignments[0]);
            SetOutController (OutController);
        }
    }
    mainSect->ValueList.RemoveAndDeleteAll();
    mainSect->SectionList.RemoveAndDeleteAll();

    char ch;
    while ((ch = getopt (argc, argv, "Hh?vLlpnf:C:c:d:m:P:")) != -1) {
        switch (ch) {
        case 'c':
            if (mainSect && optarg && *optarg) {
                CDynamicString value;
                value.Set (optarg);
                if (value.IsEmpty() == vFalse) {
                    FileParser.ParseLine (&value);
                    ConfigValuesAssignment (&mainSect->ValueList, &MainAssignments[0]);
                    ConfigValuesAssignment (&mainSect->ValueList, &Assignments[0]);
                    mainSect->ValueList.RemoveAndDeleteAll();
                }
            }
            break;
        case 'v':
            enableEcho = vTrue;
            break;
        case 'L':
            LogLevel = LOG_TRACE_ALL;
            break;
        }
    }
    if (  (LogFileName.IsEmpty() == vFalse) && LogLevel) {
        hLogFile = fopen ((char *)LogFileName.GetPointer(), "a");
        dprint ("LogFile=%x-%s ", hLogFile, (char *)LogFileName.GetPointer());
        if (hLogFile) {
            fchmod (fileno (hLogFile), LogFileMode);
        }
    }
    WriteLog (LOG_INFO, "\nC4H-Version " VERSION
    #ifdef C_PLATFORM_64
                " (64Bit).\n\n");
    #else
                " (32Bit).\n\n");
    #endif
    Identifier.Set (&OrgIdentifier);
    FaxNumber.Set (&OrgFaxNumber);
    SetRingingTimeout (RingingTimeVal);
    if (GetFormat() == FaxFormat_Hylafax) {
        SpoolDir.SetRelative (HYLAFAX_POLL_DIR); // Adjust SpoolDir to correct place
    }
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::SetTagLine (CDynamicString *pTagLineFormat) {
    dhead ("CFaxSend::SetTagLine", DCON_CFaxSend);
    dassert (pTagLineFormat != 0);
    // parsing TagLine
    if (!pTagLineFormat->IsEmpty()) {
        tUInt pos = pTagLineFormat->FindChar ('|');
        if (pos != MAXVAL_tUInt) {
            // '|' was found!
            if (pos > 0) {
                // Ignore empty Identifier
                Identifier.Set (pTagLineFormat->GetPointer(), pos);
            }
            pos++;
            if (pos < pTagLineFormat->GetLen()) {
                // Ignore empty FaxNumber
                FaxNumber.Set (pTagLineFormat->GetPointer() + pos, pTagLineFormat->GetLen() - pos);
            }
        } else {
            Identifier.Set (pTagLineFormat);
        }
    } else {
        Identifier.Set (&OrgIdentifier);
        FaxNumber.Set (&OrgFaxNumber);
    }
}

/*===========================================================================*\
\*===========================================================================*/

tSInfo CFaxSend::Send (CDynamicString *Number, CMultiString *FaxFilesList, tUInt Resolution,
                       tUInt jobID, tUInt *pCommID, CDynamicString *pPollStr) {
    dhead ("CFaxSend::Send", DCON_CFaxSend);
    dassert (Number != 0);
    dparams ("%s,%u,%u", Number->GetPointer(), Resolution, jobID);
    dassert (FaxFilesList != 0);
    exitState = 0;
    // check params for fax polling
    PollString.RemoveAll();
    if (pPollStr && (pPollStr->IsEmpty() == vFalse)) {
        dassert (pPollStr->IsEmpty() == vFalse);
        if (PollString.Set (pPollStr) == vFalse) {
            WriteLog (LOG_ERR, "Memory Error during initialisation.\n");
            return iErr_OutOfMemory;
        }
        if (PollRcvdCmd.IsEmpty() == vTrue) {
            // no script specified
            dwarning (0);
            WriteLog (LOG_ERR, "Fax polling enabled without a valid PollRcvd script.");
            exitState = 13;
            return iErr_IniFile_ParamCombination;
        }
        if (FaxFilesList->GetFirst() != 0) {
            dwarning (0);
            dprint ("Wrong FaxFile=%s ",FaxFilesList->GetFirst()->GetPointer());
            WriteLog (LOG_ERR, "Not allowed to poll and send a Fax within the same connection.");
            exitState = 13;
            return iErr_IniFile_ParamCombination;
        }
    } else {
        dassert (FaxFilesList->IsEmpty() == vFalse);
    }

    char *ModeText;
    switch (GetFormat()) {
    default: {
        ModeText = "Hylafax";
        tUInt commID = GetJobFile();
        dassert (pCommID != 0);
        if (*pCommID) {
            *pCommID = commID;
        }
        WriteLog (LOG_INFO, "SESSION BEGIN %09u %s\n", jobID, FaxNumber.GetPointer());
        WriteLog (LOG_INFO, "%s FAX: JOB %u DEST %s COMMID %09u\n",
                  PollString.IsEmpty() ? "SEND" : "POLL", jobID, Number->GetPointer(), commID);
        break;
    }
    case FaxFormat_TIFF:
        ModeText = "TIFF";
        break;
    case FaxFormat_G3:
        ModeText = "G3";
        break;
    case FaxFormat_SFF:
        ModeText = "SFF";
        break;
    }
    TelNumber.ErrorDetected();
    TelNumber.Set (&NumberPrefix);
    if (  (Number->GetChar (0) == '+')
       && (!NumberPlusReplace.IsEmpty())) {
        if (Number->GetLen() <= 1) {
            WriteLog (LOG_ERR, "Telephonenumber to small (only includes '+').\n");
            return iErr_OutOfMemory;
        }
        TelNumber.Append (&NumberPlusReplace);
        dassert (Number->GetPointer() != 0);
        dassert (Number->GetLen() > 1);
        TelNumber.Append (Number->GetPointer() + 1, Number->GetLen() - 1);
    } else {
        TelNumber.Append (Number);
    }
    if (TelNumber.ErrorDetected() == vTrue) {
        WriteLog (LOG_ERR, "Memory Error during initialisation.\n");
        return iErr_OutOfMemory;
    }

    tSInfo fret = i_Okay;
    cp_b3configuration_faxext B3Config (b3_faxextopt_highres | b3_faxextopt_jbig, &FaxNumber, &Identifier);
    cp_ncpi_faxext            NCPI     (ncpi_faxextopt_highres);

    WriteLog (LOG_INFO, "Try to connect to fax number %s in %s mode%s on controller %d.\n",
              TelNumber.GetPointer(), ModeText, PollString.IsEmpty() ? "" : " (polling active)",
              GetOutController());
    if (PollString.IsEmpty()) {
        fret = OpenSend (FaxFilesList, Resolution);
    } else {
        NCPI.Options |= ncpi_faxextopt_poll;  // Enable FaxPolling
    }
    if (infoError (fret)) {
        switch (fret) {
        case iErr_Filter_OpenFailed:
            exitState = 11;
            break;
        case iErr_File_NotExist:
            exitState = 1;
            break;
        }
        RETURN ('x', fret);
    }
    exitState       = 10;
    basicUniqueClear (&SendDataInUse);

    SetB1Parameters (b1prot_G3Fax);
    SetB2Parameters (b2prot_G3Fax);
    SetB3Parameters (b3prot_G3Fax_extended, &B3Config);
    SetNCPI (&NCPI);
    SetOwnNumber (&OutMSN, (SuppressMSN) ? CALLNUM_INDICATOR_SUPPRESS : CALLNUM_INDICATOR_DEFAULT);

    MakeCall (&TelNumber, (UseFaxCIPValue) ? cipvalue_G3Fax : cipvalue_3_1kHzAudio);
    RETURN ('x', i_Okay);
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::PageEndReached (tBool /* IsLastPage */) {
    dhead ("CFaxSend::PageEndReached", DCON_CFaxSend);
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CFaxSend::GetRecvFileMode (void) {
    dhead ("CFaxSend::GetRecvFileMode", DCON_CFaxSend);
    return RecvFileMode;
}

/*===========================================================================*\
\*===========================================================================*/

CDynamicString *CFaxSend::GetSpoolDir (void) {
    dhead ("CFaxSend::GetSpoolDir", DCON_CFaxSend);
    return &SpoolDir;
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::IncomingCall (c_cipvalue, CConstString *, CConstString *) {
    dhead ("CFaxSend::IncomingCall", DCON_CFaxSend);
    // Nothing to do!
    dwarning (0);
    AnswerCall (creject_Ignore);
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::DoConnecting (void) {
    dhead ("CFaxSend::DoConnecting", DCON_CFaxSend);
    // Nothing to do!
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::ConnectProceeding (CConstString *) {
    dhead ("CFaxSend::ConnectProceeding", DCON_CFaxSend);
    ConnectionStarted();
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::IsConnected (cp_ncpi_all *pNCPI) {
    dhead ("CFaxSend::IsConnected", DCON_CFaxSend);
    ConnectLog (pNCPI);
    exitState = 11;
    if (PollString.IsEmpty() == vFalse) {
        if (!pNCPI || !CheckOneFlag (pNCPI->Options, 0x1))  {
            // Polling could not be enabled => drop
            WriteLog (LOG_ERR, "Fax polling property could not be negotiated!\n");
            exitState = 8;
        } else if (infoError (OpenRecv())) {
            WriteLog (LOG_ERR, "Can't create file for the fax!\n");
        } else {
            // fax polling is active. Now wait for data
            return;
        }
        WriteLog (LOG_ERR, "Disconnecting now.\n");
        Disconnect (DisconnectKind_Fast);
    } else {
        #ifdef GENERATE_DEBUGSFFDATAFILE
            dassert (DebugSffDataFile == 0);
            DebugSffDataFile = fopen ("/var/log/c2faxsend_dbgdatafile.sff", "w");
        #endif
        SendData();
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::PutDataComplete (void *hDataID, c_info /*Reason*/) {
    dhead ("CFaxSend::PutDataComplete", DCON_CFaxSend);
    //dprint ("CFaxSend::PutDataComplete Reason 0x%X (%s).\n", Reason, capi_info2str (Reason));
    dprint ("CFaxSend::PutDataComplete PageCount = %d\n", GetPageCount());
    dassert (PollString.IsEmpty() == vTrue);

    delete [] (tUChar *)hDataID;
    SendData();
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::GetData (tUByte *Data, tUShort DataLength, tUShort DataHandle) {
    dhead ("CFaxSend::GetData", DCON_CFaxSend);
    dassert (PollString.IsEmpty() == vFalse);
    if (infoError (RecvSFF (Data, DataLength))) {
        Disconnect (DisconnectKind_Fast);
    }
    GetDataComplete (DataHandle);
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::DoDisconnecting (c_info Reason, cp_ncpi_all *pNCPI) {
    dhead ("CFaxSend::DoDisconnecting", DCON_CFaxSend);
    dprint ("CFaxSend::DoDisconnecting GetSendState = %d\n", GetSendState());
    dwarning (pNCPI == 0);
    if (pNCPI) {
        // sync PageCount with the value returned from CAPI
        if (GetFormat() == FaxFormat_SFF) {
            SetPageCount (pNCPI->Pages);
        } else if (GetPageCount() != pNCPI->Pages) {
            dwarning (exitState != 0);
            dwarning (GetPageCount() > pNCPI->Pages);
            WriteLog (LOG_ERR, "Only %d from %d pages are transfered!\n", pNCPI->Pages, GetPageCount());
            SetPageCount (pNCPI->Pages);
            if (exitState == 0) {
                exitState = 12;
            }
        }
    }
    if (PollString.IsEmpty()) {
        if ((GetSendState() < 2) || (ci_NoError != Reason)) {
            WriteLog (LOG_INFO, "Connection was dropped with Reason 0x%X (%s) before Fax was completely transfered.\n",
                      Reason, capi_info2str (Reason));
            exitState = 12;
        }
    } else {
        if ((GetRecvState() < 3) || (ci_NoError != Reason)) {
            WriteLog (LOG_INFO, "Connection was dropped with Reason 0x%X (%s) before Fax was completely received.\n",
                      Reason, capi_info2str (Reason));
            exitState = 12;
        } else {
            exitState = 0;
        }
    }
    StopSend();
    StopRecv();
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::IsDisconnected (c_info Reason) {
    dhead ("CFaxSend::IsDisconnected", DCON_CFaxSend);
    WriteLog (LOG_INFO, "Connection dropped with Reason 0x%X (%s).\n", Reason, capi_info2str (Reason));

    // connection dropped before established (10) or transfer still in progress (11)
    if ((exitState == 10) || (exitState == 11)) {
        if (exitState == 11) {
            exitState = 12;
        }
        switch ((Reason >> 8) & 0xFF) {
        case 0x00:
        case 0x10:
            exitState = 2;
            break;
        case 0x11:
        case 0x20:
        case 0x30:
            exitState = 3;
            break;
        case 0x33:
            exitState = ((Reason & 0xFF) == 0x01) ? 5 : 10;
            break;
        case 0x34:
            switch (Reason & 0xFF) {
            case 0x91:
            case 0xA2:
            case 0xA9:
                exitState = 4;
                break;
            }
            break;
        }
    }
    CloseSend();
    CloseRecv();
    #ifdef GENERATE_DEBUGSFFDATAFILE
        if (DebugSffDataFile) {
            fclose (DebugSffDataFile);
            DebugSffDataFile = 0;
        }
    #endif
    ResetChannel();

    if (!PollString.IsEmpty() && GetRecvFiles() && !GetRecvFiles()->IsEmpty()) {
        // notify about received fax
        dassert (!PollRcvdCmd.IsEmpty());
        CDynamicString executeCommand;
        executeCommand.Set (&PollRcvdCmd);
        char *StateText = "";

        if (GetFormat() == FaxFormat_Hylafax) {
            switch (GetRecvState()) {
            default:
            case 0:
                StateText = "Can't build up connection for incoming call.";
                break;
            case 1:
                StateText = "Connection was established but no fax data was received.";
                break;
            case 2:
                StateText = "Connection droped unexpected during fax receiving.";
                break;
            case 3:
                break;
            }

            CDynamicString commStr;
            commStr.Print ("%09u", m_commID);

            // Hylafax: <mailaddr> <qfile> <ModemDeviceID> <CommID> <Reason>
            executeCommand.PrintAppend (" \"%S\" \"%S\" \"%S\" \"%S\" \"%s\"", &PollString,
                                        GetRecvFiles()->GetFirst(), &DeviceName, &commStr, StateText);
        } else  {
            // mgetty:  <RecvStatus> <Hangup Code> "<sender id>" "<poll text>" <nr of pages> <file(s)>
            tUInt recvStatus = 0;
            switch (GetRecvState()) {
            default:
            case 0:
                recvStatus = 6;
                break;
            case 1:
                recvStatus = 7;
                break;
            case 2:
                recvStatus = 12;
                break;
            case 3:
                break;
            }
            executeCommand.PrintAppend (" %u 0x%X \"%S\" \"%S\" %u", recvStatus, Reason, GetReceiveID(),
                                        &PollString, GetPageCount());
            for (COneMultiString *pLauf = GetRecvFiles()->GetFirst(); (pLauf != 0); pLauf = pLauf->GetNext()) {
                executeCommand.PrintAppend (" \"%s\"", pLauf->GetPointer());
            }
        }

        if (executeCommand.ErrorDetected() == vTrue) {
            WriteLog (LOG_INFO, "Fax received and stored under %s.\n", GetRecvFiles()->GetFirst());
            WriteLog (LOG_ERR, "Allocating memory error! Can't notify new received fax!\n");
            exitState = 9;
        } else {
            WriteLog (LOG_INFO, "Fax received and calling '%s'.\n", executeCommand.GetPointer());
            fflush (0);
            runCmd ((char *)executeCommand.GetPointer(), vTrue);
        }
        GetRecvFiles()->RemoveAndDeleteAll();
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::IsReseted (void) {
    dhead ("CFaxSend::IsReseted", DCON_CFaxSend);
    if (hLogFile != 0) {
        fclose (hLogFile);
        hLogFile = 0;
    }
    if (finishFunc) {
        finishFunc();
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::SendData (void) {
    dhead ("CFaxSend::SendData", DCON_CFaxSend);
    tUInt   len    = 0;
    tUChar *buffer = 0;

    if (basicUniqueStart (&SendDataInUse) == vTrue) {
        do {
            while ((IsPutQueueFull() == vFalse) && (GetSendState() < 2)) {
                buffer = new tUChar [CAPI_BUFFER_SIZE];
                if (!buffer) {
                    WriteLog (LOG_WARNING, "Allocating memory for new buffer failed!\n");
                    dwarning (0);
                    break;
                }
                switch (SendSFF (buffer, CAPI_BUFFER_SIZE, &len)) {
                case i_Okay:
                    if (len) {
                        //if (UseOnlyLowRes == vTrue) {
                            //TODO: eliminate every second line on fine faxes
                            //SffLowResFilter.Change (buffer, len);
                        //}
                        #ifdef GENERATE_DEBUGSFFDATAFILE
                            if (DebugSffDataFile) {
                                fwrite (buffer, len, 1, DebugSffDataFile);
                            }
                        #endif
                        if (PutData (buffer, len, buffer) == vFalse) {
                            dassert (0);
                            delete [] buffer;
                            Disconnect (DisconnectKind_Fast);
                            break;
                        }
                        len = 0;
                    } else {
                        delete [] buffer;
                    }
                    if (GetSendState() == 2) {
                        WriteLog (LOG_INFO, "Fax file completely transfered to CAPI.\n");
                        exitState = 0;
                        Disconnect (DisconnectKind_WaitNoPut);
                    }
                    break;
                case iErr_File_NotExist:
                    exitState = 1;
                    // no break!
                default:
                    delete [] buffer;
                    Disconnect (DisconnectKind_Fast);
                    break;
                }
            }
        } while (basicUniqueStop (&SendDataInUse) == vFalse);
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::WriteLog (tSInt priority, char *text, ...) {
    dhead ("CFaxSend::WriteLog", DCON_CFaxSend);
    char   *statusText = 0;
    char    timebuf[MAX_STRING_SIZE];
    timeval tv;
    va_list params;
    va_start (params, text);

    gettimeofday (&tv, 0);
    strftime (timebuf, MAX_STRING_SIZE, "%h %d %T", localtime((time_t*) &tv.tv_sec));
    sprintf (timebuf + strlen(timebuf), ".%02lu: [%5d]: ", tv.tv_usec / 10000, getpid());

    if (hJobFile) {
        fprintf (hJobFile, timebuf);
        vfprintf (hJobFile, text, params);
        va_end (params);
        va_start (params, text);
    }
    if (enableEcho) {
        vprintf (text, params);
        va_end (params);
        va_start (params, text);
    }

    switch (priority) {
    default:
        if (LogLevel < LOG_TRACE_ALL) {
            return;
        }
        break;
    case LOG_INFO:
    case LOG_NOTICE:
        if (LogLevel < LOG_TRACE_INFOS) {
            return;
        }
        statusText = "INFO: ";
        break;
    case LOG_WARNING:
        if (LogLevel < LOG_TRACE_WARNINGS) {
            return;
        }
        statusText = "WARNING: ";
        break;
    case LOG_ERR:
        if (LogLevel < LOG_TRACE_ERRORS) {
            return;
        }
        statusText = "ERROR: ";
        break;
    }

    if (hLogFile) {
        fprintf (hLogFile, timebuf);
        fprintf (hLogFile, DEFAULT_SENDPROG_NAME " - ");
        if (statusText) {
            fprintf (hLogFile, statusText);
        }
        vfprintf (hLogFile, text, params);
        va_end (params);
        va_start (params, text);
        fflush (hLogFile);
    }
    if (priority == LOG_ERR) {
        char errortext[MAX_STRING_SIZE];
        vsnprintf (errortext, MAX_STRING_SIZE, text, params);
        openlog (DEFAULT_SENDPROG_NAME, LOG_PERROR, 0);
        syslog (priority, errortext);
        closelog();
    }
    va_end (params);
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CFaxSend::GetJobFile (void) {
    dhead ("CFaxSend::GetJobFile", DCON_CFaxSend);
    dassert (GetFormat() == FaxFormat_Hylafax);

    CloseJobFile();
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
        tmpFileD = open (astring, O_RDWR | O_CREAT | O_EXCL, LogFileMode);
        dprint ("try(%s,%u,%d) ", astring, strlen (astring), tmpFileD);
        if (tmpFileD >= 0) {
            flock (tmpFileD, LOCK_EX | LOCK_NB);
            hJobFile = fdopen (tmpFileD, "w");
            dprint ("fdopen(%x,%d) ", hJobFile, tmpFileD);
            if (hJobFile != 0) {
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
    m_commID = (hJobFile) ? tmpJobFileNr : 0;
    return m_commID;
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxSend::CloseJobFile (void) {
    if (hJobFile != 0) {
        fclose (hJobFile);
        hJobFile = 0;
    }
    m_commID = 0;
}


/*===========================================================================*\
\*===========================================================================*/

