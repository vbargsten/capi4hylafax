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

#ifndef _CHANNEL_H_
#define _CHANNEL_H_

#include "Facility.h"
#include "DataHand.h"
#include "MSNList.h"
#include "aFlags.h"

/*===========================================================================*\
\*===========================================================================*/

class CTransferChannel : private CCAPI20_Facility {
public:
    enum tDisconnectKind {
        DisconnectKind_Hard = 1,    // Makes Release
        DisconnectKind_Fast,        // Makes DisconnectReq (FastDisconnect = vTrue)
        DisconnectKind_Normal,      // Makes DisconnectReq
        DisconnectKind_WaitNoPut,   // Wait for all Outstanding DATA_B3_CONF's (without new PutData's)
        DisconnectKind_WaitWithPut  // Wait for all Outstanding DATA_B3_CONF's (with new PutData's)
    };

    CTransferChannel (void);
    virtual ~CTransferChannel (void);

    tCapiState CurState (void);
    tUShort    CountPutQueueItems (void);
    tBool      IsPutQueueEmpty (void);
    tBool      IsPutQueueFull (void);
    void       SetMaxDataSize (tUInt MaxDataSize);  // MaxDataSize == 0 => CAPI_MAX_DATA_SIZE
    void       SetRingingTimeout (tUInt secs);

    void SetOutController (tUInt Controller);       // OutController is used by MakeCall and UnparkCall
    tUInt GetOutController (void);                  // ActiveController is the Controller who is currently in use
    tUInt GetActiveController (void);               // both can differ if listen is active for another Controller
    void SetOwnNumber (CConstString *pMyNumber, tUByte Indicator = CALLNUM_INDICATOR_DEFAULT);
    CConstString *GetOwnNumber (void);
    void SetB1Parameters (c_b1prot B1Protocol, cp_b1configuration *pB1Config = 0);
    void SetB2Parameters (c_b2prot B2Protocol);
    void SetB3Parameters (c_b3prot B3Protocol, cp_b3configuration *pB3Config = 0);
    void SetAdditionalInfo (tUInt setflags, CCStruct *pBChannelInfo, CCStruct *pKeypadFacility = 0, CCStruct *pUserUserData = 0,
                            CCStruct *pFacilityDataArray = 0, CCStruct *pSendingComplete = 0);
    void SetNCPI (cp_ncpi *pNCPI);

    tBool SetMSNList (CCntrlMSNList *pCntrlMSNList);
    CCntrlMSNList *GetMSNList (void);
    void CheckListen (void);
    virtual void ListenError (tUInt /*Controller*/, c_info /*Info*/) {}     // Controller == 0 => For all Controller

    tBool MakeCall (CConstString *pTelNumber, c_cipvalue CIPValue);         // vFalse => Illegal state
    void  AlertCall (void);
    void  AnswerCall (c_reject Reject = creject_Accept);
    tBool PutData (tUByte *Data, tUShort DataLength, void *hDataID);        // vFalse => QueueFull or Disconnect
    void  GetDataComplete (tUShort DataHandle);
    void  Disconnect (tDisconnectKind kind = DisconnectKind_Normal);
    void  DisconnectConfirm (void);                                         // async answer for DisconnectInquiry
    void  ResetChannel (void);
    // Call ResetChannel always before deleting this class !!!
    void  UpdateBParameters (void);

    virtual void IncomingCall (c_cipvalue CIPValue, CConstString *pOppositeNumber, CConstString *pMyCalledNumber) = 0;
    virtual void DialingBegin (void) {}                                     // (ConnectReq)
    virtual void DialingEnd (void) {}                                       // (ConnectConf)
    virtual void ConnectProceeding (CConstString * /*ConnectedNumber */) {} // (ConnectActiveInd)
    virtual void IsConnected (cp_ncpi_all *pNCPI) = 0;
    virtual void PutDataComplete (void *hDataID, c_info Info) = 0;
    virtual void GetData (tUByte *Data, tUShort DataLength, tUShort DataHandle) = 0;
    virtual void DisconnectInquiry (c_info /*ReasonB3*/, cp_ncpi_all * /*pNCPI*/); // needs DisconnectConfirm answer
    virtual void IsDisconnected (c_info Reason) = 0;
    virtual void IsReseted (void) = 0;
    virtual void UpdateBParametersComplete (c_info /*Info*/) {}

#ifdef USE_SUPPLEMENTARY_SERVICES
    void HoldCall (void);
    void UnholdCall (void);
    void ParkCall (CConstString *pIDStr);
    void UnparkCall (CConstString *pIDStr);
    void RedirectCall (CDynamicString *pTelNumber);
    void TransferCall (CTransferChannel *pChannel);                 // this is in hold, pChannel is active
    void Conference (CTransferChannel *pChannel, tBool doBegin);    // this is in hold, pChannel is active
    void DTMFListen (tBool activate);
    void DTMFSend (CConstString *pDTMFDigits, tUShort ToneDuration = 0, tUShort GapDuration = 0);

