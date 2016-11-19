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

#include "Channel.h"
#include "dbgCAPI.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define CAPI_CALLEDPARTYNUM_MAXLEN              70

#define DTMF_STATE_N                             0    // N = Nothing
#define DTMF_STATE_B                             1    // B = Listen Begin
#define DTMF_STATE_E                             2    // E = Listen End
#define DTMF_STATE_S                             3    // S = Send
#define DTMF_STATE_BE                            4
#define DTMF_STATE_BS                            5
#define DTMF_STATE_EB                            6
#define DTMF_STATE_ES                            7
#define DTMF_STATE_SE                            8
#define DTMF_STATE_SB                            9
#define DTMF_STATE_BSE                          10
#define DTMF_STATE_ESB                          11

#define CHANNEL_STATUS_INIT                     0x00
#define CHANNEL_STATUS_WAIT_REGISTER            0x01
#define CHANNEL_STATUS_WAIT_RELEASE             0x02
#define CHANNEL_STATUS_RESET                    0x04
#define CHANNEL_STATUS_DISCONNECT_WAIT_NO       0x08
#define CHANNEL_STATUS_DISCONNECT_WAIT_WITH     0x10
#define CHANNEL_STATUS_DISCONNECT_WAIT          0x18

/*===========================================================================*\
    CTransferChannel - Konstruktor
\*===========================================================================*/

CTransferChannel::CTransferChannel (void)
  : m_pMSNList (0),
    m_deleteMSNList (vFalse),
    m_WaitForDDINumbers (vFalse),
    m_OutController (IllegalController),
    m_ActiveController (IllegalController),
    m_CIPValue (cipvalue_Undefined),
    m_MaxDataSize (CAPI_MAX_DATA_SIZE),
    m_DDIPrefixLen (0),
    m_SupplServType (0),
    m_DTMFState (DTMF_STATE_N),
    m_DTMFToneDuration (0),
    m_DTMFGapDuration (0),
    m_DTMFListen (vFalse),
    m_ManualRegister (vFalse),
    m_Reseting (CHANNEL_STATUS_INIT),
    m_CheckListenInUse (0) {

    dhead ("CTransferChannel-Constructor", DCON_CTransferChannel);
    // TODO!!: Handle Error condition!!!!!
    m_SendBlock.InitEntries (CAPI_MAX_OUTSTANDING_FRAMES);
}


/*===========================================================================*\
    CTransferChannel - Destruktor
\*===========================================================================*/

CTransferChannel::~CTransferChannel (void) {
    dhead ("CTransferChannel-Destructor", DCON_CTransferChannel);
    dassert (CurState() == cs_Released);
    dassert (m_Reseting == CHANNEL_STATUS_INIT);
    dwarning (m_ManualRegister == vFalse);
    if (m_deleteMSNList == vTrue) {
        delete m_pMSNList;
    }
    m_pMSNList         = 0;
    m_deleteMSNList    = vFalse;
    m_ManualRegister   = vFalse;
    m_OutController    = IllegalController;
    m_ActiveController = IllegalController;
    m_CIPValue         = cipvalue_Undefined;
    m_SupplServType    = 0;
}


/*===========================================================================*\
    CTransferChannel::ResetChannel
\*===========================================================================*/

void CTransferChannel::ResetChannel (void) {
    dhead ("CTransferChannel::ResetChannel", DCON_CTransferChannel);
    dparams ("%x,%x", CurState(), m_Reseting);
    dwarning (m_Reseting == CHANNEL_STATUS_INIT);
    GetProtectClass()->BeginWrite();
    m_ManualRegister = vFalse;
    if (  (CurState() == cs_Released)
       && (m_Reseting == CHANNEL_STATUS_INIT)
       && (m_CheckListenInUse == 0)) {
        GetProtectClass()->EndWrite();
        IsReseted();
    } else {
        SetBitsAll (&m_Reseting, CHANNEL_STATUS_RESET);
        GetProtectClass()->EndWrite();
        Disconnect (DisconnectKind_Fast);
        SetMSNList (0);
    }
}


/*===========================================================================*\
    CTransferChannel::CheckListen
\*===========================================================================*/

