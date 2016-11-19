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

#include "dbgSTD.h"
#include "dbgCAPI.h"
#include "dbgLINUX.h"
#include "ExtFuncs.h"
#include <stdio.h>

/*===========================================================================*\
\*===========================================================================*/

tUInt DCON_CFaxSend                 = Dbg_Level_Internal;
tUInt DCON_CFaxReceive              = Dbg_Level_Internal;
tUInt DCON_CFaxReceiveDevice        = Dbg_Level_Internal;
tUInt DCON_CFaxReceiveMain          = Dbg_Level_Internal;
tUInt DCON_SffFilter                = Dbg_Level_Infos;
tUInt DCON_TifFilter                = Dbg_Level_Infos;
tUInt DCON_CBasicFaxConversion      = Dbg_Level_Internal;


/*===========================================================================*\
    CAPI
\*===========================================================================*/

tUInt DCON_CAPITrace                = Dbg_Level_Logs;
tUInt DCON_CCAPI20_MsgBase          = Dbg_Level_Infos;
tUInt DCON_CCAPI20_Channel          = Dbg_Level_Infos;
tUInt DCON_CCAPI20_Facility         = Dbg_Level_Infos;
tUInt DCON_CTransferChannel         = Dbg_Level_Internal;
tUInt DCON_CCntrlMSNList            = Dbg_Level_Internal;
tUInt DCON_PLCIList                 = Dbg_Level_Infos;
tUInt DCON_CCStruct                 = Dbg_Level_Infos;
tUInt DCON_CCAPI20_Statistic        = Dbg_Level_Infos;


/*===========================================================================*\
    others
\*===========================================================================*/

tUInt DCON_StringFuncs              = Dbg_Level_Infos;
tUInt DCON_CDynamicString           = Dbg_Level_Infos;
tUInt DCON_CStandardByteQueue       = Dbg_Level_Infos;
tUInt DCON_CStandardQueue           = Dbg_Level_Infos;
tUInt DCON_CStandardArray           = Dbg_Level_Infos;
tUInt DCON_CDynamicStore            = Dbg_Level_Infos;
tUInt DCON_CPointerList             = Dbg_Level_Infos;
tUInt DCON_CBasicTimerSupport       = Dbg_Level_Infos;
tUInt DCON_ConfigParser             = Dbg_Level_Internal;
tUInt DCON_ConfigParserAssi         = Dbg_Level_Internal;

tUInt DCON_CapiBase                 = Dbg_Level_Infos;
tUInt DCON_Protect                  = Dbg_Level_Infos;
tUInt DCON_SecTimer                 = Dbg_Level_Infos;
tUInt DCON_Eventing                 = Dbg_Level_Infos;

/*===========================================================================*\
\*===========================================================================*/

const char *DbgIPrintLinePrefix = "";
unsigned DbgIPrintLinePrefixLen = 0;

#if defined (DCON_FILE_NAME) && (USE_DCON_INFO_CLASS > 0)

#include "DbgFile.h"

void DbgIPrintStr (const char *output) {
    DebugFilePrint (output);
}

#else

#include <stdio.h>

void DbgIPrintStr (const char *output) {
    puts (output);
}

#endif

void DbgIBreakPoint (void) {
}

/*===========================================================================*\
\*===========================================================================*/