    virtual void CallHolded (c_info /*Info*/) {}
    virtual void CallUnholded (c_info /*Info*/) {}
    virtual void CallParked (c_info /*Info*/) {}
    virtual void CallUnparked (c_info /*Info*/) {}
    virtual void CallRedirected (c_info /*Info*/) {}
    virtual void CallTransfered (c_info /*Info*/) {}
    virtual void ConferenceComplete (tBool /*Begin*/, c_info /*Info*/) {}
    virtual void DTMFReceived (tUInt /*count*/, tUChar * /*pDTMFChars*/) {}
    virtual void DTMFSendComplete (c_info /*Info*/) {}
    virtual void DTMFListenError (c_info /*Info*/) {}

private:
    /*-----------------------------------------------------------------------*\
        virtual functions from CCAPI20_Facility
    \*-----------------------------------------------------------------------*/
    void fConf_DTMF (c_info Info);
    void fInd_DTMF (CCStruct *pDTMFdigits);
    void fInd_Hold (c_info Reason);
    void fInd_Retrieve (c_info Reason);
    void fInd_Suspend (c_info Reason);
    void fInd_Resume (c_info Reason);
    void fInd_CD (c_info Reason);
    void fInd_ECT (c_info Reason);
    void fInd_3PTY_Begin (c_info Reason);
    void fInd_3PTY_End (c_info Reason);
#endif

protected:
    // enhanced functionality, only needed in very special cases
    void CapiStart (void);
    void CapiStop (void);
    void GetProfile (tUInt Controller = 0);
    void ManufacturerCall (c_dword Function, CCStruct *pData);
    virtual void CapiStarted (c_info /*ErrorCode*/) {}
    virtual void GetProfileComplete (tUInt/* Controller*/, cm_getprofile * /*pProfile*/, c_info /*ErrorCode*/) {}
    virtual void ManufacturerCallComplete (c_dword /*Error*/, c_dword /*Function*/, CCStruct * /*pData*/) {}
    virtual void ManufacturerNotify (c_dword /*Function*/, CCStruct * /*pData*/) {}

private:
    /*-----------------------------------------------------------------------*\
        virtual functions from CCAPI20_Channel
    \*-----------------------------------------------------------------------*/
    void Register_Complete (c_info ErrorCode);
    void Release_Complete (void);
    void ConnectConf (void);
    void DataB3Conf (cm_datab3conf *Params);
    void ListenConf (c_dword Controller, c_info Info);
    void ManufacturerConf (c_dword Controller, cm_manufacturer *Params);
    void SelectBProtocolConf (c_info Info);
    void ConnectInd (c_dword Controller, cm_connectind *Params);
    void ConnectActiveInd (CCStruct *pConnectedNumber);
    void ConnectB3ActiveInd (CCStruct *pNCPI);
    void DataB3Ind (cm_datab3 *Params);
    void DisconnectB3Ind (cm_disconnectb3ind *Params);
    void DisconnectInd (c_info Reason);
    void InfoInd (cm_infoind *Params);
    void ManufacturerInd (c_dword Controller, cm_manufacturer *Params);
    void fConf_PMWakeup (c_dword Controller, c_info Info, c_word NumberAccepted);
    void GET_PROFILE_COMPLETE (tUInt Controller, cm_getprofile *pProfile, c_info ErrorCode);

