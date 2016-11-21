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

#include "MSNList.h"
#include "dbgCAPI.h"
#include <stdio.h>
/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class CControllerInfo : public CSortPListElement {
public:
    CControllerInfo (void);
    virtual ~CControllerInfo (void);

    void DelAllMSNs (void);
    tCompare Compare (void *RefCompData);
    void *GetRefCompData (void);

    tUInt          Controller;
    tUInt          InfoMask;
    tUInt          CIPMask;
    tUInt          DDILen;
    CDynamicString TelNumPrefix;
    CMultiString   MSNList;
    tBool          PMSupportable;
};


/*===========================================================================*\
\*===========================================================================*/

static tCompare ArrayFindFunc_NumComp (CConstString *Num1, CConstString *Num2) {
    dhead ("ArrayFindFunc_NumComp", DCON_CCntrlMSNList);
    dassert (Num1 != 0);
    dassert (Num2 != 0);
    dparams ("%s,%s", Num1->GetPointer(), Num2->GetPointer());
    dassert (Num1->IsEmpty() == vFalse);
    tCompare fret = vHigher;
    if (Num2->IsEmpty() == vFalse) {
        fret = a_compare (Num1->GetLen(), Num2->GetLen());
        if (fret == vEqual) {
            fret = s_strncmp (Num1->GetPointer(), Num2->GetPointer(), Num1->GetLen());
        }
    }
    RETURN ('x', fret);
}

static tCompare ArrayFindFunc_MSNComp (CConstString *Num1, CCStruct *Num2) {
    dhead ("ArrayFindFunc_MSNComp", DCON_CCntrlMSNList);
    dassert (Num1 != 0);
    dassert (Num2 != 0);
    dassert (Num1->IsEmpty() == vFalse);
    dassert (Num2->GetLen() > 1);
    tUInt len = a_min (Num1->GetLen(), Num2->GetLen() - 1);
    RETURN ('x', s_strncmp (Num1->GetPointer() + Num1->GetLen() - len, Num2->GetData() + Num2->GetLen() - len, len));
}

