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

#ifndef _CCSMESG_H_
#define _CCSMESG_H_

#include "CCStruct.h"
#include "CCSParam.h"

#pragma pack(1)
/*===========================================================================*\
    GetProfile
\*===========================================================================*/

struct cm_getprofile {
    c_word  NumControllers;
    c_word  NumBChannels;
    c_dword GlobalOptions;
    c_dword B1ProtocolSupport;
    c_dword B2ProtocolSupport;
    c_dword B3ProtocolSupport;
    c_byte  Reserved[24];
    c_byte  ManufacturerSpecific[20];
};

/*===========================================================================*\
\*===========================================================================*/

class cm_messagebody : public CCStruct {
protected:
    cm_messagebody (tCapiStructType type, void *data, tUInt len, tFormatChar *format)
      : CCStruct (type, data, len, format) {}

    cm_messagebody (tCapiStructType type, void *data, tFormatChar *format)
      : CCStruct (type, data, format) {}
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_message : public CCStruct {
public:
    cm_message (void)
      : CCStruct (cxt_Message, &ApplID, cxf_MSGLEN cxf_0A cxf_STRUCT),
        ApplID (0),
        Command (0),
        Subcommand (0),
        MessageNumber (0),
        Address (0),
        pMessageBody (0) {}

    c_word          ApplID;
    c_byte          Command;
    c_byte          Subcommand;
    c_word          MessageNumber;
    c_dword         Address;
    cm_messagebody *pMessageBody;
};


/*===========================================================================*\
    All Messages
\*===========================================================================*/
#ifdef C_PLATFORM_64

class cm_datab3 : public cm_messagebody {
public:
    cm_datab3 (c_dataptr data = 0, c_word length = 0, c_word handle = 0, c_word flags = 0)
      : cm_messagebody (cxt_DataB3, &DataLength, cxf_NOLEN cxf_Z_04 cxf_0E),
        DataLength (length),
        DataHandle (handle),
        Flags (flags),
        Data (data) {}

    c_word    DataLength;
    c_word    DataHandle;
    c_word    Flags;
    c_dataptr Data;
};

#else // C_PLATFORM_64

class cm_datab3 : public cm_messagebody {
public:
    cm_datab3 (c_dataptr data = 0, c_word length = 0, c_word handle = 0, c_word flags = 0)
      : cm_messagebody (cxt_DataB3, &Data, 0x0A, cxf_NOLEN cxf_CONST),
        Data (data),
        DataLength (length),
        DataHandle (handle),
        Flags (flags) {}

