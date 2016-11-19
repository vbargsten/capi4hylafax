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

#ifndef _CAPISTAT_H_
#define _CAPISTAT_H_

#include "CapiLay.h"
#include "ProtClas.h"

/*===========================================================================*\
\*===========================================================================*/

class CCAPI20_Statistic : private CCAPI20_Layer {
public:
    enum ctStatDeep {
        StatDeep_OnlyInstall,
        StatDeep_CountCntrl,
        StatDeep_CntrlProps,
        StatDeep_DSLProps,
        StatDeep_CntrlFacilities
    };

    enum ctGlobalOptionsFlag {
        gof_InternalController    = 0,
        gof_ExternalController    = 1,
        gof_Handset               = 2,
        gof_DTMF                  = 3,
        gof_SupplementaryServices = 4,
        gof_ChannelAllocation     = 5,
        gof_BChannelOperation     = 6
    };

    enum ctSupplServicesFlag {
        ssf_HoldRetrieve          = 0,
        ssf_TerminalPortability   = 1,
        ssf_ECT                   = 2,
        ssf_3PTY                  = 4,
        ssf_CallForwarding        = 5,
        ssf_CD                    = 6,
        ssf_MCID                  = 7,
        ssf_CCBS                  = 8
    };

    CCAPI20_Statistic (ctStatDeep statdeep = StatDeep_CntrlFacilities);
    virtual ~CCAPI20_Statistic (void);

    void FreshUp (tBool FreshUpOnError = vTrue);
    virtual void FreshUpComplete (c_info Info);
    void SetFreshUpCallBack (void (*CallBackFunc) (void *Handle, c_info Info), void *Handle);
    tBool FreshUpInUse (void);
    void FreshUpCancel (void);

    c_info CapiInstalled (void);

    tUInt CountControllers (void);
    tUInt CountBChannels (tUInt Controller);

    c_dword Profile_GlobalOptions (tUInt Controller);
    c_dword Profile_B1Protocol (tUInt Controller);
    c_dword Profile_B2Protocol (tUInt Controller);
    c_dword Profile_B3Protocol (tUInt Controller);
    c_dword SupplServices (tUInt Controller);

    tUInt DSL_FindController (tUInt Controller = 0); // Start search at (Controller + 1)
    tUInt DSL_GetModemState (tUInt Controller);
    tUInt DSL_GetSpeed (tUInt Controller);
    tBool DSL_GetMACAddress (tUInt Controller, tUByte *pAddress); // sizeof (pAddress) >= 6

private:
    void REGISTER_COMPLETE (c_info ErrorCode);
    void RELEASE_COMPLETE (void);
    void GET_PROFILE_COMPLETE (tUInt Controller, cm_getprofile *pProfile, c_info ErrorCode);
    void FACILITY_CONF (c_word Messagenumber, c_dword Address, cm_facilityconf *Params);
    void MANUFACTURER_CONF (c_word Messagenumber, c_dword Controller, cm_manufacturer *Params);
    tBool MANUFACTURER_IND (c_word Messagenumber, c_dword Controller, cm_manufacturer *Params);

    void DSLManuReq (tUInt Controller, CCStruct *pManuData);
    void CallFinished (void);
    void UpdateCurrent (c_info Info);

    struct ctIntCntrlData {
        tUInt   NumBChannels;
        c_dword GlobalOptions;
        c_dword B1ProtocolSupport;
        c_dword B2ProtocolSupport;
        c_dword B3ProtocolSupport;
        c_dword SupplServices;
        tUInt   DSL_ModemState;
        tUInt   DSL_Speed;
        tUByte  DSL_MACAddress [6];
        tUByte  DSL_Retries;
    
        ctIntCntrlData() : 
           NumBChannels(0),
           GlobalOptions(0),
           B1ProtocolSupport(0),
           B2ProtocolSupport(0),
           B3ProtocolSupport(0),
           SupplServices(0),
           DSL_ModemState(0),
           DSL_Speed(0),
           DSL_Retries(0) {
            for (unsigned int i=0; i<6; i++) {
                DSL_MACAddress[i] = 0;
            }        
        }
    
    };
    struct ctIntCapiData {
        c_info          CapiInstalled;
        tUInt           CountControllers;
        ctIntCntrlData *pControllerData;
    };

    CProtectClass   m_Protect;
    ctIntCapiData  *m_pCurrentData;
    ctIntCapiData  *m_pFreshingData;
    tUInt           m_FreshingController;
    ctStatDeep      m_Deep;
    tUInt           m_CountReqCalls;
    void          (*m_CallBackFunc) (void *, c_info);
    void           *m_CallBackHandle;
    tBool           m_FreshOnError;
};


/*===========================================================================*\
\*===========================================================================*/

inline void CCAPI20_Statistic::SetFreshUpCallBack (void (*CallBackFunc) (void *, c_info), void *Handle) {
    m_CallBackHandle = Handle;
    m_CallBackFunc   = CallBackFunc;
}

inline tBool CCAPI20_Statistic::FreshUpInUse (void) {
    return (m_pFreshingData != 0);
}

inline void CCAPI20_Statistic::FreshUpCancel (void) {
    if (m_pFreshingData != 0) {
        RELEASE();
    }
}

/*===========================================================================*\
\*===========================================================================*/

#endif

