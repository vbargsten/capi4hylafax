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

#include "CCStruct.h"
#include "aString.h"
#include "c2msgdef.h"
#include "dbgCAPI.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define cxfv_NOLEN      0x4F    // must be the first
#define cxfv_MSGLEN     0x4E    // must be the only one (except cxf_NOLEN)
#define cxfv_CONST      0x4D    // must be the only one (except cxf_NOLEN)
#define cxfv_STRUCT     0x4C
#define cxfv_STRING     0x4B
#define cxfv_PACKET     0x4A
#define cxfv_ARRAY      0x49
#define cxfv_MULTIPLY   0x48
#define cxfv_INFO       0x47
#define cxfv_REJECT     0x46
#define cxfv_CIPVAL     0x45
#define cxfv_B1PROT     0x44
#define cxfv_B2PROT     0x43
#define cxfv_B3PROT     0x42
#define cxfv_FACIL      0x41
#define cxfv_SUPSRV     0x40
#define cxfv_Z_06       0x3F
#define cxfv_Z_05       0x3E
#define cxfv_Z_04       0x3D
#define cxfv_Z_03       0x3C
#define cxfv_Z_02       0x3B
#define cxfv_Z_01       0x3A
#define cxfv_Z_STRUCT   0x39
#define cxfv_I_12       0x38
#define cxfv_I_10       0x37
#define cxfv_I_0E       0x36
#define cxfv_I_0C       0x35
#define cxfv_I_04       0x34
#define cxfv_I_03       0x33
#define cxfv_I_02       0x32


/*===========================================================================*\
\*===========================================================================*/

CCStruct::~CCStruct (void) {
    dhead ("CCStruct-Destructor", DCON_CCStruct);
    if (m_MaxLen) {
        delete [] m_pData;
    }
    m_pData  = 0;
    m_Len    = 0;
    m_MaxLen = 0;
}

/*===========================================================================*\
\*===========================================================================*/

