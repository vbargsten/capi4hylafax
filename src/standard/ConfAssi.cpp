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

#include "ConfAssi.h"
#include "LinePars.h"
#include "dbgSTD.h"
#include <string.h>
#include <stdlib.h>

/*===========================================================================*\
\*===========================================================================*/

void ConfigValuesAssignment (CPointerList *ValueList, ConfigAssignment_Item *ItemList) {
    dhead ("ConfigValuesAssignment", DCON_ConfigParserAssi);
    dassert (ValueList != 0);
    dassert (ItemList != 0);

    ConfigAssignment_Item *lauf;
    for (CConfigParserValue *curValue = (CConfigParserValue *)ValueList->GetFirst();
         curValue != 0; curValue = (CConfigParserValue *)curValue->GetNext()) {

        dassert (curValue->Name.GetPointer() != 0);
        dassert (curValue->DataList.GetFirst() != 0);
        dwarning (((COneMultiString *)curValue->DataList.GetFirst())->GetPointer() != 0);
        for (lauf = ItemList; (lauf != 0) && (lauf->Name != 0); lauf++) {
            if (strcasecmp ((char *)curValue->Name.GetPointer(), lauf->Name) == 0) {
                dprint ("Found %s in ConfigList. ", lauf->Name);
                switch (lauf->Type) {
                case ConfigAssignment_String: {
                    COneMultiString *pOMS = (COneMultiString *)curValue->DataList.GetFirst();
                    if (pOMS) {
                        ((CDynamicString *)lauf->Data)->Set (pOMS);
                        while ((pOMS = pOMS->GetNext()) != 0) {
                            ((CDynamicString *)lauf->Data)->Append (" ", 1);
                            ((CDynamicString *)lauf->Data)->Append (pOMS);
                        }
                    } else {
                        dprint ("Illegal String-Data.\n");
                    }
                    dprint ("String-Data=%s.\n", ((CDynamicString *)lauf->Data)->GetPointer());
                    break;
                }
                case ConfigAssignment_StringList:
                    dprint ("String-ListData\n");
                    for (COneMultiString *sl = (COneMultiString *)curValue->DataList.GetFirst();
                         sl != 0; sl = sl->GetNext()) {
                        COneMultiString *newStr = new COneMultiString;
                        if (newStr) {
                            newStr->Set (sl);
                            ((CPointerList *)lauf->Data)->AddLast (newStr);
                        }
                    }
                    break;
                case ConfigAssignment_UInt:
                    if ((curValue->DataList.GetFirst())->GetPointer() != 0) {
                        char *Last;
                        tUInt val = strtol ((char *)((COneMultiString *)curValue->DataList.GetFirst())->GetPointer(),
                                            &Last, 0);
                        if (*Last == '\0') {
                            dprint ("UInt-Data=%x.\n", val);
                            *((tUInt *)lauf->Data) = val;
                        } else {
                            dprint ("Illegal UInt-Data.\n");
                        }
                    } else {
                        dprint ("Illegal UInt-Data(2).\n");
                    }
                    break;
                }
            }
        }
    }
}

/*===========================================================================*\
\*===========================================================================*/

tUInt ConfigValuesTestName (CPointerList *ValueList, ConfigAssignment_Item *searchItem) {
    dhead ("ConfigValuesTestName", DCON_ConfigParserAssi);
    dassert (ValueList != 0);
    dassert (searchItem != 0);
    tUInt fret = 0;
    for (CConfigParserValue *curValue = (CConfigParserValue *)ValueList->GetFirst();
         curValue != 0; curValue = (CConfigParserValue *)curValue->GetNext()) {

        dassert (curValue->Name.GetPointer() != 0);
        dassert (curValue->DataList.GetFirst() != 0);
        dassert (((COneMultiString *)curValue->DataList.GetFirst())->GetPointer() != 0);
        if (strcasecmp ((char *)curValue->Name.GetPointer(), searchItem->Name) == 0) {
            fret = 1;
            switch (searchItem->Type) {
            case ConfigAssignment_String:
                if (  (searchItem->Data != 0)
                   && (strcasecmp ((char *)((CDynamicString *)searchItem->Data)->GetPointer(),
                                   (char *)((COneMultiString *)curValue->DataList.GetFirst())->GetPointer()) == 0)) {
                    fret = 2;
                }
                break;
            case ConfigAssignment_StringList:
                // TODO: not implememted yet!
                break;
            case ConfigAssignment_UInt:
                {
                    char *Last;
                    tUInt val = strtol ((char *)((COneMultiString *)curValue->DataList.GetFirst())->GetPointer(),
                                        &Last, 0);
                    if ((*Last == '\0') && (searchItem->Data != 0) && (val == *((tUInt *)searchItem->Data))) {
                        fret = 2;
                    }
                    break;
                }
            }
            break;
        }
    }
    RETURN ('x', fret);
}

/*===========================================================================*\
\*===========================================================================*/