void CTransferChannel::CheckListen (void) {
    dhead ("CTransferChannel::CheckListen", DCON_CTransferChannel);
    dprint ("state=%x,pMSNList=%p ", GetState(), m_pMSNList);
    GetProtectClass()->BeginWrite();
    if (CheckBitsOne (m_Reseting, CHANNEL_STATUS_RESET)) {
        dprint ("m_Reseting=%x, m_CheckListenInUse=%x ", m_Reseting, m_CheckListenInUse);
        if ((GetState() == cs_Registered) && (m_ManualRegister == vFalse)) {
            SetBitsAll (&m_Reseting, CHANNEL_STATUS_WAIT_RELEASE);
            GetProtectClass()->EndWrite();
            dprint ("Start Release: ");
            Release();
            dprint ("Finished Release! ");
        } else {
            // nothing more to do, because someone else is responsible for calling CheckFinishReset
            GetProtectClass()->EndWrite();
        }
        return;
    }
    if (basicInc (&m_CheckListenInUse) > 0) {
        GetProtectClass()->EndWrite();
        return;
    } else {
        dprint ("m_Reseting=%x, m_CheckListenInUse=%x ", m_Reseting, m_CheckListenInUse);
        GetProtectClass()->EndWrite();
        do {
            CCntrlMSNList *pTmpMSNList = m_pMSNList;
            switch (GetState()) {
            case cs_Released:
                dprint ("cl-State=cs_Released\n");
                if (pTmpMSNList && (pTmpMSNList->IsEmpty() == vFalse)) {
                    dwarning (CheckBitsOne (m_Reseting, CHANNEL_STATUS_RESET) == vFalse);
                    GetProtectClass()->BeginWrite();
                    SetBitsAll (&m_Reseting, CHANNEL_STATUS_WAIT_REGISTER);
                    GetProtectClass()->EndWrite();
                    Register (m_MaxDataSize);
                }
                break;
            case cs_Registering:
                dprint ("cl-State=cs_Registering\n");
                // nothing to do!
                break;
            case cs_Registered:
                dprint ("cl-State=cs_Registered\n");
                if ((!pTmpMSNList || (pTmpMSNList->IsEmpty() == vTrue)) && (m_ManualRegister == vFalse)) {
                    GetProtectClass()->BeginWrite();
                    SetBitsAll (&m_Reseting, CHANNEL_STATUS_WAIT_RELEASE);
                    GetProtectClass()->EndWrite();
                    dprint ("Start Release: ");
                    Release();
                    dprint ("Finished Release! ");
                    break;
                }
                // no break!
            default:
                dprint ("cl-State=default\n");
                if (pTmpMSNList) {
                    tUInt                       i;
                    tUInt                       newCntrler;
                    tUInt                       countMSNs;
                    cp_facilityreq_wakeupparam  OneWakeupNumber;
                    cp_facilityreq_wakeupparam *WakeupList;
                    COneMultiString            *pCurNumber;
                    cm_listenreq                newListenP;

                    dprint ("cl-ResetGetNextMask\n");
                    pTmpMSNList->ResetGetNextMask();
                    dprint ("cl-MSNList-Loop\n");
                    while (pTmpMSNList->GetNextMask (&newCntrler, &newListenP.InfoMask, &newListenP.CIPMask) == vTrue) {
                        dprint ("cl-newCntrler=%x\n", newCntrler);
                        dassert (newCntrler != IllegalController);
                        ListenReq (newCntrler, &newListenP);
                        dprint ("cl-AfterListen\n");
                        if (pTmpMSNList->GetWakeupSupport (newCntrler) == vTrue) {
                            dprint ("cl-WakeUpEnabled\n");
                            CMultiString *allMSNs = pTmpMSNList->GetAllMSNs (newCntrler);
                            if (  !allMSNs
                               || CheckBitsAll (newListenP.InfoMask,
                                                (infomask_CalledPartyNum | infomask_SendingComplete))
                               || pTmpMSNList->ExistMSNRange (newCntrler)) {
                                // empty MSNList, MSN Ranges or DDI in use => disable Wakeup
                                dwarning (0);
                                fReq_PMWakeup (newCntrler, 0, 0);
                            } else {
                                dassert (allMSNs->IsEmpty() == vFalse);
                                countMSNs = allMSNs->CountElements();
                                if (countMSNs > 1) {
                                    WakeupList = new cp_facilityreq_wakeupparam [countMSNs];
                                    if (WakeupList) {
                                        pCurNumber = allMSNs->GetFirst();
                                        for (i = 0; i < countMSNs; i++, pCurNumber = pCurNumber->GetNext()) {
                                            dassert (pCurNumber != 0);
                                            dassert (WakeupList[i].pCalledPartyNumber != 0);
                                            WakeupList[i].pCalledPartyNumber->pNumber = pCurNumber;
                                            WakeupList[i].CIPMask                     = newListenP.CIPMask;
                                        }
                                        dassert (countMSNs <= MAXVAL_tUShort);
                                        fReq_PMWakeup (newCntrler, (tUShort)countMSNs, WakeupList);
                                        delete [] WakeupList;
                                    } else {
                                        dwarning (0);
                                        fReq_PMWakeup (newCntrler, 0, 0);
                                    }
                                } else  {
                                    dassert (OneWakeupNumber.pCalledPartyNumber != 0);
                                    dassert (allMSNs->GetFirst() != 0);
                                    OneWakeupNumber.pCalledPartyNumber->pNumber = allMSNs->GetFirst();
                                    OneWakeupNumber.CIPMask                     = newListenP.CIPMask;
                                    dassert (OneWakeupNumber.pCalledPartyNumber != 0);
                                    fReq_PMWakeup (newCntrler, 1, &OneWakeupNumber);
                                }
                            }
                        }
                        dprint ("cl-NextIteration\n");
                    }
                }
                break;
            }
            dprint ("Cur-m_CheckListenInUse=%x ", m_CheckListenInUse);
        } while (GetProtectClass()->Dec (&m_CheckListenInUse) > 1);
        dprint ("Loop terminated! ");
    }
    if (GetState() == cs_Released) {
        dprint ("Calling CheckFinishReset! ");
        CheckFinishReset (CHANNEL_STATUS_INIT);
    }
}


/*===========================================================================*\
    CTransferChannel::MakeCall
\*===========================================================================*/

tBool CTransferChannel::MakeCall (CConstString *pTelNumber, c_cipvalue CipValue) {
    dhead ("CTransferChannel::MakeCall", DCON_CTransferChannel);
    dwarning (pTelNumber != 0);
    dwarning (CipValue != cipvalue_Undefined);
    dwarning (m_ActiveController == IllegalController);
    dassert (m_OutController != IllegalController);

    if (  (CheckBitsOne (GetProtectClass()->Get (&m_Reseting), CHANNEL_STATUS_RESET))
       || (m_ActiveController != IllegalController)
       || (GetState() > cs_Registered)) {
        RETURN ('x', vFalse);
    }
    if (pTelNumber) {
        if (m_CalledPartyNum.Set (pTelNumber) == vFalse) {
            dwarning (0);
            RETURN ('x', vFalse);
        }
    } else {
        m_CalledPartyNum.RemoveAll();
    }
    m_ActiveController = m_OutController;
    m_CIPValue         = CipValue;
    m_SupplServType    = 0;
    dprint ("state=%x ", GetState());
    switch (GetState()) {
    case cs_Released:
        dwarning (CheckBitsOne (m_Reseting, CHANNEL_STATUS_RESET) == vFalse);
        GetProtectClass()->BeginWrite();
        if (CheckBitsOne (m_Reseting, CHANNEL_STATUS_RESET)) {
            GetProtectClass()->EndWrite();
            m_ActiveController = IllegalController;
            m_CIPValue         = cipvalue_Undefined;
            m_CalledPartyNum.RemoveAll();
            RETURN ('x', vFalse);
        }
        SetBitsAll (&m_Reseting, CHANNEL_STATUS_WAIT_REGISTER);
        GetProtectClass()->EndWrite();
        Register (m_MaxDataSize);
        break;
    case cs_Registering:
        /*----- Nothing To Do! -----*/
        break;
    default:
        dwarning (0);
        break;
    case cs_Registered:
        {
            DialingBegin();
            cp_callednumber callednumparam (&m_CalledPartyNum);
            cm_connectreq ConParams (m_CIPValue, &callednumparam, &m_sMSN, 0, 0, &m_BProtocol, &m_AdditionalInfo);
            if (ConnectReq (m_ActiveController, &ConParams) == vFalse) {
                m_ActiveController = IllegalController;
                m_CIPValue         = cipvalue_Undefined;
                m_CalledPartyNum.RemoveAll();
                RETURN ('x', vFalse);
            }
        }
        break;
    }
    RETURN ('x', vTrue);
}


