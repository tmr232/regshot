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

LANGUAGETEXT asLangTexts[cLangStrings];

LPTSTR szDefaultLanguage = TEXT("English");
LPTSTR szItemTranslator  = TEXT("Translator=");
LPTSTR szSectionCurrent  = TEXT("CURRENT");
LPTSTR szOriginal        = TEXT("[Original]");

LPTSTR lpszLanguage;
size_t cchMaxLanguageNameLen;

LPTSTR lpgrszLangSection;


// ----------------------------------------------------------------------
// Initialize text strings with English defaults
// ----------------------------------------------------------------------
VOID SetTextsToDefaultLanguage(VOID)
{
    // Clear all structures (pointers, IDs, etc.)
    ZeroMemory(asLangTexts, sizeof(asLangTexts));

    // Set English default language strings
    asLangTexts[iszTextKey].lpString               = TEXT("Keys:");
    asLangTexts[iszTextValue].lpString             = TEXT("Values:");
    asLangTexts[iszTextDir].lpString               = TEXT("Dirs:");
    asLangTexts[iszTextFile].lpString              = TEXT("Files:");
    asLangTexts[iszTextTime].lpString              = TEXT("Time:");
    asLangTexts[iszTextKeyAdd].lpString            = TEXT("Keys added:");
    asLangTexts[iszTextKeyDel].lpString            = TEXT("Keys deleted:");
    asLangTexts[iszTextValAdd].lpString            = TEXT("Values added:");
    asLangTexts[iszTextValDel].lpString            = TEXT("Values deleted:");
    asLangTexts[iszTextValModi].lpString           = TEXT("Values modified:");
    asLangTexts[iszTextFileAdd].lpString           = TEXT("Files added:");
    asLangTexts[iszTextFileDel].lpString           = TEXT("Files deleted:");
    asLangTexts[iszTextFileModi].lpString          = TEXT("Files [attributes?] modified:");
    asLangTexts[iszTextDirAdd].lpString            = TEXT("Folders added:");
    asLangTexts[iszTextDirDel].lpString            = TEXT("Folders deleted:");
    asLangTexts[iszTextDirModi].lpString           = TEXT("Folders attributes changed:");
    asLangTexts[iszTextTotal].lpString             = TEXT("Total changes:");
    asLangTexts[iszTextComments].lpString          = TEXT("Comments:");
    asLangTexts[iszTextDateTime].lpString          = TEXT("Datetime:");
    asLangTexts[iszTextComputer].lpString          = TEXT("Computer:");
    asLangTexts[iszTextUsername].lpString          = TEXT("Username:");
    asLangTexts[iszTextAbout].lpString             = TEXT("About");
    asLangTexts[iszTextError].lpString             = TEXT("Error");
    asLangTexts[iszTextErrorExecViewer].lpString   = TEXT("Error call External Viewer!");
    asLangTexts[iszTextErrorCreateFile].lpString   = TEXT("Error creating file!");
    asLangTexts[iszTextErrorOpenFile].lpString     = TEXT("Error open file!");
    asLangTexts[iszTextErrorMoveFP].lpString       = TEXT("Error move file pointer!");

    asLangTexts[iszTextButtonShot1].lpString       = TEXT("&1st shot");
    asLangTexts[iszTextButtonShot1].nIDDlgItem     = IDC_1STSHOT;

    asLangTexts[iszTextButtonShot2].lpString       = TEXT("&2nd shot");
    asLangTexts[iszTextButtonShot2].nIDDlgItem     = IDC_2NDSHOT;

    asLangTexts[iszTextButtonCompare].lpString     = TEXT("C&ompare");
    asLangTexts[iszTextButtonCompare].nIDDlgItem   = IDC_COMPARE;

    asLangTexts[iszTextButtonClear].lpString       = TEXT("&Clear");
    asLangTexts[iszTextButtonClear].nIDDlgItem     = IDC_CLEAR1;

    asLangTexts[iszTextButtonQuit].lpString        = TEXT("&Quit");
    asLangTexts[iszTextButtonQuit].nIDDlgItem      = IDC_CANCEL1;

    asLangTexts[iszTextButtonAbout].lpString       = TEXT("&About");
    asLangTexts[iszTextButtonAbout].nIDDlgItem     = IDC_ABOUT;

    asLangTexts[iszTextTextMonitor].lpString       = TEXT("&Monitor..");
    //asLangTexts[iszTextTextMonitor].nIDDlgItem     = IDC_MONITOR;

    asLangTexts[iszTextTextCompare].lpString       = TEXT("Compare logs save as:");
    asLangTexts[iszTextTextCompare].nIDDlgItem     = IDC_STATICSAVEFORMAT;

    asLangTexts[iszTextTextOutput].lpString        = TEXT("Output path:");
    asLangTexts[iszTextTextOutput].nIDDlgItem      = IDC_STATICOUTPUTPATH;

    asLangTexts[iszTextTextComment].lpString       = TEXT("Add comment into the log:");
    asLangTexts[iszTextTextComment].nIDDlgItem     = IDC_STATICADDCOMMENT;

    asLangTexts[iszTextRadioPlain].lpString        = TEXT("Plain &TXT");
    asLangTexts[iszTextRadioPlain].nIDDlgItem      = IDC_RADIO1;

    asLangTexts[iszTextRadioHTML].lpString         = TEXT("&HTML document");
    asLangTexts[iszTextRadioHTML].nIDDlgItem       = IDC_RADIO2;

    asLangTexts[iszTextTextScan].lpString          = TEXT("&Scan dir1[;dir2;dir3;...;dir nn]:");
    asLangTexts[iszTextTextScan].nIDDlgItem        = IDC_CHECKDIR;

    asLangTexts[iszTextMenuShot].lpString          = TEXT("&Shot");
    asLangTexts[iszTextMenuShotSave].lpString      = TEXT("Shot and Sa&ve...");
    asLangTexts[iszTextMenuShotLoad].lpString      = TEXT("Loa&d...");
    asLangTexts[iszTextMenuClearAllShots].lpString = TEXT("&Clear All");
    asLangTexts[iszTextMenuClearShot1].lpString    = TEXT("Clear &1st shot");
    asLangTexts[iszTextMenuClearShot2].lpString    = TEXT("Clear &2nd shot");

    // Set translator too
    lpCurrentTranslator = szOriginal;
}

