/*
    Copyright 1999-2003,2007 TiANWEi
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


//-------------------------------------------------------------
// Show error message
//-------------------------------------------------------------
VOID ErrMsg(LPCSTR note)
{
    MessageBox(hWnd, note, asLangTexts[iszTextError].lpString, MB_ICONHAND);
}


//-------------------------------------------------------------
// Routine to debug
//-------------------------------------------------------------
#ifdef DEBUGLOG
extern char *szCRLF;

VOID DebugLog(LPSTR filename, LPSTR lpstr, HWND hDlg, BOOL bisCR)
{
    DWORD   length;
    DWORD   nPos;

    hFile = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        ErrMsg(asLangTexts[iszTextErrorCreateFile].lpString);
    } else {
        nPos = SetFilePointer(hFile, 0, NULL, FILE_END);
        if (nPos == 0xFFFFFFFF) {
            ErrMsg(asLangTexts[iszTextErrorMoveFP].lpString);
        } else {

            length = strlen(lpstr);
            WriteFile(hFile, lpstr, length, &NBW, NULL);
            if (NBW != length) {
                //ErrMsg(asLangTexts[iszTextErrorWriteFile].lpString);
            }
            if (bisCR == TRUE) {
                WriteFile(hFile, szCRLF, sizeof(szCRLF) - 1, &NBW, NULL);
            }
        }
    }
    CloseHandle(hFile);
}
#endif


//------------------------------------------------------------
// Routine to replace invalid chars in comment fields
//------------------------------------------------------------
BOOL ReplaceInValidFileName(LPSTR lpf)
{
    char lpInvalid[] = "\\/:*?\"<>|"; // 1.8.2
    DWORD   i, j;
    size_t  nLen;
    BOOL    bLegal = FALSE;
    nLen = strlen(lpf);

    for (i = 0; i < nLen; i++) {
        for (j = 0; j < sizeof(lpInvalid) - 1; j++) { // changed at 1.8.2 from 9 to sizeof()-1
            if (*(lpf + i) == *(lpInvalid + j)) {
                *(lpf + i) = '-';                     // 0x2D; check for invalid chars and replace it (return FALSE;)
            } else if (*(lpf + i) != 0x20 && *(lpf + i) != 0x09) { // At least one non-space, non-tab char needed!
                bLegal = TRUE;
            }

        }
    }
    return bLegal;
}


// ----------------------------------------------------------------------
// Find lpszSearch's position in lpgrszSection
//
// Functionality is similar to GetPrivateProfileString(), but return value is a
// pointer inside a memory block with all strings (normally a section buffer),
// which avoids handling multiple pointers individually.
// The section buffer must not contain unnecessary whitespaces, comments,
// empty lines, etc. Windows' GetPrivateProfileSection() already cares about this.
// ----------------------------------------------------------------------
LPTSTR FindKeyInIniSection(LPTSTR lpgrszSection, LPTSTR lpszSearch, size_t cchSectionLen, size_t cchSearchLen)
{
    size_t i;
    size_t nszSectionLen;

    if ((NULL == lpgrszSection) || (NULL == lpszSearch) || (1 > cchSearchLen)) {
        return NULL;
    }

    for (i = 0; i < cchSectionLen; i++) {
        if (0 == lpgrszSection[i]) {  // reached the end of the section buffer
            break;
        }

        nszSectionLen = _tcslen(&lpgrszSection[i]);
        if (nszSectionLen <= cchSearchLen) {
            i += nszSectionLen;  // skip this string as it is too short (or would result in an empty return string)
            continue;
        }

        if (0 == _tcsnccmp(&lpgrszSection[i], lpszSearch, cchSearchLen)) {
            return &lpgrszSection[(i + cchSearchLen)];
        }

        i += nszSectionLen;  // skip this string as it does not match
    }

    return NULL;
}
