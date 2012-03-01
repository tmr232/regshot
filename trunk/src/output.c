/*
    Copyright 1999-2003,2007,2011 TiANWEi
    Copyright 2007 Belogorokhov Youri
    Copyright 2011-2012 Regshot Team

    This file is part of Regshot.

    Regshot is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 2.1 of the License, or
    (at your option) any later version.

    Regshot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Regshot.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "global.h"

// Some strings used to write to HTML or TEXT file
TCHAR szCRLF[]             = TEXT("\r\n");  // {0x0d,0x0a,0x00};
TCHAR szTextLine[]         = TEXT("\r\n----------------------------------\r\n");
TCHAR szHTML_BR[]          = TEXT("<BR>\r\n");
TCHAR szHTMLBegin[]        = TEXT("<HTML>\r\n");
TCHAR szHTMLEnd[]          = TEXT("</HTML>\r\n");
TCHAR szHTMLHeadBegin[]    = TEXT("<HEAD>\r\n");
TCHAR szHTML_CType[]       =
#ifdef _UNICODE
    TEXT("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-16\">\r\n");
#else
    TEXT("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=ISO-8859-1\">\r\n");
#endif
TCHAR szHTMLHeadEnd[]      = TEXT("</HEAD>\r\n");
TCHAR szHTMLTd1Begin[]     = TEXT("<TR><TD BGCOLOR=\"#669999\" ALIGN=\"LEFT\"><FONT COLOR=\"WHITE\"><B>");
TCHAR szHTMLTd1End[]       = TEXT("</B></FONT></TD></TR>\r\n");
TCHAR szHTMLTd2Begin[]     = TEXT("<TR><TD NOWRAP><FONT COLOR=\"BLACK\">");
TCHAR szHTMLTd2End[]       = TEXT("</FONT></TD></TR>\r\n");
// color idea got from HANDLE(Youri) at wgapatcher.ru :) 1.8
TCHAR szHTML_CSS[]         = TEXT("<STYLE TYPE = \"text/css\">td{font-family:\"Tahoma\";font-size:9pt}\
tr{font-size:9pt}body{font-size:9pt}\
.o{background:#E0F0E0}.n{background:#FFFFFF}</STYLE>\r\n");  // 1.8.2 from e0e0e0 to e0f0e0 by Charles
TCHAR szHTMLBodyBegin[]    = TEXT("<BODY BGCOLOR=\"#FFFFFF\" TEXT=\"#000000\" LINK=\"#C8C8C8\">\r\n");
TCHAR szHTMLBodyEnd[]      = TEXT("</BODY>\r\n");
TCHAR szHTMLTableBegin[]   = TEXT("<TABLE BORDER=\"0\" WIDTH=\"480\">\r\n");
TCHAR szHTMLTableEnd[]     = TEXT("</TABLE>\r\n");
TCHAR szHTMLSpan1[]        = TEXT("<SPAN CLASS=\"o\">");
TCHAR szHTMLSpan2[]        = TEXT("<SPAN CLASS=\"n\">");
TCHAR szHTMLSpanEnd[]      = TEXT("</SPAN>");
TCHAR szHTMLWebSiteBegin[] = TEXT("<FONT COLOR=\"#888888\">Created with <A HREF=\"http://sourceforge.net/projects/regshot/\">");
TCHAR szHTMLWebSiteEnd[]   = TEXT("</A></FONT><BR>\r\n");

HANDLE hFile;  // Handle of file regshot use


// ----------------------------------------------------------------------
// Several routines to write to an output file
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
VOID WriteTableHead(LPTSTR lpszText, DWORD nCount, BOOL fAsHTML)
{
    TCHAR szCount[17];

    szCount[16] = 0;
    _sntprintf(szCount, 16, TEXT("%u"), nCount);

    if (fAsHTML) {
        WriteFile(hFile, szHTML_BR, (DWORD)(_tcslen(szHTML_BR) * sizeof(TCHAR)), &NBW, NULL);
        WriteFile(hFile, szHTMLTableBegin, (DWORD)(_tcslen(szHTMLTableBegin) * sizeof(TCHAR)), &NBW, NULL);
        WriteFile(hFile, szHTMLTd1Begin, (DWORD)(_tcslen(szHTMLTd1Begin) * sizeof(TCHAR)), &NBW, NULL);
    } else {
        WriteFile(hFile, szTextLine, (DWORD)(_tcslen(szTextLine) * sizeof(TCHAR)), &NBW, NULL);
    }

    WriteFile(hFile, lpszText, (DWORD)(_tcslen(lpszText) * sizeof(TCHAR)), &NBW, NULL);
    WriteFile(hFile, szCount, (DWORD)(_tcslen(szCount) * sizeof(TCHAR)), &NBW, NULL);

    if (fAsHTML) {
        WriteFile(hFile, szHTMLTd1End, (DWORD)(_tcslen(szHTMLTd1End) * sizeof(TCHAR)), &NBW, NULL);
        WriteFile(hFile, szHTMLTableEnd, (DWORD)(_tcslen(szHTMLTableEnd) * sizeof(TCHAR)), &NBW, NULL);
    } else {
        WriteFile(hFile, szTextLine, (DWORD)(_tcslen(szTextLine) * sizeof(TCHAR)), &NBW, NULL);
    }
}

// ----------------------------------------------------------------------
VOID WritePart(LPCOMRESULT lpComResultStart, BOOL fAsHTML, BOOL fUseColor)
{
    size_t fColor;  // color flip-flop flag
    size_t nCharsToWrite;
    size_t nCharsToGo;
    LPTSTR lpszResult;
    LPCOMRESULT lpComResult;

    if (fAsHTML) {
        WriteFile(hFile, szHTMLTableBegin, (DWORD)(_tcslen(szHTMLTableBegin) * sizeof(TCHAR)), &NBW, NULL);
        WriteFile(hFile, szHTMLTd2Begin, (DWORD)(_tcslen(szHTMLTd2Begin) * sizeof(TCHAR)), &NBW, NULL);
    }

    fColor = 0;
    for (lpComResult = lpComResultStart; NULL != lpComResult; lpComResult = lpComResult->lpnextresult) {
        if (fAsHTML) {
            // 1.8.0: zebra/flip-flop colors
            if (fUseColor) {
                if (0 == fColor) {
                    WriteFile(hFile, szHTMLSpan1, (DWORD)(_tcslen(szHTMLSpan1) * sizeof(TCHAR)), &NBW, NULL);
                } else {
                    WriteFile(hFile, szHTMLSpan2, (DWORD)(_tcslen(szHTMLSpan2) * sizeof(TCHAR)), &NBW, NULL);
                }
                fColor = 1 - fColor;
            }
        }

        lpszResult = lpComResult->lpresult;
        for (nCharsToGo = _tcslen(lpszResult); 0 < nCharsToGo;) {
            nCharsToWrite = nCharsToGo;
            if (HTMLWRAPLENGTH < nCharsToWrite) {
                nCharsToWrite = HTMLWRAPLENGTH;
            }

            WriteFile(hFile, lpszResult, (DWORD)(nCharsToWrite * sizeof(TCHAR)), &NBW, NULL);
            lpszResult += nCharsToWrite;
            nCharsToGo -= nCharsToWrite;

            if (0 == nCharsToGo) {
                break;  // skip newline
            }

            if (fAsHTML) {
                WriteFile(hFile, szHTML_BR, (DWORD)(_tcslen(szHTML_BR) * sizeof(TCHAR)), &NBW, NULL);
            } else {
                WriteFile(hFile, szCRLF, (DWORD)(_tcslen(szCRLF) * sizeof(TCHAR)), &NBW, NULL);
            }
        }

        if (fAsHTML) {
            if (fUseColor) {
                WriteFile(hFile, szHTMLSpanEnd, (DWORD)(_tcslen(szHTMLSpanEnd) * sizeof(TCHAR)), &NBW, NULL);
            }
            WriteFile(hFile, szHTML_BR, (DWORD)(_tcslen(szHTML_BR) * sizeof(TCHAR)), &NBW, NULL);
        } else {
            WriteFile(hFile, szCRLF, (DWORD)(_tcslen(szCRLF) * sizeof(TCHAR)), &NBW, NULL);
        }
    }

    if (fAsHTML) {
        WriteFile(hFile, szHTMLTd2End, (DWORD)(_tcslen(szHTMLTd2End) * sizeof(TCHAR)), &NBW, NULL);
        WriteFile(hFile, szHTMLTableEnd, (DWORD)(_tcslen(szHTMLTableEnd) * sizeof(TCHAR)), &NBW, NULL);
    }
}

// ----------------------------------------------------------------------
VOID WriteTitle(LPTSTR lpszTitle, LPTSTR lpszValue, BOOL fAsHTML)
{
    if (fAsHTML) {
        WriteFile(hFile, szHTMLTableBegin, (DWORD)(_tcslen(szHTMLTableBegin) * sizeof(TCHAR)), &NBW, NULL);
        WriteFile(hFile, szHTMLTd1Begin, (DWORD)(_tcslen(szHTMLTd1Begin) * sizeof(TCHAR)), &NBW, NULL);
    }

    WriteFile(hFile, lpszTitle, (DWORD)(_tcslen(lpszTitle) * sizeof(TCHAR)), &NBW, NULL);
    WriteFile(hFile, lpszValue, (DWORD)(_tcslen(lpszValue) * sizeof(TCHAR)), &NBW, NULL);

    if (fAsHTML) {
        WriteFile(hFile, szHTMLTd1End, (DWORD)(_tcslen(szHTMLTd1End) * sizeof(TCHAR)), &NBW, NULL);
        WriteFile(hFile, szHTMLTableEnd, (DWORD)(_tcslen(szHTMLTableEnd) * sizeof(TCHAR)), &NBW, NULL);
    } else {
        WriteFile(hFile, szCRLF, (DWORD)(_tcslen(szCRLF) * sizeof(TCHAR)), &NBW, NULL);
    }
}

// ----------------------------------------------------------------------
VOID WriteHTMLBegin(void)
{
    WriteFile(hFile, szHTMLBegin, (DWORD)(_tcslen(szHTMLBegin) * sizeof(TCHAR)), &NBW, NULL);
    WriteFile(hFile, szHTMLHeadBegin, (DWORD)(_tcslen(szHTMLHeadBegin) * sizeof(TCHAR)), &NBW, NULL);
    WriteFile(hFile, szHTML_CType, (DWORD)(_tcslen(szHTML_CType) * sizeof(TCHAR)), &NBW, NULL);
    WriteFile(hFile, szHTML_CSS, (DWORD)(_tcslen(szHTML_CSS) * sizeof(TCHAR)), &NBW, NULL);
    WriteFile(hFile, szHTMLHeadEnd, (DWORD)(_tcslen(szHTMLHeadEnd) * sizeof(TCHAR)), &NBW, NULL);
    WriteFile(hFile, szHTMLBodyBegin, (DWORD)(_tcslen(szHTMLBodyBegin) * sizeof(TCHAR)), &NBW, NULL);

    WriteFile(hFile, szHTMLWebSiteBegin, (DWORD)(_tcslen(szHTMLWebSiteBegin) * sizeof(TCHAR)), &NBW, NULL);
    WriteFile(hFile, lpszProgramName, (DWORD)(_tcslen(lpszProgramName) * sizeof(TCHAR)), &NBW, NULL);
    WriteFile(hFile, szHTMLWebSiteEnd, (DWORD)(_tcslen(szHTMLWebSiteEnd) * sizeof(TCHAR)), &NBW, NULL);
}

// ----------------------------------------------------------------------
VOID WriteHTMLEnd(void)
{
    WriteFile(hFile, szHTMLBodyEnd, (DWORD)(_tcslen(szHTMLBodyEnd) * sizeof(TCHAR)), &NBW, NULL);
    WriteFile(hFile, szHTMLEnd, (DWORD)(_tcslen(szHTMLEnd) * sizeof(TCHAR)), &NBW, NULL);
}

// ----------------------------------------------------------------------
VOID WriteHTML_BR(void)
{
    WriteFile(hFile, szHTML_BR, (DWORD)(_tcslen(szHTML_BR) * sizeof(TCHAR)), &NBW, NULL);
}
