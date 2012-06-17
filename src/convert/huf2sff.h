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

#ifndef HUF2SFF_H
#define HUF2SFF_H

#include "aTypes.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define QUEUESIZE                   4096
#define MAX_HUF_SFF_BUFFER_LEN      8192

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

typedef struct inode { 
    int token;   /* Token number for this node.     */
    int next[2]; /* Pointers to next nodes in tree. */
} INODE;

/*===========================================================================*\
\*===========================================================================*/

class CSFilter_Huf2Sff {
public:
    CSFilter_Huf2Sff ();
    virtual ~CSFilter_Huf2Sff (void);

    tBool Open (tUInt vRes, tUInt DataBitOrder); 
    void Close (void);
    
    void PutPageHeader (tBool lastPage);
    void PutChar (tUChar ch);
    tUInt Get (tUChar *buffer, tUInt maxLen);

    void  SetVRes (tUInt vRes);
    tUInt CurSize (void);
    tBool IsEOP (void);

private:
    void HUFMANinit (void);
    int HUFenq (unsigned char *buff, int len);
    int HUFenq1 (int val);
    void HUFtstBit (int bit);

    struct DATA_QUEUE {   
        unsigned char Qdata[QUEUESIZE];
        int Qread;    /* Read  Position   */
        int Qwrite;   /* Write Position   */
    };
    struct TXHUF {
        int        inESCAPE;
        int        EOP;
        int        lineValid;
        DATA_QUEUE dq;
        int        curInd;
        int        EOLfound, EOLcount0;
        int        white;
        int        run0, run1;
        int        EmptyLine;
        int        numBytes;
        int        outByte, outBits;
        int        default_v_res;
        INODE     *curNode;
        tUChar     AuxBuff[MAX_HUF_SFF_BUFFER_LEN];
    };
    
    TXHUF txHuf;
};

/*===========================================================================*\
\*===========================================================================*/

inline void  CSFilter_Huf2Sff::SetVRes (tUInt vRes) {
    txHuf.default_v_res = vRes;
}

/*===========================================================================*\
\*===========================================================================*/

#endif

