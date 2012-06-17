/*---------------------------------------------------------------------------*\
 * Copyright (C) 2000 AVM GmbH. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, and WITHOUT
 * ANY LIABILITY FOR ANY DAMAGES arising out of or in connection
 * with the use or performance of this software. See the
 * GNU General Public License for further details.
\*---------------------------------------------------------------------------*/

#ifndef _BASICFAX_H_
#define _BASICFAX_H_

#include "ftif2huf.h"
#include "sff2huf.h"
#include "huf2ftif.h"
#include "huf2sff.h"
#include "MultiStr.h"
#include "InfoType.h"
#include "defaults.h"
#include "CCSParam.h"
#include <time.h>

/*===========================================================================*\
\*===========================================================================*/

class CBasicFaxConversion {
public:
    CBasicFaxConversion (void);
    virtual ~CBasicFaxConversion (void);
    void ConnectLog (cp_ncpi_all *pNCPI);
    void WriteXferLog (const char *cmd, tUInt commid, tUChar *jobtag, tUInt *pJobid, char *devname,
                       CDynamicString *pUser, CDynamicString *pDestination, char *statustext);

    tSInfo OpenSend (CMultiString *pSendFileList, tUInt Resolution);
    void StopSend (void);
    void CloseSend (void);
    tSInfo SendSFF (tUByte *buffer, tUInt maxLen, tUInt *pLen);

    tSInfo OpenRecv (void);
    void StopRecv (void);
    void CloseRecv (void);
    tSInfo RecvSFF (tUByte *buffer, tUInt len);

    FaxFormatType GetFormat (void);
    void SetFormat (FaxFormatType faxtype);

    tUInt GetPageCount (void);
    void SetPageCount (tUInt pc);

    tUInt GetBaudRate (void);

    CMultiString *GetRecvFiles (void);
    tUInt GetResolution (void);
    tUInt GetSendState (void);
    tUInt GetRecvState (void);
    tBool GetRecvIllegalLines (tUInt *pCountIllegalLines, tUInt *pCountAllLines);
    void ConnectionInd (void);
    void ConnectionStarted (void);

    void SetReceiveID (CDynamicString *pRecvIDStr);
    CDynamicString *GetReceiveID (void);
    time_t GetStartTime (void);

    virtual void PageEndReached (tBool IsLastPage) = 0;
    virtual void WriteLog (tSInt /*priority*/, char * /*text*/, ...) {}
    virtual tUInt GetRecvFileMode (void) { return DEFAULT_FILE_MODE; }
    virtual CDynamicString *GetSpoolDir (void) { return 0; }

private:
    time_t GetStopTime (void);
    time_t GetConnectionTime (void);

    tSInfo FillSffFilter (void);
    tSInt GetRecvFile (void);
    tSInt GetNextRecvFile (void);

    CSFilter_fTif2Huf m_sTifFilter;
    CSFilter_Huf2Sff  m_sSffFilter;
    CSFilter_Huf2fTif m_rTifFilter;
    CSFilter_Sff2Huf  m_rSffFilter;

    FaxFormatType    m_format;
    COneMultiString *m_pCurSendFile;
    CDynamicString  *m_pRecvString;
    CMultiString     m_RecvFileList;
    tUInt            m_PageCount;
    tUInt            m_BaudRate;
    tUInt            m_VResolution;
    tUInt            m_finishedFaxFile;
    tUInt            m_recvState;
    tUInt            m_faxFileNr;
    tSInt            m_FaxFileD;
    tSInt            m_RecvFile;
    time_t           m_StartTime;
    time_t           m_ConnectStartTime;
    time_t           m_StartPageTime;
    time_t           m_StopTime;
};


/*===========================================================================*\
\*===========================================================================*/

inline FaxFormatType CBasicFaxConversion::GetFormat (void) {
    return m_format;
}

inline void CBasicFaxConversion::SetFormat (FaxFormatType faxtype) {
    m_format = faxtype;
}

inline tUInt CBasicFaxConversion::GetPageCount (void) {
    return m_PageCount;
}

inline void CBasicFaxConversion::SetPageCount (tUInt pc) {
    m_PageCount = pc;
}

inline tUInt CBasicFaxConversion::GetBaudRate (void) {
    return m_BaudRate;
}

inline tUInt CBasicFaxConversion::GetResolution (void) {
    return m_VResolution;
}

inline tUInt CBasicFaxConversion::GetSendState (void) {
    return m_finishedFaxFile;
}

inline tUInt CBasicFaxConversion::GetRecvState (void) {
    return m_recvState;
}

inline tBool CBasicFaxConversion::GetRecvIllegalLines (tUInt *pCountIllegalLines, tUInt *pCountAllLines) {
    return m_rSffFilter.GetIllegalLines (pCountIllegalLines, pCountAllLines);
}

inline void CBasicFaxConversion::ConnectionInd (void) {
    m_StartTime = m_StartPageTime = m_StopTime = time (0);
    m_ConnectStartTime = 0;
}

inline void CBasicFaxConversion::ConnectionStarted (void) {
    m_ConnectStartTime = m_StartPageTime = time (0);
}

inline void CBasicFaxConversion::SetReceiveID (CDynamicString *pRecvIDStr) {
    m_pRecvString = pRecvIDStr;
}

inline CDynamicString *CBasicFaxConversion::GetReceiveID (void) {
    return m_pRecvString;
}

inline time_t CBasicFaxConversion::GetStartTime (void) {
    return m_StartTime;
}

inline time_t CBasicFaxConversion::GetStopTime (void) {
    return m_StopTime;
}

inline time_t CBasicFaxConversion::GetConnectionTime (void) {
    return m_StopTime - m_ConnectStartTime;
}

/*===========================================================================*\
\*===========================================================================*/

#endif
