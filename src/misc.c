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


// ----------------------------------------------------------------------
// Show error message
// ----------------------------------------------------------------------
VOID ErrMsg(LPTSTR lpszErrMsg)
{
    MessageBox(hWnd, lpszErrMsg, asLangTexts[iszTextError].lpString, MB_ICONHAND);
}


#ifdef DEBUGLOG
// debug log files
TCHAR szDebugTryToGetValueLog[] = TEXT("debug_trytogetvalue.log");
TCHAR szDebugValueNameDataLog[] = TEXT("debug_valuenamedata.log");
TCHAR szDebugKeyLog[] = TEXT("debug_key.log");

// ----------------------------------------------------------------------
// Write message to debug log file
// ----------------------------------------------------------------------
VOID DebugLog(LPTSTR lpszFileName, LPTSTR lpszDbgMsg, BOOL fAddCRLF)
{
    size_t nLen;
    DWORD nPos;

    hFile = CreateFile(lpszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile) {
        ErrMsg(asLangTexts[iszTextErrorCreateFile].lpString);
        return;
    }

    nPos = SetFilePointer(hFile, 0, NULL, FILE_END);
    if (0xFFFFFFFF == nPos) {
        ErrMsg(asLangTexts[iszTextErrorMoveFP].lpString);
    } else {
        nLen = _tcslen(lpszDbgMsg) * sizeof(TCHAR);
        WriteFile(hFile, lpszDbgMsg, (DWORD)nLen, &NBW, NULL);
        if (NBW != nLen) {
            //ErrMsg(asLangTexts[iszTextErrorWriteFile].lpString);
        }
        if (fAddCRLF) {
            WriteFile(hFile, szCRLF, (DWORD)(_tcslen(szCRLF) * sizeof(TCHAR)), &NBW, NULL);
        }
    }

    CloseHandle(hFile);
}
#endif


// ----------------------------------------------------------------------
// Replace invalid chars for a file name
// ----------------------------------------------------------------------
TCHAR szInvalid[] = TEXT("\\/:*?\"<>|");  // 1.8.2

BOOL ReplaceInvalidFileNameChars(LPTSTR lpszFileName)
{
    size_t nInvalidLen;
    size_t nFileNameLen;
    size_t i, j;
    BOOL fFileNameIsLegal;
    BOOL fCharIsValid;

    nInvalidLen = _tcslen(szInvalid);
    nFileNameLen = _tcslen(lpszFileName);

    fFileNameIsLegal = FALSE;
    for (i = 0; i < nFileNameLen; i++) {
        fCharIsValid = TRUE;  // valid chars expected

        if ((TCHAR)'\t' == lpszFileName[i]) {  // replace tab with space
            lpszFileName[i] = (TCHAR)' ';
        } else {  // replace invalid char with underscore
            for (j = 0; j < nInvalidLen; j++) {
                if (lpszFileName[i] == szInvalid[j]) {
                    lpszFileName[i] = (TCHAR)'_';
                    fCharIsValid = FALSE;
                    break;
                }
            }
        }

        if ((fCharIsValid) && ((TCHAR)' ' != lpszFileName[i])) {  // At least one valid non-space char needed
            fFileNameIsLegal = TRUE;
        }
    }
    return fFileNameIsLegal;
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
