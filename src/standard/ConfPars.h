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

#ifndef _CONFPARS_H_
#define _CONFPARS_H_

#include "LinePars.h"
#include "MultiStr.h"
#include "InfoType.h"

/*---------------------------------------------------------------------------*\
    tSInfo - Values
\*---------------------------------------------------------------------------*/

#define iErr_IniFile_WrongAssignCoding  CalculateInfoValue (infoType_IniFile, infoKind_Error, 1)
#define iErr_IniFile_TwoMuchSectionEnds CalculateInfoValue (infoType_IniFile, infoKind_Error, 2)
#define iErr_IniFile_ParamCombination   CalculateInfoValue (infoType_IniFile, infoKind_Error, 3)

/*===========================================================================*\
\*===========================================================================*/

class CConfigParserValue : public CPointerListElement {
public:
    CDynamicString  Name;
    CMultiString    DataList;
};

class CConfigParserSection : public CPointerListElement {
public:
    CConfigParserSection (void) {
        pUpperSection = 0;
    }
    virtual ~CConfigParserSection (void) {
        pUpperSection = 0;
    }

    CConfigParserSection *pUpperSection;
    CPointerList          SectionList;
    CPointerList          ValueList;
};

/*===========================================================================*\
\*===========================================================================*/

class CConfigGrammarParser : private CConfigLineParser {
public:
    CConfigGrammarParser (CConfigParserSection *pMainSection = 0);
    virtual ~CConfigGrammarParser (void);
    tBool ReinitMainSection (void);

    CConfigParserSection *GetMainSection (void);

    tSInfo ParseLine (CDynamicString *StrLine); // StrLine == 0 => Finish
    tSInfo ParseWords (CMultiString *WordList); // WordList == 0 => Finish

private:
    CConfigParserSection *pMainSec;
    CConfigParserSection *pCurSec;
};

/*===========================================================================*\
\*===========================================================================*/

inline CConfigParserSection *CConfigGrammarParser::GetMainSection (void) {
    return pMainSec;
}

/*===========================================================================*\
\*===========================================================================*/

#endif

