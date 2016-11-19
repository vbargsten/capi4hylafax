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

#ifndef _ASTRING_H_
#define _ASTRING_H_

#include "aTypes.h"
#include "aStdLib.h"
#include <string.h>

#ifdef __linux__

#define _strnicmp       strncasecmp
#define _wcsnicmp       wcsncasecmp

#endif

/*===========================================================================*\
    String length and size
\*===========================================================================*/

tBool s_strempty (const tChar   *string);    // without (string == 0) check
tBool s_strempty (const tWiChar *string);
tBool s_strempty (const char    *string);

tBool s_strnull (const tChar   *string);    // strempty with (string == 0) check
tBool s_strnull (const tWiChar *string);
tBool s_strnull (const char    *string);

tSize s_strlen (const tChar   *string);
tSize s_strlen (const tWiChar *string);
tSize s_strlen (const char    *string);

tSize s_strsize (const tChar   *string);     // with (string == 0) check
tSize s_strsize (const tWiChar *string);
tSize s_strsize (const char    *string);


/*===========================================================================*\
    Copy
\*===========================================================================*/

void s_strcpy (tChar   *dest, const tChar   *src);
void s_strcpy (tChar   *dest, const tWiChar *src);
void s_strcpy (tChar   *dest, const char    *src);
void s_strcpy (tWiChar *dest, const tChar   *src);
void s_strcpy (tWiChar *dest, const tWiChar *src);
void s_strcpy (tWiChar *dest, const char    *src);
void s_strcpy (char    *dest, const tChar   *src);
void s_strcpy (char    *dest, const tWiChar *src);
void s_strcpy (char    *dest, const char    *src);


/*===========================================================================*\
    N-Copy
\*===========================================================================*/

void s_strncpy (tChar   *dest, const tChar   *src, tSize len);
void s_strncpy (tChar   *dest, const tWiChar *src, tSize len);
void s_strncpy (tChar   *dest, const char    *src, tSize len);
void s_strncpy (tWiChar *dest, const tChar   *src, tSize len);
void s_strncpy (tWiChar *dest, const tWiChar *src, tSize len);
void s_strncpy (tWiChar *dest, const char    *src, tSize len);
void s_strncpy (char    *dest, const tChar   *src, tSize len);
void s_strncpy (char    *dest, const tWiChar *src, tSize len);
void s_strncpy (char    *dest, const char    *src, tSize len);

void s_memcpy (void *dest, void *src, tSize len);


/*===========================================================================*\
    1-Copy
\*===========================================================================*/

void s_charcpy (tChar *dest,   tChar   src);
void s_charcpy (tChar *dest,   tWiChar src);
void s_charcpy (tChar *dest,   char    src);
void s_charcpy (tWiChar *dest, tChar   src);
void s_charcpy (tWiChar *dest, tWiChar src);
void s_charcpy (tWiChar *dest, char    src);


/*===========================================================================*\
    Move
\*===========================================================================*/

void s_strmov (tChar   *string, tOffset offset, tSize len);
void s_strmov (tWiChar *string, tOffset offset, tSize len);

void s_memmov (void *pointer, tOffset offset, tSize len);


/*===========================================================================*\
    Set
\*===========================================================================*/

void s_strset (tChar   *string, tChar   setch, tSize len);
void s_strset (tWiChar *string, tWiChar setch, tSize len);

void s_memset (void *dest, tUByte setby, tSize len);
void s_memzero (void *dest, tSize len);


/*===========================================================================*\
    Clean       (return the new strlen)
\*===========================================================================*/

tUInt s_strclean (tChar *string, tChar *CleanChars);
tUInt s_strkeep  (tChar *string, tChar *KeepChars);


/*===========================================================================*\
    Duplicate
\*===========================================================================*/

tBool s_strdup (tChar   **pDest, const tChar   *src, tSize len = 0);
tBool s_strdup (tChar   **pDest, const tWiChar *src, tSize len = 0);
tBool s_strdup (tWiChar **pDest, const tChar   *src, tSize len = 0);
tBool s_strdup (tWiChar **pDest, const tWiChar *src, tSize len = 0);

void s_strdel (tChar   **pString);
void s_strdel (tWiChar **pString);

/*===========================================================================*\
    Find
\*===========================================================================*/

tChar   *s_findfirstchar (tChar   chSearch, tChar   *string);
tWiChar *s_findfirstchar (tWiChar chSearch, tWiChar *string);
tChar   *s_findfirstchar (tChar   chSearch, tChar   *string, tSize len);
tWiChar *s_findfirstchar (tWiChar chSearch, tWiChar *string, tSize len);

