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

#include "CapiMsg.h"
#include "c2msgdef.h"
#include "DCMTrace.h"
#include "osmem.h"
#include "dbgCAPI.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#ifndef NDEBUG

#define REGISTER_STATE_RELEASED     0
#define REGISTER_STATE_RELEASING    1
#define REGISTER_STATE_REGISTERING  2
#define REGISTER_STATE_REGISTERED   3

#endif

/*===========================================================================*\
    Interne Makros
\*===========================================================================*/

// NOTE: 1024 could be to small in some situations
//#define CapiCalcRegMsgBufSize(chans)    (1024 * (1 + (chans)))
#define CapiCalcRegMsgBufSize(chans)        (4096 * (1 + (chans)))

#define CapiMsg_SetMsgLen(name, len)                                    \
    set16 ((name), (len))


#define CapiMsg_SetApplID(name, applid)                                 \
    set16 ((name) + 2, (applid))


#define CapiMsg_SetCmd(name, capiCmd)                                   \
    (name)[4] = (capiCmd)


#define CapiMsg_SetSubCmd(name, capiSubCmd)                             \
    (name)[5] = (capiSubCmd)


#define CapiMsg_SetHeader(name, cmd, subcmd, msgnr, addr)               \
    CapiMsg_SetCmd (name, cmd);                                         \
    CapiMsg_SetSubCmd (name, subcmd);                                   \
    set16 ((name) + 6, (msgnr));                                        \
    set32 ((name) + 8, (addr))

#define CapiMsg_SetFullHeader(name, applid, cmd, subcmd, msgnr, addr)   \
    CapiMsg_SetMsgLen (name, CAPI_MSG_HEADER_ADDR_SIZE);                \
    CapiMsg_SetApplID (name, applid);                                   \
    CapiMsg_SetHeader (name, cmd, subcmd, msgnr, addr);


/*===========================================================================*\
    CCAPI20_MsgBase Konstruktor
\*===========================================================================*/

CCAPI20_MsgBase::CCAPI20_MsgBase (void)
  : m_hCapiBase (vIllegalHandle),
    m_CurPutMessage (0) {

    dhead ("CCAPI20_MsgBase-Konstruktor", DCON_CCAPI20_MsgBase);
    // TODO!!!: Error handling!!!!
    m_Protect.Init();
    PutMsgbufferQueue.Init (CAPI_MAX_MESSAGE_SIZE, CAPI_MAX_MESSAGE_COUNTER);
    #ifndef NDEBUG
        m_RegisterState = REGISTER_STATE_RELEASED;
    #endif
}


/*===========================================================================*\
    CCAPI20_MsgBase Destruktor
\*===========================================================================*/

CCAPI20_MsgBase::~CCAPI20_MsgBase (void) {
    dhead ("CCAPI20_MsgBase-Destruktor", DCON_CCAPI20_MsgBase);
    dassert (m_hCapiBase == vIllegalHandle);
    dassert (GetApplID() == IllegalApplID);
    //dassert (m_hCapiBase != vIllegalHandle);
    #ifndef NDEBUG
        dassert (m_RegisterState == REGISTER_STATE_RELEASED);
    #endif

    dwarning (m_CurPutMessage == 0);
    FreeCurPutMsg();
}


/*===========================================================================*\
    CCAPI20_MsgBase::REGISTER
\*===========================================================================*/

void CCAPI20_MsgBase::REGISTER (tUInt MaxB3Connection, tUInt MaxB3Blks, tUInt MaxSizeB3) {
    dhead ("CCAPI20_MsgBase::REGISTER", DCON_CCAPI20_MsgBase);
    dassert (GetApplID() == IllegalApplID);
    dassert (m_hCapiBase == vIllegalHandle);
    dassert (MaxB3Blks >= CAPI_MIN_OUTSTANDING_FRAMES);    // Check for to small values
    dassert (MaxSizeB3 >= CAPI_MIN_DATA_SIZE);
    dassert (MaxB3Blks <= CAPI_MAX_OUTSTANDING_FRAMES);    // Check for to great values
    dassert (MaxSizeB3 <= CAPI_MAX_DATA_SIZE);
    dwarning (m_CurPutMessage == 0);
    dwarning (PutMsgbufferQueue.IsQueueEmpty() == vTrue);
    #ifndef NDEBUG
        dassert (m_RegisterState == REGISTER_STATE_RELEASED);
        m_RegisterState = REGISTER_STATE_REGISTERING;
    #endif

    // TODO: Adjust size of PutMsgbufferQueue or MaxB3Connection!

    PutMsgbufferQueue.FreeQueue();
    FreeCurPutMsg();
    CapiBase_Register (this, CapiCalcRegMsgBufSize (MaxB3Connection), MaxB3Connection, MaxB3Blks, MaxSizeB3);
}


