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

#ifndef _CEVENT_H_
#define _CEVENT_H_

#ifndef CEVENT_USE_SEMAPHORE
/*===========================================================================*\
\*===========================================================================*/

#include "Eventing.h"

class CEvent {
public:
    CEvent (void) : m_hEvent (vIllegalHandle) {}

    virtual ~CEvent (void) {
        Event_Destroy (&m_hEvent);
    }

    tBool Init (void) {
        return Event_Create (&m_hEvent);
    }

    void Set (void) {
        Event_Set (m_hEvent);
    }

    void Clear (void) {
        Event_Clear (m_hEvent);
    }

    tBool Wait (tUInt msecs) {
        return Event_Wait (m_hEvent, msecs);
    }

private:
    thEvent m_hEvent;
};


/*===========================================================================*\
\*===========================================================================*/
#else // #ifndef CEVENT_USE_SEMAPHORE

#include "ProtClas.h"
#include "SemaFunc.h"

class CEvent : private CProtectClass {
public:
    CEvent (void);
    virtual ~CEvent (void);

    tBool Init (void);

    void  Set (void);
    void  Clear (void);
    tBool Wait (tUInt msecs);

private:
    thSemaphore m_hSema;
    tBool       m_Flag;
    tBool       m_Wait;
    tBool       m_IncCalled;
};


/*===========================================================================*\
\*===========================================================================*/

inline CEvent::CEvent (void)
  : m_hSema (vIllegalHandle),
    m_Flag (vFalse),
    m_Wait (vFalse),
    m_IncCalled (vFalse) {
}

inline CEvent::~CEvent (void) {
    Semaphore_Destroy (&m_hSema);
}

inline tBool CEvent::Init (void) {
    tBool fret = Semaphore_Init (&m_hSema);
    if (fret == vTrue) {
        fret = CProtectClass::Init();
        if (fret == vFalse) {
            Semaphore_Destroy (&m_Sema);
        }
    }
    return fret;
}

inline void CEvent::Set (void) {
    BeginWrite();
    if (m_Flag == vFalse) {
        m_Flag = vTrue;
        if ((m_Wait == vTrue) && (m_IncCalled == vFalse)) {
            m_IncCalled == vTrue;
            EndWrite();
            Semaphore_Inc (m_hSema);
            return;
        }
    }
    EndWrite();
    return;
}

inline void CEvent::Clear (void) {
    BeginWrite();
    m_Flag = vFalse;
    EndWrite();
}

inline tBool CEvent::Wait (tUInt msecs) {
    tBool fret = vTrue;
    BeginWrite();
    if (!m_Flag) {
        m_Wait = vTrue;
        EndWrite();
        fret = Semaphore_Wait (m_hSema, msecs);
        BeginWrite();
        m_Wait = vFalse;
        if ((fret == vFalse) && (m_IncCalled == vTrue)) {
            EndWrite();
            // Set nach Wait aufgerufen => Semaphore muﬂ nun noch wieder in den Grundzustand
            fret = Semaphore_Wait (m_hSema, 1);
            BeginWrite();
        }
    }
    m_Flag      = vFalse;
    m_IncCalled = vFalse;
    EndWrite();
    return fret;
}


#endif // #ifndef CEVENT_USE_SEMAPHORE
/*===========================================================================*\
\*===========================================================================*/

#endif
