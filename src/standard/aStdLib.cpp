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
#include "CString.h"
#include "DataPack.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

static char DigitForStrings[16] =  {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
};

/*===========================================================================*\
\*===========================================================================*/

tUByte a_count1bits (tUInt32 value) {
    tUByte fret = 0;
    for (; value > 0; value >>= 1) {
        if (value & 0x1) {
            fret++;
        }
    }
    return fret;
}

tUByte a_count1bits (tUInt64 value) {
    tUByte fret = 0;

    #ifdef USE_64TO32BIT_DIVISION
        fret = a_count1bits (*(tUInt32 *)&value) + a_count1bits (*(((tUInt32 *)&value) + 1));
    #else
        for (; value > 0; value >>= 1) {
            if (value & 0x1) {
                fret++;
            }
        }
    #endif

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
    tULong p   = (tULong) pointer;
    tChar *fmt = ((sizeof (p) > 4) ? (tChar *)"    .    :    .    " : (tChar *)"    :    "); // must be symmetric!
    tUInt  i   = s_strlen (fmt);

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
    tULong   p   = (tULong) pointer;
    const tWiChar *fmt = (sizeof (p) > 4) ? L"    .    :    .    " : L"    :    "; // must be symmetric!
    tUInt    i   = s_strlen (fmt);

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

static tSByte a_hex2bin_conv (tStringChar ch) {
    switch (ch) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return ch - '0';
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
        return ch - 'A' + 10;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
        return ch - 'a' + 10;
    }
    return -1;
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

tBool a_hex2bin (CConstString *pHexString, CDataPacket *pBinPacket) {
    //dassert (pHexString != 0);
    //dassert (pBinPacket != 0);
    tString str  = pHexString->GetPointer();
    tUByte *bin  = pBinPacket->GetLenPointer();
    tSByte  val1 = -1;
    tSByte  val2;
    //dassert (bin != 0);
    for (tSize len = pHexString->GetLen(); len > 0; len--, str++) {
        val2 = a_hex2bin_conv (*str);
        if (val2 >= 0) {
            if (val1 == -1) {
                val1 = val2;
            } else {
                if (!pBinPacket->GetFreeLen()) {
                    if (infoError (pBinPacket->Resize (pBinPacket->GetMaxLen() + (len / 2)))) {
                        return vFalse;
                    }
                }
                *bin = (val1 << 4) + val2;
                val1 = -1;
                bin++;
                pBinPacket->IncLen();
            }
        }
    }
    return vTrue;
}


/*===========================================================================*\
\*===========================================================================*/