    c_dataptr Data;
    c_word    DataLength;
    c_word    DataHandle;
    c_word    Flags;
};

#endif // C_PLATFORM_64
/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_manufacturer : public cm_messagebody {
public:
    cm_manufacturer (c_dword manuid = 0, c_dword mclass = 0, c_dword function = 0, CCStruct *pdata = 0)
      : cm_messagebody (cxt_Manufacturer, &ManuID, cxf_NOLEN cxf_0C cxf_STRUCT),
        ManuID (manuid),
        Class (mclass),
        Function (function),
        pManuData (pdata) {}

    c_dword   ManuID;
    c_dword   Class;
    c_dword   Function;
    CCStruct *pManuData;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_manufacturer_ref : public cm_manufacturer {
public:
    cm_manufacturer_ref (void) : cm_manufacturer (0, 0, 0, &m_ManuData) {}

private:
    CCStruct m_ManuData;
};


/*===========================================================================*\
    Requests
\*===========================================================================*/

class cm_connectreq : public cm_messagebody {
public:
    cm_connectreq (c_cipvalue value = cipvalue_Undefined, CCStruct *pcallednum = 0,
                   CCStruct *pcallingnum = 0, CCStruct *pcalledsub = 0, CCStruct *pcallingsub = 0,
                   cp_bprotocol *prot = 0, cp_addinfo *addinfo = 0)
      : cm_messagebody (cxt_ConnectReq, &CIPValue, cxf_NOLEN cxf_CIPVAL cxf_STRUCT cxf_STRUCT
                        cxf_STRUCT cxf_STRUCT cxf_STRUCT cxf_Z_03 cxf_STRUCT),
        CIPValue (value),
        pCalledPartyNumber (pcallednum),
        pCallingPartyNumber (pcallingnum),
        pCalledPartySubaddress (pcalledsub),
        pCallingPartySubaddress (pcallingsub),
        pBProtocol (prot),
        pAdditionalInfo (addinfo) {}

    c_cipvalue        CIPValue;
    CCStruct         *pCalledPartyNumber;
    CCStruct         *pCallingPartyNumber;
    CCStruct         *pCalledPartySubaddress;
    CCStruct         *pCallingPartySubaddress;
    cp_bprotocol     *pBProtocol;
    cp_addinfo       *pAdditionalInfo;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_facilityreq : public cm_messagebody {
public:
    cm_facilityreq (c_facilselect select = facilselect_Handset, CCStruct *param = 0)
      : cm_messagebody (cxt_FacilityReq, &FacilitySelector, cxf_NOLEN cxf_FACIL cxf_STRUCT),
        FacilitySelector (select),
        pFacilityParam (param) {}

    c_facilselect FacilitySelector;
    CCStruct     *pFacilityParam;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_inforeq : public cm_messagebody {
public:
    cm_inforeq (CCStruct *callednum = 0, cp_addinfo *addinfo = 0)
      : cm_messagebody (cxt_InfoReq, &pCalledPartyNumber, cxf_NOLEN cxf_STRUCT cxf_STRUCT),
        pCalledPartyNumber (callednum),
        pAdditionalInfo (addinfo) {}

    CCStruct   *pCalledPartyNumber;
    cp_addinfo *pAdditionalInfo;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_listenreq : public cm_messagebody {
public:
    cm_listenreq (c_dword infom = 0, c_dword cipm = 0, c_dword cipm2 = 0, CCStruct *pnumber = 0)
      : cm_messagebody (cxt_ListenReq, &InfoMask, cxf_NOLEN cxf_0C cxf_STRUCT cxf_Z_STRUCT),
        InfoMask (infom),
        CIPMask (cipm),
        CIPMask2 (cipm2),
        pCallingPartyNumber (pnumber) {}

    c_dword   InfoMask;
    c_dword   CIPMask;
    c_dword   CIPMask2;
    CCStruct *pCallingPartyNumber;
};


/*===========================================================================*\
    Confirmations
\*===========================================================================*/

class cm_datab3conf : public cm_messagebody {
public:
    cm_datab3conf (c_word han = 0, c_info inf = ci_NoError)
      : cm_messagebody (cxt_DataB3Conf, &DataHandle, cxf_NOLEN cxf_02 cxf_INFO),
        DataHandle (han),
        Info (inf) {}

    c_word DataHandle;
    c_info Info;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_facilityconf : public cm_messagebody {
public:
    cm_facilityconf (void)
      : cm_messagebody (cxt_FacilityConf, &Info, cxf_NOLEN cxf_INFO cxf_FACIL cxf_STRUCT),
        Info (ci_NoError),
        FacilitySelector (facilselect_Handset),
        pFacilityParam (&m_FacilityParam) {}

    c_info        Info;
    c_facilselect FacilitySelector;
    CCStruct     *pFacilityParam;

private:
    CCStruct      m_FacilityParam;
};


/*===========================================================================*\
    Indications
\*===========================================================================*/

class cm_connectind : public cm_messagebody {
public:
    cm_connectind (void)
      : cm_messagebody (cxt_ConnectInd, &CIPValue, cxf_NOLEN cxf_CIPVAL cxf_STRUCT cxf_STRUCT cxf_STRUCT cxf_STRUCT
                                                   cxf_Z_STRUCT cxf_Z_STRUCT cxf_Z_STRUCT cxf_STRUCT),
        CIPValue (cipvalue_Undefined),
        pCalledPartyNumber (&m_CalledPartyNumber),
        pCallingPartyNumber (&m_CallingPartyNumber),
        pCalledPartySubaddress (&m_CalledPartySubaddress),
        pCallingPartySubaddress (&m_CallingPartySubaddress),
        pAdditionalInfo (&m_AdditionalInfo) {}

    c_cipvalue  CIPValue;
    CCStruct   *pCalledPartyNumber;
    CCStruct   *pCallingPartyNumber;
    CCStruct   *pCalledPartySubaddress;
    CCStruct   *pCallingPartySubaddress;
    cp_addinfo *pAdditionalInfo;

private:
    CCStruct       m_CalledPartyNumber;
    CCStruct       m_CallingPartyNumber;
    CCStruct       m_CalledPartySubaddress;
    CCStruct       m_CallingPartySubaddress;
    cp_addinfo_ref m_AdditionalInfo;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_connectactiveind : public cm_messagebody {
public:
    cm_connectactiveind (void)
      : cm_messagebody (cxt_ConnectActiveInd, &pConnectedNumber, cxf_NOLEN cxf_STRUCT cxf_STRUCT),
        pConnectedNumber (&m_ConnectedNumber),
        pConnectedSubaddress (&m_ConnectedSubaddress) {}

    CCStruct *pConnectedNumber;
    CCStruct *pConnectedSubaddress;

private:
    CCStruct  m_ConnectedNumber;
    CCStruct  m_ConnectedSubaddress;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_disconnectb3ind : public cm_messagebody {
public:
    cm_disconnectb3ind (void)
      : cm_messagebody (cxt_DisconnectB3Ind, &ReasonB3, cxf_NOLEN cxf_INFO cxf_STRUCT),
        ReasonB3 (ci_NoError),
        pNCPI (&m_NCPI) {}

    c_info    ReasonB3;
    CCStruct *pNCPI;

private:
    CCStruct  m_NCPI;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_facilityind : public cm_messagebody {
public:
    cm_facilityind (void)
      : cm_messagebody (cxt_FacilityInd, &FacilitySelector, cxf_NOLEN cxf_FACIL cxf_STRUCT),
        FacilitySelector (facilselect_Handset),
        pFacilityParam (&m_FacilityParam) {}

    c_facilselect FacilitySelector;
    CCStruct     *pFacilityParam;

private:
    CCStruct      m_FacilityParam;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_infoind : public cm_messagebody {
public:
    cm_infoind (void)
      : cm_messagebody (cxt_InfoInd, &InfoNumber, cxf_NOLEN cxf_02 cxf_STRUCT),
        InfoNumber (0),
        pInfoElement (&mi_InfoElement) {}

    c_word    InfoNumber;
    CCStruct *pInfoElement;

private:
    CCStruct mi_InfoElement;
};


/*===========================================================================*\
    Responses
\*===========================================================================*/

class cm_connectresp : public cm_messagebody {
public:
    cm_connectresp (c_reject reject = creject_Ignore, cp_bprotocol *prot = 0, CCStruct *pconnnum = 0, CCStruct *pconnsub = 0,
                    cp_addinfo *addinfo = 0)
      : cm_messagebody (cxt_ConnectResp, &Reject, cxf_NOLEN cxf_REJECT cxf_STRUCT cxf_STRUCT cxf_STRUCT cxf_Z_STRUCT cxf_STRUCT),
        Reject (reject),
        pBProtocol (prot),
        pConnectedNumber (pconnnum),
        pConnectedSubaddress (pconnsub),
        pAdditionalInfo (addinfo) {}

    c_reject      Reject;
    cp_bprotocol *pBProtocol;
    CCStruct     *pConnectedNumber;
    CCStruct     *pConnectedSubaddress;
    cp_addinfo   *pAdditionalInfo;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_connectb3resp : public cm_messagebody {
public:
    cm_connectb3resp (c_reject rej = creject_Reject_Normal, CCStruct *ncpi = 0)
      : cm_messagebody (cxt_ConnectB3Resp, &Reject, cxf_NOLEN cxf_REJECT cxf_STRUCT),
        Reject (rej),
        pNCPI (ncpi) {}

    c_reject  Reject;
    CCStruct *pNCPI;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_datab3resp : public cm_messagebody {
public:
    cm_datab3resp (c_word han)
      : cm_messagebody (cxt_DataB3Resp, &DataHandle, cxf_NOLEN cxf_02),
        DataHandle (han) {}

    c_word DataHandle;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cm_facilityresp : public cm_messagebody {
public:
    cm_facilityresp (c_facilselect select = facilselect_Handset, CCStruct *param = 0)
      : cm_messagebody (cxt_FacilityResp, &FacilitySelector, cxf_NOLEN cxf_FACIL cxf_STRUCT),
        FacilitySelector (select),
        pFacilityParam (param) {}

    c_facilselect FacilitySelector;
    CCStruct     *pFacilityParam;
};


/*===========================================================================*\
\*===========================================================================*/
#pragma pack()

#endif
