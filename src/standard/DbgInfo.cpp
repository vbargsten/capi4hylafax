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

#include <stdarg.h>
#include "DbgInfo.h"
#include "CDbgIDS.h"
#include "aStrPrt.h"
#include "aString.h"
#include "ExtFuncs.h"
#include <stdlib.h>

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define DEBUGINFO_PRINTBUF_SIZE     100
#define dwrite_MAX_CHAR_LENGTH      150

/*===========================================================================*\
\*===========================================================================*/

#if (USE_DEBUG_INFO_CLASS > _USE_DEBUG_INFO_CLASS_NONE)

/*===========================================================================*\
\*===========================================================================*/

static tBool BreakPointsAreActive = vFalse;

void dActivateBreakPoint (tBool activate) {
    BreakPointsAreActive = activate;
}


/*===========================================================================*\
\*===========================================================================*/

void dwriteErr (unsigned DL, const char *format, ...) {
    if (DL >= Dbg_Level_Errors) {
        char string[dwrite_MAX_CHAR_LENGTH];
        va_list param;
        va_start (param, format);
        s_strcpy (string, DbgIPrintLinePrefix);
        if (a_vsnprintf (string + DbgIPrintLinePrefixLen, dwrite_MAX_CHAR_LENGTH - DbgIPrintLinePrefixLen, format, param) >= 0) {
            DbgIPrintStr (string);
        }
        va_end (param);
    }
}

void dwriteWrn (unsigned DL, const char *format, ...) {
    if (DL >= Dbg_Level_Infos) {
        char string[dwrite_MAX_CHAR_LENGTH];
        va_list param;
        va_start (param, format);
        s_strcpy (string, DbgIPrintLinePrefix);
        if (a_vsnprintf (string + DbgIPrintLinePrefixLen, dwrite_MAX_CHAR_LENGTH - DbgIPrintLinePrefixLen, format, param) >= 0) {
            DbgIPrintStr (string);
        }
        va_end (param);
    }
}

void dwriteLog (unsigned DL, const char *format, ...) {
    if (DL >= Dbg_Level_Logs) {
        char string[dwrite_MAX_CHAR_LENGTH];
        va_list param;
        va_start (param, format);
        s_strcpy (string, DbgIPrintLinePrefix);
        if (a_vsnprintf (string + DbgIPrintLinePrefixLen, dwrite_MAX_CHAR_LENGTH - DbgIPrintLinePrefixLen, format, param) >= 0) {
            DbgIPrintStr (string);
        }
        va_end (param);
    }
}

void dwriteS (unsigned DL, const char *format, ...) {
    if (DL >= Dbg_Level_Short) {
        char string[dwrite_MAX_CHAR_LENGTH];
        va_list param;
        va_start (param, format);
        s_strcpy (string, DbgIPrintLinePrefix);
        if (a_vsnprintf (string + DbgIPrintLinePrefixLen, dwrite_MAX_CHAR_LENGTH - DbgIPrintLinePrefixLen, format, param) >= 0) {
            DbgIPrintStr (string);
        }
        va_end (param);
    }
}

void dwriteE (unsigned DL, const char *format, ...) {
    if (DL >= Dbg_Level_Entry) {
        char string[dwrite_MAX_CHAR_LENGTH];
        va_list param;
        va_start (param, format);
        s_strcpy (string, DbgIPrintLinePrefix);
        if (a_vsnprintf (string + DbgIPrintLinePrefixLen, dwrite_MAX_CHAR_LENGTH - DbgIPrintLinePrefixLen, format, param) >= 0) {
            DbgIPrintStr (string);
        }
        va_end (param);
    }
}

void dwriteI (unsigned DL, const char *format, ...) {
    if (DL >= Dbg_Level_Internal) {
        char string[dwrite_MAX_CHAR_LENGTH];
        va_list param;
        va_start (param, format);
        s_strcpy (string, DbgIPrintLinePrefix);
        if (a_vsnprintf (string + DbgIPrintLinePrefixLen, dwrite_MAX_CHAR_LENGTH - DbgIPrintLinePrefixLen, format, param) >= 0) {
            DbgIPrintStr (string);
        }
        va_end (param);
    }
}

#endif

/*===========================================================================*\
\*===========================================================================*/
#if (USE_DEBUG_INFO_CLASS == _USE_DEBUG_INFO_CLASS_EMULATION)