/*===========================================================================*\
    CTransferChannel::AnswerCall
\*===========================================================================*/

void CTransferChannel::AnswerCall (c_reject Reject) {
    dhead ("CTransferChannel::AnswerCall", DCON_CTransferChannel);
    dparams ("%x", Reject);
    cp_connectednumber ConnectNum;
    ConnectNum.pNumber = &m_CalledPartyNum;
    cm_connectresp ConParams (Reject, &m_BProtocol, (m_CalledPartyNum.IsEmpty()) ? 0 : &ConnectNum, 0, &m_AdditionalInfo);
    ConnectResp (&ConParams);
}


/*===========================================================================*\
    CTransferChannel::PutData
\*===========================================================================*/

tBool CTransferChannel::PutData (tUByte *Data, tUShort DataLength, void *hDataID) {
    dhead ("CTransferChannel::PutData", DCON_CTransferChannel);
    dparams ("d=%p,l=%x,h=%x", Data, DataLength, hDataID);
    if (CheckBitsOne (m_Reseting, CHANNEL_STATUS_DISCONNECT_WAIT_NO | CHANNEL_STATUS_RESET)) {
        dwarning (0);
        RETURN ('x',  vFalse);
    }
    tUShort CapiHandle;
    if (m_SendBlock.New (hDataID, &CapiHandle) == vFalse) {
        //PutDataComplete (hDataID, i_QueueFull);
        RETURN ('x', vFalse);
    }
    // dassert (CapiHandle <= MAXVAL_tUShort); // check is pointless since range limited by datatype
    cm_datab3 DataParams (Data, DataLength, CapiHandle, 0);
    DataB3Req (&DataParams);
    RETURN ('x', vTrue);
}


/*===========================================================================*\
    CTransferChannel::Disconnect
\*===========================================================================*/

void CTransferChannel::Disconnect (tDisconnectKind kind) {
    dhead ("CTransferChannel::Disconnect", DCON_CTransferChannel);
    dparams ("%x", kind);
    switch (kind) {
    case DisconnectKind_WaitWithPut:
        if (IsPutQueueEmpty() == vFalse) {
            m_Reseting = 4;
        } else {
            DisconnectReq();
        }
        break;
    case DisconnectKind_WaitNoPut:
        if (IsPutQueueEmpty() == vFalse) {
            m_Reseting = 2;
        } else {
            DisconnectReq();
        }
        break;
    default:
    case DisconnectKind_Normal:
        DisconnectReq();
        break;
    case DisconnectKind_Fast:
        DisconnectReq (vTrue);
        break;
    case DisconnectKind_Hard:
        GetProtectClass()->BeginWrite();
        SetBitsAll (&m_Reseting, CHANNEL_STATUS_WAIT_RELEASE);
        GetProtectClass()->EndWrite();
        dwarning (m_ManualRegister == vTrue);
        m_ManualRegister = vTrue;
        Release();
        break;
    }
}


/*===========================================================================*\
    CTransferChannel::UpdateBParameters
\*===========================================================================*/

void CTransferChannel::UpdateBParameters (void) {
    dhead ("CTransferChannel::UpdateBParameters", DCON_CTransferChannel);
    SelectBProtocolReq (&m_BProtocol);
}


/*===========================================================================*\
    CTransferChannel::HoldCall
\*===========================================================================*/
#ifdef USE_SUPPLEMENTARY_SERVICES

void CTransferChannel::HoldCall (void) {
    dhead ("CTransferChannel::HoldCall", DCON_CTransferChannel);
    if (GetState() > cs_D_Connected) {
        m_SupplServType = supplserv_Hold;
        DisconnectReq();
    } else {
        fReq_Hold();
    }
}


/*===========================================================================*\
    CTransferChannel::UnholdCall
\*===========================================================================*/

void CTransferChannel::UnholdCall (void) {
    dhead ("CTransferChannel::UnholdCall", DCON_CTransferChannel);
    fReq_Retrieve();
}


/*===========================================================================*\
    CTransferChannel::ParkCall
\*===========================================================================*/

void CTransferChannel::ParkCall (CConstString *pIDStr) {
    dhead ("CTransferChannel::ParkCall", DCON_CTransferChannel);
    if (GetState() < cs_D_Connected) {
        CallParked (ci_Int_IllegalState);
    } else if (m_ParkIdentity.Set (pIDStr) == vFalse) {
        CallParked (ci_Int_MemoryFullErr);
    } else if (GetState() > cs_D_Connected) {
        m_SupplServType = supplserv_Suspend;
        DisconnectReq();
    } else {
        fReq_Suspend (&m_ParkIdentity);
        m_ParkIdentity.RemoveAll();
    }
}


/*===========================================================================*\
    CTransferChannel::UnparkCall
\*===========================================================================*/