static tCompare ArrayFindFunc_DDIComp (CConstString *Num1, CConstString *Num2) {
    dhead ("ArrayFindFunc_DDIComp", DCON_CCntrlMSNList);
    dassert (Num1 != 0);
    dassert (Num1->IsEmpty() == vFalse);
    dassert (Num2 != 0);
    dassert (Num2->IsEmpty() == vFalse);
    tCompare fret = a_compare (Num1->GetLen(), Num2->GetLen());
    if (fret != vHigher) {
        fret = s_strncmp (Num1->GetPointer(), Num2->GetPointer(), Num1->GetLen());
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

static tSInfo Alloc_CntrlInfo_MSNRange (CMSNRange **ppRange, CConstString *aMSN, CConstString *bMSN) {
    dhead ("Alloc_CntrlInfo_MSNRange", DCON_CCntrlMSNList);
    dassert (ppRange != 0);
    dassert (aMSN != 0);
    dassert (aMSN->IsEmpty() == vFalse);
    tSInfo fret = iErr_OutOfMemory;
    *ppRange    = new CMSNRange;
    if (*ppRange && ((*ppRange)->Set (aMSN) == vTrue)) {
        if (!bMSN || bMSN->IsEmpty()) {
            RETURN ('x', vTrue);
        }
        if ((*ppRange)->m_End.Set (bMSN) == vTrue) {
            if (  ((*ppRange)->GetLen() != (*ppRange)->m_End.GetLen())
               || (ArrayFindFunc_NumComp (*ppRange, &(*ppRange)->m_End) != vLower)) {
                fret = iErr_MSNList_InvalidValue;
            } else {
                RETURN ('x', vTrue);
            }
        }
    }
    delete *ppRange;
    *ppRange = 0;
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

static tResult ArrayFindFunc_Range (CPointerListElement *pElement, void *RefData) {
    dhead ("ArrayFindFunc_Range", DCON_CCntrlMSNList);
    dparams ("%x,%x", pElement, RefData);
    dassert (pElement != 0);
    dassert (RefData != 0);
    dassert (((CMSNRange *)RefData)->IsEmpty() == vFalse);
    // sorting criterion: 1.) length ; 2.) digits
    switch (ArrayFindFunc_NumComp ((CMSNRange *)pElement, (CMSNRange *)RefData)) {
    case vEqual:
        // Equal values are not allowed!
        dwarning (0);
        RETURN ('x', vError);
    case vHigher:
        if (ArrayFindFunc_NumComp ((CMSNRange *)pElement, &((CMSNRange *)RefData)->m_End) != vHigher) {
            // Equal values or overlapping ranges are not allowed!
            dwarning (0);
            RETURN ('x', vError);
        }
        RETURN ('x', vCancel);
    case vLower:
        if (  (((CMSNRange *)pElement)->m_End.IsEmpty() == vFalse)
           && (ArrayFindFunc_NumComp (&((CMSNRange *)pElement)->m_End, (CMSNRange *)RefData) != vLower)) {
            // Equal values or overlapping ranges are not allowed!
            dwarning (0);
            RETURN ('x', vError);
        }
        break;
    }
    RETURN ('x', vNo);
}


/*===========================================================================*\
\*===========================================================================*/

static tResult ArrayFindFunc_MSN (CPointerListElement *pElement, void *RefData) {
    dhead ("ArrayFindFunc_MSN", DCON_CCntrlMSNList);
    dparams ("%x,%x", pElement, RefData);
    dassert (pElement != 0);
    dassert (RefData != 0);
    switch (ArrayFindFunc_MSNComp (((CMSNRange *)pElement), (CCStruct *)RefData)) {
    case vEqual:
        RETURN ('x', vYes);
    case vHigher:
        //RETURN ('x', vCancel);
        // because of the length-specific sorting we can not stop here
        break;
    case vLower:
        if (((CMSNRange *)pElement)->m_End.IsEmpty() == vFalse) {
            switch (ArrayFindFunc_MSNComp (&((CMSNRange *)pElement)->m_End, (CCStruct *)RefData)) {
            case vEqual:
            case vHigher:
                RETURN ('x', vYes);
            case vLower:
                break;
            }
        }
        break;
    }
    RETURN ('x', vNo);
}


/*===========================================================================*\
\*===========================================================================*/

static tResult ArrayFindFunc_DDI (CPointerListElement *pElement, void *RefData) {
    dhead ("ArrayFindFunc_DDI", DCON_CCntrlMSNList);
    dparams ("%x,%x", pElement, RefData);
    dassert (pElement != 0);
    dassert (RefData != 0);
    // Compare length and digits and ignore subaddresstype of MSN.
    switch (ArrayFindFunc_DDIComp ((CMSNRange *)pElement, (CConstString *)RefData)) {
    case vHigher:
        RETURN ('x', vCancel);
    case vEqual:
        RETURN ('x', vYes);
    case vLower:
        if (((CMSNRange *)pElement)->m_End.IsEmpty() == vFalse) {
            switch (ArrayFindFunc_DDIComp (&((CMSNRange *)pElement)->m_End, (CConstString *)RefData)) {
            case vHigher:
                RETURN ('x', vYes);
            case vEqual:
                RETURN ('x', vYes);
                break;
            case vLower:
                break;
            }
        }
        break;
    }
    RETURN ('x', vNo);
}


/*===========================================================================*\
\*===========================================================================*/

tBool CCntrlMSNList::Set (tUInt Controller, tUInt InfoMask, tUInt CIPMask) {
    dhead ("CCntrlMSNList::Set", DCON_CCntrlMSNList);
    dassert (Controller > 0);
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    if (!pE) {
        dprint ("New ControllerInfo! ");
        if (!CIPMask && !InfoMask) {
            RETURN ('x', vTrue);
        }
        pE = new CControllerInfo;
        if (!pE) {
            RETURN ('x', vFalse);
        }
        pE->Controller = Controller;
        CSortPointerList::AddFromLow (pE);
    }
    if ((pE->DDILen > 0) || (pE->TelNumPrefix.IsEmpty() == vFalse)) {
        InfoMask |= infomask_CalledPartyNum | infomask_SendingComplete;
    }
    pE->CIPMask  = CIPMask;
    pE->InfoMask = InfoMask;
    RETURN ('x', vTrue);
}


/*===========================================================================*\
\*===========================================================================*/

tBool CCntrlMSNList::Add (tUInt Controller, tUInt InfoMask, tUInt CIPMask) {
    dhead ("CCntrlMSNList::Add", DCON_CCntrlMSNList);
    dassert (Controller > 0);
    if (!CIPMask && !InfoMask) {
        return vTrue;
    }
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    if (!pE) {
        dprint ("New ControllerInfo! ");
        pE = new CControllerInfo;
        if (!pE) {
            RETURN ('x', vFalse);
        }
        pE->Controller = Controller;
        CSortPointerList::AddFromLow (pE);
    }
    pE->CIPMask  |= CIPMask;
    pE->InfoMask |= InfoMask;
    RETURN ('x', vTrue);
}

/*===========================================================================*\
\*===========================================================================*/

tBool CCntrlMSNList::SetDDI (tUInt Controller, CConstString *pTelNumPrefix, tUInt MaxDDILen) {
    dhead ("CCntrlMSNList::SetDDI", DCON_CCntrlMSNList);
    dassert (Controller > 0);
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    if (!pE) {
        dprint ("New ControllerInfo! ");
        if ((MaxDDILen == 0) && ((pTelNumPrefix == 0) || (pTelNumPrefix->IsEmpty() == vTrue))) {
            RETURN ('x', vTrue);
        }
        pE = new CControllerInfo;
        if (!pE || (pE->TelNumPrefix.Set (pTelNumPrefix) == vFalse)) {
            delete pE;
            RETURN ('x', vFalse);
        }
        pE->Controller = Controller;
        pE->DDILen     = MaxDDILen;
        CSortPointerList::AddFromLow (pE);
    } else if (pTelNumPrefix) {
        if (pE->TelNumPrefix.Set (pTelNumPrefix) == vFalse) {
            RETURN ('x', vFalse);
        }
    } else {
        pE->TelNumPrefix.RemoveAll();
    }
    if (MaxDDILen || (pE->TelNumPrefix.IsEmpty() == vFalse)) {
        // SET DDI
        pE->InfoMask |= infomask_CalledPartyNum | infomask_SendingComplete;
    } else {
        // CLEAR DDI
        // TODO?: perhaps somebody has set the infomask-flags externally
        pE->InfoMask &= ~(infomask_CalledPartyNum | infomask_SendingComplete);
    }
    pE->DDILen = MaxDDILen;
    RETURN ('x', vTrue);
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CCntrlMSNList::SetMSN (tUInt Controller, CConstString *aMSN, CConstString *bMSN) {
    dhead ("CCntrlMSNList::SetMsn", DCON_CCntrlMSNList);
    dparams ("%x,%s,%s", Controller, aMSN, bMSN);
    dassert (Controller > 0);
    if (aMSN && aMSN->IsEmpty()) {
        aMSN = 0;
    }
    dassert ((aMSN != 0) || (bMSN == 0)); // (aMSN == 0 && bMSN != 0) is not allowed
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    if (!pE) {
        if (!aMSN) {
            RETURN ('x', i_Okay);
        }
        pE = new CControllerInfo;
        if (!pE) {
            RETURN ('x', iErr_OutOfMemory);
        }
        pE->Controller = Controller;
        CSortPointerList::AddFromLow (pE);
    } else {
        pE->DelAllMSNs();
        if (!aMSN) {
            RETURN ('x', i_Okay);
        }
    }
    CMSNRange *newRange = 0;
    tSInfo fret = Alloc_CntrlInfo_MSNRange (&newRange, aMSN, bMSN);
    dwarning (infoSuccess (fret));
    if (infoSuccess (fret)) {
        dassert (newRange != 0);
        dassert (newRange->IsEmpty() == vFalse);  // no empty MSN's allowed
        pE->MSNList.AddFirst (newRange);
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CCntrlMSNList::AddMSN (tUInt Controller, CConstString *aMSN, CConstString *bMSN) {
    dhead ("CCntrlMSNList::AddMsn", DCON_CCntrlMSNList);
    dassert (aMSN != 0);
    dparams ("%x,%s,%s", Controller, aMSN->GetPointer(), (bMSN) ? bMSN->GetPointer() : 0);
    dassert (Controller > 0);
    dassert (aMSN->IsEmpty() == vFalse);
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    if (!pE) {
        pE = new CControllerInfo;
        if (!pE) {
            RETURN ('x', iErr_OutOfMemory);
        }
        pE->Controller = Controller;
        CSortPointerList::AddFromLow (pE);
    }

    // MSNs are added to a sortedArray
    CPointerListElement *foundPos = 0;
    CMSNRange           *newRange = 0;
    tSInfo fret = Alloc_CntrlInfo_MSNRange (&newRange, aMSN, bMSN);
    if (infoSuccess (fret)) {
        dassert (newRange != 0);
        dassert (newRange->IsEmpty() == vFalse); // no empty MSN's allowed
        switch (pE->MSNList.FindFwd (ArrayFindFunc_Range, newRange, &foundPos)) {
        case vNo:
            // Add at the end
            pE->MSNList.AddLast (newRange);
            break;
        case vCancel:
            dassert (foundPos != 0);
            foundPos->AddBefore (newRange);
            break;
        default:
            dassert (0);
            // no break
        case vError:
            // overlapping ranges found
            dwarning (0);
            delete newRange;
            fret = iErr_MSNList_MSNOverlap;
            break;
        case vYes:
            // dupplicate Entry found
            dwarning (0);
            delete newRange;
            fret = iErr_MSNList_MSNEqual;
            break;
        }
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

void CCntrlMSNList::Remove (tUInt Controller) {
    dhead ("CCntrlMSNList::Remove", DCON_CCntrlMSNList);
    dparams ("%x", Controller);
    dassert (Controller > 0);
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    if (pE) {
        delete pE;
    }
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CCntrlMSNList::FindCallInfo (tUInt Controller, c_cipvalue CIPValue, CCStruct *pMSN, tUInt *pDDIPrefixLen) {
    dhead ("CCntrlMSNList::FindCallInfo", DCON_CCntrlMSNList);
    dparams ("%x,%x", Controller, CIPValue);
    dassert (Controller > 0);
    dassert (pDDIPrefixLen != 0);
    *pDDIPrefixLen = 0;
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    dassert (pE != 0);
    if (pE && ((1 << CIPValue) & pE->CIPMask)) {
        dprint ("Found Cntrl with valid CIPValue! ");
        if (pE->InfoMask & infomask_CalledPartyNum) {
            dwarning (pE->TelNumPrefix.IsEmpty() == vFalse);
            if ((pE->DDILen == 0) && (pE->MSNList.IsEmpty() == vTrue)) {
                dassert (0);
                RETURN ('x', iWrn_MSNList_RejectCall);
            }
            *pDDIPrefixLen = 0;
            dprint ("-Handle DDI(%x)- ", pMSN->GetLen());
            dwarning (pE->TelNumPrefix.GetLen() < pMSN->GetLen());
            if ((pMSN->GetLen() > 1) && (pE->TelNumPrefix.GetLen() > 0) && (pE->TelNumPrefix.GetLen() < pMSN->GetLen())) {
                c_byte *pFind = s_findstring (pMSN->GetData() + 1, pMSN->GetLen() - 1,
                                              pE->TelNumPrefix.GetPointer(), pE->TelNumPrefix.GetLen());
                // calculate len for DDI Number Prefix
                dwarning (pFind != 0);
                if (pFind) {
                    dassert (pFind > pMSN->GetData());
                    *pDDIPrefixLen = pFind - pMSN->GetData() + pE->TelNumPrefix.GetLen() - 1; // - 1 to ignore the 0xA0 Byte
                    dprint ("DDIPrefixLen=%x ", *pDDIPrefixLen);
                }
            }
            //return (len > *pDDIPrefixLen) ? TestDDINumber (Controller, pData + *pDDIPrefixLen, len - *pDDIPrefixLen) :
            //                                iWrn_MSNList_DDICall;
            RETURN ('x', iWrn_MSNList_DDICall);
        }
        // Global Call
        dprint ("AllowGlobalCall=%x,MSNLen=%x,MSNList=%x ", AllowGlobalCall, pMSN->GetLen(), pE->MSNList.IsEmpty());
        if (pMSN->GetLen() > 1) {
            if (  (pE->MSNList.IsEmpty() == vTrue)
               || (pE->MSNList.FindFwd (ArrayFindFunc_MSN, pMSN) == vYes)) {
                RETURN ('x', i_Okay);
            }
        } else if (AllowGlobalCall == vTrue) {
            RETURN ('x', i_Okay);
        }
    }
    RETURN ('x', iWrn_MSNList_RejectCall);
}


/*===========================================================================*\
\*===========================================================================*/

tSInfo CCntrlMSNList::TestDDINumber (tUInt Controller, CConstString *DDINumber) {
    dhead ("CCntrlMSNList::TestDDINumber", DCON_CCntrlMSNList);
    dparams ("%x,%s", Controller, DDINumber->GetPointer());
    dassert (Controller > 0);
    dassert (DDINumber != 0);
    dassert (DDINumber->IsEmpty() == vFalse);
    tSInfo           fret = iWrn_MSNList_RejectCall;
    CControllerInfo *pE   = (CControllerInfo *)FindHigher (&Controller);
    dwarning (pE != 0);
    if (pE) {
        dassert (pE->InfoMask & infomask_CalledPartyNum);
        dwarning (pE->DDILen > 0);
        dprint ("%x,%x,%x ", pE->DDILen, DDINumber->GetLen(), pE->MSNList.IsEmpty());
        if (pE->MSNList.IsEmpty() == vTrue) {
            fret = (DDINumber->GetLen() >= pE->DDILen) ? i_Okay : iWrn_MSNList_DDICall;
        } else {
            fret = (pE->MSNList.FindFwd (ArrayFindFunc_DDI, DDINumber) == vYes) ? i_Okay : iWrn_MSNList_DDICall;
        }
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

tBool CCntrlMSNList::GetMask (tUInt Controller, tUInt *pInfoMask, tUInt *pCIPMask) {
    dhead ("CCntrlMSNList::BuildMask", DCON_CCntrlMSNList);
    dassert (Controller != IllegalController);
    dassert (pCIPMask != 0);
    dassert (pInfoMask != 0);
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    if (!pE) {
        RETURN ('x', vFalse);
    }
    if ((pE->InfoMask & infomask_CalledPartyNum) && (pE->DDILen == 0) && (pE->MSNList.IsEmpty() == vFalse)) {
        dwarning (0);
        RETURN ('x', vFalse);
    }
    *pCIPMask  = pE->CIPMask;
    *pInfoMask = pE->InfoMask;
    //if (!*pCIPMask && !*pInfoMask) {
    //    pCurPos = (CControllerInfo *)pE->GetPrev();
    //    delete pE;
    //}
    RETURN ('x', vTrue);
}


/*===========================================================================*\
\*===========================================================================*/

tBool CCntrlMSNList::GetNextMask (tUInt *pController, tUInt *pInfoMask, tUInt *pCIPMask) {
    printf("controller info target address: %x, this: %x\n", pController, this); 
    dhead ("CCntrlMSNList::BuildNextMask", DCON_CCntrlMSNList);
    dassert (pController != 0);
    dassert (pCIPMask != 0);
    dassert (pInfoMask != 0);
    tBool fret          = vFalse;
    CControllerInfo *pE = pCurPos;
    pE = (CControllerInfo *)((pE != 0) ? pE->GetHigher() : GetLowest());
    if (pE) {
        //printf("pE is not zero.\n");
    
        //if (pE != (CControllerInfo *)GetLowest()) {
        //  printf("pE is not lowest.\n");
        //} 
        
        while (  (pE->InfoMask & infomask_CalledPartyNum)
              && (pE->DDILen == 0)
              && (pE->MSNList.IsEmpty() == vTrue)) {
            dwarning (0);
            //printf("Trying next.\n");
            pE = (CControllerInfo *)pE->GetHigher();
            if (pE == 0) {
                //printf("nope, next is zero.\n");
                pCurPos = 0;
                RETURN ('x', fret);
            }
        }
        dassert (pE != 0);
        *pController = pE->Controller;
        *pCIPMask    = pE->CIPMask;
        *pInfoMask   = pE->InfoMask;
        fret         = vTrue;
        dprint ("Cntrl=%x,Info=%x,CIP=%x ", *pController, *pInfoMask, *pCIPMask);
    }
    pCurPos = pE;
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

CMultiString *CCntrlMSNList::GetAllMSNs (tUInt Controller) {
    dhead ("CCntrlMSNList::GetMSN", DCON_CCntrlMSNList);
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    dwarning (pE != 0);
    RETURN ('x', (!pE || pE->MSNList.IsEmpty()) ? 0 : &pE->MSNList);
}


/*===========================================================================*\
\*===========================================================================*/

CMSNRange *CCntrlMSNList::GetMSN (tUInt Controller, tUInt pos) {
    dhead ("CCntrlMSNList::GetMSN", DCON_CCntrlMSNList);
    CMSNRange       *pMSN = 0;
    CControllerInfo *pE   = (CControllerInfo *)FindHigher (&Controller);
    dwarning (pE != 0);
    if (pE) {
        dassert (pE->MSNList.CountElements() > pos);
        pMSN = (CMSNRange *)pE->MSNList.GetFirst();
        for (tUInt i = pos; (pMSN != 0) && (i > 0); i--) {
            pMSN = (CMSNRange *)pMSN->GetNext();
        }
    }
    RETURN ('x', pMSN);
}


/*===========================================================================*\
\*===========================================================================*/

CDynamicString *CCntrlMSNList::GetDDIOffset (tUInt Controller) {
    dhead ("CCntrlMSNList::GetDDIOffset", DCON_CCntrlMSNList);
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    dwarning (pE != 0);
    RETURN ('x', (!pE || pE->TelNumPrefix.IsEmpty()) ? 0 : &pE->TelNumPrefix);
}


/*===========================================================================*\
\*===========================================================================*/

tUInt CCntrlMSNList::GetDDILen (tUInt Controller) {
    dhead ("CCntrlMSNList::GetDDILen", DCON_CCntrlMSNList);
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    dwarning (pE != 0);
    RETURN ('x', (!pE) ? 0 : pE->DDILen);
}


/*===========================================================================*\
\*===========================================================================*/

tUInt CCntrlMSNList::GetCountMSN (tUInt Controller) {
    dhead ("CCntrlMSNList::GetCountMSN", DCON_CCntrlMSNList);
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    RETURN ('x', (pE) ? pE->MSNList.CountElements() : 0);
}


/*===========================================================================*\
\*===========================================================================*/

tBool CCntrlMSNList::ExistMSNRange (tUInt Controller) {
    dhead ("CCntrlMSNList::ExistMSNRange", DCON_CCntrlMSNList);
    tBool            fret = vFalse;
    CControllerInfo *pE   = (CControllerInfo *)FindHigher (&Controller);
    dwarning (pE != 0);
    if (pE) {
        for (CMSNRange *MSNRange = (CMSNRange *)pE->MSNList.GetFirst();
             MSNRange != 0;
             MSNRange = (CMSNRange *)MSNRange->GetNext()) {

            if (MSNRange->m_End.IsEmpty() == vFalse) {
                fret = vTrue;
                break;
            }
        }
    }
    RETURN ('x', fret);
}


/*===========================================================================*\
\*===========================================================================*/

void CCntrlMSNList::SetWakeupSupport (tUInt Controller, tBool IsSupported) {
    dhead ("CCntrlMSNList::SetWakeupSupport", DCON_CCntrlMSNList);
    dassert (Controller > 0);
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    if (pE) {
        pE->PMSupportable = IsSupported;
    }
}


/*===========================================================================*\
\*===========================================================================*/

tBool CCntrlMSNList::GetWakeupSupport (tUInt Controller) {
    dhead ("CCntrlMSNList::GetWakeupSupport", DCON_CCntrlMSNList);
    dassert (Controller > 0);
    CControllerInfo *pE = (CControllerInfo *)FindHigher (&Controller);
    RETURN ('x', (pE) ? pE->PMSupportable : vFalse);
}


/*===========================================================================*\
    CControllerInfo - Funktionen
\*===========================================================================*/

CControllerInfo::CControllerInfo (void)
  : Controller (0),
    InfoMask (0),
    CIPMask (0),
    DDILen (0),
    PMSupportable (vTrue) {

    dhead ("CControllerInfo-Constructor", DCON_CCntrlMSNList);
}

CControllerInfo::~CControllerInfo (void) {
    dhead ("CControllerInfo-Destructor", DCON_CCntrlMSNList);
    DelAllMSNs();
}

void CControllerInfo::DelAllMSNs (void) {
    dhead ("CControllerInfo::DelAllMSNs", DCON_CCntrlMSNList);
    COneMultiString *delMSN;
    while ((delMSN = MSNList.GetLast()) != 0) {
        delete delMSN;
    }
}

tCompare CControllerInfo::Compare (void *RefCompData) {
    dhead ("CControllerInfo::Compare", DCON_CCntrlMSNList);
    dparams ("%x,%x", Controller, *((tUInt *)RefCompData));
    return (*((tUInt *)RefCompData) < Controller) ? vLower : (*((tUInt *)RefCompData) == Controller) ? vEqual : vHigher;
}

void *CControllerInfo::GetRefCompData (void) {
    dhead ("CControllerInfo::GetRefCompData", DCON_CCntrlMSNList);
    return &Controller;
}


/*===========================================================================*\
\*===========================================================================*/

