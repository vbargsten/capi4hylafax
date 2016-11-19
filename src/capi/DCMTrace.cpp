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

#include "DCMTrace.h"
#include "c2msgdef.h"
#include "dbgCAPI.h"

#ifndef NDEBUG
/*===========================================================================*\
\*===========================================================================*/

static tUChar *DbgStrCapiCommand (c_byte Command, c_byte SubCommand) {
    const char *fret;
    switch (SubCommand) {
    case CAPI_REQ:
        switch (Command) {
        case CAPI_ALERT:
            fret = "ALERT_REQ";
            break;
        case CAPI_CONNECT:
            fret = "CONNECT_REQ";
            break;
        case CAPI_CONNECT_ACTIVE:
            fret = "CONNECT_ACTIVE_REQ";
            break;
        case CAPI_CONNECT_B3_ACTIVE:
            fret = "CONNECT_B3_ACTIVE_REQ";
            break;
        case CAPI_CONNECT_B3:
            fret = "CONNECT_B3_REQ";
            break;
        case CAPI_CONNECT_B3_T90_ACTIVE:
            fret = "CONNECT_B3_T90_ACTIVE_REQ";
            break;
        case CAPI_DATA_B3:
            fret = "DATA_B3_REQ";
            break;
        case CAPI_DISCONNECT_B3:
            fret = "DISCONNECT_B3_REQ";
            break;
        case CAPI_DISCONNECT:
            fret = "DISCONNECT_REQ";
            break;
        case CAPI_FACILITY:
            fret = "FACILITY_REQ";
            break;
        case CAPI_INFO:
            fret = "INFO_REQ";
            break;
        case CAPI_LISTEN:
            fret = "LISTEN_REQ";
            break;
        case CAPI_MANUFACTURER:
            fret = "MANUFACTURER_REQ";
            break;
        case CAPI_RESET_B3:
            fret = "RESET_B3_REQ";
            break;
        case CAPI_SELECT_B_PROTOCOL:
            fret = "SELECT_B_PROTOCOL_REQ";
            break;
        default:
            fret = "Wrong REQ-Command!";
            break;
        }
        break;
    case CAPI_CONF:
        switch (Command) {
        case CAPI_ALERT:
            fret = "ALERT_CONF";
            break;
        case CAPI_CONNECT:
            fret = "CONNECT_CONF";
            break;
        case CAPI_CONNECT_ACTIVE:
            fret = "CONNECT_ACTIVE_CONF";
            break;
        case CAPI_CONNECT_B3_ACTIVE:
            fret = "CONNECT_B3_ACTIVE_CONF";
            break;
        case CAPI_CONNECT_B3:
            fret = "CONNECT_B3_CONF";
            break;
        case CAPI_CONNECT_B3_T90_ACTIVE:
            fret = "CONNECT_B3_T90_ACTIVE_CONF";
            break;
        case CAPI_DATA_B3:
            fret = "DATA_B3_CONF";
            break;
        case CAPI_DISCONNECT_B3:
            fret = "DISCONNECT_B3_CONF";
            break;
        case CAPI_DISCONNECT:
            fret = "DISCONNECT_CONF";
            break;
        case CAPI_FACILITY:
            fret = "FACILITY_CONF";
            break;
        case CAPI_INFO:
            fret = "INFO_CONF";
            break;
        case CAPI_LISTEN:
            fret = "LISTEN_CONF";
            break;
        case CAPI_MANUFACTURER:
            fret = "MANUFACTURER_CONF";
            break;
        case CAPI_RESET_B3:
            fret = "RESET_B3_CONF";
            break;
        case CAPI_SELECT_B_PROTOCOL:
            fret = "SELECT_B_PROTOCOL_CONF";
            break;
        default:
            fret = "Wrong CONF-Command!";
            break;
        }
        break;
    case CAPI_IND:
        switch (Command) {
        case CAPI_ALERT:
            fret = "ALERT_IND";
            break;
        case CAPI_CONNECT:
            fret = "CONNECT_IND";
            break;
        case CAPI_CONNECT_ACTIVE:
            fret = "CONNECT_ACTIVE_IND";
            break;
        case CAPI_CONNECT_B3_ACTIVE:
            fret = "CONNECT_B3_ACTIVE_IND";
            break;
        case CAPI_CONNECT_B3:
            fret = "CONNECT_B3_IND";
            break;
        case CAPI_CONNECT_B3_T90_ACTIVE:
            fret = "CONNECT_B3_T90_ACTIVE_IND";
            break;
        case CAPI_DATA_B3:
            fret = "DATA_B3_IND";
            break;
        case CAPI_DISCONNECT_B3:
            fret = "DISCONNECT_B3_IND";
            break;
        case CAPI_DISCONNECT:
            fret = "DISCONNECT_IND";
            break;
        case CAPI_FACILITY:
            fret = "FACILITY_IND";
            break;
        case CAPI_INFO:
            fret = "INFO_IND";
            break;
        case CAPI_LISTEN:
            fret = "LISTEN_IND";
            break;
        case CAPI_MANUFACTURER:
            fret = "MANUFACTURER_IND";
            break;
        case CAPI_RESET_B3:
            fret = "RESET_B3_IND";
            break;
        case CAPI_SELECT_B_PROTOCOL:
            fret = "SELECT_B_PROTOCOL_IND";
            break;
        default:
            fret = "Wrong IND-Command!";
            break;
        }
        break;
    case CAPI_RESP:
        switch (Command) {
        case CAPI_ALERT:
            fret = "ALERT_RESP";
            break;
        case CAPI_CONNECT:
            fret = "CONNECT_RESP";
            break;
        case CAPI_CONNECT_ACTIVE:
            fret = "CONNECT_ACTIVE_RESP";
            break;
        case CAPI_CONNECT_B3_ACTIVE:
            fret = "CONNECT_B3_ACTIVE_RESP";
            break;
        case CAPI_CONNECT_B3:
            fret = "CONNECT_B3_RESP";
            break;
        case CAPI_CONNECT_B3_T90_ACTIVE:
            fret = "CONNECT_B3_T90_ACTIVE_RESP";
            break;
        case CAPI_DATA_B3:
            fret = "DATA_B3_RESP";
            break;
        case CAPI_DISCONNECT_B3:
            fret = "DISCONNECT_B3_RESP";
            break;
        case CAPI_DISCONNECT:
            fret = "DISCONNECT_RESP";
            break;
        case CAPI_FACILITY:
            fret = "FACILITY_RESP";
            break;
        case CAPI_INFO:
            fret = "INFO_RESP";
            break;
        case CAPI_LISTEN:
            fret = "LISTEN_RESP";
            break;
        case CAPI_MANUFACTURER:
            fret = "MANUFACTURER_RESP";
            break;
        case CAPI_RESET_B3:
            fret = "RESET_B3_RESP";
            break;
        case CAPI_SELECT_B_PROTOCOL:
            fret = "SELECT_B_PROTOCOL_RESP";
            break;
        default:
            fret = "Wrong RESP-Command!";
            break;
        }
        break;
    default:
        fret = "Wrong SubCommand!";
        break;
    }
    return (tUChar *)fret;
}

