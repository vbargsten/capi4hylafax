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

#include "aString.h"
#include "aStdLib.h"
#include "dbgSTD.h"

/*===========================================================================*\
\*===========================================================================*/

void s_strmov (tChar *string, tOffset offset, tSize len) {
    dhead ("s_strmov", DCON_StringFuncs);
    tChar *dest;
    tSInt   direct;
    if (offset < 0) {
        direct  = 1;
    } else {
        direct  = -1;
        string += len - 1;
    }
    dest = string + offset;

    for (; len > 0; len--) {
        *dest   = *string;
        dest   += direct;
        string += direct;
    }
}

/*===========================================================================*\
\*===========================================================================*/

void s_strmov (tWiChar *string, tOffset offset, tSize len) {
    dhead ("s_strmovW", DCON_StringFuncs);
    tWiChar *dest;
    tSInt     direct;
    if (offset < 0) {
        direct  = 1;
    } else {
        direct  = -1;
        string += len - 1;
    }
    dest = string + offset;

    for (; len > 0; len--) {
        *dest   = *string;
        dest   += direct;
        string += direct;
    }
}

/*===========================================================================*\
\*===========================================================================*/

tUInt s_strclean (tChar *string, tChar *CleanChars) {
    dhead ("s_strclean", DCON_StringFuncs);
    dparams ("'%s'(%p),'%s'", string, string, CleanChars);
    dassert (string != 0);
    dassert (CleanChars != 0);
    tUInt count = 0;
    if (*string != '\0') {
        dprint (".%c.", *string);
        while (s_findfirstchar (*string, CleanChars) == 0) {
            string++;
            count++;
            dprint (".%c.", *string);
            if (*string == '\0') {
                dprint (" Nothing2Do! ");
                return count;
            }
        }
        dprint ("cleaning at '%s'(%p). ", string, string);
        for (tChar *current = string + 1; *current != '\0'; current++) {
            dprint (".%c.", *current);
            if (s_findfirstchar (*current, CleanChars) == 0) {
                dprint ("-");
                *string++ = *current;
                count++;
            }
        }
        dprint ("finished at '%s'(%p). ", string, string);
        *string = '\0';
    }
    return count;
}

/*===========================================================================*\
\*===========================================================================*/

tUInt s_strkeep (tChar *string, tChar *KeepChars) {
    dhead ("s_strkeep", DCON_StringFuncs);
    dparams ("'%s'(%p),'%s'", string, string, KeepChars);
    dassert (string != 0);
    dassert (KeepChars != 0);
    tUInt count = 0;
    if (*string != '\0') {
        dprint (".%c.", *string);
        while (s_findfirstchar (*string, KeepChars) != 0) {
            string++;
            count++;
            dprint (".%c.", *string);
            if (*string == '\0') {
                dprint (" Nothing2Do! ");
                return count;
            }
        }
        dprint ("starting at '%s'(%p). ", string, string);
        for (tChar *current = string + 1; *current != '\0'; current++) {
            dprint (".%c.", *current);
            if (s_findfirstchar (*current, KeepChars) != 0) {
                dprint ("-");
                *string++ = *current;
                count++;
            }
        }
        dprint ("finished at '%s'(%p). ", string, string);
        *string = '\0';
    }
    return count;
}

/*===========================================================================*\
\*===========================================================================*/

