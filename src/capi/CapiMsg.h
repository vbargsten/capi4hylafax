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

#ifndef _CAPIMSG_H_
#define _CAPIMSG_H_

#include "CapiBase.h"
#include "c2msgdef.h"
#include "DynStore.h"
#include "ProtClas.h"
#include <stdarg.h>

/*===========================================================================*\
\*===========================================================================*/

class CCAPI20_MsgBase : private CCallBackCapiBase {
public:
    CCAPI20_MsgBase (void);
    virtual ~CCAPI20_MsgBase (void);

    c_word GetApplID (void);

    /*-----------------------------------------------------------------------*\
        CAPI - REGISTER, RELEASE, GET_PROFILE
    \*-----------------------------------------------------------------------*/
    void REGISTER (tUInt MaxB3Connection, tUInt MaxB3Blks, tUInt MaxSizeB3);
    void RELEASE  (void);

    void GET_PROFILE (tUInt Controller);

    virtual void REGISTER_COMPLETE (c_info ErrorCode) = 0;
    virtual void RELEASE_COMPLETE (void) = 0;

    virtual void GET_PROFILE_COMPLETE (tUInt /*Controller*/, cm_getprofile * /*pProfile*/, c_info /*ErrorCode*/) {}

    /*-----------------------------------------------------------------------*\
        CAPI - Requests
    \*-----------------------------------------------------------------------*/
    void ALERT_REQ                         (c_word    Messagenumber,
                                            c_dword   PLCI,
                                            CCStruct *pAdditionalInfo);

    void CONNECT_REQ                       (c_word         Messagenumber,
                                            c_dword        Controller,
                                            cm_connectreq *Params);

    void CONNECT_B3_REQ                    (c_word    Messagenumber,
                                            c_dword   PLCI,
                                            CCStruct *pNCPI);

    void DATA_B3_REQ                       (c_word     Messagenumber,
                                            c_dword    NCCI,
                                            cm_datab3 *Params);

    void DISCONNECT_B3_REQ                 (c_word    Messagenumber,
                                            c_dword   NCCI,
                                            CCStruct *pNCPI);

    void DISCONNECT_REQ                    (c_word   Messagenumber,
                                            c_dword  PLCI);

    void FACILITY_REQ                      (c_word          Messagenumber,
                                            c_dword         Address,
                                            cm_facilityreq *Params);

    void INFO_REQ                          (c_word      Messagenumber,
                                            c_dword     Address,
                                            cm_inforeq *Params);

    void LISTEN_REQ                        (c_word        Messagenumber,
                                            c_dword       Controller,
                                            cm_listenreq *Params);

    void MANUFACTURER_REQ                  (c_word           Messagenumber,
                                            c_dword          Controller,
                                            cm_manufacturer *Params);

    void RESET_B3_REQ                      (c_word    Messagenumber,
                                            c_dword   NCCI,
                                            CCStruct *pNCPI);

    void SELECT_B_PROTOCOL_REQ             (c_word        Messagenumber,
                                            c_dword       PLCI,
                                            cp_bprotocol *Params);

    /*-----------------------------------------------------------------------*\
        virtual CAPI - Confirmation
    \*-----------------------------------------------------------------------*/

    virtual void ALERT_CONF                (c_word  /*Messagenumber*/,
                                            c_dword /*PLCI*/,
                                            c_info  /*Info*/) {}

    virtual void CONNECT_CONF              (c_word  /*Messagenumber*/,
                                            c_dword /*PLCI*/,
                                            c_info  /*Info*/) {}

    virtual void CONNECT_B3_CONF           (c_word  /*Messagenumber*/,
                                            c_dword /*NCCI*/,
                                            c_info  /*Info*/) {}

    virtual void DATA_B3_CONF              (c_word          /*Messagenumber*/,
                                            c_dword         /*NCCI*/,
                                            cm_datab3conf * /*Params*/) {}

    virtual void DISCONNECT_B3_CONF        (c_word  /*Messagenumber*/,
                                            c_dword /*NCCI*/,
                                            c_info  /*Info*/) {}

    virtual void DISCONNECT_CONF           (c_word  /*Messagenumber*/,
                                            c_dword /*PLCI*/,
                                            c_info  /*Info*/) {}

    virtual void FACILITY_CONF             (c_word            /*Messagenumber*/,
                                            c_dword           /*Address*/,
                                            cm_facilityconf * /*Params*/) {}

    virtual void INFO_CONF                 (c_word  /*Messagenumber*/,
                                            c_dword /*Address*/,
                                            c_info  /*Info*/) {}