    /*-----------------------------------------------------------------------*\
        Interne Funktionen
    \*-----------------------------------------------------------------------*/
    void ResetSendBlock (void);
    tUInt CheckFinishReset (tUInt NewResetState); // 0 state set, 1 reset wait, 2 reset done

    /*-----------------------------------------------------------------------*\
        Interne Variablen
    \*-----------------------------------------------------------------------*/
    CDataHandleAdmin   m_SendBlock;
    CCntrlMSNList     *m_pMSNList;
    tBool              m_deleteMSNList;
    tBool              m_WaitForDDINumbers;

    c_dword            m_OutController;
    c_dword            m_ActiveController;
    c_cipvalue         m_CIPValue;
    cp_bprotocol_store m_BProtocol;
    cp_addinfo_store   m_AdditionalInfo;

    cp_callingnumber   m_sMSN;
    CDynamicString     m_CalledPartyNum;
    CDynamicString     m_CallingNumber;
    CDynamicString     m_ParkIdentity;
    CDynamicString     m_DTMFDigits;
    tUInt              m_MaxDataSize;
    tUInt              m_DDIPrefixLen;
    tUInt              m_SupplServType;
    tUInt              m_DTMFState;
    tUShort            m_DTMFToneDuration;
    tUShort            m_DTMFGapDuration;
    tBool              m_DTMFListen;
    tBool              m_ManualRegister;
    tUInt              m_Reseting;
    tUInt              m_CheckListenInUse;
};


/*===========================================================================*\
    Inline Funktionen
\*===========================================================================*/

inline tCapiState CTransferChannel::CurState (void) {
    return GetState();
}

inline tUShort CTransferChannel::CountPutQueueItems (void) {
    return m_SendBlock.CurCount();
}

inline tBool CTransferChannel::IsPutQueueEmpty (void) {
    return m_SendBlock.IsEmpty();
}

inline tBool CTransferChannel::IsPutQueueFull (void) {
    return m_SendBlock.IsFull();
}

inline void CTransferChannel::SetMaxDataSize (tUInt MaxDataSize) {
    if (!MaxDataSize || (MaxDataSize > CAPI_MAX_DATA_SIZE)) {
        MaxDataSize = CAPI_MAX_DATA_SIZE;
    }
    m_MaxDataSize = MaxDataSize;
}

