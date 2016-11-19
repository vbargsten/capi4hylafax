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

#include "CapiChan.h"
#include "dbgCAPI.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define TIMER_FOR_RINGING                    1
#define TIMER_FOR_BCHANNEL                   2
#define TIMER_FOR_DISCONNECT                 3
#define TIMER_BCHANNEL_WAITTIME             90 // sec
#define TIMER_DISCONNECT_WAITTIME           20 // sec

static void StateChangeTimer_CallBack (void *pCapiChannel, void *kind) {
    dhead ("StateChangeTimer_CallBack", DCON_CCAPI20_Channel);
    dparams ("%x", kind);
    dassert (pCapiChannel != 0);
    CCAPI20_Channel *pChan = (CCAPI20_Channel *)pCapiChannel;
    switch (pChan->GetState()) {
    case cs_B_ConnectPending:
    case cs_D_Connected:
        if ((tULong)kind == TIMER_FOR_RINGING) {
            dwarning (0); // this kind of timer should be cancelled before
            break;
        }
        // no break!
    case cs_D_ConnectPending:
    case cs_D_ConnectRequest:
        pChan->DisconnectReq();
        break;
    case cs_D_DisconnectPending:
        dwarning (0);
        pChan->Release();
        break;
    default:
        // in this states the timer should have been canceled before
        dwarning (0);
        break;
    }
}


/*===========================================================================*\
    CCAPI20_Channel Konstruktor
\*===========================================================================*/

CCAPI20_Channel::CCAPI20_Channel (void)
  : hStateChangeTimer (vIllegalHandle),
    curState (cs_Released),
    RingingTimeout (0) {

    dhead ("CCAPI20_Channel-Constructor", DCON_CCAPI20_Channel);
    m_Protect.Init();
    SetController (IllegalController);
    SetPLCI (IllegalPLCI);
    SetNCCI (IllegalNCCI);
    SetInitiator (vFalse);
    SecTimer_Create (&hStateChangeTimer);
}


/*===========================================================================*\
    CCAPI20_Channel Destruktor
\*===========================================================================*/

CCAPI20_Channel::~CCAPI20_Channel (void) {
    dhead ("CCAPI20_Channel-Destructor", DCON_CCAPI20_Channel);
    dassert (GetState() == cs_Released);
    dassert (GetPLCI() == IllegalPLCI);
    dassert (GetNCCI() == IllegalNCCI);
    dwarning (GetInitiator() == vFalse);
    SetNCPI (0);
    if (hStateChangeTimer != vIllegalHandle) {
        SecTimer_Destroy (&hStateChangeTimer);
    }
}


/*===========================================================================*\
    CCAPI20_Channel::SetNCPI
\*===========================================================================*/

void CCAPI20_Channel::SetNCPI (CCStruct *pNCPI) {
    dhead ("CCAPI20_Channel::SetNCPI", DCON_CCAPI20_Channel);
    m_NCPIout.ParseStruct (pNCPI);
}


/*===========================================================================*\
    CCAPI20_Channel::AcceptIncomingCall
\*===========================================================================*/

tBool CCAPI20_Channel::AcceptIncomingCall (void) {
    dhead ("CCAPI20_Channel::AcceptIncomingCall", DCON_CCAPI20_Channel);
    dwarning (GetState() == cs_D_ConnectIndication);
    if (GetState() != cs_D_ConnectIndication) {
        RETURN ('x', vFalse);
    }
    dassert (GetPLCI() != IllegalPLCI);
    if (SetNewPLCIEntry (GetPLCI()) == vTrue) {
        RETURN ('x', vTrue);
    }
    DisconnectReq();
    //if (SetState (cs_D_DisconnectPending, cs_D_ConnectIndication) == vTrue) {
    //    dassert (hStateChangeTimer != vIllegalHandle);
    //    SecTimer_Call (hStateChangeTimer, TIMER_DISCONNECT_WAITTIME, StateChangeTimer_CallBack,
    //                   this, (void *)TIMER_FOR_DISCONNECT);
    //    cm_connectresp crIgnore;
    //    CONNECT_RESP (GetNextMsgNr(), GetPLCI(), &crIgnore);
    //}
    RETURN ('x', vFalse);
}


