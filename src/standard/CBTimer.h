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

#ifndef _CBTIMER_H_
#define _CBTIMER_H_

#include "aTypes.h"
#include "aString.h"
#include "Protect.h"

/*===========================================================================*\
    Definitons for CallBack-Functions
\*===========================================================================*/

typedef void (*tCallBackTimerFunc) (void *, void *);

typedef class CBasicTimerSupport *(*tBasicTimerInitFunc) (void);


/*===========================================================================*\
    A Basic Timer class for internal use only
\*===========================================================================*/

class CBasicTimerSupport {
public:
    CBasicTimerSupport (void);
    virtual ~CBasicTimerSupport (void) {}

    void SetHandles (void *handle1, void *handle2);
    void SetCallBack (tCallBackTimerFunc func);
    void DoCallBack (void);

    static tBool Create (tHandle *pTimer, tBasicTimerInitFunc func);
    static void Destroy (tHandle *pTimer);

private:
    tCallBackTimerFunc  m_func;
#ifdef USE_TIMER_POOL
    CBasicTimerSupport *m_next;
#endif
    void               *m_handle1;
    void               *m_handle2;
};


/*===========================================================================*\
    Timer Pool
\*===========================================================================*/
#ifdef USE_TIMER_POOL

tUInt BasicTimerPool_Alloc (tUInt count, tBasicTimerInitFunc func);

void BasicTimerPool_Free (tUInt count);


#endif // USE_TIMER_POOL
/*===========================================================================*\
    CBasicTimerSupport - inline functions
\*===========================================================================*/

inline CBasicTimerSupport::CBasicTimerSupport (void)
  : m_func (0),
#ifdef USE_TIMER_POOL
    m_next (0),
#endif
    m_handle1 (0),
    m_handle2 (0) {
}

inline void CBasicTimerSupport::SetHandles (void *handle1, void *handle2) {
    m_handle1 = handle1;
    m_handle2 = handle2;
}

inline void CBasicTimerSupport::SetCallBack (tCallBackTimerFunc func) {
    m_func = func;
}

inline void CBasicTimerSupport::DoCallBack (void) {
    // dassert (m_func != 0);
    m_func (m_handle1, m_handle2);
}


/*===========================================================================*\
\*===========================================================================*/

#endif