void CTransferChannel::UnparkCall (CConstString *pIDStr) {
    dhead ("CTransferChannel::UnparkCall", DCON_CTransferChannel);
    dassert (m_OutController != IllegalController);
    dassert (m_ActiveController == IllegalController);
    if (GetState() > cs_Registered) {
        CallUnparked (ci_Int_IllegalState);
    } else if (m_ParkIdentity.Set (pIDStr) == vFalse) {
        CallUnparked (ci_Int_MemoryFullErr);
    } else {
        switch (GetState()) {
        case cs_Released:
        //case cs_Releasing:
            dwarning (CheckBitsOne (m_Reseting, CHANNEL_STATUS_RESET) == vFalse);
            GetProtectClass()->BeginWrite();
            if (CheckBitsOne (m_Reseting, CHANNEL_STATUS_RESET)) {
                GetProtectClass()->EndWrite();
                break;
            }
            SetBitsAll (&m_Reseting, CHANNEL_STATUS_WAIT_REGISTER);
            GetProtectClass()->EndWrite();
            m_ActiveController = m_OutController;
            m_CIPValue         = cipvalue_Undefined;
            m_SupplServType    = supplserv_Resume;
            Register (m_MaxDataSize);
            break;
        case cs_Registering:
            m_ActiveController = m_OutController;
            m_CIPValue         = cipvalue_Undefined;
            m_SupplServType    = supplserv_Resume;
            break;
        default:
            dassert (0);
            // no break!
        case cs_Registered:
            m_ActiveController = m_OutController;
            m_CIPValue         = cipvalue_Undefined;
            fReq_Resume (m_ActiveController, &m_ParkIdentity);
            m_ParkIdentity.RemoveAll();
            break;
        }
    }
}


/*===========================================================================*\
    CTransferChannel::RedirectCall
\*===========================================================================*/

void CTransferChannel::RedirectCall (CDynamicString *pTelNumber) {
    dhead ("CTransferChannel::RedirectCall", DCON_CTransferChannel);
    dassert (pTelNumber != 0);
    fReq_CD (vTrue, pTelNumber);
}


/*===========================================================================*\
    CTransferChannel::TransferCall
\*===========================================================================*/

void CTransferChannel::TransferCall (CTransferChannel *pChannel) {
    dhead ("CTransferChannel::TransferCall", DCON_CTransferChannel);
    dassert (pChannel != 0);
    dassert (GetState() == cs_Holded);
    dassert (pChannel->GetState() == cs_Connected);
    fReq_ECT (pChannel);
}


/*===========================================================================*\
    CTransferChannel::Conference
\*===========================================================================*/

void CTransferChannel::Conference (CTransferChannel *pChannel, tBool doBegin) {
    dhead ("CTransferChannel::Conference", DCON_CTransferChannel);
    dparams ("%x", doBegin);
    dassert (pChannel != 0);
    if (doBegin == vTrue) {
        fReq_3PTY_Begin (pChannel);
    } else {
        fReq_3PTY_End (pChannel);
    }
}


/*===========================================================================*\
    CTransferChannel::DTMFListen
\*===========================================================================*/

void CTransferChannel::DTMFListen (tBool activate) {
    dhead ("CTransferChannel::DTMFListen", DCON_CTransferChannel);
    dparams ("%x", activate);
    dwarning (GetState() == cs_Connected);
    if (activate == m_DTMFListen) {
        return;
    }
    m_DTMFListen = activate;
    if (GetState() != cs_Connected) {
        return;
    }
    if (activate == vTrue) {
        switch (m_DTMFState) {
        case DTMF_STATE_N:
            m_DTMFState = DTMF_STATE_B;
            fReq_DTMF (dtmffunction_StartListen, 0, 0, 0, 0);
            break;
        case DTMF_STATE_B:
            // nothing to do!
            break;
        case DTMF_STATE_E:
            m_DTMFState = DTMF_STATE_BE;
            break;
        case DTMF_STATE_S:
            m_DTMFState = DTMF_STATE_SB;
            break;
        case DTMF_STATE_BE:
            m_DTMFState = DTMF_STATE_B;
            break;
        case DTMF_STATE_BS:
            m_DTMFState = DTMF_STATE_B;
            break;
        case DTMF_STATE_EB:
            // nothing to do!
            break;
        case DTMF_STATE_ES:
            m_DTMFState = DTMF_STATE_ESB;
            break;
        case DTMF_STATE_SE:
            m_DTMFState = DTMF_STATE_SB;
            break;
        case DTMF_STATE_SB:
            // nothing to do!
            break;
        case DTMF_STATE_BSE:
            m_DTMFState = DTMF_STATE_BS;
            break;
        default:
            dassert (0);
            break;
        }
    } else {
        switch (m_DTMFState) {
        case DTMF_STATE_N:
            m_DTMFState = DTMF_STATE_E;
            fReq_DTMF (dtmffunction_StopListen, 0, 0, 0, 0);
            break;
        case DTMF_STATE_B:
            m_DTMFState = DTMF_STATE_BE;
            break;
        case DTMF_STATE_E:
            // nothing to do!
            break;
        case DTMF_STATE_S:
            m_DTMFState = DTMF_STATE_SE;
            break;
        case DTMF_STATE_BE:
            // nothing to do!
            break;
        case DTMF_STATE_BS:
            m_DTMFState = DTMF_STATE_BSE;
            break;
        case DTMF_STATE_EB:
            m_DTMFState = DTMF_STATE_E;
            break;
        case DTMF_STATE_ES:
            // nothing to do!
            break;
        case DTMF_STATE_SE:
            // nothing to do!
            break;
        case DTMF_STATE_SB:
            m_DTMFState = DTMF_STATE_SE;
            break;
            break;
        case DTMF_STATE_BSE:
            // nothing to do!
            break;
        default:
            dassert (0);
            break;
        }
    }
}


/*===========================================================================*\
    CTransferChannel::DTMFSend
\*===========================================================================*/

