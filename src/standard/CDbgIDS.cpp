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

#include "CDbgIDS.h"
#include "aStrPrt.h"
#include "ExtFuncs.h"
#include "dbgSTD.h"
#include <string.h>

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

CDbgIDynString::CDbgIDynString (void) {
    //DbgIPrintStr ("DynString-Konstruktor ! ");
    length = lastlength = 0;
    maxlength = DBGDYNSTRING_STARTMAXLENGTH;
    memset (startbuffer, 0, DBGDYNSTRING_STARTMAXLENGTH);
    buffer = &startbuffer[0];

    //buffer = new char [DBGDYNSTRING_STARTMAXLENGTH];
    //DbgIPrintStr ("After new char ");
    //if (buffer) {
    //    maxlength = DBGDYNSTRING_STARTMAXLENGTH;
    //}
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

CDbgIDynString::~CDbgIDynString (void) {
    if (buffer != startbuffer) {
        delete [] buffer;
    }
    buffer = 0;
    maxlength = length = lastlength = 0;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

int CDbgIDynString::AddTo (tUInt tabsize, const char *format, ...) {
    va_list param;
    va_start (param, format);
    int fret = vAddTo (tabsize, format, param);
    va_end (param);
    return fret;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

int CDbgIDynString::vAddTo (tUInt tabsize, const char *format, va_list param) {
    if (!maxlength || !buffer) {
        return -1;
    }
    int newlen = a_vsnprintf_tab (buffer + length, maxlength - length, format, param, tabsize);
    //if (newlen <= 0) {
    //    char *newbuf = new char [maxlength * 2];
    //    if (newbuf) {
    //        strcpy (newbuf, buffer);
    //        if (buffer != startbuffer) {
    //            delete [] buffer;
    //        }
    //        buffer = newbuf;
    //        newlen = a_vsnprintf_tab (buffer + length, maxlength - length, format, param, tabsize);
    //    }
    //}
    if (newlen > 0) {
        lastlength = length;
        length += newlen;
    }
    return newlen;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

char *CDbgIDynString::GetLastEntry (void) {
    //DbgIAssert (maxlength != 0);
    //DbgIAssert (buffer != 0);
    //DbgIAssert (length < maxlength);
    //DbgIAssert (lastlength <= length);

    if (!length || (lastlength == length) || !maxlength || !buffer) {
        return 0;
    }
    buffer[length] = '\0';
    length = lastlength;
    return (buffer + length);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

char *CDbgIDynString::GetAll (void) {
    //DbgIAssert (maxlength != 0);
    //DbgIAssert (buffer != 0);
    //DbgIAssert (length < maxlength);
    //DbgIAssert (lastlength <= length);

    if (!length || !maxlength || !buffer) {
        return 0;
    }
    buffer[length] = '\0';
    length = lastlength = 0;
    return buffer;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDbgIDynString::Delete (void) {
    //DbgIAssert (maxlength != 0);
    //DbgIAssert (buffer != 0);
    //DbgIAssert (length < maxlength);
    //DbgIAssert (lastlength <= length);

    length = lastlength = 0;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/