/*===========================================================================*\
    CCAPI20_Channel::Register
\*===========================================================================*/

void CCAPI20_Channel::Register (tUInt MaxDataSize, tUInt MaxInFrames) {
    dhead ("CCAPI20_Channel::Register", DCON_CCAPI20_Channel);
    dparams ("t=%x", this);
    dassert (GetState() == cs_Released);
    dassert (MaxDataSize <= CAPI_MAX_DATA_SIZE);
    dwarning (MaxDataSize > 100);
    if (  (m_Protect.Init() == vFalse)
       || (  (hStateChangeTimer == vIllegalHandle)
          && (SecTimer_Create (&hStateChangeTimer) == vFalse))) {
        Register_Complete (ci_Int_TimerInitFailure);
    } else if (SetState (cs_Registering, cs_Released) == vTrue) {
        REGISTER (1, MaxInFrames, MaxDataSize);
    } else {
        dwarning (0);
    }
}


/*===========================================================================*\
    CCAPI20_Channel::Release
\*===========================================================================*/

void CCAPI20_Channel::Release (void) {
    dhead ("CCAPI20_Channel::Release", DCON_CCAPI20_Channel);
    dassert (GetState() > cs_Released);
    dassert (GetState() < cs_D_DisconnectPending);
    m_Protect.BeginWrite();
    switch (GetState()) {
    case cs_Released:
        dwarning (0);
        m_Protect.EndWrite();
        Release_Complete();
        break;
    case cs_Releasing:
        // nothing to do!
        m_Protect.EndWrite();
        break;
    case cs_Registering:
        curState = cs_Releasing;
        m_Protect.EndWrite();
        ResetValues();
        break;
    default:
        dwarning (0);
        // no break;
    case cs_Registered:
    case cs_RegisterDelayed:
        curState = cs_Releasing;
        m_Protect.EndWrite();
        ResetValues();
        RELEASE();
    }
}


/*===========================================================================*\
    CCAPI20_Channel::ConnectReq
\*===========================================================================*/

tBool CCAPI20_Channel::ConnectReq (c_dword Controller, cm_connectreq *Params) {
    dhead ("CCAPI20_Channel::ConnectReq", DCON_CCAPI20_Channel);
    dassert (Params != 0);
    if (SetState (cs_D_ConnectRequest, cs_Registered) == vFalse) {
        RETURN ('x', vFalse);
    }
    dassert (GetController() == IllegalController);
    SetController (Controller);
    SetInitiator (vTrue);
    CONNECT_REQ (GetNextMsgNr(), Controller, Params);
    RETURN ('x', vTrue);
}


/*===========================================================================*\
    CCAPI20_Channel::DataB3Req
\*===========================================================================*/

void CCAPI20_Channel::DataB3Req (cm_datab3 *Params) {
    dhead ("CCAPI20_Channel::DataB3Req", DCON_CCAPI20_Channel);
    dassert (Params != 0);
    dwarning (GetState() == cs_Connected);
    if (GetState() == cs_Connected) {
        dassert (GetNCCI() != IllegalNCCI);
        DATA_B3_REQ (GetNextMsgNr(), GetNCCI(), Params);
    } else {
        cm_datab3conf cp (Params->DataHandle, ci_Int_NotConnected);
        DataB3Conf (&cp);
    }
}


/*===========================================================================*\
    CCAPI20_Channel::DisconnectReq
\*===========================================================================*/

