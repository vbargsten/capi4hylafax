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

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>
#include "faxsend.h"
#include "ConfAssi.h"
#include "FilePars.h"
#include "DynQueue.h"
#include "aString.h"
#include "PrctlDmp.h"
#include "DbgFile.h"
#include "osImport.h"
#include "misc.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

static sem_t syncsema;

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

static void FinishFunc (void) {
    sem_post (&syncsema);
}

/*===========================================================================*\
\*===========================================================================*/

static void WriteQFile (tChar *QFileName, tUInt commID, tUInt HylafaxExitState, CFaxSend *fs, char **statustext) {
    dhead ("WriteQFile", DCON_CFaxSend);
    dassert (QFileName != 0);
    CDynamicQueue textQ (60, sizeof (char *));
    char  text[4 * MAX_STRING_SIZE];
    char *ne;
    tSInt triescount;
    tUInt dialortry  = 1;
    *statustext      = "";
    FILE *qf         = fopen ((char *)QFileName, "r+");

    if (!qf) {
        return;
    }
    switch (fs->exitState) {
    case 255:
        *statustext = "Sending job";
        dialortry  = 0;
        break;
    case 0:
        dialortry  = 2;
        break;
    case 1:
        *statustext = "Can't open fax file.";
        break;
    case 2:
        *statustext = "CAPI not installed or active.";
        break;
    case 3:
        *statustext = "Error in communication to CAPI.";
        break;
    case 4:
        *statustext = "The telephone number is busy or temporary not available.";
        break;
    case 5:
        *statustext = "Protocol error layer 1: Mostly your ISDN cable is not inserted.";
        break;
    case 6:
        *statustext = "Can't build up B-Channel.";
        break;
    case 7:
        dialortry  = 2;
        *statustext = "Connection build up, but data transfer wasn't possible.";
        break;
    case 8:
        *statustext = "Fax polling property could not be negotiated.";
        break;
    case 9:
        *statustext = "PollRcvd script could not be started. Fax was completely received.";
        break;
    case 10:
        *statustext = "An error occured while trying to establish the connection.";
        break;
    case 11:
        *statustext = "Can't initialize conversion filters.";
        break;
    case 12:
        dialortry  = 2;
        *statustext = "An error occured during the fax transfer.";
        break;
    case 13:
        *statustext = "Wrong command line or help parameter was specified.";
        break;
    default:
    case 15:
        *statustext = "Very hard error.";
        break;
    }

    // we asume that we can always get a whole line with fgets
    while (!feof (qf) && fgets (text, sizeof (text), qf)) {
        if (!*text) {
            continue;
        }
        ne = strchr (text, ':');
        if (!ne || (ne == text)) {
            continue;
        }
        *ne = '\0';
        if (strcasecmp ("returned", text) == 0) {
            dprint ("found returned ");
            sprintf (ne + 1, "%u\n", HylafaxExitState);
        } else if (strcasecmp ("status", text) == 0) {
            dprint ("found status ");
            sprintf (ne + 1, "%s\n", *statustext);
        } else if (commID && (strcasecmp ("commid", text) == 0)) {
            dprint ("found commid ");
            sprintf (ne + 1, "%09u\n", commID);
        } else if (strcasecmp ("npages", text) == 0) {
            dprint ("found npages ");
            sprintf (ne + 1, "%u\n", fs->GetPageCount());
        } else if (((dialortry == 1) || (dialortry == 2)) && ((strcasecmp ("ndials", text) == 0) || (strcasecmp ("totdials", text) == 0))) {
            dprint ("found _dials ");
            triescount = atol (ne + 1) + 1;
            if (triescount <= 0) {
                dprintErr ("negative Value ");
                triescount = 1;
            }
            sprintf (ne + 1, "%u\n", triescount);
        } else if ((dialortry == 2) && ((strcasecmp ("ntries", text) == 0) || (strcasecmp ("tottries", text) == 0))) {
            dprint ("found _tries ");
            triescount = atol (ne + 1) + 1;
            if (triescount <= 0) {
                dprintErr ("negative Value ");
                triescount = 1;
            }
            sprintf (ne + 1, "%u\n", triescount);
        } else if (strcasecmp ("signalrate", text) == 0) {
            dprint ("found signalrate ");
            sprintf (ne + 1, "%d\n", fs->GetBaudRate());
        } else if (strcasecmp ("dataformat", text) == 0) {
            dprint ("found dataformat ");
            sprintf (ne + 1, "1-D MR\n"); // SFF includes 1-D MR (Huffman) data
        }

        *ne = ':';
        ne  = new char[strlen (text) + 1];
        if (ne) {
            strcpy (ne, text);
            if (textQ.Put (&ne) == vFalse) {
                delete [] ne;
            }
        }
    }
    fseek (qf, 0, SEEK_SET);
    ftruncate (fileno (qf), 0);
    while (textQ.Get (&ne) == vTrue) {
        dassert (ne != 0);
        fprintf (qf, ne);
        delete [] ne;
    }
    fclose (qf);
}