    virtual void LISTEN_CONF               (c_word  /*Messagenumber*/,
                                            c_dword /*Controller*/,
                                            c_info  /*Info*/) {}

    virtual void MANUFACTURER_CONF         (c_word            /*Messagenumber*/,
                                            c_dword           /*Controller*/,
                                            cm_manufacturer * /*Params*/) {}

    virtual void RESET_B3_CONF             (c_word  /*Messagenumber*/,
                                            c_dword /*NCCI*/,
                                            c_info  /*Info*/) {}

    virtual void SELECT_B_PROTOCOL_CONF    (c_word  /*Messagenumber*/,
                                            c_dword /*PLCI*/,
                                            c_info  /*Info*/) {}

    /*-----------------------------------------------------------------------*\
        virtual CAPI - Indications
    \*-----------------------------------------------------------------------*/

    virtual tBool CONNECT_IND               (c_word         Messagenumber,
                                             c_dword        PLCI,
                                             cm_connectind *Params);

    virtual tBool CONNECT_ACTIVE_IND        (c_word               Messagenumber,
                                             c_dword              PLCI,
                                             cm_connectactiveind *Params);

    virtual tBool CONNECT_B3_IND            (c_word    Messagenumber,
                                             c_dword   NCCI,
                                             CCStruct *pNCPI);

    virtual tBool CONNECT_B3_ACTIVE_IND     (c_word    Messagenumber,
                                             c_dword   NCCI,
                                             CCStruct *pNCPI);

    virtual tBool CONNECT_B3_T90_ACTIVE_IND (c_word   Messagenumber,
                                             c_dword  NCCI);

    virtual tBool DATA_B3_IND               (c_word     Messagenumber,
                                             c_dword    NCCI,
                                             cm_datab3 *Params);

    virtual tBool DISCONNECT_B3_IND         (c_word              Messagenumber,
                                             c_dword             NCCI,
                                             cm_disconnectb3ind *Params);

    virtual tBool DISCONNECT_IND            (c_word  Messagenumber,
                                             c_dword PLCI,
                                             c_info  Reason);

    virtual tBool FACILITY_IND              (c_word          Messagenumber,
                                             c_dword         Address,
                                             cm_facilityind *Params);

    virtual tBool INFO_IND                  (c_word      Messagenumber,
                                             c_dword     Address,
                                             cm_infoind *Params);

    virtual tBool MANUFACTURER_IND          (c_word           Messagenumber,
                                             c_dword          Controller,
                                             cm_manufacturer *Params);

    virtual tBool RESET_B3_IND              (c_word    Messagenumber,
                                             c_dword   NCCI,
                                             CCStruct *pNCPI);

    /*-----------------------------------------------------------------------*\
        CAPI - Responses
    \*-----------------------------------------------------------------------*/
    void CONNECT_RESP                      (c_word          Messagenumber,
                                            c_dword         PLCI,
                                            cm_connectresp *Params);

    void CONNECT_ACTIVE_RESP               (c_word  Messagenumber,
                                            c_dword PLCI);

    void CONNECT_B3_RESP                   (c_word            Messagenumber,
                                            c_dword           NCCI,
                                            cm_connectb3resp *Params);

    void CONNECT_B3_ACTIVE_RESP            (c_word  Messagenumber,
                                            c_dword NCCI);

    void CONNECT_B3_T90_ACTIVE_RESP        (c_word  Messagenumber,
                                            c_dword NCCI);

    void DATA_B3_RESP                      (c_word         Messagenumber,
                                            c_dword        NCCI,
                                            cm_datab3resp *Params);

    void DISCONNECT_B3_RESP                (c_word  Messagenumber,
                                            c_dword NCCI);

    void DISCONNECT_RESP                   (c_word  Messagenumber,
                                            c_dword PLCI);

    void FACILITY_RESP                     (c_word           Messagenumber,
                                            c_dword          Address,
                                            cm_facilityresp *Params);

    void INFO_RESP                         (c_word  Messagenumber,
                                            c_dword Address);

    void MANUFACTURER_RESP                 (c_word           Messagenumber,
                                            c_dword          Controller,
                                            cm_manufacturer *Params);

    void RESET_B3_RESP                     (c_word  Messagenumber,
                                            c_dword NCCI);

