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

#ifndef _FACILITY_H_
#define _FACILITY_H_

#include "CapiChan.h"
#include "CCSFacil.h"

#define USE_SUPPLEMENTARY_SERVICES

/*===========================================================================*\
\*===========================================================================*/

class CCAPI20_Facility : protected CCAPI20_Channel {
public:
    /*-----------------------------------------------------------------------*\
        Facility Req, Conf & Ind except Supplementary Services
    \*-----------------------------------------------------------------------*/
    void fReq_DTMF (c_dtmffunction  Function,
                    c_word          ToneDuration,
                    c_word          GapDuration,
                    CDynamicString *pDTMFDigits,
                    CCStruct       *pDTMFCharacteristics);

    virtual void fConf_DTMF (c_info /*Info*/) {}
    // DTMF information mapping into c_info:
    //      0 = ci_NoError
    //      1 = ci_Int_IncorrectDTMFDigit
    //      2 = ci_Int_UnknownDTMFRequest

    virtual void fInd_DTMF (CCStruct * /*pDTMFdigits*/) {}

    /*-----------------------------------------------------------------------*/

    void fReq_PMWakeup (c_dword Controller, c_word NumberRequested, cp_facilityreq_wakeupparam *NumberList);

    virtual void fConf_PMWakeup (c_dword /*Controller*/, c_info /*Info*/, c_word /*NumberAccepted*/) {}


    /*-----------------------------------------------------------------------*\
        Supplementary Services - REQUEST
    \*-----------------------------------------------------------------------*/
#ifdef USE_SUPPLEMENTARY_SERVICES
    void fReq_Listen (c_dword Controller, c_dword NotificationMask);
    void fReq_Hold (void);
    void fReq_Retrieve (void);
    void fReq_Suspend (CDynamicString *pCallIdentity);
    void fReq_Resume (c_dword Controller, CDynamicString *pCallIdentity);
    void fReq_ECT (CCAPI20_Channel *pChannel);
    void fReq_3PTY_Begin (CCAPI20_Channel *pChannel);
    void fReq_3PTY_End (CCAPI20_Channel *pChannel);
    void fReq_CD (tBool PresentationAllowed, CDynamicString *pDeflectToNumber);
    void fReq_MCID (void);
    //void fReq_CCBS_Request (???);
    //void fReq_CCBS_Deactivate (???);
    //void fReq_CCBS_Interrogate (???);
    //void fReq_CCBS_Call (c_cipvalue CIPValue, cm_bprotocol BProtocol);

    /*-----------------------------------------------------------------------*\
        Supplementary Services - CONFIRMATION
    \*-----------------------------------------------------------------------*/
    virtual void fConf_Listen (c_info /*Info*/) {}

    /*-----------------------------------------------------------------------*\
        Supplementary Services - INDICATION
    \*-----------------------------------------------------------------------*/
    virtual void fInd_Hold (c_info /*Reason*/) {}
    virtual void fInd_Retrieve (c_info /*Reason*/) {}
    virtual void fInd_Suspend (c_info /*Reason*/) {}
    virtual void fInd_Resume (c_info /*Reason*/) {}
    virtual void fInd_ECT (c_info /*Reason*/) {}
    virtual void fInd_3PTY_Begin (c_info /*Reason*/) {}
    virtual void fInd_3PTY_End (c_info /*Reason*/) {}
    virtual void fInd_CD (c_info /*Reason*/) {}
    virtual void fInd_MCID (c_info /*Reason*/) {}
    //virtual void fInd_CCBS_Request (c_info /*Reason*/, ???);
    //virtual void fInd_CCBS_Deactivate (c_info /*Reason*/, ???);
    //virtual void fInd_CCBS_Interrogate (c_info /*Reason*/, ???);
    //virtual void fInd_CCBS_Call (c_info /*Reason*/);
    virtual void fInd_Hold_Notify (void) {}
    virtual void fInd_Retrieve_Notify (void) {}
    virtual void fInd_Suspend_Notify (void) {}
    virtual void fInd_Resume_Notify (void) {}
    virtual void fInd_CallIsDiverting_Notify (void) {}
    virtual void fInd_DiversionActive_Notify (void) {}
    //virtual void fInd_DiversionInformation_Notify
    //virtual void fInd_CT_Alerted_Notify
    //virtual void fInd_CT_Active_Notify
    virtual void fInd_ConferenceEstablished (void) {}
    virtual void fInd_ConferenceDisconnected (void) {}
    //virtual void fInd_CCBS_EraseCallLinkageID (???);
    //virtual void fInd_CCBS_Status (???);
    //virtual void fInd_CCBS_RemoteUserFree (???);
    //virtual void fInd_CCBS_BFree (???);
    //virtual void fInd_CCBS_Erase (???);
    //virtual void fInd_CCBS_StopAlerting (???);
    //virtual void fInd_CCBS_InfoRetain (???);

    /*-----------------------------------------------------------------------*\
        Supplementary Services - RESPONSES
    \*-----------------------------------------------------------------------*/

private:
    /*-----------------------------------------------------------------------*\
        internal functions for Supplementary Services
    \*-----------------------------------------------------------------------*/
    void CallSupplServFacilityReq (c_dword Address, c_supplserv Function, CCStruct *pSupplService);
    tBool HandleSupplServicesInd (c_dword Address, c_supplserv Function, CCStruct *pParams);
#endif // #ifdef USE_SUPPLEMENTARY_SERVICES

private:
    /*-----------------------------------------------------------------------*\
        virtual functions from CCAPI20_ChannelForManager
    \*-----------------------------------------------------------------------*/
    void FACILITY_CONF (c_word Messagenumber, c_dword Address, cm_facilityconf *Params);
    tBool FACILITY_IND (c_word Messagenumber, c_dword Address, cm_facilityind *Params);
};


/*===========================================================================*\
    CCAPI20_Facility - inline functions
\*===========================================================================*/
#ifdef USE_SUPPLEMENTARY_SERVICES

inline void CCAPI20_Facility::CallSupplServFacilityReq (c_dword Address, c_supplserv Function, CCStruct *pSupplService) {
    cp_facility_supplserv supplserv (Function, pSupplService);
    cm_facilityreq        cpfac (facilselect_SupplServices, &supplserv);
    FACILITY_REQ (GetNextMsgNr(), Address, &cpfac);
}

#endif
/*===========================================================================*\
\*===========================================================================*/

#endif