/*===========================================================================*\
\*===========================================================================*/

static void OneSendSession (CFaxSend *pFS, tUInt Resolution, tUInt JobID, tUInt *pCommID, CDynamicString *pTelNumber,
                            CDynamicString *pPollStr, CMultiString *pFileList) {
    dhead ("OneSendSession", DCON_CFaxSend);
    dassert (pFS != 0);
    dassert (pTelNumber != 0);
    dassert (pPollStr != 0);
    dassert (pFileList != 0);
    if ((pFileList->IsEmpty() == vTrue) && (pPollStr->IsEmpty() == vTrue)) {
        pFS->WriteLog (LOG_ERR, "No file to send specified! Forgot the format option (-f)?\n\n");
        pFS->exitState = 13;
        return;
    }
    if (pTelNumber->IsEmpty() == vFalse) {
        pTelNumber->SetLen (s_strkeep (pTelNumber->GetPointer(), (tChar *)"+0123456789*#RrMm"));
    }
    if (pTelNumber->IsEmpty() == vTrue) {
        pFS->WriteLog (LOG_ERR, "No telephone number specified!\n\n");
        pFS->exitState = 13;
        return;
    }
    if (pFS->Send (pTelNumber,
                   pFileList,
                   Resolution,
                   JobID,
                   pCommID,
                   pPollStr) == i_Okay) {
        while (sem_wait (&syncsema)) {
            if (errno != EINTR) {
                break;
            }
        }
    }
    dprint ("fs.exitState=%x ", pFS->exitState);
    pFS->CloseJobFile();
}


/*===========================================================================*\
\*===========================================================================*/

static void OneQFileSend (CFaxSend *pFS, char *deviceName, CDynamicString *pQFile) {
    dhead ("OneQFileSend", DCON_CFaxSend);
    dassert (pFS != 0);
    dassert (deviceName != 0);
    dassert (pQFile != 0);
    dparams ("%s", pQFile->GetPointer());
    CMultiString      FaxFileList;
    CDynamicString    TelNumber;
    CDynamicString    JobTag;
    CDynamicString    MailAddr;
    CDynamicString    PollStr;
    CDynamicString    TagLineFormat;
    tUInt             Resolution    = 0;
    tUInt             JobID         = 0;
    tUInt             commID        = 0;
    tUInt             HylaExitState = 1;
    char             *statustext;
    CConfigFileParser FileParser;
    switch (FileParser.ParseFile (pQFile->GetPointer())) {
    case iErr_File_OpenFailed:
        pFS->WriteLog (LOG_ERR, "Can't open qfile \"%s\".\n", (char *)pQFile->GetPointer());
        pFS->exitState = 3;
        return;
    case i_Okay:
        break;
    case iErr_OutOfMemory:
        dassert (0);
        pFS->WriteLog (LOG_ERR, "Running out of memory during reading of qfile!\n");
        pFS->exitState = 3;
        return;
    default:
        dassert (0);
        pFS->WriteLog (LOG_ERR, "Can't read qfile \"%s\"!\n", (char *)pQFile->GetPointer());
        pFS->exitState = 3;
        return;
    }
    CConfigParserSection *mainSect = FileParser.GetMainSection();
    if (!mainSect) {
        pFS->WriteLog (LOG_ERR, "Can't find needed information in qfile \"%s\".\n", (char *)pQFile->GetPointer());
        pFS->exitState = 3;
        return;
    }
    ConfigAssignment_Item Assignments[] = {
        { "number",     ConfigAssignment_String,     &TelNumber },
        { "fax",        ConfigAssignment_StringList, &FaxFileList },
        { "resolution", ConfigAssignment_UInt,       &Resolution },
        { "jobid",      ConfigAssignment_UInt,       &JobID },
        { "jobtag",     ConfigAssignment_String,     &JobTag },
        { "mailaddr",   ConfigAssignment_String,     &MailAddr },
        { "poll",       ConfigAssignment_String,     &PollStr },
        { "tagline",    ConfigAssignment_String,     &TagLineFormat },
        { 0,            ConfigAssignment_UInt,       0 }
    };
    ConfigValuesAssignment (&mainSect->ValueList, &Assignments[0]);

    // for polling we need the mailaddr
    if (PollStr.IsEmpty() == vFalse) {
        PollStr.Set (&MailAddr);
        if (PollStr.IsEmpty() == vTrue) {
            PollStr.Set (" ", 1);
        }
    }

    // remove numbers from fax list
    COneMultiString *pLauf = FaxFileList.GetFirst();
    while (pLauf != 0) {
        dassert (pLauf->GetLen() > 0);
        switch (pLauf->GetChar (0)) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            {
                COneMultiString *toDel = pLauf;
                pLauf = pLauf->GetNext();
                delete toDel;
            }
            break;
        default:
            pLauf = pLauf->GetNext();
            break;
        }
    }
    // Write "Sending job" into queue file
    pFS->exitState = 255;
    WriteQFile (pQFile->GetPointer(), commID, 0, pFS, &statustext);
    pFS->exitState = 0;
    pFS->SetTagLine (&TagLineFormat);

    OneSendSession (pFS, Resolution, JobID, &commID, &TelNumber, &PollStr, &FaxFileList);
    dprint ("fs.exitState=%x ", pFS->exitState);

    switch (pFS->exitState) {
    case 2:
    case 4:
    case 5:
    case 6:
    case 7:
    case 10:
    case 12:
        // retry
        HylaExitState = 0;
        break;
    case 15:
    default:
        // error
        HylaExitState = 1;
        break;
    case 0:
        // okay
        HylaExitState = 2;
        break;
    case 1:
    case 3:
    case 11:
        // reformat
        HylaExitState = 3;
        break;
    }
    WriteQFile (pQFile->GetPointer(), commID, HylaExitState, pFS, &statustext);
    pFS->exitState = HylaExitState;
    pFS->WriteXferLog (PollStr.IsEmpty() ? "SEND" : "POLL", commID, JobTag.GetPointer(), &JobID,
                       deviceName, &MailAddr, &TelNumber, statustext);
    dprint ("fs.exitState=%x ", pFS->exitState);
    pFS->WriteLog (LOG_INFO, "SESSION END\n");
    pFS->CloseJobFile();
}


