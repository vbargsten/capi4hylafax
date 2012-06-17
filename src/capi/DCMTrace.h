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

#ifndef _DCMTRACE_H_
#define _DCMTRACE_H_

#include "capi2def.h"

/*===========================================================================*\
\*===========================================================================*/

#define CMTRACE_Unknown             0
#define CMTRACE_Register            1
#define CMTRACE_Release             2
#define CMTRACE_GetProfile          3
#define CMTRACE_PutMessage          4
#define CMTRACE_GetMessage          5

/*===========================================================================*\
\*===========================================================================*/

//void DbgCapiMsgTraceInit (tUChar *FileName);
//void DbgCapiMsgTraceDeInit (void);

#ifndef NDEBUG

void DbgCapiMsgTrace (c_message theMsg,
                      c_info error = ci_NoError,
                      unsigned kind = CMTRACE_Unknown);

#else

inline void DbgCapiMsgTrace (c_message /*theMsg*/,
                             c_info /*error*/,
                             unsigned /*kind*/) {}

#endif

/*===========================================================================*\
\*===========================================================================*/

#endif