// ----------------------------------------------------------------------
// Get available languages from language ini and add to combo box
// An English section in language ini will be ignored
// ----------------------------------------------------------------------
VOID LoadAvailableLanguagesFromIni(HWND hDlg)
{
    LRESULT nResult;
    LPTSTR lpgrszSectionNames;
    DWORD cchSectionNames;
    size_t i;
    size_t nLanguageNameLen;

    // Always add default language to combo box and select it as default
    nResult = SendDlgItemMessage(hDlg, IDC_COMBOLANGUAGE, CB_ADDSTRING, (WPARAM)0, (LPARAM)szDefaultLanguage);  // TODO: handle CB_ERR and CB_ERRSPACE
    SendDlgItemMessage(hDlg, IDC_COMBOLANGUAGE, CB_SETCURSEL, (WPARAM)nResult, (LPARAM)0);
    cchMaxLanguageNameLen = _tcslen(szDefaultLanguage);

    // Get sections (=language names) from language ini
    lpgrszSectionNames = MYALLOC0(MAX_INI_SECTION_CHARS * sizeof(TCHAR));
    cchSectionNames = GetPrivateProfileSectionNames(lpgrszSectionNames, MAX_INI_SECTION_CHARS, lpLanguageIni);
    if (1 < cchSectionNames) {
        for (i = 0; i < cchSectionNames; i++) {
            if (0 == lpgrszSectionNames[i]) {  // reached the end of the section names buffer
                break;
            }

            nLanguageNameLen = _tcslen(&lpgrszSectionNames[i]);

            if ((0 != _tcsicmp(&lpgrszSectionNames[i], szSectionCurrent)) && (0 != _tcsicmp(&lpgrszSectionNames[i], szDefaultLanguage))) {
                nResult = SendDlgItemMessage(hDlg, IDC_COMBOLANGUAGE, CB_ADDSTRING, (WPARAM)0, (LPARAM)&lpgrszSectionNames[i]);  // TODO: handle CB_ERR and CB_ERRSPACE
                if (nLanguageNameLen > cchMaxLanguageNameLen) {
                    cchMaxLanguageNameLen = nLanguageNameLen;
                }
            }

            i += nLanguageNameLen;  // skip to next string
        }
    }
    MYFREE(lpgrszSectionNames);

    // Allocate memory for longest language name, and copy default language name to it
    lpszLanguage = MYALLOC0((cchMaxLanguageNameLen + 1) * sizeof(TCHAR));
    _tcscpy(lpszLanguage, szDefaultLanguage);
}

