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

#ifndef _CONFPARS_CPP_
#define _CONFPARS_CPP_

#include "ConfPars.h"
#include "dbgSTD.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

enum GrammarParseState {
    ps_SearchName,
    ps_SearchAssign,
    ps_SearchData,
    ps_SearchEnd
};

/*===========================================================================*\
\*===========================================================================*/

CConfigGrammarParser::CConfigGrammarParser (CConfigParserSection *pMainSection) {
    dhead ("CConfigGrammarParser-Constructor", DCON_ConfigParser);
    if (!pMainSection) {
        pMainSection = new CConfigParserSection;
    }
    pCurSec = pMainSec = pMainSection;
}

/*===========================================================================*\
\*===========================================================================*/

CConfigGrammarParser::~CConfigGrammarParser (void) {
    dhead ("CConfigGrammarParser-Destructor", DCON_ConfigParser);
    // TODO?: delete is not always the best idea here
    delete pMainSec;
    pMainSec = 0;
}

/*===========================================================================*\
\*===========================================================================*/

tBool CConfigGrammarParser::ReinitMainSection (void) {
    dhead ("CConfigGrammarParser::ReinitMainSection", DCON_ConfigParser);
    delete pMainSec;
    pMainSec = new CConfigParserSection;
    pCurSec = pMainSec;
    return (pMainSec != 0);
}

/*===========================================================================*\
\*===========================================================================*/

tSInfo CConfigGrammarParser::ParseLine (CDynamicString *StrLine) {
    dhead ("CConfigGrammarParser::ParseLine", DCON_ConfigParser);
    tSInfo fret = i_Okay;
    if ((StrLine) && (CConfigLineParser::ParseLine (StrLine) == vFalse)) {
        return iErr_OutOfMemory;
    }
    CMultiString *wordList = GetWordList();
    dwarning (wordList != 0);
    if (wordList) {
        fret = ParseWords (wordList);
        if ((fret != i_Pending) || (!StrLine)) {
            wordList->RemoveAndDeleteAll();
        }
    }
    return fret;
}

/*===========================================================================*\
\*===========================================================================*/

