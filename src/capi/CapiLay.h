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

#ifndef _CAPILAY_H_
#define _CAPILAY_H_

#include "CapiMsg.h"
#include "PLCIList.h"
#include "Protect.h"

/*===========================================================================*\
\*===========================================================================*/

class CCAPI20_Layer : public CCAPI20_MsgBase {
public:
    CCAPI20_Layer (void);
    virtual ~CCAPI20_Layer (void);

    c_word GetNextMsgNr (void);

    tBool SetNewPLCIEntry (c_dword PLCI);
    void FreePLCIEntry (c_dword PLCI);

private:
    c_word nr;
    static tUInt CountLayers;
    static thProtect counterMutex;
};

/*===========================================================================*\
\*===========================================================================*/

inline CCAPI20_Layer::CCAPI20_Layer (void) {
    nr = 0;
    // assert (CountLayers + 1 != 0);
    if (  (Protect_IsCreated (counterMutex) == vFalse)
       && (Protect_Create (&counterMutex) == vFalse)) {
        // throw 
    }
    Protect_BeginWrite(counterMutex);
    CountLayers++;
    Protect_EndWrite(counterMutex);
    
    if (InitPLCIList() == vFalse) {
            // TODO: was nun?
    }
    
    //// IMPORTANT: Ich gehe davon aus, dass dieser Constructor serialized aufgerufen wird
    //if (CountLayers++ == 0) {
    //    if (InitPLCIList() == vFalse) {
    //        // assert (0);
    //        // TODO: was nun?
    //    }
    //}
}

inline CCAPI20_Layer::~CCAPI20_Layer (void) {
    // assert (CountLayers > 0);
    
    
    Protect_BeginWrite(counterMutex);
    CountLayers--;
    
    if (CountLayers == 0) {
        DeinitPLCIList();
        //Protect_EndWrite(counterMutex);
        // we wont destroy it -- could be critical if new layer is created 
        // Protect_Destroy(&counterMutex);
    }
    
    Protect_EndWrite(counterMutex);
}

inline c_word CCAPI20_Layer::GetNextMsgNr (void) {
    return ++nr;
}

inline tBool CCAPI20_Layer::SetNewPLCIEntry (c_dword PLCI) {
    return TestAndSetPLCIEntry (PLCI);
}

inline void CCAPI20_Layer::FreePLCIEntry (c_dword PLCI) {
    if (PLCI != IllegalPLCI) {
        UnsetPLCIEntry (PLCI);
    }
}


/*===========================================================================*\
\*===========================================================================*/

#endif

