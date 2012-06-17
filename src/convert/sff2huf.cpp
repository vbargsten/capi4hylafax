/*---------------------------------------------------------------------------*\
 * Copyright (C) 2000 AVM GmbH. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, and WITHOUT
 * ANY LIABILITY FOR ANY DAMAGES arising out of or in connection
 * with the use or performance of this software. See the
 * GNU General Public License for further details.
\*---------------------------------------------------------------------------*/

#include <string.h>
#include "sff2huf.h"
#include "revbyte.h"
#include "osImport.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define SFF_DOCUMENT_HEADER_SIZE        20
#define SFF_PAGE_HEADER_SIZE            18

// max number of bytes caused by expansion of empty lines
// + additional 40 bytes safty margin
#define HUFF_SFF_ADD_TO_PUT_IN          ((253 - 216) * (HUF_EMPTY_LINE_CHAR_SIZE + 3) + 40)
#define HUF_END_OF_LINE_CHAR_SIZE       2
//#define HUF_END_OF_PAGE_CHAR_SIZE       2
#define HUF_END_OF_PAGE_CHAR_SIZE       8
#define HUF_EMPTY_LINE_CHAR_SIZE        4
#define SFF                             0x66666653
#define DLE_CHAR                        0x10

/*===========================================================================*\
\*===========================================================================*/

CSFilter_Sff2Huf::CSFilter_Sff2Huf (void) {
    dhead ("CSFilter_Sff2Huf - Constructor", DCON_SffFilter);
}

/*===========================================================================*\
\*===========================================================================*/

CSFilter_Sff2Huf::~CSFilter_Sff2Huf (void) {
    dhead ("CSFilter_Sff2Huf - Destructor", DCON_SffFilter);
}

/*===========================================================================*\
\*===========================================================================*/

