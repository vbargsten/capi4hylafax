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

#ifndef _osmem_h
#define _osmem_h

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/
#include "aString.h"

#define ISALIGNED(p)   (0 == (PtrToUlong (p) & 3))

/*===========================================================================*\
\*===========================================================================*/

void*   getPtr(tUByte *m);
tUInt64 get64 (tUByte *m);
tUInt32 get32 (tUByte *m);
tUInt16 get16 (tUByte *m);
tUByte  get8  (tUByte *m);

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

tUInt64 getNBO64 (tUByte *m);       // Network Byte Order
tUInt32 getNBO32 (tUByte *m);
tUInt16 getNBO16 (tUByte *m);

tUInt64 getLE64 (tUByte *m);        // Little Endian
tUInt32 getLE32 (tUByte *m);
tUInt16 getLE16 (tUByte *m);

tUInt64 getBE64 (tUByte *m);        // Big Endian
tUInt32 getBE32 (tUByte *m);
tUInt16 getBE16 (tUByte *m);

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

tUInt64 getNBO64 (tUInt64 v);
tUInt32 getNBO32 (tUInt32 v);
tUInt16 getNBO16 (tUInt16 v);

tUInt64 getLE64 (tUInt64 v);
tUInt32 getLE32 (tUInt32 v);
tUInt16 getLE16 (tUInt16 v);

tUInt64 getBE64 (tUInt64 v);
tUInt32 getBE32 (tUInt32 v);
tUInt16 getBE16 (tUInt16 v);


/*===========================================================================*\
\*===========================================================================*/

void setPtr (tUByte *m, void   *d);
void set64  (tUByte *m, tUInt64 d);
void set32  (tUByte *m, tUInt32 d);
void set16  (tUByte *m, tUInt16 d);
void set8   (tUByte *m, tUByte  d);

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void setNBO64 (tUByte *m, tUInt64 d);
void setNBO32 (tUByte *m, tUInt32 d);
void setNBO16 (tUByte *m, tUInt16 d);

void setLE64  (tUByte *m, tUInt64 d);
void setLE32  (tUByte *m, tUInt32 d);
void setLE16  (tUByte *m, tUInt16 d);

void setBE64  (tUByte *m, tUInt64 d);
void setBE32  (tUByte *m, tUInt32 d);
void setBE16  (tUByte *m, tUInt16 d);


/*===========================================================================*\
\*===========================================================================*/

inline void *getPtr (tUByte *m) {
    #ifdef C_PLATFORM_64
        return (void *)get64 (m);
    #else
        return (void *)get32 (m);
    #endif
}

inline tUInt64 get64 (tUByte *m) {
    tUInt64 value;
    s_memcpy (&value, m, sizeof (value));
    return value;
}

inline tUInt32 get32 (tUByte *m) {
    tUInt32 value;
    s_memcpy (&value, m, sizeof (value));
    return value;
}

inline tUInt16 get16 (tUByte *m) {
    tUInt16 value;
    s_memcpy (&value, m, sizeof (value));
    return value;
}

