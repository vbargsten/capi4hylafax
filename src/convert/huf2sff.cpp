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
#include "huf2sff.h"
#include "hufman.h"
#include "osImport.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define HUF_END_OF_LINE_CHAR_SIZE       2
#define HUF_END_OF_PAGE_CHAR_SIZE       2
#define HUF_EMPTY_LINE_CHAR_SIZE        4
#define HUFF_SFF_ADD_TO_PUT_IN          4

#define STANDARD_VER_RES                0           /*  98 lpi  Standard */
#define FEIN_VER_RES                    1           /* 196 lpi  Fein     */
#define HOCHFEIN_VER_RES                2           /* 392 lpi  Hochfein */

#define STANDARD_HOR_RES                0           /* 203 dpi  Standard */

#define MH_CODING                       0
#define SFF                             0x66666653

typedef struct ff {   
    unsigned int    Kennung;
    unsigned char   Version;
    unsigned char   reserved;
    unsigned short  userInfo;
    unsigned short  numPages;
    unsigned short  firstPage;
    unsigned int    lastPage;
    unsigned int    fileSize;
    unsigned char   ID[8];
} SFF_HEADER;

typedef struct ssf_page {   
    unsigned char   vert_res;
    unsigned char   horiz_res;
    unsigned char   coding;
    unsigned char   specials;
    unsigned short  xsize;
    unsigned short  ysize;
    unsigned int    prevPage;
    unsigned int    nextPage;
} SFF_PAGE;

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define MODULO      (QUEUESIZE-1)
#define FSIZE       (( QUEUESIZE*3) / 4)
#define SUCC(x)     (((x)+1)& MODULO )
#define LEER(q)     ( (q)->Qread == (q)->Qwrite )
#define VOLL(q)     (SUCC((q)->Qwrite)==((q)->Qread))
#define FILL(q)     (((q)->Qwrite + QUEUESIZE - (q)->Qread ) & MODULO )
#define FREE(q)     (MODULO - FILL(q))

/*===========================================================================*\
\*===========================================================================*/

CSFilter_Huf2Sff::CSFilter_Huf2Sff (void) {
    dhead ("CSFilter_Huf2Sff - Constructor", DCON_SffFilter);
}

/*===========================================================================*\
\*===========================================================================*/

CSFilter_Huf2Sff::~CSFilter_Huf2Sff (void) {
    dhead ("CSFilter_Huf2Sff - Destructor", DCON_SffFilter);
}

/*===========================================================================*\
\*===========================================================================*/

tBool CSFilter_Huf2Sff::Open (tUInt vRes, tUInt /*DataBitOrder*/) {
    dhead ("CSFilter_Huf2Sff::Open", DCON_SffFilter);
    SFF_HEADER header;
    int        len;

    memset (&txHuf, 0, sizeof(TXHUF));
    txHuf.lineValid     = 0;
    txHuf.default_v_res = vRes;
    HUFMANinit();

    len               = sizeof (SFF_HEADER);
    memset ((char *) &header, 0, len);
    header.Kennung    = SFF;
    header.Version    =   1;
    header.numPages   =   0;    /* Weisnix erst am Ende */
    header.firstPage  = (unsigned short) len;
    header.lastPage   =   0;    /* Weisnix erst am Ende */
    header.fileSize   =   0;    /* Weisnix erst am Ende */
    memcpy (header.ID, "(c) AVM ", 8);
    HUFenq ((unsigned char *)&header, len);
    return vTrue;  
}


/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Huf2Sff::Close (void) {
    dhead ("CSFilter_Huf2Sff::Close", DCON_SffFilter);
    memset (&txHuf, 0, sizeof(TXHUF));
}
 