    /*-----------------------------------------------------------------------*\
        Error Handling
    \*-----------------------------------------------------------------------*/
    virtual void HandleMessageError (tUInt type, c_info ErrorCode, c_message cmsg);
    // type: 0 = PutMessage, 1 = GetMessage, 2 = GetProfile
    // cmsg: Nur bei PutMessage != 0 !!

private:
    /*-----------------------------------------------------------------------*\
        Virtuelle Funktionen von CCAPI20_Base
    \*-----------------------------------------------------------------------*/
    void RegisterComplete (thCapiBase hBase, c_info ErrorCode);
    void ReleaseComplete (void);
    void PutMessageComplete (c_info ErrorCode);
    void GetMessageReceived (c_byte *pReturnMessage, c_info ErrorCode);
    void GetProfileComplete (tUInt Controller, cm_getprofile *pProfile, c_info ErrorCode);

    /*-----------------------------------------------------------------------*\
        Interne Funtkionen
    \*-----------------------------------------------------------------------*/
    void AllocAndPutMsg (c_byte Command, c_byte SubCommand, c_word Messagenumber,
                         c_dword Address, tBool ZeroIsEmptyStruct, CCStruct *Params);
    void FreeCurPutMsg (void);
    void EmptyPutMsgbufferQueue (void);
    void HandleGetMessage (c_byte *Message);

    /*-----------------------------------------------------------------------*\
        Interne Klassen und Variablen
    \*-----------------------------------------------------------------------*/
    CDynStoreQueue PutMsgbufferQueue;

    CProtectClass m_Protect;
    thCapiBase    m_hCapiBase;
    c_message     m_CurPutMessage;
#ifndef NDEBUG
    tUInt         m_RegisterState;
#endif
};


/*===========================================================================*\
    inline Funktionen der Klasse CCAPI20_MsgBase
\*===========================================================================*/

