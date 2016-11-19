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

#include "aStdLib.h"
#include "aString.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

static char DigitForStrings[16] =  {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
};

/*===========================================================================*\
\*===========================================================================*/

tUByte a_count1bits (tUInt value) {
    tUByte fret = 0;
    for (; value > 0; value >>= 1) {
        if (value & 0x1) {
            fret++;
        }
    }
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/

tSInt a_atol (char *string) {
    tSInt value = 0;
    tSInt sign = 1;

    //ASSERT (string != 0);

    while (*string == ' ') {
        string++;
    }
    switch (*string) {
    case '-':
        sign = -1;
        // kein break!!
    case '+':
        string++;
        break;
    }
    for (; ((*string != '\0') && (*string >= '0') && (*string <= '9')); string++) {
        value = value * 10 + *string - '0';
    }
    return value * sign;
}

/*===========================================================================*\
\*===========================================================================*/

tSInt a_strtoln (tChar *nptr, tChar **endptr, tSize maxlength, tSInt base) {
    tSInt value = 0;
    tSInt sign = 1;
    tChar figure;

    //ASSERT (nptr != 0);
    //ASSERT ((base >= 2) && (base <= 16));
    //ASSERT (maxlength != 0);

    while ((*nptr == ' ')) {
        nptr++;
    }
    switch (*nptr) {
    case '-':
        sign = -1;
        // no break!!
    case '+':
        nptr++;
        break;
    }
    for (; ((*nptr != '\0') && (maxlength)); nptr++, maxlength--) {
        figure = (tChar)(*nptr - '0');
        if (figure >= ('A'-'0')) {
            figure = (tChar)(figure - ((figure >= 'a'-'0') ? 'a'-'0'-10 : 'A'-'0'-10));
        }
        if (figure >= base) {
            break;
        }
        value = value * base + figure;
    }
    if (endptr) {
        *endptr = nptr;
    }
    return value * sign;
}

/*===========================================================================*\
\*===========================================================================*/

tSInt a_strtoln (tWiChar *nptr, tWiChar **endptr, tSize maxlength, tSInt base) {
    tSInt   value = 0;
    tSInt   sign = 1;
    tWiChar figure;

    //ASSERT (nptr != 0);
    //ASSERT ((base >= 2) && (base <= 16));
    //ASSERT (maxlength != 0);

    while ((*nptr == ' ')) {
        nptr++;
    }
    switch (*nptr) {
    case '-':
        sign = -1;
        // no break!!
    case '+':
        nptr++;
        break;
    }
    for (; ((*nptr != '\0') && (maxlength)); nptr++, maxlength--) {
        figure = (tWiChar)(*nptr - '0');
        if (figure >= ('A'-'0')) {
            figure = (tWiChar)(figure - ((figure >= 'a'-'0') ? 'a'-'0'-10 : 'A'-'0'-10));
        }
        if (figure >= base) {
            break;
        }
        value = value * base + figure;
    }
    if (endptr) {
        *endptr = nptr;
    }
    return value * sign;
}

/*===========================================================================*\
\*===========================================================================*/

tUInt a_ltoan (tSInt value, char *string, tSize maxlen, tSInt radix) {
    tUInt uvalue;
    char  buffer[sizeof (tULong) * 8];
    tUInt bufpos = sizeof (buffer) - 1;
    char  prechar = '\0';

    // ASSERT (string != 0);
    // ASSERT ((radix <= 16) && (radix >= -16) && (radix != 0) && (radix != 1) && (radix != -1));
    // ASSERT (maxlen > 1);

    if (radix < 0) {
        radix *= -1;
        if (value < 0) {
            value *= -1;
            prechar = '-';
            maxlen--;
        }
    }
    uvalue = (tUInt) value;

    while (uvalue >= (tUInt)radix) {
        buffer[bufpos--] = DigitForStrings[uvalue % radix];
        uvalue /= radix;
    }
    buffer[bufpos] = DigitForStrings[uvalue % radix];

    bufpos = sizeof (buffer) - bufpos;

    if (maxlen <= bufpos) {
        // Platz fr String zu klein
        *string = '\0';
        return 0;
    }

    // Konvertierung in den String kopieren
    if (prechar) {
        *string++ = prechar;
    }
    s_memcpy (string, buffer + sizeof (buffer) - bufpos, bufpos);

    string[bufpos] = '\0';
    return bufpos + ((prechar) ? 1 : 0);
}

/*===========================================================================*\
\*===========================================================================*/

char *a_strupr (char *pString) {
    if (pString) {
        char *lauf;
        for (lauf = pString; *lauf != '\0'; lauf++) {
            if ((*lauf >= 'a') && (*lauf <= 'z')) {
                *lauf -= ('a' - 'A');
            }
        }
    }
    return pString;
}

/*===========================================================================*\
\*===========================================================================*/

tUInt a_pointer2string (void *pointer, tChar *string) {
    tULong  p   = (tULong) pointer;
    tChar  *fmt = ((sizeof (p) > 4) ? (tChar *)"    .    :    .    " : (tChar *)"    :    "); // must be symmetric!
    unsigned i  = s_strlen (fmt);

    string[i] = '\0';
    string--;
    for (; i > 0; i--, fmt++) {
        if (*fmt == ' ') {
            string [i] = DigitForStrings[p % 0x10];
            p /= 0x10;
        } else  {
            string [i] = *fmt;
        }
    }
    return (sizeof (p) > 4) ? 19 : 9;
}

/*===========================================================================*\
\*===========================================================================*/

tUInt a_pointer2string (void *pointer, tWiChar *string) {
    tULong    p   = (tULong) pointer;
    const tWiChar  *fmt = (sizeof (p) > 4) ? L"    .    :    .    " : L"    :    "; // must be symmetric!
    unsigned i  = s_strlen (fmt);

    string[i] = '\0';
    string--;
    for (; i > 0; i--, fmt++) {
        if (*fmt == ' ') {
            string [i] = DigitForStrings[p % 0x10];
            p /= 0x10;
        } else  {
            string [i] = *fmt;
        }
    }
    return (sizeof (p) > 4) ? 19 : 9;
}

/*===========================================================================*\
\*===========================================================================*/

