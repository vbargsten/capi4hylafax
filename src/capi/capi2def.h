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

#ifndef _CAPI2DEF_H_
#define _CAPI2DEF_H_

#include "aTypes.h"
#include "InfoType.h"
#include "osmem.h"

/*===========================================================================*\
\*===========================================================================*/

#define IllegalApplID                   0
#define IllegalController               0
#define IllegalPLCI                     0
#define IllegalNCCI                     0

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define CAPI_DEFAULT_S0_LINES_COUNT     2
#define CAPI_MAX_MESSAGE_SIZE           4096
#define CAPI_MAX_DATA_SIZE              2048
#define CAPI_MAX_OUTSTANDING_FRAMES     8
#define CAPI_MAX_CONTROLLER_NUMBER      127

#define CAPI_MIN_DATA_SIZE              100
#define CAPI_MIN_OUTSTANDING_FRAMES     2

#define CALLNUM_INDICATOR_DEFAULT           ((tUByte) 0x80)
#define CALLNUM_INDICATOR_SUPPRESS          ((tUByte) 0xA0)

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

/*----- basic-type definitions -----*/

typedef tUByte    c_byte;
typedef tUInt16   c_word;
typedef tUInt32   c_dword;
typedef tUInt64   c_qword;
typedef c_byte   *c_message;
typedef c_byte   *c_dataptr;

/*----- capi state machine type and values -----*/

enum _tCapiState {                      // DO NOT CHANGE THE ORDER!!
    cs_Released,                        // inital state
    cs_Releasing,                       // => initial state
    cs_Registering,                     // building up to register state
    cs_RegisterDelayed,                 // CAPI not ready, so remember Register and complete a 2. time later
    cs_Registered,                      // listen or make call state
    cs_D_DisconnectRespPending,         // Ind received, waiting for DataB3Resp => => cs_Registered
    cs_D_DisconnectPending,             // => cs_Registered
    cs_CallDeflecting,                  // => cs_Registered
    cs_D_ConnectRequest,                // start make call => cs_D_ConnectPending
    cs_D_ConnectIndication,             // get incoming call => Answer => cs_D_ConnectPending
    cs_ResumePending,                   // => D-Channel state
    cs_D_ConnectPending,                // => D-Channel state
    cs_SuspendPending,                  // => cs_D_DisconnectPending
    cs_Holded,                          // Holded state
    cs_HoldPending,                     // from cs_D_Connected to cs_Holded
    cs_RetrievePending,                 // from cs_Holded to cs_D_Connected
    cs_ProtocolPending,                 // SelectBProtocol in use => cs_D_Connected
    cs_D_Connected,                     // D-Channel (connected) state
    cs_B_DisconnectRespPending,         // Ind received, waiting for DataB3Resp => D-Channel state
    cs_B_DisconnectPending,             // => cs_B_DisconnectRespPending
    cs_B_ConnectPending,                // => B-Channel state
    cs_Connected                        // B-Channel (connected) state
};
typedef tUInt tCapiState;


/*----- errornumbers -----*/

enum c_info {
    ci_NoError                        = 0x0000,
    ci_Wrn_NCPIignored                = 0x0001,
    ci_Wrn_FlagsIgnored               = 0x0002,
    ci_Wrn_AlertAlreadySend           = 0x0003,
    ci_Wrn_HighestWarningValue        = 0x00FF,     // Special Warning Value
    ci_Reg_ToManyAppls                = 0x1001,
    ci_Reg_LogBlkSizeToSmall          = 0x1002,
    ci_Reg_BuffExeceeds64k            = 0x1003,
    ci_Reg_MsgBufSizeToSmall          = 0x1004,
    ci_Reg_NumLogConnNotSupported     = 0x1005,
    ci_Reg_Reserved                   = 0x1006,
    ci_Reg_Busy                       = 0x1007,
    ci_Reg_OSResourceErr              = 0x1008,
    ci_Reg_NotInstalled               = 0x1009,
    ci_Reg_CtrlerNotSupportExtEquip   = 0x100a,
    ci_Reg_CtrlerOnlySupportExtEquip  = 0x100b,
    ci_Msg_IllAppNr                   = 0x1101,
    ci_Msg_IllCmdOrSubcmdOrMsgToSmall = 0x1102,
    ci_Msg_SendQueueFull              = 0x1103,
    ci_Msg_ReceiveQueueEmpty          = 0x1104,
    ci_Msg_ReceiveOverflow            = 0x1105,
    ci_Msg_UnknownNotPar              = 0x1106,
    ci_Msg_Busy                       = 0x1107,
    ci_Msg_OSResourceErr              = 0x1108,
    ci_Msg_NotInstalled               = 0x1109,
    ci_Msg_CtrlerNotSupportExtEquip   = 0x110a,
    ci_Msg_CtrlerOnlySupportExtEquip  = 0x110b,
    ci_Cod_MsgNotSupportInCurState    = 0x2001,
    ci_Cod_IllegalAddress             = 0x2002,
    ci_Cod_OutOfPLCI                  = 0x2003,
    ci_Cod_OutOfNCCI                  = 0x2004,
    ci_Cod_OutOfListen                = 0x2005,
    ci_Cod_OutOfFax                   = 0x2006,
    ci_Cod_IllegalMsgParamCoding      = 0x2007,
    ci_Ser_B1ProtNotSupported         = 0x3001,
    ci_Ser_B2ProtNotSupported         = 0x3002,
    ci_Ser_B3ProtNotSupported         = 0x3003,
    ci_Ser_B1ProtParamNotSupported    = 0x3004,
    ci_Ser_B2ProtParamNotSupported    = 0x3005,
    ci_Ser_B3ProtParamNotSupported    = 0x3006,
    ci_Phy_ProtErrLayer1              = 0x3301,
    ci_Phy_ProtErrLayer2              = 0x3302,
    ci_Phy_ProtErrLayer3              = 0x3303,
    ci_Phy_T30ConnectToNoFax          = 0x3311,
    ci_Net_UnspecifiedCallClearing    = 0x3400,

