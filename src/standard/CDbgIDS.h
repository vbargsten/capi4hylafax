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

#ifndef _CDBGIDS_H_
#define _CDBGIDS_H_

#include <stdarg.h>

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define DBGDYNSTRING_STARTMAXLENGTH     200

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class CDbgIDynString {
public:
    CDbgIDynString (void);
    ~CDbgIDynString (void);

    int AddTo (unsigned tabsize, const char *format, ...);
    int vAddTo (unsigned tabsize, const char *format, va_list param);
    char *GetLastEntry (void);
    char *GetAll (void);
    void Delete (void);
    unsigned IsEmpty (void);

private:
    unsigned maxlength;
    unsigned length;
    unsigned lastlength;
    char *buffer;
    char startbuffer[DBGDYNSTRING_STARTMAXLENGTH];
};

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline unsigned CDbgIDynString::IsEmpty (void) {
    return (length == 0);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#endif