/*===========================================================================*\
\*===========================================================================*/

static void usage (char *prgName) {
    char *name = strrchr (prgName, '/');
    if (!name) {
        name = prgName;
    } else {
        name++;
    }
    printf ("usage: %s [-f format] [-C cfile] [-c param] [-m dev]\n\t\t "
            "[-d number] [-P mailadr] [-v] [-V] [file(s)]\n", name);
    printf ("\t-f       Specify the output format. Allowed types are:\n");
    printf ("\t         HYLAFAX (default), TIFF, SFF, G3\n");
    printf ("\t-C       Name and path of the config file\n");
    printf ("\t-c       One Entry from the config-file of type: {value}:{data}\n");
    printf ("\t-m       Virtual device name only needed for Hylafax. Default: %s\n", DEFAULT_DEVICE_NAME);
    printf ("\t-P       Fax Polling. At the end pollrcvd with \"mailadr\" is called\n");
    printf ("\t-L       Activate Logging (Filename have to be specified in the config file).\n");
    printf ("\t-v       Start in verbose mode\n");
    printf ("\t-V       Show version information\n");
    printf ("\tfile(s): Depending on the format(-f) switch.\n");
    printf ("\t         HYLAFAX: one or more qfile(s)\n");
    printf ("\t         SFF:     one         sff file\n");
    printf ("\t         TIFF:    one or more tiff file(s)\n");
}

/*===========================================================================*\
\*===========================================================================*/

