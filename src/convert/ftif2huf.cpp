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

#include "ftif2huf.h"
#include "aFlags.h"
#include "osImport.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define DLE                             0x10

#define FAX_RESOLUTION_LOW              98
#define FAX_RESOLUTION_HIGH             196

/*===========================================================================*\
\*===========================================================================*/

CSFilter_fTif2Huf::CSFilter_fTif2Huf (void) {
    dhead ("CSFilter_fTif2Huf - Constructor", DCON_TifFilter);
    hTiff           = 0;
    stripBuf        = 0;
    YResolution     = 0;
    Close(); // this close initialize the other members
}

/*===========================================================================*\
\*===========================================================================*/

CSFilter_fTif2Huf::~CSFilter_fTif2Huf (void) {
    dhead ("CSFilter_fTif2Huf - Destructor", DCON_TifFilter);
    Close();
}

/*===========================================================================*\
\*===========================================================================*/

tBool CSFilter_fTif2Huf::Open (tUChar *TifFileName) {
    dhead ("CSFilter_fTif2Huf::Open", DCON_TifFilter);
    dassert (TifFileName != 0);
    dassert (hTiff == 0);
    hTiff = TIFFOpen ((char *)TifFileName, "r");
    if (hTiff) {
        SetPageInfo();
        tSInfo info;
        GetStrip (0, 0, &info);
        dwarning (info == i_Okay);
        return vTrue;
    }
    return vFalse;
}

/*===========================================================================*\
\*===========================================================================*/

void CSFilter_fTif2Huf::Close (void) {
    dhead ("CSFilter_fTif2Huf::Close", DCON_TifFilter);
    if (hTiff) {
        TIFFClose (hTiff);
        hTiff = 0;
    }
    if (stripBuf) {
        delete [] stripBuf;
        stripBuf = 0;
    }
    BitRevField     = 0;
    StripCountField = 0;
    YResolution     = 0;
    nStrips         = 0;
    curStrip        = -1; // GetToPageEnd add 1 before it first uses curStrip so we start correctly with 0
    stripBufMaxLen  = 0;
    stripBufLen     = 0;
    stripBufOff     = 0;
}

/*===========================================================================*\
\*===========================================================================*/

tBool CSFilter_fTif2Huf::CheckForCorrectFaxFile (void) {
    dhead ("CSFilter_fTif2Huf::CheckForCorrectFaxFile", DCON_TifFilter);
    // At the moment only b/w G3 Fax is allowed
    tUInt32 lValue;
    tUInt16 sValue = 0;
    if ((TIFFGetField (hTiff, TIFFTAG_BITSPERSAMPLE, &sValue) == 1) && (sValue == 1)) {
        sValue = 0;
        if ((TIFFGetField (hTiff, TIFFTAG_SAMPLESPERPIXEL, &sValue) == 1) && (sValue == 1)) {
            sValue = 0;
            if ((TIFFGetField (hTiff, TIFFTAG_COMPRESSION, &sValue) == 1) && (sValue == COMPRESSION_CCITTFAX3)) {
                lValue = 0;
                if ((TIFFGetField (hTiff, TIFFTAG_IMAGEWIDTH, &lValue) == 1) && (lValue == 1728)) {
                    lValue = 0;
                    if (TIFFGetField (hTiff, TIFFTAG_GROUP4OPTIONS, &lValue) == 0) {
                        lValue = 0;
                        if (  (TIFFGetField (hTiff, TIFFTAG_GROUP3OPTIONS, &lValue) == 1)
                           && (CheckBitsClrAll (lValue, 0x3)) == vTrue) {
                            RETURN ('x', vTrue);
                        }
                    }
                }
            }
        }
    }
    RETURN ('x', vFalse);
}


/*===========================================================================*\
\*===========================================================================*/

