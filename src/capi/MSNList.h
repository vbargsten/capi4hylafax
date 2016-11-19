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

#ifndef _MSNLIST_H_
#define _MSNLIST_H_

#include "capi2def.h"
#include "CCStruct.h"
#include "SortList.h"
#include "InfoType.h"
#include "MultiStr.h"

/*---------------------------------------------------------------------------*\
    tSInfo
\*---------------------------------------------------------------------------*/

#define iWrn_MSNList_RejectCall     CalculateInfoValue (infoType_MSNList, infoKind_Warning, 1)
#define iWrn_MSNList_DDICall        CalculateInfoValue (infoType_MSNList, infoKind_Warning, 2)
#define iErr_MSNList_MSNEqual       CalculateInfoValue (infoType_MSNList, infoKind_Error,   1)
#define iErr_MSNList_MSNOverlap     CalculateInfoValue (infoType_MSNList, infoKind_Error,   2)
#define iErr_MSNList_InvalidValue   CalculateInfoValue (infoType_MSNList, infoKind_Error,   3)


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

class CMSNRange : public COneMultiString {
public:
    CDynamicString m_End;
};


/*===========================================================================*\
    CCntrlMSNList
\*===========================================================================*/

class CCntrlMSNList : private CSortPointerList {
public:
    CCntrlMSNList (void);

    // Set remove every previous entry for the controller. Add integrate the new values to the previous one.
    // Set and Add doesn't change anything for the entries from SetMSN and AddMSN and vice versa.
    tBool Set (tUInt Controller, tUInt InfoMask, tUInt CIPMask);
    tBool Add (tUInt Controller, tUInt InfoMask, tUInt CIPMask);

    tBool SetDDI (tUInt Controller, CConstString *pTelNumPrefix, tUInt MaxDDILen = 0);
    tSInfo SetMSN (tUInt Controller, CConstString *aMSN, CConstString *bMSN = 0);
    tSInfo AddMSN (tUInt Controller, CConstString *aMSN, CConstString *bMSN = 0);
    // if MaxDDILen is 0, some MSN/DDI numbers must be entered in SetMSN/AddMSN!
    // if bMSN is set, it is used as a range [aMSN - bMSN]

    void Remove (tUInt Controller);
    void RemoveAll (void);

    tSInfo FindCallInfo (tUInt Controller, c_cipvalue CIPValue, CCStruct *MSN, tUInt *pDDIPrefixLen);
    tSInfo TestDDINumber (tUInt Controller, CConstString *DDINumber);
    // *pMainNumLen only valid if return = i_AccpeptCall

    void ResetGetNextMask (void);
    tBool GetNextMask (tUInt *pController, tUInt *pInfoMask, tUInt *pCIPMask);  // vFalse == no more masks
    tBool GetMask (tUInt Controller, tUInt *pInfoMask, tUInt *pCIPMask);        // vFalse == Controller is not in the list

    CMultiString   *GetAllMSNs (tUInt Controller);        // Elements: CMSNRange
    CMSNRange      *GetMSN (tUInt Controller, tUInt pos);
    CDynamicString *GetDDIOffset (tUInt Controller);
    tUInt           GetDDILen (tUInt Controller);
    tUInt           GetCountMSN (tUInt Controller);
    tBool           ExistMSNRange (tUInt Controller);

    void SetWakeupSupport (tUInt Controller, tBool IsSupported);
    tBool GetWakeupSupport (tUInt Controller);

    tBool IsEmpty (void);
    tBool ExistController (tUInt Controller);
    void SetGlobalCallOpt (tBool value);

private:
    class CControllerInfo *pCurPos;
    tBool AllowGlobalCall;
};

/*===========================================================================*\
    inline functions from CCntrlMSNList
\*===========================================================================*/

inline CCntrlMSNList::CCntrlMSNList (void)
  : AllowGlobalCall (vTrue) {

    ResetGetNextMask();
}

inline void CCntrlMSNList::RemoveAll (void) {
    RemoveAndDeleteAll();
}

inline void CCntrlMSNList::ResetGetNextMask (void) {
    pCurPos = 0;
}

inline tBool CCntrlMSNList::IsEmpty (void) {
    return CSortPointerList::IsEmpty();
}

inline tBool CCntrlMSNList::ExistController (tUInt Controller) {
    return (FindHigher (&Controller) != 0);
}

inline void CCntrlMSNList::SetGlobalCallOpt (tBool value) {
    AllowGlobalCall = value;
}

/*===========================================================================*\
\*===========================================================================*/

#endif
