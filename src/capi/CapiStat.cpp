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

#include "CapiStat.h"
#include "CCSFacil.h"
#include "aString.h"
#include "aFlags.h"
#include "dbgCAPI.h"

/*---------------------------------------------------------------------------*\
#define   MODEM_STARTED                     1
#define   MODEM_SYNCED                      2
#define   MODEM_SELFTEST_FAILED             3
#define   MODEM_RETRAIN                     4
#define   MODEM_DATAPHASE                   8

#define   MODEM_RETRAIN_OPTIONS_FAILED      16
#define   MODEM_RETRAIN_TIMEOUT             17
#define   MODEM_RETRAIN_WRONG_CRC           18
#define   MODEM_RETRAIN_LOF                 19
#define   MODEM_RETRAIN_LOS                 20
#define   MODEM_RETRAIN_BITSRATE_TOO_HIGH   21
#define   MODEM_RETRAIN_SELFTEST            22
\*---------------------------------------------------------------------------*/

#define CAPI_MANU_ID_AVM            0x214D5641
#define CAPI_MANU_FUNCTION_ADSL     0x07

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/
#pragma pack(1)

class cp_Manu_DSLParameter : public CCStruct {
public:
    cp_Manu_DSLParameter (void)
      : CCStruct (cxt_Manufacturer, &ModemState, 6, cxf_04 cxf_STRUCT cxf_STRUCT),
        ModemState (0),
        pIdentification (&m_Identification),
        pLineParameters (&m_LineParameters) {}

