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

#ifndef _MULTISTR_H_
#define _MULTISTR_H_

#include "CString.h"
#include "PtrList.h"

/*===========================================================================*\
\*===========================================================================*/

class COneMultiString : public CDynamicString, public CPointerListElement {
public:
    COneMultiString (void) {}
    COneMultiString (tFormatChar *string, tUInt len = 0) : CDynamicString (string, len) {}
    COneMultiString (const CConstString *pcstr) : CDynamicString (pcstr) {};
    
    COneMultiString *GetNext (void)             { return (COneMultiString *)CPointerListElement::GetNext(); }
    COneMultiString *GetPrev (void)             { return (COneMultiString *)CPointerListElement::GetPrev(); }
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class CMultiString : public CPointerList {
public:
    COneMultiString *GetFirst (void)            { return (COneMultiString *)CPointerList::GetFirst(); }
    COneMultiString *GetLast (void)             { return (COneMultiString *)CPointerList::GetLast();  }
};

/*===========================================================================*\
\*===========================================================================*/

#endif