void CCAPI20_Channel::DisconnectReq (tBool FastDisconnect) {
    dhead ("CCAPI20_Channel::DisconnectReq", DCON_CCAPI20_Channel);
    dwarning (GetState() >= cs_Registered);
    m_Protect.BeginWrite();
    switch (GetState()) {
    case cs_D_ConnectRequest:
        curState = cs_D_DisconnectPending;
        dassert (hStateChangeTimer != vIllegalHandle);
        m_Protect.EndWrite();
        SecTimer_Call (hStateChangeTimer, TIMER_DISCONNECT_WAITTIME, StateChangeTimer_CallBack,
                       this, (void *)TIMER_FOR_DISCONNECT);
        break;

    case cs_D_ConnectIndication: {
        curState = cs_D_DisconnectPending;
        m_Protect.EndWrite();
        SecTimer_Call (hStateChangeTimer, TIMER_DISCONNECT_WAITTIME, StateChangeTimer_CallBack,
                       this, (void *)TIMER_FOR_DISCONNECT);
        cm_connectresp crIgnore;
        CONNECT_RESP (GetNextMsgNr(), GetPLCI(), &crIgnore);
        break;
    }
    case cs_D_DisconnectRespPending:
    case cs_D_DisconnectPending:
        /*----- Do Nothing!! -----*/
        m_Protect.EndWrite();
        break;

    case cs_B_DisconnectRespPending:
    case cs_B_DisconnectPending:
        if (FastDisconnect == vFalse) {
            m_Protect.EndWrite();
            break;
        }
        // no break; -> Okay we make a second test for FastDisconnect before calling DISCONNECT_REQ,
        // but that doesn't matter

    case cs_B_ConnectPending:
    case cs_Connected:
        if (FastDisconnect == vFalse) {
            dassert (GetNCCI() != IllegalNCCI);
            curState = cs_B_DisconnectPending;
            m_Protect.EndWrite();
            DISCONNECT_B3_REQ (GetNextMsgNr(), GetNCCI(), &m_NCPIout);
            break;
        }
        // no break!

    default:
        dassert (GetPLCI() != IllegalPLCI);
        dwarning (GetNCCI() == IllegalNCCI);
        curState = cs_D_DisconnectPending;
        dassert (hStateChangeTimer != vIllegalHandle);
        m_Protect.EndWrite();
        SecTimer_Call (hStateChangeTimer, TIMER_DISCONNECT_WAITTIME, StateChangeTimer_CallBack,
                       this, (void *)TIMER_FOR_DISCONNECT);
        DISCONNECT_REQ (GetNextMsgNr(), GetPLCI());
        break;

    case cs_Released:
    case cs_Releasing:
    case cs_Registering:
    case cs_RegisterDelayed:
        dwarning (0); // DisconnectReq should not been called in this states!
        // no break!
    case cs_Registered:
        m_Protect.EndWrite();
        DisconnectInd (ci_NoError);
        break;
    }
}


/*===========================================================================*\
    CCAPI20_Channel::ConnectResp
\*===========================================================================*/

void CCAPI20_Channel::ConnectResp (cm_connectresp *Params) {
    dhead ("CCAPI20_Channel::ConnectResp", DCON_CCAPI20_Channel);
    dassert (Params != 0);
    dwarning (GetState() == cs_D_ConnectIndication);
    dassert (GetInitiator() == vFalse);
    dassert (GetPLCI() != IllegalPLCI);

    // TODO!!: for DDI this handling isn't correct!!!!!!!
    if (Params->Reject == creject_Accept) {
        if (SetState (cs_D_ConnectPending, cs_D_ConnectIndication) == vTrue) {
            CONNECT_RESP (GetNextMsgNr(), GetPLCI(), Params);
        } else {
            dwarning (0);
        }
    } else {
        if (SetState (cs_D_DisconnectPending, cs_D_ConnectIndication) == vTrue) {
            CONNECT_RESP (GetNextMsgNr(), GetPLCI(), Params);
            ResetValues();
            SetState (cs_Registered, cs_D_DisconnectPending);
        } else {
            dwarning (0);
        }
    }
}


/*===========================================================================*\
    CCAPI20_Channel::DataB3Resp
\*===========================================================================*/

void CCAPI20_Channel::DataB3Resp (c_word DataHandle) {
    dhead ("CCAPI20_Channel::DataB3Resp", DCON_CCAPI20_Channel);
    dassert (GetNCCI() != IllegalNCCI);
    dwarning (GetState() == cs_Connected);

    if (GetState() == cs_Connected) {
        // default way
        cm_datab3resp db3r (DataHandle);
        DATA_B3_RESP (GetNextMsgNr(), GetNCCI(), &db3r);
    }
}


/*===========================================================================*\
    CCAPI20_Channel::DisconnectB3Resp
\*===========================================================================*/

