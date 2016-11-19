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

#ifndef _CCSTRUCT_H_
#define _CCSTRUCT_H_

#include "capi2def.h"
#include "CString.h"
#include "DataPack.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define iWrn_Struct_NoChanges       CalculateInfoValue (infoType_Packet, infoKind_Warning, 1)
#define iErr_Struct_LenToSmall      CalculateInfoValue (infoType_Packet, infoKind_Error,   1)
#define iErr_Struct_WrongFormat     CalculateInfoValue (infoType_Packet, infoKind_Error,   2)
#define iErr_Struct_WrongLength     CalculateInfoValue (infoType_Packet, infoKind_Error,   3)
#define iErr_Struct_LenOutOfRange   CalculateInfoValue (infoType_Packet, infoKind_Error,   4)


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define cxf_NOLEN       "\x4F"      // must be the first in a cxf-list
#define cxf_MSGLEN      "\x4E"      // must be the first in a cxf-list
#define cxf_CONST       "\x4D"      // must be the only one (except cxf_NOLEN)
#define cxf_STRUCT      "\x4C"
#define cxf_STRING      "\x4B"      // could only be the last one
#define cxf_PACKET      "\x4A"      // could only be the last one
#define cxf_ARRAY       "\x49"      // Data is a c_word Number followed by a CCStruct **
#define cxf_MULTIPLY    "\x48"      // Data is a c_word Number followed by the CCStruct *, which is multiplied Number times
#define cxf_INFO        "\x47"
#define cxf_REJECT      "\x46"
#define cxf_CIPVAL      "\x45"
#define cxf_B1PROT      "\x44"
#define cxf_B2PROT      "\x43"
#define cxf_B3PROT      "\x42"
#define cxf_FACIL       "\x41"
#define cxf_SUPSRV      "\x40"
#define cxf_Z_06        "\x3F"      // write some 0 in Buffer
#define cxf_Z_05        "\x3E"
#define cxf_Z_04        "\x3D"
#define cxf_Z_03        "\x3C"
#define cxf_Z_02        "\x3B"
#define cxf_Z_01        "\x3A"
#define cxf_Z_STRUCT    "\x39"
#define cxf_I_12        "\x38"      // ignore x-Bytes of Data
#define cxf_I_10        "\x37"
#define cxf_I_0E        "\x36"
#define cxf_I_0C        "\x35"
#define cxf_I_04        "\x34"
#define cxf_I_03        "\x33"
#define cxf_I_02        "\x32"

#define cxf_01          "\x01"
#define cxf_02          "\x02"
#define cxf_03          "\x03"
#define cxf_04          "\x04"
#define cxf_05          "\x05"
#define cxf_06          "\x06"
#define cxf_07          "\x07"
#define cxf_08          "\x08"
#define cxf_09          "\x09"
#define cxf_0A          "\x0A"
#define cxf_0B          "\x0B"
#define cxf_0C          "\x0C"
#define cxf_0D          "\x0D"
#define cxf_0E          "\x0E"
#define cxf_0F          "\x0F"
#define cxf_10          "\x10"
#define cxf_11          "\x11"
#define cxf_12          "\x12"
#define cxf_28          "\x28"


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

enum tCapiStructType {
    cxt_RawData,
    cxt_StructData,         // default type for CCStruct childs
    cxt_Message,

    cxt_ConnectReq,
    cxt_ConnectInd,
    cxt_ConnectActiveInd,
    cxt_ConnectResp,
    cxt_ConnectB3Resp,
    cxt_DataB3,
    cxt_DataB3Conf,
    cxt_DataB3Resp,
    cxt_DisconnectB3Ind,
    cxt_FacilityReq,
    cxt_FacilityConf,
    cxt_FacilityInd,
    cxt_FacilityResp,
    cxt_InfoReq,
    cxt_InfoInd,
    cxt_ListenReq,
    cxt_Manufacturer,

    cxt_BProtocol,
    cxt_B1Config,
    cxt_B2Config,
    cxt_B3Config,
    cxt_AdditionalInfo,
    cxt_Ncpi,

    cxt_CalledPartyNumber,
    cxt_CallingPartyNumber,

    cxt_FacilityParam,
    cxt_FacilityParam_SupplServ,
    cxt_FacilityReq_DTMF,
    cxt_FacilityReq_DTMFDigits,
    cxt_FacilityReq_WakeUp,
    cxt_FacilityReq_WakeUpParam,
    cxt_FacilityConf_DTMF,
    cxt_FacilityConf_WakeUp
};


/*===========================================================================*\
\*===========================================================================*/

class CCStruct {
public:
    CCStruct (tCapiStructType datatype = cxt_RawData);
    CCStruct (const void *data, tUInt len, tCapiStructType datatype = cxt_RawData);
    virtual ~CCStruct (void);
    void Reset (void);

