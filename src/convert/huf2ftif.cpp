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

#include <unistd.h>
#include <time.h>
#include <string.h>
#include "huf2ftif.h"
#include "osImport.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define DLE_CHAR        0x10

/*===========================================================================*\
\*===========================================================================*/

CSFilter_Huf2fTif::CSFilter_Huf2fTif (void) {
    dhead ("CSFilter_Huf2fTif - Constructor", DCON_TifFilter);
    hTiff = 0;
}

/*===========================================================================*\
\*===========================================================================*/

CSFilter_Huf2fTif::~CSFilter_Huf2fTif (void) {
    dhead ("CSFilter_Huf2fTif - Destructor", DCON_TifFilter);
    Close(); 
}

/*===========================================================================*\
\*===========================================================================*/

tBool CSFilter_Huf2fTif::Open (int fdTifFile, tUChar *TifFileName) {
    dhead ("CSFilter_Huf2fTif::Open", DCON_TifFilter);
    dassert (TifFileName != 0);
    dassert (hTiff == 0);
    countPage = 1;
    hTiff     = TIFFFdOpen (fdTifFile, (char *)TifFileName, "w");
    if (hTiff) {
        return vTrue;
    } else  {
        close (fdTifFile);
    }
    return vFalse; 
}

/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Huf2fTif::Close (void) {
    dhead ("CSFilter_Huf2fTif::Close", DCON_TifFilter);
    if (hTiff) {
        TIFFClose (hTiff);
        hTiff = 0;
    }
}

/*===========================================================================*\
\*===========================================================================*/

tBool CSFilter_Huf2fTif::Put (tUChar *buffer, tUInt len) {
    dhead ("CSFilter_Huf2fTif::Put", Dbg_Level_Errors); //DCON_TifFilter);
    dparams ("%x,%x", buffer, len);
    dassert (buffer != 0);
    dwarning (len > 0);
    // Doppelte DLE´s entfernen
    //tUChar *lauf1 = buffer;
    //tUChar *lauf2 = buffer;
    //for (tUInt i = len; i > 0; i--, lauf1++, lauf2++) {
    //    *lauf1 = *lauf2;
    //    if ((*lauf2 == DLE_CHAR) && (i > 1)) {
    //        lauf2++;
    //        len--;
    //        i--;
    //    } 
    //}
    TIFFWriteRawStrip (hTiff, 0, buffer, len);
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

tBool CSFilter_Huf2fTif::PutPageHeader (tUInt yRes, tUInt xRes, tUInt length, tUInt width, tUInt recvparams,
                                        tUChar *description, tUChar *manufact, tUChar *model, 
                                        tUChar *hostname, tUChar *software) {
    dhead ("CSFilter_Huf2fTif::PutPageHeader", DCON_TifFilter);
    dparams ("%x,%x,%x,%x,%s,%s,%s,%s,%s", yRes, xRes, length, width, description, manufact, model, hostname,
             software);
    char dateTime[24];
    time_t curTime;
    time (&curTime);
    strftime (dateTime, sizeof (dateTime), "%Y:%m:%d %H:%M:%S", localtime (&curTime));
    dprint ("Time=%s ", dateTime);
    
    TIFFSetField (hTiff, TIFFTAG_BITSPERSAMPLE,        1);
    TIFFSetField (hTiff, TIFFTAG_SAMPLESPERPIXEL,      1);
    TIFFSetField (hTiff, TIFFTAG_COMPRESSION,          COMPRESSION_CCITTFAX3);
    TIFFSetField (hTiff, TIFFTAG_FILLORDER,            FILLORDER_LSB2MSB);
    TIFFSetField (hTiff, TIFFTAG_GROUP3OPTIONS,        GROUP3OPT_FILLBITS);
    TIFFSetField (hTiff, TIFFTAG_ORIENTATION,          ORIENTATION_TOPLEFT);
    TIFFSetField (hTiff, TIFFTAG_PHOTOMETRIC,          PHOTOMETRIC_MINISWHITE);
    TIFFSetField (hTiff, TIFFTAG_PLANARCONFIG,         PLANARCONFIG_CONTIG);
    TIFFSetField (hTiff, TIFFTAG_RESOLUTIONUNIT,       RESUNIT_INCH);
    TIFFSetField (hTiff, TIFFTAG_SUBFILETYPE,          FILETYPE_PAGE);

    TIFFSetField (hTiff, TIFFTAG_IMAGEWIDTH,	       width);
    TIFFSetField (hTiff, TIFFTAG_XRESOLUTION,	       (float)(1.0 * xRes));
    TIFFSetField (hTiff, TIFFTAG_YRESOLUTION,	       (float)(1.0 * yRes));
    TIFFSetField (hTiff, TIFFTAG_DATETIME,	       dateTime);
    TIFFSetField (hTiff, TIFFTAG_PAGENUMBER,           countPage, 0);
    //TIFFSetField (hTiff, TIFFTAG_FAXSUBADDRESS,      (const char*) sub);

    if (recvparams) {
        TIFFSetField (hTiff, TIFFTAG_FAXRECVPARAMS,    recvparams);
    }
    if (description && *description) {
        TIFFSetField (hTiff, TIFFTAG_IMAGEDESCRIPTION, description);
    }
    if (manufact && *manufact) {
        TIFFSetField (hTiff, TIFFTAG_MAKE,	       manufact);
    }
    if (model && *model) {
        TIFFSetField (hTiff, TIFFTAG_MODEL,	       model);
    }
    if (hostname && *hostname) {
        TIFFSetField (hTiff, TIFFTAG_HOSTCOMPUTER,     hostname);
    }
    if (software && *software) {
        TIFFSetField (hTiff, TIFFTAG_SOFTWARE,	       software);
    }
    if (length) {
        TIFFSetField (hTiff, TIFFTAG_IMAGELENGTH,      length);
        TIFFSetField (hTiff, TIFFTAG_ROWSPERSTRIP,     length);
    } else {
        TIFFSetField (hTiff, TIFFTAG_ROWSPERSTRIP,     (uint32)-1);
    }
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

void CSFilter_Huf2fTif::PutPageEnd (tBool /*LastPage*/, tUInt length, tUInt duration) {
    dhead ("CSFilter_Huf2fTif::PutPageEnd", DCON_TifFilter);
    //dparams ("%x,%x", LastPage, length);
    if (length) {
        TIFFSetField (hTiff, TIFFTAG_IMAGELENGTH, length);
    }
    if (duration) {
        TIFFSetField (hTiff, TIFFTAG_FAXRECVTIME, duration);
    }
    TIFFWriteDirectory (hTiff);
    countPage++;
}

/*===========================================================================*\
\*===========================================================================*/