void CCAPI20_Channel::DisconnectB3Resp (void) {
    dhead ("CCAPI20_Channel::DisconnectB3Resp", DCON_CCAPI20_Channel);
    dassert (GetNCCI() != IllegalNCCI);
    dwarning (GetState() == cs_B_DisconnectRespPending);
    if (SetState (cs_D_Connected, cs_B_DisconnectRespPending) == vTrue) {
        DISCONNECT_B3_RESP (GetNextMsgNr(), GetNCCI());
        SetNCCI (IllegalNCCI);
        if (SetState (cs_D_DisconnectPending, cs_D_Connected) == vTrue) {
            dassert (hStateChangeTimer != vIllegalHandle);
            SecTimer_Call (hStateChangeTimer, TIMER_DISCONNECT_WAITTIME, StateChangeTimer_CallBack,
                           this, (void *)TIMER_FOR_DISCONNECT);
            DISCONNECT_REQ (GetNextMsgNr(), GetPLCI());
        }
    } else if (SetState (cs_Registered, cs_D_DisconnectRespPending) == vTrue) {
        dwarning (0);
        SecTimer_Cancel (hStateChangeTimer);
        DISCONNECT_B3_RESP (GetNextMsgNr(), GetNCCI());
        SetNCCI (IllegalNCCI);
        DISCONNECT_RESP (GetNextMsgNr(), GetPLCI());
        ResetValues();
        DisconnectInd (ci_NoError);
    } else {
        dassert (0);
    }
}


/*===========================================================================*\
    CCAPI20_Channel::REGISTER_COMPLETE
\*===========================================================================*/

void CCAPI20_Channel::REGISTER_COMPLETE (c_info ErrorCode) {
    dhead ("CCAPI20_Channel::REGISTER_COMPLETE", DCON_CCAPI20_Channel);
    dwarning ((GetState() == cs_Registering) || (GetState() == cs_RegisterDelayed));
    dwarning (ErrorCode == ci_NoError);
    if (GetState() == cs_Releasing) {
        ResetValues();
        if (CapiInfoIsError (ErrorCode) && (ErrorCode != ci_Int_RegisterDelayed)) {
            Release_Complete();
        } else {
            RELEASE();
        }
        return;
    }
    dassert ((GetState() == cs_Registering) || (GetState() == cs_RegisterDelayed));
    tCapiState new_cs;
    switch (ErrorCode) {
    default:
        if (CapiInfoIsError (ErrorCode)) {
            dassert (GetApplID() == IllegalApplID);
            new_cs = cs_Released;
            break;
        }
        dwarning (0);
        // no break;
    case ci_NoError:
        dassert (GetApplID() != IllegalApplID);
        new_cs = cs_Registered;
        break;
    case ci_Int_RegisterDelayed:
        dassert (GetApplID() == IllegalApplID);
        new_cs = cs_RegisterDelayed;
        break;
    }
    dassert ((GetState() == cs_Registering) || (GetState() == cs_RegisterDelayed));
    m_Protect.BeginWrite();
    curState = new_cs;
    m_Protect.EndWrite();
    dprint ("curstate=%x ", curState);
    Register_Complete (ErrorCode);
}


/*===========================================================================*\
    CCAPI20_Channel::RELEASE_COMPLETE
\*===========================================================================*/

void CCAPI20_Channel::RELEASE_COMPLETE (void) {
    dhead ("CCAPI20_Channel::RELEASE_COMPLETE", DCON_CCAPI20_Channel);
    dparams ("%x", GetState());
    ResetValues();
    dassert (hStateChangeTimer != vIllegalHandle);
    dinfo ((GetState() == cs_Registered) || (GetState() == cs_Releasing) || (GetState() == cs_RegisterDelayed),
           "ReleaseComplete, but State=%x ", GetState());
    SecTimer_Cancel (hStateChangeTimer);

    m_Protect.BeginWrite();
    tCapiState oldState = curState;
    curState            = cs_Released;
    m_Protect.EndWrite();
    switch (oldState) {
    case cs_Released:
        dassert (0);
        break;
    case cs_RegisterDelayed:
    case cs_Releasing:
    case cs_Registered:
    case cs_Registering:
        dwarning (0);
        break;
    default:
        DisconnectInd (ci_NoError);
        break;
    }
    Release_Complete();
}


/*===========================================================================*\
    CCAPI20_Channel - Conf-Messages
\*===========================================================================*/

