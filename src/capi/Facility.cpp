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

#include "Facility.h"
#include "aString.h"
#include "dbgCAPI.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define MAX_CALL_IDENTITY_LEN           100     // < 250 !!!

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::fReq_DTMF (c_dtmffunction Function, c_word ToneDuration, c_word GapDuration,
                                  CDynamicString *pDTMFDigits, CCStruct *pDTMFCharacteristics) {
    dhead ("CCAPI20_Facility::fReq_DTMF", DCON_CCAPI20_Facility);
    // generate cm_facility struct
    cp_facilityreq_dtmf fdtmf ((c_word)Function, ToneDuration, GapDuration, pDTMFDigits, pDTMFCharacteristics);
    cm_facilityreq      cpfac (facilselect_DTMF, &fdtmf);
    c_dword             localNCCI = GetNCCI();

    if (GetState() != cs_Connected) {
        fConf_DTMF (ci_Int_NotConnected);
        return;
    }
    dassert (localNCCI != IllegalNCCI);
    FACILITY_REQ (GetNextMsgNr(), localNCCI, &cpfac);
}

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::fReq_PMWakeup (c_dword Controller, c_word NumberRequested, cp_facilityreq_wakeupparam *NumberList) {
    dhead ("CCAPI20_Facility::fReq_PMWakeup", DCON_CCAPI20_Facility);
    dassert ((NumberRequested == 0) || (NumberList != 0));
    // calculate needed size and allocate it
    cm_facilityreq cpfac (facilselect_PMWakeup);
    if (!NumberRequested) {
        // empty struct
        CCStruct frWakeUp ( "\x00\x00\x00", 3);
        cpfac.pFacilityParam = &frWakeUp;
        FACILITY_REQ (GetNextMsgNr(), Controller, &cpfac);
    } else {
        cp_facilityreq_wakeup frWakeUp (NumberRequested, NumberList);
        cpfac.pFacilityParam = &frWakeUp;
    }
}

/*===========================================================================*\
\*===========================================================================*/
#ifdef USE_SUPPLEMENTARY_SERVICES

