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

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <errno.h>
#include <signal.h>
#include "recvdev.h"
#include "FilePars.h"
#include "ConfAssi.h"
#include "PrctlDmp.h"
#include "DbgFile.h"
#include "osImport.h"

/*===========================================================================*\
\*===========================================================================*/

static CFaxReceiveMain *g_CFaxRecvMain = 0;

static void sighandler_term (int signum) {
    dhead ("sighandler_term", DCON_CFaxReceiveMain);
    dassert (g_CFaxRecvMain != 0);
    if (g_CFaxRecvMain) {
        g_CFaxRecvMain->StopReceive();
    }
    signal (signum, sighandler_term);
}

/*===========================================================================*\
\*===========================================================================*/

CFaxReceiveMain::CFaxReceiveMain (void)
  : format (FaxFormat_Hylafax),
    LogFileName (DEFAULT_C4H_LOGFILENAME),
    LogLevel (0),
    hLogFile (0),
    enableEcho (vFalse),
    configName (0) {

    dhead ("CFaxReceiveMain-Constructor", DCON_CFaxReceiveMain);
    SpoolDir.Set (DEFAULT_SPOOL_DIRECTORY);
}

/*===========================================================================*\
\*===========================================================================*/

CFaxReceiveMain::~CFaxReceiveMain (void) {
    dhead ("CFaxReceiveMain-Destructor", DCON_CFaxReceiveMain);
    if (hLogFile != NULL) {
        fclose (hLogFile);
        hLogFile = NULL;
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveMain::WriteLog (tSInt priority, const char *text, ...) {
    dhead ("CFaxReceiveMain::WriteLog", DCON_CFaxReceiveMain);
    va_list params;
    va_start (params, text);
    vWriteLog (priority, text, params);
    va_end (params);
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveMain::vWriteLog (tSInt priority, const char *text, va_list params) {
    dhead ("CFaxReceiveMain::vWriteLog", DCON_CFaxReceiveMain);
    const char   *statusText = NULL;
    char    timebuf[MAX_STRING_SIZE];
    va_list lparams;
    timeval tv;

    gettimeofday (&tv, 0);
    strftime (timebuf, MAX_STRING_SIZE, "%h %d %T", localtime((time_t*) &tv.tv_sec));
    sprintf (timebuf + strlen(timebuf), ".%02lu: [%5d]: ", tv.tv_usec / 10000, getpid());

    if (enableEcho) {
        va_copy(lparams, params);
        vprintf (text, lparams);
        va_end(lparams);
    }

    // the logging is filtered depending on the trace level and the log priority
    // info, warning and error loggings get a prefix to inform aboout the kind of the information
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
        fprintf (hLogFile, "%s", timebuf);
        fprintf (hLogFile, "%s - ", RecvProgName.GetPointer());
        if (statusText) {
            fprintf (hLogFile, "%s", statusText);
        }
        va_copy(lparams, params);
        vfprintf (hLogFile, text, lparams);
        va_end(lparams);
        fflush (hLogFile);
    }

    // errors should be added to the syslog, too
    if (priority == LOG_ERR) {
        char errortext[MAX_STRING_SIZE];
        vsnprintf (errortext, MAX_STRING_SIZE, text, params);
        openlog (DEFAULT_RECVPROG_NAME, LOG_PERROR, 0);
        syslog (priority, "%s", errortext);
        closelog();
    }
}


/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveMain::GenerateHylafaxConfigFile (CDynamicString *pName, CDynamicString *pFaxNumber,
                                                 CDynamicString *pIdentifier) {
    dhead ("CFaxReceiveMain::GenerateHylafaxConfigFile", DCON_CFaxReceiveMain);
    dassert (pName != 0);
    dassert (pName->IsEmpty() == vFalse);

    CDynamicString LinkName;
    LinkName.Print ("%S/" HYLAFAX_CONFIG_PREFIX "%S", &SpoolDir, pName);
    if (LinkName.ErrorDetected()) {
        return;
    }
    // check for symlink
    struct stat filestat;
    if (lstat ((char *)LinkName.GetPointer(), &filestat) == 0) {
        if (S_ISLNK (filestat.st_mode)) {
            // delete symlink
            if (unlink ((char *)LinkName.GetPointer()) != 0) {
                dwarning (0);
                WriteLog (LOG_ERR, "Can't delete symbolic link \"%s\" to regenerate it as regular file!",
                          LinkName.GetPointer());
                return;
            }
        } else {
            if (!S_ISREG (filestat.st_mode)) {
                dwarning (0);
                WriteLog (LOG_ERR, "Config file \"%s\" is not a regular file! Please change it or delete the file!",
                          LinkName.GetPointer());
            }
            return;
        }
    }

    if (access ((char *)LinkName.GetPointer(), F_OK) != 0) {
        //symlink ((char *)configName, (char *)LinkName.GetPointer());
        FILE *conffile = fopen ((char *)LinkName.GetPointer(), "w");
        if (conffile) {
            fprintf (conffile,
                     "HylafaxDeviceName:          %s\nFAXNumber:                  %s\n"
                     "LocalIdentifier:            \"%s\"\n",
                     pName->GetPointer(), pFaxNumber->GetPointer(), pIdentifier->GetPointer());
            fclose (conffile);
            return;
        }
    }
    WriteLog (LOG_ERR, "Can't create special HylaFAX config file \"%s\"! "
                       "Probably a missing directory or user rights?\n", LinkName.GetPointer());
}


/*===========================================================================*\
\*===========================================================================*/

tBool CFaxReceiveMain::Config (char *CommandLineSpoolDir) {
    dhead ("CFaxReceiveMain::Config", DCON_CFaxReceiveMain);
    dassert (configName != 0);
    CConfigFileParser FileParser;
    printf ("The config file is %s.\n", configName);
    switch (FileParser.ParseFile (configName)) {
    case iErr_File_OpenFailed:
        printf ("Can't open config file \"%s\". Use default values.\n", configName);
        break;
    case i_Okay:
        break;
    case iErr_OutOfMemory:
        dassert (0);
        printf ("Running out of memory during reading of config file!\n");
        return vFalse;
    default:
        dassert (0);
        printf ("Can't read config file \"%s\"!\n", configName);
        return vFalse;
    }
    CConfigParserSection *mainSect = FileParser.GetMainSection();
    if (!mainSect) {
        printf ("No information found in the config file.\n");
        CFaxReceiveDevice *pNewDevice = new CFaxReceiveDevice (this, format);
        if (!pNewDevice) {
            printf ("Running out of Memory!\n");
            return vFalse;
        }
        AddLast (pNewDevice);
    }

    tUInt tmpLogLevel = 0;
    ConfigAssignment_Item Assignments[] = {
        { "SpoolDir",        ConfigAssignment_String, &SpoolDir },
        { "FaxRcvdCmd",      ConfigAssignment_String, &FaxRcvdCmd },
        { "FaxReceiveUser",  ConfigAssignment_String, &FaxUser },
        { "FaxReceiveGroup", ConfigAssignment_String, &FaxGroup },
        { "LogFile",         ConfigAssignment_String, &LogFileName },
        { "LogFileMode",     ConfigAssignment_UInt,   &LogFileMode },
        { "LogTraceLevel",   ConfigAssignment_UInt,   &tmpLogLevel },
        { 0,                 ConfigAssignment_UInt,   0 }
    };
    ConfigValuesAssignment (&mainSect->ValueList, &Assignments[0]);
    if (CommandLineSpoolDir) {
        SpoolDir.Set (CommandLineSpoolDir);
    }
    if (tmpLogLevel > LogLevel) {
        LogLevel = tmpLogLevel;
    }
    if ((LogFileName.IsEmpty() == vFalse) && (LogLevel != 0)) {
        hLogFile = fopen ((char *)LogFileName.GetPointer(), "a");
        if (hLogFile) {
            fchmod (fileno(hLogFile), LogFileMode);
        }
    }

    WriteLog (LOG_INFO, "\nC4H-Version " VERSION
    #ifdef C_PLATFORM_64
                " (64Bit).\n\n");
    #else
                " (32Bit).\n\n");
    #endif
    WriteLog (LOG_INFO, "The SpoolDirectory is %s.\n", SpoolDir.GetPointer());
    umask (0);
    setupPermissions();
    InitPrctlDumpable();

    for (CConfigParserSection *laufSec = (CConfigParserSection *)mainSect->SectionList.GetFirst();
         laufSec; laufSec = (CConfigParserSection *)laufSec->GetNext()) {

        CFaxReceiveDevice *pNewDevice = new CFaxReceiveDevice (this, format);
        if (!pNewDevice) {
            printf ("Running out of Memory!\n");
            return vFalse;
        }
        pNewDevice->Config (laufSec);

        // if DeviceName still exist delete the new one
        CFaxReceiveDevice *oldOnes = (CFaxReceiveDevice *)GetFirst();
        while (oldOnes) {
            if (strcmp ((char *)oldOnes->DeviceName.GetPointer(), (char *)pNewDevice->DeviceName.GetPointer()) == 0) {
                delete pNewDevice;
                pNewDevice = 0;
                break;
            }
            oldOnes = (CFaxReceiveDevice *)oldOnes->GetNext();
        }
        if (pNewDevice) {
            AddLast (pNewDevice);
        }
    }
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

tBool CFaxReceiveMain::StartReceive (void) {
    dhead ("CFaxReceiveMain::StartReceive", DCON_CFaxReceiveMain);
    if (SpoolDir.IsEmpty() == vTrue) {
        dassert (0);
        WriteLog (LOG_ERR, "An empty Spool Directory was specified but is not allowed!");
        return vFalse;
    }
    if (chdir ((char *)SpoolDir.GetPointer()) == -1) {
        dassert (0);
        WriteLog (LOG_ERR, "Can't change to Spool Directory \"%s\"! "
                  "The directory is missing or inaccessible.\n", SpoolDir.GetPointer());
        return vFalse;
    }
    const char *ModeText = "Hylafax";
    switch (format) {
    case FaxFormat_TIFF:
        ModeText = "TIFF";
        break;
    case FaxFormat_G3:
        ModeText = "G3";
        break;
    case FaxFormat_SFF:
        ModeText = "SFF";
        break;
    default:
        // nothing to do
        break;
    }
    CFaxReceiveDevice *pCurDev = (CFaxReceiveDevice *)GetFirst();
    while (pCurDev != 0) {
        CFaxReceiveDevice *ptmpDev = pCurDev;
        pCurDev = (CFaxReceiveDevice *)pCurDev->GetNext();
        if (ptmpDev->StartReceive() == vFalse) {
            delete ptmpDev;
        }
    }
    if (IsEmpty() == vFalse) {
        WriteLog (LOG_INFO, "\nStarted in %s mode and waiting on incoming calls.\n", ModeText);
        return vTrue;
    }
    WriteLog (LOG_INFO, "\nNo device is waiting for faxes so the program can terminate now.\n");
    return vFalse;
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveMain::StopReceive (void) {
    dhead ("CFaxReceiveMain::StopReceive", DCON_CFaxReceiveMain);
    CFaxReceiveDevice *lauf = (CFaxReceiveDevice *)GetFirst();
    while (lauf != 0) {
        CFaxReceiveDevice *accessDev = lauf;
        lauf = (CFaxReceiveDevice *)lauf->GetNext();
        accessDev->StopReceive();
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveMain::WaitOnFIFOs (void) {
    dhead ("CFaxReceiveMain::WaitOnFIFO", DCON_CFaxReceiveMain);
    tSInt          hF;
    tSInt          hFmax;
    fd_set         readfds;
    fd_set         exceptfds;
    struct timeval twait;

    CFaxReceiveDevice *lauf;
    while (IsEmpty() == vFalse) {
        for (lauf = (CFaxReceiveDevice *)GetFirst(); (lauf != 0); lauf = (CFaxReceiveDevice *)lauf->GetNext()) {
            lauf->RecreateFIFO();
        }
        do {
            hFmax = 0;
            FD_ZERO (&readfds);
            FD_ZERO (&exceptfds);
            for (lauf = (CFaxReceiveDevice *)GetFirst(); (lauf != 0); lauf = (CFaxReceiveDevice *)lauf->GetNext()) {
                hF = lauf->GetFIFOHandle();
                dprint ("hF=%x ", hF);
                if (hF >= 0) {
                    FD_SET (hF, &readfds);
                    FD_SET (hF, &exceptfds);
                    if (hF > hFmax) {
                        hFmax = hF;
                    }
                }
            }
            twait.tv_sec  = WAIT_TIME_IN_SEC;
            twait.tv_usec = 0;
            dprint ("hFmax=%x ", hFmax);
            if (select (hFmax + 1, &readfds, 0, &exceptfds, &twait) == -1) {
                //dprintErr ("select returns an Error=%s(%x) ", strerror (errno), errno);
                sleep (30);
                continue;
                //break;
            }
            dprint ("select\n");
            lauf = (CFaxReceiveDevice *)GetFirst();
            while (lauf != 0) {
                hF = lauf->GetFIFOHandle();
                dwarning (hF >= 0);
                if ((hF >= 0) && (FD_ISSET (hF, &readfds))) {
                    CFaxReceiveDevice *accessDev = lauf;
                    lauf = (CFaxReceiveDevice *)lauf->GetNext();
                    accessDev->FIFOchanged();
                } else {
                    lauf = (CFaxReceiveDevice *)lauf->GetNext();
                }
            }
        } while (IsEmpty() == vFalse);
    }
}


/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveMain::setupPermissions (void) {
    dhead ("CFaxReceiveMain::setupPermissions", DCON_CFaxReceiveMain);
    if (FaxUser.IsEmpty() || (strcasecmp ((char *)FaxUser.GetPointer(), "root") == 0)) {
        // nothing to do!
        return;
    }
    dassert (FaxUser.GetPointer() != 0);
    if (getuid() != 0) {
        WriteLog (LOG_ERR, "The fax server must be started with real uid root.\n");
        return;
    }
    uid_t euid = geteuid();
    passwd *pwd = getpwnam ((char *)FaxUser.GetPointer());
    if (!pwd) {
        WriteLog (LOG_ERR, "No fax user %s defined on your system!\n", FaxUser.GetPointer());
    } else {
        tUInt errflags = 0;
        if (!FaxGroup.IsEmpty()) {
            group *grp = getgrnam ((char *)FaxGroup.GetPointer());
            if (!grp) {
                errflags |= 0x1;
                WriteLog (LOG_ERR, "No fax group %s defined on your system!\n", FaxGroup.GetPointer());
            } else {
                pwd->pw_gid = grp->gr_gid;
            }
            endgrent();
        } else {
            errflags |= 0x1;
        }
        if (euid == 0) {
            if (initgroups (pwd->pw_name, pwd->pw_gid) < 0) {
                WriteLog (LOG_ERR, "Can not setup permissions (supplementary group list)\n");
            }
            if (setegid (pwd->pw_gid) < 0) {
                errflags |= 0x1;
                WriteLog (LOG_ERR, "Can not setup permissions (gid)\n");
            }
            if (seteuid (pwd->pw_uid) < 0) {
                errflags |= 0x2;
                WriteLog (LOG_ERR, "Can not setup permissions (uid)\n");
            }
        } else {
            uid_t faxuid = pwd->pw_uid;
            uid_t faxgid = pwd->pw_gid;
            setpwent();
            pwd = getpwuid(euid);
            if (!pwd) {
                errflags |= 0x3;
                WriteLog (LOG_ERR, "Can not figure out the identity of uid %u\n", euid);
            }
            if (pwd->pw_uid != faxuid) {
                errflags |= 0x2;
                WriteLog (LOG_ERR, "Configuration error: The uid couldn't changed to fax user %s.\n",
                          FaxUser.GetPointer());
            }
            if (setegid (faxgid) < 0) {
                errflags |= 0x1;
                WriteLog (LOG_ERR, "Can not setup permissions (gid)\n");
            }
        }
        switch (errflags) {
        case 0:
            WriteLog (LOG_INFO, "Changed to effective user \"%s\" and group \"%s\" successfully.\n", FaxUser.GetPointer(), FaxGroup.GetPointer());
            break;
        case 1:
            WriteLog (LOG_INFO, "Changed to effective user \"%s\" successfully.\n", FaxUser.GetPointer());
            break;
        case 2:
            WriteLog (LOG_INFO, "Changed to effective group \"%s\" successfully.\n", FaxGroup.GetPointer());
            break;
        default:
            break;
        }
    }
    endpwent();
}

/*===========================================================================*\
\*===========================================================================*/

void CFaxReceiveMain::usage (void) {
    printf ("usage: %s [-f format] [-q dir] [-C cfile] [-v] [-V] [dev]\n", RecvProgName.GetPointer());
    printf ("\t-f   Specify the output format. Allowed types are:\n");
    printf ("\t     HYLAFAX (default), TIFF, SFF.\n");
    printf ("\t-q   Directory where received faxes are to be stored.\n");
    printf ("\t     Default: %s\n", DEFAULT_SPOOL_DIRECTORY);
    printf ("\t-C   Name and path of the config file\n");
    printf ("\t-L   Activate Logging (Filename have to be specified in the config file).\n");
    printf ("\t-v   Start in verbose mode\n");
    printf ("\t-V   Show version information\n");
    printf ("\tdev: Virtual device name only needed for Hylafax.\n");
    printf ("\t     Default: %s\n", DEFAULT_DEVICE_NAME);
}

/*===========================================================================*\
\*===========================================================================*/

tSInt CFaxReceiveMain::main (tSInt argc, char **argv) {
    dhead ("CFaxReceive::main", DCON_CFaxReceiveMain);
    char deviceName[255]  = DEFAULT_DEVICE_NAME;
    char *cl_spooldir = 0;
    int   ch;

    // get program name
    char *name = strrchr (argv[0], '/');
    if (!name) {
        name = argv[0];
    } else {
        name++;
    }
    if (RecvProgName.Set (name) == vFalse) {
        printf ("\nError: Memory full!\n");
        return 1;
    }

    // handle command line parameters
    while ((ch = getopt (argc, argv, "Hh?VvLDpq:C:f:")) != -1) {
        switch (ch) {
        default:
            usage();
            return 1;
        case 'V':
            printf ("c2faxrecv Version: %s (c) AVM GmbH\n", VERSION);
            return 1;
        case 'D':
        case 'p':
            break;
        case 'v':
            enableEcho = vTrue;
            break;
        case 'L':
            LogLevel = LOG_TRACE_ALL;
            break;
        case 'q':
            if (optarg && *optarg) {
                cl_spooldir = optarg;
            }
            break;
        case 'C':
            if (optarg && *optarg) {
                configName = (tUChar *)optarg;
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
                usage();
                return 1;
            }
            break;
        }
    }
    dprint ("%x,%x-", optind, argc);
    if (optind < argc) {
        char * deviceName_in = strrchr (argv[optind], '/');
        if (deviceName_in) {
            deviceName_in++;
            strncpy(deviceName, deviceName_in, sizeof(deviceName));
        } else {
            //deviceName = argv[optind];
            strncpy(deviceName, argv[optind], sizeof(deviceName));
        }
    }
    if (!configName) {
        configName = (tChar *)DEFAULT_CONFIG_FILE;
    }

    if (Config (cl_spooldir) == vFalse) {
        return 1;
    }
    if (StartReceive() == vFalse) {
        return 2;
    }
    g_CFaxRecvMain = this;
    signal (SIGRTMIN, SIG_IGN);
    signal (SIGINT,  sighandler_term);
    signal (SIGTERM, sighandler_term);
    if (format == FaxFormat_Hylafax) {
        WaitOnFIFOs();
    } else {
        while (IsEmpty() == vFalse) {
            sleep (10);
        }
    }
    return 0;
}

/*===========================================================================*\
\*===========================================================================*/

int main (int argc, char **argv) {
    DebugFileOpen();
    CFaxReceiveMain recvmain;
    int fret = recvmain.main (argc, argv);
    DebugFileClose();
    return fret;
}

/*===========================================================================*\
\*===========================================================================*/

