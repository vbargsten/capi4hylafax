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

#include "LinePars.h"
#include "aString.h"
#include "dbgSTD.h"

/*===========================================================================*\
\*===========================================================================*/

tBool ParseConfigLine (CDynamicString *StrLine, CMultiString *WordList) {
    dhead ("ParseConfigLine", DCON_ConfigParser);
    dassert (StrLine != 0);
    dassert (WordList != 0);
    COneMultiString *newWord;
    tString          startPos;
    tUInt            rest     = StrLine->GetLen();
    tString          lauf     = StrLine->GetPointer();
    dassert ((lauf == 0) || (rest != 0));
    do {
        for (; rest > 0; rest--, lauf++) {
            dassert (*lauf != '\0');
            if ((*lauf != ' ') && (*lauf != '\t')) {
                break;
            }
        }
        if (rest) {
            rest--;
            startPos = lauf;
            switch (*lauf++) {
            default:
                for (; rest > 0; rest--, lauf++) {
                    dassert (*lauf != '\0');
                    if ((s_findfirstchar (*lauf, (tUChar *)" \t=:,-[{}]") != 0) && (*(lauf - 1) != '\\')) {
                        break;
                    }
                }
                break;
            case '"':
                for (; rest > 0; rest--, lauf++) {
                    dassert (*lauf != '\0');
                    if ((*lauf == '"') && (*(lauf - 1) != '\\')) {
                        // the last " must included to this word
                        lauf++;
                        rest--;
                        break;
                    }
                }
                break;
            case '[':
                //for (; rest > 0; rest--, lauf++) {
                //    dassert (*lauf != '\0');
                //    if ((*lauf == ']') && (*(lauf - 1) != '\\')) {
                //        break;
                //    }
                //}
                //break;
            case ']':
            case '{':
            case '}':
            case '=':
            case ':':
            case ',':
            case '-':
            //case ';':
                break;
            case '#':
                if (rest) {
                    lauf += rest;
                    rest = 0;
                }
                break;
            }
            newWord = new COneMultiString;
            if (!newWord || (newWord->Set (startPos, lauf - startPos) == vFalse)) {
                delete newWord;
                return vFalse;
            }
            dprint ("AddWord=%s(%x)\n", newWord->GetPointer(), newWord->GetLen());
            WordList->AddLast (newWord);
        }
    } while (rest > 0);
    return vTrue;
}

/*===========================================================================*\
\*===========================================================================*/
