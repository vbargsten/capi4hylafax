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

#include "PrctlDmp.h"
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/prctl.h>
#include <linux/prctl.h>

#ifndef PR_GET_DUMPABLE
#define PR_GET_DUMPABLE 3
#endif

#ifndef PR_SET_DUMPABLE
#define PR_SET_DUMPABLE 4
#endif

/*===========================================================================*\
\*===========================================================================*/

void InitPrctlDumpable (void) {
    int ret;
    errno = 0;
    ret   = prctl(PR_GET_DUMPABLE);
    if (errno == EINVAL) {
        //printf ("Warning: PR_GET_DUMPABLE not supported by current kernel\n");
    } else if (ret == 0) {
        errno = 0;
        if (prctl (PR_SET_DUMPABLE, 1) < 0) {
            if (errno == EINVAL) {
                printf ("Warning: PR_SET_DUMPABLE not supported by current kernel\n");
            } else {
                fprintf (stderr, "Error: PR_SET_DUMPABLE failed - %s (%d)\n", strerror(errno), errno);
            }
        } else if (prctl(PR_GET_DUMPABLE) != 0) {
            //printf ("Info: program will dump core if needed");
        } else {
            fprintf (stderr, "Error: program not dumpable after PR_SET_DUMPABLE :-(\n");
        }
    }
}

/*===========================================================================*\
\*===========================================================================*/
