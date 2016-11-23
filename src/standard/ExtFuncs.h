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

#ifndef _EXTFUNCS_H_
#define _EXTFUNCS_H_

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#ifndef NDEBUG

    #define USE_DEBUG_INFO_CLASS        1
    #define DEBUG_INDENT_WIDTH          0
    //#define DEBUG_FILE_NAME             "/var/log/c2faxfcalls.log"

#else

    #define USE_DEBUG_INFO_CLASS        0

#endif

/*---------------------------------------------------------------------------*/

#define NAMEATFUNCSTART_BEGIN           ""
#define NAMEATFUNCSTART_END             "<"

#define NAMEATFUNCEND_BEGIN             ">"
#define NAMEATFUNCEND_END               ""

#define NAMEINEVERYLINE_BEGIN           ""
#define NAMEINEVERYLINE_END             ": "

#define NAMEFORSHORT_BEGIN              ""
#define NAMEFORSHORT_END                " "

#define PARAMETERS_SHORT_BEGIN          "("
#define PARAMETERS_SHORT_END            ")"

#define PARAMETERS_BEGIN                "("
#define PARAMETERS_END                  ")<"

#define RETURNVALUE_BEGIN               "("
#define RETURNVALUE_END                 ")>"


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define DbgIAssert(a)

extern const char *DbgIPrintLinePrefix;
extern unsigned DbgIPrintLinePrefixLen;

extern void DbgIPrintStr (const char *output);

extern void DbgIBreakPoint (void);

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#endif
