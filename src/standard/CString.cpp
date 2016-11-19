/*---------------------------------------------------------------------------*\

    Copyright (C) 2000 AVM GmbH. All rights reserved.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.
                                                                               *
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY, without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, and WITHOUT
    ANY LIABILITY FOR ANY DAMAGES arising out of or in connection
    with the use or performance of this software. See the
    GNU General Public License for further details.

\*---------------------------------------------------------------------------*/

#include "CString.h"
#include "aString.h"
#include "aStdLib.h"
#include "aStrPrt.h"
#include "aFlags.h"
#include "dbgSTD.h"

/*---------------------------------------------------------------------------*\
    internal defines
\*---------------------------------------------------------------------------*/

#define CSTRING_PRINT_BufferSize    80          // Must be enough to take a int or
                                                // pointer to string conversion string
#define CSTRING_PRINT_MemoryError   -1

#define PRINTFLAGS_ACTIVE           0x0001      // Placeholder found
#define PRINTFLAGS_READY            0x0002      // Placeholder fully insert
#define PRINTFLAGS_MINUS            0x0004      // Output left oriented
#define PRINTFLAGS_PLUS             0x0008      // Use sign '+' or '-'
#define PRINTFLAGS_ZERO             0x0010      // Fill with leading zeros
#define PRINTFLAGS_BLANK            0x0020      // Use sign '-' or ' '
#define PRINTFLAGS_HASH             0x0040      // Show 0 or 0x before numbers
#define PRINTFLAGS_LARGE            0x0080      // Placeholder is in LARGE-format
#define PRINTFLAGS_SHORT            0x0100      // Placeholder is in SHORT-format
#define PRINTFLAGS_FAR              0x0200      // Placeholder is a FAR-pointer
#define PRINTFLAGS_NEAR             0x0400      // Placeholder is a NEAR-pointer
#define PRINTFLAGS_QUESTION         0x0800      // Sizeof Char specified before string
#define PRINTFLAGS_STRINGTYPE       0x1000      // Placeholder is a CDynamicString-Pointer

/*===========================================================================*\
\*===========================================================================*/

