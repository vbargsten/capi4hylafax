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

#ifndef _RECVMAIN_H_
#define _RECVMAIN_H_

#include "PtrList.h"
#include "FilePars.h"
#include "PathName.h"
#include "defaults.h"
#include <stdarg.h>

/*===========================================================================*\
\*===========================================================================*/

class CFaxReceiveMain : public CPointerList {
public:
    CFaxReceiveMain (void);
    ~CFaxReceiveMain (void);

    void WriteLog (tSInt priority, char *text, ...);
    void vWriteLog (tSInt priority, char *text, va_list params);
    void GenerateHylafaxConfigFile (CDynamicString *pName, CDynamicString *pFaxNumber, CDynamicString *pIdentifier);

    tBool Config (char *CommandLineSpoolDir);
    tBool StartReceive (void);
    void  StopReceive (void);
    tSInt main (tSInt argc, char **argv);

    void setupPermissions (void);
    void usage (void);

    CDynamicString FaxRcvdCmd;
    CPathName      SpoolDir;
    tUInt          LogFileMode;

private:
    void WaitOnFIFOs (void);

    FaxFormatType  format;
    CDynamicString RecvProgName;
    CDynamicString FaxUser;
    CDynamicString FaxGroup;
    CDynamicString LogFileName;
    tUInt          LogLevel;
    FILE          *hLogFile;
    tBool          enableEcho;
    tUChar        *configName;
};

/*===========================================================================*\
\*===========================================================================*/

#endif