inline void CTransferChannel::SetRingingTimeout (tUInt secs) {
    CCAPI20_Channel::SetRingingTimeout (secs);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void CTransferChannel::SetOutController (tUInt Controller) {
    // dassert (Controller != IllegalController);
    m_OutController = Controller;
}

inline tUInt CTransferChannel::GetOutController (void) {
    return m_OutController;
}

inline tUInt CTransferChannel::GetActiveController (void) {
    return m_ActiveController;
}

inline void CTransferChannel::SetOwnNumber (CConstString *pMyNumber, tUByte Indicator) {
    // dassert (pMyNumber != 0);
    if (m_sMSN.pNumber->Set (pMyNumber) == vTrue) {
        m_sMSN.Indicator = Indicator;
    }
}

inline CConstString *CTransferChannel::GetOwnNumber (void) {
    return m_sMSN.pNumber;
}

inline void CTransferChannel::SetB1Parameters (c_b1prot B1Protocol, cp_b1configuration *pB1Config) {
    if (infoSuccess (m_BProtocol.pB1Configuration->ParseStruct (pB1Config))) {
        m_BProtocol.B1Protocol = B1Protocol;
    }
}

inline void CTransferChannel::SetB2Parameters (c_b2prot B2Protocol) {
    m_BProtocol.B2Protocol = B2Protocol;
}

inline void CTransferChannel::SetB3Parameters (c_b3prot B3Protocol, cp_b3configuration *pB3Config) {
    if (infoSuccess (m_BProtocol.pB3Configuration->ParseStruct (pB3Config))) {
        m_BProtocol.B3Protocol = B3Protocol;
    }
}

inline void CTransferChannel::SetAdditionalInfo (tUInt setflags, CCStruct *pBChannelInfo, CCStruct *pKeypadFacility,
                                                 CCStruct *pUserUserData, CCStruct *pFacilityDataArray, CCStruct *pSendingComplete) {
    if (CheckOneFlag (setflags, 0)) {
        m_AdditionalInfo.pBChannelInformation->ParseStruct (pBChannelInfo);
    }
    if (CheckOneFlag (setflags, 1)) {
        m_AdditionalInfo.pKeypadFacility->ParseStruct (pKeypadFacility);
    }
    if (CheckOneFlag (setflags, 2)) {
        m_AdditionalInfo.pUserUserData->ParseStruct (pUserUserData);
    }
    if (CheckOneFlag (setflags, 3)) {
        m_AdditionalInfo.pFacilityDataArray->ParseStruct (pFacilityDataArray);
    }
    if (CheckOneFlag (setflags, 4)) {
        m_AdditionalInfo.pSendingComplete->ParseStruct (pSendingComplete);
    }
}

inline void CTransferChannel::SetNCPI (cp_ncpi *pNCPI) {
    CCAPI20_Channel::SetNCPI (pNCPI);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CTransferChannel::SetMSNList (CCntrlMSNList *pCntrlMSNList) {
    if (m_deleteMSNList == vTrue) {
        delete m_pMSNList;
    }
    if (pCntrlMSNList != NULL) {
        m_pMSNList = new CCntrlMSNList(*pCntrlMSNList);
        m_deleteMSNList = vTrue;
    } else {
        m_pMSNList = NULL;
        m_deleteMSNList = vFalse;
    }
    CheckListen();
    return vTrue;
}

inline CCntrlMSNList *CTransferChannel::GetMSNList (void) {
    if (!m_pMSNList) {
        m_pMSNList      = new CCntrlMSNList;
        m_deleteMSNList = vTrue;
    }
    return m_pMSNList;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void CTransferChannel::AlertCall (void) {
    if (GetState() == cs_D_ConnectIndication) {
        AlertReq (0);
    }
}

inline void CTransferChannel::GetDataComplete (tUShort DataHandle) {
    DataB3Resp (DataHandle);
}

inline void CTransferChannel::DisconnectConfirm (void) {
    switch (CurState()) {
    case cs_B_DisconnectRespPending:
    case cs_D_DisconnectRespPending:
        DisconnectB3Resp();
        break;
    default:
        break;
    }
}

inline void CTransferChannel::ConnectConf (void) {
    DialingEnd();
}

inline void CTransferChannel::DisconnectInquiry (c_info /*ReasonB3*/, cp_ncpi_all * /*pNCPI*/) {
    DisconnectConfirm();
}

inline void CTransferChannel::GET_PROFILE_COMPLETE (tUInt Controller, cm_getprofile *pProfile, c_info ErrorCode) {
    GetProfileComplete (Controller, pProfile, ErrorCode);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/
#ifdef USE_SUPPLEMENTARY_SERVICES

inline void CTransferChannel::fInd_DTMF (CCStruct *pDTMFdigits) {
    // dassert (pDTMFdigits == 0);
    // dwarning (m_DTMFListen == vTrue);
    if ((m_DTMFListen == vTrue) && (pDTMFdigits->GetLen())) {
        DTMFReceived (pDTMFdigits->GetLen(), pDTMFdigits->GetData());
    }
}

inline void CTransferChannel::fInd_Hold (c_info Reason) {
    CallHolded (Reason);
}

inline void CTransferChannel::fInd_Retrieve (c_info Reason) {
    CallUnholded (Reason);
}

inline void CTransferChannel::fInd_Suspend (c_info Reason) {
    CallParked (Reason);
}

inline void CTransferChannel::fInd_Resume (c_info Reason) {
    if (CapiInfoIsSuccess (Reason)) {
        SelectBProtocolReq (&m_BProtocol);
    }
    CallUnparked (Reason);
}

inline void CTransferChannel::fInd_CD (c_info Reason) {
    CallRedirected (Reason);
}

inline void CTransferChannel::fInd_ECT (c_info Reason) {
    CallTransfered (Reason);
}

inline void CTransferChannel::fInd_3PTY_Begin (c_info Reason) {
    ConferenceComplete (vTrue, Reason);
}

inline void CTransferChannel::fInd_3PTY_End (c_info Reason) {
    ConferenceComplete (vFalse, Reason);
}

#endif // USE_SUPPLEMENTARY_SERVICES
/*===========================================================================*\
\*===========================================================================*/

#endif