tChar *s_findstring (tChar *fullString, tSize fullLen, tChar *search, tSize searchLen);


/*===========================================================================*\
    Compare
\*===========================================================================*/

tCompare s_strcmp (const tChar   *str1, const tChar   *str2);
tCompare s_strcmp (const tChar   *str1, const tWiChar *str2);
tCompare s_strcmp (const tWiChar *str1, const tChar   *str2);
tCompare s_strcmp (const tWiChar *str1, const tWiChar *str2);

tCompare s_strncmp (const tChar   *str1, const tChar   *str2, tSize len);
tCompare s_strncmp (const tChar   *str1, const tWiChar *str2, tSize len);
tCompare s_strncmp (const tWiChar *str1, const tChar   *str2, tSize len);
tCompare s_strncmp (const tWiChar *str1, const tWiChar *str2, tSize len);

tCompare s_strnicmp (const tChar   *str1, const tChar   *str2, tSize len);
tCompare s_strnicmp (const tWiChar *str1, const tWiChar *str2, tSize len);

tCompare s_memcmp (const tUByte *mem1, const tUByte *mem2, tSize len);


/*===========================================================================*\
    inline Funktionen
\*===========================================================================*/

inline tBool s_strempty (const tChar *string) {
    return (*string != 0) ? vFalse : vTrue;
}

inline tBool s_strempty (const tWiChar *string) {
    return (*string != 0) ? vFalse : vTrue;
}

inline tBool s_strempty (const char *string) {
    return (*string != 0) ? vFalse : vTrue;
}

inline tBool s_strnull (const tChar *string) {
    return !string || s_strempty (string);
}

inline tBool s_strnull (const tWiChar *string) {
    return !string || s_strempty (string);
}

