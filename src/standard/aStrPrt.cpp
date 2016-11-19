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

#include "aStrPrt.h"
#include "aStdLib.h"
#include "aString.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define PRINTFLAGS_ACTIVE           0x001       // Placeholder found
#define PRINTFLAGS_READY            0x002       // Placeholder fully insert
#define PRINTFLAGS_MINUS            0x004       // Output left oriented
#define PRINTFLAGS_PLUS             0x008       // Use sign '+' or '-'
#define PRINTFLAGS_ZERO             0x010       // Fill with leading zeros
#define PRINTFLAGS_BLANK            0x020       // Use sign '-' or ' '
#define PRINTFLAGS_HASH             0x040       // Show 0 or 0x before numbers
#define PRINTFLAGS_LARGE            0x080       // Placeholder is in LARGE-format
#define PRINTFLAGS_SHORT            0x100       // Placeholder is in SHORT-format
#define PRINTFLAGS_FAR              0x200       // Placeholder is a FAR-pointer
#define PRINTFLAGS_NEAR             0x400       // Placeholder is a NEAR-pointer
#define PRINTFLAGS_QUESTION         0x800       // Sizeof Char specified before string


/*===========================================================================*\
    internal functions for "vsnprintf" !
\*===========================================================================*/

tUInt ltoanX (tSInt64 value, char *string, tSize maxlen, tSByte radix, tUInt width, tUInt flags) {
    static tUChar DigitForStrings[16] =  {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
        'a', 'b', 'c', 'd', 'e', 'f'
    };
    tUInt64 uvalue;
    tUChar  prechar[3];
    tUChar  buffer[sizeof (tSInt64) * 8];
    tSize   precharlen = 0;
    tSize   bufpos     = sizeof (buffer) - 1;

    // ASSERT (string != 0);
    // ASSERT ((radix <= 16) && (radix >= -16) && (radix != 0) && (radix != 1) && (radix != -1));
    // ASSERT (maxlen > 1);

    if (value < 0) {
        if (radix < 0) {
            value *= -1;
            prechar[precharlen++] = '-';
        }
    } else {
        if (flags & PRINTFLAGS_PLUS) {
            prechar[precharlen++] = '+';
        } else if (flags & PRINTFLAGS_BLANK) {
            prechar[precharlen++] = ' ';
        }
    }
    if (radix < 0) {
        radix *= -1;
    }
    if (flags & PRINTFLAGS_HASH) {
        switch (radix) {
        case 8:
            prechar[precharlen++] = '0';
            break;
        case 16:
            prechar[precharlen++] = '0';
            prechar[precharlen++] = 'x';
            break;
        }
    }

    uvalue = (tUInt64) value;
    tUInt64 orgval;
    while (uvalue >= (tUInt64)radix) {
        orgval  = uvalue;
        //uvalue /= radix;
        Special64BitDiv (&uvalue, (tUByte)radix);
        buffer[bufpos--] = DigitForStrings[(tUByte)(orgval - uvalue * radix)];
    }
    buffer[bufpos] = DigitForStrings[uvalue];

    uvalue = bufpos;
    bufpos = sizeof (buffer) - bufpos + precharlen;

    if (maxlen <= bufpos) {
        *string = '0';
        return 0;
    }
    if ((width > bufpos) && !(flags & PRINTFLAGS_MINUS)) {
        width -= bufpos;
        s_memset (string, ((flags & PRINTFLAGS_ZERO) ? '0' : ' '), width);
        string += width;
    } else {
        width = 0;
    }

    if (precharlen) {
        s_memcpy (string, prechar, precharlen);
    }
    s_memcpy (string + precharlen, buffer + uvalue, bufpos - precharlen);

    string[bufpos] = '\0';
    return bufpos + width;
}


/*===========================================================================*\
\*===========================================================================*/