tBool CSFilter_Sff2Huf::Open (tUInt DataBitOrder) {
    dhead ("CSFilter_Sff2Huf::Open", DCON_SffFilter);
    rxSFFInit (DataBitOrder);
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Sff2Huf::Close (void) {
    dhead ("CSFilter_Sff2Huf::Close", DCON_SffFilter);
}

/*===========================================================================*\
\*===========================================================================*/

CSFilter_Sff2Huf::ctInfo CSFilter_Sff2Huf::PutChar (tUChar ch) {
    dhead ("CSFilter_Sff2Huf::PutChar", DCON_SffFilter);
    ctInfo fret = i_Normal;
    dassert (rxSff.count < MAX_SFF_HUF_BUFFER_LEN);

    if (rxSff.count > MAX_SFF_HUF_BUFFER_LEN - HUFF_SFF_ADD_TO_PUT_IN) {
        return i_QueueFull;
    }
    if (rxSff.Status != rxSFF_End) {
        PutQueueData (ch);
    }
    switch (rxSff.Status) {
    case rxSFF_Start:
        if (rxSff.count >= sizeof (tUInt)) {
            tUInt sffID;
            GetQueueData (&sffID, sizeof (tUInt));
            if (sffID != SFF) {
                rxSFFInit (rxSff.BitOrder);
                fret = i_Error_NoFax;
            } else  {
                rxSff.Status = rxSFF_RecvDocHeader;
            }
        }
        break;

    case rxSFF_RecvDocHeader:
        if (rxSff.count >= (SFF_DOCUMENT_HEADER_SIZE - sizeof (tUInt))) {
            IgnoreQueueData (6 * sizeof (tUByte));
            GetQueueData (&rxSff.OffsetFirstPage, sizeof (tUShort));
            IgnoreQueueData (8 * sizeof (tUByte));
            rxSff.OffsetFirstPage = (tUShort) ((rxSff.OffsetFirstPage <= SFF_DOCUMENT_HEADER_SIZE) ?
                                   0 : rxSff.OffsetFirstPage - SFF_DOCUMENT_HEADER_SIZE);
            rxSff.Status = rxSFF_GetDocHeader;
        }
        break;

    case rxSFF_GetDocHeader:
        if (rxSff.count > rxSff.OffsetFirstPage) {
            IgnoreQueueData (rxSff.OffsetFirstPage);
            if (ch != 254) {
                rxSFFInit (rxSff.BitOrder);
                fret = i_Error_NoFax;
            } else {
                rxSff.Status = rxSFF_RecvPageHeaderLen;
            }
            rxSff.offset = rxSff.count;
        }
        break;

    case rxSFF_RecvPageHeaderLen:
        if (ch) {
            if (ch < SFF_PAGE_HEADER_SIZE - 2 * sizeof (tUByte)) {
                rxSFFInit (rxSff.BitOrder);
                fret = i_Error_IllegalCoding;
            } else {
                rxSff.OffsetPageHeader = ch + sizeof (tUByte);
                rxSff.Status           = rxSFF_RecvPageHeader;
            }
        } else {
            dassert (rxSff.count <= MAX_SFF_HUF_BUFFER_LEN - HUF_END_OF_PAGE_CHAR_SIZE);
            PutQueueData (0x00);
            PutQueueData (0x08);
            PutQueueData (0x80);
            PutQueueData (0x00);
            PutQueueData (0x08);
            PutQueueData (0x80);
            PutQueueData (0x00);
            PutQueueData (0x80);
            //PutQueueData (DLE_CHAR);
            //PutQueueData (0x03);
            rxSff.VirtCount += HUF_END_OF_PAGE_CHAR_SIZE;
            rxSff.CountDataHeaders++;
            rxSff.Status = rxSFF_End;
            fret = i_LastPage;
        }
        break;

    case rxSFF_RecvPageHeader:
        if (rxSff.count - rxSff.offset >= rxSff.OffsetPageHeader) {
            if (rxSff.PageInfo.PageNumber == 0) {
                /*----- First PageHeader received -----*/
                /*----- remember VResolution and drop PageHeader -----*/
                rxSff.offset  = 0;
                rxSff.PageLen = 0;
                rxSff.PageInfo.PageNumber++;
                IgnoreQueueData (2 * sizeof (tUByte));
                GetQueueData (&rxSff.PageInfo.ResolutionVertical, sizeof (tUByte));
                GetQueueData (&rxSff.PageInfo.ResolutionHorizontal, sizeof (tUByte));
                IgnoreQueueData (2 * sizeof (tUByte));
                GetQueueData (&rxSff.PageInfo.LineLength, sizeof (tUShort));
                GetQueueData (&rxSff.PageInfo.PageLength, sizeof (tUShort));
                IgnoreQueueData (rxSff.OffsetPageHeader - 9);
                dassert (rxSff.count == 0);
                dassert (rxSff.CountDataHeaders == 0);
            } else {
                dassert (rxSff.count <= MAX_SFF_HUF_BUFFER_LEN - HUF_END_OF_PAGE_CHAR_SIZE);
                PutQueueData (0x00);
                PutQueueData (0x08);
                PutQueueData (0x80);
                PutQueueData (0x00);
                PutQueueData (0x08);
                PutQueueData (0x80);
                PutQueueData (0x00);
                PutQueueData (0x80);
                //PutQueueData (DLE_CHAR);
                //PutQueueData (0x03);
                rxSff.VirtCount += HUF_END_OF_PAGE_CHAR_SIZE;
                rxSff.CountDataHeaders++;
            }
            rxSff.Status = rxSFF_RecvDataHeader;
        }
        break;

    case rxSFF_RecvSpecialLen:
        if (ch) {
            rxSff.offset = rxSff.count;
            rxSff.LineLen = ch;
            rxSff.Status = rxSFF_GetSpecialLen;
        } else {
            rxSff.CountIllegalLines++;
            rxSff.CountAllLines++;
            rxSff.CountDataHeaders++;
            rxSff.Status = rxSFF_RecvDataHeader;
        }
        break;

    case rxSFF_GetSpecialLen:
        if (rxSff.count - rxSff.offset >= rxSff.LineLen) {
            rxSff.CountDataHeaders++;
            rxSff.Status = rxSFF_RecvDataHeader;
        }
        break;

    case rxSFF_RecvDataHeader:
        switch (ch) {
        case 255:
            rxSff.Status = rxSFF_RecvSpecialLen;
            break;
        case 254:
            rxSff.Status = rxSFF_RecvPageHeaderLen;
            break;
        case 0:
            rxSff.Status = rxSFF_RecvDataHeaderEx;
            break;
        default:
            if (ch <= 216) {
                /*----- save length always in the same format -----*/
                dassert (rxSff.count <= MAX_SFF_HUF_BUFFER_LEN - sizeof (tUShort));
                PutQueueData (ch);
                PutQueueData (0);
                rxSff.LineLen = ch;
                rxSff.Status = rxSFF_RecvData;
            } else {
                tUShort EmptyLen;
                ch -= 216;
                EmptyLen = (tUShort) (ch * (HUF_EMPTY_LINE_CHAR_SIZE + 3) - 1);
                if (rxSff.count + EmptyLen > MAX_SFF_HUF_BUFFER_LEN - 2) {
                    fret = i_QueueFull;
                    break;
                }
                rxSff.VirtCount += EmptyLen;
                rxSff.CountAllLines += ch;
                //rxSff.CountDataHeaders++;
                //PutQueueData ((tUChar)(EmptyLen & 0xFF));
                //PutQueueData ((tUChar)(EmptyLen >> 8));
                //rxSff.Status = rxSFF_RecvData;
                for (; ch > 0; ch--) {
                    rxSff.CountDataHeaders++;
                    PutQueueData (4);
                    PutQueueData (0);

                    rxSff.Status = rxSFF_RecvData;
                    /*----- Add characters for a empty line -----*/
                    PutQueueData (0xB2);
                    PutQueueData (0x59);
                    PutQueueData (0x01);
                    PutQueueData (0x80);
                    rxSff.Status = rxSFF_RecvDataHeader;
                    if (ch > 1) {
                        PutQueueData (0);
                    }
                }
            }
            break;
        }
        rxSff.offset = rxSff.count;
        break;

    case rxSFF_RecvDataHeaderEx:
        if (rxSff.count - rxSff.offset >= sizeof (tUShort)) {
            LookQueueData (rxSff.offset, &rxSff.LineLen, sizeof (tUShort));
            dassert (rxSff.LineLen > 216);
            dassert (rxSff.LineLen <= 32767);
            rxSff.offset = rxSff.count;
            rxSff.Status = (rxSff.LineLen != 0) ? rxSFF_RecvData : rxSFF_RecvDataHeader;
        }
        break;

    case rxSFF_RecvData:
        dassert (rxSff.count - rxSff.offset <= rxSff.LineLen);
        rxSff.VirtCount++;
        //if (((rxSff.BitOrder) ? ch : REVBYTE[ch]) == DLE_CHAR) {
        //    /*----- Escape-characters must be added twice -----*/
        //    dassert (rxSff.count < MAX_SFF_HUF_BUFFER_LEN);
        //    PutQueueData (ch);
        //    rxSff.LineLen++;
        //    rxSff.VirtCount++;
        //}

        if (rxSff.count - rxSff.offset >= rxSff.LineLen) {
            /*----- add EndOfLine-character -----*/
            dassert (rxSff.count <= MAX_SFF_HUF_BUFFER_LEN - HUF_END_OF_LINE_CHAR_SIZE);
            PutQueueData (0x00);
            PutQueueData (0x80);
            rxSff.LineLen += HUF_END_OF_LINE_CHAR_SIZE;
            rxSff.VirtCount += HUF_END_OF_LINE_CHAR_SIZE;

            /*----- remember the changed LineLen in the right place!! -----*/
            dassert (rxSff.offset > sizeof (tUShort));
            dassert (rxSff.offset + rxSff.LineLen == rxSff.count);
            SetInQueueData (rxSff.offset - sizeof (tUShort), &rxSff.LineLen, sizeof (tUShort));
            rxSff.CountAllLines++;
            rxSff.CountDataHeaders++;
            rxSff.Status = rxSFF_RecvDataHeader;
            //cprint ("->SFFSetLineLen=%x, DataHeader=%x\n", rxSff.LineLen, rxSff.CountDataHeaders);
        }
        break;

    case rxSFF_End:
    default:
        dassert (0);
        break;
    }
    RETURN ('x', fret);
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CSFilter_Sff2Huf::GetStrip (tUChar *buffer, tUInt len, CSFilter_Sff2Huf::ctInfo *info) {
    dhead ("CSFilter_Sff2Huf::GetStrip", DCON_SffFilter);
    dassert (buffer != 0);
    dassert (len > 0);
    dassert (info != 0);

    tUInt size = CurStripSize();
    if (!size) {
        *info = i_StripNotComplete;
        return 0;
    } else if (size > len) {
        *info = i_BufferToSmall;
        return 0;
    }
    return GetToPageEnd (buffer, len, info);
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CSFilter_Sff2Huf::GetToPageEnd (tUChar *buffer, tUInt maxLen, CSFilter_Sff2Huf::ctInfo *info) {
    dhead ("CSFilter_Sff2Huf::GetToPageEnd", DCON_SffFilter);
    unsigned n;
    int SendBytes = 0;
    dassert (buffer != 0);
    dassert (info != 0);
    dassert (maxLen >= HUF_END_OF_PAGE_CHAR_SIZE);

    *info = rxSff.info;
    rxSff.info = i_Normal;
    if (!rxSff.VirtCount || *info != i_Normal) {
        return 0;
    }
    //dwarning (rxSff.count > 0);

    while (maxLen > 0) {
        if (rxSff.DeqLineLen) {
            dassert (rxSff.DeqLineLen <= rxSff.VirtCount);
            n = (maxLen > rxSff.DeqLineLen) ? rxSff.DeqLineLen : maxLen;
            dassert (n <= rxSff.count);
            GetQueueData (buffer, n);
            buffer += n;
            SendBytes += n;
            maxLen -= n;
            rxSff.VirtCount -= n;
            rxSff.DeqLineLen = (tUShort)(rxSff.DeqLineLen - n);
        }
        if (!rxSff.DeqLineLen) {
            // calculate the next value for DeqLineLen
            // Note: it´s important not to wait for the next loop, because if called from GetStrip
            //       maxLen is always == DeqLineLen and so GetStrip never gets an LastPageEnd info
            tUByte DataHeader;
            if (rxSff.info != i_Normal) {
                /*----- Bei erreichen des Seitenende sofort Funktion benden -----*/
                *info = rxSff.info;
                rxSff.info = i_Normal;
                break;
            }
            if (!rxSff.CountDataHeaders) {
                /*----- Es ist noch nicht genügend da -> Beende Funktion -----*/
                break;
            }
            rxSff.CountDataHeaders--;
            GetQueueData (&DataHeader, sizeof (tUByte));
            switch (DataHeader) {
            case 255: {
                tUByte IgnoreLen;
                dassert (rxSff.count >= sizeof (tUByte));
                GetQueueData (&IgnoreLen, sizeof (tUByte));
                if (IgnoreLen) {
                    IgnoreQueueData (IgnoreLen);
                }
                break; }
            case 254: {
                tUByte PageHeaderLen;
                dassert (rxSff.count >= sizeof (tUByte) + HUF_END_OF_PAGE_CHAR_SIZE);
                rxSff.OldPageInfo            = rxSff.PageInfo;
                rxSff.OldPageInfo.PageLength = rxSff.PageLen; // remember the real PageLen
                GetQueueData (&PageHeaderLen, sizeof (tUByte));
                if (PageHeaderLen) {
                    dassert (PageHeaderLen >= SFF_PAGE_HEADER_SIZE - 2 * sizeof (tUByte));
                    dassert (rxSff.count >= SFF_PAGE_HEADER_SIZE + HUF_END_OF_PAGE_CHAR_SIZE - 2);
                    rxSff.info = i_NewPage;
                    rxSff.PageInfo.PageNumber++;
                    GetQueueData (&rxSff.PageInfo.ResolutionVertical, sizeof (tUByte));
                    GetQueueData (&rxSff.PageInfo.ResolutionHorizontal, sizeof (tUByte));
                    IgnoreQueueData (2 * sizeof (tUByte));
                    GetQueueData (&rxSff.PageInfo.LineLength, sizeof (tUShort));
                    GetQueueData (&rxSff.PageInfo.PageLength, sizeof (tUShort));
                    IgnoreQueueData (PageHeaderLen - 8);
                } else {
                    rxSff.info                = i_LastPage;
                    rxSff.PageInfo.PageLength = rxSff.PageLen;
                    dassert (rxSff.Status == rxSFF_End);
                }
                rxSff.PageLen    = 0;
                rxSff.DeqLineLen = HUF_END_OF_PAGE_CHAR_SIZE;
                break; }
            default:
                rxSff.PageLen++;
                dassert (rxSff.count > sizeof (tUShort));
                GetQueueData (&rxSff.DeqLineLen, sizeof (tUShort));
                //cprint ("<-SFFGetLineLen=%x, Count=%x, CountHeader=%x\n", rxSff.DeqLineLen, rxSff.count,
                //        rxSff.CountDataHeaders);
                dassert (rxSff.DeqLineLen > 0);
                dassert (rxSff.DeqLineLen < 32767);
                dassert (rxSff.DeqLineLen <= rxSff.count);
                break;
            }
        }
    }
    return SendBytes;
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CSFilter_Sff2Huf::CurStripSize (void) {
    dhead ("CSFilter_Sff2Huf::CurStripSize", DCON_SffFilter);
    if (rxSff.DeqLineLen) {
        // return rest of current strip
        return rxSff.DeqLineLen;
    }
    tUByte  dummy;
    tUInt   offset = 0;
    tUShort size   = 0;
    tUInt   cHead  = rxSff.CountDataHeaders;

    // loop over all PageHeader and DataLines till a valid strip size is found
    for (; cHead && !size; cHead--) {
        // get first byte of data header
        LookQueueData (offset++, &dummy, sizeof (tUByte));
        if (dummy == 255) {
            LookQueueData (offset++, &dummy, sizeof (tUByte));
            offset += dummy;
        } else if (dummy == 254) {
            LookQueueData (offset++, &dummy, sizeof (tUByte));
            if (dummy) {
                offset += SFF_PAGE_HEADER_SIZE - 2;
            } else {
                // end of fax reached and no strip size found
                break;
            }
        } else {
            LookQueueData (offset, &size, sizeof (tUShort));
            offset += sizeof (tUShort);
        }
    }
    return size;
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CSFilter_Sff2Huf::CurSize (void) {
    dhead ("CSFilter_Sff2Huf::CurSize", DCON_SffFilter);
    dassert (rxSff.count >= rxSff.VirtCount);
    return (rxSff.CountDataHeaders) ? rxSff.VirtCount : 0;
}

/*===========================================================================*\
\*===========================================================================*/

tBool CSFilter_Sff2Huf::GetCurPageInfo (ctPageInfo *pPInfo) {
    dhead ("CSFilter_Sff2Huf::GetCurPageInfo", DCON_SffFilter);
    dassert (pPInfo != 0);
    if (rxSff.PageInfo.PageNumber > 0) {
        *pPInfo = rxSff.PageInfo;
        return vTrue;
    }
    return vFalse;
}

/*===========================================================================*\
\*===========================================================================*/

tBool CSFilter_Sff2Huf::GetLastPageInfo (ctPageInfo *pPInfo) {
    dhead ("CSFilter_Sff2Huf::GetLastPageInfo", DCON_SffFilter);
    dassert (pPInfo != 0);
    if (rxSff.OldPageInfo.PageNumber > 0) {
        *pPInfo = rxSff.OldPageInfo;
        return vTrue;
    }
    return vFalse;
}

/*===========================================================================*\
\*===========================================================================*/

tBool CSFilter_Sff2Huf::GetIllegalLines (tUInt *pCountIllegalLines, tUInt *pCountAllLines) {
    dhead ("CSFilter_Sff2Huf::GetIllegalLines", DCON_SffFilter);
    if (pCountIllegalLines) *pCountIllegalLines = rxSff.CountIllegalLines;
    if (pCountAllLines)     *pCountAllLines     = rxSff.CountAllLines;
    return (rxSff.CountIllegalLines || !rxSff.CountAllLines);
}


/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Sff2Huf::rxSFFInit (tUInt DataBitOrder) {
    dhead ("CSFilter_Sff2Huf::rxSFFInit", DCON_SffFilter);
    dassert (DataBitOrder <= 1);

    memset (&rxSff, 0, sizeof (RXSFF));
    rxSff.Status      = rxSFF_Start;
    //rxSff.VResolution = -1;
    rxSff.info        = i_Normal;
    rxSff.BitOrder    = DataBitOrder;
}

/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Sff2Huf::PutQueueData (tUChar c) {
    dhead ("CSFilter_Sff2Huf::PutQueueData", DCON_SffFilter);
    dassert (rxSff.count < MAX_SFF_HUF_BUFFER_LEN);

    rxSff.buffer[rxSff.head] = (!rxSff.BitOrder && (rxSff.Status == rxSFF_RecvData)) ? REVBYTE[c] : c;
    rxSff.count++;
    if (++rxSff.head >= MAX_SFF_HUF_BUFFER_LEN) {
        rxSff.head = 0;
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Sff2Huf::IgnoreQueueData (tUInt len) {
    dhead ("CSFilter_Sff2Huf::IgnoreQueueData", DCON_SffFilter);
    dassert (rxSff.count >= len);

    rxSff.count -= len;
    if (rxSff.offset) {
        dassert (rxSff.offset >= len);
        rxSff.offset -= len;
    }
    rxSff.tail += len;
    if (rxSff.tail >= MAX_SFF_HUF_BUFFER_LEN) {
        rxSff.tail -= MAX_SFF_HUF_BUFFER_LEN;
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Sff2Huf::GetQueueData (void *buf, tUInt len) {
    dhead ("CSFilter_Sff2Huf::GetQueueData", DCON_SffFilter);
    dassert (buf != 0);
    dassert (rxSff.count >= len);

    rxSff.count -= len;
    if (rxSff.offset) {
        dassert (rxSff.offset >= len);
        rxSff.offset -= len;
    }
    if (rxSff.tail + len > MAX_SFF_HUF_BUFFER_LEN) {
        tUInt dummy = MAX_SFF_HUF_BUFFER_LEN - rxSff.tail;
        memcpy (buf, rxSff.buffer + rxSff.tail, dummy);
        rxSff.tail = len - dummy;
        memcpy (((char *)buf) + dummy, rxSff.buffer, rxSff.tail);
    } else {
        memcpy (buf, rxSff.buffer + rxSff.tail, len);
        rxSff.tail += len;
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Sff2Huf::LookQueueData (tUInt offset, void *buf, tUInt len) {
    dhead ("CSFilter_Sff2Huf::LookQueueData", DCON_SffFilter);
    dassert (buf != 0);
    dassert (rxSff.count >= offset + len);

    offset += rxSff.tail;
    if (offset >= MAX_SFF_HUF_BUFFER_LEN) {
        offset -= MAX_SFF_HUF_BUFFER_LEN;
    }
    if (offset + len > MAX_SFF_HUF_BUFFER_LEN) {
        tUInt dummy = MAX_SFF_HUF_BUFFER_LEN - offset;
        memcpy (buf, rxSff.buffer + offset, dummy);
        memcpy (((char *)buf) + dummy, rxSff.buffer, len - dummy);
    } else {
        memcpy (buf, rxSff.buffer + offset, len);
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Sff2Huf::SetInQueueData (tUInt offset, void *buf, tUInt len) {
    dhead ("CSFilter_Sff2Huf::SetInQueueData", DCON_SffFilter);
    dassert (buf != 0);
    dassert (rxSff.count >= offset + len);

    offset += rxSff.tail;
    if (offset >= MAX_SFF_HUF_BUFFER_LEN) {
        offset -= MAX_SFF_HUF_BUFFER_LEN;
    }
    if (offset + len > MAX_SFF_HUF_BUFFER_LEN) {
        tUInt dummy = MAX_SFF_HUF_BUFFER_LEN - offset;
        memcpy (rxSff.buffer + offset, buf, dummy);
        memcpy (rxSff.buffer, ((char *)buf) + dummy, len - dummy);
    } else {
        memcpy (rxSff.buffer + offset, buf, len);
    }
}

/*===========================================================================*\
\*===========================================================================*/


