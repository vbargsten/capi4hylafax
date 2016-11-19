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

#ifndef _ASTDLIB_H_
#define _ASTDLIB_H_

#include "aTypes.h"

/*===========================================================================*\
\*===========================================================================*/

tUInt a_max (tUInt a, tUInt b);
tUInt a_min (tUInt a, tUInt b);

/*===========================================================================*\
\*===========================================================================*/

tSign a_getsign (tSInt value);

tCompare a_compare (tUInt value1, tUInt value2);

tUByte a_count1bits (tUInt value);

/*===========================================================================*\
\*===========================================================================*/

tSInt a_atol (char *string);

tSInt a_strtoln (const char *nptr, char **endptr, tSize maxlength, tSInt base);
tSInt a_strtoln (tChar *nptr, tChar **endptr, tSize maxlength, tSInt base);
tSInt a_strtoln (tWiChar *nptr, tWiChar **endptr, tSize maxlength, tSInt base);

tSInt a_strtol (const char *nptr, char **endptr, tSInt base);
tSInt a_strtol (tChar *nptr, tChar **endptr, tSInt base);
tSInt a_strtol (tWiChar *nptr, tWiChar **endptr, tSInt base);

tUInt a_ltoan (tSInt value, char *string, tSize maxlen, tSInt radix);

char *a_strupr (char *string);

/*===========================================================================*\
\*===========================================================================*/

tUInt a_pointer2string (void *pointer, char *string);
tUInt a_pointer2string (void *pointer, tChar *string);
tUInt a_pointer2string (void *pointer, tWiChar *string);


/*===========================================================================*\
    inline functions
\*===========================================================================*/

inline tUInt a_max (tUInt a, tUInt b) {
    return (a > b) ? a : b;
}

inline tUInt a_min (tUInt a, tUInt b) {
    return (a < b) ? a : b;
}

inline tSign a_getsign (tSInt value) {
    return (!value) ? vZero : (value > 0) ? vPlus : vMinus;
}

inline tCompare a_compare (tUInt value1, tUInt value2) {
    return (value1 == value2) ? vEqual : (value1 > value2) ? vHigher : vLower;
}

inline tSInt a_strtoln (const char *nptr, char **endptr, tSize maxlength, tSInt base) {
    return a_strtoln ((tChar *)nptr, (tChar **)endptr, maxlength, base);
}

inline tSInt a_strtol (const char *nptr, char **endptr, tSInt base) {
    return a_strtoln (nptr, endptr, MAXVAL_tUInt, base);
}

inline tSInt a_strtol (tChar *nptr, tChar **endptr, tSInt base) {
    return a_strtoln (nptr, endptr, MAXVAL_tUInt, base);
}

inline tSInt a_strtol (tWiChar *nptr, tWiChar **endptr, tSInt base) {
    return a_strtoln (nptr, endptr, MAXVAL_tUInt, base);
}

inline tUInt a_pointer2string (void *pointer, char *string) {
    return a_pointer2string (pointer, (tChar *)string);
}

/*===========================================================================*\
\*===========================================================================*/

#endif
