
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

#ifndef _C2MSGDEF_H_
#define _C2MSGDEF_H_

#include "capi2def.h"
#include "aString.h"

/*===========================================================================*\
\*===========================================================================*/

/*----- CAPI commands -----*/

#define CAPI_ALERT                          0x01
#define CAPI_CONNECT                        0x02
#define CAPI_CONNECT_ACTIVE                 0x03
#define CAPI_CONNECT_B3_ACTIVE              0x83
#define CAPI_CONNECT_B3                     0x82
#define CAPI_CONNECT_B3_T90_ACTIVE          0x88
#define CAPI_DATA_B3                        0x86
#define CAPI_DISCONNECT_B3                  0x84
#define CAPI_DISCONNECT                     0x04
#define CAPI_FACILITY                       0x80
#define CAPI_INFO                           0x08
#define CAPI_LISTEN                         0x05
#define CAPI_MANUFACTURER                   0xff
#define CAPI_RESET_B3                       0x87
#define CAPI_SELECT_B_PROTOCOL              0x41

/*----- CAPI subcommands -----*/

#define CAPI_REQ                            0x80
#define CAPI_CONF                           0x81
#define CAPI_IND                            0x82
#define CAPI_RESP                           0x83

/*----- CAPI full commands -----*/

#define CAPI_ALERT_REQ                      ((CAPI_REQ  << 8) | CAPI_ALERT)
#define CAPI_ALERT_CONF                     ((CAPI_REQ  << 8) | CAPI_ALERT)
#define CAPI_CONNECT_REQ                    ((CAPI_REQ  << 8) | CAPI_CONNECT)
#define CAPI_CONNECT_CONF                   ((CAPI_CONF << 8) | CAPI_CONNECT)
#define CAPI_CONNECT_IND                    ((CAPI_IND  << 8) | CAPI_CONNECT)
#define CAPI_CONNECT_RESP                   ((CAPI_RESP << 8) | CAPI_CONNECT)
#define CAPI_CONNECT_ACTIVE_IND             ((CAPI_IND  << 8) | CAPI_CONNECT_ACTIVE)
#define CAPI_CONNECT_ACTIVE_RESP            ((CAPI_RESP << 8) | CAPI_CONNECT_ACTIVE)
#define CAPI_CONNECT_B3_ACTIVE_IND          ((CAPI_IND  << 8) | CAPI_CONNECT_B3_ACTIVE)
#define CAPI_CONNECT_B3_ACTIVE_RESP         ((CAPI_RESP << 8) | CAPI_CONNECT_B3_ACTIVE)
#define CAPI_CONNECT_B3_REQ                 ((CAPI_REQ  << 8) | CAPI_CONNECT_B3)
#define CAPI_CONNECT_B3_CONF                ((CAPI_CONF << 8) | CAPI_CONNECT_B3)
#define CAPI_CONNECT_B3_IND                 ((CAPI_IND  << 8) | CAPI_CONNECT_B3)
#define CAPI_CONNECT_B3_RESP                ((CAPI_RESP << 8) | CAPI_CONNECT_B3)
#define CAPI_CONNECT_B3_T90_ACTIVE_IND      ((CAPI_IND  << 8) | CAPI_CONNECT_B3_T90_ACTIVE)
#define CAPI_CONNECT_B3_T90_ACTIVE_RESP     ((CAPI_RESP << 8) | CAPI_CONNECT_B3_T90_ACTIVE)
#define CAPI_DATA_B3_REQ                    ((CAPI_REQ  << 8) | CAPI_DATA_B3)
#define CAPI_DATA_B3_CONF                   ((CAPI_CONF << 8) | CAPI_DATA_B3)
#define CAPI_DATA_B3_IND                    ((CAPI_IND  << 8) | CAPI_DATA_B3)
#define CAPI_DATA_B3_RESP                   ((CAPI_RESP << 8) | CAPI_DATA_B3)
#define CAPI_DISCONNECT_B3_REQ              ((CAPI_REQ  << 8) | CAPI_DISCONNECT_B3)
#define CAPI_DISCONNECT_B3_CONF             ((CAPI_CONF << 8) | CAPI_DISCONNECT_B3)
#define CAPI_DISCONNECT_B3_IND              ((CAPI_IND  << 8) | CAPI_DISCONNECT_B3)
#define CAPI_DISCONNECT_B3_RESP             ((CAPI_RESP << 8) | CAPI_DISCONNECT_B3)
#define CAPI_DISCONNECT_REQ                 ((CAPI_REQ  << 8) | CAPI_DISCONNECT)
#define CAPI_DISCONNECT_CONF                ((CAPI_CONF << 8) | CAPI_DISCONNECT)
#define CAPI_DISCONNECT_IND                 ((CAPI_IND  << 8) | CAPI_DISCONNECT)
#define CAPI_DISCONNECT_RESP                ((CAPI_RESP << 8) | CAPI_DISCONNECT)
#define CAPI_FACILITY_REQ                   ((CAPI_REQ  << 8) | CAPI_CAPI_FACILITY)
#define CAPI_FACILITY_CONF                  ((CAPI_CONF << 8) | CAPI_CAPI_FACILITY)
#define CAPI_FACILITY_IND                   ((CAPI_IND  << 8) | CAPI_CAPI_FACILITY)
#define CAPI_FACILITY_RESP                  ((CAPI_RESP << 8) | CAPI_CAPI_FACILITY)
#define CAPI_INFO_REQ                       ((CAPI_REQ  << 8) | CAPI_INFO)
#define CAPI_INFO_CONF                      ((CAPI_CONF << 8) | CAPI_INFO)
#define CAPI_INFO_IND                       ((CAPI_IND  << 8) | CAPI_INFO)
#define CAPI_INFO_RESP                      ((CAPI_RESP << 8) | CAPI_INFO)
#define CAPI_LISTEN_REQ                     ((CAPI_REQ  << 8) | CAPI_LISTEN)
#define CAPI_LISTEN_CONF                    ((CAPI_CONF << 8) | CAPI_LISTEN)
#define CAPI_MANUFACTURER_REQ               ((CAPI_REQ  << 8) | CAPI_MANUFACTURER)
#define CAPI_MANUFACTURER_CONF              ((CAPI_CONF << 8) | CAPI_MANUFACTURER)
#define CAPI_MANUFACTURER_IND               ((CAPI_IND  << 8) | CAPI_MANUFACTURER)
#define CAPI_MANUFACTURER_RESP              ((CAPI_RESP << 8) | CAPI_MANUFACTURER)
#define CAPI_RESET_B3_REQ                   ((CAPI_REQ  << 8) | CAPI_RESET_B3)
#define CAPI_RESET_B3_CONF                  ((CAPI_CONF << 8) | CAPI_RESET_B3)
#define CAPI_RESET_B3_IND                   ((CAPI_IND  << 8) | CAPI_RESET_B3)
#define CAPI_RESET_B3_RESP                  ((CAPI_RESP << 8) | CAPI_RESET_B3)
#define CAPI_SELECT_B_PROTOCOL_REQ          ((CAPI_REQ  << 8) | CAPI_SELECT_B_PROTOCOL)
#define CAPI_SELECT_B_PROTOCOL_CONF         ((CAPI_CONF << 8) | CAPI_SELECT_B_PROTOCOL)

