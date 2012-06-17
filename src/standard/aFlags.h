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

inline tUInt32 FlagToBits32 (tUByte Flag) {
    return (((tUInt32)1) << Flag);
}

inline tUInt64 FlagToBits64 (tUByte Flag) {
    #ifdef USE_64TO32BIT_DIVISION
        if (Flag >= 32) {
            tUInt64 value = 0;
            *(((tUInt32 *)&value) + 1) = FlagToBits32 (Flag - 32);
            return value;
        } else {
            return FlagToBits32 (Flag);
        }
    #else
        return (((tUInt64)1) << Flag);
    #endif
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void SetBitsAll (tUInt32 *pValue, tUInt32 Bits) {
    *pValue |= Bits;
}

inline void SetBitsAll (tUInt64 *pValue, tUInt64 Bits) {
    *pValue |= Bits;
}

inline void SetOneFlag (tUInt32 *pValue, tUByte Flag) {
    SetBitsAll (pValue, FlagToBits32 (Flag));
}

inline void SetOneFlag (tUInt64 *pValue, tUByte Flag) {
    SetBitsAll (pValue, FlagToBits64 (Flag));
}

inline void ClearBitsAll (tUInt32 *pValue, tUInt32 Bits) {
    *pValue &= ~Bits;
}

inline void ClearBitsAll (tUInt64 *pValue, tUInt64 Bits) {
    *pValue &= ~Bits;
}

inline void ClearOneFlag (tUInt32 *pValue, tUByte Flag) {
    ClearBitsAll (pValue, FlagToBits32 (Flag));
}

inline void ClearOneFlag (tUInt64 *pValue, tUByte Flag) {
    ClearBitsAll (pValue, FlagToBits64 (Flag));
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt32 SetBitsAll (tUInt32 Value, tUInt32 Bits) {
    return (Value | Bits);
}

inline tUInt64 SetBitsAll (tUInt64 Value, tUInt64 Bits) {
    return (Value | Bits);
}

inline tUInt32 SetOneFlag (tUInt32 Value, tUByte Flag) {
    return SetBitsAll (Value, FlagToBits32 (Flag));
}

inline tUInt64 SetOneFlag (tUInt64 Value, tUByte Flag) {
    return SetBitsAll (Value, FlagToBits64 (Flag));
}

inline tUInt32 ClearBitsAll (tUInt32 Value, tUInt32 Bits) {
    return (Value & ~Bits);
}

inline tUInt64 ClearBitsAll (tUInt64 Value, tUInt64 Bits) {
    return (Value & ~Bits);
}

inline tUInt32 ClearOneFlag (tUInt32 Value, tUByte Flag) {
    return ClearBitsAll (Value, FlagToBits32 (Flag));
}

inline tUInt64 ClearOneFlag (tUInt64 Value, tUByte Flag) {
    return ClearBitsAll (Value, FlagToBits64 (Flag));
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
