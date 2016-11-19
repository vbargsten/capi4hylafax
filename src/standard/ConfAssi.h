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

#ifndef _CONFASSI_H_
#define _CONFASSI_H_

#include "ConfPars.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

enum ConfigAssignment_Types {           // Data-Type
    ConfigAssignment_String,            // CDynamicString*
    ConfigAssignment_StringList,        // CMultiString*
    ConfigAssignment_UInt               // tUInt*
};

struct ConfigAssignment_Item {
    char                  *Name;
    ConfigAssignment_Types Type;
    void                  *Data;
};

/*===========================================================================*\
\*===========================================================================*/

void ConfigValuesAssignment (CPointerList *ValueList, ConfigAssignment_Item *ItemList);

tUInt ConfigValuesTestName (CPointerList *ValueList, ConfigAssignment_Item *searchItem);
// return:  0 = Name not found
//          1 = Name found, but Data is different
//          2 = Name found and Data is identical

/*===========================================================================*\
\*===========================================================================*/

#endif

