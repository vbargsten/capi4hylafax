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

#ifndef _WAITSTAT_H_
#define _WAITSTAT_H_

#include "CapiStat.h"
#include "Eventing.h"

/*===========================================================================*\
\*===========================================================================*/

class CCAPI20_WaitStatistic : public CCAPI20_Statistic {
public:
    CCAPI20_WaitStatistic (ctStatDeep statdeep = StatDeep_CntrlFacilities);
    virtual ~CCAPI20_WaitStatistic (void);

    void FreshUp (tUInt maxWaitTime = 0);   // in ms

private:
    void FreshUpComplete (c_info Info);

    thEvent m_hEvent;
};


/*===========================================================================*\
    CCAPI20_WaitStatistic - inline functions
\*===========================================================================*/

inline CCAPI20_WaitStatistic::CCAPI20_WaitStatistic (ctStatDeep statdeep)
  : CCAPI20_Statistic (statdeep),
    m_hEvent (vIllegalHandle) {

    Event_Create (&m_hEvent);
}

inline CCAPI20_WaitStatistic::~CCAPI20_WaitStatistic (void) {
    Event_Destroy (&m_hEvent);
}

inline void CCAPI20_WaitStatistic::FreshUp (tUInt maxWaitTime) {
    CCAPI20_Statistic::FreshUp();
    if (  (Event_Wait (m_hEvent, maxWaitTime) == vFalse)
       && (FreshUpInUse() == vTrue)) {
        FreshUpCancel();
        // TODO: Wait infinite is probably not so good?
        Event_Wait (m_hEvent, 0);
    }
}

inline void CCAPI20_WaitStatistic::FreshUpComplete (c_info) {
    Event_Set (m_hEvent);
}


/*===========================================================================*\
\*===========================================================================*/


#endif