/*===========================================================================*\
    CCAPI20_MsgBase::RELEASE
\*===========================================================================*/

void CCAPI20_MsgBase::RELEASE  (void) {
    dhead ("CCAPI20_MsgBase::RELEASE", DCON_CCAPI20_MsgBase);
    #ifndef NDEBUG
        dwarning (m_RegisterState == REGISTER_STATE_REGISTERED);
        m_RegisterState = REGISTER_STATE_RELEASING;
    #endif
    dwarning (GetApplID() != IllegalApplID);
    dwarning (m_hCapiBase != vIllegalHandle);
    dwarning (PutMsgbufferQueue.IsQueueEmpty() == vTrue);
    PutMsgbufferQueue.FreeQueue();
    // queue a illegal Message to indicate EmptyPutMsgbufferQueue that it must call Release
    AllocAndPutMsg (0xFF, 0xFF, 0, 0, vFalse, 0);
}


/*===========================================================================*\
    CCAPI20_MsgBase::HandleMessageError
\*===========================================================================*/

void CCAPI20_MsgBase::HandleMessageError (tUInt type, c_info ErrorCode, c_message cmsg) {
    dhead ("CCAPI20_MsgBase::HandleMessageError", DCON_CCAPI20_MsgBase);
    dparams ("%x,%x", ErrorCode);
    if (ErrorCode == ci_Msg_IllAppNr) {
        if (GetApplID() != IllegalApplID) {
            RELEASE();
        }
    } else {
        switch (type) {
        case 0:
            // PutMessage Error
            if (!cmsg) {
                dassert (0);
                break;
            }
            switch (CapiMsg_GetSubCmd (cmsg)) {
            case CAPI_REQ:
                CapiMsg_SetSubCmd (cmsg, CAPI_CONF);
                switch (CapiMsg_GetCmd (cmsg)) {
                case CAPI_ALERT:
                case CAPI_CONNECT:
                case CAPI_DISCONNECT_B3:
                case CAPI_DISCONNECT:
                case CAPI_CONNECT_B3:
                case CAPI_INFO:
                case CAPI_LISTEN:
                case CAPI_RESET_B3:
                case CAPI_SELECT_B_PROTOCOL:
                    set16 (cmsg + CAPI_MSG_HEADER_ADDR_SIZE, (tUShort)ErrorCode);
                    CapiMsg_SetMsgLen (cmsg, CAPI_MSG_HEADER_ADDR_SIZE + sizeof (c_word));
                    break;

                case CAPI_DATA_B3:
                    set16 (cmsg + CAPI_MSG_HEADER_ADDR_SIZE, CapiMsg_GetWord (cmsg, sizeof (c_dword) + sizeof (c_word)));
                    set16 (cmsg + CAPI_MSG_HEADER_ADDR_SIZE + sizeof (c_word), (tUShort)ErrorCode);
                    CapiMsg_SetMsgLen (cmsg, CAPI_MSG_HEADER_ADDR_SIZE + 2 * sizeof (c_word));
                    break;

                case CAPI_FACILITY: {
                    for (tUInt i = CAPI_MSG_HEADER_ADDR_SIZE; i < CapiMsg_GetMsgLen (cmsg); i++) {
                        set8 (cmsg + i + sizeof (c_word), cmsg[i]);
                    }
                    set16 (cmsg + CAPI_MSG_HEADER_ADDR_SIZE, (tUShort)ErrorCode);
                    CapiMsg_SetMsgLen (cmsg, (c_word)(CapiMsg_GetMsgLen (cmsg) + sizeof (c_word)));
                    break;
                }

                case CAPI_MANUFACTURER:
                    set32 (cmsg + CAPI_MSG_HEADER_ADDR_SIZE + sizeof (c_dword), ErrorCode);
                    /*----- I hope it's enough to set class to ErrorCode -----*/
                    dprintErr ("MANUFACTURER_REQ-Error detected!!");
                    break;

                default:
                    dprintErr ("Illegal Command for a CAPI-Request!");
                    break;
                }
                HandleGetMessage (cmsg);
                break;
            case CAPI_RESP:
                dprintErr ("RESP-Error: MsgNr=%x, Address=%x, Command=%x, Error=%x!",
                           CapiMsg_GetMsgNr (cmsg), CapiMsg_GetAddr (cmsg), CapiMsg_GetCmd(cmsg), ErrorCode);
                break;
            default:
                dprintErr ("Wrong Subcommand!");
                break;
            }
            break;
        case 1:
            // GetMessage Error
            RELEASE();
            // or make a GetMessage();
            break;
        case 2:
            // GetProfile Error
            break;
        default:
            dwarning (0);
            break;
        }
    }
}


