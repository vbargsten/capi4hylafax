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

#ifndef _ASTRPRT_H_
#define _ASTRPRT_H_

#include <stdarg.h>
#include "aTypes.h"

/*===========================================================================*\
    vsnprintf
        Speichert eine formatierte Ausgabe im buffer mit maximal maxcount
        Zeichen (inklusive abschliessende '\0')
        Aufbau wie _vsnprintf (stdio.h) mit Besonderheiten im format:
        %b: "Binäre Ausgabe". Ist der "char"-Parameter 0 wird FALSE ausgegeben,
            ansonsten TRUE. Wenn nicht genug Platz ist oder die Precision auf 1
            eingestellt wurde verkürzt sich die Ausgabe auf 'T' und 'F'.
        %t: Einschub nach '\n'
        Zur Zeit werden folgende "Format Specificationen nicht unterstützt:
            - Die Prefixe:  F, N, h, I
            - Die Typen:    f, e, E, g, G, n
            - Precision:    .[Zahl], da auch keine Float's unterstützt!
        Der Prefix 'l' bzw. 'L' wird nicht von %b, %t, %c, %p unterstützt.

        buffer:     buffer != 0
        maxcount:   maxcount > 0
        format:     format != 0 / "Format-String" (s.z.B. printf und oben)
        argptr:     argptr != 0
        return:     Anzahl der in Buffer geschriebenen Zeichen (ohne '\0')
                    Ist Anzahl < 0 trat ein Fehler auf (normalerweise maxcount
                    zu klein). Inhalt von buffer ist dann undefiniert
\*===========================================================================*/

tSInt a_vsnprintf_tab (char *buffer, tUInt maxcount, const char *format, va_list argptr, tUInt tabsize);
tSInt a_vsnprintf (char *buffer, tUInt maxcount, const char *format, va_list argptr);
tSInt a_snprintf (char *buffer, tUInt maxcount, const char *format, ...);

/*===========================================================================*\
\*===========================================================================*/

#ifdef USE_64TO32BIT_DIVISION

void Special64BitDiv (tUInt64 *pValue64, tUByte divisor);
// Important:       0x02 <= divisor <= 0x10         !!!!
// Function with split a 64Bit Division in multiple 32bit operations.
// This is needed from the printf's for %d, %x in some 32Bit enviroments
// if the linker returns function __aulldiv is missing.

#else

inline void Special64BitDiv (tUInt64 *pValue64, tUByte divisor) {
    (*pValue64) /= divisor;
}

#endif


/*===========================================================================*\
\*===========================================================================*/

#endif