/*===========================================================================*\
\*===========================================================================*/

#define CAPI_MSG_HEADER_SIZE                8
#define CAPI_MSG_HEADER_ADDR_SIZE           12      // Header + Controller/NCCI/PLCI
#define CAPI_MAX_MESSAGE_COUNTER            (CAPI_MAX_OUTSTANDING_FRAMES * CAPI_DEFAULT_S0_LINES_COUNT)

#ifdef C_PLATFORM_64
#define CAPI_MSG_DATA_POS                   (CAPI_MSG_HEADER_ADDR_SIZE + 10)
#else
#define CAPI_MSG_DATA_POS                   CAPI_MSG_HEADER_ADDR_SIZE
#endif

/*===========================================================================*\
\*===========================================================================*/

#define CapiMsg_GetMsgLen(name)         get16 (name)
#define CapiMsg_GetApplID(name)         get16 ((name) + 2)
#define CapiMsg_GetCmd(name)            get8  ((name) + 4)
#define CapiMsg_GetSubCmd(name)         get8  ((name) + 5)
#define CapiMsg_GetFullCmd(name)        get16 ((name) + 4)
#define CapiMsg_GetMsgNr(name)          get16 ((name) + 6)
#define CapiMsg_GetAddr(name)           get32 ((name) + 8)

#define CapiMsg_GetWord(name, pos)      get16 ((name) + (pos) + CAPI_MSG_HEADER_ADDR_SIZE)
#define CapiMsg_GetInfo(name, pos)      ((c_info)CapiMsg_GetWord (name, pos))
#define CapiMsg_GetData(name)           ((c_dataptr)getPtr ((name) + CAPI_MSG_DATA_POS))

/*===========================================================================*\
\*===========================================================================*/

#endif
