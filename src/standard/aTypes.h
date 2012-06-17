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

#ifndef _ATYPES_H_
#define _ATYPES_H_

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#if !defined C_PLATFORM_64 && defined SIZEOF_VOID_P && (SIZEOF_VOID_P == 8)
#define C_PLATFORM_64
#endif

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#ifndef ASTRING_DONT_USE_WCHAR
    #include <wchar.h>
#endif

#if defined HAVE_STDINT_H && HAVE_STDINT_H > 0
    #include <stdint.h>
#else

typedef signed char        int8_t;
typedef short int          int16_t;
typedef int                int32_t;

typedef unsigned char      uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int       uint32_t;

#ifdef __linux__
    #if __WORDSIZE == 64
        typedef long int int64_t;
        typedef unsigned long int uint64_t;
    #else
        typedef long long int int64_t;
        typedef unsigned long long int uint64_t;
    #endif
#else
    typedef __int64 int64_t;
    typedef unsigned __int64 uint64_t;
#endif // __linux__

#endif // __STDC_VERSION__

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#ifdef __cplusplus

typedef bool    tBool;
#define vFalse  false
#define vTrue   true

#else

typedef enum {
    vFalse = 0,
    vTrue  = 1
} tBool;

#endif

typedef enum {
    vMinus = -1,
    vZero  = 0,
    vPlus  = 1
} tSign;

typedef enum {
    vLower  = -1,
    vEqual  = 0,
    vHigher = 1
} tCompare;

typedef enum {
    vError  = -2,
    vCancel = -1,
    vNo     = 0,
    vYes    = 1
} tResult;

typedef uint8_t     tUChar;
typedef int8_t      tSChar;
typedef tUChar      tChar;

#ifndef ASTRING_DONT_USE_WCHAR
typedef wchar_t     tWiChar;
#else
typedef uint16_t    tWiChar;
#endif
typedef tWiChar     tUWiChar;

typedef uint8_t     tUByte;
typedef int8_t      tSByte;

typedef uint16_t    tUInt16;
typedef int16_t     tSInt16;

typedef uint32_t    tUInt32;
typedef int32_t     tSInt32;

typedef uint64_t    tUInt64;
typedef int64_t     tSInt64;

typedef tUInt16     tUShort;
typedef tSInt16     tSShort;

typedef tUInt32     tUInt;      // default type
typedef tSInt32     tSInt;      // default signed type

// Integer, with size of a pointer
#ifdef C_PLATFORM_64
typedef tUInt64     tULong;
typedef tSInt64     tSLong;
#else
typedef tUInt32     tULong;
typedef tSInt32     tSLong;
#endif

typedef tULong      tSize;
typedef tSLong      tOffset;
typedef void      **tHandle;

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define vIllegalHandle      0

#define MAXVAL_tChar        ((tChar) -1)
#define MAXVAL_tUChar       ((tUChar) -1)
#define MAXVAL_tSChar       ((tSChar) ((MAXVAL_tUChar - 1) / 2))
#define MINVAL_tSChar       ((tSChar) - MAXVAL_tSChar - 1)

#define MAXVAL_tWiChar      ((tWiChar) -1)
#define MAXVAL_tUWiChar     ((tUWiChar) -1)

#define MAXVAL_tUByte       ((tUByte) -1)
#define MAXVAL_tSByte       ((tSByte) ((MAXVAL_tUByte - 1) / 2))
#define MINVAL_tSByte       ((tSByte) - MAXVAL_tSByte - 1)

#define MAXVAL_tUInt16      ((tUInt16) -1)
#define MAXVAL_tSInt16      ((tSInt16) ((MAXVAL_tUInt16 - 1) / 2))
#define MINVAL_tSInt16      ((tSShort) - MAXVAL_tSInt16 - 1)

#define MAXVAL_tUInt32      ((tUInt32) -1)
#define MAXVAL_tSInt32      ((tSInt32) ((MAXVAL_tUInt32 - 1) / 2))
#define MINVAL_tSInt32      ((tSInt32) - MAXVAL_tSInt32 - 1)

#define MAXVAL_tUInt64      ((tUInt64) -1)
#define MAXVAL_tSInt64      ((tSInt64) ((MAXVAL_tUInt64 - 1) / 2))
#define MINVAL_tSInt64      ((tSInt64) - MAXVAL_tSInt64 - 1)

#define MAXVAL_tUShort      ((tUShort) -1)
#define MAXVAL_tSShort      ((tSShort) ((MAXVAL_tUShort - 1) / 2))
#define MINVAL_tSShort      ((tSShort) - MAXVAL_tSShort - 1)

#define MAXVAL_tUInt        ((tUInt) -1)
#define MAXVAL_tSInt        ((tSInt) ((MAXVAL_tUInt - 1) / 2))
#define MINVAL_tSInt        ((tSInt) - MAXVAL_tSInt - 1)

#define MAXVAL_tULong       ((tULong) -1)
#define MAXVAL_tSLong       ((tSLong) ((MAXVAL_tULong - 1) / 2))
#define MINVAL_tSLong       ((tSLong) - MAXVAL_tSLong - 1)

#define MAXVAL_tSize        ((tSize) -1)
#define MAXVAL_tOffset      ((tOffset) ((MAXVAL_tSize - 1) / 2))
#define MINVAL_tOffset      ((tOffset) - MAXVAL_tOffset - 1)

#endif