inline c_word CCAPI20_MsgBase::GetApplID (void) {
    return ApplID();
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void CCAPI20_MsgBase::GET_PROFILE (tUInt Controller) {
    //dassert (m_hCapiBase != vIllegalHandle);
    CapiBase_GetProfile (m_hCapiBase, Controller);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void CCAPI20_MsgBase::GetProfileComplete (tUInt Controller, cm_getprofile *pProfile, c_info ErrorCode) {
    if (ErrorCode != ci_NoError) {
        //dassert (ErrorCode != ci_Msg_Busy);
        //dassert (ErrorCode != ci_Msg_SendQueueFull);
        // TODO: H.M.E.-Call with Controller & pProfile Info
        HandleMessageError (2, ErrorCode, 0);
    }
    GET_PROFILE_COMPLETE (Controller, pProfile, ErrorCode);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void CCAPI20_MsgBase::FreeCurPutMsg (void) {
    m_Protect.BeginWrite();
    c_byte *toDel   = m_CurPutMessage;
    m_CurPutMessage = 0;
    m_Protect.EndWrite();
    PutMsgbufferQueue.Free (toDel);
}


/*===========================================================================*\
    CCAPI20_MsgBase: Fast alle CAPI - Requests
\*===========================================================================*/

inline void CCAPI20_MsgBase::ALERT_REQ (c_word Messagenumber, c_dword PLCI, CCStruct *pAdditionalInfo) {
    AllocAndPutMsg (CAPI_ALERT, CAPI_REQ, Messagenumber, PLCI, vTrue, pAdditionalInfo);
}

inline void CCAPI20_MsgBase::CONNECT_REQ (c_word Messagenumber, c_dword Controller, cm_connectreq *Params) {
    AllocAndPutMsg (CAPI_CONNECT, CAPI_REQ, Messagenumber, Controller, vTrue, Params);
}

inline void CCAPI20_MsgBase::CONNECT_RESP (c_word Messagenumber, c_dword PLCI, cm_connectresp *Params) {
    AllocAndPutMsg (CAPI_CONNECT, CAPI_RESP, Messagenumber, PLCI, vTrue, Params);
}


inline void CCAPI20_MsgBase::CONNECT_B3_REQ (c_word Messagenumber, c_dword PLCI, CCStruct *pNCPI) {
    AllocAndPutMsg (CAPI_CONNECT_B3, CAPI_REQ, Messagenumber, PLCI, vTrue, pNCPI);
}

inline void CCAPI20_MsgBase::DATA_B3_REQ (c_word Messagenumber, c_dword NCCI, cm_datab3 *Params) {
    AllocAndPutMsg (CAPI_DATA_B3, CAPI_REQ, Messagenumber, NCCI, vTrue, Params);
}

inline void CCAPI20_MsgBase::DISCONNECT_B3_REQ (c_word Messagenumber, c_dword NCCI, CCStruct *pNCPI) {
    AllocAndPutMsg (CAPI_DISCONNECT_B3, CAPI_REQ, Messagenumber, NCCI, vTrue, pNCPI);
}

inline void CCAPI20_MsgBase::DISCONNECT_REQ (c_word Messagenumber, c_dword PLCI) {
    AllocAndPutMsg (CAPI_DISCONNECT, CAPI_REQ, Messagenumber, PLCI, vTrue, 0);
}

inline void CCAPI20_MsgBase::FACILITY_REQ (c_word Messagenumber, c_dword Address, cm_facilityreq *Params) {
    AllocAndPutMsg (CAPI_FACILITY, CAPI_REQ, Messagenumber, Address, vTrue, Params);
}

inline void CCAPI20_MsgBase::INFO_REQ (c_word Messagenumber, c_dword Address, cm_inforeq *Params) {
    AllocAndPutMsg (CAPI_INFO, CAPI_REQ, Messagenumber, Address, vTrue, Params);
}

inline void CCAPI20_MsgBase::LISTEN_REQ (c_word Messagenumber, c_dword Controller, cm_listenreq *Params) {
    AllocAndPutMsg (CAPI_LISTEN, CAPI_REQ, Messagenumber, Controller, vTrue, Params);
}

inline void CCAPI20_MsgBase::MANUFACTURER_REQ (c_word Messagenumber, c_dword Controller, cm_manufacturer *Params) {
    AllocAndPutMsg (CAPI_MANUFACTURER, CAPI_REQ, Messagenumber, Controller, vTrue, Params);
}

inline void CCAPI20_MsgBase::RESET_B3_REQ (c_word Messagenumber, c_dword NCCI, CCStruct *pNCPI) {
    AllocAndPutMsg (CAPI_RESET_B3, CAPI_REQ, Messagenumber, NCCI, vTrue, pNCPI);
}

inline void CCAPI20_MsgBase::SELECT_B_PROTOCOL_REQ (c_word Messagenumber, c_dword PLCI, cp_bprotocol *Params) {
    AllocAndPutMsg (CAPI_SELECT_B_PROTOCOL, CAPI_REQ, Messagenumber, PLCI, vTrue, Params);
}


/*===========================================================================*\
    CCAPI20_MsgBase: Alle CAPI - Indications
\*===========================================================================*/

inline tBool CCAPI20_MsgBase::CONNECT_IND (c_word Messagenumber, c_dword PLCI, cm_connectind *) {
    cm_connectresp crIgnore;
    CONNECT_RESP (Messagenumber, PLCI, &crIgnore);
    return vTrue;
}

inline tBool CCAPI20_MsgBase::CONNECT_ACTIVE_IND (c_word Messagenumber, c_dword PLCI, cm_connectactiveind *) {
    CONNECT_ACTIVE_RESP (Messagenumber, PLCI);
    DISCONNECT_REQ (0, PLCI);
    return vTrue;
}

inline tBool CCAPI20_MsgBase::CONNECT_B3_IND (c_word Messagenumber, c_dword NCCI, CCStruct *) {
    cm_connectb3resp cb3r;
    CONNECT_B3_RESP (Messagenumber, NCCI, &cb3r);
    return vTrue;
}

inline tBool CCAPI20_MsgBase::CONNECT_B3_ACTIVE_IND (c_word Messagenumber, c_dword NCCI, CCStruct *) {
    CONNECT_B3_ACTIVE_RESP (Messagenumber, NCCI);
    DISCONNECT_B3_REQ (0, NCCI, 0);
    return vTrue;
}

inline tBool CCAPI20_MsgBase::CONNECT_B3_T90_ACTIVE_IND (c_word Messagenumber, c_dword NCCI) {
    CONNECT_B3_T90_ACTIVE_RESP (Messagenumber, NCCI);
    return vTrue;
}

inline tBool CCAPI20_MsgBase::DATA_B3_IND (c_word Messagenumber, c_dword NCCI, cm_datab3 *Params) {
    // dassert (Params != 0);
    cm_datab3resp db3r (Params->DataHandle);
    DATA_B3_RESP (Messagenumber, NCCI, &db3r);
    return vTrue;
}

inline tBool CCAPI20_MsgBase::DISCONNECT_B3_IND (c_word Messagenumber, c_dword NCCI, cm_disconnectb3ind *) {
    DISCONNECT_B3_RESP (Messagenumber, NCCI);
    return vTrue;
}

inline tBool CCAPI20_MsgBase::DISCONNECT_IND (c_word Messagenumber, c_dword PLCI, c_info) {
    DISCONNECT_RESP (Messagenumber, PLCI);
    return vTrue;
}

inline tBool CCAPI20_MsgBase::FACILITY_IND (c_word Messagenumber, c_dword Address, cm_facilityind *Params) {
    // dassert (Params != 0);
    cm_facilityresp RespParams (Params->FacilitySelector, 0);
    FACILITY_RESP (Messagenumber, Address, &RespParams);
    return vTrue;
}

inline tBool CCAPI20_MsgBase::INFO_IND (c_word Messagenumber, c_dword Address, cm_infoind *) {
    INFO_RESP (Messagenumber, Address);
    return vTrue;
}

inline tBool CCAPI20_MsgBase::MANUFACTURER_IND (c_word Messagenumber, c_dword Controller, cm_manufacturer *Params) {
    MANUFACTURER_RESP (Messagenumber, Controller, Params);
    return vTrue;
}

inline tBool CCAPI20_MsgBase::RESET_B3_IND (c_word Messagenumber, c_dword NCCI, CCStruct *) {
    RESET_B3_RESP (Messagenumber, NCCI);
    return vTrue;
}


/*===========================================================================*\
    CCAPI20_MsgBase: Nearly All CAPI - Responses
\*===========================================================================*/

inline void CCAPI20_MsgBase::CONNECT_ACTIVE_RESP (c_word Messagenumber, c_dword PLCI) {
    AllocAndPutMsg (CAPI_CONNECT_ACTIVE, CAPI_RESP, Messagenumber, PLCI, vFalse, 0);
}

inline void CCAPI20_MsgBase::CONNECT_B3_RESP (c_word Messagenumber, c_dword NCCI, cm_connectb3resp *Params) {
    AllocAndPutMsg (CAPI_CONNECT_B3, CAPI_RESP, Messagenumber, NCCI, vTrue, Params);
}

inline void CCAPI20_MsgBase::CONNECT_B3_ACTIVE_RESP (c_word Messagenumber, c_dword NCCI) {
    AllocAndPutMsg (CAPI_CONNECT_B3_ACTIVE, CAPI_RESP, Messagenumber, NCCI, vFalse, 0);
}

inline void CCAPI20_MsgBase::CONNECT_B3_T90_ACTIVE_RESP (c_word Messagenumber, c_dword NCCI) {
    AllocAndPutMsg (CAPI_CONNECT_B3_T90_ACTIVE, CAPI_RESP, Messagenumber, NCCI, vFalse, 0);
}

inline void CCAPI20_MsgBase::DATA_B3_RESP (c_word Messagenumber, c_dword NCCI, cm_datab3resp *Params) {
    AllocAndPutMsg (CAPI_DATA_B3, CAPI_RESP, Messagenumber, NCCI, vTrue, Params);
}

inline void CCAPI20_MsgBase::DISCONNECT_B3_RESP (c_word Messagenumber, c_dword NCCI) {
    AllocAndPutMsg (CAPI_DISCONNECT_B3, CAPI_RESP, Messagenumber, NCCI, vFalse, 0);
}

inline void CCAPI20_MsgBase::DISCONNECT_RESP (c_word Messagenumber, c_dword PLCI) {
    AllocAndPutMsg (CAPI_DISCONNECT, CAPI_RESP, Messagenumber, PLCI, vFalse, 0);
}

inline void CCAPI20_MsgBase::FACILITY_RESP (c_word Messagenumber, c_dword Address, cm_facilityresp *Params) {
    AllocAndPutMsg (CAPI_FACILITY, CAPI_RESP, Messagenumber, Address, vTrue, Params);
}

inline void CCAPI20_MsgBase::INFO_RESP (c_word Messagenumber, c_dword Address) {
    AllocAndPutMsg (CAPI_INFO, CAPI_RESP, Messagenumber, Address, vFalse, 0);
}

inline void CCAPI20_MsgBase::MANUFACTURER_RESP (c_word Messagenumber, c_dword Controller, cm_manufacturer *Params) {
    AllocAndPutMsg (CAPI_MANUFACTURER, CAPI_RESP, Messagenumber, Controller, vTrue, Params);
}

inline void CCAPI20_MsgBase::RESET_B3_RESP (c_word Messagenumber, c_dword NCCI) {
    AllocAndPutMsg (CAPI_RESET_B3, CAPI_RESP, Messagenumber, NCCI, vFalse, 0);
}


/*===========================================================================*\
\*===========================================================================*/

#endif