void CCStruct::Reset (void) {
    dhead ("CCStruct::Reset", DCON_CCStruct);
    const tFormatChar *format = (char*)m_Format;
    switch (*format) {
    case cxfv_NOLEN:
    case cxfv_MSGLEN:
        format++;
        break;
    }
    if (*format == cxfv_CONST) {
        m_Len = 0;
        //if (m_pData == 0) {
        //    m_Len = 0;
        //} else {
        //    s_memzero (m_pData, m_Len);
        //}
    } else {
        m_Len = 0;
        tUByte *pSData = m_pData;
        dassert (pSData != 0);
        for (; *format != '\0'; format++) {
            switch (*format) {
            case cxfv_CONST:
            case cxfv_NOLEN:
            case cxfv_MSGLEN:
                dassert (0);
                break;
            case cxfv_STRUCT:
                if (*(CCStruct **)pSData) {
                    (*(CCStruct **)pSData)->Reset();
                }
                pSData += sizeof (CCStruct *);
                break;
            case cxfv_STRING:
                if (*(CDynamicString **)pSData) {
                    (*(CDynamicString **)pSData)->RemoveAll();
                }
                pSData += sizeof (CDynamicString *);
                break;
            case cxfv_PACKET:
                if (*(CDataPacket **)pSData) {
                    (*(CDataPacket **)pSData)->SetLen (0);
                }
                pSData += sizeof (CDataPacket *);
                break;
            case cxfv_ARRAY: {
                c_word count    = *(c_word *)pSData;
                pSData         += sizeof (c_word);
                CCStruct **lauf = *(CCStruct ***)pSData;
                pSData         += sizeof (CCStruct **);
                dwarning (lauf != 0);
                if (lauf) {
                    for (; count > 0; count--, lauf++) {
                        if (*lauf) {
                            (*lauf)->Reset();
                        }
                    }
                }
                break;
            }
            case cxfv_MULTIPLY: {
                // ignore counter
                pSData += sizeof (c_word);
                if (*(CCStruct **)pSData) {
                    (*(CCStruct **)pSData)->Reset();
                }
                pSData += sizeof (CCStruct **);
                break;
            }
            case cxfv_INFO:
                *(c_info *)pSData = ci_NoError;
                pSData += sizeof (c_info);
                break;
            case cxfv_REJECT:
                *(c_reject *)pSData = creject_Ignore;
                pSData += sizeof (c_reject);
                break;
            case cxfv_CIPVAL:
                *(c_cipvalue *)pSData = cipvalue_Undefined;
                pSData += sizeof (c_cipvalue);
                break;
            case cxfv_B1PROT:
                *(c_b1prot *)pSData = b1prot_HDLC;
                pSData += sizeof (c_b1prot);
                break;
            case cxfv_B2PROT:
                *(c_b2prot *)pSData = b2prot_X75;
                pSData += sizeof (c_b2prot);
                break;
            case cxfv_B3PROT:
                *(c_b3prot *)pSData = b3prot_Transparent;
                pSData += sizeof (c_b3prot);
                break;
            case cxfv_FACIL:
                *(c_facilselect *)pSData = facilselect_Handset;
                pSData += sizeof (c_facilselect);
                break;
            case cxfv_SUPSRV:
                *(c_supplserv *)pSData = supplserv_GetSupportedServices;
                pSData += sizeof (c_supplserv);
                break;
            case cxfv_Z_06:
            case cxfv_Z_05:
            case cxfv_Z_04:
            case cxfv_Z_03:
            case cxfv_Z_02:
            case cxfv_Z_01:
            case cxfv_Z_STRUCT:
                // nothing to do!
                break;
            case cxfv_I_12:
                pSData += 0x12;
                break;
            case cxfv_I_10:
                pSData += 0x10;
                break;
            case cxfv_I_0E:
                pSData += 0x0E;
                break;
            case cxfv_I_0C:
                pSData += 0x0C;
                break;
            case cxfv_I_04:
                pSData += 0x04;
                break;
            case cxfv_I_03:
                pSData += 0x03;
                break;
            case cxfv_I_02:
                pSData += 0x02;
                break;
            default:
                s_memzero (pSData, (tUByte)*format);
                pSData += (tUByte)*format;
                break;
            }
        }
    }
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CCStruct::CalcStructSize (void) {
    dhead ("CCStruct::CalcStructSize", DCON_CCStruct);
    dparams ("%x,%x,l=%x", m_ClassType, m_DataType, m_Len);
    dassert (m_Format != 0);
    if (m_Len == 0) {
        const tFormatChar *format = m_Format;
        tUByte      *pSData = m_pData;
        switch (*format) {
        case cxfv_CONST:
            // nothing to do!
            break;
        case cxfv_NOLEN:
        case cxfv_MSGLEN:
            format++;
            if (*format == cxfv_CONST) {
                // nothing to do!
                break;
            }
            // no break!
        default:
            dassert (pSData != 0);
            for (; *format != '\0'; format++) {
                switch (*format) {
                case cxfv_CONST:
                case cxfv_NOLEN:
                case cxfv_MSGLEN:
                    dassert (0);
                    break;
                case cxfv_STRUCT:
                    m_Len  += (*(CCStruct **)pSData) ? (*(CCStruct **)pSData)->CalcStructSize() : 1;
                    dprint ("-cxfv_STRUCT(%x,%p)-", m_Len, pSData);
                    pSData += sizeof (CCStruct *);
                    break;
                case cxfv_STRING:
                    if (*(CDynamicString **)pSData) {
                        m_Len += (*(CDynamicString **)pSData)->GetLen();
                    }
                    dprint ("-cxfv_STRING(%x,%p)-", m_Len, pSData);
                    pSData += sizeof (CDynamicString *);
                    break;
                case cxfv_PACKET:
                    if (*(CDataPacket **)pSData) {
                        m_Len += (*(CDataPacket **)pSData)->GetLen();
                    }
                    dprint ("-cxfv_PACKET(%x,%p)-", m_Len, pSData);
                    pSData += sizeof (CDataPacket *);
                    break;
                case cxfv_ARRAY: {
                    c_word count    = *(c_word *)pSData;
                    pSData         += sizeof (c_word);
                    CCStruct **lauf = *(CCStruct ***)pSData;
                    pSData         += sizeof (CCStruct **);
                    dwarning (lauf != 0);
                    if (lauf) {
                        for (; count > 0; count--, lauf++) {
                            m_Len  += (*lauf) ? (*lauf)->CalcStructSize() : 1;
                            dprint ("-cxfv_ARRAY(%x,%p)-", m_Len, lauf);
                        }
                    }
                    break;
                }
                case cxfv_MULTIPLY: {
                    c_word count = *(c_word *)pSData;
                    pSData      += sizeof (c_word);
                    m_Len       += ((*(CCStruct **)pSData) ? (*(CCStruct **)pSData)->CalcStructSize() : 1) * count;
                    pSData      += sizeof (CCStruct *);
                    dprint ("-cxfv_MULTIPLY(%x,%p)-", m_Len, pSData);
                    break;
                }
                case cxfv_INFO:
                    m_Len  += 2;
                    dprint ("-cxfv_INFO(%x,%p)-", m_Len, pSData);
                    pSData += sizeof (c_info);
                    break;
                case cxfv_REJECT:
                    m_Len  += 2;
                    dprint ("-cxfv_REJECT(%x,%p)-", m_Len, pSData);
                    pSData += sizeof (c_reject);
                    break;
                case cxfv_CIPVAL:
                    m_Len  += 2;
                    dprint ("-cxfv_CIPVAL(%x,%p)-", m_Len, pSData);
                    pSData += sizeof (c_cipvalue);
                    break;
                case cxfv_B1PROT:
                    m_Len  += 2;
                    dprint ("-cxfv_B1PROT(%x,%p)-", m_Len, pSData);
                    pSData += sizeof (c_b1prot);
                    break;
                case cxfv_B2PROT:
                    m_Len  += 2;
                    dprint ("-cxfv_B2PROT(%x,%p)-", m_Len, pSData);
                    pSData += sizeof (c_b2prot);
                    break;
                case cxfv_B3PROT:
                    m_Len  += 2;
                    dprint ("-cxfv_B3PROT(%x,%p)-", m_Len, pSData);
                    pSData += sizeof (c_b3prot);
                    break;
                case cxfv_FACIL:
                    m_Len  += 2;
                    dprint ("-cxfv_FACIL(%x,%p)-", m_Len, pSData);
                    pSData += sizeof (c_facilselect);
                    break;
                case cxfv_SUPSRV:
                    m_Len  += 2;
                    dprint ("-cxfv_SUPSERV(%x,%p)-", m_Len, pSData);
                    pSData += sizeof (c_supplserv);
                    break;
                case cxfv_Z_06:
                    m_Len += 6;
                    dprint ("-cxfv_Z_06(%x)-", m_Len);
                    break;
                case cxfv_Z_05:
                    m_Len += 5;
                    dprint ("-cxfv_Z_05(%x)-", m_Len);
                    break;
                case cxfv_Z_04:
                    m_Len += 4;
                    dprint ("-cxfv_Z_04(%x)-", m_Len);
                    break;
                case cxfv_Z_03:
                    m_Len += 3;
                    dprint ("-cxfv_Z_03(%x)-", m_Len);
                    break;
                case cxfv_Z_02:
                    m_Len += 2;
                    dprint ("-cxfv_Z_02(%x)-", m_Len);
                    break;
                case cxfv_Z_01:
                case cxfv_Z_STRUCT:
                    m_Len += 1;
                    dprint ("-cxfv_Z_01(%x)-", m_Len);
                    break;
                case cxfv_I_12:
                case cxfv_I_10:
                case cxfv_I_0E:
                case cxfv_I_0C:
                case cxfv_I_04:
                case cxfv_I_03:
                case cxfv_I_02:
                    // Nothing to do!
                    break;
                default:
                    dassert ((tUByte)*format < 0x10);
                    m_Len  += (tUByte)*format;
                    dprint ("-cxfv_%x(%x)-", (tUByte)*format, m_Len);
                    pSData += (tUByte)*format;
                    break;
                }
            }
            break;
        }
    }
    dassert (m_Len <= MAXVAL_tUShort);
    switch (*m_Format) {
    case cxfv_NOLEN:
        RETURN ('x', m_Len);
    case cxfv_MSGLEN:
        RETURN ('x', m_Len + 2);
    }
    RETURN ('x', m_Len + ((m_Len > 254) ? 3 : 1));
}


/*===========================================================================*\
\*===========================================================================*/

void CCStruct::ResetStructSize (tBool recursive) {
    dhead ("CCStruct::ResetStructSize", DCON_CCStruct);
    const tFormatChar *format = m_Format;
    switch (*format) {
    case cxfv_CONST:
        break;
    case cxfv_NOLEN:
    case cxfv_MSGLEN:
        format++;
        if (*format == cxfv_CONST) {
            break;
        }
        // no break!
    default:
        m_Len = 0;
        if (recursive == vFalse) {
            break;
        }
        tUByte *pSData = m_pData;
        dassert (pSData != 0);
        for (; *format != '\0'; format++) {
            switch (*format) {
            case cxfv_CONST:
            case cxfv_NOLEN:
            case cxfv_MSGLEN:
                dassert (0);
                break;
            case cxfv_STRUCT:
                if (*(CCStruct **)pSData) {
                    (*(CCStruct **)pSData)->ResetStructSize();
                }
                // no break!
            case cxfv_STRING:
            case cxfv_PACKET:
                pSData += sizeof (void *);
                break;
            case cxfv_ARRAY: {
                c_word count    = *(c_word *)pSData;
                pSData         += sizeof (c_word);
                CCStruct **lauf = *(CCStruct ***)pSData;
                pSData         += sizeof (CCStruct **);
                dwarning (lauf != 0);
                if (lauf) {
                    for (; count > 0; count--, lauf++) {
                        if (*lauf) {
                            (*lauf)->ResetStructSize();
                        }
                    }
                }
                break;
            }
            case cxfv_MULTIPLY: {
                // ignore counter
                pSData += sizeof (c_word);
                if (*(CCStruct **)pSData) {
                    (*(CCStruct **)pSData)->ResetStructSize();
                }
                pSData += sizeof (CCStruct **);
                break;
            }
            case cxfv_INFO:
                pSData += sizeof (c_info);
                break;
            case cxfv_REJECT:
                pSData += sizeof (c_reject);
                break;
            case cxfv_CIPVAL:
                pSData += sizeof (c_cipvalue);
                break;
            case cxfv_B1PROT:
                pSData += sizeof (c_b1prot);
                break;
            case cxfv_B2PROT:
                pSData += sizeof (c_b2prot);
                break;
            case cxfv_B3PROT:
                pSData += sizeof (c_b3prot);
                break;
            case cxfv_FACIL:
                pSData += sizeof (c_facilselect);
                break;
            case cxfv_SUPSRV:
                pSData += sizeof (c_supplserv);
                break;
            case cxfv_Z_06:
            case cxfv_Z_05:
            case cxfv_Z_04:
            case cxfv_Z_03:
            case cxfv_Z_02:
            case cxfv_Z_01:
            case cxfv_Z_STRUCT:
                // nothing to do!
                break;
            case cxfv_I_12:
                pSData += 0x12;
                break;
            case cxfv_I_10:
                pSData += 0x10;
                break;
            case cxfv_I_0E:
                pSData += 0x0E;
                break;
            case cxfv_I_0C:
                pSData += 0x0C;
                break;
            case cxfv_I_04:
                pSData += 0x04;
                break;
            case cxfv_I_03:
                pSData += 0x03;
                break;
            case cxfv_I_02:
                pSData += 0x02;
                break;
            default:
                pSData += (tUByte)*format;
                break;
            }
        }
    }
}


/*===========================================================================*\
\*===========================================================================*/

void CCStruct::FillBuffer (tUByte **ppBuffer) {
    dhead ("CCStruct::FillBuffer", DCON_CCStruct);
    dassert (ppBuffer != 0);
    dassert (*ppBuffer != 0);
    //dwarning (m_Len > 0);
    dassert (m_Len <= MAXVAL_tUShort);
    dassert ((m_pData != 0) || (m_Len == 0));
    dassert (m_Format != 0);
    switch (*m_Format) {
    case cxfv_NOLEN:
        // nothing to do!
        break;
    case cxfv_MSGLEN:
        set16 (*ppBuffer, (tUShort)(m_Len + 2));
        *ppBuffer += 2;
        break;
    default:
        if (m_Len > 254) {
            set8 (*ppBuffer,  255);
            (*ppBuffer)++;
            set16 (*ppBuffer, (tUShort)m_Len);
            *ppBuffer += 2;
        } else {
            set8 (*ppBuffer,  (tUByte)m_Len);
            (*ppBuffer)++;
        }
        break;
    }
    FillData (ppBuffer);
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CCStruct::ParseStruct (CCStruct *pStruct) {
    dhead ("CCStruct::ParseStruct", DCON_CCStruct);
    tSInfo fret = i_Okay;
    if (m_ClassType == cxt_RawData) {
        if (pStruct) {
            if (pStruct->CalcStructSize() > MAXVAL_tUShort) {
                dassert (0);
                fret = iErr_Struct_LenOutOfRange;
            } else if (  (m_MaxLen < pStruct->m_Len)
                      && (Resize (pStruct->m_Len) == vFalse)) {
                fret = iErr_OutOfMemory;
            } else {
                dassert (m_pData != 0);
                m_Len         = pStruct->m_Len;
                m_DataType    = pStruct->m_DataType;
                tUByte *pData = m_pData;
                pStruct->FillData (&pData);
            }
        } else {
            m_Len      = 0;
            m_DataType = cxt_RawData;
        }
    } else if (pStruct->GetClassType() != cxt_RawData) {
        dassert (0);
        fret = iErr_Struct_WrongFormat;
    } else {
        dassert (m_MaxLen == 0) ;
        dassert (m_pData != 0);
        if (pStruct) {
            dassert (pStruct->m_DataType == cxt_RawData);
            dassert (pStruct->m_Len <= MAXVAL_tUShort);
            if (pStruct->m_Len && pStruct->m_pData) {
                if (m_Len > pStruct->m_Len) {
                    fret = iErr_Struct_LenToSmall;
                } else {
                    tUByte *pData = pStruct->m_pData;
                    fret          = ParseData (&pData, pStruct->m_Len);
                }
            } else {
                fret = iWrn_Struct_NoChanges;
            }
        } else {
            fret = iWrn_Struct_NoChanges;
        }
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CCStruct::ParseBuffer (tUByte **ppBuffer, tUInt *pLen) {
    dhead ("CCStruct::ParseBuffer", DCON_CCStruct);
    dassert (pLen != 0);
    dassert (ppBuffer != 0);
    dassert (*ppBuffer != 0);   // TODO: ???????????????????
    if (*pLen == 0) {
        dwarning (0);
        RETURN ('x', iErr_Struct_LenToSmall);
    }
    dassert (m_Format != 0);
    tUInt len;
    switch (*m_Format) {
    case cxfv_NOLEN:
        len = ((m_Format[1] == cxfv_CONST) && (m_Len) && (m_pData)) ? m_Len : *pLen;
        break;
    case cxfv_MSGLEN:
        if (*pLen < 0x10) {  // Test for complete Messageheader!
            dwarning (0);
            RETURN ('x', iErr_Struct_WrongLength);
        }
        len = get16 (*ppBuffer);
        *ppBuffer += 2;
        *pLen     -= 2;
        break;
    default:
        len = get8 (*ppBuffer);
        (*ppBuffer)++;
        (*pLen)--;
        if (len > 254) {
            if (*pLen < 2) {
                dwarning (0);
                RETURN ('x', iErr_Struct_WrongLength);
            }
            len = get16 (*ppBuffer);
            *ppBuffer += 2;
            *pLen     -= 2;
        }
        if (len > *pLen) {
            dwarning (0);
            RETURN ('x', iErr_Struct_WrongLength);
        }
        break;
    }
    if (m_Len > len) {
        dwarning (0);
        RETURN ('x', iErr_Struct_LenToSmall);
    }
    if (!len) {
        // Detected an empty Struct
        Reset();
        return i_Okay;
    }
    *pLen -= len;
    return ParseData (ppBuffer, len);
}


/*===========================================================================*\
\*===========================================================================*/

void CCStruct::FillData (tUByte **ppBuffer) {
    dhead ("CCStruct::FillData", DCON_CCStruct);
    dassert (ppBuffer != 0);
    dassert (*ppBuffer != 0);
    //dwarning (m_Len > 0);
    dassert (m_Len <= MAXVAL_tUShort);
    dassert ((m_pData != 0) || (m_Len == 0));
    dassert (m_Format != 0);
    const tFormatChar *format = m_Format;
    switch (*format) {
    case cxfv_NOLEN:
    case cxfv_MSGLEN:
        format++;
        break;
    }
    if (*format == cxfv_CONST) {
        s_memcpy (*ppBuffer, m_pData, m_Len);
        *ppBuffer += m_Len;
    } else {
        tUByte *pSData = m_pData;
        dassert (pSData != 0);
        for (; *format != '\0'; format++) {
            switch (*format) {
            case cxfv_CONST:
            case cxfv_NOLEN:
            case cxfv_MSGLEN:
                dassert (0);
                break;
            case cxfv_STRUCT:
                if (*(CCStruct **)pSData) {
                    (*(CCStruct **)pSData)->FillBuffer (ppBuffer);
                } else {
                    set8 (*ppBuffer, 0);
                    (*ppBuffer)++;
                }
                pSData += sizeof (CCStruct *);
                break;
            case cxfv_STRING:
                if (*(CDynamicString **)pSData) {
                    CDynamicString *pString = *(CDynamicString **)pSData;
                    dassert (m_Len >= pString->GetLen());
                    s_memcpy (*ppBuffer, pString->GetPointer(), pString->GetLen());
                    *ppBuffer += pString->GetLen();
                }
                pSData += sizeof (CDynamicString *);
                break;
            case cxfv_PACKET:
                if (*(CDataPacket **)pSData) {
                    CDataPacket *pPacket = *(CDataPacket **)pSData;
                    dassert (m_Len > pPacket->GetLen());
                    s_memcpy (*ppBuffer, pPacket->GetPointer(), pPacket->GetLen());
                    *ppBuffer += pPacket->GetLen();
                }
                pSData += sizeof (CDataPacket *);
                break;
            case cxfv_ARRAY: {
                c_word count    = *(c_word *)pSData;
                pSData         += sizeof (c_word);
                CCStruct **lauf = *(CCStruct ***)pSData;
                pSData         += sizeof (CCStruct **);
                dwarning (lauf != 0);
                if (lauf) {
                    for (; count > 0; count--, lauf++) {
                        if (*lauf) {
                            (*lauf)->FillBuffer (ppBuffer);
                        } else {
                            set8 (*ppBuffer, 0);
                            (*ppBuffer)++;
                        }
                    }
                }
                break;
            }
            case cxfv_MULTIPLY: {
                c_word count = *(c_word *)pSData;
                pSData      += sizeof (c_word);
                if (*(CCStruct **)pSData) {
                    for (; count > 0; count--) {
                        (*(CCStruct **)pSData)->FillBuffer (ppBuffer);
                    }
                } else {
                    s_memzero (ppBuffer, count);
                    *ppBuffer += count;
                }
                pSData += sizeof (CCStruct *);
                break;
            }
            case cxfv_INFO:
                set16 (*ppBuffer, (tUInt16)*(c_info *)pSData);
                pSData += sizeof (c_info);
                *ppBuffer += 2;
                break;
            case cxfv_REJECT:
                set16 (*ppBuffer, (tUInt16)*(c_reject *)pSData);
                pSData += sizeof (c_reject);
                *ppBuffer += 2;
                break;
            case cxfv_CIPVAL:
                set16 (*ppBuffer, (tUInt16)*(c_cipvalue *)pSData);
                pSData += sizeof (c_cipvalue);
                *ppBuffer += 2;
                break;
            case cxfv_B1PROT:
                set16 (*ppBuffer, (tUInt16)*(c_b1prot *)pSData);
                pSData += sizeof (c_b1prot);
                *ppBuffer += 2;
                break;
            case cxfv_B2PROT:
                set16 (*ppBuffer, (tUInt16)*(c_b2prot *)pSData);
                pSData += sizeof (c_b2prot);
                *ppBuffer += 2;
                break;
            case cxfv_B3PROT:
                set16 (*ppBuffer, (tUInt16)*(c_b3prot *)pSData);
                pSData += sizeof (c_b3prot);
                *ppBuffer += 2;
                break;
            case cxfv_FACIL:
                set16 (*ppBuffer, (tUInt16)*(c_facilselect *)pSData);
                pSData += sizeof (c_facilselect);
                *ppBuffer += 2;
                break;
            case cxfv_SUPSRV:
                set16 (*ppBuffer, (tUInt16)*(c_supplserv *)pSData);
                pSData += sizeof (c_supplserv);
                *ppBuffer += 2;
                break;
            case cxfv_Z_06:
                s_memzero (*ppBuffer, 6);
                *ppBuffer += 0x06;
                break;
            case cxfv_Z_05:
                s_memzero (*ppBuffer, 5);
                *ppBuffer += 0x05;
                break;
            case cxfv_Z_04:
                set32 (*ppBuffer, 0);
                *ppBuffer += 0x04;
                break;
            case cxfv_Z_03:
                s_memzero (*ppBuffer, 3);
                *ppBuffer += 0x03;
                break;
            case cxfv_Z_02:
                set16 (*ppBuffer, 0);
                *ppBuffer += 0x02;
                break;
            case cxfv_Z_01:
            case cxfv_Z_STRUCT:
                set8 (*ppBuffer, 0);
                *ppBuffer += 0x01;
                break;
            case cxfv_I_12:
                pSData += 0x12;
                break;
            case cxfv_I_10:
                pSData += 0x10;
                break;
            case cxfv_I_0E:
                pSData += 0x0E;
                break;
            case cxfv_I_0C:
                pSData += 0x0C;
                break;
            case cxfv_I_04:
                pSData += 0x04;
                break;
            case cxfv_I_03:
                pSData += 0x03;
                break;
            case cxfv_I_02:
                pSData += 0x02;
                break;
            default:
                dassert ((tUByte)*format < 0x10);
                s_memcpy (*ppBuffer, pSData, (tUByte)*format);
                pSData    += *format;
                *ppBuffer += *format;
                break;
            }
        }
    }
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CCStruct::ParseData (tUByte **ppBuffer, tUInt len) {
    dhead ("CCStruct::ParseData", DCON_CCStruct);
    dassert (len > 0);
    dassert (ppBuffer != 0);
    dassert (*ppBuffer != 0);
    dassert (m_Format != 0);
    dprintBuf (*ppBuffer, len);
    const tFormatChar *format = m_Format;
    switch (*format) {
    case cxfv_NOLEN:
    case cxfv_MSGLEN:
        format++;
        break;
    }
    if (*format == cxfv_CONST) {
        if (m_pData == 0) {
            m_pData = *ppBuffer;
            m_Len   = len;
        } else {
            if (m_Len == 0) {
                if ((m_MaxLen < len) && (Resize (len) == vFalse)) {
                    RETURN ('x', iErr_OutOfMemory);
                }
                m_Len = len;
            }
            dassert (m_Len <= len);
            s_memcpy (m_pData, *ppBuffer, len);
        }
        *ppBuffer += len;
    } else {
        tUByte *pSData = m_pData;
        m_Len          = len;
        dassert (pSData != 0);
        for (; *format != '\0'; format++) {
            dprint ("%x.", **ppBuffer);
            switch (*format) {
            case cxfv_CONST:
            case cxfv_NOLEN:
            case cxfv_MSGLEN:
                dassert (0);
                break;
            case cxfv_STRUCT:
                if (*(CCStruct **)pSData) {
                    tSInfo fret = (*(CCStruct **)pSData)->ParseBuffer (ppBuffer, &len);
                    if (infoError (fret)) {
                        dwarning (0);
                        RETURN ('x', fret);
                    }
                    pSData += sizeof (CCStruct *);
                    break;
                } else if (**ppBuffer != '\0') {
                    dassert (0);
                    RETURN ('x', iErr_Struct_WrongFormat);
                } else {
                    (*ppBuffer)++;
                    len--;
                }
                break;
            case cxfv_STRING:
                if (*(CDynamicString **)pSData) {
                    if ((*(CDynamicString **)pSData)->Set (*ppBuffer, len) == vFalse) {
                        dwarning (0);
                        RETURN ('x', iErr_OutOfMemory);
                    }
                } else {
                    dassert (0);
                    // Ist das ein assert, warning oder gar nichts??
                }
                RETURN ('x', i_Okay);
            case cxfv_PACKET:
                if (*(CDataPacket **)pSData) {
                    if (  ((*(CDataPacket **)pSData)->GetMaxLen() < len)
                       && ((*(CDataPacket **)pSData)->Resize (len) == vFalse)) {
                        dwarning (0);
                        RETURN ('x', iErr_OutOfMemory);
                    }
                    s_memcpy ((*(CDataPacket **)pSData)->GetPointer(), *ppBuffer, len);
                    (*(CDataPacket **)pSData)->SetLen (len);
                } else {
                    dassert (0);
                    // Ist das ein assert, warning oder gar nichts??
                }
                RETURN ('x', i_Okay);
            case cxfv_ARRAY: {
                tSInfo fret;
                c_word count    = *(c_word *)pSData;
                pSData         += sizeof (c_word);
                CCStruct **lauf = *(CCStruct ***)pSData;
                pSData         += sizeof (CCStruct **);
                dwarning (lauf != 0);
                if (lauf) {
                    for (; count > 0; count--, lauf++) {
                        if (*lauf) {
                            fret = (*lauf)->ParseBuffer (ppBuffer, &len);
                            if (infoError (fret)) {
                                dwarning (0);
                                RETURN ('x', fret);
                            }
                            break;
                        } else if (**ppBuffer != '\0') {
                            dassert (0);
                            RETURN ('x', iErr_Struct_WrongFormat);
                        } else {
                            (*ppBuffer)++;
                            len--;
                        }
                    }
                }
                break;
            }
            case cxfv_MULTIPLY:
                dassert (0); // MULTIPLY is not parseable!!!
                RETURN ('x', iErr_Struct_WrongFormat);
            case cxfv_INFO:
                if (len < 2) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *(c_info *)pSData = (c_info)get16 (*ppBuffer);
                pSData    += sizeof (c_info);
                *ppBuffer += 2;
                len       -= 2;
                break;
            case cxfv_REJECT:
                if (len < 2) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *(c_reject *)pSData = (c_reject)get16 (*ppBuffer);
                pSData    += sizeof (c_reject);
                *ppBuffer += 2;
                len       -= 2;
                break;
            case cxfv_CIPVAL:
                if (len < 2) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *(c_cipvalue *)pSData = (c_cipvalue)get16 (*ppBuffer);
                pSData    += sizeof (c_cipvalue);
                *ppBuffer += 2;
                len       -= 2;
                break;
            case cxfv_B1PROT:
                if (len < 2) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *(c_b1prot *)pSData = (c_b1prot)get16 (*ppBuffer);
                pSData    += sizeof (c_b1prot);
                *ppBuffer += 2;
                len       -= 2;
                break;
            case cxfv_B2PROT:
                if (len < 2) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *(c_b2prot *)pSData = (c_b2prot)get16 (*ppBuffer);
                pSData    += sizeof (c_b2prot);
                *ppBuffer += 2;
                len       -= 2;
                break;
            case cxfv_B3PROT:
                if (len < 2) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *(c_b3prot *)pSData = (c_b3prot)get16 (*ppBuffer);
                pSData    += sizeof (c_b3prot);
                *ppBuffer += 2;
                len       -= 2;
                break;
            case cxfv_FACIL:
                if (len < 2) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *(c_facilselect *)pSData = (c_facilselect)get16 (*ppBuffer);
                pSData    += sizeof (c_facilselect);
                *ppBuffer += 2;
                len       -= 2;
                break;
            case cxfv_SUPSRV:
                if (len < 2) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *(c_supplserv *)pSData = (c_supplserv)get16 (*ppBuffer);
                pSData    += sizeof (c_supplserv);
                *ppBuffer += 2;
                len       -= 2;
                break;
            case cxfv_Z_06:
                if (len < 0x06) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *ppBuffer += 0x06;
                break;
            case cxfv_Z_05:
                if (len < 0x05) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *ppBuffer += 0x05;
                break;
            case cxfv_Z_04:
                if (len < 0x04) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *ppBuffer += 0x04;
                break;
            case cxfv_Z_03:
                if (len < 0x03) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *ppBuffer += 0x03;
                break;
            case cxfv_Z_02:
                if (len < 0x02) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *ppBuffer += 0x02;
                break;
            case cxfv_Z_01:
                if (len < 0x01) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                *ppBuffer += 0x01;
                break;
            case cxfv_Z_STRUCT:
                if (len < sizeof (c_byte)) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                } else {
                    tUInt dlen = **ppBuffer;
                    (*ppBuffer)++;
                    len--;
                    if (dlen > 254) {
                        if (len < 2) {
                            dwarning (0);
                            RETURN ('x', iErr_Struct_WrongLength);
                        }
                        dlen = get16 (*ppBuffer);
                        *ppBuffer += 2;
                        len       -= 2;
                    }
                    if (dlen > len) {
                        dwarning (0);
                        RETURN ('x', iErr_Struct_WrongLength);
                    }
                    *ppBuffer += dlen;
                    len       -= dlen;
                }
                break;
            case cxfv_I_12:
                pSData += 0x12;
                break;
            case cxfv_I_10:
                pSData += 0x10;
                break;
            case cxfv_I_0E:
                pSData += 0x0E;
                break;
            case cxfv_I_0C:
                pSData += 0x0C;
                break;
            case cxfv_I_04:
                pSData += 0x04;
                break;
            case cxfv_I_03:
                pSData += 0x03;
                break;
            case cxfv_I_02:
                pSData += 0x02;
                break;
            default:
                dassert ((tUByte)*format < cxfv_I_02);
                if (len < (tUByte)*format) {
                    dassert (0);
                    RETURN ('x', iErr_Struct_LenToSmall);
                }
                s_memcpy (pSData, *ppBuffer, (tUByte)*format);
                pSData    += (tUByte)*format;
                *ppBuffer += (tUByte)*format;
                len       -= (tUByte)*format;
                break;
            }
        }
        if (len) {
            m_Len -= len;
            if (*m_Format != cxfv_NOLEN) {
                *ppBuffer += len;
            }
        }
    }
    RETURN ('x', i_Okay);
}


/*===========================================================================*\
\*===========================================================================*/

tBool CCStruct::Resize (tUInt maxlen) {
    dhead ("CCStruct::Resize", DCON_CCStruct);
    dparams ("%x", maxlen);
    dassert (maxlen > 0);
    dwarning (m_ClassType == cxt_RawData);
    if (maxlen) {
        tUByte *newData = new tUByte [maxlen];
        if (!newData) {
            RETURN ('x', vFalse);
        }
        if (m_MaxLen) {
            dprint ("-old deleted- ");
            delete [] m_pData;
        }
        m_pData  = newData;
        m_Len    = 0;
        m_MaxLen = maxlen;
    }
    RETURN ('x', vTrue);
}


/*===========================================================================*\
\*===========================================================================*/
