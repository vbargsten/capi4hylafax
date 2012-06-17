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

#ifndef _CCSFACIL_H_
#define _CCSFACIL_H_

#include "CCSParam.h"

#pragma pack(1)
/*===========================================================================*\
\*===========================================================================*/

class cp_facilitynumber : public CCStruct {
public:
    cp_facilitynumber (CDynamicString *num = 0, c_byte indicator = CALLNUM_INDICATOR_DEFAULT)
      : CCStruct (cxt_CallingPartyNumber, &FacilityType, cxf_03 cxf_STRING),
        FacilityType (0x01),
        NumberType (0x00),
        Indicator (indicator),
        pNumber (num) {}

    c_byte          FacilityType;
    c_byte          NumberType;
    c_byte          Indicator;
    CDynamicString *pNumber;

    void ResetNumber (void) {
        FacilityType = 0x01;
        NumberType   = 0x00;
        Indicator    = CALLNUM_INDICATOR_DEFAULT;
        pNumber->RemoveAll();
    };
};


/*===========================================================================*\
\*===========================================================================*/

class cp_facility_dtmfdigits : public CCStruct {
public:
    cp_facility_dtmfdigits (CDynamicString *pdigits)
      : CCStruct (cxt_FacilityReq_DTMFDigits, &pDigits, cxf_STRING),
        pDigits (pdigits) {}

    CDynamicString *pDigits;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_facilityreq_dtmf : public CCStruct {
public:
    cp_facilityreq_dtmf (c_word func = 0, c_word tone = 0, c_word gap = 0, CDynamicString *pdigits = 0, CCStruct *pchar = 0)
      : CCStruct (cxt_FacilityReq_DTMF, &Function, cxf_06 cxf_STRUCT cxf_STRUCT),
        Function (func),
        ToneDuration (tone),
        GapDuration (gap),
        pDTMFDigits (&m_DTMFDigits),
        pDTMFCharacteristics (pchar),
        m_DTMFDigits (pdigits) {}

    c_word                  Function;
    c_word                  ToneDuration;
    c_word                  GapDuration;
    cp_facility_dtmfdigits *pDTMFDigits;
    CCStruct               *pDTMFCharacteristics;

private:
    cp_facility_dtmfdigits  m_DTMFDigits;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_facilityconf_dtmf : public CCStruct {
public:
    cp_facilityconf_dtmf (void)
      : CCStruct (cxt_FacilityConf_DTMF, &DTMFInformation, cxf_02),
        DTMFInformation (0) {}

    c_word DTMFInformation;
};


/*===========================================================================*\
\*===========================================================================*/

class cp_facilityreq_wakeupparam : public CCStruct {
public:
    cp_facilityreq_wakeupparam (CDynamicString *num = 0, c_dword mask = 0)
      : CCStruct (cxt_FacilityReq_WakeUpParam, &pCalledPartyNumber, cxf_STRUCT cxf_04),
        pCalledPartyNumber (&m_CalledPartyNumber),
        CIPMask (mask),
        m_CalledPartyNumber (num) {}

    cp_callednumber *pCalledPartyNumber;
    c_dword          CIPMask;

private:
    cp_callednumber  m_CalledPartyNumber;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_facilityreq_wakeup : public CCStruct {
public:
    cp_facilityreq_wakeup (c_word num, cp_facilityreq_wakeupparam *param)
      : CCStruct (cxt_FacilityReq_WakeUp, &NumberAwakeParameters, &m_Format[0]),
        NumberAwakeParameters (num),
        pWakeUpParam (param) {

        s_strncpy (&m_Format[0], cxf_02 cxf_ARRAY "\x00\x00", 5);
        set16 ((c_byte *)(m_Format + 2), num);
    }

private:
    c_word                      NumberAwakeParameters;
    cp_facilityreq_wakeupparam *pWakeUpParam;
    tFormatChar                 m_Format[5];
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_facilityconf_wakeup : public CCStruct {
public:
    cp_facilityconf_wakeup (void)
      : CCStruct (cxt_FacilityConf_WakeUp, &AcceptedAwakeParameters, cxf_02),
        AcceptedAwakeParameters (0) {}

    c_word AcceptedAwakeParameters;
};


/*===========================================================================*\
\*===========================================================================*/

class cp_facility_supplserv : public CCStruct {
public:
    cp_facility_supplserv (c_supplserv sfunc = supplserv_GetSupportedServices, CCStruct *param = 0)
      : CCStruct (cxt_FacilityParam_SupplServ, &Function, cxf_02 cxf_STRUCT),
        Function (sfunc),
        pParameter (param) {}

    c_supplserv Function;
    CCStruct   *pParameter;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_facility_supplserv_ref : public cp_facility_supplserv {
public:
    cp_facility_supplserv_ref (void)
      : cp_facility_supplserv (supplserv_GetSupportedServices, &m_Parameter) {}

private:
    CCStruct m_Parameter;
};


/*===========================================================================*\
\*===========================================================================*/

class cp_facility_supplserv_conf : public CCStruct {
public:
    class ICConfParam : public CCStruct {
    public:
        ICConfParam (void)
          : CCStruct (cxt_FacilityParam_SupplServ, &Info, 2, cxf_INFO cxf_04),
            Info (ci_NoError),
            SupportedServices (0) {}

        c_info  Info;
        c_dword SupportedServices;
    };

    cp_facility_supplserv_conf (void)
      : CCStruct (cxt_FacilityParam_SupplServ, &Function, cxf_02 cxf_STRUCT),
        Function (supplserv_GetSupportedServices),
        pParameter (&m_Parameter) {}

    c_supplserv  Function;
    ICConfParam *pParameter;

private:
    ICConfParam  m_Parameter;
};


/*===========================================================================*\
\*===========================================================================*/

class cp_supplserv_cdreq : public CCStruct {
public:
    cp_supplserv_cdreq (c_word allowed, CDynamicString *num)
      : CCStruct (cxt_FacilityParam_SupplServ, &PresentationAllowed, cxf_02 cxf_STRUCT),
        PresentationAllowed (allowed),
        pDeflectToNumber (&m_DeflectToNumber),
        m_DeflectToNumber (num) {}

    c_word             PresentationAllowed;
    cp_facilitynumber *pDeflectToNumber;

private:
    cp_facilitynumber  m_DeflectToNumber;
};


/*===========================================================================*\
\*===========================================================================*/
#pragma pack()

#endif