void CTransferChannel::DTMFSend (CConstString *pDTMFDigits, tUShort ToneDuration, tUShort GapDuration) {
    dhead ("CTransferChannel::DTMFSend", DCON_CTransferChannel);
    dassert (pDTMFDigits != 0);
    dassert (GetState() == cs_Connected);
    dwarning (m_DTMFDigits.GetLen() == 0);
    if (m_DTMFDigits.IsEmpty() == vFalse) {
        DTMFSendComplete (ci_Int_FunctionInUse);
    } else if (m_DTMFDigits.Set (pDTMFDigits) == vTrue) {
        dassert (m_DTMFDigits.GetLen() > 0);
        m_DTMFToneDuration = ToneDuration;
        m_DTMFGapDuration  = GapDuration;
        switch (m_DTMFState) {
        case DTMF_STATE_N:
            m_DTMFState = DTMF_STATE_S;
            fReq_DTMF (dtmffunction_SendDigits, m_DTMFToneDuration, m_DTMFGapDuration, &m_DTMFDigits, 0);
            m_DTMFDigits.RemoveAll();
            break;
        case DTMF_STATE_B:
            m_DTMFState = DTMF_STATE_BS;
            break;
        case DTMF_STATE_E:
            m_DTMFState = DTMF_STATE_ES;
            break;
        case DTMF_STATE_BE:
            m_DTMFState = DTMF_STATE_BSE;
            break;
        case DTMF_STATE_EB:
            m_DTMFState = DTMF_STATE_ESB;
            break;
        default:
        case DTMF_STATE_BS:
        case DTMF_STATE_ES:
        case DTMF_STATE_BSE:
        case DTMF_STATE_ESB:
            dassert (0);
            // no break!
        case DTMF_STATE_S:
        case DTMF_STATE_SE:
        case DTMF_STATE_SB:
            dwarning (0);
            m_DTMFDigits.RemoveAll();
            DTMFSendComplete (ci_Int_FunctionInUse);
            break;
        }
    } else {
        DTMFSendComplete (ci_Int_MemoryFullErr);
    }
}

/*===========================================================================*\
    CTransferChannel::fConf_DTMF
\*===========================================================================*/

void CTransferChannel::fConf_DTMF (c_info Info) {
    dhead ("CTransferChannel::fConf_DTMF", DCON_CTransferChannel);
    dprint ("%x", Info);
    tUInt now  = DTMF_STATE_N;
    tUInt next = DTMF_STATE_N;
    switch (m_DTMFState) {
    case DTMF_STATE_N:
        dwarning (0);
        break;
    case DTMF_STATE_B:
        m_DTMFState = DTMF_STATE_N;
        now         = DTMF_STATE_B;
        break;
    case DTMF_STATE_E:
        m_DTMFState = DTMF_STATE_N;
        now         = DTMF_STATE_E;
        break;
    case DTMF_STATE_S:
        m_DTMFState = DTMF_STATE_N;
        now         = DTMF_STATE_S;
        break;
    case DTMF_STATE_BE:
        m_DTMFState = DTMF_STATE_E;
        next        = DTMF_STATE_E;
        now         = DTMF_STATE_B;
        break;
    case DTMF_STATE_BS:
        m_DTMFState = DTMF_STATE_S;
        next        = DTMF_STATE_S;
        now         = DTMF_STATE_B;
        break;
    case DTMF_STATE_ES:
        m_DTMFState = DTMF_STATE_S;
        next        = DTMF_STATE_S;
        now         = DTMF_STATE_E;
        break;
    case DTMF_STATE_SB:
        m_DTMFState = DTMF_STATE_B;
        next        = DTMF_STATE_B;
        now         = DTMF_STATE_S;
        break;
    case DTMF_STATE_SE:
        m_DTMFState = DTMF_STATE_E;
        next        = DTMF_STATE_E;
        now         = DTMF_STATE_S;
        break;
    case DTMF_STATE_EB:
        m_DTMFState = DTMF_STATE_B;
        next        = DTMF_STATE_B;
        now         = DTMF_STATE_E;
        break;
    case DTMF_STATE_BSE:
        m_DTMFState = DTMF_STATE_SE;
        next        = DTMF_STATE_S;
        now         = DTMF_STATE_B;
        break;
    case DTMF_STATE_ESB:
        m_DTMFState = DTMF_STATE_SB;
        next        = DTMF_STATE_S;
        now         = DTMF_STATE_E;
        break;
    default:
        dassert (0);
        break;
    }
    switch (now) {
    case DTMF_STATE_N:
        break;
    case DTMF_STATE_B:
        if (CapiInfoIsError (Info)) {
            m_DTMFListen = vFalse;
            DTMFListenError (Info);
        }
        break;
    case DTMF_STATE_E:
        break;
    case DTMF_STATE_S:
        DTMFSendComplete (Info);
        break;
    default:
        dassert (0);
        break;
    }
    switch (next) {
    case DTMF_STATE_N:
        break;
    case DTMF_STATE_B:
        fReq_DTMF (dtmffunction_StartListen, 0, 0, 0, 0);
        break;
    case DTMF_STATE_E:
        fReq_DTMF (dtmffunction_StopListen, 0, 0, 0, 0);
        break;
    case DTMF_STATE_S:
        dwarning (m_DTMFDigits.GetLen() > 0);
        fReq_DTMF (dtmffunction_SendDigits, m_DTMFToneDuration, m_DTMFGapDuration, &m_DTMFDigits, 0);
        m_DTMFDigits.RemoveAll();
        break;
    default:
        dassert (0);
        break;
    }
}


#endif
/*===========================================================================*\
    CTransferChannel::CapiStart
\*===========================================================================*/

void CTransferChannel::CapiStart (void) {
    dhead ("CTransferChannel::CapiStart", DCON_CTransferChannel);
    dwarning (m_ManualRegister == vFalse);
    m_ManualRegister = vTrue;
    if (CurState() == cs_Released) {
        dwarning (CheckBitsOne (m_Reseting, CHANNEL_STATUS_RESET) == vFalse);
        GetProtectClass()->BeginWrite();
        SetBitsAll (&m_Reseting, CHANNEL_STATUS_WAIT_REGISTER);
        GetProtectClass()->EndWrite();
        Register (m_MaxDataSize);
    }
}


/*===========================================================================*\
    CTransferChannel::CapiStop
\*===========================================================================*/

void CTransferChannel::CapiStop (void) {
    dhead ("CTransferChannel::CapiStop", DCON_CTransferChannel);
    dwarning (m_ManualRegister == vTrue);
    if ((m_ManualRegister == vTrue) && (CurState() >= cs_Registered)) {
        m_ManualRegister = vFalse;
        GetProtectClass()->BeginWrite();
        SetBitsAll (&m_Reseting, CHANNEL_STATUS_WAIT_RELEASE);
        GetProtectClass()->EndWrite();
        Release();
    }
}


/*===========================================================================*\
    CTransferChannel::GetProfile
\*===========================================================================*/