inline tBool s_strnull (const char *string) {
    return !string || s_strempty (string);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tSize s_strlen (const tChar *string) {
    return strlen ((const char *)string);
}

inline tSize s_strlen (const tWiChar *string) {
    return wcslen (string);
}

inline tSize s_strlen (const char *string) {
    return strlen (string);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tSize s_strsize (const tChar *string) {
    return (string) ? (s_strlen (string) + 1) : 0;
}

inline tSize s_strsize (const tWiChar *string) {
    return (string) ? (s_strlen (string) + 1) * sizeof (tWiChar) : 0;
}

inline tSize s_strsize (const char *string) {
    return (string) ? (s_strlen (string) + 1) : 0;
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void s_strcpy (tChar *dest, const tChar *src) {
    strcpy ((char *)dest, (char *)src);
}

inline void s_strcpy (tChar *dest, const tWiChar *src) {
    while (*src != 0) {
        s_charcpy (dest++, *src++);
    }
    *dest = '\0';
}

inline void s_strcpy (tChar *dest, const char *src) {
    s_strcpy (dest, (tChar *)src);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void s_strcpy (tWiChar *dest, const tChar *src) {
    while (*src != 0) {
        s_charcpy (dest++, *src++);
    }
    *dest = '\0';
}

inline void s_strcpy (tWiChar *dest, const tWiChar *src) {
    wcscpy (dest, src);
}

inline void s_strcpy (tWiChar *dest, const char *src) {
    s_strcpy (dest, (tChar *)src);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void s_strcpy (char *dest, const tChar *src) {
    s_strcpy ((tChar *)dest, src);
}

inline void s_strcpy (char *dest, const tWiChar *src) {
    s_strcpy ((tChar *)dest, src);
}

inline void s_strcpy (char *dest, const char *src) {
    s_strcpy ((tChar *)dest, src);
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void s_strncpy (tChar *dest, const tChar *src, tSize len) {
    memcpy (dest, src, len);
}

inline void s_strncpy (tChar *dest, const tWiChar *src, tSize len) {
    for (; len > 0; len--) {
        s_charcpy (dest++, *src++);
    }
}

inline void s_strncpy (tChar *dest, const char *src, tSize len) {
    s_strncpy (dest, (tChar *)src, len);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void s_strncpy (tWiChar *dest, const tChar *src, tSize len) {
    for (; len > 0; len--) {
        s_charcpy (dest++, *src++);
    }
}

inline void s_strncpy (tWiChar *dest, const tWiChar *src, tSize len) {
    memcpy (dest, src, len * sizeof (tWiChar));
}

inline void s_strncpy (tWiChar *dest, const char *src, tSize len) {
    s_strncpy (dest, (tChar *)src, len);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void s_strncpy (char *dest, const tChar *src, tSize len) {
    s_strncpy ((tChar *)dest, src, len);
}

inline void s_strncpy (char *dest, const tWiChar *src, tSize len) {
    s_strncpy ((tChar *)dest, src, len);
}

inline void s_strncpy (char *dest, const char *src, tSize len) {
    s_strncpy ((tChar *)dest, src, len);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void s_charcpy (tChar *dest, tChar src) {
    *dest = src;
}

inline void s_charcpy (tChar *dest, tWiChar src) {
    *dest = (tChar)src;
}

inline void s_charcpy (tChar *dest, char src) {
    *dest = (tChar)src;
}

inline void s_charcpy (tWiChar *dest, tChar src) {
    *dest = (tWiChar)src;
}

inline void s_charcpy (tWiChar *dest, tWiChar src) {
    *dest = (tWiChar)src;
}

inline void s_charcpy (tWiChar *dest, char src) {
    *dest = (tWiChar)src;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void s_strset (tChar *string, tChar setch, tSize len) {
    memset (string, setch, len);
}

inline void s_strset (tWiChar *string, tWiChar setch, tSize len) {
    for (; len > 0; len--) {
        *string++ = setch;
    }
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void s_memcpy (void *dest, void *src, tSize len) {
    memcpy (dest, src, len);
}

inline void s_memmov (void *dest, tOffset offset, tSize len) {
    s_strmov ((tUByte *)dest, offset, len);
}

inline void s_memset (void *dest, tUByte setby, tSize len) {
    memset (dest, setby, len);
}

inline void s_memzero (void *dest, tSize len) {
    s_memset (dest, '\0', len);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline void s_strdel (tChar **pString) {
    if (*pString) {
        tChar *tmpStr = *pString;
        *pString = 0;
        delete [] tmpStr;
    }
}

inline void s_strdel (tWiChar **pString) {
    if (*pString) {
        tWiChar *tmpStr = *pString;
        *pString = 0;
        delete [] tmpStr;
    }
}

inline tChar *s_findfirstchar (tChar chSearch, tChar *string) {
    return (tChar *)strchr ((char *)string, chSearch);
}

inline tWiChar *s_findfirstchar (tWiChar chSearch, tWiChar *string) {
    return wcschr (string, chSearch);
}

inline tChar *s_findfirstchar (tChar chSearch, tChar *string, tSize len) {
    for (; len > 0; len--, string++) {
        if (chSearch == *string) {
            return string;
        }
    }
    return 0;
}

inline tWiChar *s_findfirstchar (tWiChar chSearch, tWiChar *string, tSize len) {
    for (; len > 0; len--, string++) {
        if (chSearch == *string) {
            return string;
        }
    }
    return 0;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tCompare s_strcmp (const tChar *str1, const tChar *str2) {
    // assert (tCompare == tSign)!
    return (tCompare)a_getsign (strcmp ((const char *)str1, (const char *)str2));
}

inline tCompare s_strcmp (const tWiChar *str1, const tWiChar *str2) {
    // assert (tCompare == tSign)!
    return (tCompare)a_getsign (wcscmp (str1, str2));
}

inline tCompare s_strncmp (const tChar *str1, const tChar *str2, tSize len) {
    // assert (tCompare == tSign)!
    return (tCompare)a_getsign (strncmp ((const char *)str1, (const char *)str2, len));
}

inline tCompare s_strncmp (const tWiChar *str1, const tWiChar *str2, tSize len) {
    // assert (tCompare == tSign)!
    return (tCompare)a_getsign (wcsncmp (str1, str2, len));
}

inline tCompare s_strnicmp (const tChar *str1, const tChar *str2, tSize len) {
    // assert (tCompare == tSign)!
    return (tCompare)a_getsign (_strnicmp ((const char *)str1, (const char *)str2, len));
}

inline tCompare s_strnicmp (const tWiChar *str1, const tWiChar *str2, tSize len) {
    // assert (tCompare == tSign)!
    return (tCompare)a_getsign (_wcsnicmp (str1, str2, len));
}

inline tCompare s_memcmp (const tUByte *mem1, const tUByte *mem2, tSize len) {
    // assert (tCompare == tSign)!
    return (tCompare)a_getsign (memcmp (mem1, mem2, len));
    //for (; len > 0; len--, mem1++, mem2++) {
    //    if (*mem1 != *mem2) {
    //        return (*mem1 < *mem2) ? vLower : vHigher;
    //    }
    //}
    //return vEqual;
}

/*===========================================================================*\
\*===========================================================================*/

#endif
