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

#ifndef _CAPICHAN_H_
#define _CAPICHAN_H_

#include "CapiLay.h"
#include "SecTimer.h"
#include "ProtClas.h"

/*===========================================================================*\
\*===========================================================================*/

class CCAPI20_Channel : protected CCAPI20_Layer {
public:
    CCAPI20_Channel (void);
    virtual ~CCAPI20_Channel (void);
    void SetRingingTimeout (tUInt secs);    // secs == 0 no timeout, wait-time after ConnectConf

    tCapiState GetState (void);
    c_dword GetController (void);
    c_dword GetPLCI (void);
    c_dword GetNCCI (void);

    void SetNCPI (CCStruct *pNCPI);
    tBool AcceptIncomingCall (void);
    // AcceptIncommingCall have to be called before AlertReq and ConnectResp
    // return:  vTrue  - allow to accept incoming call
    //          vFalse - don't accept incoming call

    /*-----------------------------------------------------------------------*\
        Anmeldung
    \*-----------------------------------------------------------------------*/
    void Register (tUInt MaxDataSize = CAPI_MAX_DATA_SIZE, tUInt MaxInFrames = CAPI_MAX_OUTSTANDING_FRAMES);
    void Release (void);
    virtual void Register_Complete (c_info ErrorCode) = 0;
    virtual void Release_Complete (void) = 0;

    /*-----------------------------------------------------------------------*\
        CAPI - Requests
    \*-----------------------------------------------------------------------*/
    void  AlertReq           (CCStruct *pAdditionalInfo);
    tBool ConnectReq         (c_dword Controller, cm_connectreq *Params);
    void  DataB3Req          (cm_datab3 *Params);
    void  DisconnectReq      (tBool FastDisconnect = vFalse);    // Fast == Disconnect directly the D-Channel
    void  ListenReq          (c_dword Controller, cm_listenreq *Params);
    void  ManufacturerReq    (c_dword Controller, cm_manufacturer *Params);
    void  SelectBProtocolReq (cp_bprotocol *Params);

    /*-----------------------------------------------------------------------*\
        virtual CAPI - Confirmations
    \*-----------------------------------------------------------------------*/
    virtual void ConnectConf         (void) {}
    virtual void DataB3Conf          (cm_datab3conf * /*Params*/) {}
    virtual void ListenConf          (c_dword /*Controller*/, c_info /*Info*/) {}
    virtual void ManufacturerConf    (c_dword /*Controller*/, cm_manufacturer * /*Params*/) {}
    virtual void SelectBProtocolConf (c_info /*Info*/) {}

    /*-----------------------------------------------------------------------*\
        virtual CAPI - Indications
    \*-----------------------------------------------------------------------*/
    virtual void ConnectInd         (c_dword /*Controller*/, cm_connectind * /*Params*/) {}
    virtual void ConnectActiveInd   (CCStruct * /*pConnectedNumber*/) {}
    virtual void ConnectB3ActiveInd (CCStruct * /*pNCPI*/) {}
    virtual void DataB3Ind          (cm_datab3 * /*Params*/) {}
    virtual void DisconnectB3Ind    (cm_disconnectb3ind * /*Params*/) {}
    virtual void DisconnectInd      (c_info /*Reason*/) {}
    virtual void InfoInd            (cm_infoind * /*Params*/) {}
    virtual void ManufacturerInd    (c_dword /*Controller*/, cm_manufacturer * /*Params*/) {}

    /*-----------------------------------------------------------------------*\
        CAPI - Responses
    \*-----------------------------------------------------------------------*/
    void ConnectResp      (cm_connectresp *Params);
    void DataB3Resp       (c_word DataHandle);
    void DisconnectB3Resp (void);

protected:
    void ConnectB3Req (void);

    tBool SetState (tCapiState newState, tCapiState oldState);
    void  SetController (c_dword cntrl);
    void  SetPLCI (c_dword PLCI);
    void  SetNCCI (c_dword NCCI);
    void  SetInitiator (tBool activate);
    tBool GetInitiator (void);

    CProtectClass *GetProtectClass (void);

private:
    /*-----------------------------------------------------------------------*\
        virtuelle Funktionen der CCAPI20_ChannelForManager
    \*-----------------------------------------------------------------------*/
    void REGISTER_COMPLETE (c_info ErrorCode);
    void RELEASE_COMPLETE (void);

    void CONNECT_CONF (c_word Messagenumber, c_dword PLCI, c_info Info);
    void CONNECT_B3_CONF (c_word Messagenumber, c_dword NCCI, c_info Info);
    void DATA_B3_CONF (c_word Messagenumber, c_dword NCCI, cm_datab3conf *Params);
    void DISCONNECT_B3_CONF (c_word Messagenumber, c_dword NCCI, c_info Info);
    void DISCONNECT_CONF (c_word Messagenumber, c_dword PLCI, c_info Info);
    void LISTEN_CONF (c_word Messagenumber, c_dword Controller, c_info Info);
    void MANUFACTURER_CONF (c_word Messagenumber, c_dword Controller, cm_manufacturer *Params);
    void SELECT_B_PROTOCOL_CONF (c_word Messagenumber, c_dword PLCI, c_info Info);