void CTransferChannel::GetProfile (tUInt Controller) {
    dhead ("CTransferChannel::GetProfile", DCON_CTransferChannel);
    dassert (GetState() >= cs_Registered);
    GET_PROFILE (Controller);
}


/*===========================================================================*\
    CTransferChannel::ManufacturerCall
\*===========================================================================*/

void CTransferChannel::ManufacturerCall (c_dword Function, CCStruct *pData) {
    dhead ("CTransferChannel::ManufacturerCall", DCON_CTransferChannel);
    dassert (m_OutController != IllegalController);
    dassert (GetState() >= cs_Registered);
    cm_manufacturer Params (0x214D5641, 0, Function, pData);
    ManufacturerReq (m_OutController, &Params);
}


/*===========================================================================*\
    CTransferChannel::Register_Complete
\*===========================================================================*/

void CTransferChannel::Register_Complete (c_info ErrorCode) {
    dhead ("CTransferChannel::Register_Complete", DCON_CTransferChannel);
    dwarning (ErrorCode == ci_NoError);
    if (CapiInfoIsSuccess (ErrorCode)) {
        GetProtectClass()->BeginWrite();
        if (CheckBitsOne (m_Reseting, CHANNEL_STATUS_RESET)) {
            SetBitsAll (&m_Reseting, CHANNEL_STATUS_WAIT_RELEASE);
            GetProtectClass()->EndWrite();
            Release();
            return;
        }
        m_Reseting = CHANNEL_STATUS_INIT;
        GetProtectClass()->EndWrite();
        if (m_ManualRegister == vTrue) {
            CapiStarted (ci_NoError);
        }
        if (m_ActiveController != IllegalController) {
            DialingBegin();
            #ifdef USE_SUPPLEMENTARY_SERVICES
                if (m_SupplServType == supplserv_Resume) {
                    fReq_Resume (m_ActiveController, &m_ParkIdentity);
                } else
            #endif
            {
                cp_callednumber callednumparam (&m_CalledPartyNum);
                cm_connectreq ConParams (m_CIPValue, &callednumparam, &m_sMSN, 0, 0, &m_BProtocol, &m_AdditionalInfo);
                if (ConnectReq (m_ActiveController, &ConParams) == vFalse) {
                    dassert (0);
                }
            }
        }
        m_SupplServType = 0;
        m_ParkIdentity.RemoveAll();
        CheckListen();
    } else if (ErrorCode != ci_Int_RegisterDelayed) {
        if (m_ActiveController != IllegalController) {
            m_ActiveController = IllegalController;
            m_CIPValue         = cipvalue_Undefined;
            m_CalledPartyNum.RemoveAll();
            IsDisconnected (ErrorCode);
        }
        if (m_pMSNList && (m_pMSNList->IsEmpty() == vFalse)) {
            ListenError (0, ErrorCode);
        }
        if (m_ManualRegister == vTrue) {
            m_ManualRegister = vFalse;
            CapiStarted (ErrorCode);
        }
        CheckFinishReset (CHANNEL_STATUS_INIT);
    }
}


/*===========================================================================*\
    CTransferChannel::Release_Complete
\*===========================================================================*/

void CTransferChannel::Release_Complete (void) {
    dhead ("CTransferChannel::Release_Complete", DCON_CTransferChannel);
    dassert (m_ActiveController == IllegalController);

    tUInt newState = (  (m_ActiveController != IllegalController)
                     || (m_ManualRegister == vTrue)
                     || (m_pMSNList && (m_pMSNList->IsEmpty() == vFalse))) ?
                     CHANNEL_STATUS_WAIT_REGISTER : CHANNEL_STATUS_INIT;
    dprint ("newState=%x,ActiceC=%x,m_pMSNList=%p ", newState, m_ActiveController, m_pMSNList);
    if ((CheckFinishReset (newState) == 0) && (newState == CHANNEL_STATUS_WAIT_REGISTER)) {
        Register (m_MaxDataSize);
    }
}


/*===========================================================================*\
    CTransferChannel::DataB3Conf
\*===========================================================================*/

void CTransferChannel::DataB3Conf (cm_datab3conf *Params) {
    dhead ("CTransferChannel::DataB3Conf", DCON_CTransferChannel);
    dassert (Params != 0);
    void *hDataID;
    if (m_SendBlock.Free (Params->DataHandle, &hDataID) == vTrue) {
        PutDataComplete (hDataID, Params->Info);
        if (CheckBitsOne (m_Reseting, CHANNEL_STATUS_DISCONNECT_WAIT) && (IsPutQueueEmpty() == vTrue)) {
            Disconnect();
        }
    } else {
        dprintErr ("Great CAPI Error (Now disconnecting)!!!\n");
        Disconnect();
        dassert (0);
    }
}


/*===========================================================================*\
    CTransferChannel::ListenConf
\*===========================================================================*/

void CTransferChannel::ListenConf (c_dword Controller, c_info Info) {
    dhead ("CTransferChannel::ListenConf", DCON_CTransferChannel);
    dparams ("c=%x,i=%x", Controller, Info);
    if (CapiInfoIsError (Info) == vTrue) {
        //if (Info == ci_Cod_IllegalAddress) {
        //    // illegal controller-number can be deleted
        //    m_pMSNList.Remove (Controller);
        //    CheckListen();
        //}
        ListenError (Controller, Info);
    }
}


/*===========================================================================*\
    CTransferChannel::ManufacturerConf
\*===========================================================================*/

void CTransferChannel::ManufacturerConf (c_dword, cm_manufacturer *Params) {
    dhead ("CTransferChannel::ManufacturerConf", DCON_CTransferChannel);
    ManufacturerCallComplete (Params->Class, Params->Function, Params->pManuData);
}


/*===========================================================================*\
    CTransferChannel::SelectBProtocolConf
\*===========================================================================*/

void CTransferChannel::SelectBProtocolConf (c_info Info) {
    dhead ("CTransferChannel::SelectBProtocolConf", DCON_CTransferChannel);
    UpdateBParametersComplete (Info);
}


/*===========================================================================*\
    CTransferChannel::ConnectInd
\*===========================================================================*/