// ----------------------------------------------------------------------
// Get selected language name and check if it is available
// ----------------------------------------------------------------------
BOOL GetSelectedLanguage(HWND hDlg)
{
    LPTSTR lpszSelectedLanguage;
    DWORD cchLanguageName;
    LRESULT nResult;

    lpszSelectedLanguage = MYALLOC0((cchMaxLanguageNameLen + 1) * sizeof(TCHAR));
    cchLanguageName = GetPrivateProfileString(szSectionCurrent, szSectionCurrent, NULL, lpszLanguage, (DWORD)(cchMaxLanguageNameLen + 1), lpLanguageIni);
    if (1 < cchLanguageName) {
        nResult = SendDlgItemMessage(hDlg, IDC_COMBOLANGUAGE, CB_FINDSTRINGEXACT, (WPARAM)0, (LPARAM)lpszLanguage);
        if (CB_ERR != nResult) {
            SendDlgItemMessage(hDlg, IDC_COMBOLANGUAGE, CB_SETCURSEL, (WPARAM)nResult, (LPARAM)0);
            MYFREE(lpszSelectedLanguage);
            return TRUE;
        }
    }
    MYFREE(lpszSelectedLanguage);
    return FALSE;
}

// ----------------------------------------------------------------------
// Set text strings to selected language
// ----------------------------------------------------------------------
VOID SetTextsToSelectedLanguage(HWND hDlg)
{
    LRESULT nResult, nResult2;
    DWORD cchSection;
    int i;
    LPTSTR lpszMatchValue;
    TCHAR  szIniKey[17];

    // Get language index from combo box
    nResult = SendDlgItemMessage(hDlg, IDC_COMBOLANGUAGE, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    if (CB_ERR == nResult) {
        return;
    }

    // Get language name of language index from combo box
    nResult2 = SendDlgItemMessage(hDlg, IDC_COMBOLANGUAGE, CB_GETLBTEXTLEN, (WPARAM)nResult, (LPARAM)NULL);
    if ((CB_ERR == nResult2) || ((size_t)nResult2 > cchMaxLanguageNameLen)) {
        return;
    }
    nResult = SendDlgItemMessage(hDlg, IDC_COMBOLANGUAGE, CB_GETLBTEXT, (WPARAM)nResult, (LPARAM)lpszLanguage);
    if (CB_ERR == nResult) {
        return;
    }

    // Write new language selection to language ini
    WritePrivateProfileString(szSectionCurrent, szSectionCurrent, lpszLanguage, lpLanguageIni);

    // Nothing more to do for default language
    if (0 == _tcsicmp(lpszLanguage, szDefaultLanguage)) {
        return;
    }

    // Get ini section of language
    if (NULL == lpgrszLangSection) {
        lpgrszLangSection = MYALLOC0(MAX_INI_SECTION_CHARS * sizeof(TCHAR));
    }
    cchSection = GetPrivateProfileSection(lpszLanguage, lpgrszLangSection, MAX_INI_SECTION_CHARS, lpLanguageIni);

    // Find language strings and assign if not empty
    szIniKey[16] = 0;  // saftey NULL char
    for (i = 0; i < cLangStrings; i++) {
        _sntprintf(szIniKey, 16, TEXT("%u%s"), (i + 1), TEXT("="));
        lpszMatchValue = FindKeyInIniSection(lpgrszLangSection, szIniKey, cchSection, _tcslen(szIniKey));
        if (NULL != lpszMatchValue) {
            // pointer returned points to char directly after equal char ("="), and is not empty
            asLangTexts[i].lpString = lpszMatchValue;
        }

        // Update gui text with language string if id provided
        if (0 != asLangTexts[i].nIDDlgItem) {
            SetDlgItemText(hDlg, asLangTexts[i].nIDDlgItem, asLangTexts[i].lpString);
        }
    }

    // Get translator's name
    lpszMatchValue = FindKeyInIniSection(lpgrszLangSection, szItemTranslator, cchSection, _tcslen(szItemTranslator));
    if (NULL != lpszMatchValue) {
        lpCurrentTranslator = lpszMatchValue;
    } else {
        lpCurrentTranslator = szOriginal;
    }
}