    c_dword   ModemState;
    CCStruct *pIdentification;
    CCStruct *pLineParameters;

private:
    CCStruct  m_Identification;
    CCStruct  m_LineParameters;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

struct cManu_LineParameters {
    c_word AttainableKBitrateUS;           /* in KBit/s */
    c_word AttainableKBitrateDS;           /* in KBit/s */
    c_word UsedOptionUS;                   /* [-1,3] */
    c_word UsedOptionDS;                   /* [-1,3] */
    c_byte US_FastUsed[4];                 /* bool */
    c_word US_DatabytesPerSymbol[4];       /* dezimal */
    c_byte US_CheckbytesPerCodeword[4];    /* dezimal */
    c_byte US_SymbolsPerCodeword[4];       /* dezimal */
    c_word US_InterleaveDepth[4];          /* dezimal */
    c_byte DS_FastUsed[4];                 /* bool */
    c_word DS_DatabytesPerSymbol[4];       /* dezimal */
    //c_byte DS_CheckbytesPerCodeword[4];    /* dezimal */
    //c_byte DS_SymbolsPerCodeword[4];       /* dezimal */
    //c_word DS_InterleaveDepth[4];          /* dezimal */
};


#pragma pack()
/*===========================================================================*\
\*===========================================================================*/

CCAPI20_Statistic::CCAPI20_Statistic (ctStatDeep statdeep)
  : m_pCurrentData (0),
    m_pFreshingData (0),
    m_FreshingController (0),
    m_Deep (statdeep),
    m_CountReqCalls (0),
    m_CallBackFunc (0),
    m_CallBackHandle (0),
    m_FreshOnError (vFalse) {

    dhead ("CCAPI20_Statistic-Constructor", DCON_CCAPI20_Statistic);
    m_Protect.Init();
}

/*===========================================================================*\
\*===========================================================================*/

CCAPI20_Statistic::~CCAPI20_Statistic (void) {
    dhead ("CCAPI20_Statistic-Destructor", DCON_CCAPI20_Statistic);
    dassert (m_pFreshingData == 0);
    if (m_Protect.IsInitialized()) {
        m_Protect.BeginWrite();
        ctIntCapiData *pDelData = m_pCurrentData;
        m_pCurrentData          = 0;
        m_Protect.EndWrite();
        if (pDelData) {
            delete [] pDelData->pControllerData;
            delete pDelData;
        }
    } else {
        dassert (m_pCurrentData == 0);
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Statistic::FreshUp (tBool FreshUpOnError) {
    dhead ("CCAPI20_Statistic::FreshUp", DCON_CCAPI20_Statistic);
    if (m_Protect.Init()) {
        m_Protect.BeginWrite();
        if (!m_pFreshingData) {
            m_pFreshingData  = new ctIntCapiData;
            m_Protect.EndWrite();
            dassert (m_pFreshingData != 0);   // this case not handled at the moment
            dassert (m_CountReqCalls == 0);
            if (m_pFreshingData) {
                m_pFreshingData->CapiInstalled    = ci_Reg_NotInstalled;
                m_pFreshingData->CountControllers = 0;
                m_pFreshingData->pControllerData  = 0;
                m_FreshingController              = 0;
                m_CountReqCalls                   = 0;
                m_FreshOnError                    = FreshUpOnError;
                REGISTER (1, 2, CAPI_MAX_MESSAGE_SIZE);
            } else {
                FreshUpComplete (ci_Int_MemoryFullErr);
            }
        } else {
            m_Protect.EndWrite();
        }
    } else {
        FreshUpComplete (ci_Int_MemoryFullErr);
    }
}


/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Statistic::FreshUpComplete (c_info Info) {
    dhead ("CCAPI20_Statistic::FreshUpComplete", DCON_CCAPI20_Statistic);
    if (m_CallBackFunc) {
        m_CallBackFunc (m_CallBackHandle, Info);
    }
}


/*===========================================================================*\
\*===========================================================================*/

c_info CCAPI20_Statistic::CapiInstalled (void) {
    dhead ("CCAPI20_Statistic::CapiInstalled", DCON_CCAPI20_Statistic);
    c_info fret = ci_Reg_Busy;
    m_Protect.BeginRead();
    if (m_pCurrentData) {
        fret = m_pCurrentData->CapiInstalled;
    }
    m_Protect.EndRead();
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

tUInt CCAPI20_Statistic::CountControllers (void) {
    dhead ("CCAPI20_Statistic::CountControllers", DCON_CCAPI20_Statistic);
    dassert (m_Deep >= StatDeep_CountCntrl);
    tUInt fret = 0;
    m_Protect.BeginRead();
    if (m_pCurrentData) {
        fret = m_pCurrentData->CountControllers;
    }
    m_Protect.EndRead();
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/

tUInt CCAPI20_Statistic::CountBChannels (tUInt Controller) {
    dhead ("CCAPI20_Statistic::CountBChannels", DCON_CCAPI20_Statistic);
    dassert (m_Deep >= StatDeep_CntrlProps);
    tUInt fret = 0;
    m_Protect.BeginRead();
    if (m_pCurrentData && m_pCurrentData->CountControllers) {
        dassert (Controller > 0);
        dassert (Controller <= m_pCurrentData->CountControllers);
        fret = m_pCurrentData->pControllerData[Controller - 1].NumBChannels;
    }
    m_Protect.EndRead();
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/

c_dword CCAPI20_Statistic::Profile_GlobalOptions (tUInt Controller) {
    dhead ("CCAPI20_Statistic::Profile_GlobalOptions", DCON_CCAPI20_Statistic);
    dassert (m_Deep >= StatDeep_CntrlProps);
    c_dword fret = 0;
    m_Protect.BeginRead();
    if (m_pCurrentData && m_pCurrentData->pControllerData) {
        dassert (Controller > 0);
        dassert (Controller <= m_pCurrentData->CountControllers);
        fret = m_pCurrentData->pControllerData[Controller - 1].GlobalOptions;
    }
    m_Protect.EndRead();
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/

c_dword CCAPI20_Statistic::Profile_B1Protocol (tUInt Controller) {
    dhead ("CCAPI20_Statistic::Profile_B1Protocol", DCON_CCAPI20_Statistic);
    dassert (m_Deep >= StatDeep_CntrlProps);
    c_dword fret = 0;
    m_Protect.BeginRead();
    if (m_pCurrentData && m_pCurrentData->pControllerData) {
        dassert (Controller > 0);
        dassert (Controller <= m_pCurrentData->CountControllers);
        fret = m_pCurrentData->pControllerData[Controller - 1].B1ProtocolSupport;
    }
    m_Protect.EndRead();
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/

c_dword CCAPI20_Statistic::Profile_B2Protocol (tUInt Controller) {
    dhead ("CCAPI20_Statistic::Profile_B2Protocol", DCON_CCAPI20_Statistic);
    dassert (m_Deep >= StatDeep_CntrlProps);
    c_dword fret = 0;
    m_Protect.BeginRead();
    if (m_pCurrentData && m_pCurrentData->pControllerData) {
        dassert (Controller > 0);
        dassert (Controller <= m_pCurrentData->CountControllers);
        fret = m_pCurrentData->pControllerData[Controller - 1].B2ProtocolSupport;
    }
    m_Protect.EndRead();
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/

c_dword CCAPI20_Statistic::Profile_B3Protocol (tUInt Controller) {
    dhead ("CCAPI20_Statistic::Profile_B3Protocol", DCON_CCAPI20_Statistic);
    dassert (m_Deep >= StatDeep_CntrlProps);
    c_dword fret = 0;
    m_Protect.BeginRead();
    if (m_pCurrentData && m_pCurrentData->pControllerData) {
        dassert (Controller > 0);
        dassert (Controller <= m_pCurrentData->CountControllers);
        fret = m_pCurrentData->pControllerData[Controller - 1].B3ProtocolSupport;
    }
    m_Protect.EndRead();
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/

c_dword CCAPI20_Statistic::SupplServices (tUInt Controller) {
    dhead ("CCAPI20_Statistic::SupplServices", DCON_CCAPI20_Statistic);
    dassert (m_Deep >= StatDeep_CntrlFacilities);
    c_dword fret = 0;
    m_Protect.BeginRead();
    if (m_pCurrentData && m_pCurrentData->pControllerData) {
        dassert (Controller > 0);
        dassert (Controller <= m_pCurrentData->CountControllers);
        fret = m_pCurrentData->pControllerData[Controller - 1].SupplServices;
    }
    m_Protect.EndRead();
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/

tUInt CCAPI20_Statistic::DSL_FindController (tUInt Controller) {
    dhead ("CCAPI20_Statistic::DSL_FindController", DCON_CCAPI20_Statistic);
    tUInt fret = 0;
    m_Protect.BeginRead();
    if (m_pCurrentData && m_pCurrentData->pControllerData) {
        dassert (Controller <= m_pCurrentData->CountControllers);
        for (; Controller < m_pCurrentData->CountControllers; Controller++) {
            if (CheckOneFlag (m_pCurrentData->pControllerData[Controller].B1ProtocolSupport, b1prot_ATM) == vTrue) {
                fret = Controller + 1;
                break;
            }
        }
    }
    m_Protect.EndRead();
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

tUInt CCAPI20_Statistic::DSL_GetModemState (tUInt Controller) {
    dhead ("CCAPI20_Statistic::DSL_GetModemState", DCON_CCAPI20_Statistic);
    tUInt fret = 0;
    m_Protect.BeginRead();
    if (m_pCurrentData && m_pCurrentData->pControllerData) {
        dassert (Controller > 0);
        dassert (Controller <= m_pCurrentData->CountControllers);
        if (CheckOneFlag (m_pCurrentData->pControllerData[Controller - 1].B1ProtocolSupport, b1prot_ATM) == vTrue) {
            fret = m_pCurrentData->pControllerData[Controller - 1].DSL_ModemState;
        }
    }
    m_Protect.EndRead();
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

tUInt CCAPI20_Statistic::DSL_GetSpeed (tUInt Controller) {
    dhead ("CCAPI20_Statistic::DSL_GetSpeed", DCON_CCAPI20_Statistic);
    tUInt fret = 0;
    m_Protect.BeginRead();
    if (m_pCurrentData && m_pCurrentData->pControllerData) {
        dassert (Controller > 0);
        dassert (Controller <= m_pCurrentData->CountControllers);
        if (CheckOneFlag (m_pCurrentData->pControllerData[Controller - 1].B1ProtocolSupport, b1prot_ATM) == vTrue) {
            fret = m_pCurrentData->pControllerData[Controller - 1].DSL_Speed;
            // Korregiere DSL_Speed (s. MANUFACTURER_IND)
            if (fret) {
                fret--;
            }
        }
    }
    m_Protect.EndRead();
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

tBool CCAPI20_Statistic::DSL_GetMACAddress (tUInt Controller, tUByte *pAddress) {
    dhead ("CCAPI20_Statistic::DSL_GetMACAddress", DCON_CCAPI20_Statistic);
    tBool fret = vFalse;
    m_Protect.BeginRead();
    if (m_pCurrentData && m_pCurrentData->pControllerData) {
        dassert (Controller > 0);
        dassert (Controller <= m_pCurrentData->CountControllers);
        if (  (CheckOneFlag (m_pCurrentData->pControllerData[Controller - 1].B1ProtocolSupport, b1prot_ATM) == vTrue)) {
//           && (*(tUInt32 *)m_pCurrentData->pControllerData[Controller - 1].DSL_MACAddress != 0)) {
            
            bool all_mac_bytes_zero = true;
            for (size_t mac_byte_idx=0; mac_byte_idx < sizeof(((ctIntCntrlData*)NULL)->DSL_MACAddress); ++mac_byte_idx) {
                if (m_pCurrentData->pControllerData[Controller - 1].DSL_MACAddress[mac_byte_idx] != 0) {
                    all_mac_bytes_zero = false;
                }
            }
           
            if (!all_mac_bytes_zero) {
                // seems to be a valid MAC Address
                s_memcpy (pAddress, m_pCurrentData->pControllerData[Controller - 1].DSL_MACAddress, 6);
                fret = vTrue;
            }
        }
    }
    m_Protect.EndRead();
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Statistic::REGISTER_COMPLETE (c_info ErrorCode) {
    dhead ("CCAPI20_Statistic::REGISTER_COMPLETE", DCON_CCAPI20_Statistic);
    dparams ("%x", ErrorCode);
    dassert (m_pFreshingData != 0);
    dassert (m_FreshingController == 0);
    m_pFreshingData->CapiInstalled = ErrorCode;
    if (ErrorCode == ci_NoError) {
        if (m_Deep > StatDeep_OnlyInstall) {
            GET_PROFILE (1);
        } else {
            RELEASE();
        }
    } else {
        UpdateCurrent (ErrorCode);
    }
}


/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Statistic::RELEASE_COMPLETE (void) {
    dhead ("CCAPI20_Statistic::RELEASE_COMPLETE", DCON_CCAPI20_Statistic);
    dassert (m_pFreshingData != 0);
    UpdateCurrent (ci_NoError);
}


/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Statistic::GET_PROFILE_COMPLETE (tUInt /*Controller*/, cm_getprofile *pProfile, c_info ErrorCode) {
    dhead ("CCAPI20_Statistic::GET_PROFILE_COMPLETE", DCON_CCAPI20_Statistic);
    dparams ("%p,%x", pProfile, ErrorCode);
    dassert (m_pFreshingData != 0);
    if (ErrorCode != ci_NoError) {
        //TODO?: Return error error code in Complete?
        RELEASE();
        return;
    }
    dassert (pProfile != 0);
    //dprintBuf (pProfile, sizeof (*pProfile));

    // Is this the first get profile call? Then check the numbers of controllers
    if (!m_pFreshingData->pControllerData) {
        dprint ("NumContrl=%x ", pProfile->NumControllers);
        m_pFreshingData->CountControllers = pProfile->NumControllers;
        if ((m_Deep <= StatDeep_CountCntrl) || (!pProfile->NumControllers)) {
            RELEASE();
            return;
        }
        m_pFreshingData->pControllerData = new ctIntCntrlData [pProfile->NumControllers];
        if (!m_pFreshingData->pControllerData) {
            delete m_pFreshingData;
            m_pFreshingData = 0;
            //TODO: FreshUpComplete (ci_Int_MemoryFullErr);
            dwarning (0);
            RELEASE();
            return;
        }
        s_memzero (m_pFreshingData->pControllerData, sizeof (ctIntCntrlData) * pProfile->NumControllers);
    }
    m_pFreshingData->pControllerData[m_FreshingController].NumBChannels = pProfile->NumBChannels;
    s_memcpy (&m_pFreshingData->pControllerData[m_FreshingController].GlobalOptions,
              &pProfile->GlobalOptions, 4 * sizeof (c_dword));

    if (++m_FreshingController < m_pFreshingData->CountControllers) {
        GET_PROFILE (m_FreshingController + 1);
    } else if (m_Deep >= StatDeep_DSLProps) {
        // GetProfile is finished, now start manu- and facility req's
        CCStruct manudata_speed ("\x03\x00", 2);
        CCStruct manudata_mac   ("\x08\x06\x00\x00\x00\x00\x00\x00\x00", 8);
        tUInt i;
        dassert (m_CountReqCalls == 0);
        m_Protect.Inc (&m_CountReqCalls);
        dprint ("m_CountReqCalls-Inc=%x ", m_CountReqCalls);
        for (i = 0; i < m_pFreshingData->CountControllers; ) {
            if (CheckOneFlag (m_pFreshingData->pControllerData[i++].B1ProtocolSupport, b1prot_ATM) == vTrue) {
                // Found DSL Controller
                m_Protect.Add (&m_CountReqCalls, (tUInt)3); // 3 for the req's and one for the ind / nolisten-req
                m_pFreshingData->pControllerData[i - 1].DSL_Retries = 5; // 5 Tries to get the DSL Informations
                DSLManuReq (i, &manudata_speed);
                DSLManuReq (i, &manudata_mac);
            }
        }
        if (m_Deep >= StatDeep_CntrlFacilities) {
            CCStruct       FacilGetSupplServ ("\x00\x00\x00", 3);
            cm_facilityreq FacParams         (facilselect_SupplServices, &FacilGetSupplServ);
            for (i = 0; i < m_pFreshingData->CountControllers; ) {
                if (CheckOneFlag (m_pFreshingData->pControllerData[i++].GlobalOptions,
                                  gof_SupplementaryServices) == vTrue) {
                    // Supplementary Services are supported
                    m_Protect.Inc (&m_CountReqCalls);
                    FACILITY_REQ (GetNextMsgNr(), i, &FacParams);
                }
            }
        }
        CallFinished();
    } else {
        RELEASE();
    }
}


/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Statistic::FACILITY_CONF (c_word, c_dword Address, cm_facilityconf *Params) {
    dhead ("CCAPI20_Statistic::FACILITY_CONF", DCON_CCAPI20_Statistic);
    dassert (Address > 0);
    dassert (Params != 0);
    dassert (Params->pFacilityParam != 0);
    dassert (m_pFreshingData != 0);
    dassert (Address <= m_pFreshingData->CountControllers);
    if (  CapiInfoIsSuccess (Params->Info)
       && (Params->FacilitySelector == facilselect_SupplServices)) {
        cp_facility_supplserv_conf SupplConf;
        if (  infoSuccess (SupplConf.ParseStruct (Params->pFacilityParam))
           && (SupplConf.Function == supplserv_GetSupportedServices)
           && CapiInfoIsSuccess (SupplConf.pParameter->Info)) {
            m_pFreshingData->pControllerData[Address - 1].SupplServices = SupplConf.pParameter->SupportedServices;
        }
    }
    CallFinished();
}


/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Statistic::MANUFACTURER_CONF (c_word, c_dword Controller, cm_manufacturer *Params) {
    dhead ("CCAPI20_Statistic::MANUFACTURER_CONF", DCON_CCAPI20_Statistic);
    dassert (Params != 0);
    dassert (Params->ManuID == CAPI_MANU_ID_AVM);
    dassert (m_pFreshingData != 0);
    dwarning (Params->Class == 0);
    if (  (Params->ManuID == CAPI_MANU_ID_AVM)
       && (Params->Function == CAPI_MANU_FUNCTION_ADSL)) {
        dassert (Params->pManuData != 0);
        dwarning (Params->pManuData->GetLen() > 0);
        if (Params->pManuData->GetLen()) {
            switch (*(c_word *)Params->pManuData->GetData()) {
            case 0x06:
                dwarning (Params->pManuData->GetLen() >= 8);
                if ((Params->Class == ci_NoError) && (Params->pManuData->GetLen() >= 8)) {
                    s_memcpy (m_pFreshingData->pControllerData[Controller - 1].DSL_MACAddress, Params->pManuData->GetData() + 2, 6);
                }
                break;
            case 0x03:
                if (CapiInfoIsError (Params->Class)) {
                    if (m_pFreshingData->pControllerData[Controller - 1].DSL_Retries) {
                        m_pFreshingData->pControllerData[Controller - 1].DSL_Retries--;
                        m_Protect.Inc (&m_CountReqCalls);
                        CCStruct manudata_speed ("\x03\x00", 2);
                        DSLManuReq (Controller, &manudata_speed);
                    } else {
                        dwarning (0);
                        dassert (m_CountReqCalls > 1);
                        m_Protect.Dec (&m_CountReqCalls);
                    }
                }
                break;
            default:
                dprintWrn ("Unexpected Service=%x ", *(c_word *)Params->pManuData->GetData());
                break;
            }
        }
    }
    CallFinished();
}


/*===========================================================================*\
\*===========================================================================*/

tBool CCAPI20_Statistic::MANUFACTURER_IND (c_word, c_dword Controller, cm_manufacturer *Params) {
    dhead ("CCAPI20_Statistic::MANUFACTURER_IND", DCON_CCAPI20_Statistic);
    dassert (Params != 0);
    dassert (Params->pManuData != 0);
    dassert (Params->ManuID == CAPI_MANU_ID_AVM);
    dassert (m_pFreshingData != 0);

    dprint ("Cntrl=%x, Func=%x, Speed=%x, Len=%x ", Controller, Params->Function,
            m_pFreshingData->pControllerData[Controller - 1].DSL_Speed, Params->pManuData->GetLen());
    if (  (Params->ManuID != CAPI_MANU_ID_AVM)
       || (Controller > m_pFreshingData->CountControllers)
       || (Params->Function != CAPI_MANU_FUNCTION_ADSL)) {
        return vFalse;
    }
    {
        cm_manufacturer RespParams (CAPI_MANU_ID_AVM, 0, CAPI_MANU_FUNCTION_ADSL, 0);
        MANUFACTURER_RESP (GetNextMsgNr(), Controller, &RespParams);
    }

    if (!m_pFreshingData->pControllerData[Controller - 1].DSL_Speed) {
        cp_Manu_DSLParameter dslparam;
        if (infoSuccess (dslparam.ParseStruct (Params->pManuData))) {
            m_pFreshingData->pControllerData[Controller - 1].DSL_ModemState = dslparam.ModemState;
            if (dslparam.ModemState < 8)  {
                // DSL not ready => stop all
                m_pFreshingData->pControllerData[Controller - 1].DSL_Retries = 0;
            } else {
                dassert (dslparam.pLineParameters != 0);
                dinfo (dslparam.pLineParameters->GetLen() >= sizeof (cManu_LineParameters), "To Short=%x!! ",
                       dslparam.pLineParameters->GetLen());
                cManu_LineParameters *pLineParams = (cManu_LineParameters *)dslparam.pLineParameters->GetData();
                if (  (dslparam.pLineParameters->GetLen() >= sizeof (cManu_LineParameters))
                   && (pLineParams->UsedOptionDS < 4)) {
                    dassert (pLineParams != 0);
                    m_pFreshingData->pControllerData[Controller - 1].DSL_Speed =
                            (pLineParams->DS_DatabytesPerSymbol[pLineParams->UsedOptionDS] * 48 / 53 * 32) * 1000 + 1;
                    // Damit CallFinished nicht zu oft aufgerufen wird, nutzte ich DSL_Speed als Testmerkmal.
                    // Dafür muß obige Zuweisung immer != 0 sein. Deshalb wird vorsorglich eine 1 addiert,
                    // die später wieder abgezogen wird.
                    dprint ("Controller=%x,DSL_Speed=%x ", Controller,
                            m_pFreshingData->pControllerData[Controller - 1].DSL_Speed);

                    // stop dsl listen
                    m_pFreshingData->pControllerData[Controller - 1].DSL_Retries = 0;
                }
            }
        }
        dwarning (0);
        if (m_pFreshingData->pControllerData[Controller - 1].DSL_Retries) {
            m_pFreshingData->pControllerData[Controller - 1].DSL_Retries--;
            m_Protect.Inc (&m_CountReqCalls);
            CCStruct manudata_speed ("\x03\x00", 2);
            DSLManuReq (Controller, &manudata_speed);
        } else {
            CallFinished();
        }
    }
    return vTrue;
}


/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Statistic::DSLManuReq (tUInt Controller, CCStruct *pManuData) {
    dhead ("CCAPI20_Statistic::DSLManuReq", DCON_CCAPI20_Statistic);
    dassert (Controller > IllegalController);
    dassert (pManuData != 0);
    cm_manufacturer pm (CAPI_MANU_ID_AVM, 0, CAPI_MANU_FUNCTION_ADSL, pManuData);
    MANUFACTURER_REQ (GetNextMsgNr(), Controller, &pm);
}


/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Statistic::CallFinished (void) {
    dhead ("CCAPI20_Statistic::CallFinished", DCON_CCAPI20_Statistic);
    dparams ("%x", m_CountReqCalls);
    dassert (m_CountReqCalls > 0);
    dassert (m_pFreshingData != 0);
    if (m_Protect.Dec (&m_CountReqCalls) == 1) {
        RELEASE();
    }
    dprint ("new_count=%x ", m_CountReqCalls);
}


/*===========================================================================*\
\*===========================================================================*/

void CCAPI20_Statistic::UpdateCurrent (c_info Info) {
    dhead ("CCAPI20_Statistic::UpdateCurrent", DCON_CCAPI20_Statistic);
    ctIntCapiData *pDelData = 0;
    m_Protect.BeginWrite();
    if ((Info != ci_NoError) && (m_FreshOnError == vFalse)) {
        // ignore this m_pFreshingData
        if (m_pFreshingData) {
            pDelData        = m_pFreshingData;
            m_pFreshingData = 0;
        }
    } else if (m_pFreshingData) {
        pDelData        = m_pCurrentData;
        m_pCurrentData  = m_pFreshingData;
        m_pFreshingData = 0;
    }
    m_Protect.EndWrite();
    if (pDelData) {
        delete [] pDelData->pControllerData;
        delete pDelData;
    }
    FreshUpComplete (Info);
}


/*===========================================================================*\
\*===========================================================================*/
