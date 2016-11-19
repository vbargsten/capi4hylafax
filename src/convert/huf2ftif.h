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

#ifndef _HUF2FTIF_H_
#define _HUF2FTIF_H_

#include <tiffio.h>
#include "aTypes.h"

/*===========================================================================*\
\*===========================================================================*/

class CSFilter_Huf2fTif {
public:
    CSFilter_Huf2fTif (void);
    virtual ~CSFilter_Huf2fTif (void);

    // info: the class ALWAYS closes the file descriptor fdTifFile
    tBool Open (int fdTifFile, tUChar *TifFileName);
    void Close (void);

    tBool Put (tUChar *buffer, tUInt len);
    tBool PutPageHeader (tUInt yRes, tUInt xRes, tUInt length, tUInt width, tUInt recvparams, 
                         tUChar *description, tUChar *manufact, tUChar *model, tUChar *hostname, 
                         tUChar *software);
    void  PutPageEnd (tBool LastPage, tUInt length = 0, tUInt duration = 0);

private:
    TIFF   *hTiff;
    tUShort countPage;
};

/*===========================================================================*\
\*===========================================================================*/
#endif

