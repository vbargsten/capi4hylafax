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

#ifndef _CSTRING_H_
#define _CSTRING_H_

#include "aTypes.h"
#include "aString.h"
#include <stdarg.h>

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define CSTRING_RESIZE_ALLOCATE_OFFSET          100

#ifdef CSTRING_CHAR_IS_WIDE_CHAR
typedef tWiChar      tStringChar;
#else
typedef tChar        tStringChar;
#endif

typedef tStringChar *tString;

typedef char        tFormatChar;

/*===========================================================================*\
\*===========================================================================*/

class CConstString {
public:
    CConstString (tString string, tSize len = 0);   // string have to be null terminated! len ist not tested!
    virtual ~CConstString (void) {}

    tBool   IsEmpty (void);
    tSize   GetLen (void);      // GetLen & Get(Max)Size return multiples of tStringChar!
    tSize   GetSize (void);
    tSize   GetMaxSize (void);
    tString GetPointer (void);
    tBool   ErrorDetected (void);

protected:
    CConstString (void);

    tString pntr;
    tSize   curLen;
    tSize   maxSize;
    tBool   detectError;
};


/*===========================================================================*\
\*===========================================================================*/

class CDynamicString : public CConstString {
public:
    CDynamicString (void) {}
    CDynamicString (tFormatChar *string, tSize len = 0);
    CDynamicString (CConstString *pcstr);
    CDynamicString (tUInt startSize);
    virtual ~CDynamicString (void);

    tBool Set (tString       string, tSize len = 0);
    tBool Set (tFormatChar  *string, tSize len = 0);
    tBool Set (CConstString *pcstr);
    tBool Append (tString       string, tSize len = 0);
    tBool Append (tFormatChar  *string, tSize len = 0);
    tBool Append (CConstString *pcstr);

    tBool Change (tString       string, tSize startPos, tSize len = 0);
    tBool Change (tFormatChar  *string, tSize startPos, tSize len = 0);
    tBool Change (CConstString *pcstr,  tSize startPos, tSize len = 0);
    tBool Insert (tString       string, tSize startPos, tSize len = 0);
    tBool Insert (tFormatChar  *string, tSize startPos, tSize len = 0);
    tBool Insert (CConstString *pcstr,  tSize startPos, tSize len = 0);

    tBool Fill (tStringChar fillChar, tSize len, tSize startPos = 0);
    tBool FillAppend (tStringChar fillChar, tUInt len);

    tSInt Print  (tFormatChar *format, ...);
    tSInt vPrint (tFormatChar *format, va_list argptr);
    tSInt PrintAppend  (tFormatChar *format, ...);
    tSInt vPrintAppend (tFormatChar *format, va_list argptr);

    tCompare Compare (tString string, tSize len = 0);
    tBool IsIdentical (tString string, tSize len = 0);

    void Get (tString string, tSize len = 0, tSize startPos = 0);
    tStringChar GetChar (tSize pos);
    tStringChar GetLastChar (void);
    tSize FindChar (tStringChar ch, tSize startpos = 0); // not found => return = MAXVAL_tSize
    tSize FindLastChar (tStringChar ch);                 // not found => return = MAXVAL_tSize

    void  SetLen (tSize newLen);
    void  CutLen (tSize cutLen);   // Result: newlen = oldLen - cutLen
    void  Remove (tSize startPos, tSize len);
    void  RemoveChars (tString RemoveChars, tSize startPos = 0, tSize len = 0);
    void  RemoveAll (void);
    tBool Resize (tSize newMaxSize);

private:
    tSInt i_vPrintAppend (tFormatChar *format, va_list argptr, tUInt tabsize, tStringChar tabchar);
    tUInt i_Print_ltoa (tFormatChar **pBuffer, tUInt64 value, tSByte radix, tUInt *pWidth, tUInt pFlags);
    tBool DynExpand (tSize newMaxSize);
};


/*===========================================================================*\
    inline fuctions from CConstString
\*===========================================================================*/

inline CConstString::CConstString (void)
  : pntr (0),
    curLen (0),
    maxSize (0),
    detectError (vFalse) {
}

inline tBool CConstString::IsEmpty (void) {
    return (curLen == 0);
}

inline tSize CConstString::GetLen (void) {
    return curLen;
}

inline tSize CConstString::GetSize (void) {
    return (GetLen()) ? curLen + 1 : 0;
}

inline tSize CConstString::GetMaxSize (void) {
    return maxSize;
}

