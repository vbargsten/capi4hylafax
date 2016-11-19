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

#ifndef _DYNSTORE_H_
#define _DYNSTORE_H_

#include "aTypes.h"
#include "ProtClas.h"

/*===========================================================================*\
\*===========================================================================*/

typedef tUByte *DynStorePointer;

/*===========================================================================*\
\*===========================================================================*/

class CDynamicStore {
public:
    CDynamicStore (void);
    virtual ~CDynamicStore (void);
    tBool Init (tUInt SizeOfData, tUInt StartCount = 0);

    DynStorePointer Alloc (void);
    void Free (DynStorePointer pData);

protected:
    struct Element {
        Element *next;
        tUByte data[1];
    };

    CProtectClass m_Protect;

private:
    Element  *pStore;
    tUInt     dataSize;

#ifndef NDEBUG
    tUInt count;
#endif
};


/*===========================================================================*\
\*===========================================================================*/

class CDynStoreQueue : public CDynamicStore {
public:
    CDynStoreQueue (void);
    virtual ~CDynStoreQueue (void);

    void InQueue (DynStorePointer pData);
    DynStorePointer OutQueue (void);

    void FreeQueue (void);
    tBool IsQueueEmpty (void);

private:
    Element  *first;
    Element  *last;
};


/*===========================================================================*\
\*===========================================================================*/

inline tBool CDynStoreQueue::IsQueueEmpty (void) {
    return (first) ? vFalse : vTrue;
}

/*===========================================================================*\
\*===========================================================================*/

#endif