/*===========================================================================*\
    CCAPI20_MsgBase::AllocAndPutMsg
\*===========================================================================*/

void CCAPI20_MsgBase::AllocAndPutMsg (c_byte Command, c_byte SubCommand, c_word Messagenumber,
                                      c_dword Address, tBool ZeroIsEmptyStruct, CCStruct *Params) {
    dhead ("CCAPI20_MsgBase::AllocAndPutMsg", DCON_CCAPI20_MsgBase);
    dparams ("%x,%x,%x,%x", Command, SubCommand, Messagenumber, Address);
    dassert (Command != 0);
    dassert (SubCommand != 0);
    //dassert (Address != 0);

    c_message cmsg = PutMsgbufferQueue.Alloc();
    dwarning (cmsg != 0);
    if (cmsg) {
        dprint ("Message(%x,%x) ", Command, SubCommand);

        CapiMsg_SetFullHeader (cmsg, GetApplID(), Command, SubCommand, Messagenumber, Address);
        tUInt slen = CAPI_MSG_HEADER_ADDR_SIZE;
        if (Params) {
            slen += Params->CalcStructSize();
            dassert (slen <= CAPI_MAX_MESSAGE_SIZE);
            if (slen <= CAPI_MAX_MESSAGE_SIZE) {
                Params->FillBuffer (cmsg + CAPI_MSG_HEADER_ADDR_SIZE);
            }
        } else if (ZeroIsEmptyStruct) {
            cmsg[CAPI_MSG_HEADER_ADDR_SIZE] = 0;
            slen += 1;
        }
        CapiMsg_SetMsgLen (cmsg, (c_word)slen);

        //dprint ("Buffer=%x, Len=%x", cmsg, CapiMsg_GetMsgLen (cmsg));
        //dprintBuf (cmsg, CapiMsg_GetMsgLen(cmsg));
        PutMsgbufferQueue.InQueue (cmsg);
        EmptyPutMsgbufferQueue();
    } else {
        dwarning (0);
        dassert (SubCommand != 0xFF);
        // alloc lMessage on the stack is worse so we use static, thats only a little better
        // because for all MsgBase-classes exit only one lMessage
        //static c_byte lMessage[CAPI_MAX_MESSAGE_SIZE];
        HandleMessageError (0, ci_Int_MemoryFullErr, 0);
    }
}


/*===========================================================================*\
    CCAPI20_MsgBase::EmptyPutMsgbufferQueue
\*===========================================================================*/

void CCAPI20_MsgBase::EmptyPutMsgbufferQueue (void) {
    dhead ("CCAPI20_MsgBase::EmptyPutMsgbufferQueue", DCON_CCAPI20_MsgBase);
    m_Protect.BeginWrite();
    if (m_CurPutMessage == 0) {
        m_CurPutMessage = PutMsgbufferQueue.OutQueue();
        dassert ((m_CurPutMessage == 0) || (CapiMsg_GetMsgLen (m_CurPutMessage) >= CAPI_MSG_HEADER_ADDR_SIZE));
        if (m_CurPutMessage) {
            m_Protect.EndWrite();
            dassert (m_hCapiBase != vIllegalHandle);
            if (CapiMsg_GetSubCmd (m_CurPutMessage) != 0xFF) {
                dassert (CapiMsg_GetApplID (m_CurPutMessage) != IllegalApplID);
                if (GetApplID() != IllegalApplID) {
                    CapiBase_PutMessage (m_hCapiBase, m_CurPutMessage);
                } else {
                    FreeCurPutMsg();
                }
            } else {
                // an RELASE was queued!!
                FreeCurPutMsg();
                CapiBase_Release (m_hCapiBase);
            }
            return;
        }
    }
    m_Protect.EndWrite();
}