void CTransferChannel::ConnectInd (c_dword Controller, cm_connectind *Params) {
    dhead ("CTransferChannel::ConnectInd", DCON_CTransferChannel);
    dassert (Params != 0);
    dassert (Params->pCalledPartyNumber != 0);
    dassert (Params->pCallingPartyNumber != 0);
    if (m_pMSNList != 0) {
        dassert (m_ActiveController == IllegalController);
        switch (m_pMSNList->FindCallInfo (Controller, Params->CIPValue, Params->pCalledPartyNumber, &m_DDIPrefixLen)) {
        case iWrn_MSNList_DDICall:
            dassert (CAPI_CALLEDPARTYNUM_MAXLEN < 254);
            dassert (m_DDIPrefixLen < CAPI_CALLEDPARTYNUM_MAXLEN);
            dassert (m_DDIPrefixLen > 0);
            if (Params->pCallingPartyNumber->GetLen() > 2) {
                m_CallingNumber.Set (Params->pCallingPartyNumber->GetData() + 2, Params->pCallingPartyNumber->GetLen() - 2);
            } else {
                m_CallingNumber.RemoveAll();
            }
            m_CalledPartyNum.RemoveAll();
            m_ActiveController  = Controller;
            m_CIPValue          = Params->CIPValue;
            return;
        case i_Okay:
            if (AcceptIncomingCall() == vTrue) {
                m_ActiveController = Controller;
                m_CIPValue         = Params->CIPValue;
                if (Params->pCalledPartyNumber->GetLen() > 1) {
                    dassert (Params->pCalledPartyNumber->GetData() != 0);
                    m_CalledPartyNum.Set (Params->pCalledPartyNumber->GetData() + 1, Params->pCalledPartyNumber->GetLen() - 1);
                } else {
                    m_CalledPartyNum.RemoveAll();
                }
                if (Params->pCallingPartyNumber->GetLen() > 2) {
                    m_CallingNumber.Set (Params->pCallingPartyNumber->GetData() + 2, Params->pCallingPartyNumber->GetLen() - 2);
                } else {
                    m_CallingNumber.RemoveAll();
                }
                dprint ("(%s)(%s)\n", m_CallingNumber.GetPointer(), m_CalledPartyNum.GetPointer());
                IncomingCall (m_CIPValue, &m_CallingNumber, &m_CalledPartyNum);
            }
            return;
        case iWrn_MSNList_RejectCall:
            break;
        default:
            dassert (0);
            break;
        }
    }
    {
        cm_connectresp crIgnore;
        ConnectResp (&crIgnore);
    }
}


/*===========================================================================*\
    CTransferChannel::ConnectActiveInd
\*===========================================================================*/

void CTransferChannel::ConnectActiveInd (CCStruct *pConnectedNumber) {
    dhead ("CTransferChannel::ConnectActiveInd", DCON_CTransferChannel);
    dassert (pConnectedNumber != 0);
    CDynamicString ConNum;
    if (pConnectedNumber->GetLen() > 2) {
        ConNum.Set (pConnectedNumber->GetData() + 2, pConnectedNumber->GetLen() - 2);
    }
    ConnectProceeding (&ConNum);
}


/*===========================================================================*\
    CTransferChannel::ConnectB3ActiveInd
\*===========================================================================*/

void CTransferChannel::ConnectB3ActiveInd (CCStruct *pNCPI) {
    dhead ("CTransferChannel::ConnectB3ActiveInd", DCON_CTransferChannel);
    dassert (pNCPI != 0);
    if (m_DTMFListen == vTrue) {
        fReq_DTMF (dtmffunction_StartListen, 0, 0, 0, 0);
    }
    if (pNCPI->GetLen() > 0) {
        cp_ncpi_all NCPIall (m_BProtocol.B3Protocol);
        tSInfo      fret = NCPIall.ParseStruct (pNCPI);
        dwarning (fret == i_Okay);
        //// Check ExtendedFax-Options, if CAPI tries to Build up with a set of not allowed features
        //if ((B3Prot == b3prot_G3Fax_extended) && (NCPIcapi->Options != 0)) {
        //    c_word *FAXoption;
        //    if (GetBeginAndLenOfStructData (B3Config, &(c_byte *)FAXoption) > 4 * sizeof (c_byte)) {
        //       FAXoption += 1;
        //       if (FAXoption & NCPIcapi->Options);
        //    }
        //}
        IsConnected (infoSuccess (fret) ? &NCPIall : 0);
    } else {
        IsConnected (0);
    }
}


/*===========================================================================*\
    CTransferChannel::DataB3Ind
\*===========================================================================*/

void CTransferChannel::DataB3Ind (cm_datab3 *Params) {
    dhead ("CTransferChannel::DataB3Ind", DCON_CTransferChannel);
    dassert (Params != 0);
    GetData (Params->Data, Params->DataLength, Params->DataHandle);
}


/*===========================================================================*\
    CTransferChannel::DisconnectB3Ind
\*===========================================================================*/

void CTransferChannel::DisconnectB3Ind (cm_disconnectb3ind *Params) {
    dhead ("CTransferChannel::DisconnectB3Ind", DCON_CTransferChannel);
    dassert (Params != 0);
    dassert (Params->pNCPI != 0);
    GetProtectClass()->BeginWrite();
    ClearBitsAll (&m_Reseting, CHANNEL_STATUS_DISCONNECT_WAIT);
    GetProtectClass()->EndWrite();
    ResetSendBlock();

    if (Params->pNCPI->GetLen() > 0) {
        cp_ncpi_all NCPIall (m_BProtocol.B3Protocol);
        NCPIall.ParseStruct (Params->pNCPI);
        DisconnectInquiry (Params->ReasonB3, &NCPIall);
    } else {
        DisconnectInquiry (Params->ReasonB3, 0);
    }

#ifdef USE_SUPPLEMENTARY_SERVICES
    switch (m_SupplServType) {
    case supplserv_Hold:
        fReq_Hold();
        break;
    case supplserv_Suspend:
        fReq_Suspend (&m_ParkIdentity);
        break;
    }
    m_SupplServType = 0;
    m_ParkIdentity.RemoveAll();
#endif
}


