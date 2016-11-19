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

#ifndef _CCSPARAM_H_
#define _CCSPARAM_H_

#include "CCStruct.h"

#pragma pack(1)
/*===========================================================================*\
\*===========================================================================*/

class cp_content_dword : public CCStruct {
public:
    cp_content_dword (c_dword val = 0)
      : CCStruct (cxt_StructData, &Value, sizeof (c_dword)),
        Value (val) {}

    c_dword Value;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_content_string : public CCStruct {
public:
    cp_content_string (CDynamicString *str = 0)
      : CCStruct (&pString, cxf_STRING),
        pString (str) {}

    CDynamicString *pString;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_content_packet : public CCStruct {
public:
    cp_content_packet (CDataPacket *pack = 0)
      : CCStruct (&pPacket, cxf_PACKET),
        pPacket (pack) {}

    CDataPacket *pPacket;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_content_selector : public CCStruct {
public:
    cp_content_selector (c_dword select = 0, CCStruct *param = 0)
      : CCStruct (&Selector, cxf_04 cxf_STRUCT),
        Selector (select),
        pParameter (param) {}

    c_dword   Selector;
    CCStruct *pParameter;
};

class cp_content_selector_store : public cp_content_selector {
public:
    cp_content_selector_store (c_dword select = 0)
      : cp_content_selector (select, &m_Parameter) {}

private:
    CCStructStore m_Parameter;
};


/*===========================================================================*\
\*===========================================================================*/

class cp_callednumber : public CCStruct {
public:
    cp_callednumber (CDynamicString *num = 0)
      : CCStruct (cxt_CalledPartyNumber, &Type, cxf_01 cxf_STRING),
        Type (0x80),
        pNumber (num) {}

    c_byte          Type;
    CDynamicString *pNumber;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_callingnumber : public CCStruct {
public:
    cp_callingnumber (c_byte indicator = CALLNUM_INDICATOR_DEFAULT)
      : CCStruct (cxt_CallingPartyNumber, &Type, cxf_02 cxf_STRING),
        Type (0x00),
        Indicator (indicator),
        pNumber (&m_Number) {}

    c_byte          Type;
    c_byte          Indicator;
    CDynamicString *pNumber;

    void ResetNumber (void) {
        Type      = 0x00;
        Indicator = CALLNUM_INDICATOR_DEFAULT;
        pNumber   = &m_Number;
        m_Number.RemoveAll();
    };

private:
    CDynamicString m_Number;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

typedef cp_callingnumber cp_connectednumber;


/*===========================================================================*\
\*===========================================================================*/

class cp_bprotocol : public CCStruct {
public:
    cp_bprotocol (c_b1prot b1prot = b1prot_HDLC, c_b2prot b2prot = b2prot_X75,
                  c_b3prot b3prot = b3prot_Transparent, CCStruct *pb1config = 0, CCStruct *pb2config = 0,
                  CCStruct *pb3config = 0, CCStruct *pgconfig = 0)
      : CCStruct (cxt_BProtocol, &B1Protocol, cxf_B1PROT cxf_B2PROT cxf_B3PROT cxf_STRUCT cxf_STRUCT cxf_STRUCT cxf_STRUCT),
        B1Protocol (b1prot),
        B2Protocol (b2prot),
        B3Protocol (b3prot),
        pB1Configuration (pb1config),
        pB2Configuration (pb2config),
        pB3Configuration (pb3config),
        pGlobalConfiguration (pgconfig) {}

    c_b1prot  B1Protocol;
    c_b2prot  B2Protocol;
    c_b3prot  B3Protocol;
    CCStruct *pB1Configuration;
    CCStruct *pB2Configuration;
    CCStruct *pB3Configuration;
    CCStruct *pGlobalConfiguration;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_bprotocol_store : public cp_bprotocol {
public:
    cp_bprotocol_store (void)
     : cp_bprotocol (b1prot_HDLC, b2prot_X75, b3prot_Transparent, &m_B1Configuration,
                     &m_B2Configuration, &m_B3Configuration, &m_GlobalConfiguration) {}
private:
    CCStructStore m_B1Configuration;
    CCStructStore m_B2Configuration;
    CCStructStore m_B3Configuration;
    CCStructStore m_GlobalConfiguration;
};


/*===========================================================================*\
\*===========================================================================*/

class cp_b1configuration : public CCStruct {
protected:
    cp_b1configuration (tCapiStructType type, void *data, const tFormatChar *format)
      : CCStruct (type, data, format) {}
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_b1configuration_atm : public cp_b1configuration {
public:
    cp_b1configuration_atm (c_word vcc = 0, c_word vpi = 0, c_word vci = 0)
      : cp_b1configuration (cxt_B1Config, &VCC, cxf_06),
        VCC (vcc),
        VPI (vpi),
        VCI (vci) {}

    c_word VCC;
    c_word VPI;
    c_word VCI;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_b1configuration_all : public cp_b1configuration {
public:
    cp_b1configuration_all (c_b1prot b1prot)
      : cp_b1configuration (cxt_B1Config, &MaximumBitRate, &m_Format[0]),
        MaximumBitRate (0),
        BitsPerCharacter (0),
        Parity (0),
        StopBits (0),
        Options (0),
        SpeedNegotiation (0),
        TransmitLevelInDB (0),
        ATM_VCC (0),
        ATM_VPI (0),
        ATM_VCI (0) {

        switch (b1prot) {
        default:
            // dwarning (0);
            // no break;
        case b1prot_HDLC:
        case b1prot_Transparent:
        case b1prot_invertedHDLC:
        case b1prot_56kBitTransparent:
            s_strncpy (&m_Format[0], cxf_CONST, 2);
            break;
        case b1prot_V110_asynch:
        case b1prot_V110_synch:
            s_strncpy (&m_Format[0], cxf_08, 2);
            break;
        case b1prot_G3Fax:
            s_strncpy (&m_Format[0], cxf_08 cxf_I_04 cxf_02, 4);
            break;
        case b1prot_Modem_negotiate:
        case b1prot_Modem_asynch:
        case b1prot_Modem_synch:
            s_strncpy (&m_Format[0], cxf_0C, 2);
            break;
        case b1prot_ATM:
            s_strncpy (&m_Format[0], cxf_I_0E cxf_06, 3);
            break;
        }
    }

    c_word MaximumBitRate;                 // 2, 3, 4, 7, 8, 9
    c_word BitsPerCharacter;               // 2, 7, 8
    c_word Parity;                         // 2, 7, 8
    c_word StopBits;                       // 2, 7, 8
    c_word Options;                        // 7, 8, 9
    c_word SpeedNegotiation;               // 7, 8, 9
    c_word TransmitLevelInDB;              // 4
    c_word ATM_VCC;                        // 28
    c_word ATM_VPI;                        // 28
    c_word ATM_VCI;                        // 28

private:
    tFormatChar m_Format[4];
};


/*===========================================================================*\
\*===========================================================================*/

class cp_b3configuration : public CCStruct {
protected:
    cp_b3configuration (void *data, const tFormatChar *format)
      : CCStruct (cxt_B1Config, data, format) {}
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_b3configuration_faxext : public cp_b3configuration {
public:
    cp_b3configuration_faxext (c_word opts = 0, CDynamicString *station = 0, CDynamicString *head = 0)
      : cp_b3configuration (&Options, cxf_02 cxf_Z_02 cxf_STRUCT cxf_STRUCT),
        Options (opts),
        pStationID (&m_StationID),
        pHeadLine (&m_HeadLine),
        m_StationID (station),
        m_HeadLine (head) {}

    c_word             Options;
    cp_content_string *pStationID;
    cp_content_string *pHeadLine;

private:
    tFormatChar       m_Format[7];
    cp_content_string m_StationID;
    cp_content_string m_HeadLine;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_b3configuration_all : public cp_b3configuration {
public:
    cp_b3configuration_all (c_b3prot b3prot, CDynamicString *station = 0, CDynamicString *head = 0)
      : cp_b3configuration (&LIC, &m_Format[0]),
        LIC (0),
        HIC (0),
        LTC (0),
        HTC (0),
        LOC (0),
        HOC (0),
        ModuloMode (0),
        WindowSize (0),
        Resolution (0),
        Options (0),
        Format (0),
        pStationID (&m_StationID),
        pHeadLine (&m_HeadLine),
        m_StationID (station),
        m_HeadLine (head) {

        switch (b3prot) {
        default:
            // dwarning (0);
            // no break!
        case b3prot_Transparent:
        case b3prot_Modem:
        case b3prot_ADSL:
            s_strncpy (&m_Format[0], cxf_CONST, 2);
            break;
        case b3prot_T90NL:
        case b3prot_ISO8208:
        case b3prot_X25:
            s_strncpy (&m_Format[0], cxf_10, 2);
            break;
        case b3prot_G3Fax:
            s_strncpy (&m_Format[0], cxf_I_10 cxf_02 cxf_I_02 cxf_02 cxf_STRUCT cxf_STRUCT, 7);
            break;
        case b3prot_G3Fax_extended:
            s_strncpy (&m_Format[0], cxf_I_12 cxf_04 cxf_STRUCT cxf_STRUCT, 5);
            break;
        }
    }

    c_word             LIC;                            // 1, 2, 3
    c_word             HIC;                            // 1, 2, 3
    c_word             LTC;                            // 1, 2, 3
    c_word             HTC;                            // 1, 2, 3
    c_word             LOC;                            // 1, 2, 3
    c_word             HOC;                            // 1, 2, 3
    c_word             ModuloMode;                     // 1, 2, 3
    c_word             WindowSize;                     // 1, 2, 3
    c_word             Resolution;                     // 4
    c_word             Options;                        // 5
    c_word             Format;                         // 4, 5
    cp_content_string *pStationID;                     // 4, 5
    cp_content_string *pHeadLine;                      // 4, 5

private:
    tFormatChar       m_Format[7];
    cp_content_string m_StationID;
    cp_content_string m_HeadLine;
};


/*===========================================================================*\
\*===========================================================================*/

class cp_addinfo : public CCStruct {
public:
    cp_addinfo (CCStruct *chinfo = 0, CCStruct *keyfac = 0, CCStruct *useruser = 0, CCStruct *facdata = 0)
      : CCStruct (cxt_AdditionalInfo, &pBChannelInformation, cxf_STRUCT cxf_STRUCT cxf_STRUCT cxf_STRUCT),
        pBChannelInformation (chinfo),
        pKeypadFacility (keyfac),
        pUserUserData (useruser),
        pFacilityDataArray (facdata) {}

    CCStruct *pBChannelInformation;
    CCStruct *pKeypadFacility;
    CCStruct *pUserUserData;
    CCStruct *pFacilityDataArray;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_addinfo_store : public cp_addinfo {
public:
    cp_addinfo_store (void)
      : cp_addinfo (&m_BChannelInformation, &m_KeypadFacility, &m_UserUserData, &m_FacilityDataArray) {}

private:
    CCStructStore m_BChannelInformation;
    CCStructStore m_KeypadFacility;
    CCStructStore m_UserUserData;
    CCStructStore m_FacilityDataArray;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_addinfo_ref : public cp_addinfo {
public:
    cp_addinfo_ref (void)
      : cp_addinfo (&m_BChannelInformation, &m_KeypadFacility, &m_UserUserData, &m_FacilityDataArray) {}

private:
    CCStruct m_BChannelInformation;
    CCStruct m_KeypadFacility;
    CCStruct m_UserUserData;
    CCStruct m_FacilityDataArray;
};


/*===========================================================================*\
\*===========================================================================*/

class cp_ncpi : public CCStruct {
protected:
    cp_ncpi (void *data, const tFormatChar *format)
      : CCStruct (cxt_Ncpi, data, format) {}
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_ncpi_faxext : public cp_ncpi {
public:
    cp_ncpi_faxext (c_word opt = 0)
      : cp_ncpi (&Options, cxf_Z_02 cxf_02 cxf_Z_05),
        Options (opt) {}

    c_word Options;
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class cp_ncpi_all : public cp_ncpi {
public:
    cp_ncpi_all (c_b3prot b3prot)
      : cp_ncpi (&Rate, &m_Format[0]),
        Rate (0),
        Protocol (0),
        Options (0),
        Resolution (0),
        Format (0),
        Pages (0),
        Flags (0),
        Group (0),
        Channel (0),
        pReceiveID (&m_ReceiveID) {

        switch (b3prot) {
        case b3prot_ISO8208:
        case b3prot_X25:
            s_strncpy (&m_Format[0], cxf_I_0C cxf_03, 3);
            break;
        case b3prot_G3Fax:
            s_strncpy (&m_Format[0], cxf_02 cxf_I_04 cxf_06 cxf_I_03 cxf_STRUCT, 6);
            break;
        case b3prot_G3Fax_extended:
            s_strncpy (&m_Format[0], cxf_02 cxf_I_02 cxf_02 cxf_I_02 cxf_04 cxf_I_03 cxf_STRUCT, 8);
            break;
        case b3prot_Modem:
            s_strncpy (&m_Format[0], cxf_04, 2);
            break;
        default:
            // nothing to do
            break;
        }
    }

    c_word    Rate;                           // 4, 5, 7
    c_word    Protocol;                       // 7
    c_word    Options;                        // 5
    c_word    Resolution;                     // 4
    c_word    Format;                         // 4, 5
    c_word    Pages;                          // 4, 5
    c_byte    Flags;                          // 2, 3
    c_byte    Group;                          // 2, 3
    c_byte    Channel;                        // 2, 3
    CCStruct *pReceiveID;                     // 4, 5

private:
    tFormatChar m_Format[8];
    CCStruct    m_ReceiveID;
};


/*===========================================================================*\
\*===========================================================================*/
#pragma pack()

#endif

