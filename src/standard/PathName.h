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

#ifndef _PATHNAME_H_
#define _PATHNAME_H_

#include "CString.h"

#ifndef CPATHNAME_DELIMITER
#define CPATHNAME_DELIMITER     '/'
#endif

/*===========================================================================*\
\*===========================================================================*/

class CPathName : public CDynamicString {
public:
    CPathName (void);
    CPathName (tFormatChar *GlobalPathName);
    CPathName (CDynamicString *GlobalPathName);

    tBool SetGlobal (CDynamicString *PathName);
    tBool SetGlobal (tFormatChar *PathName, tSize len = 0);
    tBool SetRelative (CDynamicString *RelPathName);
    tBool SetRelative (tFormatChar *RelPathName, tSize len = 0);

    tUInt UpperPath (tUInt levels = 1);
    // return: the remaining levels (=> 0 always means success)

    void SetDelimiter (tChar delimiter);

private:
    tChar m_Delimiter;
};


/*===========================================================================*\
    CPathName - inline functions
\*===========================================================================*/

inline CPathName::CPathName (void)
  : m_Delimiter (CPATHNAME_DELIMITER) {
}

inline CPathName::CPathName (tFormatChar *GlobalPathName)
  : CDynamicString (GlobalPathName),
    m_Delimiter (CPATHNAME_DELIMITER) {
}

inline CPathName::CPathName (CDynamicString *GlobalPathName)
  : CDynamicString (GlobalPathName),
    m_Delimiter (CPATHNAME_DELIMITER) {
}

inline tBool CPathName::SetGlobal (CDynamicString *PathName) {
    return Set (PathName);
}

inline tBool CPathName::SetGlobal (tFormatChar *PathName, tSize len) {
    return Set (PathName, len);
}

inline tBool CPathName::SetRelative (CDynamicString *RelPathName) {
    //dassert (RelPathName != 0);
    if (IsEmpty() == vFalse) {
        if (GetLastChar() == m_Delimiter) {
            if (RelPathName->GetChar (0) == m_Delimiter) {
                // delete one of both delimiters
                CutLen (1);
            }
        } else if (  (RelPathName->GetChar (0) != m_Delimiter)
                  && (Append (&m_Delimiter, 1) == vFalse)) {
            return vFalse;
        }
    }
    return Append (RelPathName);
}

inline tBool CPathName::SetRelative (tFormatChar *RelPathName, tSize len) {
    //dassert (RelPathName != 0);
    if (IsEmpty() == vFalse) {
        if (GetLastChar() == m_Delimiter) {
            if (*RelPathName == m_Delimiter) {
                // delete one of both delimiters
                CutLen (1);
            }
        } else if (  (*RelPathName != m_Delimiter)
                  && (Append (&m_Delimiter, 1) == vFalse)) {
            return vFalse;
        }
    }
    return Append (RelPathName, len);
}

inline tUInt CPathName::UpperPath (tUInt levels) {
    for (; levels > 0; levels--) {
        tSize pos = FindLastChar (m_Delimiter);
        if (pos == MAXVAL_tUInt) {
           break;
        }
        SetLen (pos);
    }
    return levels;
}

inline void CPathName::SetDelimiter (tChar delimiter) {
    m_Delimiter = delimiter;
}

/*===========================================================================*\
\*===========================================================================*/

#endif

