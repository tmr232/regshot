/*
    Copyright 1999-2003,2007 TiANWEi
    Copyright 2004 tulipfan
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
#include "version.h"

TCHAR *lpszProgramName = REGSHOT_TITLE " " REGSHOT_VERSION_STRING;  // tfx  add program titile
char *str_aboutme = "Regshot is a free and open source registry compare utility.\nversion: " REGSHOT_VERSION_DESCRIPTION "\n\nhttp://sourceforge.net/projects/regshot/\n\n" REGSHOT_VERSION_COPYRIGHT "\n\n";

LPTSTR REGSHOTINI          = TEXT("regshot.ini"); // tfx
LPTSTR REGSHOTLANGUAGEFILE = TEXT("language.ini");

REGSHOT Shot1;
REGSHOT Shot2;


// this new function added by Youri in 1.8.2, for expanding path in browse dialog
int CALLBACK SelectBrowseFolder(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    UNREFERENCED_PARAMETER(lParam);
    if (uMsg == BFFM_INITIALIZED) {
        SendMessage(hWnd, BFFM_SETSELECTION, 1, lpData);
    }
    return 0;
}


//--------------------------------------------------
// Main Dialog Proc
//--------------------------------------------------
BOOL CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    size_t  nLengthofStr;
    //BYTE    nFlag;

    UNREFERENCED_PARAMETER(lParam);

    switch (message) {
        case WM_INITDIALOG:

            SendDlgItemMessage(hDlg, IDC_EDITCOMMENT, EM_SETLIMITTEXT, (WPARAM)COMMENTLENGTH, (LPARAM)0);
            SendDlgItemMessage(hDlg, IDC_EDITPATH, EM_SETLIMITTEXT, (WPARAM)MAX_PATH, (LPARAM)0);
            SendDlgItemMessage(hDlg, IDC_EDITDIR, EM_SETLIMITTEXT, (WPARAM)(EXTDIRLEN / 2), (LPARAM)0);

            //enlarge some buffer in 201201
            lpszLanguage      = NULL;
            lpExtDir          = MYALLOC0(EXTDIRLEN + 4);      // EXTDIRLEN is actually 4*max_path
            lpLanguageIni     = MYALLOC0(MAX_PATH * 4 + 4);   // for language.ini
            lpRegshotIni      = MYALLOC0(MAX_PATH * 4 + 4);   // for regshot.ini
            lpKeyName         = MYALLOC0(MAX_PATH * 2 + 2);   // For scan engine store keyname
            lpValueName       = MYALLOC0(1024 * 16 * 2);      // For scan engine store valuename
            lpValueData       = MYALLOC0(ESTIMATE_VALUEDATA_LENGTH);  // For scan engine store valuedata estimate
            lpszMessage       = MYALLOC0((REGSHOT_MESSAGE_LENGTH + 1) * sizeof(TCHAR));  // For status bar text message store
            lpWindowsDirName  = MYALLOC0((MAX_PATH + 1) * sizeof(TCHAR));
            lpTempPath        = MYALLOC0((MAX_PATH + 1) * sizeof(TCHAR));
            lpStartDir        = MYALLOC0((MAX_PATH + 1) * sizeof(TCHAR));
            lpOutputpath      = MYALLOC0((MAX_PATH + 1) * sizeof(TCHAR));  // store last save/open hive file dir, +1 for possible change in CompareShots()
            lpgrszLangSection = NULL;

            ZeroMemory(&Shot1, sizeof(Shot1));
            ZeroMemory(&Shot2, sizeof(Shot2));

            GetWindowsDirectory(lpWindowsDirName, MAX_PATH);
            nLengthofStr = strlen(lpWindowsDirName);
            if (nLengthofStr > 0 && *(lpWindowsDirName + nLengthofStr - 1) == '\\') {
                *(lpWindowsDirName + nLengthofStr - 1) = 0x00;
            }
            GetTempPath(MAX_PATH, lpTempPath);

            //_asm int 3;
            GetCurrentDirectory(MAX_PATH + 1, lpStartDir);      // fixed in 1.8.2 former version use getcommandline()
            strcpy(lpLanguageIni, lpStartDir);
            if (*(lpLanguageIni + strlen(lpLanguageIni) - 1) != '\\') {    // 1.8.2
                strcat(lpLanguageIni, "\\");
            }
            strcat(lpLanguageIni, REGSHOTLANGUAGEFILE);

            SetTextsToDefaultLanguage();
            LoadAvailableLanguagesFromIni(hDlg);
            if (GetSelectedLanguage(hDlg)) {
                SetTextsToSelectedLanguage(hDlg);
            }

            SendMessage(hDlg, WM_COMMAND, (WPARAM)IDC_CHECKDIR, (LPARAM)0);

            lpLastSaveDir = lpOutputpath;
            lpLastOpenDir = lpOutputpath;

            strcpy(lpRegshotIni, lpStartDir);
            if (*(lpRegshotIni + strlen(lpRegshotIni) - 1) != '\\') {
                strcat(lpRegshotIni, "\\");
            }
            strcat(lpRegshotIni, REGSHOTINI);

            LoadSettingsFromIni(hDlg); // tfx

            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDC_1STSHOT:
                    CreateShotPopupMenu();
                    is1 = TRUE;
                    GetWindowRect(GetDlgItem(hDlg, IDC_1STSHOT), &rect);
                    TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, rect.left + 10, rect.top + 10, 0, hDlg, NULL);
                    DestroyMenu(hMenu);

                    return(TRUE);

                case IDC_2NDSHOT:
                    CreateShotPopupMenu();
                    is1 = FALSE;
                    GetWindowRect(GetDlgItem(hDlg, IDC_2NDSHOT), &rect);
                    TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, rect.left + 10, rect.top + 10, 0, hDlg, NULL);
                    DestroyMenu(hMenu);
                    return(TRUE);

                case IDM_SHOTONLY:
                    if (is1) {
                        Shot(&Shot1);
                    } else {
                        Shot(&Shot2);
                    }

                    return(TRUE);

                case IDM_SHOTSAVE:
                    if (is1) {
                        Shot(&Shot1);
                        SaveHive(&Shot1);
                    } else {
                        Shot(&Shot2);
                        SaveHive(&Shot2);
                    }

                    return(TRUE);

                case IDM_LOAD:
                    if (is1) {
                        LoadHive(&Shot1);
                    } else {
                        LoadHive(&Shot2);
                    }

                    //if (is1LoadFromHive || is2LoadFromHive)
                    //  SendMessage(GetDlgItem(hWnd,IDC_CHECKDIR),BM_SETCHECK,(WPARAM)0x00,(LPARAM)0);

                    return(TRUE);

                    /*case IDC_SAVEREG:
                        SaveRegistry(Shot1.lpHKLM,Shot1.lpHKU);
                        return(TRUE);*/

                case IDC_COMPARE:
                    EnableWindow(GetDlgItem(hDlg, IDC_COMPARE), FALSE);
                    UI_BeforeClear();
                    CompareShots(&Shot1, &Shot2);
                    ShowWindow(GetDlgItem(hDlg, IDC_PBCOMPARE), SW_HIDE);
                    EnableWindow(GetDlgItem(hDlg, IDC_CLEAR1), TRUE);
                    SetFocus(GetDlgItem(hDlg, IDC_CLEAR1));
                    SendMessage(hDlg, DM_SETDEFID, (WPARAM)IDC_CLEAR1, (LPARAM)0);
                    SetCursor(hSaveCursor);
                    MessageBeep(0xffffffff);
                    return(TRUE);

                case IDC_CLEAR1:
                    hMenuClear = CreatePopupMenu();
                    AppendMenu(hMenuClear, MF_STRING, IDM_CLEARALLSHOTS, asLangTexts[iszTextMenuClearAllShots].lpString);
                    AppendMenu(hMenuClear, MF_MENUBARBREAK, IDM_BREAK, NULL);
                    AppendMenu(hMenuClear, MF_STRING, IDM_CLEARSHOT1, asLangTexts[iszTextMenuClearShot1].lpString);
                    AppendMenu(hMenuClear, MF_STRING, IDM_CLEARSHOT2, asLangTexts[iszTextMenuClearShot2].lpString);
                    //AppendMenu(hMenuClear,MF_STRING,IDM_CLEARRESULT,"Clear comparison result");
                    SetMenuDefaultItem(hMenuClear, IDM_CLEARALLSHOTS, FALSE);

                    //if (lpHeadFile != NULL)
                    //{
                    //  EnableMenuItem(hMenuClear,IDM_CLEARSHOT1,MF_BYCOMMAND|MF_GRAYED);
                    //  EnableMenuItem(hMenuClear,IDM_CLEARSHOT2,MF_BYCOMMAND|MF_GRAYED);
                    //}
                    //else
                    {
                        if (Shot1.lpHKLM != NULL) {
                            EnableMenuItem(hMenuClear, IDM_CLEARSHOT1, MF_BYCOMMAND | MF_ENABLED);
                        } else {
                            EnableMenuItem(hMenuClear, IDM_CLEARSHOT1, MF_BYCOMMAND | MF_GRAYED);
                        }

                        if (Shot2.lpHKLM != NULL) {
                            EnableMenuItem(hMenuClear, IDM_CLEARSHOT2, MF_BYCOMMAND | MF_ENABLED);
                        } else {
                            EnableMenuItem(hMenuClear, IDM_CLEARSHOT2, MF_BYCOMMAND | MF_GRAYED);
                        }
                    }
                    GetWindowRect(GetDlgItem(hDlg, IDC_CLEAR1), &rect);
                    TrackPopupMenu(hMenuClear, TPM_LEFTALIGN | TPM_LEFTBUTTON, rect.left + 10, rect.top + 10, 0, hDlg, NULL);
                    DestroyMenu(hMenuClear);
                    return(TRUE);

                case IDM_CLEARALLSHOTS:
                    UI_BeforeClear();
                    FreeShot(&Shot1);
                    FreeShot(&Shot2);
                    FreeAllCompareResults();
                    UI_AfterClear();
                    EnableWindow(GetDlgItem(hWnd, IDC_CLEAR1), FALSE);
                    return(TRUE);

                case IDM_CLEARSHOT1:
                    UI_BeforeClear();
                    FreeShot(&Shot1);
                    FreeAllCompareResults();
                    ClearKeyMatchTag(Shot2.lpHKLM);  // we clear Shot2's tag
                    ClearKeyMatchTag(Shot2.lpHKU);
                    ClearHeadFileMatchTag(Shot2.lpHF);
                    UI_AfterClear();
                    return(TRUE);

                case IDM_CLEARSHOT2:
                    UI_BeforeClear();
                    FreeShot(&Shot2);
                    FreeAllCompareResults();
                    ClearKeyMatchTag(Shot1.lpHKLM);  // we clear Shot1's tag
                    ClearKeyMatchTag(Shot1.lpHKU);
                    ClearHeadFileMatchTag(Shot1.lpHF);
                    UI_AfterClear();
                    return(TRUE);

                    /*case IDM_CLEARRESULT:
                        UI_BeforeClear();
                        FreeAllCompareResults();
                        ClearKeyMatchTag(Shot1.lpHKLM);
                        ClearKeyMatchTag(Shot2.lpHKLM);
                        ClearKeyMatchTag(Shot1.lpHKU);
                        ClearKeyMatchTag(Shot2.lpHKU);
                        ClearHeadFileMatchTag(Shot1.lpHF);
                        ClearHeadFileMatchTag(Shot2.lpHF);
                        UI_AfterClear();
                        return(TRUE);*/

                case IDC_CHECKDIR:
                    if (SendMessage(GetDlgItem(hDlg, IDC_CHECKDIR), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == 1) {
                        EnableWindow(GetDlgItem(hDlg, IDC_EDITDIR), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDC_BROWSE1), TRUE);
                    } else {
                        EnableWindow(GetDlgItem(hDlg, IDC_EDITDIR), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_BROWSE1), FALSE);
                    }
                    return(TRUE);

                case IDC_CANCEL1:
                case IDCANCEL:

                    SaveSettingsToIni(hDlg);  // tfx
                    PostQuitMessage(0);
                    return(TRUE);

                case IDC_BROWSE1: {

                    LPITEMIDLIST lpidlist;
                    BrowseInfo1.hwndOwner = hDlg;
                    BrowseInfo1.pszDisplayName = MYALLOC0(MAX_PATH * 2 + 2);
                    //BrowseInfo1.lpszTitle = "Select:";
                    BrowseInfo1.ulFlags = 0;     // 3 lines added in 1.8.2
                    BrowseInfo1.lpfn = NULL;
                    BrowseInfo1.lParam = 0;

                    lpidlist = SHBrowseForFolder(&BrowseInfo1);

                    if (lpidlist != NULL) {
                        size_t  nWholeLen;

                        SHGetPathFromIDList(lpidlist, BrowseInfo1.pszDisplayName);
                        nLengthofStr = GetDlgItemText(hDlg, IDC_EDITDIR, lpExtDir, EXTDIRLEN / 2);
                        nWholeLen = nLengthofStr + strlen(BrowseInfo1.pszDisplayName);

                        if (nWholeLen < EXTDIRLEN + 1) {
                            strcat(lpExtDir, ";");
                            strcat(lpExtDir, BrowseInfo1.pszDisplayName);

                        } else {
                            strcpy(lpExtDir, BrowseInfo1.pszDisplayName);
                        }

                        SetDlgItemText(hDlg, IDC_EDITDIR, lpExtDir);
                        MYFREE(lpidlist);
                    }

                    MYFREE(BrowseInfo1.pszDisplayName);
                }
                return(TRUE);

                case IDC_BROWSE2: {

                    LPITEMIDLIST lpidlist;
                    BrowseInfo1.hwndOwner = hDlg;
                    BrowseInfo1.pszDisplayName = MYALLOC0(MAX_PATH * 2 + 2);
                    //BrowseInfo1.lpszTitle = "Select:";

                    //-----------------
                    // Added by Youri in 1.8.2 ,Thanks!
                    // if you add this code, the browse dialog will be expand path and have button "Create Folder"
                    BrowseInfo1.ulFlags |= 0x0040; // BIF_NEWDIALOGSTYLE;    // button "Create Folder" and resizable
                    BrowseInfo1.lpfn = SelectBrowseFolder;                   // function for expand path
                    BrowseInfo1.lParam = (LPARAM)BrowseInfo1.pszDisplayName;
                    // Initilize selection path
                    GetDlgItemText(hDlg, IDC_EDITPATH, BrowseInfo1.pszDisplayName, MAX_PATH);
                    //-----------------

                    lpidlist = SHBrowseForFolder(&BrowseInfo1);
                    if (lpidlist != NULL) {
                        SHGetPathFromIDList(lpidlist, BrowseInfo1.pszDisplayName);
                        SetDlgItemText(hDlg, IDC_EDITPATH, BrowseInfo1.pszDisplayName);
                        MYFREE(lpidlist);
                    }

                    MYFREE(BrowseInfo1.pszDisplayName);
                }
                return(TRUE);

                case IDC_COMBOLANGUAGE:
                    SetTextsToDefaultLanguage();
                    SetTextsToSelectedLanguage(hDlg);
                    return(TRUE);

                case IDC_ABOUT: {
                    LPSTR   lpAboutBox;
                    //_asm int 3;
                    lpAboutBox = MYALLOC0(SIZEOF_ABOUTBOX);
                    // it is silly that when wsprintf encounters a NULL string, it will write the whole string to NULL!
                    sprintf(lpAboutBox, "%s%s%s%s%s%s", str_aboutme, "[", lpszLanguage, "]", " by: ", lpCurrentTranslator);
                    MessageBox(hDlg, lpAboutBox, asLangTexts[iszTextAbout].lpString, MB_OK);
                    MYFREE(lpAboutBox);
                    return(TRUE);
                }
            }

    }
    return(FALSE);
}


