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
    CConstString (const tString string, tSize len = 0);   // string have to be null terminated! len ist not tested!
    virtual ~CConstString (void) {}

    tBool   IsEmpty (void) const;
    tSize   GetLen (void) const;      // GetLen & Get(Max)Size return multiples of tStringChar!
    tSize   GetSize (void) const;
    tSize   GetMaxSize (void) const;
    tString GetPointer (void) const;
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
    CDynamicString (const tFormatChar *string, tSize len = 0);
    CDynamicString (const CConstString *pcstr);
    CDynamicString (tUInt startSize);
    virtual ~CDynamicString (void);

    tBool Set (const tString       string, tSize len = 0);
    tBool Set (const tFormatChar  *string, tSize len = 0);
    tBool Set (const CConstString *pcstr);
    tBool Append (const tString       string, tSize len = 0);
    tBool Append (const tFormatChar  *string, tSize len = 0);
    tBool Append (const CConstString *pcstr);

    tBool Change (const tString       string, tSize startPos, tSize len = 0);
    tBool Change (const tFormatChar  *string, tSize startPos, tSize len = 0);
    tBool Change (const CConstString *pcstr,  tSize startPos, tSize len = 0);
    tBool Insert (const tString       string, tSize startPos, tSize len = 0);
    tBool Insert (const tFormatChar  *string, tSize startPos, tSize len = 0);
    tBool Insert (const CConstString *pcstr,  tSize startPos, tSize len = 0);

    tBool Fill (tStringChar fillChar, tSize len, tSize startPos = 0);
    tBool FillAppend (tStringChar fillChar, tUInt len);

    tSInt Print  (const tFormatChar *format, ...);
    tSInt vPrint (const tFormatChar *format, va_list argptr);
    tSInt PrintAppend  (const tFormatChar *format, ...);
    tSInt vPrintAppend (const tFormatChar *format, va_list argptr);

    tCompare Compare (const tString string, tSize len = 0);
    tBool IsIdentical (const tString string, tSize len = 0);

    void Get (tString string, tSize len = 0, tSize startPos = 0);
    tStringChar GetChar (tSize pos);
    tStringChar GetLastChar (void);
    tSize FindChar (const tStringChar ch, tSize startpos = 0); // not found => return = MAXVAL_tSize
    tSize FindLastChar (const tStringChar ch);                 // not found => return = MAXVAL_tSize

    void  SetLen (tSize newLen);
    void  CutLen (tSize cutLen);   // Result: newlen = oldLen - cutLen
    void  Remove (tSize startPos, tSize len);
    void  RemoveChars (tString RemoveChars, tSize startPos = 0, tSize len = 0);
    void  RemoveAll (void);
    tBool Resize (tSize newMaxSize);

private:
    tSInt i_vPrintAppend (const tFormatChar *format, va_list argptr, tUInt tabsize, tStringChar tabchar);
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

inline tBool CConstString::IsEmpty (void) const {
    return (curLen == 0);
}

inline tSize CConstString::GetLen (void) const {
    return curLen;
}

inline tSize CConstString::GetSize (void) const {
    return (GetLen()) ? curLen + 1 : 0;
}

inline tSize CConstString::GetMaxSize (void) const {
    return maxSize;
}

inline tString CConstString::GetPointer (void) const {
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

inline tBool CDynamicString::Set (const tString string, tSize len) {
    RemoveAll();
    return Append (string, len);
}

inline tBool CDynamicString::Set (const tFormatChar *string, tSize len) {
    RemoveAll();
    return Append (string, len);
}

inline tBool CDynamicString::Set (const CConstString *pcstr) {
    if (pcstr->GetLen()) {
        return Set (pcstr->GetPointer(), pcstr->GetLen());
    }
    RemoveAll();
    return vTrue;
}

inline tBool CDynamicString::Append (const tString string, tSize len) {
    return Change (string, GetLen(), len);
}

inline tBool CDynamicString::Append (const tFormatChar *string, tSize len) {
    return Change (string, GetLen(), len);
}

inline tBool CDynamicString::Append (const CConstString *pcstr) {
    return (pcstr->GetLen()) ? Change (pcstr->GetPointer(), GetLen(), pcstr->GetLen()) : vTrue;
}

inline tBool CDynamicString::Change (const CConstString *pcstr, tSize startPos, tSize len) {
    return (pcstr->GetLen()) ? Change (pcstr->GetPointer(), startPos, (len) ? len : pcstr->GetLen()) : vTrue;
}

inline tBool CDynamicString::Insert (const CConstString *pcstr, tSize startPos, tSize len) {
    return (pcstr->GetLen()) ? Insert (pcstr->GetPointer(), startPos, (len) ? len : pcstr->GetLen()) : vTrue;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CDynamicString::FillAppend (const tStringChar fillChar, tUInt len) {
    return Fill (fillChar, len, GetLen());
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tSInt CDynamicString::Print (const tFormatChar *format, ...) {
    va_list arg_ptr;
    va_start (arg_ptr, format);
    tUInt fret = vPrint (format, arg_ptr);
    va_end (arg_ptr);
    return fret;
}

inline tSInt CDynamicString::vPrint (const tFormatChar *format, va_list argptr) {
    RemoveAll();
    return vPrintAppend (format, argptr);
}

inline tSInt CDynamicString::PrintAppend (const tFormatChar *format, ...) {
    va_list arg_ptr;
    va_start (arg_ptr, format);
    tUInt fret = vPrintAppend (format, arg_ptr);
    va_end (arg_ptr);
    return fret;
}

inline tSInt CDynamicString::vPrintAppend (const tFormatChar *format, va_list argptr) {
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
    int len = GetLen();
    
    if (len > 0) {
        return pntr[GetLen() - 1];
    } else {
        return '\0';
    }
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

