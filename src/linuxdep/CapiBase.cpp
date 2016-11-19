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

#include "CapiBase.h"
#include "c2msgdef.h"
#include "dbgLINUX.h"

#define __NO_CAPIUTILS__
#include <capi20.h>
#include <pthread.h>

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

struct InternalCapiEntry {
    CCallBackCapiBase *pBaseClass;
    pthread_t          Thread;
    tBool              StopThread;
};

static void *CapiBase_WaitForSignalThread (InternalCapiEntry *pice);


/*===========================================================================*\
\*===========================================================================*/

void CapiBase_Register (CCallBackCapiBase *pBaseClass, tUInt, tUInt MaxB3Connection, tUInt MaxB3Blks,
                        tUInt MaxSizeB3) {
    dhead ("CapiBase_Register", DCON_CapiBase);
    dassert (pBaseClass != 0);
    c_info             fret = ci_Int_MemoryFullErr;
    InternalCapiEntry *pice = new InternalCapiEntry;
    dwarning (pice != 0);
    pBaseClass->Set_ApplID (IllegalApplID);
    if (pice) {
        pice->pBaseClass = pBaseClass;
        pice->Thread     = 0;
        pice->StopThread = vFalse;
        fret = (c_info)capi20_isinstalled();
        if (fret == ci_NoError) {
            tUInt applid;
            fret = (c_info)capi20_register (MaxB3Connection, MaxB3Blks, MaxSizeB3, &applid);
            dprint ("ApplID=%x, Register-Return=%x ", applid, fret);
            if ((applid != IllegalApplID) && (CapiInfoIsSuccess (ci_NoError))) {
                pBaseClass->Set_ApplID (applid);
                if (!pthread_create (&pice->Thread, 0, (void *(*)(void *))CapiBase_WaitForSignalThread, pice)) {
                    // everything is initialised successfully
                    return;
                }
                dwarning (0);
                pBaseClass->Set_ApplID (IllegalApplID);
                fret  = (c_info)capi20_release (applid);
                // what can we do if release failed?
                dassert (fret == ci_NoError);
                fret = ci_Reg_OSResourceErr;
            }
        }
        delete pice;
    }
    dassert (fret != ci_NoError);
    pBaseClass->RegisterComplete (vIllegalHandle, fret);
}


/*===========================================================================*\
\*===========================================================================*/

void CapiBase_Release (thCapiBase Base) {
    dhead ("CapiBase_Release", DCON_CapiBase);
    dassert (Base != vIllegalHandle);
    InternalCapiEntry *pice = (InternalCapiEntry *)Base;
    dassert (pice->pBaseClass->ApplID() != IllegalApplID);
    c_word applid           = pice->pBaseClass->ApplID();
    pice->pBaseClass->Set_ApplID (IllegalApplID);
    pice->StopThread        = vTrue;
    dprint ("Terminate Thread(%x)!!! ", pice->pBaseClass);

    // send a message to terminate the CAPI_WAIT_FOR_SIGNAL
    static c_word ListenMsg[] = { 0x1a, 0x00, CAPI_LISTEN_REQ, 0x00,
                                  0x01, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00 };
    capi20_put_message (applid, (c_message)ListenMsg);
    // Call to CAPI_RELEASE will be done at the end of the thread
}


/*===========================================================================*\
\*===========================================================================*/

void CapiBase_PutMessage (thCapiBase Base, c_message Message) {
    dhead ("CapiBase_PutMessage", DCON_CapiBase);
    dassert (Base != vIllegalHandle);
    ((InternalCapiEntry *)Base)->pBaseClass->PutMessageComplete ((c_info)capi20_put_message (
                ((InternalCapiEntry *)Base)->pBaseClass->ApplID(), Message));
}


/*===========================================================================*\
\*===========================================================================*/

void CapiBase_GetProfile (thCapiBase Base, tUInt Controller) {
    dhead ("CapiBase_GetProfile", DCON_CapiBase);
    dassert (Base != vIllegalHandle);
    cm_getprofile Buffer;
    ((InternalCapiEntry *)Base)->pBaseClass->GetProfileComplete (Controller, &Buffer,
                (c_info)capi20_get_profile (Controller, (c_message)&Buffer));
}


/*===========================================================================*\
\*===========================================================================*/

void *CapiBase_WaitForSignalThread (InternalCapiEntry *pice) {
    dhead ("CapiBase_WaitForSignalThread", DCON_CapiBase);
    dassert (pice != 0);
    c_message pCAPIMessage;
    c_word    applid = pice->pBaseClass->ApplID();
    tUInt     fret   = ci_NoError;
    dassert (applid != IllegalApplID);
    if (pice->StopThread == vFalse) {
        pice->pBaseClass->RegisterComplete ((thCapiBase)pice, ci_NoError);
        while (pice->StopThread == vFalse) {
            // after the ThreadState was checked, check the result from CAPI_WAIT_FOR_SIGNAL
            switch (fret) {
            case ci_NoError:
            case ci_Reg_Busy:
            case ci_Msg_Busy:
            case ci_Msg_SendQueueFull:
            case ci_Msg_ReceiveQueueEmpty:
                break;
            default:
                /*----- Ein schwieriger Fehler! -----*/
                dwarning (0);
                pice->pBaseClass->GetMessageReceived (0, (c_info)fret);
                break;
            }
            dprint ("-CAPI_GET_MESSAGE(%x)- ", pice->pBaseClass);
            fret = capi20_get_message (applid, &pCAPIMessage);
            switch (fret) {
            case ci_Reg_Busy:
            case ci_Msg_Busy:
            case ci_Msg_SendQueueFull:
            case ci_Msg_ReceiveQueueEmpty:
                break;
            default:
                dwarning (0);
                // no break!
            case ci_NoError:
                pice->pBaseClass->GetMessageReceived (pCAPIMessage, (c_info)fret);
                break;
            }
            if (pice->StopThread == vTrue) {
                dprint ("Release was called in GetMessageReceived(%x)! ", pice->pBaseClass);
                break;
            }
            dprint ("-CAPI_WAIT_FOR_SIGNAL(%x)- ", pice->pBaseClass);
            fret = capi20_waitformessage (applid, 0);
            dprint ("-loop(%x)- ", pice->pBaseClass);
        }
    }
    dprint ("Call CAPI_RELEASE!! ");
    dassert (pice->pBaseClass->ApplID() == IllegalApplID);
    fret = capi20_release (applid);
    dassert (fret == 0);
    dprint ("-ThreadQueueEnd(%x)- ", pice->pBaseClass);
    CCallBackCapiBase *pBaseClass = pice->pBaseClass;
    delete pice;
    pBaseClass->ReleaseComplete();
    return 0;
}


/*===========================================================================*\
\*===========================================================================*/