void dparams (const char *format, ...) {
    va_list param;
    va_start (param, format);
    char output[DBGDYNSTRING_STARTMAXLENGTH];
    if (a_vsnprintf(output, DBGDYNSTRING_STARTMAXLENGTH, format, param) >= 0) {
        DbgIPrintFull (PARAMETERS_BEGIN "%s" PARAMETERS_END, output);
    } else {
        DbgIPrintFull ("\ndparams: buffer to small\n");
    }
    va_end (param);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void dprintBuf (const void *data, unsigned length) {
    if (data && length) {
        unsigned       i;
        unsigned       j;
        unsigned       laufbuffree;
        char           outstring[DEBUGINFO_PRINTBUF_SIZE];
        char          *laufbuf;
        unsigned char *databuf;

        for (i = 0; i < length; i += 0x10) {
            a_snprintf (outstring, DEBUGINFO_PRINTBUF_SIZE, "\n%05x: ", (i & 0xFFFFF));
            laufbuffree = DEBUGINFO_PRINTBUF_SIZE - 8;
            laufbuf = outstring + 8;
            databuf = (((unsigned char *)data) + i);
            for (j = 0; j < 0x10; j++) {
                if (i + j < length) {
                    if (j && (j % 4 == 0)) {
                        DbgIAssert (laufbuffree >= 5);
                        a_snprintf (laufbuf, laufbuffree, "' %02x ", *databuf);
                        laufbuffree -= 5;
                        laufbuf     += 5;
                    } else {
                        DbgIAssert (laufbuffree >= 3);
                        a_snprintf (laufbuf, laufbuffree, "%02x ", *databuf);
                        laufbuf += 3;
                    }
                    databuf++;
                } else {
                    if (j % 4 == 0) {
                        DbgIAssert (laufbuffree >= 5);
                        a_snprintf (laufbuf, laufbuffree, "     ");
                        laufbuf += 5;
                    } else {
                        DbgIAssert (laufbuffree >= 3);
                        a_snprintf (laufbuf, laufbuffree, "   ");
                        laufbuf += 3;
                    }
                }
            }
            DbgIAssert (laufbuffree >= 20);
            *(laufbuf++) = '|';
            *(laufbuf++) = ' ';
            databuf = (((unsigned char *)data) + i);
            for (j = 0; ((j < 0x10) && (i + j < length)); j++, laufbuf++, databuf++) {
                *laufbuf = (*databuf < 32) ? '.' : *databuf;
            }
            *(laufbuf++) = ' ';
            *laufbuf     = '\0';
            DbgIPrintFull ("%s", outstring);
        }
        DbgIPrintFull ("\n");
    } else {
        DbgIPrintFull ("DATA: (p=%x, l=%x)\n", data, length);
    }
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

int dinfo (const int expression, const char *format, ...) {
    int fret = 0;
    if (!expression) {
        va_list param;
        va_start (param, format);
        char output[DBGDYNSTRING_STARTMAXLENGTH];
        fret = a_vsnprintf(output, DBGDYNSTRING_STARTMAXLENGTH, format, param);
        if (fret >= 0) {
            DbgIPrintFull ("[info: %s]\n", output);
        }
        va_end (param);
    }
    return fret;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

unsigned DbgIPrintValue (unsigned val, const char *sval, const char *filename, const unsigned int line) {
    DbgIPrintFull ("Call of '%s' in %s(%d) returns %x.\n", sval, filename, line, val);
    return val;
}


/*===========================================================================*\
\*===========================================================================*/
#elif (USE_DEBUG_INFO_CLASS >= _USE_DEBUG_INFO_CLASS_NORMAL)

static unsigned IndentSpaces = 0;

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

CDebugInfo::CDebugInfo(const char *FuncName, unsigned DebugLevel, unsigned PrintLevel,
                       const char *_FileName, unsigned _LineNumber) {
    RetValue.format = 0;
    debuglevel = DebugLevel;
    filename = NULL;
    
    if (DebugLevel > Dbg_Level_Nothing) {
        spaces        = IndentSpaces;
        IndentSpaces += DEBUG_INDENT_WIDTH;
        name          = FuncName;
        linenumber    = _LineNumber;
        printlevel    = PrintLevel;
        //filename      = _FileName;
        
        unsigned int filenameLength = 0;
        unsigned int pathLength = 0;
        const char * curFileNameChar = _FileName;
        
        for (; *curFileNameChar != '\0'; curFileNameChar++) {
            if ((*curFileNameChar == '\\') || (*curFileNameChar == '/')) {
                //filename = _FileName + 1;
                filenameLength = 0;
            } else {
                ++filenameLength;
            }
            ++pathLength;
        }
        
        filename = (char *) malloc(filenameLength + 1);
        s_strcpy(filename, _FileName+pathLength-filenameLength);
    
        
        if (printlevel & Prt_Level_NameAtFuncStart) {
            if (debuglevel == Dbg_Level_Short) {
                RememberOutput (NAMEFORSHORT_BEGIN "%s%s", DbgIPrintLinePrefix, name);
                DbgPrintIntern();
            } else if (debuglevel >= Dbg_Level_Entry) {
                if (printlevel & Prt_Level_FileNameAtStart) {
                    RememberOutput ("%*t" NAMEATFUNCSTART_BEGIN "%s%s in %s(%d)" NAMEATFUNCSTART_END,
                                    spaces, ' ', DbgIPrintLinePrefix, name, filename, linenumber);
                } else {
                    RememberOutput ("%*t" NAMEATFUNCSTART_BEGIN "%s%s" NAMEATFUNCSTART_END,
                                    spaces, ' ', DbgIPrintLinePrefix, name);
                }
                DbgPrintIntern();
            }
        }
    } else {
        name       = 0;
        //filename   = 0;
        linenumber = 0;
        printlevel = 0;
        spaces     = 0;
    }
}


/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

CDebugInfo::~CDebugInfo (void) {
    /*----- Ende der Funktion ausgeben -----*/
    if (debuglevel == Dbg_Level_Short) {
        RememberOutput (NAMEFORSHORT_END, name);
        DbgPrintIntern();
    } else if ((debuglevel >= Dbg_Level_Entry) && (printlevel & Prt_Level_NameAtFuncStart)) {
        if ((debuglevel >= Dbg_Level_Entry) && (RetValue.format != '\0')) {
            char ParamOutput[sizeof(NAMEATFUNCEND_BEGIN) + sizeof(NAMEATFUNCEND_END) +
                             sizeof(RETURNVALUE_BEGIN) + sizeof(RETURNVALUE_END) + 8] =
                            "%*t" NAMEATFUNCEND_BEGIN RETURNVALUE_BEGIN "%." RETURNVALUE_END "%s%*t" NAMEATFUNCEND_END;

                ParamOutput[sizeof(NAMEATFUNCEND_BEGIN) + sizeof(RETURNVALUE_BEGIN) + 2] = RetValue.format;
                RememberOutput (ParamOutput, spaces, ' ', RetValue.p, name,
                                spaces - DEBUG_INDENT_WIDTH, ' ');
        } else {
            RememberOutput ("%*t" NAMEATFUNCEND_BEGIN "%s%*t" NAMEATFUNCEND_END,
                            spaces, ' ', name, spaces - DEBUG_INDENT_WIDTH, ' ');
        }
        DbgPrintIntern();
    }
    if (debuglevel > Dbg_Level_Nothing) {
        IndentSpaces -= DEBUG_INDENT_WIDTH;
    }

    /*----- Speicherplatz aufräumen -----*/
    name = 0;
    //filename = 0;
    free(filename);
    linenumber = 0;
    debuglevel = printlevel = 0;
    spaces = 0;
    RetValue.format = 0;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::print (const char *format, ...) {
    if ((name == 0) || (debuglevel < Dbg_Level_Internal)) return;

    va_list param;
    va_start (param, format);
    vRememberOutput (format, param);
    DbgPrintIntern ();
    va_end (param);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::printBuf (const void *data, unsigned length) {
    if (data && length && (debuglevel >= Dbg_Level_Internal)) {
        unsigned       i = 0;
        unsigned       j;
        unsigned       laufbuffree;
        char           outstring[DEBUGINFO_PRINTBUF_SIZE];
        char          *laufbuf;
        unsigned char *databuf;

        for (;;) {
            a_snprintf (outstring, DEBUGINFO_PRINTBUF_SIZE, "\n%05x: ", (i & 0xFFFFF));
            laufbuffree = DEBUGINFO_PRINTBUF_SIZE - 8;
            laufbuf = outstring + 8;
            databuf = (((unsigned char *)data) + i);
            for (j = 0; j < 0x10; j++) {
                if (i + j < length) {
                    if (j && (j % 4 == 0)) {
                        DbgIAssert (laufbuffree >= 5);
                        a_snprintf (laufbuf, laufbuffree, "' %02x ", *databuf);
                        laufbuffree -= 5;
                        laufbuf     += 5;
                    } else {
                        DbgIAssert (laufbuffree >= 3);
                        a_snprintf (laufbuf, laufbuffree, "%02x ", *databuf);
                        laufbuf += 3;
                    }
                    databuf++;
                } else {
                    if (j % 4 == 0) {
                        DbgIAssert (laufbuffree >= 5);
                        a_snprintf (laufbuf, laufbuffree, "     ");
                        laufbuf += 5;
                    } else {
                        DbgIAssert (laufbuffree >= 3);
                        a_snprintf (laufbuf, laufbuffree, "   ");
                        laufbuf += 3;
                    }
                }
            }
            DbgIAssert (laufbuffree >= 20);
            *(laufbuf++) = '|';
            *(laufbuf++) = ' ';
            databuf = (((unsigned char *)data) + i);
            for (j = 0; ((j < 0x10) && (i + j < length)); j++, laufbuf++, databuf++) {
                *laufbuf = (*databuf < 32) ? '.' : *databuf;
            }
            *(laufbuf++) = ' ';
            *laufbuf     = '\0';
            i            += 0x10;
            if (i >= length) {
                break;
            }
            RememberOutput ("%s", outstring);
            DbgPrintIntern();
        }
        RememberOutput ("%s\n", outstring);
        DbgPrintIntern();
    } else if (debuglevel >= Dbg_Level_Internal) {
        RememberOutput ("DATA: (p=%x, l=%x)\n", data, length);
        DbgPrintIntern();
    }
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

unsigned CDebugInfo::printValue (unsigned val, const char *sval, const unsigned int line) {
    //RememberOutput ("VALUE: '%s' in %s(%d) = %x\n", sval, filename, line, val);
    if (  (debuglevel >= Dbg_Level_Internal)
       || ((val != 0) && (debuglevel >= Dbg_Level_Short))) {
        RememberOutput ("VALUE: '%s' = %x\n", sval, filename, line, val);
        DbgPrintIntern();
    }
    return val;
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::printWrn (const char *format, ...) {
    if ((name == 0)|| (debuglevel < Dbg_Level_Infos)) return;

    va_list param;
    va_start (param, format);
    if (printlevel & Prt_Level_FileNameAtInfo) {
        RememberOutput ("%s[NOTE(%s in %s(%d+)): ", DbgIPrintLinePrefix, name, filename, linenumber);
        vRememberOutput (format, param);
        RememberOutput ("]\n");
    } else {
        RememberOutput ("%s[NOTE(%s): ", DbgIPrintLinePrefix, name);
        vRememberOutput (format, param);
        RememberOutput ("]\n");
    }
    DbgPrintIntern();
    va_end (param);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::printErr (const char *format, ...) {
    if ((name == 0) || (debuglevel < Dbg_Level_Errors)) return;

    va_list param;
    va_start (param, format);
    if (printlevel & Prt_Level_FileNameAtInfo) {
        RememberOutput ("%s[ERROR(%s in %s(%d+)): ", DbgIPrintLinePrefix, name, filename, linenumber);
        vRememberOutput (format, param);
        RememberOutput ("]\n");
    } else {
        RememberOutput ("%s[ERROR(%s): ", DbgIPrintLinePrefix, name);
        vRememberOutput (format, param);
        RememberOutput ("]\n");
    }
    DbgPrintIntern();
    va_end (param);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::assertion (const char *exp, const unsigned int line) {
    if ((name == 0) || (debuglevel < Dbg_Level_Errors)) return;

    if (printlevel & Prt_Level_FuncNameAtError) {
        RememberOutput ("%s[assertion failed: (%s) %s in %s(%d)]\n", DbgIPrintLinePrefix, exp, name, filename, line);
    } else {
        RememberOutput ("%s[assertion failed: (%s) in %s(%d)]\n", DbgIPrintLinePrefix, exp, filename, line);
    }
    DbgPrintIntern();
    if (BreakPointsAreActive) {
        DbgIBreakPoint();
    }
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::assertion (const int expValue, const char *exp, const char *fname,
                            const unsigned int line) {
    if (expValue || !name || (debuglevel < Dbg_Level_Errors)) return;
    unsigned last = 0;
    for (unsigned i = 0; fname[i] != '\0'; i++) {
        if ((fname[i] == '\\') || (fname[i] =='/')) {
            last = i + 1;
        }
    }
    fname += last;
    if (printlevel & Prt_Level_FuncNameAtError) {
        RememberOutput ("%s[assertion failed: (%s) %s in %s(%d)]\n", DbgIPrintLinePrefix, exp, name, fname, line);
    } else {
        RememberOutput ("%s[assertion failed: (%s) in %s(%d)]\n", DbgIPrintLinePrefix, exp, fname, line);
    }
    DbgPrintIntern();
    if (BreakPointsAreActive) {
        DbgIBreakPoint();
    }
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::warning (const char *exp, const unsigned int line) {
    if (!name || (debuglevel < Dbg_Level_Infos)) return;

    if (printlevel & Prt_Level_FuncNameAtError) {
        RememberOutput ("%s[warning: (%s) %s in %s(%d)]\n", DbgIPrintLinePrefix, exp, name, filename, line);
    } else {
        RememberOutput ("%s[warning: (%s) in %s(%d)]\n", DbgIPrintLinePrefix, exp, filename, line);
    }
    DbgPrintIntern();
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::warning (const int expValue, const char *exp, const char *fname, const unsigned int line) {
    if (expValue || !name || (debuglevel < Dbg_Level_Infos)) return;
    unsigned last = 0;
    for (unsigned i = 0; fname[i] != '\0'; i++) {
        if ((fname[i] == '\\') || (fname[i] =='/')) {
            last = i + 1;
        }
    }
    fname += last;
    if (printlevel & Prt_Level_FuncNameAtError) {
        RememberOutput ("%s[warning: (%s) %s in %s(%d)]\n", DbgIPrintLinePrefix, exp, name, fname, line);
    } else {
        RememberOutput ("%s[warning: (%s) in %s(%d)]\n", DbgIPrintLinePrefix, exp, fname, line);
    }
    DbgPrintIntern();
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::info (const int expression, const char *format, ...) {
    if (expression || !name || (debuglevel < Dbg_Level_Infos)) return;
    va_list param;
    va_start (param, format);
    if (printlevel & Prt_Level_FileNameAtInfo) {
        RememberOutput ("%s[info(%s in %s(%d+)): ", DbgIPrintLinePrefix, name, filename, linenumber);
        vRememberOutput (format, param);
        RememberOutput ("]\n");
    } else {
        RememberOutput ("%s[info(%s): ", DbgIPrintLinePrefix, name);
        vRememberOutput (format, param);
        RememberOutput ("]\n");
    }
    DbgPrintIntern();
    va_end (param);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/
#include <stdio.h>

void CDebugInfo::PrintParams (const char *format, ...) {
    if (name == 0) return;

    va_list param;
    va_start (param, format);
    switch (debuglevel) {
    case Dbg_Level_Short:
        RememberOutput (PARAMETERS_SHORT_BEGIN);
        vRememberOutput (format, param);
        RememberOutput (PARAMETERS_SHORT_END);
        DbgPrintIntern();
        break;
    case Dbg_Level_Entry:
    case Dbg_Level_Internal:
        RememberOutput (PARAMETERS_BEGIN);
        vRememberOutput (format, param);
        RememberOutput (PARAMETERS_END);
        DbgPrintIntern();
        break;
    }
    va_end (param);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::SetRetVal (const char format, ...) {
    va_list param;
    va_start (param, format);

    RetValue.format = format;
    // Lieber einen kleine Hack, weil schneller !!!
    // Es wird sich der speicher-größte Parameter gemerkt und
    // erst bei der Ausgabe die Fallabfrage gemacht.
    RetValue.p = va_arg (param, void *);
    va_end (param);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::ShowAllOutputs (void) {
    if (!Output.IsEmpty()) {
        char *outstr = Output.GetAll();
        DbgIPrintStr (outstr);
    }
    /*----- TODO: Funktionsnamen ausgeben, immer oder wennn nichts in der OutputQueue -----*/
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::RememberOutput (const char *format, ...) {
    va_list param;
    va_start (param, format);
    vRememberOutput (format, param);
    va_end (param);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::vRememberOutput (const char *format, va_list param) {
    if (printlevel & Prt_Level_NameInEveryLine) {
        Output.AddTo (spaces, NAMEINEVERYLINE_BEGIN "%s" NAMEINEVERYLINE_END, name);
    }
    Output.vAddTo (spaces, format, param);
}

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

void CDebugInfo::DbgPrintIntern (void) {
    DbgIPrintStr (Output.GetAll());
}


/*===========================================================================*\
\*===========================================================================*/
#endif /* #elif (USE_DEBUG_INFO_CLASS >= _USE_DEBUG_INFO_CLASS_NORMAL) */