/*===========================================================================*\
    CCAPI20_MsgBase::RegisterComplete
\*===========================================================================*/

void CCAPI20_MsgBase::RegisterComplete (thCapiBase Base, c_info ErrorCode) {
    dhead ("CCAPI20_MsgBase::RegisterComplete", DCON_CCAPI20_MsgBase);
    dparams ("hBase=%p,ApplId=%x,ErrorCode=%x", Base, ApplID(), ErrorCode);
    dassert (m_CurPutMessage == 0);
    #if !defined NDEBUG && defined USE_DEBUG_INFO_CLASS && (USE_DEBUG_INFO_CLASS > _USE_DEBUG_INFO_CLASS_NONE)
        DbgCapiMsgTrace (0, ErrorCode, CMTRACE_Register);
    #endif
    if (Base != vIllegalHandle) {
        dassert ((m_hCapiBase == 0) || (m_hCapiBase == Base));
        dassert ((ErrorCode == ci_Int_RegisterDelayed) || (CapiInfoIsSuccess (ErrorCode) == vTrue));
        dassert ((ErrorCode == ci_Int_RegisterDelayed) || (ApplID() != IllegalApplID));
        m_hCapiBase = Base;
        #ifndef NDEBUG
            if (ErrorCode != ci_Int_RegisterDelayed) {
                dassert (m_RegisterState == REGISTER_STATE_REGISTERING);
                m_RegisterState = REGISTER_STATE_REGISTERED;
            }
        #endif
    } else {
        m_hCapiBase = vIllegalHandle;
        dassert (CapiInfoIsRegError (ErrorCode) == vTrue);
        dassert (ErrorCode != ci_Reg_Busy);
        dassert (ApplID() == IllegalApplID);
        #ifndef NDEBUG
            dassert (m_RegisterState == REGISTER_STATE_REGISTERING);
            m_RegisterState = REGISTER_STATE_RELEASED;
        #endif
    }
    REGISTER_COMPLETE (ErrorCode);
}


/*===========================================================================*\
    CCAPI20_MsgBase::ReleaseComplete
\*===========================================================================*/

void CCAPI20_MsgBase::ReleaseComplete (void) {
    dhead ("CCAPI20_MsgBase::ReleaseComplete", DCON_CCAPI20_MsgBase);
    dwarning (m_CurPutMessage == 0);
    dwarning (GetApplID() == IllegalApplID);
    dwarning (PutMsgbufferQueue.IsQueueEmpty() == vTrue);
    dassert (m_hCapiBase != vIllegalHandle);
    #if !defined NDEBUG && defined USE_DEBUG_INFO_CLASS && (USE_DEBUG_INFO_CLASS > _USE_DEBUG_INFO_CLASS_NONE)
        DbgCapiMsgTrace (0, ci_NoError, CMTRACE_Release);
    #endif
    #ifndef NDEBUG
        dassert (m_RegisterState == REGISTER_STATE_RELEASING);
        m_RegisterState = REGISTER_STATE_RELEASED;
    #endif
    m_hCapiBase = vIllegalHandle;
    PutMsgbufferQueue.FreeQueue();
    FreeCurPutMsg();
    RELEASE_COMPLETE();
}


/*===========================================================================*\
    CCAPI20_MsgBase::PutMessageComplete
\*===========================================================================*/

