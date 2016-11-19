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

#ifndef _CAPIBASE_H_
#define _CAPIBASE_H_

#include "CCSMesg.h"

/*===========================================================================*\
\*===========================================================================*/

class CCallBackCapiBase;

typedef tHandle thCapiBase;

void CapiBase_Register (CCallBackCapiBase *pBaseClass, tUInt MsgBufSize, tUInt MaxB3Connection,
                        tUInt MaxB3Blks, tUInt MaxSizeB3);
void CapiBase_Release  (thCapiBase Base);
void CapiBase_PutMessage (thCapiBase Base, c_byte *pMessage);
//void CapiBase_GetManufacturer (thCapiBase Base, tUInt Controller);
//void CapiBase_GetVersion      (thCapiBase Base, tUInt Controller);
//void CapiBase_GetSerialNumber (thCapiBase Base, tUInt Controller);
void CapiBase_GetProfile (thCapiBase Base, tUInt Controller);


/*===========================================================================*\
\*===========================================================================*/

class CCallBackCapiBase {
public:
    CCallBackCapiBase (void);
    virtual ~CCallBackCapiBase (void) {}

    c_word ApplID (void);
    void Set_ApplID (c_word ApplID);

    virtual void RegisterComplete (thCapiBase Base, c_info ErrorCode) = 0;
    virtual void ReleaseComplete (void) = 0;

    virtual void PutMessageComplete (c_info ErrorCode) = 0;
    virtual void GetMessageReceived (c_byte *pMessage, c_info ErrorCode) = 0;
    // return: vTrue - Okay, vFalse - Release was called or an very hard error occured

    //virtual void GetManufacturerComplete (c_byte * /*lpBuffer*/, c_info /*ErrorCode*/) {}
    //virtual void GetVersionComplete      (c_byte * /*version*/, c_info /*ErrorCode*/) {}
    //virtual void GetSerialNumberComplete (c_byte * /*lpBuffer*/, c_info /*ErrorCode*/) {}
    virtual void GetProfileComplete (tUInt Controller, cm_getprofile *pProfile, c_info ErrorCode) = 0;

private:
    c_word m_Applid;
};


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline CCallBackCapiBase::CCallBackCapiBase (void)
  : m_Applid (IllegalApplID) {
}

inline c_word CCallBackCapiBase::ApplID (void) {
    return m_Applid;
}

inline void CCallBackCapiBase::Set_ApplID (c_word ApplID) {
    m_Applid = ApplID;
}


/*===========================================================================*\
\*===========================================================================*/

#endif