tUInt CSFilter_fTif2Huf::GetToPageEnd (tUByte *buffer, tUInt maxLen, tSInfo *pInfo) {
    dhead ("CSFilter_fTif2Huf::GetToPageEnd", DCON_TifFilter);
    dassert (maxLen > 0);
    dassert (pInfo != 0);
    tSInfo lInfo = i_Pending; // initialise with an (nearly) unused value
    tUInt  len   = 0;

    do {
        dprint ("stripBufLen=%x, curStrip=%x, nStrips=%x, last=%x ", stripBufLen, curStrip, nStrips,
                TIFFLastDirectory (hTiff));
        if (stripBufLen <= stripBufOff) {
            stripBufLen = 0;
            stripBufOff = 0;
            do {
                curStrip++;
                if ((tUInt)curStrip >= nStrips) {
                    // end of page reached
                    if (TIFFLastDirectory (hTiff) || !TIFFReadDirectory (hTiff)) {
                        lInfo = iWrn_Filter_LastPageEnd;
                    } else {
                        lInfo = iWrn_Filter_PageEnd;
                        SetPageInfo();
                    }
                    break;
                }
                stripBufLen = StripCountField[curStrip];
                if (stripBufMaxLen < stripBufLen) {
                    // buffer to small => resize
                    tUChar *tmpBuf = new tUChar[stripBufLen];
                    if (!tmpBuf) {
                        stripBufLen = 0;
                        lInfo       = iErr_OutOfMemory;
                        break;
                    }
                    delete [] stripBuf;
                    stripBuf       = tmpBuf;
                    stripBufMaxLen = stripBufLen;
                }
            } while (TIFFReadRawStrip (hTiff, curStrip, stripBuf, stripBufLen) < 0);
            // now we have a new strip or a stop condition
        }
        for (; (stripBufOff < stripBufLen); len++, stripBufOff++) {
            if (len >= maxLen) {
                lInfo = i_Okay;
                break;
            }
            buffer[len] = BitRevField[stripBuf[stripBufOff]];
            //if (buffer[len] == DLE) {
            //    len++;
            //    if (len < maxLen) {
            //        buffer[len] = DLE;
            //    } else {
            //        len  -= 2;
            //        lInfo = i_Okay;
            //        break;
            //    }
            //}
        }
    } while (lInfo == i_Pending);
    // lInfo changed from the unused value to a return condition
    *pInfo = lInfo;
    return len;
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CSFilter_fTif2Huf::GetStrip (tUByte *buffer, tUInt maxLen, tSInfo *pInfo) {
    dhead ("CSFilter_fTif2Huf::GetStrip", DCON_TifFilter);
    dassert (pInfo != 0);
    tSInfo lInfo = i_Pending; // initialise with a (nearly) unused value
    tUInt  len   = 0;

    do {
        dprint ("stripBufLen=%x, curStrip=%x, nStrips=%x, last=%x ", stripBufLen, curStrip, nStrips,
                TIFFLastDirectory (hTiff));
        if (stripBufLen <= stripBufOff) {
            stripBufLen = 0;
            stripBufOff = 0;
            do {
                curStrip++;
                if ((tUInt)curStrip >= nStrips) {
                    // end of page reached
                    if (TIFFLastDirectory (hTiff) || !TIFFReadDirectory (hTiff)) {
                        lInfo = iWrn_Filter_LastPageEnd;
                    } else {
                        lInfo = iWrn_Filter_PageEnd;
                        SetPageInfo();
                    }
                    break;
                }
                stripBufLen = StripCountField[curStrip];
                if (stripBufMaxLen < stripBufLen) {
                    // buffer to small => resize
                    tUChar *tmpBuf = new tUChar[stripBufLen];
                    if (!tmpBuf) {
                        stripBufLen = 0;
                        lInfo       = iErr_OutOfMemory;
                        break;
                    }
                    delete [] stripBuf;
                    stripBuf       = tmpBuf;
                    stripBufMaxLen = stripBufLen;
                }
            } while (TIFFReadRawStrip (hTiff, curStrip, stripBuf, stripBufLen) < 0);
            // now we have a new strip or a stop condition
        }
        dprint ("l=%x,o=%x ", stripBufLen, stripBufOff);
        if (stripBufOff < stripBufLen) {
            lInfo = i_Okay;
            //tUInt countDLE = 0;
            //for (tUInt i = stripBufOff; i < stripBufLen; i++) {
            //    if (BitRevField[stripBuf[i]] == DLE) {
            //        countDLE++;
            //    }
            //}
            if (maxLen >= (stripBufLen - stripBufOff)) { // + countDLE)) {
                for (; (stripBufOff < stripBufLen); len++, stripBufOff++) {
                    buffer[len] = BitRevField[stripBuf[stripBufOff]];
                    //if (buffer[len] == DLE) {
                    //    len++;
                    //    buffer[len] = DLE;
                    //}
                }
            }
        }
    } while (lInfo == i_Pending);
    // lInfo changed from the unused value to a return condition
    *pInfo = lInfo;
    RETURN ('x', len);
}

/*===========================================================================*\
\*===========================================================================*/

void CSFilter_fTif2Huf::SetPageInfo (void) {
    dhead ("CSFilter_fTif2Huf::SetPageInfo", DCON_TifFilter);
    dassert (hTiff != 0);
    float   intyres = 0.0;
    tUShort fillorder;
    nStrips  = TIFFNumberOfStrips (hTiff);
    curStrip = -1;  // GetToPageEnd add 1 before it first uses curStrip so we start correctly with 0
    TIFFGetFieldDefaulted (hTiff, TIFFTAG_FILLORDER, &fillorder);
    BitRevField = TIFFGetBitRevTable (fillorder != FILLORDER_LSB2MSB);
    TIFFGetField (hTiff, TIFFTAG_STRIPBYTECOUNTS, &StripCountField);
    TIFFGetField (hTiff, TIFFTAG_YRESOLUTION, &intyres);
    YResolution = (tUInt)intyres;
}

/*===========================================================================*\
\*===========================================================================*/