/*BOOL SetPrivilege(HANDLE hToken, LPCSTR pString, BOOL bEnablePrivilege)
{
    TOKEN_PRIVILEGES    tp;
    LUID    luid;
    TOKEN_PRIVILEGES    tpPrevious;
    DWORD   cbSize = sizeof(TOKEN_PRIVILEGES);

    if (!LookupPrivilegeValue(NULL,pString,&luid))
        return FALSE;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = 0;
    if (!AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),&tpPrevious,&cbSize))
        return FALSE;
    tpPrevious.PrivilegeCount = 1;
    tpPrevious.Privileges[0].Luid = luid;
    if (bEnablePrivilege)
        tpPrevious.Privileges[0].Attributes| = (SE_PRIVILEGE_ENABLED);
    else
        tpPrevious.Privileges[0].Attributes^ = ((tpPrevious.Privileges[0].Attributes)&(SE_PRIVILEGE_ENABLED));
    if (!AdjustTokenPrivileges(hToken,FALSE,&tpPrevious,cbSize,NULL,NULL))
        return FALSE;
    return TRUE;
}*/


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine, int nCmdShow)
{

    /*
    BOOL    bWinNTDetected;
    HANDLE  hToken = 0;
    OSVERSIONINFO winver;
    winver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&winver);
    bWinNTDetected = (winver.dwPlatformId == VER_PLATFORM_WIN32_NT) ? TRUE : FALSE;
    //hWndMonitor be created first for the multilanguage interface.

    //FARPROC       lpfnDlgProc;
    //lpfnDlgProc = MakeProcInstance((FARPROC)DialogProc,hInstance);    // old style of create dialogproc
    */
    UNREFERENCED_PARAMETER(lpszCmdLine);
    UNREFERENCED_PARAMETER(hPrevInstance);

    hHeap = GetProcessHeap(); // 1.8.2
    hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DialogProc);

    SetClassLongPtr(hWnd, GCLP_HICON, (LONG_PTR)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON)));

    SetWindowText(hWnd, lpszProgramName);  // tfx set program title to lpszProgramName to avoid edit resource file
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    //SetPriorityClass(hInstance,31);
    /*if (bWinNTDetected)
      {
          if (OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken) == TRUE)
          {
              if (SetPrivilege(hToken,"SeSystemProfilePrivilege",TRUE) == TRUE)
              {
                  MessageBox(hWnd,"We are in system level,enjoy!","Info:",MB_OK);
              }
              CloseHandle(hToken);
          }
      }*/
    while (GetMessage(&msg, NULL, (WPARAM)NULL, (LPARAM)NULL)) {
        if (!IsDialogMessage(hWnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return(int)(msg.wParam);
}