void CCAPI20_MsgBase::PutMessageComplete (c_info ErrorCode) {
    dhead ("CCAPI20_MsgBase::PutMessageComplete", DCON_CCAPI20_MsgBase);
    dassert (m_CurPutMessage != 0);
    dassert ((m_CurPutMessage == 0) || (CapiMsg_GetMsgLen (m_CurPutMessage) >= CAPI_MSG_HEADER_ADDR_SIZE));
    #if !defined NDEBUG && defined USE_DEBUG_INFO_CLASS && (USE_DEBUG_INFO_CLASS > _USE_DEBUG_INFO_CLASS_NONE)
        DbgCapiMsgTrace (m_CurPutMessage, ErrorCode, CMTRACE_PutMessage);
    #endif
    if (ErrorCode != ci_NoError) {
        dassert (ErrorCode != ci_Msg_Busy);
        dassert (ErrorCode != ci_Msg_SendQueueFull);
        HandleMessageError (0, ErrorCode, m_CurPutMessage);
    }
    FreeCurPutMsg();
    EmptyPutMsgbufferQueue();
}


/*===========================================================================*\
    CCAPI20_MsgBase::GetMessageReceived
\*===========================================================================*/

void CCAPI20_MsgBase::GetMessageReceived (c_byte *pMessage, c_info ErrorCode) {
    dhead ("CCAPI20_MsgBase::GetMessageReceived", DCON_CCAPI20_MsgBase);
    dparams ("buf=%p, len=%x, err=%x", pMessage, (pMessage) ? CapiMsg_GetMsgLen(pMessage) : 0,
             ErrorCode);
    dassert (ErrorCode != ci_Msg_Busy);
    dassert (ErrorCode != ci_Msg_SendQueueFull);
    #if !defined NDEBUG && defined USE_DEBUG_INFO_CLASS && (USE_DEBUG_INFO_CLASS > _USE_DEBUG_INFO_CLASS_NONE)
        DbgCapiMsgTrace (pMessage, ErrorCode, CMTRACE_GetMessage);
    #endif
    #ifdef NDEBUG
        dwarning (m_RegisterState == REGISTER_STATE_REGISTERED);
    #endif
    if (ErrorCode == ci_NoError) {
        dassert (m_hCapiBase != vIllegalHandle);
        dassert (pMessage != 0);
        HandleGetMessage (pMessage);
    } else {
        dassert (ErrorCode != ci_Msg_Busy);
        dassert (ErrorCode != ci_Msg_SendQueueFull);
        HandleMessageError (1, ErrorCode, 0);
    }
}


/*===========================================================================*\
    CCAPI20_MsgBase::HandleGetMessage
\*===========================================================================*/

