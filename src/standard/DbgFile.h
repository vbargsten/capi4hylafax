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

#ifndef _DBGFILE_H_
#define _DBGFILE_H_

#include "ExtFuncs.h"

/*===========================================================================*\
\*===========================================================================*/
#if defined (DEBUG_FILE_NAME) && (USE_DEBUG_INFO_CLASS > 0)

extern unsigned DebugFileOpen (void);
extern void DebugFileClose (void);

extern void DebugFilePrint (char *string);

#else

#define DebugFileOpen()
#define DebugFileClose()

#endif
/*===========================================================================*\
\*===========================================================================*/

#endif
