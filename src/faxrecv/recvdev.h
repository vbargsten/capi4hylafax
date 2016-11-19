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

#ifndef _RECVDEV_H_
#define _RECVDEV_H_

#include "PtrList.h"
#include "recvmain.h"
#include "MSNList.h"
#include "defaults.h"

/*===========================================================================*\
\*===========================================================================*/

class CFaxReceiveDevice : public CPointerListElement {
public:
    CFaxReceiveDevice (CFaxReceiveMain *MainFaxClass, FaxFormatType faxtype);
    virtual ~CFaxReceiveDevice (void);

    void Config (CConfigParserSection *section);
    tBool StartReceive (void);
    void StopReceive (void);

    // function called from CFaxReceive
    void ReceiveStoped (void);

    // all about FIFO
    tBool RecreateFIFO (void);
    tSInt GetFIFOHandle (void);
    void FIFOchanged (void);
    tBool NotifyFIFO (tBool Receive, const char Type, const char *Data);
    tBool NotifyFIFO (const char Type);

    // write functions
    void WriteLog (tSInt priority, const char *text, ...);
    void vWriteLog (tSInt priority, const char *text, va_list args);
    void WriteStatus (const char *text);

    CDynamicString   DeviceName;
    CFaxReceiveMain *MainClass;
    CDynamicString   FaxNumber;
    tUInt            RecvFileMode;

private:
    CPointerList     FaxThreads;
    CCntrlMSNList    CIPMSNList;
    FaxFormatType    format;
    tUInt            CountThreads;
    tSInt            hFIFOfile;
    FILE            *hStatusFile;
    tBool            Starting;
};

/*===========================================================================*\
    inline CFaxReciveDevice functions
\*===========================================================================*/

inline tSInt CFaxReceiveDevice::GetFIFOHandle (void) {
    return hFIFOfile;
}

/*===========================================================================*\
\*===========================================================================*/

#endif
