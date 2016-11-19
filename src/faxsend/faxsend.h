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

#ifndef _FAXSEND_H_
#define _FAXSEND_H_

#include "Channel.h"
#include "BasicFax.h"
#include "ConfPars.h"
#include "PathName.h"
#include "defaults.h"

/*===========================================================================*\
\*===========================================================================*/

class CFaxSend : public CTransferChannel, public CBasicFaxConversion {
public:
    CFaxSend (void (*FinishedFunc)(void));
    virtual ~CFaxSend (void);
    tBool Config (tChar *confName, char *devName, FaxFormatType faxType, int argc, char **argv);
    void  SetTagLine (CDynamicString *pTagLineFormat);

    tSInfo Send (CDynamicString *Number, CMultiString *FaxFilesList, tUInt Resolution,
                 tUInt jobID, tUInt *pCommID, CDynamicString *pPollStr);

    void WriteLog (tSInt priority, const char *text, ...);

    tUInt exitState;
    void CloseJobFile (void);

private:
    // virtual functions from CBasicFaxConversion
    void PageEndReached (tBool IsLastPage);
    tUInt GetRecvFileMode (void);
    CDynamicString *GetSpoolDir (void);

    // virtual functions from CTransferChannel
    void IncomingCall (c_cipvalue CIPValue, CConstString *pOppositeNumber, CConstString *pMyCalledNumber);
    void DoConnecting (void);
    void ConnectProceeding (CConstString *ConnectedNumber);
    void IsConnected (cp_ncpi_all *pNCPI);
    void PutDataComplete (void *hDataID, c_info Info);
    void GetData (tUByte *Data, tUShort DataLength, tUShort DataHandle);
    void DoDisconnecting (c_info ReasonB3, cp_ncpi_all *pNCPI);
    void IsDisconnected (c_info Reason);
    void IsReseted (void);

    void SendData (void);
    tUInt GetJobFile (void);
    void (*finishFunc)(void);

    CPathName        SpoolDir;
    CDynamicString   DeviceName;
    CDynamicString   PollRcvdCmd;
    CDynamicString   PollString;
    CDynamicString   TelNumber;
    CDynamicString   FaxNumber;
    CDynamicString   Identifier;
    CDynamicString   OrgFaxNumber;
    CDynamicString   OrgIdentifier;
    CDynamicString   OutMSN;
    CDynamicString   NumberPrefix;
    CDynamicString   NumberPlusReplace;
    CDynamicString   LogFileName;
    tUInt            LogLevel;
    tUInt            LogFileMode;
    tUInt            SuppressMSN;
    tUInt            UseFaxCIPValue;
    tUInt            RecvFileMode;
    tUInt            m_commID;
    FILE            *hLogFile;
    FILE            *hJobFile;
    tBool            enableEcho;
    tUInt            SendDataInUse;
    //tBool            UseOnlyLowRes;
};

/*===========================================================================*\
\*===========================================================================*/
#endif