    // The following values are enhancements of the C++ CAPI state machine ant NOT(!) CAPI conform.
    ci_Int_IncorrectDTMFDigit         = 0xFF00,     // Mapping of "DTMF Information" into c_info.
    ci_Int_UnknownDTMFRequest         = 0xFF01,     // Mapping of "DTMF Information" into c_info.
    ci_Int_MemoryFullErr              = 0xFFF0,     // "new" returns a NULL Pointer
    ci_Int_NotConnected               = 0xFFF1,     // B-Channel not established anymore
    ci_Int_MsgSizeOverflow            = 0xFFF2,     // Message length is greater than CAPI_MAX_MESSAGE_SIZE
    ci_Int_TimerInitFailure           = 0xFFF3,     // A needed Timer couldn't be initialized
    ci_Int_IllegalState               = 0xFFF4,
    ci_Int_FunctionInUse              = 0xFFF5,
    ci_Int_OSResourceErr              = 0xFFF6,     // Problems with the OS so we can't do what was requested
    ci_Int_RegisterDelayed            = 0xFFFF      // CAPI not ready so wait and call RegisterComplete later a second time
};

/*----- rejectvalues -----*/

enum c_reject {
    creject_Accept                    = 0,
    creject_Ignore                    = 1,
    creject_Reject_Normal             = 2,
    creject_Reject_Busy               = 3,
    creject_Reject_NotAvailable       = 4,
    creject_Reject_Facility           = 5,
    creject_Reject_Unacceptable       = 6,
    creject_Reject_Incompatible       = 7,
    creject_Reject_OutOfOrder         = 8
};

/*----- CIP Values -----*/

enum c_cipvalue {
    cipvalue_Undefined                = 0,
    cipvalue_Speech                   = 1,
    cipvalue_Digital                  = 2,
    cipvalue_RestrictedDigital        = 3,
    cipvalue_3_1kHzAudio              = 4,
    cipvalue_7kHzAudio                = 5,
    cipvalue_Video                    = 6,
    cipvalue_PacketMode               = 7,
    cipvalue_56kBitRateAdaption       = 8,
    cipvalue_UnrestDigitalTones       = 9,
    cipvalue_Telephony                = 16,
    cipvalue_G3Fax                    = 17,
    cipvalue_G4Fax                    = 18,
    cipvalue_TeletexG4Fax             = 19,
    cipvalue_TeletexProcessable       = 20,
    cipvalue_TeletexBasicMode         = 21,
    cipvalue_Videotex                 = 22,
    cipvalue_Telex                    = 23,
    cipvalue_MessageHandlingSystems   = 24,
    cipvalue_OSIApplication           = 25,
    cipvalue_7kHzTelephony            = 26,
    cipvalue_VideoTelephony1          = 27,
    cipvalue_VideoTelephony2          = 28,
};

/*----- CIP Masks -----*/