inline tUByte get8 (tUByte *m) {
    return *m;
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt64 getNBO64 (tUByte *m) {
    return getBE64 (m);
}

inline tUInt32 getNBO32 (tUByte *m) {
    return getBE32 (m);
}

inline tUInt16 getNBO16 (tUByte *m) {
    return getBE16 (m);
}


/*---------------------------------------------------------------------------*\
    Implementation for little endian computers
\*---------------------------------------------------------------------------*/

inline tUInt64 getLE64 (tUByte *m) {
    return (((tUInt64)getLE32 (m + sizeof (tUInt32))) << 32) | (tUInt32)getLE32 (m);
}

inline tUInt32 getLE32 (tUByte *m) {
    return (((tUInt32)getLE16 (m + sizeof (tUInt16))) << 16) | (tUInt32)getLE16 (m);
}

inline tUInt16 getLE16 (tUByte *m) {
    return (tUInt16)((((tUInt16)m[1]) << 8) | (tUInt16)m[0]);
}

inline tUInt64 getBE64 (tUByte *m) {
    return (((tUInt64)getBE32 (m)) << 32) | (tUInt32)getBE32 (m + sizeof (tUInt32));
}

inline tUInt32 getBE32 (tUByte *m) {
    return (((tUInt32)getBE16 (m)) << 16) | (tUInt32)getBE16 (m + sizeof (tUInt16));
}

inline tUInt16 getBE16 (tUByte *m) {
    return (tUInt16)((((tUInt16)m[0]) << 8) | (tUInt16)m[1]);
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUInt64 getNBO64 (tUInt64 v) {
    return getNBO64 ((tUByte *)&v);
}

inline tUInt32 getNBO32 (tUInt32 v) {
    return getNBO32 ((tUByte *)&v);
}

inline tUInt16 getNBO16 (tUInt16 v) {
    return getNBO16 ((tUByte *)&v);
}

inline tUInt64 getLE64 (tUInt64 v) {
    return getLE64 ((tUByte *)&v);
}

inline tUInt32 getLE32 (tUInt32 v) {
    return getLE32 ((tUByte *)&v);
}

inline tUInt16 getLE16 (tUInt16 v) {
    return getLE16 ((tUByte *)&v);
}

inline tUInt64 getBE64 (tUInt64 v) {
    return getBE64 ((tUByte *)&v);
}

inline tUInt32 getBE32 (tUInt32 v) {
    return getBE32 ((tUByte *)&v);
}

inline tUInt16 getBE16 (tUInt16 v) {
    return getBE16 ((tUByte *)&v);
}


/*===========================================================================*\
\*===========================================================================*/

inline void setPtr (tUByte *m, void *d)  {
    //assert (m != 0);
    #ifdef C_PLATFORM_64
        set64 (m, (tUInt64)d);
    #else
        set32 (m, (tUInt32)d);
    #endif
}

inline void set64 (tUByte *m, tUInt64 d)  {
    //assert (m != 0);
    s_memcpy (m, &d, sizeof (tUInt64));
}

inline void set32 (tUByte *m, tUInt32 d) {
    //assert (m != 0);
    s_memcpy (m, &d, sizeof (tUInt32));
}

inline void set16 (tUByte *m, tUInt16 d) {
    //assert (m != 0);
    s_memcpy (m, &d, sizeof (tUInt16));
}

inline void set8 (tUByte *m, tUByte d) {
    //assert (m != 0);
    *m = d;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void setNBO64 (tUByte *m, tUInt64 d) {
    setBE64 (m, d);
}

inline void setNBO32 (tUByte *m, tUInt32 d) {
    setBE32 (m, d);
}

inline void setNBO16 (tUByte *m, tUInt16 d) {
    setBE16 (m, d);
}


/*---------------------------------------------------------------------------*\
    Implementation for little endian computers
\*---------------------------------------------------------------------------*/

inline void setLE64  (tUByte *m, tUInt64 d) {
    setLE32 (m, (tUInt32)d);
    setLE32 (m + sizeof (tUInt32), (tUInt32)(d >> 32));
}

inline void setLE32  (tUByte *m, tUInt32 d) {
    setLE16 (m, (tUInt16)d);
    setLE16 (m + sizeof (tUInt16), (tUInt16)(d >> 16));
}

inline void setLE16  (tUByte *m, tUInt16 d) {
    m[0] = (tUByte)(d & 0xFF);
    m[1] = (tUByte)(d >> 8);
}

inline void setBE64  (tUByte *m, tUInt64 d) {
    setBE32 (m, (tUInt32)(d >> 16));
    setBE32 (m + sizeof (tUInt32), (tUInt32)d);
}

inline void setBE32  (tUByte *m, tUInt32 d) {
    setBE16 (m, (tUInt16)(d >> 16));
    setBE16 (m + sizeof (tUInt16), (tUInt16)d);
}

inline void setBE16  (tUByte *m, tUInt16 d) {
    m[0] = (tUByte)(d >> 8);
    m[1] = (tUByte)(d & 0xFF);
}


/*===========================================================================*\
\*===========================================================================*/

#endif
