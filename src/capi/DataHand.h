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

#ifndef _DATAHAND_H_
#define _DATAHAND_H_

#include "aTypes.h"
#include "ProtClas.h"

/*===========================================================================*\
\*===========================================================================*/

class CDataHandleAdmin {
public:
    CDataHandleAdmin (void);
    virtual ~CDataHandleAdmin (void);
    tBool InitEntries (tUShort MaxEntries);

    tBool New (void *NewDataHandle, tUShort *RetHandle);    // NewDataHandle != 0 !!!!
    tBool Free (tUShort InHandle, void **RetDataHandle);

    tBool IsEmpty (void);
    tBool IsFull (void);
    tUShort CurCount (void);
    tUShort MaxEntries (void);

    tBool DelNextEntry (void **HandleToDel = 0);     // False: nichts da zum löschen

private:
    struct HandleArray {
        void   *pointer;
        tUShort next;
        tBool   inuse;
    } *m_Array;

    CProtectClass m_Protect;
    tUShort       m_maxEntries;
    tUShort       m_curEntries;
    tUShort       m_nextOne;
};

/*===========================================================================*\
    inline Funktionen
\*===========================================================================*/

inline tBool CDataHandleAdmin::IsEmpty (void) {
    return (m_curEntries == 0) ? vTrue : vFalse;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tBool CDataHandleAdmin::IsFull (void) {
    return (m_curEntries == m_maxEntries) ? vTrue : vFalse;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUShort CDataHandleAdmin::CurCount (void) {
    return m_curEntries;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

inline tUShort CDataHandleAdmin::MaxEntries (void) {
    return m_maxEntries;
}

/*===========================================================================*\
\*===========================================================================*/

#endif
