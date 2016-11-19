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

#ifndef _AFLAGS_H_
#define _AFLAGS_H_

#include "aTypes.h"

/*===========================================================================*\
\*===========================================================================*/

inline tUInt FlagToBits (tUByte Flag) {
    return (1 << Flag);
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void SetBitsAll (tUInt *pValue, tUInt Bits) {
    *pValue |= Bits;
}

inline void SetOneFlag (tUInt *pValue, tUByte Flag) {
    SetBitsAll (pValue, FlagToBits (Flag));
}

inline void ClearBitsAll (tUInt *pValue, tUInt Bits) {
    *pValue &= ~Bits;
}

inline void ClearOneFlag (tUInt *pValue, tUByte Flag) {
    ClearBitsAll (pValue, FlagToBits (Flag));
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt SetBitsAll (tUInt Value, tUInt Bits) {
    return (Value | Bits);
}

inline tUInt SetOneFlag (tUInt Value, tUByte Flag) {
    return SetBitsAll (Value, FlagToBits (Flag));
}

inline tUInt ClearBitsAll (tUInt Value, tUInt Bits) {
    return (Value & ~Bits);
}

inline tUInt ClearOneFlag (tUInt Value, tUByte Flag) {
    return ClearBitsAll (Value, FlagToBits (Flag));
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CheckBitsAll (tUInt Value, tUInt Bits) {
    return (Value & Bits) == Bits;
}

inline tBool CheckBitsOne (tUInt Value, tUInt Bits) {
    return (Value & Bits) != 0;
}

inline tBool CheckBitsClrAll (tUInt Value, tUInt Bits) {
    return (Value & Bits) == 0;
}

inline tBool CheckBitsClrOne (tUInt Value, tUInt Bits) {
    return (Value & Bits) != Bits;
}

inline tBool CheckBitsSetAndClr (tUInt Value, tUInt SetBits, tUInt ClrBits) {
    return (Value & (SetBits | ClrBits)) == SetBits;
}

inline tBool CheckOneFlag (tUInt Value, tUByte Flag) {
    return ((Value >> Flag) & 1) != 0;
}


/*===========================================================================*\
\*===========================================================================*/

#endif