inline tString CConstString::GetPointer (void) {
    return (GetLen()) ? pntr : 0;
}

inline tBool CConstString::ErrorDetected (void) {
    if (detectError == vFalse) {
        return vFalse;
    }
    detectError = vFalse;
    return vTrue;
}


/*===========================================================================*\
    inline fuctions from CDynamicString
\*===========================================================================*/

inline tBool CDynamicString::Set (tString string, tSize len) {
    RemoveAll();
    return Append (string, len);
}

inline tBool CDynamicString::Set (tFormatChar *string, tSize len) {
    RemoveAll();
    return Append (string, len);
}

inline tBool CDynamicString::Set (CConstString *pcstr) {
    if (pcstr->GetLen()) {
        return Set (pcstr->GetPointer(), pcstr->GetLen());
    }
    RemoveAll();
    return vTrue;
}

inline tBool CDynamicString::Append (tString string, tSize len) {
    return Change (string, GetLen(), len);
}

inline tBool CDynamicString::Append (tFormatChar *string, tSize len) {
    return Change (string, GetLen(), len);
}

inline tBool CDynamicString::Append (CConstString *pcstr) {
    return (pcstr->GetLen()) ? Change (pcstr->GetPointer(), GetLen(), pcstr->GetLen()) : vTrue;
}

inline tBool CDynamicString::Change (CConstString *pcstr, tSize startPos, tSize len) {
    return (pcstr->GetLen()) ? Change (pcstr->GetPointer(), startPos, (len) ? len : pcstr->GetLen()) : vTrue;
}

inline tBool CDynamicString::Insert (CConstString *pcstr, tSize startPos, tSize len) {
    return (pcstr->GetLen()) ? Insert (pcstr->GetPointer(), startPos, (len) ? len : pcstr->GetLen()) : vTrue;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CDynamicString::FillAppend (tStringChar fillChar, tUInt len) {
    return Fill (fillChar, len, GetLen());
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tSInt CDynamicString::Print (tFormatChar *format, ...) {
    va_list arg_ptr;
    va_start (arg_ptr, format);
    tUInt fret = vPrint (format, arg_ptr);
    va_end (arg_ptr);
    return fret;
}

inline tSInt CDynamicString::vPrint (tFormatChar *format, va_list argptr) {
    RemoveAll();
    return vPrintAppend (format, argptr);
}

inline tSInt CDynamicString::PrintAppend (tFormatChar *format, ...) {
    va_list arg_ptr;
    va_start (arg_ptr, format);
    tUInt fret = vPrintAppend (format, arg_ptr);
    va_end (arg_ptr);
    return fret;
}

inline tSInt CDynamicString::vPrintAppend (tFormatChar *format, va_list argptr) {
    return i_vPrintAppend (format, argptr, 0, 0);
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CDynamicString::IsIdentical (tString string, tSize len) {
    if (!len) {
        len = s_strlen (string);
    }
    return ((len == GetLen()) && (s_strcmp (string, pntr) == vEqual)) ? vTrue : vFalse;
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tStringChar CDynamicString::GetChar (tSize pos) {
    //assert (pos < GetLen());
    //assert (pntr != 0);
    return (pos < GetLen()) ? pntr[pos] : '\0';
}

inline tStringChar CDynamicString::GetLastChar (void) {
    //assert (GetLen() > 0);
    //assert (pntr != 0);
    return pntr[GetLen() - 1];
}

inline void CDynamicString::SetLen (tSize newLen) {
    // assert (pntr != 0);
    // warning (newLen > curLen);
    if (curLen > newLen) {
        curLen = newLen;
        pntr[curLen] = '\0';
    }
}

inline void CDynamicString::CutLen (tSize cutLen) {
    // assert (pntr != 0);
    // warning (cutLen > curLen);
    if (curLen) {
        if (curLen > cutLen) {
            curLen -= cutLen;
        } else {
            curLen = 0;
        }
        pntr[curLen] = '\0';
    }
}

inline void CDynamicString::RemoveAll (void) {
    if (curLen) {
        // assert (pntr != 0);
        curLen  = 0;
        pntr[0] = '\0';
    }
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CDynamicString::DynExpand (tSize newMaxSize) {
    return Resize ((GetMaxSize() + CSTRING_RESIZE_ALLOCATE_OFFSET > newMaxSize) ?
                    GetMaxSize() + CSTRING_RESIZE_ALLOCATE_OFFSET : newMaxSize);
}

/*===========================================================================*\
\*===========================================================================*/

#endif