tSInt a_vsnprintf_tab (char *buffer, tUInt maxcount, const char *format, va_list argptr, tUInt tabsize) {
    tUInt PrtFlags   = 0;           // Possible Flags for "placeholder"
    tUInt PrtWidth   = 0;           // Width of "placeholder"
    tSInt PrtPreci   = -2;          // Precision of "placeholder"
    tUInt PrtValue   = 0;           // Returnvalue of convert function
    const char *savepos    = format;
    char  argchar[255]    = "";
    char  tabchar    = ' ';
    tUInt count      = maxcount;    // free place in buffer
    tUInt ErrorFound = 0;

    char * va_arg_res = NULL;

    while ((count > 1) && (*format != 0) && (!ErrorFound)) {
        if (PrtFlags) {

            /*----- exams placeholders (%...) -----*/
            switch (*format) {

            /*----- FLAGS -----*/
            case '-':
                PrtFlags |= PRINTFLAGS_MINUS;
                break;
            case '+':
                PrtFlags |= PRINTFLAGS_PLUS;
                break;
            case ' ':
                PrtFlags |= PRINTFLAGS_BLANK;
                break;
            case '#':
                PrtFlags |= PRINTFLAGS_HASH;
                break;
            case '?':
                PrtFlags |= PRINTFLAGS_QUESTION;
                break;
            case 'l':
            case 'L':
                PrtFlags |= PRINTFLAGS_LARGE;
                break;

            case 'h':
                PrtFlags |= PRINTFLAGS_SHORT;
                break;

            /*----- WIDTH or PRECISION-----*/
            case '0':
                if (PrtPreci == -2) {
                    PrtFlags |= PRINTFLAGS_ZERO;
                }
                // kein break !
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (PrtPreci == -2) {
                    PrtWidth = a_strtol (format, (char **)&format, 10);
                } else {
                    PrtPreci = a_strtol (format, (char **)&format, 10);
                }
                format--;
                break;
            case '*':
                if (PrtPreci == -2) {
                    PrtWidth = va_arg (argptr, unsigned);
                } else {
                    PrtPreci = va_arg (argptr, unsigned);
                }
                break;

            /*----- TYPE -----*/
            case 'b':
                if ((char)va_arg (argptr, int)) {
                    strncpy(argchar, "TRUE", sizeof(argchar));
                    PrtValue = 4;
                } else {
                    strncpy(argchar, "FALSE", sizeof(argchar));
                    PrtValue = 5;
                }
                if (PrtPreci == 1) {
                    PrtValue = 1;
                }
                if ((PrtWidth > (unsigned)PrtValue) && !(PrtFlags & PRINTFLAGS_MINUS)) {
                    PrtWidth = (PrtWidth - PrtValue >= count) ? count - 1: PrtWidth - PrtValue;
                    s_memset (buffer, ' ', PrtWidth);
                    count -= PrtWidth;
                    buffer += PrtWidth;
                }
                if (count > (unsigned)PrtValue) {
                    s_memcpy (buffer, argchar, PrtValue);
                } else if (count > 1) {
                    /*----- show only 'T'- or 'F' -----*/
                    *buffer = *argchar;
                    PrtValue = 1;
                } else {
                    PrtValue = 0;
                }
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 't':
                tabsize = PrtWidth;
                tabchar = (char)va_arg (argptr, int);

                PrtWidth = 0;
                PrtPreci = -2;
                buffer--;
                count++;
                PrtValue = 1;
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'd':
            case 'i':
                PrtValue = ltoanX ((PrtFlags & PRINTFLAGS_LARGE) ? va_arg (argptr, tSInt64) : va_arg (argptr, tSInt),
                                   buffer, count, -10, PrtWidth, PrtFlags);
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'u':
                PrtValue = ltoanX ((PrtFlags & PRINTFLAGS_LARGE) ? va_arg (argptr, tUInt64) : va_arg (argptr, tUInt),
                                   buffer, count, 10, PrtWidth, PrtFlags);
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'o':
                PrtValue = ltoanX ((PrtFlags & PRINTFLAGS_LARGE) ? va_arg (argptr, tUInt64) : va_arg (argptr, tUInt),
                                   buffer, count, 8, PrtWidth, PrtFlags);
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'x':
                PrtValue = ltoanX ((PrtFlags & PRINTFLAGS_LARGE) ? va_arg (argptr, tUInt64) : va_arg (argptr, tUInt),
                                   buffer, count, 0x10, PrtWidth, PrtFlags);
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'X':
                PrtValue = ltoanX ((PrtFlags & PRINTFLAGS_LARGE) ? va_arg (argptr, tUInt64) : va_arg (argptr, tUInt),
                                   buffer, count, 0x10, PrtWidth, PrtFlags);
                if (PrtValue) {
                    a_strupr (buffer);
                }
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'c':
                if ((PrtFlags & PRINTFLAGS_QUESTION) && (va_arg (argptr, tUInt) > 1)) {
                    PrtFlags |= PRINTFLAGS_LARGE;
                }
                if ((PrtWidth > 1) && !(PrtFlags & PRINTFLAGS_MINUS)) {
                    PrtValue = (PrtWidth > count) ? (count - 1) : (PrtWidth - 1);
                    memset (buffer, ' ', PrtValue);
                } else {
                    PrtValue = 0;
                }
                if (count - PrtValue > 1) {
                    buffer[PrtValue] = (char)va_arg (argptr, int);
                    PrtValue++;
                }
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 's':
                if ((PrtFlags & PRINTFLAGS_QUESTION) && (va_arg (argptr, tUInt) > 1)) {
                    PrtFlags |= PRINTFLAGS_LARGE;
                }

                va_arg_res = va_arg (argptr, char *);
                
                if (!va_arg_res) {
                    PrtFlags &= ~PRINTFLAGS_LARGE;  // remove LARGE-FLAG    
                    strncpy(argchar, "[NULL]", sizeof(argchar));
                } else {
                    strncpy(argchar, va_arg_res, sizeof(argchar));
                    argchar[sizeof(argchar)-1] = '\0';
                }
                if (PrtFlags & PRINTFLAGS_LARGE) {
                    PrtValue = 0;
                    while (argchar[PrtValue * 2] != '\0') {
                        PrtValue++;
                    }
                } else {
                    PrtValue = s_strlen (argchar);
                }
                if ((PrtWidth > (unsigned) PrtValue) && !(PrtFlags & PRINTFLAGS_MINUS)) {
                    PrtWidth = (PrtWidth - PrtValue >= count) ? (count - 1) : (PrtWidth - PrtValue);
                    memset (buffer, ' ', PrtWidth);
                    count -= PrtWidth;
                    buffer += PrtWidth;
                }
                if ((unsigned) PrtValue >= count) {
                    PrtValue = count - 1;
                }
                if (PrtValue) {
                    if (PrtFlags & PRINTFLAGS_LARGE) {
                        for (ErrorFound = 0; PrtValue - ErrorFound > 0; ErrorFound++) {
                            buffer[ErrorFound] = argchar[ErrorFound * 2];
                        }
                        ErrorFound = 0;
                    }
                    else {
                        s_memcpy (buffer, argchar, PrtValue);
                    }
                } else {
                    buffer--;
                    count++;
                    PrtValue = 1;
                }
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'p':
                if ((PrtWidth > 9) && !(PrtFlags & PRINTFLAGS_MINUS)) {
                    PrtWidth = (PrtWidth - 9 >= count) ? count - 1: PrtWidth - 9;
                    memset (buffer, ' ', PrtWidth);
                    count -= PrtWidth;
                    buffer += PrtWidth;
                }
                PrtValue = ((sizeof (void *) > 4) ? 19 : 9 );
                if (count > (unsigned)PrtValue) {
                    a_pointer2string (va_arg (argptr, void *), buffer);
                } else {
                    PrtValue = 0;
                }
                PrtFlags |= PRINTFLAGS_READY;
                break;

            /*----- Found PRECISION -----*/
            case '.':
                if (PrtPreci == -2) {
                    PrtPreci = -1;
                    break;
                }
                /*----- no Break! -----*/

            /*----- OTHERS: wrong placeholder -----*/
            default:
                PrtValue = (int) (format - savepos + 1);
                if ((unsigned) PrtValue >= count) {
                    PrtValue = count - 1;
                }
                s_strncpy (buffer, savepos, PrtValue);
                PrtFlags |= PRINTFLAGS_READY;
                break;
            } //end: switch (*format)

            if (PrtFlags & PRINTFLAGS_READY) {
                // placeholder found
                if (PrtValue > 0) {
                    if ((PrtPreci >= 0) && ((tUInt)PrtPreci < PrtValue)) {
                        /*----- TODO: wrong!!! -----*/
                        PrtValue = PrtPreci;
                    }
                    count -= PrtValue;
                    buffer += PrtValue;
                    if ((PrtWidth > (unsigned)PrtValue) && (PrtFlags & PRINTFLAGS_MINUS)) {
                        PrtWidth -= PrtValue;
                        if (PrtWidth >= count) {
                            PrtWidth = count - 1;
                        }
                        count -= PrtWidth;
                        for (; (PrtWidth > 0); PrtWidth--) {
                            *buffer++ = ' ';
                        }
                    }
                } else {
                    ErrorFound = 1;
                }
                PrtFlags = 0;
                PrtWidth = 0;
                PrtPreci = -2;
                PrtValue = 0;
            }

        } else if (*format == '%') {
            //ASSERT (PrtFlags == 0);
            savepos = format;
            if (*(format + 1) == '%') {
                count--;
                *buffer++ = *format++;
            } else {
                PrtValue = 0;
                PrtWidth = 0;
                PrtPreci = -2;
                PrtFlags = PRINTFLAGS_ACTIVE;
            }

        } else {
            count--;
            *buffer++ = *format;
            if ((*format == '\n') && (tabsize > 0) && (tabsize < count) && (tabchar > 0)) {
                memset (buffer, tabchar, tabsize);
                buffer += tabsize;
                count  -= tabsize;
            }
        }
        format++;
    } //end: while

    *buffer = '\0';
    if (ErrorFound) {
        return -1;
    }
    return (maxcount - count);
}