/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Huf2Sff::PutPageHeader (tBool lastPage) {
    dhead ("CSFilter_Huf2Sff::PutPageHeader", DCON_SffFilter);
    SFF_PAGE    sp;
    int         len;
    len = sizeof (SFF_PAGE);
    HUFenq1 (254);
    HUFenq1 (lastPage ? 0 : len);
    if (lastPage) {
        return;
    }

    /*---------------------------------------------------------------------*/
    memset ((char *)&sp, 0, len);
    sp.vert_res  = (unsigned char) txHuf.default_v_res; // STANDARD_VER_RES;
    sp.horiz_res = STANDARD_HOR_RES;
    sp.coding    = MH_CODING;
    sp.xsize     = 1728;
    HUFenq ((unsigned char *)&sp, len);
}
 
/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Huf2Sff::PutChar (tUChar ch) {
    dhead ("CSFilter_Huf2Sff::PutChar", Dbg_Level_Infos); //DCON_SffFilter);
    /*---------------------------------------------------------------------*/
    //if (txHuf.inESCAPE) { 
    //    txHuf.inESCAPE = 0;
    //    switch (ch) {
    //    case 0x10 :  /* DLE DLE -> DLE senden */
    //        break;
    //    case 0x03 :  /* DLE 03  -> End of Page */
    //        txHuf.lineValid = 0;
    //        txHuf.EOP       = 1;
    //        return;
    //    case 0x12 :  /* DLE DLE -> DLE DLE senden */
    //        break;
    //    default :
    //        break;
    //    }
    //} else if (ch == 0x10) { 
    //    txHuf.inESCAPE = 1;
    //    return;
    //}

    /*---------------------------------------------------------------------*/
    HUFtstBit (ch & 0x01);
    HUFtstBit (ch & 0x02);
    HUFtstBit (ch & 0x04);
    HUFtstBit (ch & 0x08);
    HUFtstBit (ch & 0x10);
    HUFtstBit (ch & 0x20);
    HUFtstBit (ch & 0x40);
    HUFtstBit (ch & 0x80);
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CSFilter_Huf2Sff::Get (tUChar *buffer, tUInt maxLen) {
    dhead ("CSFilter_Huf2Sff::Get", DCON_SffFilter);
    int         len1;
    int         len2;
    DATA_QUEUE *q    = &txHuf.dq;
    int         i    = FILL (q);
    if (maxLen > (tUInt)i) {
        maxLen = i;
    }
    if (!maxLen) {
        return 0;
    }
    len1 = q->Qread + maxLen > QUEUESIZE ? QUEUESIZE - q->Qread : maxLen;
    len2 = maxLen - len1;
    if (maxLen <= 0 || len1 <= 0 || len2 < 0 || maxLen > QUEUESIZE) {
        return 0;
    }
    memcpy (buffer, q->Qdata + q->Qread, len1);
    q->Qread = ( q->Qread + len1 ) & MODULO;

    if (len2) { 
        memcpy (buffer + len1, q->Qdata + q->Qread, len2);
        q->Qread = (q->Qread + len2) & MODULO;
    }
    RETURN ('x', maxLen);
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CSFilter_Huf2Sff::CurSize (void) {
    dhead ("CSFilter_Huf2Sff::CurSize", DCON_SffFilter);
    RETURN ('x', FILL (&txHuf.dq));
}

/*===========================================================================*\
\*===========================================================================*/

tBool CSFilter_Huf2Sff::IsEOP (void) {
    dhead ("CSFilter_Huf2Sff::IsEOP", DCON_SffFilter);
    if (txHuf.EOP) {
        txHuf.EOP = 0; 
        RETURN ('x', vTrue);
    }
    RETURN ('x', vFalse);
}

/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Huf2Sff::HUFMANinit (void) {
    dhead ("CSFilter_Huf2Sff::HUFMANinit", Dbg_Level_Infos); //DCON_SffFilter);
    txHuf.curInd    = 
    txHuf.numBytes  =
    txHuf.run0      = 
    txHuf.run1      =
    txHuf.EOLfound  = 
    txHuf.EOLcount0 =
    txHuf.outByte   = 
    txHuf.outBits   = 0;
    txHuf.curNode   = WhiteHUFS;
    txHuf.white     = 1;
}

/*===========================================================================*\
\*===========================================================================*/

int CSFilter_Huf2Sff::HUFenq (unsigned char *buff, int len) {
    dhead ("CSFilter_Huf2Sff::HUFenq", Dbg_Level_Infos); //DCON_SffFilter);
    DATA_QUEUE *q = &txHuf.dq;
    int free, len1, len2;
    free = FREE (q);
    if (len > free) {
        len = free;
    }
    if (!len) {
        return 0;
    }

    len1 = q->Qwrite + len > QUEUESIZE ? QUEUESIZE - q->Qwrite : len;
    len2 = len - len1;
    if (len <= 0 || len1 <= 0 || len2 < 0 || len > QUEUESIZE) {
        return 0;
    }
    memcpy (q->Qdata + q->Qwrite, buff, len1);
    q->Qwrite = (q->Qwrite + len1) & MODULO;

    if (len2) { memcpy( q->Qdata + q->Qwrite, buff + len1, len2 );
      q->Qwrite = (q->Qwrite + len2) & MODULO;
    }
    return len ;
}

/*===========================================================================*\
\*===========================================================================*/

int CSFilter_Huf2Sff::HUFenq1 (int val) {
    dhead ("CSFilter_Huf2Sff::HUFenq1", Dbg_Level_Infos); //DCON_SffFilter);
    DATA_QUEUE *q = &txHuf.dq;
    if (VOLL (q)) {
        return 0;
    }
    q->Qdata [q->Qwrite] = (unsigned char) val;
    q->Qwrite = SUCC (q->Qwrite);
    return 1; 
}
 
/*=====================================================================*\
\*=====================================================================*/

void CSFilter_Huf2Sff::HUFtstBit (int bit) {
    dhead ("CSFilter_Huf2Sff::HUFtstBit", Dbg_Level_Infos); //DCON_SffFilter);
    int         huf;
    int         len;

    bit = bit ? 1 : 0;
    if (!txHuf.EOLfound && txHuf.lineValid) { 
        txHuf.outByte = ( txHuf.outByte >> 1 );
        if (bit) {
            txHuf.outByte |= 0x80;
        }
        txHuf.outBits++;

        if (txHuf.outBits >= 8) { 
            txHuf.AuxBuff[txHuf.numBytes++] = (unsigned char)txHuf.outByte;
            txHuf.outByte  = txHuf.outBits = 0;
        }

        txHuf.curInd = txHuf.curNode[txHuf.curInd].next[bit];
        if ((huf = txHuf.curNode[txHuf.curInd].token) >= 0) { 
            if (huf == EOL) { 
                txHuf.EOLfound = 1;
                len = txHuf.numBytes;
                if (len > 216) { 
                    HUFenq1 (0x00);
                    HUFenq1 (len & 0x0ff);
                    HUFenq1 (len >> 8   );
                } else {
                    HUFenq1 (len);
                }
                HUFenq (txHuf.AuxBuff, len);
            } else { 
                txHuf.run1 += huf;
                if (!txHuf.white && huf) {
                    txHuf.EmptyLine = 0;
                }
                if (huf < TCLIM) { 
                    txHuf.white = !txHuf.white;
                    txHuf.run0  = txHuf.run1;
                }
                txHuf.curNode = txHuf.white ? WhiteHUFS : BlackHUFS;
                txHuf.curInd  = 0;
            }
        }
    }
    /*---------------------------------------------------------------------*/
    if (bit) {
        if (txHuf.EOLcount0 >= 11) {
            if (!txHuf.lineValid) { 
                txHuf.lineValid = 1;
                txHuf.EOP = 0;
                PutPageHeader(0);
            }
            HUFMANinit();
        }
        txHuf.EOLcount0 = 0;
    } else {
        txHuf.EOLcount0++;
    }
}

/*===========================================================================*\
\*===========================================================================*/