    /*=======================================================================*\
        CalcStructSize must always be called before FillBuffer
        CalcStructSize returns the needed size of the Buffer!
    \*=======================================================================*/
    tUInt  CalcStructSize (void);
    void   ResetStructSize (tBool recursive = vTrue);

    void   FillBuffer (tUByte *pBuffer);
    void   FillBuffer (tUByte **ppBuffer);

    tSInfo ParseStruct  (CCStruct *pStruct);
    tSInfo ParseMessage (c_message Message);

    tSInfo ParseBuffer  (tUByte *pBuffer, tUInt len = 0);
    tSInfo ParseBuffer  (tUByte **ppBuffer, tUInt *pLen);

    tBool  ResizeAndSet (tUInt len, tBool ResizeOnlyIfNeeded);

    tUByte *GetData (void);
    tUInt   GetLen (void);

    tCapiStructType GetDataType (void);
    tCapiStructType GetClassType (void);

protected:
    CCStruct (void *data, tFormatChar *format);
    CCStruct (tCapiStructType type, void *data, tUInt len, tFormatChar *format = cxf_CONST);
    CCStruct (tCapiStructType type, void *data, tFormatChar *format);

private:
    void   FillData  (tUByte **ppBuffer);
    tSInfo ParseData (tUByte **ppBuffer, tUInt Len);
    tBool  Resize    (tUInt maxlen);

    tCapiStructType m_ClassType;
    tCapiStructType m_DataType;
    tFormatChar    *m_Format;
    tUInt           m_Len;
    tUInt           m_MaxLen;
    tUByte         *m_pData;
};


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class CCStructStore : public CCStruct {
public:
    CCStructStore (tCapiStructType datatype = cxt_RawData) : CCStruct ("", 0, datatype) {}
};


/*===========================================================================*\
    Inline Functions
\*===========================================================================*/

inline CCStruct::CCStruct (tCapiStructType datatype)
  : m_ClassType (cxt_RawData),
    m_DataType (datatype),
    m_Format (cxf_CONST),
    m_Len (0),
    m_MaxLen (0),
    m_pData (0) {
}

inline CCStruct::CCStruct (const void *data, tUInt len, tCapiStructType datatype)
  : m_ClassType (cxt_RawData),
    m_DataType (datatype),
    m_Format (cxf_CONST),
    m_Len (len),
    m_MaxLen (0),
    m_pData ((tUByte *)data) {
}

inline CCStruct::CCStruct (void *data, tFormatChar *format)
  : m_ClassType (cxt_StructData),
    m_DataType (cxt_StructData),
    m_Format (format),
    m_Len (0),
    m_MaxLen (0),
    m_pData ((tUByte *)data) {
}

inline CCStruct::CCStruct (tCapiStructType type, void *data, tUInt len, tFormatChar *format)
  : m_ClassType (type),
    m_DataType (type),
    m_Format (format),
    m_Len (len),
    m_MaxLen (0),
    m_pData ((tUByte *)data) {

    // dassert (type != cxt_RawData);
}

inline CCStruct::CCStruct (tCapiStructType type, void *data, tFormatChar *format)
  : m_ClassType (type),
    m_DataType (type),
    m_Format (format),
    m_Len (0),
    m_MaxLen (0),
    m_pData ((tUByte *)data) {

    // dassert (type != cxt_RawData);
}

inline void CCStruct::FillBuffer (tUByte *pBuffer) {
    FillBuffer (&pBuffer);
}

inline tSInfo CCStruct::ParseMessage (c_message Message) {
    // dassert (Message != 0);
    // dassert (get16 (Message) > 12);
    return ParseBuffer (Message + 12, get16 (Message) - 12);
}

inline tSInfo CCStruct::ParseBuffer (tUByte *pBuffer, tUInt len) {
    return ParseBuffer (&pBuffer, &len);
}

inline tBool CCStruct::ResizeAndSet (tUInt len, tBool ResizeOnlyIfNeeded) {
    // dassert (m_ClassType == cxt_RawData);
    if (  (  (len > m_MaxLen)
          || (ResizeOnlyIfNeeded == vFalse))
       && (Resize (len) == vFalse)) {
        m_Len = 0;
        return vFalse;
    }
    s_memzero (m_pData, len);
    m_Len = len;
    return vTrue;
}

inline tUByte *CCStruct::GetData (void) {
    // dassert (type == cxt_RawData);
    return m_pData;
}

inline tUInt CCStruct::GetLen (void) {
    // dassert (type == cxt_RawData);
    return m_Len;
}

inline tCapiStructType CCStruct::GetDataType (void) {
    return m_DataType;
}

inline tCapiStructType CCStruct::GetClassType (void) {
    return m_ClassType;
}

/*===========================================================================*\
\*===========================================================================*/

#endif
