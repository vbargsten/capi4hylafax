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

#ifndef _DBGINFO_H_
#define _DBGINFO_H_

#include "aTypes.h"
#include "ExtFuncs.h"

/*---------------------------------------------------------------------------*\
    Debug- und Print-Levels
\*---------------------------------------------------------------------------*/

#define Dbg_Level_Nothing           0x00    // Wirklich nichts ausgeben
#define Dbg_Level_Errors            0x01    // Nur Asserts und Errors
#define Dbg_Level_Infos             0x02    // Assert, Errors, Warnungen und Infos
#define Dbg_Level_Logs              0x03    // Assert, Errors, Warnungen, Infos und Logs
#define Dbg_Level_Short             0x04    // -"- und der Funktionsaufruf (inkl. Parameter)
#define Dbg_Level_Entry             0x06    // -"- und Funktionsaufrufe und -ende (inkl. Parameter)
#define Dbg_Level_Internal          0x07    // Alles wird ausgegeben

                                            // Print-Ausgabe von:
#define Prt_Level_NoFunctionName    0x0000  // Keine Ausgabe des Funktonsnamen
#define Prt_Level_NameInEveryLine   0x0040  // Name der Funkton bei jeder
                                            // Print-Ausgabe angeben
#define Prt_Level_NameAtFuncStart   0x0080  // Name der Funktion nur am Funktions-
                                            // Ein- & Austrittspunkt angeben
#define Prt_Level_FileNameAtStart   0x0100  // Dateiname nur beim Funktionsaufruf
#define Prt_Level_FileNameAtInfo    0x0200  // Dateiname vor jeder Debug-Ausgabe
#define Prt_Level_FuncNameAtError   0x0400  // Funktionsname statt Dateiname bei Fehlern

#define Prt_Level_Default           (Prt_Level_NameAtFuncStart |    \
                                     Prt_Level_FileNameAtInfo)

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define _USE_DEBUG_INFO_CLASS_NONE          0       // Reihenfolge wichtig!!
#define _USE_DEBUG_INFO_CLASS_NORMAL        1

#ifndef USE_DEBUG_INFO_CLASS
#error USE_DEBUG_INFO_CLASS not defined!
#endif

/*---------------------------------------------------------------------------*\
    Activate Assert-BreakPoints
\*---------------------------------------------------------------------------*/
#if (USE_DEBUG_INFO_CLASS == _USE_DEBUG_INFO_CLASS_NONE)

#define dActivateBreakPoint(activate)

#else

void dActivateBreakPoint (tBool activate);

#endif


/*---------------------------------------------------------------------------*\
    dwriteE & dwriteI
\*---------------------------------------------------------------------------*/
#if (USE_DEBUG_INFO_CLASS == _USE_DEBUG_INFO_CLASS_NONE)

#define dwriteErr(DL, format, ...)
#define dwriteWrn(DL, format, ...)
#define dwriteLog(DL, format, ...)
#define dwriteS(DL, format, ...)
#define dwriteE(DL, format, ...)
#define dwriteI(DL, format, ...)

#else

void dwriteErr (unsigned DL, const char *format, ...);      // Logs
void dwriteWrn (unsigned DL, const char *format, ...);      // Logs
void dwriteLog (unsigned DL, const char *format, ...);      // Logs
void dwriteS   (unsigned DL, const char *format, ...);      // Short
void dwriteE   (unsigned DL, const char *format, ...);      // Entry
void dwriteI   (unsigned DL, const char *format, ...);      // Internal

#endif

#if (USE_DEBUG_INFO_CLASS == _USE_DEBUG_INFO_CLASS_NONE)
/*---------------------------------------------------------------------------*\
    Release Makros
\*---------------------------------------------------------------------------*/

#define dhead(name, DL)
#define dheadP(name, DL, PL)
#define dprintBuf(data, length)
#define dassert(expr)
#define dwarning(expr)
#define dvalue(val)                         (val)
#define RETURN(format, retval)              return (retval)


#ifdef __linux__

#define dparams(format, x...)
#define dprint(format, x...)
#define dprintWrn(format, x...)
#define dprintErr(format, x...)
#define dinfo(expression, format, x...)

#else

#pragma warning (disable: 4002)             // Ignore: "too many parameters for macro"
#define dparams(format)
#define dprint(format)
#define dprintWrn(format)
#define dprintErr(format)
#define dinfo(expression, format)

#endif


#elif (USE_DEBUG_INFO_CLASS >= _USE_DEBUG_INFO_CLASS_NORMAL)
/*---------------------------------------------------------------------------*\
    Includes nur fürs Debuggen
\*---------------------------------------------------------------------------*/

#include <stdarg.h>
#include "CDbgIDS.h"

/*---------------------------------------------------------------------------*\
    Debug Makros & Funktionen
\*---------------------------------------------------------------------------*/

#define dhead(name, DL)             CDebugInfo _CDI_Object ((const char *)name, DL, Prt_Level_Default, __FILE__, __LINE__)
#define dheadP(name, DL, PL)        CDebugInfo _CDI_Object ((const char *)name, DL, PL, __FILE__, __LINE__)


//void dparams (const char *format, ...);
#define dparams                     _CDI_Object.PrintParams


//int dprint (const char *format, ...);
#define dprint                      _CDI_Object.print


#define dprintBuf(data, length)     _CDI_Object.printBuf (data, length)


#define dprintWrn                   _CDI_Object.printWrn


#define dprintErr                   _CDI_Object.printErr


#define dassert(expr)               (void) ((expr) || (_CDI_Object.assertion (#expr, __LINE__), 0))

#define dwarning(expr)              (void) ((expr) || (_CDI_Object.warning (#expr, __LINE__), 0))


//int dinfo (const int expression, const char *format, ...);
#define dinfo                       _CDI_Object.info


#define dvalue(val)                 _CDI_Object.printValue (val, #val, __LINE__)


#define RETURN(format, retval)      _CDI_Object.SetRetVal ((format), (retval));     \
                                    return (retval)


/*---------------------------------------------------------------------------*\
    CDebugInfo - Class
\*---------------------------------------------------------------------------*/

class CDebugInfo {
public:
    CDebugInfo (const char *FuncName, unsigned DebugLevel, unsigned PrintLevel,
                const char *_FileName = "?", unsigned _LineNumber = 0);
    virtual ~CDebugInfo (void);

    void print (const char *format, ...);
    void printBuf (const void *data, unsigned length);
    unsigned printValue (unsigned val, const char *sval, const unsigned int line);

    void printWrn (const char *format, ...);
    void printErr (const char *format, ...);

    void assertion (const char *exp, const unsigned int line);
    void assertion (const int expValue, const char *exp, const char *fname, const unsigned int line);

    void warning (const char *output, const unsigned int line);
    void warning (const int expression, const char *output, const char *fname, const unsigned int line);

    void info (const int expression, const char *format, ...);

    void PrintParams (const char *format, ...);
    void SetRetVal (const char format, ...);

    void ShowAllOutputs (void);

protected:
    void RememberOutput (const char *format, ...);
    void vRememberOutput (const char *format, va_list param);

private:
    const char *name;
    char *filename;
    unsigned linenumber;
    unsigned debuglevel;
    unsigned printlevel;
    unsigned spaces;
    CDbgIDynString Output;

    struct RetValueStruct {
        char format;
        union  {
            void *p;
            char *s;
            char  c;
            tUInt u;
            tSInt i;
            tBool b;
        };
    } RetValue;

    void DbgPrintIntern (void);
};

#endif /* (USE_DEBUG_INFO_CLASS) */
/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#endif