/*===========================================================================*\
\*===========================================================================*/

tSInt a_vsnprintf (char *buffer, tUInt maxcount, const char *format, va_list argptr) {
    return a_vsnprintf_tab (buffer, maxcount, format, argptr, 0);
}


/*===========================================================================*\
\*===========================================================================*/

tSInt a_snprintf (char *buffer, tUInt maxcount, const char *format, ...) {
    tSInt fret;
    va_list arg_ptr;
    va_start (arg_ptr, format);
    fret = a_vsnprintf_tab (buffer, maxcount, format, arg_ptr, 0);
    va_end (arg_ptr);
    return fret;
}


/*===========================================================================*\
\*===========================================================================*/
#ifdef USE_64TO32BIT_DIVISION

static struct {
    tUInt32 wert;
    tUInt32 rest;
} Special64BitDivArray[15] = {
    { 0x100000000 / 0x02, 0 },
    { 0x100000000 / 0x03, 1 },
    { 0x100000000 / 0x04, 0 },
    { 0x100000000 / 0x05, 1 },
    { 0x100000000 / 0x06, 4 },
    { 0x100000000 / 0x07, 4 },
    { 0x100000000 / 0x08, 0 },
    { 0x100000000 / 0x09, 4 },
    { 0x100000000 / 0x0a, 6 },
    { 0x100000000 / 0x0b, 4 },
    { 0x100000000 / 0x0c, 4 },
    { 0x100000000 / 0x0d, 9 },
    { 0x100000000 / 0x0e, 4 },
    { 0x100000000 / 0x0f, 1 },
    { 0x100000000 / 0x10, 0 }
};


void Special64BitDiv (tUInt64 *pValue64, tUByte divisor) {
    // dassert (divisor >= 0x02);
    // dassert (divisor <= 0x10);
    //if (*pValue64 >= 0x100000000) {
        tUInt32 *pValue  = ((tUInt32 *)pValue64) + 1;
        tUInt32  highVal = *pValue;
        tUInt32  lowVal;
        *pValue /= divisor;
        highVal -= (*pValue) * divisor;
        pValue--;
        lowVal   = *pValue;
        *pValue /= divisor;
        lowVal  -= (*pValue) * divisor;
        *pValue += highVal * Special64BitDivArray[divisor - 2].wert;
        lowVal  += highVal * Special64BitDivArray[divisor - 2].rest;
        *pValue += lowVal / divisor;
    //} else {
    //    *(tUInt32 *)pValue64 /= divisor;
    //}
}

#endif
/*===========================================================================*\
\*===========================================================================*/