tSInfo CConfigGrammarParser::ParseWords (CMultiString *WordList) {
    dhead ("CConfigGrammarParser::ParseWordsX", DCON_ConfigParser);
    dassert (WordList != 0);
    COneMultiString *pPStr;

    // Test for line concernator '\\'
    pPStr = WordList->GetLast();
    if (!pPStr) {
        dprint (" WordList is EMPTY ");
        RETURN ('x', i_Okay);
    }
    dassert (pPStr->GetLen() > 0);
    while (pPStr->GetChar (0) == '#') {
        pPStr = pPStr->GetPrev();
        if (!pPStr) {
            RETURN ('x', i_Okay);
        }
    }
    if ((pPStr->GetLen() == 1) && (pPStr->GetChar (0) == '\\')) {
        // found line concernator and all later comments
        COneMultiString *delPStr;
        while (pPStr) {
            delPStr = pPStr;
            pPStr   = pPStr->GetNext();
            delete delPStr;
        }
        RETURN ('x', i_Pending);
    }

    // parse words
    tBool               InRange    = vFalse;
    CConfigParserValue *pCurValue  = 0;
    GrammarParseState   ParseState = ps_SearchName;

    // parse current string list
    while ((pPStr = WordList->GetFirst()) != 0) {
        dassert (pPStr->GetLen() > 0);
        dassert (pCurSec != 0);
        //dprint ("CurWord=%s ", pPStr->GetPointer());
        switch (pPStr->GetChar (0)) {
        case '{': {
            dprint ("Found: Section OPEN\n");
            dassert (pPStr->GetLen() == 1);
            if (pCurValue) {
                if (pCurValue->DataList.IsEmpty() == vFalse) {
                    pCurSec->ValueList.AddLast (pCurValue);
                } else  {
                    delete pCurValue;
                }
                pCurValue = 0;
            }
            CConfigParserSection *newSec = new CConfigParserSection;
            if (!newSec) {
                return iErr_OutOfMemory;
            }
            newSec->pUpperSection = pCurSec;
            pCurSec->SectionList.AddLast (newSec);
            pCurSec = newSec;
            InRange = vFalse;
            break;
        }
        case '}':
            dprint ("Found: Section CLOSE\n");
            dassert (pPStr->GetLen() == 1);
            if (pCurValue) {
                if (pCurValue->DataList.IsEmpty() == vFalse) {
                    pCurSec->ValueList.AddLast (pCurValue);
                } else  {
                    delete pCurValue;
                }
                pCurValue = 0;
            }
            if (pCurSec->pUpperSection == 0) {
                return iErr_IniFile_TwoMuchSectionEnds;
            }
            pCurSec = pCurSec->pUpperSection;
            InRange = vFalse;
            break;

        case '"':
            // Remove "
            if ((pPStr->GetLen() < 2) || (pPStr->GetLastChar() != '"')) {
                // do nothing !!!!

                //delete pCurValue;
                //return i_Err_QuotationError;
            } else {
                pPStr->CutLen (1);
                pPStr->Remove (0, 1);
            }
            // TODO?: Remove \ from \" ?
            // no break!

        default:
            if (!pCurValue) {
                pCurValue = new CConfigParserValue;
                if (!pCurValue) {
                    delete pCurValue;
                    return iErr_OutOfMemory;
                }
                dprint ("Name=%s ", pPStr->GetPointer());
                pCurValue->Name.Set (pPStr);
                ParseState = ps_SearchAssign;
            } else {
                switch (ParseState) {
                case ps_SearchName:
                case ps_SearchAssign:
                case ps_SearchEnd:
                    ParseState = ps_SearchName;
                    delete pCurValue;
                    pCurValue = 0;
                    return iErr_IniFile_WrongAssignCoding;
                case ps_SearchData:
                    if (InRange == vTrue) {
                        COneMultiString *pLast = pCurValue->DataList.GetLast();
                        dassert (pLast != 0);
                        pLast->Append (pPStr);
                        InRange = vFalse;
                    } else {
                        dprint ("Data=%s ", pPStr->GetPointer());
                        pPStr->RemoveFromList();
                        pCurValue->DataList.AddLast (pPStr);
                        pPStr = 0;
                    }
                    break;
                }
            }
            break;

        case '=':
        case ':':
            dassert (pPStr->GetLen() == 1);
            InRange = vFalse;
            switch (ParseState) {
            case ps_SearchData:
                dwarning (0);
                break;
            case ps_SearchAssign:
                ParseState = ps_SearchData;
                break;
            default:
                delete pCurValue;
                return iErr_IniFile_WrongAssignCoding;
            }
            break;

        case ',':
            if (ParseState != ps_SearchData) {
                delete pCurValue;
                return iErr_IniFile_WrongAssignCoding;
            }
            InRange = vFalse;
            break;

        case '-':
            // Range found
            if (ParseState != ps_SearchData) {
                delete pCurValue;
                return iErr_IniFile_WrongAssignCoding;
            }
            if (InRange == vFalse) {
                InRange = vTrue;
                if (pCurValue->DataList.IsEmpty()) {
                    pPStr->RemoveFromList();
                    pCurValue->DataList.AddLast (pPStr);
                    pPStr = 0;
                } else {
                    COneMultiString *pLast = pCurValue->DataList.GetLast();
                    dassert (pLast != 0);
                    pLast->Append ("-", 1);
                }
            }
            break;
        //case ';':
        //    if (pCurValue) {
        //        if (pCurValue->DataList.IsEmpty() == vFalse) {
        //            pCurSec->ValueList.AddLast (pCurValue);
        //        } else  {
        //            delete pCurValue;
        //        }
        //        pCurValue = 0;
        //    }
        //    ParseState = ps_SearchName;
        //    break;

        case '#':
            InRange = vFalse;
            break;
        }
        delete pPStr;
    }
    if (pCurValue) {
        if (pCurValue->DataList.IsEmpty() == vFalse) {
            pCurSec->ValueList.AddLast (pCurValue);
        } else  {
            delete pCurValue;
        }
    }
    RETURN ('x', i_Okay);
}


/*===========================================================================*\
\*===========================================================================*/

#endif

