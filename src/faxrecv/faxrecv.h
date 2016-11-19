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

#ifndef _FAXRECV_H_
#define _FAXRECV_H_

#include "Channel.h"
#include "BasicFax.h"
#include "recvdev.h"
#include "sff2huf.h"
#include "huf2ftif.h"
#include "defaults.h"
#include <time.h>

/*===========================================================================*\
\*===========================================================================*/

class CFaxReceive : public CTransferChannel, public CBasicFaxConversion, public CPointerListElement {
public:
    CFaxReceive (CFaxReceiveDevice *FaxRecvDevice, FaxFormatType faxtype);
    virtual ~CFaxReceive (void);

    tBool StartReceive (void);
    void StopReceive (void);

private:
    // virtual functions from CTransferChannel
    void ListenError (tUInt Controller, c_info Info);
    void IncomingCall (c_cipvalue CIPValue, CConstString *pOppositeNumber, CConstString *pMyCalledNumber);
    void DoConnecting (void);
    void ConnectProceeding (CConstString *ConnectedNumber);
    void IsConnected (cp_ncpi_all *pNCPI);
    void PutDataComplete (void *hDataID, c_info Info);
    void GetData (tUByte *Data, tUShort DataLength, tUShort DataHandle);
    void DoDisconnecting (c_info ReasonB3, cp_ncpi_all *pNCPI);
    void IsDisconnected (c_info Reason);
    void IsReseted (void);

    // virtual functions from CBasicFaxConversion
    void PageEndReached (tBool IsLastPage);
    void WriteLog (tSInt priority, const char *text, ...);
    tUInt GetRecvFileMode (void);
    CDynamicString *GetSpoolDir (void);

    // internal functions
    struct NameList {
        NameList *next;
        tUChar    string[1];
    };

    tBool NotifyFIFO (char Type);
    tBool GetJobFile (void);

    CFaxReceiveDevice *FaxDevice;
    CDynamicString     ONumber;
    CDynamicString     MyNumber;
    tUInt              jobNr;
    tUInt              quitRecv;
    FILE              *hJobFile;
};

/*===========================================================================*\
\*===========================================================================*/
#endif