void CCAPI20_MsgBase::HandleGetMessage (c_byte *Message) {
    dhead ("CCAPI20_MsgBase::HandleGetMessage", DCON_CCAPI20_MsgBase);
    dassert (Message != 0);
    dassert (CapiMsg_GetMsgLen (Message) >= CAPI_MSG_HEADER_ADDR_SIZE);
    //dprintBuf (Message, CapiMsg_GetMsgLen(Message));
    switch (CapiMsg_GetSubCmd (Message)) {
    /*-------------------------------------------------------------------*\
        CAPI - Confirmations
    \*-------------------------------------------------------------------*/
    case CAPI_CONF:
        switch (CapiMsg_GetCmd (Message)) {
        case CAPI_ALERT:
            ALERT_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), CapiMsg_GetInfo (Message, 0));
            break;

        case CAPI_CONNECT:
            CONNECT_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), CapiMsg_GetInfo (Message, 0));
            break;

        case CAPI_CONNECT_B3:
            CONNECT_B3_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), CapiMsg_GetInfo (Message, 0));
            break;

        case CAPI_DATA_B3: {
            cm_datab3conf pdc;
            pdc.ParseMessage (Message);
            DATA_B3_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pdc);
            break;
        }
        case CAPI_DISCONNECT_B3:
            DISCONNECT_B3_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), CapiMsg_GetInfo (Message, 0));
            break;

        case CAPI_DISCONNECT:
            DISCONNECT_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), CapiMsg_GetInfo (Message, 0));
            break;

        case CAPI_FACILITY: {
            cm_facilityconf pf;
            pf.ParseMessage (Message);
            FACILITY_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pf);
            break;
        }
        case CAPI_INFO:
            INFO_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), CapiMsg_GetInfo (Message, 0));
            break;

        case CAPI_LISTEN:
            LISTEN_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), CapiMsg_GetInfo (Message, 0));
            break;

        case CAPI_MANUFACTURER: {
            cm_manufacturer_ref pm;
            pm.ParseMessage (Message);
            MANUFACTURER_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pm);
            break;
        }
        case CAPI_RESET_B3:
            RESET_B3_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), CapiMsg_GetInfo (Message, 0));
            break;

        case CAPI_SELECT_B_PROTOCOL:
            SELECT_B_PROTOCOL_CONF (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), CapiMsg_GetInfo (Message, 0));
            break;
        }
        break;

    /*-------------------------------------------------------------------*\
        CAPI - Indications
    \*-------------------------------------------------------------------*/
    case CAPI_IND:
        switch (CapiMsg_GetCmd (Message)) {
        case CAPI_CONNECT: {
            cm_connectind pci;
            pci.ParseMessage (Message);
            if (CONNECT_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pci) == vFalse) {

                CCAPI20_MsgBase::CONNECT_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pci);
            }
            break;
        }
        case CAPI_CONNECT_ACTIVE: {
            cm_connectactiveind pcai;
            pcai.ParseMessage (Message);
            if (CONNECT_ACTIVE_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pcai) == vFalse) {
                CCAPI20_MsgBase::CONNECT_ACTIVE_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pcai);
            }
            break;
        }
        case CAPI_CONNECT_B3: {
            CCStruct lncpi;
            lncpi.ParseMessage (Message);
            if (CONNECT_B3_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &lncpi) == vFalse) {
                CCAPI20_MsgBase::CONNECT_B3_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &lncpi);
            }
            break;
        }
        case CAPI_CONNECT_B3_ACTIVE: {
            CCStruct lncpi;
            lncpi.ParseMessage (Message);
            if (CONNECT_B3_ACTIVE_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &lncpi) == vFalse) {
                CCAPI20_MsgBase::CONNECT_B3_ACTIVE_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &lncpi);
            }
            break;
        }
        case CAPI_CONNECT_B3_T90_ACTIVE:
            if (CONNECT_B3_T90_ACTIVE_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message)) == vFalse) {
                CCAPI20_MsgBase::CONNECT_B3_T90_ACTIVE_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message));
            }
            break;

        case CAPI_DATA_B3: {
            cm_datab3 pdb3;
            pdb3.ParseMessage (Message);
            if (DATA_B3_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pdb3) == vFalse) {
                CCAPI20_MsgBase::DATA_B3_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pdb3);
            }
            break;
        }
        case CAPI_DISCONNECT_B3: {
            cm_disconnectb3ind pdbi;
            pdbi.ParseMessage (Message);
            if (DISCONNECT_B3_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pdbi) == vFalse) {
                CCAPI20_MsgBase::DISCONNECT_B3_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pdbi);
            }
            break;
        }
        case CAPI_DISCONNECT:
            if (DISCONNECT_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message),
                                CapiMsg_GetInfo (Message, 0)) == vFalse) {
                CCAPI20_MsgBase::DISCONNECT_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message),
                                CapiMsg_GetInfo (Message, 0));
            }
            break;

        case CAPI_FACILITY: {
            cm_facilityind pf;
            pf.ParseMessage (Message);
            if (FACILITY_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pf) == vFalse) {
                CCAPI20_MsgBase::FACILITY_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pf);
            }
            break;
        }
        case CAPI_INFO: {
            cm_infoind pii;
            pii.ParseMessage (Message);
            if (INFO_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pii) == vFalse) {
                CCAPI20_MsgBase::INFO_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pii);
            }
            break;
        }
        case CAPI_MANUFACTURER: {
            cm_manufacturer_ref pm;
            pm.ParseMessage (Message);
            if (MANUFACTURER_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pm) == vFalse) {
                CCAPI20_MsgBase::MANUFACTURER_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &pm);
            }
            break;
        }
        case CAPI_RESET_B3: {
            CCStruct lncpi;
            lncpi.ParseMessage (Message);
            if (RESET_B3_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &lncpi) == vFalse) {
                CCAPI20_MsgBase::RESET_B3_IND (CapiMsg_GetMsgNr (Message), CapiMsg_GetAddr (Message), &lncpi);
            }
            break;
        }
        }
    }
}


/*===========================================================================*\
\*===========================================================================*/