tBool s_strdup (tChar **pDest, const tChar *src, tSize len) {
    dhead ("s_strdup", DCON_StringFuncs);
    dassert (pDest != 0);
    tChar *tmpStr;
    if (*pDest) {
        tmpStr = *pDest;
        *pDest = 0;
        delete [] tmpStr;
    }
    if (src) {
        if (!len) {
            len = s_strlen (src);
        }
        tmpStr = new tChar [len + 1];
        if (!tmpStr) {
            return vFalse;
        }
        s_strncpy (tmpStr, src, len);
        tmpStr[len] = '\0';
        *pDest = tmpStr;
    }
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

tBool s_strdup (tChar **pDest, const tWiChar *src, tSize len) {
    dhead ("s_strdup_W", DCON_StringFuncs);
    dassert (pDest != 0);
    tChar *tmpStr;
    if (*pDest) {
        tmpStr = *pDest;
        *pDest = 0;
        delete [] tmpStr;
    }
    if (src) {
        if (!len) {
            len = s_strlen (src);
        }
        tmpStr = new tChar [len + 1];
        if (!tmpStr) {
            return vFalse;
        }
        s_strncpy (tmpStr, src, len);
        tmpStr[len] = '\0';
        *pDest = tmpStr;
    }
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

tBool s_strdup (tWiChar **pDest, const tChar *src, tSize len) {
    dhead ("s_strdupW", DCON_StringFuncs);
    dassert (pDest != 0);
    tWiChar *tmpStr;
    if (*pDest) {
        tmpStr = *pDest;
        *pDest = 0;
        delete [] tmpStr;
    }
    if (src) {
        if (!len) {
            len = s_strlen (src);
        }
        tmpStr = new tWiChar [len + 1];
        if (!tmpStr) {
            return vFalse;
        }
        s_strncpy (tmpStr, src, len);
        tmpStr[len] = '\0';
        *pDest = tmpStr;
    }
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

tBool s_strdup (tWiChar **pDest, const tWiChar *src, tSize len) {
    dhead ("s_strdupWW", DCON_StringFuncs);
    dassert (pDest != 0);
    tWiChar *tmpStr;
    if (*pDest) {
        tmpStr = *pDest;
        *pDest = 0;
        delete [] tmpStr;
    }
    if (src) {
        if (!len) {
            len = s_strlen (src);
        }
        tmpStr = new tWiChar [len + 1];
        if (!tmpStr) {
            return vFalse;
        }
        s_strncpy (tmpStr, src, len);
        tmpStr[len] = '\0';
        *pDest = tmpStr;
    }
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

tChar *s_findstring (tChar *fullString, tSize fullLen, tChar *search, tSize searchLen) {
    dhead ("s_findstring", DCON_StringFuncs);
    dassert (fullString != 0);
    dassert (fullLen > 0);
    dassert (search != 0);
    dassert (searchLen > 0);
    if (fullLen >= searchLen) {
        searchLen--;
        for (fullLen -= searchLen; fullLen > 0; fullLen++, fullString++) {
            if (  (*fullString == *search)
               && (  (searchLen == 0)
                  || (s_strncmp (fullString + 1, search + 1, searchLen) == vEqual))) {
                RETURN ('x', fullString);
            }
        }
    }
    RETURN ('x', 0);
}

/*===========================================================================*\
\*===========================================================================*/

tCompare s_strcmp (const tChar *str1, const tWiChar *str2) {
    dhead ("s_strcmp", DCON_StringFuncs);
    dassert (str1 != 0);
    dassert (str2 != 0);
    tWiChar wchr1;
    s_charcpy (&wchr1, *str1);
    while (*str1 && *str2 && (wchr1 == *str2)) {
        str1++;
        str2++;
        s_charcpy (&wchr1, *str1);
    }
    // assert (TYPE tCompare == TYPE tSign)!
    return (tCompare)a_getsign (((tSLong)*str1) - ((tSLong)*str2));
}

/*===========================================================================*\
\*===========================================================================*/

tCompare s_strcmp (const tWiChar *str1, const tChar *str2) {
    dhead ("s_strcmpW", DCON_StringFuncs);
    dassert (str1 != 0);
    dassert (str2 != 0);
    tWiChar wchr2;
    s_charcpy (&wchr2, *str2);
    while (*str1 && *str2 && (*str1 == wchr2)) {
        str1++;
        str2++;
        s_charcpy (&wchr2, *str2);
    }
    // assert (TYPE tCompare == TYPE tSign)!
    return (tCompare)a_getsign (((tSLong)*str1) - ((tSLong)*str2));
}

/*===========================================================================*\
\*===========================================================================*/

tCompare s_strncmp (const tChar *str1, const tWiChar *str2, tSize len) {
    dhead ("s_strncmp", DCON_StringFuncs);
    dassert (str1 != 0);
    dassert (str2 != 0);
    dassert (len > 0);
    tWiChar wchr1;
    s_charcpy (&wchr1, *str1);
    while (*str1 && *str2 && (wchr1 == *str2)) {
        len--;
        if (!len) {
            return vEqual;
        }
        str1++;
        str2++;
        s_charcpy (&wchr1, *str1);
    }
    // assert (TYPE tCompare == TYPE tSign)!
    return (tCompare)a_getsign (((tSLong)*str1) - ((tSLong)*str2));
}

/*===========================================================================*\
\*===========================================================================*/

tCompare s_strncmp (const tWiChar *str1, const tChar *str2, tSize len) {
    dhead ("s_strncmpW", DCON_StringFuncs);
    dassert (str1 != 0);
    dassert (str2 != 0);
    dassert (len > 0);
    tWiChar wchr2;
    s_charcpy (&wchr2, *str2);
    while (*str1 && *str2 && (*str1 == wchr2)) {
        len--;
        if (!len) {
            return vEqual;
        }
        str1++;
        str2++;
        s_charcpy (&wchr2, *str2);
    }
    // assert (TYPE tCompare == TYPE tSign)!
    return (tCompare)a_getsign (((tSLong)*str1) - ((tSLong)*str2));
}

/*===========================================================================*\
\*===========================================================================*/
