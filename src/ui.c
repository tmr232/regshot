/*
    Copyright 1999-2003,2007 TiANWEi
    Copyright 2004 tulipfan
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
#include "LMCons.h"  // for UNLEN define


char USERSSTRING_LONG[]        = "HKEY_USERS";   // 1.6 using long name, so in 1.8.1 add an option
char USERSSTRING[]             = "HKU";          // in regshot.ini, "UseLongRegHead" to control this
char LOCALMACHINESTRING[]      = "HKLM";
char LOCALMACHINESTRING_LONG[] = "HKEY_LOCAL_MACHINE";

LPTSTR lpszMessage;


void ShowHideCounters(int nCmdShow) // 1.8.2
{
    ShowWindow(GetDlgItem(hWnd, IDC_TEXTCOUNT1), nCmdShow);
    ShowWindow(GetDlgItem(hWnd, IDC_TEXTCOUNT2), nCmdShow);
    ShowWindow(GetDlgItem(hWnd, IDC_TEXTCOUNT3), nCmdShow);
}


//////////////////////////////////////////////////////////////////
VOID InitProgressBar(VOID)
{
    // The following are not so good, but they work
    nSavingKey = 0;
    nComparing = 0;
    nRegStep = nGettingKey / MAXPBPOSITION;
    nFileStep = nGettingFile / MAXPBPOSITION;
    ShowHideCounters(SW_HIDE);  // 1.8.2
    SendDlgItemMessage(hWnd, IDC_PBCOMPARE, PBM_SETRANGE, (WPARAM)0, MAKELPARAM(0, MAXPBPOSITION));
    SendDlgItemMessage(hWnd, IDC_PBCOMPARE, PBM_SETPOS, (WPARAM)0, (LPARAM)0);
    SendDlgItemMessage(hWnd, IDC_PBCOMPARE, PBM_SETSTEP, (WPARAM)1, (LPARAM)0);
    ShowWindow(GetDlgItem(hWnd, IDC_PBCOMPARE), SW_SHOW);
}


// ----------------------------------------------------------------------
// Update counters in the "status bar" of the window
// ----------------------------------------------------------------------
VOID UpdateCounters(LPTSTR lpszTitle1, LPTSTR lpszTitle2, DWORD nCount1, DWORD nCount2)
{
    //nGettingTime = GetTickCount();
    nBASETIME1 = nGettingTime;
    lpszMessage[REGSHOT_MESSAGE_LENGTH] = 0;  // safety NULL char, as translated strings could exceed buffer length
    _sntprintf(lpszMessage, REGSHOT_MESSAGE_LENGTH, TEXT("%s%u%s%u%s"), asLangTexts[iszTextTime].lpString, (nGettingTime - nBASETIME) / 1000, TEXT("s"), (nGettingTime - nBASETIME) % 1000, TEXT("ms"));
    SendDlgItemMessage(hWnd, IDC_TEXTCOUNT3, WM_SETTEXT, (WPARAM)0, (LPARAM)lpszMessage);
    _sntprintf(lpszMessage, REGSHOT_MESSAGE_LENGTH, TEXT("%s%u"), lpszTitle1, nCount1);
    SendDlgItemMessage(hWnd, IDC_TEXTCOUNT1, WM_SETTEXT, (WPARAM)0, (LPARAM)lpszMessage);
    _sntprintf(lpszMessage, REGSHOT_MESSAGE_LENGTH, TEXT("%s%u"), lpszTitle2, nCount2);
    SendDlgItemMessage(hWnd, IDC_TEXTCOUNT2, WM_SETTEXT, (WPARAM)0, (LPARAM)lpszMessage);

    UpdateWindow(hWnd);
    PeekMessage(&msg, hWnd, WM_ACTIVATE, WM_ACTIVATE, PM_REMOVE);
    //SetForegroundWindow(hWnd);
}


// ----------------------------------------------------------------------
// Prepare the GUI for the shot about to be taken
// ----------------------------------------------------------------------
VOID UI_BeforeShot(DWORD nID)
{
    hHourGlass = LoadCursor(NULL, IDC_WAIT);
    hSaveCursor = SetCursor(hHourGlass);
    EnableWindow(GetDlgItem(hWnd, nID), FALSE);
    // Added in 1.8.2
    _tcscpy(lpszMessage, TEXT(" "));  // clear the counters
    SendDlgItemMessage(hWnd, IDC_TEXTCOUNT1, WM_SETTEXT, (WPARAM)0, (LPARAM)lpszMessage);
    SendDlgItemMessage(hWnd, IDC_TEXTCOUNT2, WM_SETTEXT, (WPARAM)0, (LPARAM)lpszMessage);
    SendDlgItemMessage(hWnd, IDC_TEXTCOUNT3, WM_SETTEXT, (WPARAM)0, (LPARAM)lpszMessage);
    ShowHideCounters(SW_SHOW);
}


//--------------------------------------------------
// Reset the GUI after the shot has been taken
//--------------------------------------------------
VOID UI_AfterShot(VOID)
{
    DWORD iddef;

    if (Shot1.lpHKLM == NULL) {
        iddef = IDC_1STSHOT;
    } else if (Shot2.lpHKLM == NULL) {
        iddef = IDC_2NDSHOT;
    } else {
        iddef = IDC_COMPARE;
    }
    EnableWindow(GetDlgItem(hWnd, IDC_CLEAR1), TRUE);
    EnableWindow(GetDlgItem(hWnd, iddef), TRUE);
    SendMessage(hWnd, DM_SETDEFID, (WPARAM)iddef, (LPARAM)0);
    SetFocus(GetDlgItem(hWnd, iddef));
    SetCursor(hSaveCursor);
    MessageBeep(0xffffffff);
}


//--------------------------------------------------
// Prepare the GUI for clearing
//--------------------------------------------------
VOID UI_BeforeClear(VOID)
{
    //EnableWindow(GetDlgItem(hWnd,IDC_CLEAR1),FALSE);
    hHourGlass = LoadCursor(NULL, IDC_WAIT);
    hSaveCursor = SetCursor(hHourGlass);
    ShowHideCounters(SW_HIDE);
    UpdateWindow(hWnd);
}


//--------------------------------------------------
// Reset the GUI after the clearing
//--------------------------------------------------
VOID UI_AfterClear(VOID)
{
    DWORD   iddef = 0;
    //BOOL    bChk;   // used for file scan disable

    if (Shot1.lpHKLM == NULL) {
        iddef = IDC_1STSHOT;
    } else if (Shot2.lpHKLM == NULL) {
        iddef = IDC_2NDSHOT;
    }
    EnableWindow(GetDlgItem(hWnd, iddef), TRUE);
    EnableWindow(GetDlgItem(hWnd, IDC_COMPARE), FALSE);

    if ((Shot1.lpHKLM == NULL) && (Shot2.lpHKLM == NULL)) {
        EnableWindow(GetDlgItem(hWnd, IDC_2NDSHOT), FALSE);
        EnableWindow(GetDlgItem(hWnd, IDC_CLEAR1), FALSE);
        //bChk = TRUE;
    }
    //else  // I forgot to comment this out, fixed in 1.8.2
    //bChk = FALSE;

    //EnableWindow(GetDlgItem(hWnd,IDC_CHECKDIR),bChk); // Not used in 1.8; we only enable chk when clear all
    //SendMessage(hWnd,WM_COMMAND,(WPARAM)IDC_CHECKDIR,(LPARAM)0);

    SetFocus(GetDlgItem(hWnd, iddef));
    SendMessage(hWnd, DM_SETDEFID, (WPARAM)iddef, (LPARAM)0);
    SetCursor(hSaveCursor);
    MessageBeep(0xffffffff);
}


// -----------------------------
VOID Shot(LPREGSHOT lpShot)
{
    lpShot->lpHKLM = (LPKEYCONTENT)MYALLOC0(sizeof(KEYCONTENT));
    lpShot->lpHKU = (LPKEYCONTENT)MYALLOC0(sizeof(KEYCONTENT));

    if (bUseLongRegHead) {  // 1.8.1
        lpShot->lpHKLM->lpKeyName = MYALLOC(sizeof(LOCALMACHINESTRING_LONG));
        lpShot->lpHKU->lpKeyName = MYALLOC(sizeof(USERSSTRING_LONG));
        strcpy(lpShot->lpHKLM->lpKeyName, LOCALMACHINESTRING_LONG);
        strcpy(lpShot->lpHKU->lpKeyName, USERSSTRING_LONG);
    } else {
        lpShot->lpHKLM->lpKeyName = MYALLOC(sizeof(LOCALMACHINESTRING));
        lpShot->lpHKU->lpKeyName = MYALLOC(sizeof(USERSSTRING));
        strcpy(lpShot->lpHKLM->lpKeyName, LOCALMACHINESTRING);
        strcpy(lpShot->lpHKU->lpKeyName, USERSSTRING);
    }

    nGettingKey   = 2;
    nGettingValue = 0;
    nGettingTime  = 0;
    nGettingFile  = 0;
    nGettingDir   = 0;
    nBASETIME  = GetTickCount();
    nBASETIME1 = nBASETIME;
    if (is1) {
        UI_BeforeShot(IDC_1STSHOT);
    } else {
        UI_BeforeShot(IDC_2NDSHOT);
    }

    GetRegistrySnap(HKEY_LOCAL_MACHINE, lpShot->lpHKLM);
    GetRegistrySnap(HKEY_USERS, lpShot->lpHKU);
    nGettingTime = GetTickCount();
    UpdateCounters(asLangTexts[iszTextKey].lpString, asLangTexts[iszTextValue].lpString, nGettingKey, nGettingValue);

    if (SendMessage(GetDlgItem(hWnd, IDC_CHECKDIR), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == 1) {
        size_t  nLengthofStr;
        DWORD   i;
        LPSTR   lpSubExtDir;
        LPHEADFILE lpHF;
        LPHEADFILE lpHFTemp;

        GetDlgItemText(hWnd, IDC_EDITDIR, lpExtDir, EXTDIRLEN / 2);
        nLengthofStr = strlen(lpExtDir);

        lpHF = lpHFTemp = lpShot->lpHF;  // changed in 1.8
        lpSubExtDir = lpExtDir;

        if (nLengthofStr > 0)
            for (i = 0; i <= nLengthofStr; i++) {
                // This is the stupid filename detection routine, [seperate with ";"]
                if (*(lpExtDir + i) == 0x3b || *(lpExtDir + i) == 0x00) {
                    *(lpExtDir + i) = 0x00;

                    if (*(lpExtDir + i - 1) == '\\' && i > 0) {
                        *(lpExtDir + i - 1) = 0x00;
                    }

                    if (*lpSubExtDir != 0x00) {
                        size_t  nSubExtDirLen;

                        lpHF = (LPHEADFILE)MYALLOC0(sizeof(HEADFILE));
                        if (lpShot->lpHF == NULL) {
                            lpShot->lpHF = lpHF;
                        } else {
                            lpHFTemp->lpBrotherHF = lpHF;
                        }

                        lpHFTemp = lpHF;
                        lpHF->lpFirstFC = (LPFILECONTENT)MYALLOC0(sizeof(FILECONTENT));
                        //lpHF->lpfilecontent2 = (LPFILECONTENT)MYALLOC0(sizeof(FILECONTENT));

                        nSubExtDirLen = strlen(lpSubExtDir) + 1;
                        lpHF->lpFirstFC->lpFileName = MYALLOC(nSubExtDirLen);
                        //lpHF->lpfilecontent2->lpFileName = MYALLOC(nSubExtDirLen);

                        strcpy(lpHF->lpFirstFC->lpFileName, lpSubExtDir);
                        //strcpy(lpHF->lpfilecontent2->lpFileName,lpSubExtDir);

                        lpHF->lpFirstFC->fileattr = FILE_ATTRIBUTE_DIRECTORY;
                        //lpHF->lpfilecontent2->fileattr = FILE_ATTRIBUTE_DIRECTORY;

                        GetFilesSnap(lpHF->lpFirstFC);
                        nGettingTime = GetTickCount();
                        UpdateCounters(asLangTexts[iszTextDir].lpString, asLangTexts[iszTextFile].lpString, nGettingDir, nGettingFile);
                    }
                    lpSubExtDir = lpExtDir + i + 1;
                }
            }
    }

    lpShot->computername = MYALLOC0((MAX_COMPUTERNAME_LENGTH + 2) * sizeof(TCHAR));
    ZeroMemory(lpShot->computername, (MAX_COMPUTERNAME_LENGTH + 2) * sizeof(TCHAR));
    NBW = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerName(lpShot->computername, &NBW);

    lpShot->username = MYALLOC0((UNLEN + 2) * sizeof(TCHAR));
    ZeroMemory(lpShot->username, (UNLEN + 2) * sizeof(TCHAR));
    NBW = UNLEN + 1;
    GetUserName(lpShot->username, &NBW);

    GetSystemTime(&lpShot->systemtime);

    UI_AfterShot();
}

//--------------------------------------------------
// Show popup shortcut menu
//--------------------------------------------------
VOID CreateShotPopupMenu(VOID)
{
    hMenu = CreatePopupMenu();
    AppendMenu(hMenu, MF_STRING, IDM_SHOTONLY, asLangTexts[iszTextMenuShot].lpString);
    AppendMenu(hMenu, MF_STRING, IDM_SHOTSAVE, asLangTexts[iszTextMenuShotSave].lpString);
    AppendMenu(hMenu, MF_SEPARATOR, IDM_BREAK, NULL);
    AppendMenu(hMenu, MF_STRING, IDM_LOAD, asLangTexts[iszTextMenuShotLoad].lpString);
    SetMenuDefaultItem(hMenu, IDM_SHOTONLY, FALSE);
}
