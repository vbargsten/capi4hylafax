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

#include "aTypes.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <limits.h>
#include <paths.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

tUInt FaxPropEncode (tUInt vRes, tUInt BaudRate) {
    //params.encode:          st|bf|ec|df|ln| wd| br|vr
    //values for capi:       000  0  0 00 10 000 ??? 0/1
    tUInt br = 0;
    if (BaudRate > 12000) {
        br = 5;
    } else if (BaudRate > 9600) {
        br = 4;
    } else if (BaudRate > 7200) {
        br = 3;
    } else if (BaudRate > 4800) {
        br = 2;
    } else if (BaudRate > 2400) {
        br = 1;
    }
    return 0x100 + (br << 1) + ((vRes > 98) ? 1 : 0);
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

static void detachIO(void) {
    //endpwent();               // XXX some systems hold descriptors
    int fd = open (_PATH_DEVNULL, O_RDWR);
    dup2 (fd, STDIN_FILENO);
    dup2 (fd, STDOUT_FILENO);
    dup2 (fd, STDERR_FILENO);
    for (fd = _POSIX_OPEN_MAX - 1; (fd >= 0); fd--) {
        if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO) {
            close(fd);
        }
    }
}

static void setRealIDs (void) {
    uid_t euid = geteuid();
    seteuid (0);
    const passwd *pwd = getpwuid (euid);
    if (pwd) {
        initgroups(pwd->pw_name, pwd->pw_gid);
    } else {
        setgroups (0, NULL);
    }
    setgid (getegid());
    setuid (euid);
}

tBool runCmd (char* cmd, bool changeIDs) {
    pid_t pid = fork();
    switch (pid) {
    case 0:
        if (changeIDs) {
            setRealIDs();
        }
        detachIO();
        execl ("/bin/sh", "sh", "-c", cmd, NULL);
        sleep (1);
        _exit (127);
    default: {
        int status = 0;
        waitpid (pid, &status, 0);
        if (status == 0) {
            return vTrue;
        }
        break; }
    case -1:
        break;
    }
    return vFalse;
}


/*---------------------------------------------------------------------------*\
    Convert NCPI for B3 protocol 5, Parameter Format to readable string
    (messages CONNECT_B3_REQ, CONNECT_B3_IND, CONNECT_B3_ACTIVE_IND,
    DISCONNECT_B3_IND)
\*---------------------------------------------------------------------------*/

const char *DataFormat2str (tUInt DataFormat) {
    const char *t;
    switch (DataFormat) {
    case 0:
        t = "SFF";
        break;
    case 1:
        t = "Plain fax format (modified Huffman coding";
        break;
    case 2:
        t = "PCX";
        break;
    case 3:
        t = "DCX";
        break;
    case 4:
        t = "TIFF";
        break;
    case 5:
        t = "ASCII";
        break;
    case 6:
        t = "Extended ANSI";
        break;
    case 7:
        t = "Binary file transfer";
        break;
    default:
        t = "unknown";
        break;
    }
    return t;
}