/*===========================================================================*\
    CTransferChannel::DisconnectInd
\*===========================================================================*/

void CTransferChannel::DisconnectInd (c_info Reason) {
    dhead ("CTransferChannel::DisconnectInd", DCON_CTransferChannel);
    //dwarning (m_ActiveController != IllegalController);
    if (m_ActiveController != IllegalController) {
        m_ActiveController  = IllegalController;
        m_CIPValue          = cipvalue_Undefined;
        m_DTMFListen        = vFalse;
        GetProtectClass()->BeginWrite();
        ClearBitsAll (&m_Reseting, CHANNEL_STATUS_DISCONNECT_WAIT);
        GetProtectClass()->EndWrite();
        m_CalledPartyNum.RemoveAll();
        m_CallingNumber.RemoveAll();
        m_ParkIdentity.RemoveAll();
        ResetSendBlock();
        if (m_DDIPrefixLen > 0) {
            m_DDIPrefixLen = 0;
        } else {
            IsDisconnected (Reason);
        }
        dprint ("DisconectInd-CheckListen ");
        CheckListen();
    }
}


/*===========================================================================*\
    CTransferChannel::InfoInd
\*===========================================================================*/

void CTransferChannel::InfoInd (cm_infoind *Params) {
    dhead ("CTransferChannel::InfoInd", DCON_CTransferChannel);
    dparams ("%x", Params->InfoNumber);
    dassert (Params != 0);
    // at the moment we only used this call for DDI
    switch (Params->InfoNumber) {
    case 0x70: // called party number info element
        if (GetState() != cs_D_ConnectIndication) {
            break;
        } else {
            dassert (Params->pInfoElement != 0);
            dassert ((Params->pInfoElement->GetLen() == 0) || (Params->pInfoElement->GetData() != 0));
            //TODO: more checking if DDI is active!
            if (Params->pInfoElement->GetLen() <= 1) {
                break;
            }
            if (m_CalledPartyNum.Append (Params->pInfoElement->GetData() + 1, Params->pInfoElement->GetLen() - 1) == vFalse) {
                dwarning (0);
                m_CalledPartyNum.RemoveAll();
                cm_connectresp crIgnore;
                ConnectResp (&crIgnore);
                break;
            }
        }
        // no break!
    case 0xA1: // sending complete info element
        dprintBuf (m_CalledPartyNum.GetPointer(), m_CalledPartyNum.GetLen());
        dassert (m_ActiveController != IllegalController);
        if (  (GetState() == cs_D_ConnectIndication)
           && (m_CalledPartyNum.GetLen() > m_DDIPrefixLen)
           && (m_pMSNList != 0)) {
            CConstString ddinum (m_CalledPartyNum.GetPointer() + m_DDIPrefixLen, m_CalledPartyNum.GetLen() - m_DDIPrefixLen);
            switch (m_pMSNList->TestDDINumber (m_ActiveController, &ddinum)) {
            case iWrn_MSNList_DDICall:
                // DDI Number not complete received -> wait for more numbers
                break;
            case i_Okay:
                if (AcceptIncomingCall() == vTrue) {
                    m_DDIPrefixLen = 0;
                    IncomingCall (m_CIPValue, &m_CallingNumber, &ddinum);
                }
                break;
            case iWrn_MSNList_RejectCall: {
                m_CalledPartyNum.RemoveAll();
                cm_connectresp crIgnore;
                ConnectResp (&crIgnore);
                break;
            }
            default:
                dassert (0);
                break;
            }
        }
        break;
    }
}


/*===========================================================================*\
    CTransferChannel::ManufacturerInd
\*===========================================================================*/

void CTransferChannel::ManufacturerInd (c_dword, cm_manufacturer *Params) {
    dhead ("CTransferChannel::ManufacturerInd", DCON_CTransferChannel);
    dassert (Params != 0);
    ManufacturerNotify (Params->Function, Params->pManuData);
}


/*===========================================================================*\
    CTransferChannel::fConf_PMWakeup
\*===========================================================================*/

void CTransferChannel::fConf_PMWakeup (c_dword Controller, c_info Info, c_word NumberAccepted) {
    dhead ("CTransferChannel::fConf_PMWakeup", DCON_CTransferChannel);
    dwarning (m_pMSNList != 0);
    CCntrlMSNList *pTmpMSNList = m_pMSNList;
    if (pTmpMSNList != 0) {
        if (CapiInfoIsSuccess (Info) == vTrue) {
            if (NumberAccepted && (NumberAccepted < pTmpMSNList->GetCountMSN (Controller))) {
                dwarning (0);
                fReq_PMWakeup (Controller, 0, 0);
            }
        } else {
            dprint ("fConf_PMWakeup returned with %x. ", Info);
            pTmpMSNList->SetWakeupSupport (Controller, vFalse);
        }
    }
}


/*===========================================================================*\
    CTransferChannel::ResetSendBlock
\*===========================================================================*/

void CTransferChannel::ResetSendBlock (void) {
    dhead ("CTransferChannel::ResetSendBlock", DCON_CTransferChannel);
    void *DelHandle;
    while (m_SendBlock.DelNextEntry (&DelHandle) == vTrue) {
        PutDataComplete (DelHandle, ci_Int_NotConnected);
    }
}


/*===========================================================================*\
    CTransferChannel::CheckFinishReset
\*===========================================================================*/

tUInt CTransferChannel::CheckFinishReset (tUInt NewResetState) {
    dhead ("CTransferChannel::CheckFinishReset", DCON_CTransferChannel);
    dassert (GetState() == cs_Released);
    GetProtectClass()->BeginWrite();
    if (CheckBitsOne (m_Reseting, CHANNEL_STATUS_RESET) == vFalse) {
        m_Reseting = NewResetState;
        GetProtectClass()->EndWrite();
        RETURN ('x', 0);
    }
    if (m_CheckListenInUse == 0) {
        m_Reseting = CHANNEL_STATUS_INIT;
        GetProtectClass()->EndWrite();
        IsReseted();
        RETURN ('x', 2);
    }
    GetProtectClass()->EndWrite();
    RETURN ('x', 1);
}


/*===========================================================================*\
\*===========================================================================*/
