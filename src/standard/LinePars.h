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

#ifndef _LINEPARS_H_
#define _LINEPARS_H_

#include "MultiStr.h"

/*===========================================================================*\
\*===========================================================================*/

tBool ParseConfigLine (CDynamicString *StrLine, CMultiString *WordList);
// return = vFalse => MemoryFull!
// an item of wordlist is from type CConfigParserString

/*===========================================================================*\
\*===========================================================================*/

class CConfigLineParser {
public:
    virtual ~CConfigLineParser (void) {};

    tBool ParseLine (CDynamicString *StrLine);

    CMultiString *GetWordList (void);

private:
    CMultiString WordList;
};

/*===========================================================================*\
    inline functions
\*===========================================================================*/

inline tBool CConfigLineParser::ParseLine (CDynamicString *StrLine) {
    return ParseConfigLine (StrLine, &WordList);
}

inline CMultiString *CConfigLineParser::GetWordList (void) {
    return &WordList;
}

/*===========================================================================*\
\*===========================================================================*/

#endif