/*===========================================================================*\
\*===========================================================================*/

void DbgCapiMsgTrace (c_message theMsg, c_info error, unsigned kind) {
    if (DCON_CAPITrace > Dbg_Level_Nothing) {
        if (error) {
            // Ein Fehler ist passiert
            switch (kind) {
            case CMTRACE_Register:
                dwriteErr (DCON_CAPITrace, "CapiTrace: CAPI_REGISTER Error=%x!\n", error);
                break;
            case CMTRACE_Release:
                dwriteErr (DCON_CAPITrace, "CapiTrace: CAPI_RELEASE Error=%x!\n", error);
                break;
            case CMTRACE_GetProfile:
                dwriteErr (DCON_CAPITrace, "CapiTrace: CAPI_GET_PROFILE Error=%x!\n", error);
                break;
            case CMTRACE_PutMessage:
                dwriteErr (DCON_CAPITrace, "CapiTrace: CAPI_PUT_MESSAGE Error=%x!\n", error);
                break;
            case CMTRACE_GetMessage:
                dwriteErr (DCON_CAPITrace, "CapiTrace: CAPI_GET_MESSAGE Error=%x!\n", error);
                break;
            default:
                // TODO: theMSG auswerten
                dwriteErr (DCON_CAPITrace, "CapiTrace: CAPI_???_MESSAGE Error=%x!\n", error);
                break;
            }
        } else {
            switch (kind) {
            case CMTRACE_Register:
                dwriteLog (DCON_CAPITrace, "CapiTrace: CAPI_REGISTER Success\n");
                break;
            case CMTRACE_Release:
                dwriteLog (DCON_CAPITrace, "CapiTrace: CAPI_RELEASE Success\n");
                break;
            case CMTRACE_GetProfile:
                dwriteLog (DCON_CAPITrace, "CapiTrace: CAPI_GET_PROFILE Success!\n");
                break;
            default:
                if (theMsg) {
                    if (CapiMsg_GetCmd (theMsg) == CAPI_DATA_B3) {
                        dwriteE (DCON_CAPITrace, "CapiTrace: %s(%x) ",
                                 DbgStrCapiCommand (CapiMsg_GetCmd (theMsg), CapiMsg_GetSubCmd (theMsg)),
                                 CapiMsg_GetAddr (theMsg));
                        switch (CapiMsg_GetSubCmd (theMsg)) {
                        case CAPI_REQ:
                        case CAPI_IND:
                            dwriteE (DCON_CAPITrace, "-Length=%x -Handle=%x ", CapiMsg_GetWord (theMsg, sizeof (c_dword)),
                                     CapiMsg_GetWord (theMsg, sizeof (c_dword) + sizeof (c_word)));
                            break;
                        case CAPI_CONF:
                        case CAPI_RESP:
                            dwriteE (DCON_CAPITrace, "-Handle=%x ", CapiMsg_GetWord (theMsg, 0));
                            break;
                        }
                        if (DCON_CAPITrace > Dbg_Level_Entry) {
                            dhead ("DbgCapiMsgTrace", DCON_CAPITrace);
                            dprintBuf (theMsg, CapiMsg_GetMsgLen (theMsg));
                        }
                    } else {
                        dwriteLog (DCON_CAPITrace, "CapiTrace: %s(%x) ",
                                 DbgStrCapiCommand (CapiMsg_GetCmd (theMsg), CapiMsg_GetSubCmd (theMsg)),
                                 CapiMsg_GetAddr (theMsg));
                        switch (CapiMsg_GetFullCmd (theMsg)) {
                        case CAPI_DISCONNECT_B3_IND:
                        case CAPI_DISCONNECT_IND:
                            dwriteLog (DCON_CAPITrace, "-Reason=%x ", CapiMsg_GetWord (theMsg, 0));
                            break;
                        case CAPI_CONNECT_RESP:
                            dwriteLog (DCON_CAPITrace, "-Reject=%x ", CapiMsg_GetWord (theMsg, 0));
                            break;
                        }

                        if (DCON_CAPITrace > Dbg_Level_Entry) {
                            dhead ("DbgCapiMsgTrace", DCON_CAPITrace);
                            dprintBuf (theMsg, CapiMsg_GetMsgLen (theMsg));
                        }
                    }
                }
            }
        }
    }
}

/*===========================================================================*\
\*===========================================================================*/
#endif