enum c_cipmask {
    cipmask_Undefined                 = (1 << cipvalue_Undefined),
    cipmask_Speech                    = (1 << cipvalue_Speech),
    cipmask_Digital                   = (1 << cipvalue_Digital),
    cipmask_RestrictedDigital         = (1 << cipvalue_RestrictedDigital),
    cipmask_3_1kHzAudio               = (1 << cipvalue_3_1kHzAudio),
    cipmask_7kHzAudio                 = (1 << cipvalue_7kHzAudio),
    cipmask_Video                     = (1 << cipvalue_Video),
    cipmask_PacketMode                = (1 << cipvalue_PacketMode),
    cipmask_56kBitRateAdaption        = (1 << cipvalue_56kBitRateAdaption),
    cipmask_UnrestDigitalTones        = (1 << cipvalue_UnrestDigitalTones),
    cipmask_Telephony                 = (1 << cipvalue_Telephony),
    cipmask_G3Fax                     = (1 << cipvalue_G3Fax),
    cipmask_G4Fax                     = (1 << cipvalue_G4Fax),
    cipmask_TeletexG4Fax              = (1 << cipvalue_TeletexG4Fax),
    cipmask_TeletexProcessable        = (1 << cipvalue_TeletexProcessable),
    cipmask_TeletexBasicMode          = (1 << cipvalue_TeletexBasicMode),
    cipmask_Videotex                  = (1 << cipvalue_Videotex),
    cipmask_Telex                     = (1 << cipvalue_Telex),
    cipmask_MessageHandlingSystems    = (1 << cipvalue_MessageHandlingSystems),
    cipmask_OSIApplication            = (1 << cipvalue_OSIApplication),
    cipmask_7kHzTelephony             = (1 << cipvalue_7kHzTelephony),
    cipmask_VideoTelephony1           = (1 << cipvalue_VideoTelephony1),
    cipmask_VideoTelephony2           = (1 << cipvalue_VideoTelephony2)
};

/*----- Info Masks -----*/

enum c_infomask {
    infomask_Cause                    = 0x0001,
    infomask_DateTime                 = 0x0002,
    infomask_Display                  = 0x0004,
    infomask_UserUser                 = 0x0008,
    infomask_CallProgress             = 0x0010,
    infomask_Facility                 = 0x0020,
    infomask_ChargeInfo               = 0x0040,
    infomask_CalledPartyNum           = 0x0080,
    infomask_ChannelInfo              = 0x0100,
    infomask_EarlyB3Connect           = 0x0200,
    infomask_RedirectInfo             = 0x0400,
    infomask_SendingComplete          = 0x1000
};


/*----- B - Protocols -----*/

enum c_b1prot {
    b1prot_HDLC                       = 0,
    b1prot_Transparent                = 1,
    b1prot_V110_asynch                = 2,
    b1prot_V110_synch                 = 3,
    b1prot_G3Fax                      = 4,
    b1prot_invertedHDLC               = 5,
    b1prot_56kBitTransparent          = 6,
    b1prot_Modem_negotiate            = 7,
    b1prot_Modem_asynch               = 8,
    b1prot_Modem_synch                = 9,
    b1prot_ATM                        = 28
};

enum c_b2prot {
    b2prot_X75                        = 0,
    b2prot_Transparent                = 1,
    b2prot_SDLC                       = 2,
    b2prot_LAPD                       = 3,
    b2prot_G3Fax                      = 4,
    b2prot_PPP                        = 5,
    b2prot_IgnoreFramingError         = 6,
    b2prot_Modem_negotiate            = 7,
    b2prot_X75_V42bis                 = 8,
    b2prot_V120_asynch                = 9,
    b2prot_V120_V42bis_asynch         = 10,
    b2prot_V120_transparent           = 11,
    b2prot_LAPD_SAPI                  = 12,
    b2prot_ADSL                       = 30
};

enum c_b3prot {
    b3prot_Transparent                = 0,
    b3prot_T90NL                      = 1,
    b3prot_ISO8208                    = 2,
    b3prot_X25                        = 3,
    b3prot_G3Fax                      = 4,
    b3prot_G3Fax_extended             = 5,
  //b3prot_reserved               = 6,  Reserved == not usable => doesn't need to be defined
    b3prot_Modem                      = 7,
    b3prot_ADSL                       = 30
};

/*----- facility -----*/

enum c_facilselect {
    facilselect_Handset               = 0,
    facilselect_DTMF                  = 1,
    facilselect_V42Compression        = 2,
    facilselect_SupplServices         = 3,
    facilselect_PMWakeup              = 4,
};

