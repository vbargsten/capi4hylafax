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

#include "ExtFuncs.h"

#if defined (DEBUG_FILE_NAME) && (USE_DEBUG_INFO_CLASS > 0)

#include "DbgFile.h"
#include <stdio.h>

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

static FILE *hFile = 0;

/*===========================================================================*\
\*===========================================================================*/

unsigned DebugFileOpen (void) {
    DebugFileClose();
    hFile = fopen (DEBUG_FILE_NAME, "w");
    return (hFile != 0);
}

/*===========================================================================*\
\*===========================================================================*/

void DebugFileClose (void) {
    if (hFile) {
        fclose (hFile);
        hFile = 0;
    }
}

/*===========================================================================*\
\*===========================================================================*/

void DebugFilePrint (char *string) {
    if (hFile) {
        fprintf (hFile, string);
        fflush (hFile);
    } else {
        printf (string);
    }
}

/*===========================================================================*\
\*===========================================================================*/

#endif
