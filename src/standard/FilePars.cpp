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

#include "FilePars.h"
#include "dbgSTD.h"

/*---------------------------------------------------------------------------*\
\*---------------------------------------------------------------------------*/

#define CONFIGPARSER_MaxLength              200

/*===========================================================================*\
\*===========================================================================*/

tSInfo CConfigFileParser::ParseFile (tUChar *FileName) {
    dhead ("CConfigFileParser::ParseFile", DCON_ConfigParser);
    dassert (FileName != 0);
    tSInfo fret;
    FILE *hFile = fopen ((char *)FileName, "r");
    if (!hFile) {
        return iErr_File_OpenFailed;
    }
    fret = ParseFile (hFile);
    fclose (hFile);
    return fret;
}

/*===========================================================================*\
\*===========================================================================*/

tSInfo CConfigFileParser::ParseFile (FILE *FileHandle) {
    dhead ("CConfigFileParser::ParseFile", DCON_ConfigParser);
    dassert (FileHandle != 0);
    char           szPartLine[CONFIGPARSER_MaxLength + 1];
    CDynamicString curLine;
    tUInt          curLineNr = 0;
    tSInfo         fret;

    if (!GetMainSection()) {
        return iErr_OutOfMemory;
    }
    // TODO?: set file position to begin of file?
    // fseek (FileHandle, 0, SEEK_SET);
    while (!feof (FileHandle)) {
        if (fgets (szPartLine, CONFIGPARSER_MaxLength, FileHandle)) {
            if (curLine.Append ((tUChar *)szPartLine) == vFalse) {
                return iErr_OutOfMemory;
            }
            if (curLine.GetLen()) {
                switch (curLine.GetLastChar()) {
                case '\r':
                case '\n':
                    do {
                        curLine.CutLen (1);
                    } while (  (curLine.GetLastChar() == '\r')
                            || (curLine.GetLastChar() == '\n'));
                    dprint ("ParseLine=(%s)\n", curLine.GetPointer());
                    fret = ParseLine (&curLine);
                    switch (fret) {
                    case i_Okay:
                    case i_Pending:
                        break;
                    case iErr_OutOfMemory:
                        return fret;
                    default:
                        LineCodingError (++curLineNr, &curLine);
                        break;
                    }
                    break;
                }
            }
        }
        curLine.RemoveAll();
    }
    return (ParseLine (0) == iErr_OutOfMemory) ? iErr_OutOfMemory : i_Okay;
}


/*===========================================================================*\
\*===========================================================================*/