    tBool CONNECT_IND (c_word Messagenumber, c_dword PLCI, cm_connectind *Params);
    tBool CONNECT_ACTIVE_IND (c_word Messagenumber, c_dword PLCI, cm_connectactiveind *Params);
    tBool CONNECT_B3_IND (c_word Messagenumber, c_dword NCCI, CCStruct *pNCPI);
    tBool CONNECT_B3_ACTIVE_IND (c_word Messagenumber, c_dword NCCI, CCStruct *pNCPI);
    tBool DATA_B3_IND (c_word Messagenumber, c_dword NCCI, cm_datab3 *Params);
    tBool DISCONNECT_B3_IND (c_word Messagenumber, c_dword NCCI, cm_disconnectb3ind *Params);
    tBool DISCONNECT_IND (c_word Messagenumber, c_dword PLCI, c_info Reason);
    tBool INFO_IND (c_word Messagenumber, c_dword Address, cm_infoind *Params);
    tBool MANUFACTURER_IND (c_word Messagenumber, c_dword Controller, cm_manufacturer *Params);

    /*-----------------------------------------------------------------------*\
       interne Funktionen & Variablen
    \*-----------------------------------------------------------------------*/
    void ResetValues (void);

    CProtectClass m_Protect;
    thSecTimer    hStateChangeTimer;
    tCapiState    curState;
    c_dword       controller;
    c_dword       plci;
    c_dword       ncci;
    CCStructStore m_NCPIout;
    tUInt         RingingTimeout;
    tBool         Initiator;
};

/*===========================================================================*\
    inline Funktionen
\*===========================================================================*/

inline void CCAPI20_Channel::SetRingingTimeout (tUInt secs) {
    RingingTimeout = secs;
}

inline tCapiState CCAPI20_Channel::GetState (void) {
    return curState;
}

inline c_dword CCAPI20_Channel::GetController (void) {
    return controller;
}

inline c_dword CCAPI20_Channel::GetPLCI (void) {
    return plci;
}

inline c_dword CCAPI20_Channel::GetNCCI (void) {
    return ncci;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void CCAPI20_Channel::AlertReq (CCStruct *pAdditionalInfo) {
    if (GetState() == cs_D_ConnectIndication) {
        // assert (plci != IllegalPLCI);
        ALERT_REQ (GetNextMsgNr(), GetPLCI(), pAdditionalInfo);
    }
}

inline void CCAPI20_Channel::ListenReq (c_dword Controller, cm_listenreq *Params) {
    LISTEN_REQ (GetNextMsgNr(), Controller, Params);
}

inline void CCAPI20_Channel::ManufacturerReq (c_dword Controller, cm_manufacturer *Params) {
    MANUFACTURER_REQ (GetNextMsgNr(), Controller, Params);
}

inline void CCAPI20_Channel::SelectBProtocolReq (cp_bprotocol *Params) {
    // assert (plci != IllegalPLCI);
    // assert (GetState() == cs_D_Connected);
    if (SetState (cs_ProtocolPending, cs_D_Connected) == vTrue) {
        SELECT_B_PROTOCOL_REQ (GetNextMsgNr(), GetPLCI(), Params);
    } else {
        SelectBProtocolConf (ci_Int_IllegalState);
    }
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void CCAPI20_Channel::ConnectB3Req (void) {
    CONNECT_B3_REQ (GetNextMsgNr(), GetPLCI(), &m_NCPIout);
}

//inline void CCAPI20_Channel::DisconnectB3Req (void) {
//    DISCONNECT_B3_REQ (GetNextMsgNr(), GetNCCI(), &m_NCPIout);
//}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CCAPI20_Channel::SetState (tCapiState newState, tCapiState oldState) {
    return m_Protect.TestAndSet (&curState, oldState, newState);
}

inline void CCAPI20_Channel::SetController (c_dword cntrl) {
    controller = cntrl;
}

inline void CCAPI20_Channel::SetPLCI (c_dword PLCI) {
    plci = PLCI;
}

inline void CCAPI20_Channel::SetNCCI (c_dword NCCI) {
    ncci = NCCI;
}

inline void CCAPI20_Channel::SetInitiator (tBool activate) {
    Initiator = activate;
}

inline tBool CCAPI20_Channel::GetInitiator (void) {
    return Initiator;
}

inline CProtectClass *CCAPI20_Channel::GetProtectClass (void) {
    return &m_Protect;
}

/*===========================================================================*\
\*===========================================================================*/

#endif
