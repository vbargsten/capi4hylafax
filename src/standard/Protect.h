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

#ifndef _PROTECT_H_
#define _PROTECT_H_

#include "aTypes.h"

/*===========================================================================*\
\*===========================================================================*/
#ifdef DONT_USE_PROTECT_FEATURE

#define Protect_Define(name)
#define Protect_DefineInit(name)

#define Protect_InitHandle(p)
#define Protect_IsCreated(p)    vTrue

#define Protect_Create(p)       vTrue
#define Protect_Destroy(p)

#define Protect_BeginRead(p)
#define Protect_EndRead(p)

#define Protect_BeginWrite(p)
#define Protect_EndWrite(p)


/*===========================================================================*\
\*===========================================================================*/
#else // DONT_USE_PROTECT_FEATURE

typedef tHandle thProtect;

#define Protect_Define(name)        thProtect name
#define Protect_DefineInit(name)    thProtect name = vIllegalHandle

void  Protect_InitHandle (thProtect *pProtect);
tBool Protect_IsCreated  (thProtect Protect);

tBool Protect_Create     (thProtect *pProtect);
void  Protect_Destroy    (thProtect *pProtect);

void  Protect_BeginRead  (thProtect protect);
void  Protect_EndRead    (thProtect protect);

void  Protect_BeginWrite (thProtect protect);
void  Protect_EndWrite   (thProtect protect);


/*===========================================================================*\
\*===========================================================================*/

inline void Protect_InitHandle (thProtect *pProtect) {
    *pProtect = vIllegalHandle;
}

inline tBool Protect_IsCreated (thProtect Protect) {
    return (Protect != vIllegalHandle);
}

/*===========================================================================*\
\*===========================================================================*/

#endif // DONT_USE_PROTECT_FEATURE

#endif