CConstString::CConstString (tString string, tUInt len)
  : pntr (string),
    detectError (vFalse) {

    dhead ("CConstString-Constructor", DCON_CDynamicString);
    dassert (!len || pntr);
    if (!len)  {
        len = check_strlen (string);
    }
    curLen  = len;
    maxSize = (len) ? len + 1 : 0; // TODO?: Is this correct?
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CConstString::check_strlen (const tChar *string) {
    dhead ("CConstString::check_strlen", DCON_CDynamicString);
    dassert (string != 0);
    tSize s = s_strlen (string);
    dassert (s < MAXVAL_tUInt);
    return (tUInt)s;
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CConstString::check_strlen (const tWiChar *string) {
    dhead ("CConstString::check_strlenW", DCON_CDynamicString);
    dassert (string != 0);
    tSize s = s_strlen (string);
    dassert (s < MAXVAL_tUInt);
    return (tUInt)s;
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CConstString::check_strlen (const char *string) {
    dhead ("CConstString::check_strlenC", DCON_CDynamicString);
    dassert (string != 0);
    tSize s = s_strlen (string);
    dassert (s < MAXVAL_tUInt);
    return (tUInt)s;
}

/*===========================================================================*\
\*===========================================================================*/

CDynamicString::CDynamicString (tFormatChar *string, tUInt len) {
    dhead ("CDynamicString-Constructor", DCON_CDynamicString);
    dparams ("%s,%x", string, len);
    Set (string, len);
}

/*===========================================================================*\
\*===========================================================================*/

CDynamicString::CDynamicString (CConstString *pcstr) {
    dhead ("CDynamicString-Constructor", DCON_CDynamicString);
    dparams ("%s", pcstr);
    Set (pcstr);
}

/*===========================================================================*\
\*===========================================================================*/

CDynamicString::CDynamicString (tUInt startSize) {
    dhead ("CDynamicString-Constructor", DCON_CDynamicString);
    Resize (startSize);
}

/*===========================================================================*\
\*===========================================================================*/

CDynamicString::~CDynamicString (void) {
    dhead ("CDynamicString-Destructor", DCON_CDynamicString);
    tString delPntr = pntr;
    pntr            = 0;
    curLen          = 0;
    maxSize         = 0;
    detectError     = vFalse;
    if (delPntr)  {
        delete [] delPntr;
    }
}

/*===========================================================================*\
\*===========================================================================*/

tBool CDynamicString::Change (tString string, tUInt startPos, tUInt len) {
    dhead ("CDynamicString::Change", DCON_CDynamicString);
    dparams ("%s,%x,%x", string, startPos, len, pntr);
    dassert (string != 0);
    dassert (startPos <= GetLen());
    if (!len) {
        len = check_strlen (string);
        dprint ("CalcLen=%x ", len);
    }
    tUInt endPos = len + startPos;
    if (endPos >= GetMaxSize()) {
        if (DynExpand (endPos + 1) == vFalse) {
            return vFalse;
        }
    }
    dassert (endPos < GetMaxSize());
    dassert (pntr != 0);
    s_strncpy (pntr + startPos, string, len);
    if (endPos > GetLen()) {
        pntr[endPos] = '\0';
        curLen       = endPos;
    }
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

tBool CDynamicString::Change (tFormatChar *string, tUInt startPos, tUInt len) {
    dhead ("CDynamicString::Change", DCON_CDynamicString);
    dparams ("%s,%x,%x,%s", string, startPos, len, pntr);
    dassert (string != 0);
    dassert (startPos <= GetLen());
    if (!len) {
        len = check_strlen (string);
        dprint ("CalcLen=%x ", len);
    }
    tUInt endPos = len + startPos;
    if (endPos >= GetMaxSize()) {
        if (DynExpand (endPos + 1) == vFalse) {
            return vFalse;
        }
    }
    dassert (endPos < GetMaxSize());
    dassert (pntr != 0);
    s_strncpy (pntr + startPos, string, len);
    if (endPos > GetLen()) {
        pntr[endPos] = '\0';
        curLen       = endPos;
    }
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

tBool CDynamicString::Insert (tString string, tUInt startPos, tUInt len) {
    dhead ("CDynamicString::Insert", DCON_CDynamicString);
    dassert (string != 0);
    dassert (startPos <= GetLen());
    if (!len) {
        len = check_strlen (string);
    }
    tUInt endPos = len + GetLen();
    if (endPos >= GetMaxSize()) {
        if (DynExpand (endPos + 1) == vFalse) {
            return vFalse;
        }
    }
    dassert (endPos < GetMaxSize());
    dassert (pntr != 0);
    if (GetLen() > startPos) {
        s_strmov (pntr + startPos, len, GetLen() - startPos);
    }
    s_strncpy (pntr + startPos, string, len);
    pntr[endPos] = '\0';
    curLen       = endPos;
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

tBool CDynamicString::Fill (tStringChar fillChar, tUInt len, tUInt startPos) {
    dhead ("CDynamicString::Fill", DCON_CDynamicString);
    dparams ("%c,%x,%x,%s", fillChar, len, startPos, pntr);
    dassert (startPos <= GetLen());
    tUInt endPos = len + startPos;
    if (endPos >= GetMaxSize()) {
        if (DynExpand (endPos + 1) == vFalse) {
            return vFalse;
        }
    }
    dassert (endPos < GetMaxSize());
    dassert (pntr != 0);

    s_strset (pntr + startPos, fillChar, len);
    if (endPos > GetLen()) {
        pntr[endPos] = '\0';
        curLen       = endPos;
    }
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

tCompare CDynamicString::Compare (tString string, tUInt len) {
    dhead ("CDynamicString::Compare", DCON_CDynamicString);
    dassert (string != 0);
    if (!len) {
        len = check_strlen (string);
    }
    tCompare cmp2;
    if (len == GetLen()) {
        cmp2 = vEqual;
    } else if (len > GetLen()) {
        len  = GetLen();
        cmp2 = vHigher;
    } else {
        cmp2 = vLower;
    }
    tCompare fret = s_strncmp (string, pntr, len);
    if (fret == vEqual) {
        fret = cmp2;
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

void CDynamicString::Get (tString string, tUInt len, tUInt startPos) {
    dhead ("CDynamicString::Get", DCON_CDynamicString);
    dassert (string != 0);
    dassert (startPos <= GetLen());
    dassert (len <= GetLen());
    dassert (startPos + len <= GetLen());
    s_strncpy (string, pntr + startPos, len);
    string[len] = '\0';
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CDynamicString::FindChar (tStringChar ch, tUInt startpos) {
    dhead ("CDynamicString::FindChar", DCON_CDynamicString);
    for (; startpos < GetLen(); startpos++) {
        if (pntr[startpos] == ch) {
            RETURN ('x', startpos);
        }
    }
    RETURN ('x', MAXVAL_tUInt);
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CDynamicString::FindLastChar (tStringChar ch) {
    dhead ("CDynamicString::FindLastChar", DCON_CDynamicString);
    tUInt i = GetLen();
    while (i > 0) {
        if (pntr[--i] == ch) {
            RETURN ('x', i);
        }
    }
    RETURN ('x', MAXVAL_tUInt);
}

/*===========================================================================*\
\*===========================================================================*/

void CDynamicString::Remove (tUInt startPos, tUInt len) {
    dhead ("CDynamicString::Remove", DCON_CDynamicString);
    dparams ("%x,%x,%x", startPos, len, GetLen());
    dassert (startPos <= GetLen());
    dassert (len <= GetLen());
    dassert (startPos + len <= GetLen());
    tUInt endPos = startPos + len;
    if (endPos >= GetLen()) {
        SetLen (startPos);
    } else {
        s_strmov (pntr + endPos, -(tSLong)len, GetLen() - endPos + 1);
        dassert (curLen > len);
        curLen -= len;
    }
}

/*===========================================================================*\
\*===========================================================================*/

void CDynamicString::RemoveChars (tString RemoveChars, tUInt startPos, tUInt len) {
    dhead ("CDynamicString::RemoveChars", DCON_CDynamicString);
    if (startPos < GetLen()) {
        if (!len || (len + startPos > GetLen())) {
            len = GetLen() - startPos;
        }
        dassert (GetPointer() != 0);
        tString lauf = GetPointer() + startPos;
        for (; (len > 0) && (s_findfirstchar (*lauf, RemoveChars) == 0); len--, lauf++) {
            dassert (*lauf != 0);
        }
        for (tString current = lauf + 1; len > 0; len--, current++) {
            dassert (*current != 0);
            if (s_findfirstchar (*current, RemoveChars) == 0) {
                *lauf++ = *current;
            } else {
                curLen--;
            }
        }
        *lauf = '\0';
    }
}

/*===========================================================================*\
\*===========================================================================*/

tBool CDynamicString::Resize (tUInt newMaxSize) {
    dhead ("CDynamicString::Resize", DCON_CDynamicString);
    dassert (GetLen() + 1 <= newMaxSize);
    tString delPntr = 0;
    tString newPntr = new tStringChar [newMaxSize];
    if (!newPntr) {
        detectError = vTrue;
        return vFalse;
    }
    if (pntr) {
        s_strncpy (newPntr, pntr, GetLen() + 1);
    } else {
        newPntr[0] = '\0';
    }
    delPntr = pntr;
    pntr    = newPntr;
    maxSize = newMaxSize;
    delete [] delPntr;
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

tSInt CDynamicString::i_vPrintAppend (tFormatChar *format, va_list argptr, tUInt tabsize, tStringChar tabchar) {
    dhead ("CDynamicString::i_vPrintAppend", DCON_CDynamicString);
    dassert (format != 0);
    tFormatChar  localBuf[CSTRING_PRINT_BufferSize];    // Local internal buffer
    tFormatChar *savepos    = format;                   // Position of last "placeholder"
    tFormatChar *copyBuf    = &localBuf[0];             // Buffer that will be append to String
    tUInt        copyBufLen = 0;                        // Size of copyBuf
    tUInt        PrtFlags   = 0;                        // Possible Flags for "placeholder"
    tUInt        PrtWidth   = 0;                        // Width of "placeholder"
    tSInt        PrtPreci   = -2;                       // Precision of "placeholder"
    tUInt        startLen   = GetLen();                 // Len of str before Print was executed

    while (*format != 0) {
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
                // no break!
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
                    PrtWidth = a_strtol (format, &format, 10);
                } else {
                    PrtPreci = a_strtol (format, &format, 10);
                }
                format--;
                break;
            case '*':
                if (PrtPreci == -2) {
                    PrtWidth = va_arg (argptr, tUInt);
                } else {
                    PrtPreci = va_arg (argptr, tUInt);
                }
                break;

            /*----- TYPE -----*/
            case 'b':
                if ((tUByte)va_arg (argptr, int)) {
                    s_strncpy (localBuf, "TRUE", 5);
                    copyBufLen = 4;
                } else {
                    s_strncpy (localBuf, "FALSE", 6);
                    copyBufLen = 5;
                }
                if (PrtPreci == 1) {
                    copyBufLen = 1;
                }
                PrtFlags &= ~(PRINTFLAGS_LARGE | PRINTFLAGS_SHORT);
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 't':
                tabsize    = PrtWidth;
                s_charcpy (&tabchar, (tFormatChar)va_arg (argptr, int));
                copyBufLen = 0;
                PrtFlags  |= PRINTFLAGS_READY;
                break;
            case 'd':
            case 'i':
                copyBufLen = i_Print_ltoa (&copyBuf,
                                           (CheckBitsOne (PrtFlags, PRINTFLAGS_LARGE) ? va_arg (argptr, tUInt64) :
                                           (CheckBitsOne (PrtFlags, PRINTFLAGS_SHORT) ? (tUInt16)va_arg (argptr, int) :
                                                                                        va_arg (argptr, tUInt))),
                                           -10, &PrtWidth, PrtFlags);
                if (!copyBufLen) {
                    RETURN ('x', CSTRING_PRINT_MemoryError);
                }
                PrtFlags &= ~(PRINTFLAGS_LARGE | PRINTFLAGS_SHORT);
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'u':
                copyBufLen = i_Print_ltoa (&copyBuf,
                                           (CheckBitsOne (PrtFlags, PRINTFLAGS_LARGE) ? va_arg (argptr, tUInt64) :
                                           (CheckBitsOne (PrtFlags, PRINTFLAGS_SHORT) ? (tUInt16)va_arg (argptr, int) :
                                                                                        va_arg (argptr, tUInt))),
                                           10, &PrtWidth, PrtFlags);
                if (!copyBufLen) {
                    RETURN ('x', CSTRING_PRINT_MemoryError);
                }
                PrtFlags &= ~(PRINTFLAGS_LARGE | PRINTFLAGS_SHORT);
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'o':
                copyBufLen = i_Print_ltoa (&copyBuf,
                                           (CheckBitsOne (PrtFlags, PRINTFLAGS_LARGE) ? va_arg (argptr, tUInt64) :
                                           (CheckBitsOne (PrtFlags, PRINTFLAGS_SHORT) ? (tUInt16)va_arg (argptr, int) :
                                                                                        va_arg (argptr, tUInt))),
                                           8, &PrtWidth, PrtFlags);
                if (!copyBufLen) {
                    RETURN ('x', CSTRING_PRINT_MemoryError);
                }
                PrtFlags &= ~(PRINTFLAGS_LARGE | PRINTFLAGS_SHORT);
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'x':
                copyBufLen = i_Print_ltoa (&copyBuf,
                                           (CheckBitsOne (PrtFlags, PRINTFLAGS_LARGE) ? va_arg (argptr, tUInt64) :
                                           (CheckBitsOne (PrtFlags, PRINTFLAGS_SHORT) ? (tUInt16)va_arg (argptr, int) :
                                                                                        va_arg (argptr, tUInt))),
                                           0x10, &PrtWidth, PrtFlags);
                if (!copyBufLen) {
                    RETURN ('x', CSTRING_PRINT_MemoryError);
                }
                PrtFlags &= ~(PRINTFLAGS_SHORT | PRINTFLAGS_LARGE);
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'X':
                copyBufLen = i_Print_ltoa (&copyBuf,
                                           (CheckBitsOne (PrtFlags, PRINTFLAGS_LARGE) ? va_arg (argptr, tUInt64) :
                                           (CheckBitsOne (PrtFlags, PRINTFLAGS_SHORT) ? (tUInt16)va_arg (argptr, int) :
                                                                                        va_arg (argptr, tUInt))),
                                           - 0x10, &PrtWidth, PrtFlags);
                if (!copyBufLen) {
                    RETURN ('x', CSTRING_PRINT_MemoryError);
                }
                PrtFlags &= ~(PRINTFLAGS_SHORT | PRINTFLAGS_LARGE);
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'c':
                if ((PrtFlags & PRINTFLAGS_QUESTION) && (va_arg (argptr, tUInt) > 1)) {
                    PrtFlags |= PRINTFLAGS_LARGE;
                }
                if (PrtFlags & PRINTFLAGS_LARGE) {
                    *((tUWiChar *)copyBuf) = (tUWiChar)va_arg (argptr, int);
                } else if (PrtFlags & PRINTFLAGS_SHORT) {
                    *((tUChar *)copyBuf) = (tUChar)va_arg (argptr, int);
                } else {
                    copyBuf[0] = (tFormatChar)va_arg (argptr, int);
                }
                copyBufLen = 1;
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 's':
                if ((PrtFlags & PRINTFLAGS_QUESTION) && (va_arg (argptr, tUInt) > 1)) {
                    PrtFlags |= PRINTFLAGS_LARGE;
                }
                copyBuf = va_arg (argptr, tFormatChar *);
                if (copyBuf) {
                    if (PrtFlags & PRINTFLAGS_LARGE) {
                        copyBufLen = check_strlen ((tUWiChar *)copyBuf);
                    } else if (PrtFlags & PRINTFLAGS_SHORT) {
                        copyBufLen = check_strlen ((tUChar *)copyBuf);
                    } else {
                        copyBufLen = check_strlen (copyBuf);
                    }
                } else {
                    s_strncpy (localBuf, "[NULL]", 7);
                    copyBufLen = 6;
                }
                PrtFlags |= PRINTFLAGS_READY;
                break;
            case 'S':
                {
                    CConstString *pStr = va_arg (argptr, CConstString *);
                    if (pStr) {
                        copyBuf    = (tFormatChar *)pStr->GetPointer();
                        copyBufLen = pStr->GetLen();
                        PrtFlags  |= PRINTFLAGS_STRINGTYPE;
                    } else {
                        s_strncpy (localBuf, "[NULL]", 7);
                        copyBufLen = 6;
                    }
                    PrtFlags |= PRINTFLAGS_READY;
                }
                break;
            case 'p':
                copyBufLen = a_pointer2string (va_arg (argptr, void *), copyBuf);
                PrtFlags  |= PRINTFLAGS_READY;
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
                copyBuf    = savepos;
                copyBufLen = (tUInt)(format - savepos + 1);
                PrtFlags  |= PRINTFLAGS_READY;
                break;
            } //end: switch (*format)

            if (PrtFlags & PRINTFLAGS_READY) {
                // placeholder found
                if (copyBufLen > 0) {
                    if (!(PrtFlags & PRINTFLAGS_MINUS) && (PrtWidth > copyBufLen)) {
                        PrtWidth -= copyBufLen;
                        if (FillAppend ((PrtFlags & PRINTFLAGS_ZERO) ? '0' : ' ', PrtWidth) == vFalse)  {
                            RETURN ('x', CSTRING_PRINT_MemoryError);
                        }
                        PrtWidth = 0;
                    }
                    tUInt endPos = curLen + copyBufLen;
                    if (endPos >= GetMaxSize()) {
                        if (DynExpand (endPos + 1) == vFalse) {
                            RETURN ('x', CSTRING_PRINT_MemoryError);
                        }
                    }
                    dassert (endPos < GetMaxSize());
                    dassert (pntr != 0);
                    if (PrtFlags & PRINTFLAGS_STRINGTYPE) {
                        s_strncpy (pntr + curLen, (tStringChar *)copyBuf, copyBufLen);
                    } else if (PrtFlags & PRINTFLAGS_LARGE) {
                        s_strncpy (pntr + curLen, (tUWiChar *)copyBuf, copyBufLen);
                    } else if (PrtFlags & PRINTFLAGS_SHORT) {
                        s_strncpy (pntr + curLen, (tUChar *)copyBuf, copyBufLen);
                    } else {
                        s_strncpy (pntr + curLen, copyBuf, copyBufLen);
                    }
                    pntr[endPos] = '\0';
                    curLen       = endPos;
                    if ((PrtWidth > copyBufLen) && (FillAppend (' ', PrtWidth - copyBufLen) == vFalse)) {
                        RETURN ('x', CSTRING_PRINT_MemoryError);
                    }
                }
                PrtFlags   = 0;
                PrtWidth   = 0;
                PrtPreci   = -2;
                copyBuf    = &localBuf[0];
                copyBufLen = 0;
                savepos    = format + 1;
            }
        } else {
            switch (*format) {
            case '%':
                //ASSERT (PrtFlags == 0);
                if ((format > savepos) && (Append (savepos, (tUInt)(format - savepos)) == vFalse)) {
                    RETURN ('x', CSTRING_PRINT_MemoryError);
                }
                if (format[1] == '%') {
                    // found %% => eliminate one %
                    format++;
                    savepos = format;
                } else {
                    savepos  = format;
                    PrtFlags = PRINTFLAGS_ACTIVE;
                }
                break;
            case '\n':
                if ((tabsize > 0) && (tabchar > 0)) {
                    if (  (  (format > savepos)
                          && (Append (savepos, (tUInt)(format - savepos)) == vFalse))
                       || (FillAppend (tabchar, tabsize) == vFalse)) {
                        RETURN ('x', CSTRING_PRINT_MemoryError);
                    }
                    savepos = format + 1;
                }
                break;
            }
        }
        format++;
    } //end: while
    if ((format > savepos) && (Append (savepos, (tUInt)(format - savepos)) == vFalse)) {
        RETURN ('x', CSTRING_PRINT_MemoryError);
    }
    dassert (GetLen() >= startLen);
    RETURN ('x', GetLen() - startLen);
}

/*===========================================================================*\
\*===========================================================================*/

tUInt CDynamicString::i_Print_ltoa (tFormatChar **pBuffer, tUInt64 value, tSByte radix, tUInt *pWidth, tUInt Flags) {
    dhead ("CDynamicString::i_Print_ltoa", DCON_CDynamicString);
    dparams ("r=%x,w=%x,f=%x", radix, *pWidth, Flags);
    dassert (pBuffer != 0);
    dassert (*pBuffer != 0);
    dassert (pWidth != 0);
    dassert (radix <= 16);
    dassert (radix >= -16);
    dassert (CSTRING_PRINT_BufferSize > sizeof (tUInt64) * 8 + 3);
    static tFormatChar LowDigitForStrings[] =  {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'x'
    };
    static tFormatChar HighDigitForStrings[] =  {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'X'
    };

    tFormatChar *DigitForStrings = &LowDigitForStrings[0];
    tFormatChar *buffer          = *pBuffer + CSTRING_PRINT_BufferSize - 1;
    tFormatChar  prechar         = 0;
    tUInt        prelen          = 0;
    tUInt        bufpos          = 0;

    if (Flags & PRINTFLAGS_PLUS) {
        prechar = '+';
    } else if (Flags & PRINTFLAGS_BLANK) {
        prechar = ' ';
    }
    switch (radix) {
    case -10:
        if ((tSInt64)value < 0) {
            prechar = '-';
            value  *= -1;
        }
        radix *= -1;
        break;
    case -0x10:
        DigitForStrings = &HighDigitForStrings[0];
        radix *= -1;
        break;
    }
    dassert (radix > 1);

    tUInt64 orgval;
    while (value >= (tUInt64) radix) {
        orgval  = value;
        //value /= radix;
        Special64BitDiv (&value, (tUByte)radix);
        dassert ((tUInt)(orgval - value * radix) < 0x10);
        *buffer-- = DigitForStrings[(tUByte)(orgval - value * radix)];
        bufpos++;
    }
    bufpos++;
    *buffer  = DigitForStrings[value];
    if (Flags & PRINTFLAGS_HASH) {
        switch (radix) {
        case 16:
            *--buffer = DigitForStrings[0x10];  // x or X
            prelen++;
            // no break!
        case 8:
            *--buffer = '0';
            prelen++;
            break;
        }
    }
    if (prechar) {
        *--buffer = prechar;
        prelen++;
    }
    bufpos += prelen;

    if (prelen && ((Flags & (PRINTFLAGS_MINUS | PRINTFLAGS_ZERO)) == PRINTFLAGS_ZERO) && (*pWidth > bufpos)) {
        // PRINTFLAGS_MINUS is not set and PRINTFLAGS_ZERO ist set
        if (Append (buffer, prelen) == vTrue) {
            buffer  += prelen;
            bufpos  -= prelen;
            *pWidth -= prelen;
        } else {
            bufpos = 0;
        }
        prelen   = 0;
    }
    *pBuffer = buffer;
    return bufpos;
}

/*===========================================================================*\
\*===========================================================================*/