void CCAPI20_Facility::fReq_Listen (c_dword Controller, c_dword NotificationMask) {
    dhead ("CCAPI20_Facility::fReq_Listen", DCON_CCAPI20_Facility);
    cp_content_dword fparam (NotificationMask);
    CallSupplServFacilityReq (Controller, supplserv_Listen, &fparam);
}

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::fReq_Hold (void) {
    dhead ("CCAPI20_Facility::fReq_Hold", DCON_CCAPI20_Facility);
    if (SetState (cs_HoldPending, cs_D_Connected) == vTrue) {
        CallSupplServFacilityReq (GetPLCI(), supplserv_Hold, 0);
    } else {
        dwarning (0);
        fInd_Hold (ci_Int_IllegalState);
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::fReq_Retrieve (void) {
    dhead ("CCAPI20_Facility::fReq_Retrieve", DCON_CCAPI20_Facility);
    if (SetState (cs_RetrievePending, cs_Holded) == vTrue) {
        CallSupplServFacilityReq (GetPLCI(), supplserv_Retrieve, 0);
    } else {
        dwarning (0);
        fInd_Retrieve (ci_Int_IllegalState);
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::fReq_Suspend (CDynamicString *pCallIdentity) {
    dhead ("CCAPI20_Facility::fReq_Suspend", DCON_CCAPI20_Facility);
    dassert (pCallIdentity != 0);
    if (SetState (cs_SuspendPending, cs_D_Connected) == vTrue) {
        cp_content_string fparam (pCallIdentity);
        CallSupplServFacilityReq (GetPLCI(), supplserv_Suspend, &fparam);
    } else {
        dwarning (0);
        fInd_Suspend (ci_Int_IllegalState);
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::fReq_Resume (c_dword Controller, CDynamicString *pCallIdentity) {
    dhead ("CCAPI20_Facility::fReq_Resume", DCON_CCAPI20_Facility);
    dassert (pCallIdentity != 0);
    if (SetState (cs_ResumePending, cs_Registered) == vTrue) {
        cp_content_string fparam (pCallIdentity);
        CallSupplServFacilityReq (Controller, supplserv_Resume, &fparam);
    } else {
        dwarning (0);
        fInd_Resume (ci_Int_IllegalState);
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::fReq_ECT (CCAPI20_Channel * /*pChannel*/) {
    dhead ("CCAPI20_Facility::fReq_ECT", DCON_CCAPI20_Facility);
    //dassert (pChannel != 0);
    dwarning (GetState() == cs_Holded);
    cp_content_dword fparam (GetPLCI());
    CallSupplServFacilityReq (GetPLCI(), supplserv_ECT, &fparam);
}

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::fReq_3PTY_Begin (CCAPI20_Channel * /*pChannel*/) {
    dhead ("CCAPI20_Facility::fReq_3PTY_Begin", DCON_CCAPI20_Facility);
    //dassert (pChannel != 0);
    dwarning (GetState() == cs_Holded);
    cp_content_dword fparam (GetPLCI());
    CallSupplServFacilityReq (GetPLCI(), supplserv_3PTY_Begin, &fparam);
}

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::fReq_3PTY_End (CCAPI20_Channel * /*pChannel*/) {
    dhead ("CCAPI20_Facility::fReq_3PTY_End", DCON_CCAPI20_Facility);
    //dassert (pChannel != 0);
    dwarning (GetState() == cs_Holded);
    cp_content_dword fparam (GetPLCI());
    CallSupplServFacilityReq (GetPLCI(), supplserv_3PTY_End, &fparam);
}

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::fReq_CD (tBool PresentationAllowed, CDynamicString *pDeflectToNumber) {
    dhead ("CCAPI20_Facility::fReq_CD", DCON_CCAPI20_Facility);
    if (SetState (cs_CallDeflecting, cs_D_ConnectIndication) == vTrue) {
        cp_supplserv_cdreq sscd ((PresentationAllowed == vTrue) ? (c_word)1 : (c_word)0, pDeflectToNumber);
        CallSupplServFacilityReq (GetPLCI(), supplserv_CD, &sscd);
    } else {
        dwarning (0);
        fInd_CD (ci_Int_IllegalState);
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::fReq_MCID (void) {
    dhead ("CCAPI20_Facility::fReq_MCID", DCON_CCAPI20_Facility);
    CallSupplServFacilityReq (GetPLCI(), supplserv_MCID, 0);
}

/*===========================================================================*\
\*===========================================================================*/

tBool CCAPI20_Facility::HandleSupplServicesInd (c_dword Address, c_supplserv Function, CCStruct *pParams) {
    dhead ("CCAPI20_Facility::HandleSupplServicesInd", DCON_CCAPI20_Facility);
    dparams ("%x,%x", Address, Function);
    dassert (pParams != 0);
    if (Address != GetPLCI()) {
        if (Address > CAPI_MAX_CONTROLLER_NUMBER) {
            // Address must be a PLCI or NCCI => This Address is definitly not for this class
            RETURN ('x', vFalse);
        }
    }
    {
        // TODO: für forwarding, CCBS ist das hier zu allgemeingültig!
        cp_facility_supplserv supplserv (Function);
        cm_facilityresp fresp (facilselect_SupplServices, &supplserv);
        FACILITY_RESP (GetNextMsgNr(), Address, &fresp);
    }
    c_info Info = ci_NoError;
    if (pParams->GetLen() >= sizeof (c_word)) {
        Info = (c_info)get16 (pParams->GetData());
        dprint ("Info=%x ", Info);
    }
    switch (Function) {
    case supplserv_Hold:
        if (CapiInfoIsSuccess (Info)) {
            if (SetState (cs_Holded, cs_HoldPending) == vTrue) {
                dassert (GetNCCI() == IllegalNCCI);
                fInd_Hold (Info);
            }
        } else if (SetState (cs_D_Connected, cs_HoldPending) == vTrue) {
            dassert (GetNCCI() == IllegalNCCI);
            ConnectB3Req();
            fInd_Hold (Info);
        }
        break;
    case supplserv_Retrieve:
        if (CapiInfoIsSuccess (Info)) {
            dassert (GetNCCI() == IllegalNCCI);
            if (SetState (cs_D_Connected, cs_RetrievePending) == vTrue) {
                ConnectB3Req();
                fInd_Retrieve (Info);
            }
        } else if (SetState (cs_Holded, cs_RetrievePending) == vTrue) {
            dassert (GetNCCI() == IllegalNCCI);
            fInd_Retrieve (Info);
        }
        break;
    case supplserv_Suspend:
        if (CapiInfoIsSuccess (Info)) {
            if (SetState (cs_D_DisconnectPending, cs_SuspendPending) == vTrue) {
                fInd_Suspend (Info);
            }
        } else if (SetState (cs_D_Connected, cs_SuspendPending) == vTrue) {
            ConnectB3Req();
            fInd_Suspend (Info);
        }
        break;
    case supplserv_Resume:
        if (CapiInfoIsSuccess (Info)) {
            if (SetState(cs_D_Connected, cs_ResumePending) == vTrue) {
                fInd_Resume (Info);
                if (GetState() == cs_D_Connected) {
                    ConnectB3Req();
                }
            }
        } else {
            DisconnectReq();
            fInd_Resume (Info);
        }
        break;
    case supplserv_ECT:
        if (GetState() == cs_Holded) {
            fInd_ECT (Info);
        }
        break;
    case supplserv_3PTY_Begin:
        if (GetState() == cs_Holded) {
            fInd_3PTY_Begin (Info);
        }
        break;
    case supplserv_3PTY_End:
        if (GetState() >= cs_D_Connected) {
            fInd_3PTY_End (Info);
        }
        break;
    case supplserv_CD:
        if (SetState ((CapiInfoIsError (Info) == vTrue) ? cs_D_ConnectIndication : cs_D_DisconnectPending,
                      cs_CallDeflecting) == vTrue) {
            fInd_CD (Info);
        }
        break;
    case supplserv_MCID:
        fInd_MCID (Info);
        break;
    //case supplserv_CCBS_Request:
    //case supplserv_CCBS_Deactivate:
    //case supplserv_CCBS_Interrogate:
    //case supplserv_CCBS_Call:
    //    dassert (pParams->GetLen) >= sizeof (c_dword);
    //    fInd_CCBS_Call (*(c_word *)pParams->GetData());
    //    break;
    case supplserv_Hold_Notify:
        fInd_Hold_Notify();
        break;
    case supplserv_Retrieve_Notify:
        fInd_Retrieve_Notify();
        break;
    case supplserv_Suspend_Notify:
        fInd_Suspend_Notify();
        break;
    case supplserv_Resume_Notify:
        fInd_Resume_Notify();
        break;
    case supplserv_CallIsDiverting_Notify:
        fInd_CallIsDiverting_Notify();
        break;
    //case supplserv_DiversionActive_Notify:
    //    fInd_DiversionActive_Notify();
    //    break;
    case supplserv_ConferenceEstablished:
        fInd_ConferenceEstablished();
        break;
    case supplserv_ConferenceDisconnected:
        fInd_ConferenceDisconnected();
        break;
    //case supplserv_CCBS_EraseCallLinkID:
    //case supplserv_CCBS_Status:
    //case supplserv_CCBS_RemoteUserFree:
    //case supplserv_CCBS_B_Free:
    //case supplserv_CCBS_Erase:
    //case supplserv_CCBS_StopAlerting:
    //case supplserv_CCBS_InfoRetain:
    default:
        dprintWrn ("Unhandled SupplServInd(%x,a=%x) ", Function, Address);
        break;
    }
    RETURN ('x', vTrue);
}

#endif // #ifdef USE_SUPPLEMENTARY_SERVICES
/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Facility::FACILITY_CONF (c_word, c_dword Address, cm_facilityconf *Params) {
    dhead ("CCAPI20_Facility::FACILITY_CONF", DCON_CCAPI20_Facility);
    dprint ("%x", Params->FacilitySelector);
    dassert (Params != 0);
    dassert (Params->pFacilityParam != 0);
    dassert (sizeof (void *) == sizeof (c_dword)); // see fReq_Activate/_Deactivate
    // todo: c_dword is uint32 -> wont be sizeof void ptr on 64bit platform
    // there is no function fReq_Activate/_Deactivate
    switch (Params->FacilitySelector) {
    case facilselect_DTMF: {
        c_info di = Params->Info;
        dwarning (Params->pFacilityParam->GetLen() >= sizeof (c_word));
        if (  (CapiInfoIsSuccess (di) == vTrue)
           && (Params->pFacilityParam->GetLen() >= sizeof (c_word))) {
            switch (get16 (Params->pFacilityParam->GetData())) {
            case 0:
                // nothing to do
                break;
            case 1:
                di = ci_Int_IncorrectDTMFDigit;
                break;
            default:
                dassert (0);
                // no Break!
            case 2:
                di = ci_Int_UnknownDTMFRequest;
                break;
            }
        }
        fConf_DTMF (di);
        break;
    }
#ifdef USE_SUPPLEMENTARY_SERVICES
    case facilselect_SupplServices: {
        cp_facility_supplserv_ref fss;
        if (infoSuccess (fss.ParseStruct (Params->pFacilityParam))) {
            dassert (fss.pParameter != 0);
            c_info Info = (fss.pParameter->GetLen() >= sizeof (c_word)) ?
                          (c_info)get16 (fss.pParameter->GetData()) : Params->Info;
            dprint ("SupplServ=%x,Info=%x ", fss.Function, Info);
            if (CapiInfoIsSuccess (Info) == vTrue) {
                if (fss.Function == supplserv_Resume) {
                    SetPLCI (Address);
                }
            } else {
                // error handling
                switch (fss.Function) {
                case supplserv_Listen:
                    fConf_Listen (Info);
                    break;
                case supplserv_Hold:
                    dassert (GetPLCI() == Address);
                    dassert (GetState() == cs_HoldPending);
                    if (SetState (cs_D_Connected, cs_HoldPending) == vTrue) {
                        ConnectB3Req();
                        fInd_Hold (Info);
                    }
                    break;
                case supplserv_Retrieve:
                    dassert (GetPLCI() == Address);
                    dassert (GetState() == cs_RetrievePending);
                    if (SetState (cs_Holded, cs_RetrievePending) == vTrue) {
                        fInd_Retrieve (Info);
                    }
                    break;
                case supplserv_Suspend:
                    dassert (GetPLCI() == Address);
                    dassert (GetState() == cs_SuspendPending);
                    if (SetState (cs_D_Connected, cs_SuspendPending) == vTrue) {
                        ConnectB3Req();
                        fInd_Suspend (Info);
                    }
                    break;
                case supplserv_Resume:
                    dassert (Address != IllegalPLCI);
                    if (SetState (cs_Registered, cs_ResumePending) == vTrue) {
                        fInd_Resume (Info);
                    }
                    break;
                case supplserv_ECT:
                    fInd_ECT (Info);
                    break;
                case supplserv_3PTY_Begin:
                    fInd_3PTY_Begin (Info);
                    break;
                case supplserv_3PTY_End:
                    fInd_3PTY_End (Info);
                    break;
                case supplserv_CD:
                    dassert (GetPLCI() == Address);
                    dassert (GetState() == cs_CallDeflecting);
                    if (SetState (cs_D_ConnectIndication, cs_CallDeflecting) == vTrue) {
                        fInd_CD (Info);
                    }
                    break;
                case supplserv_MCID:
                    fInd_MCID (Info);
                    break;
                case supplserv_CCBS_Request:
                    //fInd_CCBS_Request (Info);
                    break;
                case supplserv_CCBS_Deactivate:
                    //fInd_CCBS_Deactivate (Info);
                    break;
                case supplserv_CCBS_Interrogate:
                    //fInd_CCBS_Interrogate (Info);
                    break;
                case supplserv_CCBS_Call:
                    //fInd_CCBS_Call (Info);
                    break;
                default:
                    dprintErr ("Illegal SupplServ=%x ", fss.Function);
                    dassert (0);
                    break;
                }
            }
        } else {
            dassert (0);
        }
        break;
    }
#endif // #ifdef USE_SUPPLEMENTARY_SERVICES
    case facilselect_PMWakeup:
        dprint ("FP=%x ", Params->pFacilityParam);
        dassert (  (CapiInfoIsSuccess (Params->Info) == vFalse)
                || (Params->pFacilityParam->GetLen() >= sizeof (c_word)));
        fConf_PMWakeup (Address, Params->Info,
                        (  (CapiInfoIsSuccess (Params->Info) == vTrue)
                        && (Params->pFacilityParam->GetLen() >= sizeof (c_word))) ?
                                get16 (Params->pFacilityParam->GetData()) : (tUShort)0);
        break;
    default:
        dwarning (0);
        break;
    }
}

/*===========================================================================*\
\*===========================================================================*/

tBool CCAPI20_Facility::FACILITY_IND (c_word, c_dword Address, cm_facilityind *Params) {
    dhead ("CCAPI20_Facility::FACILITY_IND", DCON_CCAPI20_Facility);
    dassert (Params != 0);
    dassert (Params->pFacilityParam != 0);
    dassert (sizeof (void *) == sizeof (c_dword)); // see fReq_Activate/_Deactivate
    tBool fret = vFalse;
    switch (Params->FacilitySelector) {
    case facilselect_DTMF:
        if ((Address == GetPLCI()) && (Params->pFacilityParam->GetLen() > 0)) {
            cm_facilityresp fresp (facilselect_DTMF, 0);
            FACILITY_RESP (GetNextMsgNr(), Address, &fresp);
            fInd_DTMF (Params->pFacilityParam);
            fret = vTrue;
        }
        break;
#ifdef USE_SUPPLEMENTARY_SERVICES
    case facilselect_SupplServices: {
        cp_facility_supplserv_ref fss;
        fret = infoSuccess (fss.ParseStruct (Params->pFacilityParam));
        if (fret == vTrue) {
            fret = HandleSupplServicesInd (Address, fss.Function, fss.pParameter);
        }
        break;
    }
#endif
    default:
        // illegal Selector
        dwarning (0);
        // no break!
    case facilselect_Handset:
    case facilselect_V42Compression:
        // not supported now
        break;
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/