enum c_supplserv {
    supplserv_GetSupportedServices    = 0x0000,
    supplserv_Listen                  = 0x0001,
    supplserv_Hold                    = 0x0002,
    supplserv_Retrieve                = 0x0003,
    supplserv_Suspend                 = 0x0004,
    supplserv_Resume                  = 0x0005,
    supplserv_ECT                     = 0x0006,
    supplserv_3PTY_Begin              = 0x0007,
    supplserv_3PTY_End                = 0x0008,
    supplserv_CF_Activate             = 0x0009,
    supplserv_CF_Deactivate           = 0x000A,
    supplserv_CF_InterrogateParams    = 0x000B,
    supplserv_CF_InterrogateNumbers   = 0x000C,
    supplserv_CD                      = 0x000D,
    supplserv_MCID                    = 0x000E,
    supplserv_CCBS_Request            = 0x000F,
    supplserv_CCBS_Deactivate         = 0x0010,
    supplserv_CCBS_Interrogate        = 0x0011,
    supplserv_CCBS_Call               = 0x0012,
    // Supplementary Services Notfications
    supplserv_Hold_Notify             = 0x8000,
    supplserv_Retrieve_Notify         = 0x8001,
    supplserv_Suspend_Notify          = 0x8002,
    supplserv_Resume_Notify           = 0x8003,
    supplserv_CallIsDiverting_Notify  = 0x8004,
    supplserv_DiversionActive_Notify  = 0x8005,
    supplserv_CF_Activate_Notify      = 0x8006,
    supplserv_CF_Deactivate_Notify    = 0x8007,
    supplserv_Diversion_Information   = 0x8008,
    supplserv_CT_Alerted_Notify       = 0x8009,
    supplserv_CT_Active_Notify        = 0x800A,
    supplserv_ConferenceEstablished   = 0x800B,
    supplserv_ConferenceDisconnected  = 0x800C,
    supplserv_CCBS_EraseCallLinkID    = 0x800D,
    supplserv_CCBS_Status             = 0x800E,
    supplserv_CCBS_RemoteUserFree     = 0x800F,
    supplserv_CCBS_B_Free             = 0x8010,
    supplserv_CCBS_Erase              = 0x8011,
    supplserv_CCBS_StopAlerting       = 0x8012,
    supplserv_CCBS_InfoRetain         = 0x8013
};

enum c_dtmffunction {
    dtmffunction_StartListen          = 1,
    dtmffunction_StopListen           = 2,
    dtmffunction_SendDigits           = 3
};

enum c_forwardtype {
    forwardtype_Unconditional         = 0,
    forwardtype_Busy                  = 1,
    forwardtype_NoReply               = 2
};

enum c_b3_faxextended_options {
    b3_faxextopt_highres              = 0x0001,
    b3_faxextopt_poll                 = 0x0002,
    b3_faxextopt_jpeg                 = 0x0400,
    b3_faxextopt_jbig                 = 0x0800,
    b3_faxextopt_nojbigpro            = 0x1000,
    b3_faxextopt_nomrcomp             = 0x2000,
    b3_faxextopt_nommrcomp            = 0x4000,
    b3_faxextopt_noecm                = 0x8000
};

enum c_ncpi_faxextended_options {
    ncpi_faxextopt_highres            = 0x0001,
    ncpi_faxextopt_poll               = 0x0002,
    ncpi_faxextopt_pollafter          = 0x0004,
    ncpi_faxextopt_jpeg               = 0x0400,
    ncpi_faxextopt_jbig               = 0x0800,
    ncpi_faxextopt_jbigpro            = 0x1000,
    ncpi_faxextopt_mrcomp             = 0x2000,
    ncpi_faxextopt_mmrcomp            = 0x4000,
    ncpi_faxextopt_noecm              = 0x8000
};


/*---------------------------------------------------------------------------*\
    Convert PLCI and NCCI
\*---------------------------------------------------------------------------*/

#define GetControllerFromPLCI(plci)     (plci & 0x0000007F)
#define GetControllerFromNCCI(ncci)     (ncci & 0x0000007F)
#define GetPLCIFromNCCI(ncci)           (ncci & 0x0000FFFF)

/*---------------------------------------------------------------------------*\
    CapiInfoIs... Makros
\*---------------------------------------------------------------------------*/

#define CapiInfoIsSuccess(info)  ((info) <= ci_Wrn_HighestWarningValue)
#define CapiInfoIsError(info)    ((info) > ci_Wrn_HighestWarningValue)
#define CapiInfoIsRegError(info) (((info) & 0xFF00) == 0x1000)

#define MapCapiErr2Info(val)     ((!val) ? i_Okay : CalculateInfoValue (infoType_Capi, \
                                  CapiInfoIsError (val) ? infoKind_Error : infoKind_Warning, (val)))


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#endif