int main (int argc, char **argv) {
    DebugFileOpen();
    dhead ("CFaxSend-main", DCON_CFaxSend);
    CFaxSend fs (FinishFunc);
    if (argc < 2) {
        usage (argv[0]);
        return 13;
    }
    InitPrctlDumpable();

    CDynamicString TelNumber;
    CMultiString   FaxFileList;
    CDynamicString PollStr;
    FaxFormatType  format     = FaxFormat_Hylafax;
    tUInt          Resolution = 0;
    char          *deviceName = DEFAULT_DEVICE_NAME;
    char          *configName = 0;
    int            ch;

    // find device name
    while ((ch = getopt (argc, argv, "Hh?VvLlpnf:C:c:d:m:P:")) != -1) {
        switch (ch) {
        default:
            usage (argv[0]);
            return 13;
        case 'V':
            printf ("c2faxsend Version: %s (c) AVM GmbH\n", VERSION);
            return 13;
        case 'L':
        case 'l':
        case 'p':
        case 'v':
        case 'c':
            break;
        case 'n':
            Resolution = 98;
            break;
        case 'C':
            if (optarg && *optarg) {
                configName = optarg;
            }
            break;
        case 'd':
            if (optarg && *optarg) {
                 TelNumber.Set (optarg);
            }
            break;
        case 'm':
            if (optarg && *optarg) {
                deviceName = optarg;
            }
            break;
        case 'P':
            if (optarg && *optarg) {
                dprint ("PollStr=%s ", optarg);
                PollStr.Set (optarg);
            }
            break;
        case 'f':
            if (!optarg || !*optarg) {
                break;
            }
            if (strcasecmp (optarg, FORMAT_STRING_HYLAFAX) == 0) {
                format = FaxFormat_Hylafax;
            } else if (strcasecmp (optarg, FORMAT_STRING_SFF) == 0) {
                format = FaxFormat_SFF;
            } else if (strcasecmp (optarg, FORMAT_STRING_TIFF) == 0) {
                format = FaxFormat_TIFF;
            } else if (strcasecmp (optarg, FORMAT_STRING_G3) == 0) {
                format = FaxFormat_G3;
            } else {
                printf ("Wrong format param %s!\n\n", optarg);
                return 13;
            }
            break;
        }
    }
    if (!configName) {
        configName = DEFAULT_CONFIG_FILE;
    }
    if ((optind >= argc) && (PollStr.IsEmpty() == vTrue)) {
        printf ("No fax file specified!\n");
        return 1;
    }
    if (fs.Config ((tChar *)configName, deviceName, format, argc, argv) == vFalse) {
        printf ("Initialisation error!\n");
        return 1;
    }
    sem_init (&syncsema, 0, 0);
    switch (format) {
    case FaxFormat_Hylafax:
        {
            COneMultiString *pNewStr;
            char *kommachar;
            char *tmpqfname;
            tUInt len;
            for (; optind < argc; optind++) {
                tmpqfname = argv[optind];
                while (tmpqfname) {
                    kommachar = strchr (tmpqfname, ',');
                    if (kommachar) {
                        len = kommachar - tmpqfname;
                        kommachar++;
                    } else {
                        len = s_strlen (tmpqfname);
                    }
                    if (len) {
                        pNewStr = new COneMultiString;
                        if (pNewStr) {
                            if (pNewStr->Set (tmpqfname, len) == vTrue) {
                                dprint ("Fax-File=%s ", pNewStr->GetPointer());
                                FaxFileList.AddLast (pNewStr);
                            } else {
                                delete pNewStr;
                            }
                        }
                    }
                    tmpqfname = kommachar;
                }
            }
            pNewStr = FaxFileList.GetFirst();
            while (pNewStr && ((fs.exitState == 0) || (fs.exitState == 2))) {
                OneQFileSend (&fs, deviceName, pNewStr);
                pNewStr = pNewStr->GetNext();
            }
        }
        break;
    case FaxFormat_SFF:
        {
            COneMultiString *pNewStr = new COneMultiString;
            if (pNewStr) {
                dprint ("Fax-File=%s ", argv[optind]);
                if (pNewStr->Set (argv[optind]) == vTrue) {
                    FaxFileList.AddLast (pNewStr);
                } else {
                    delete pNewStr;
                }
            }
            OneSendSession (&fs, Resolution, 0, 0, &TelNumber, &PollStr, &FaxFileList);
        }
        break;
    case FaxFormat_TIFF:
    case FaxFormat_G3:
        {
            COneMultiString *pNewStr;
            for (; optind < argc; optind++) {
                pNewStr = new COneMultiString;
                if (pNewStr) {
                    dprint ("Fax-File=%s ", argv[optind]);
                    if (pNewStr->Set (argv[optind]) == vTrue) {
                        FaxFileList.AddLast (pNewStr);
                    } else {
                        delete pNewStr;
                    }
                }
            }
            OneSendSession (&fs, Resolution, 0, 0, &TelNumber, &PollStr, &FaxFileList);
        }
        break;
    }
    sem_destroy (&syncsema);
    DebugFileClose();
    if (fs.exitState == 13) {
        usage (argv[0]);
    }
    return fs.exitState;
}

/*===========================================================================*\
\*===========================================================================*/