void CCAPI20_Channel::CONNECT_CONF (c_word, c_dword PLCI, c_info Info) {
    dhead ("CCAPI20_Channel::CONNECT_CONF", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    if (CapiInfoIsSuccess (Info)) {
        if (SetState (cs_D_ConnectPending, cs_D_ConnectRequest) == vTrue) {
            dassert (GetController() != IllegalController);
            dassert (GetPLCI() == IllegalPLCI);
            dassert (GetNCCI() == IllegalNCCI);
            dassert (GetInitiator() == vTrue);
            SetPLCI (PLCI);
            if (RingingTimeout) {
                dassert (hStateChangeTimer != vIllegalHandle);
                SecTimer_Call (hStateChangeTimer, RingingTimeout, StateChangeTimer_CallBack,
                               this, (void *)TIMER_FOR_RINGING);
            }
            ConnectConf();
        } else {
            dwarning (0);
            if (GetState() == cs_D_DisconnectPending) {
                dassert (GetPLCI() == IllegalPLCI);
                dassert (GetController() != IllegalController);
                dassert (hStateChangeTimer != vIllegalHandle);
                SetPLCI (PLCI);
                SecTimer_Call (hStateChangeTimer, TIMER_DISCONNECT_WAITTIME, StateChangeTimer_CallBack,
                               this, (void *)TIMER_FOR_DISCONNECT);
            }
            DISCONNECT_REQ (GetNextMsgNr(), PLCI);
        }
    } else {
        SetController (IllegalController);
        SetInitiator (vFalse);
        if (SetState (cs_Registered, cs_D_ConnectRequest) == vFalse) {
            if (SetState (cs_Registered, cs_D_DisconnectPending) == vFalse) {
                dassert (0);
            }
        }
        DisconnectInd (Info);
    }
}

void CCAPI20_Channel::CONNECT_B3_CONF (c_word, c_dword NCCI, c_info Info) {
    dhead ("CCAPI20_Channel::CONNECT_B3_CONF", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    if (CapiInfoIsSuccess (Info)) {
        if ((GetPLCIFromNCCI (NCCI) == GetPLCI()) && (SetState (cs_B_ConnectPending, cs_D_Connected) == vTrue)) {
            dassert (GetNCCI() == IllegalNCCI);
            dassert (GetInitiator() == vFalse);
            SetNCCI (NCCI);
            dprint ("curstate=%x, ncci=%x, NCCI=%x ", GetState(), GetNCCI(), NCCI);
            return;
        }
        dwarning (0);
    }
    DISCONNECT_REQ (GetNextMsgNr(), GetPLCIFromNCCI (NCCI));
}

void CCAPI20_Channel::DATA_B3_CONF (c_word, c_dword NCCI, cm_datab3conf *Params) {
    dhead ("CCAPI20_Channel::DATA_B3_CONF", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    dassert (Params != 0);
    dprint ("Handle=%x Info=%x ", Params->DataHandle, Params->Info);
    if (NCCI != GetNCCI()) {
        dwarning (0);
        DISCONNECT_B3_REQ (GetNextMsgNr(), NCCI, 0);
        return;
    }
    dwarning (GetState() == cs_Connected);
    DataB3Conf (Params);
}

void CCAPI20_Channel::DISCONNECT_B3_CONF (c_word, c_dword NCCI, c_info Info) {
    dhead ("CCAPI20_Channel::DISCONNECT_B3_CONF", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    if (NCCI == GetNCCI()) {
        dassert (GetState() == cs_B_DisconnectPending);
        if (CapiInfoIsSuccess (Info)) {
            return;
        }
        dwarning (0);
    } else {
        dwarning (0);
    }
    DISCONNECT_REQ (GetNextMsgNr(), GetPLCIFromNCCI (NCCI));
}

void CCAPI20_Channel::DISCONNECT_CONF (c_word, c_dword PLCI, c_info Info) {
    dhead ("CCAPI20_Channel::DISCONNECT_CONF", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    if (PLCI == GetPLCI()) {
        if (CapiInfoIsSuccess (Info)) {
            dassert (GetState() == cs_D_DisconnectPending);
        } else {
            dprintErr ("Info=%x ", Info);
            dassert (hStateChangeTimer != vIllegalHandle);
            SecTimer_Cancel (hStateChangeTimer);
            Release();
        }
    }
}

void CCAPI20_Channel::LISTEN_CONF (c_word, c_dword Controller, c_info Info) {
    dhead ("CCAPI20_Channel::LISTEN_CONF", DCON_CCAPI20_Channel);
    ListenConf (Controller, Info);
}

void CCAPI20_Channel::SELECT_B_PROTOCOL_CONF (c_word, c_dword PLCI, c_info Info) {
    dhead ("CCAPI20_Channel::SELECT_B_PROTOCOL_CONF", DCON_CCAPI20_Channel);
    dwarning (PLCI == GetPLCI());
    if (PLCI == GetPLCI() && (SetState (cs_ProtocolPending, cs_D_Connected) == vTrue)) {
        SelectBProtocolConf (Info);
        if (GetState() == cs_D_Connected) {
            ConnectB3Req();
        }
    }
}

void CCAPI20_Channel::MANUFACTURER_CONF (c_word, c_dword Controller, cm_manufacturer *Params) {
    dhead ("CCAPI20_Channel::MANUFACTURER_CONF", DCON_CCAPI20_Channel);
    ManufacturerConf (Controller, Params);
}


/*===========================================================================*\
    CCAPI20_Channel - Ind-Messages
\*===========================================================================*/

tBool CCAPI20_Channel::CONNECT_IND (c_word, c_dword PLCI, cm_connectind *Params) {
    dhead ("CCAPI20_Channel::CONNECT_IND", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    if (SetState (cs_D_ConnectIndication, cs_Registered) == vFalse) {
        RETURN ('x', vFalse);
    }
    dassert (GetController() == IllegalController);
    dassert (GetPLCI() == IllegalPLCI);
    dassert (GetNCCI() == IllegalNCCI);
    SetPLCI (PLCI);
    SetController (GetControllerFromPLCI (PLCI));
    ConnectInd (GetController(), Params);
    RETURN ('x', vTrue);
}

tBool CCAPI20_Channel::CONNECT_ACTIVE_IND (c_word, c_dword PLCI, cm_connectactiveind *Params) {
    dhead ("CCAPI20_Channel::CONNECT_ACTIVE_IND", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    dassert (Params != 0);
    if ((PLCI != GetPLCI()) || (SetState (cs_D_Connected, cs_D_ConnectPending) == vFalse)) {
        RETURN ('x', vFalse);
    }
    dassert (hStateChangeTimer != vIllegalHandle);
    CONNECT_ACTIVE_RESP (GetNextMsgNr(), PLCI);
    dassert (hStateChangeTimer != vIllegalHandle);
    SecTimer_Call (hStateChangeTimer, TIMER_BCHANNEL_WAITTIME, StateChangeTimer_CallBack,
                   this, (void *)TIMER_FOR_BCHANNEL);
    ConnectActiveInd (Params->pConnectedNumber);
    if (GetState() != cs_D_Connected) {
        SetInitiator (vFalse);
    } else if (GetInitiator() == vTrue) {
        SetInitiator (vFalse);
        ConnectB3Req();
    }
    RETURN ('x', vTrue);
}

tBool CCAPI20_Channel::CONNECT_B3_IND (c_word, c_dword NCCI, CCStruct *) {
    dhead ("CCAPI20_Channel::CONNECT_B3_IND", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    if ((GetPLCIFromNCCI (NCCI) != GetPLCI()) || (SetState (cs_B_ConnectPending, cs_D_Connected) == vFalse)) {
        RETURN ('x', vFalse);
    }
    dassert (GetNCCI() == IllegalNCCI);
    dassert (GetInitiator() == vFalse);
    SetNCCI (NCCI);
    cm_connectb3resp ConnB3Params (creject_Accept, &m_NCPIout);
    CONNECT_B3_RESP (GetNextMsgNr(), GetNCCI(), &ConnB3Params);
    RETURN ('x', vTrue);
}

tBool CCAPI20_Channel::CONNECT_B3_ACTIVE_IND (c_word, c_dword NCCI, CCStruct *pNCPI) {
    dhead ("CCAPI20_Channel::CONNECT_B3_ACTIVE_IND", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    if ((NCCI != GetNCCI()) || (SetState (cs_Connected, cs_B_ConnectPending) == vFalse)) {
        RETURN ('x', vFalse);
    }
    dassert (GetPLCI() != IllegalPLCI);
    dwarning (GetInitiator() == vFalse);
    CONNECT_B3_ACTIVE_RESP (GetNextMsgNr(), NCCI);
    dassert (hStateChangeTimer != vIllegalHandle);
    SecTimer_Cancel (hStateChangeTimer);
    ConnectB3ActiveInd (pNCPI);
    RETURN ('x', vTrue);
}

tBool CCAPI20_Channel::DATA_B3_IND (c_word, c_dword NCCI, cm_datab3 *Params) {
    dhead ("CCAPI20_Channel::DATA_B3_IND", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    if (NCCI != GetNCCI()) {
        RETURN ('x', vFalse);
    }
    dwarning (GetState() == cs_Connected);
    DataB3Ind (Params);
    RETURN ('x', vTrue);
}

tBool CCAPI20_Channel::DISCONNECT_B3_IND (c_word, c_dword NCCI, cm_disconnectb3ind *Params) {
    dhead ("CCAPI20_Channel::DISCONNECT_B3_IND", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    if (NCCI != GetNCCI()) {
        RETURN ('x', vFalse);
    }
    dassert ((GetState() == cs_B_ConnectPending)    || (GetState() == cs_Connected) ||
             (GetState() == cs_B_DisconnectPending) || (GetState() == cs_D_DisconnectPending));
    dassert (GetPLCI() == GetPLCIFromNCCI (NCCI));

    m_Protect.BeginWrite();
    if (curState != cs_D_DisconnectPending) {
        curState = cs_B_DisconnectRespPending;
    }
    m_Protect.EndWrite();
    DisconnectB3Ind (Params);
    RETURN ('x', vTrue);
}

tBool CCAPI20_Channel::DISCONNECT_IND (c_word, c_dword PLCI, c_info Reason) {
    dhead ("CCAPI20_Channel::DISCONNECT_IND", DCON_CCAPI20_Channel);
    dparams ("s=%x", GetState());
    if (PLCI != GetPLCI()) {
        RETURN ('x', vFalse);
    }
    dwarning (GetNCCI() == IllegalNCCI);
    dassert (GetState() > cs_Registered);
    dassert (hStateChangeTimer != vIllegalHandle);
    SecTimer_Cancel (hStateChangeTimer);
    if (SetState (cs_D_DisconnectRespPending, cs_B_DisconnectRespPending) == vFalse) {
        DISCONNECT_RESP (GetNextMsgNr(), PLCI);
        ResetValues();
        m_Protect.BeginWrite();
        curState = cs_Registered;
        m_Protect.EndWrite();
        DisconnectInd (Reason);
    }
    RETURN ('x', vTrue);
}

tBool CCAPI20_Channel::INFO_IND (c_word, c_dword Address, cm_infoind *Params) {
    dhead ("CCAPI20_Channel::INFO_IND", DCON_CCAPI20_Channel);
    dassert (Params != 0);
    // TODO: Handle InfoInd for Controller-Addresses
    if (Address != GetPLCI()) {
        RETURN ('x', vFalse);
    }
    INFO_RESP (GetNextMsgNr(), GetControllerFromPLCI (Address));
    InfoInd (Params);
    RETURN ('x', vTrue);
}

tBool CCAPI20_Channel::MANUFACTURER_IND (c_word Messagenumber, c_dword Controller, cm_manufacturer *Params) {
    dhead ("CCAPI20_Channel::MANUFACTURER_IND", DCON_CCAPI20_Channel);
    CCAPI20_Layer::MANUFACTURER_IND (Messagenumber, Controller, Params); // Auto-Answer before ManufacturerInd-call
    ManufacturerInd (Controller, Params);
    return vTrue;
}


/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Channel::ResetValues (void) {
    dhead ("CCAPI20_Channel::ResetValues", DCON_CCAPI20_Channel);
    m_Protect.BeginWrite();
    FreePLCIEntry (GetPLCI());
    SetPLCI (IllegalPLCI);
    SetNCCI (IllegalNCCI);
    SetController (IllegalController);
    SetInitiator (vFalse);
    m_Protect.EndWrite();
}


/*===========================================================================*\
\*===========================================================================*/
