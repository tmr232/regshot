/*
    Copyright 1999-2003,2007 TiANWEi
    Copyright 2004 tulipfan
	Copyright 2007 Belogorokhov Youri


    This file is part of Regshot.

    Regshot is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Regshot is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Regshot; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
/*
	Other information Please view Readme.txt and history.txt.
*/

#include "global.h"
#include "version.h"

char *str_prgname=REGSHOT_TITLE " " REGSHOT_VERSION_STRING;
char *str_aboutme="Regshot is a free and open source registry compare utility.\n\n\
http://code.google.com/p/regshot/\n\n\
Original project webpage:\n\
http://sourceforge.net/projects/regshot/\n\n";
LPSTR	REGSHOTINI			="regshot.ini"; //tfx
LPSTR	REGSHOTLANGUAGEFILE	="language.ini";

extern u_char * lan_menuclearallshots;  // Be careful of extern ref! must be the same when declare them,otherwise pointer would mis-point
extern u_char * lan_menuclearshot1;     // and I can not use sizeof to get real array size in extern ref
extern u_char * lan_menuclearshot2;
extern u_char * lan_about;
extern LPSTR str_DefaultLanguage;
extern LPSTR str_Original;


// this new function added by Youri in 1.8.2, for expand path in browse dialog
int CALLBACK SelectBrowseFolder(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED) {
		SendMessage(hWnd, BFFM_SETSELECTION, 1, lpData);
	}
	return 0;
}


//--------------------------------------------------
//Main Dialog Proc
//--------------------------------------------------
BOOL	CALLBACK	DialogProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	size_t	nLengthofStr;
	//	BYTE	nFlag;

	switch(message) {
		case	WM_INITDIALOG:

			SendDlgItemMessage(hDlg,IDC_EDITCOMMENT,EM_SETLIMITTEXT,(WPARAM)COMMENTLENGTH,(LPARAM)0);
			SendDlgItemMessage(hDlg,IDC_EDITPATH,EM_SETLIMITTEXT,(WPARAM)MAX_PATH,(LPARAM)0);
			SendDlgItemMessage(hDlg,IDC_EDITDIR,EM_SETLIMITTEXT,(WPARAM)EXTDIRLEN,(LPARAM)0);
			lpExtDir=MYALLOC0(EXTDIRLEN+2);
			lpOutputpath=MYALLOC0(MAX_PATH+5);
			lpCurrentLanguage=MYALLOC0(SIZEOF_SINGLE_LANGUAGENAME);
			lpKeyName=MYALLOC0(MAX_PATH+1);
			lpMESSAGE=MYALLOC0(128);
			lpStartDir=MYALLOC0(MAX_PATH+1);
			lpWindowsDirName=MYALLOC0(MAX_PATH+5);
			lpTempPath=MYALLOC0(MAX_PATH+2);
			lpComputerName1=MYALLOC0(COMPUTERNAMELEN);
			lpComputerName2=MYALLOC0(COMPUTERNAMELEN);
			lpUserName1=MYALLOC0(COMPUTERNAMELEN);
			lpUserName2=MYALLOC0(COMPUTERNAMELEN);
			lpSystemtime1=MYALLOC0(sizeof(SYSTEMTIME));
			lpSystemtime2=MYALLOC0(sizeof(SYSTEMTIME));
			lpCurrentTranslator=str_Original;

			GetWindowsDirectory(lpWindowsDirName,MAX_PATH);
			nLengthofStr=strlen(lpWindowsDirName);
			if (nLengthofStr>0&&*(lpWindowsDirName+nLengthofStr-1)=='\\') {
				*(lpWindowsDirName+nLengthofStr-1)=0x00;
			}
			GetTempPath(MAX_PATH,lpTempPath);

			//_asm int 3;
			GetCurrentDirectory(MAX_PATH,lpStartDir); //fixed at 1.8.2 former version use getcommandline()
			lpIni=MYALLOC0(MAX_PATH*2);
			strcpy(lpIni,lpStartDir);
			if (*(lpIni+strlen(lpIni)-1)!='\\') { // 1.8.2
				strcat(lpIni,"\\");
			}
			strcat(lpIni,REGSHOTLANGUAGEFILE);

			lpFreeStrings=MYALLOC(SIZEOF_FREESTRINGS);
			ldwTempStrings=MYALLOC0(4*60); //max is 60 strings

			if(GetLanguageType(hDlg)) {
				GetLanguageStrings(hDlg);
			} else {
				GetDefaultStrings();
			}

			/*			//To get rgst152.dat which is the ini file of regshot,but it should  be a standard ini file in future!
						hFile = CreateFile(REGSHOTDATFILE,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
						if( hFile != INVALID_HANDLE_VALUE)
						{
							if((ReadFile(hFile,&nFlag,1,&NBW,NULL)==TRUE)&&NBW==1)
							{
								SendMessage(GetDlgItem(hDlg,IDC_RADIO1),BM_SETCHECK,(WPARAM)(nFlag&0x01),(LPARAM)0);
								SendMessage(GetDlgItem(hDlg,IDC_RADIO2),BM_SETCHECK,(WPARAM)((nFlag&0x01)^0x01),(LPARAM)0);
								SendMessage(GetDlgItem(hDlg,IDC_CHECKDIR),BM_SETCHECK,(WPARAM)((nFlag&0x02)>>1),(LPARAM)0);
								//SendMessage(GetDlgItem(hDlg,IDC_CHECKTURBO),BM_SETCHECK,(WPARAM)((nFlag&0x02)>1),(LPARAM)0);
								//SendMessage(hDlg,WM_COMMAND,(WPARAM)(((DWORD)(BN_CLICKED)<<16)|(DWORD)((nFlag&0x01==1)?IDC_RADIO1:IDC_RADIO2)),(LPARAM)0);
								//SendMessage(GetDlgItem(hDlg,IDC_CHECKAUTOCOMPARE),BM_SETCHECK,(WPARAM)((nFlag&0x02)>1),(LPARAM)0);
								//SendMessage(GetDlgItem(hDlg,IDC_CHECKWRITECONTENT),BM_SETCHECK,(WPARAM)((nFlag&0x04)>>2),(LPARAM)0);
								//SendMessage(GetDlgItem(hDlg,IDC_CHECKINI),BM_SETCHECK,(WPARAM)((nFlag&0x08)>>3),(LPARAM)0);
							}
							ReadFile(hFile,&nMask,4,&NBW,NULL);

							if((ReadFile(hFile,&nLengthofStr,sizeof(nLengthofStr),&NBW,NULL)==TRUE)
								&&NBW==sizeof(nLengthofStr)&&nLengthofStr!=0)
							{

								if((ReadFile(hFile,lpExtDir,nLengthofStr,&NBW,NULL)==TRUE)&&NBW==nLengthofStr)
								{
									SetDlgItemText(hDlg,IDC_EDITDIR,lpExtDir);
								}
								else
									SetDlgItemText(hDlg,IDC_EDITDIR,lpWindowsDirName);

							}
							else
								SetDlgItemText(hDlg,IDC_EDITDIR,lpWindowsDirName);

							//the output temppath
							if((ReadFile(hFile,&nLengthofStr,sizeof(nLengthofStr),&NBW,NULL)==TRUE)
								&&NBW==sizeof(nLengthofStr)&&nLengthofStr!=0)
							{

								if((ReadFile(hFile,lpOutputpath,nLengthofStr,&NBW,NULL)==TRUE)&&NBW==nLengthofStr)
								{
									SetDlgItemText(hDlg,IDC_EDITPATH,lpOutputpath);
								}
								else
									SetDlgItemText(hDlg,IDC_EDITPATH,lpTempPath);

							}
							else
								SetDlgItemText(hDlg,IDC_EDITPATH,lpTempPath);


							CloseHandle(hFile);
						}
						else
						{
							SendMessage(GetDlgItem(hDlg,IDC_RADIO1),BM_SETCHECK,(WPARAM)0x01,(LPARAM)0);
							SendMessage(GetDlgItem(hDlg,IDC_RADIO2),BM_SETCHECK,(WPARAM)0x00,(LPARAM)0);
							SendMessage(GetDlgItem(hDlg,IDC_CHECKDIR),BM_SETCHECK,(WPARAM)0x00,(LPARAM)0);
							SetDlgItemText(hDlg,IDC_EDITDIR,lpWindowsDirName);
							SetDlgItemText(hDlg,IDC_EDITPATH,lpTempPath);
							//SendMessage(GetDlgItem(hDlg,IDC_CHECKTURBO),BM_SETCHECK,(WPARAM)0,(LPARAM)0);
						}
			*/			//EnableWindow(GetDlgItem(hDlg,IDC_CHECKWRITECONTENT),FALSE);
			SendMessage(hDlg,WM_COMMAND,(WPARAM)IDC_CHECKDIR,(LPARAM)0);

			lpLastSaveDir=lpOutputpath;
			lpLastOpenDir=lpOutputpath;

			lpRegshotIni=MYALLOC0(3*MAX_PATH);
			strcpy(lpRegshotIni,lpStartDir);
			if (*(lpRegshotIni+strlen(lpRegshotIni)-1)!='\\') {
				strcat(lpRegshotIni,"\\");
			}
			strcat(lpRegshotIni,REGSHOTINI);

			GetSnapRegs(hDlg); //tfx

			return TRUE;

		case	WM_COMMAND:
			switch(LOWORD(wParam)) {
				case	IDC_1STSHOT:
					CreateShotPopupMenu();
					is1=TRUE;
					GetWindowRect(GetDlgItem(hDlg,IDC_1STSHOT),&rect);
					TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,rect.left+10,rect.top+10,0,hDlg,NULL);
					DestroyMenu(hMenu);

					return(TRUE);
				case	IDC_2NDSHOT:
					CreateShotPopupMenu();
					is1=FALSE;
					GetWindowRect(GetDlgItem(hDlg,IDC_2NDSHOT),&rect);
					TrackPopupMenu(hMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON,rect.left+10,rect.top+10,0,hDlg,NULL);
					DestroyMenu(hMenu);
					return(TRUE);
				case	IDM_SHOTONLY:
					if(is1) {
						is1LoadFromHive=FALSE;
						Shot1();
					} else {
						is2LoadFromHive=FALSE;
						Shot2();
					}

					return(TRUE);
				case	IDM_SHOTSAVE:
					if(is1) {
						is1LoadFromHive=FALSE;
						Shot1();
						SaveHive(lpHeadLocalMachine1,lpHeadUsers1,lpHeadFile1,lpComputerName1,lpUserName1,lpSystemtime1); //I might use a struct in future!
					} else {
						is2LoadFromHive=FALSE;
						Shot2();
						SaveHive(lpHeadLocalMachine2,lpHeadUsers2,lpHeadFile2,lpComputerName2,lpUserName2,lpSystemtime2);
					}

					return(TRUE);

				case	IDM_LOAD:
					if(is1) {
						is1LoadFromHive=LoadHive(&lpHeadLocalMachine1,&lpHeadUsers1,&lpHeadFile1,&lpTempHive1);
					} else {
						is2LoadFromHive=LoadHive(&lpHeadLocalMachine2,&lpHeadUsers2,&lpHeadFile2,&lpTempHive2);
					}

					//if(is1LoadFromHive||is2LoadFromHive)
					//	SendMessage(GetDlgItem(hWnd,IDC_CHECKDIR),BM_SETCHECK,(WPARAM)0x00,(LPARAM)0);

					return(TRUE);

					//case	IDC_SAVEREG:
					//SaveRegistry(lpHeadLocalMachine1,lpHeadUsers1);
					//		return(TRUE);
				case	IDC_COMPARE:
					EnableWindow(GetDlgItem(hDlg,IDC_COMPARE),FALSE);
					UI_BeforeClear();
					CompareShots();
					ShowWindow(GetDlgItem(hDlg,IDC_PBCOMPARE),SW_HIDE);
					EnableWindow(GetDlgItem(hDlg,IDC_CLEAR1),TRUE);
					SetFocus(GetDlgItem(hDlg,IDC_CLEAR1));
					SendMessage(hDlg,DM_SETDEFID,(WPARAM)IDC_CLEAR1,(LPARAM)0);
					SetCursor(hSaveCursor);
					MessageBeep(0xffffffff);
					return(TRUE);
				case	IDC_CLEAR1:
					hMenuClear=CreatePopupMenu();
					AppendMenu(hMenuClear,MF_STRING,IDM_CLEARALLSHOTS,lan_menuclearallshots);
					AppendMenu(hMenuClear,MF_MENUBARBREAK,IDM_BREAK,NULL);
					AppendMenu(hMenuClear,MF_STRING,IDM_CLEARSHOT1,lan_menuclearshot1);
					AppendMenu(hMenuClear,MF_STRING,IDM_CLEARSHOT2,lan_menuclearshot2);
					//AppendMenu(hMenuClear,MF_STRING,IDM_CLEARRESULT,"Clear compare result");
					SetMenuDefaultItem(hMenuClear,IDM_CLEARALLSHOTS,FALSE);


					//if(lpHeadFile!=NULL)
					//{
					//	EnableMenuItem(hMenuClear,IDM_CLEARSHOT1,MF_BYCOMMAND|MF_GRAYED);
					//	EnableMenuItem(hMenuClear,IDM_CLEARSHOT2,MF_BYCOMMAND|MF_GRAYED);
					//}
					//else
					{
						if(lpHeadLocalMachine1!=NULL) {
							EnableMenuItem(hMenuClear,IDM_CLEARSHOT1,MF_BYCOMMAND|MF_ENABLED);
						} else {
							EnableMenuItem(hMenuClear,IDM_CLEARSHOT1,MF_BYCOMMAND|MF_GRAYED);
						}

						if(lpHeadLocalMachine2!=NULL) {
							EnableMenuItem(hMenuClear,IDM_CLEARSHOT2,MF_BYCOMMAND|MF_ENABLED);
						} else {
							EnableMenuItem(hMenuClear,IDM_CLEARSHOT2,MF_BYCOMMAND|MF_GRAYED);
						}
					}
					GetWindowRect(GetDlgItem(hDlg,IDC_CLEAR1),&rect);
					TrackPopupMenu(hMenuClear,TPM_LEFTALIGN|TPM_LEFTBUTTON,rect.left+10,rect.top+10,0,hDlg,NULL);
					DestroyMenu(hMenuClear);
					return(TRUE);
				case	IDM_CLEARALLSHOTS:
					UI_BeforeClear();
					FreeAllKeyContent1();  //Note!! if loadfromhive and contains file,we should let lpHeadFile to NULL
					FreeAllKeyContent2();
					FreeAllCompareResults();

					FreeAllFileHead(lpHeadFile1);
					FreeAllFileHead(lpHeadFile2);

					lpHeadFile1=NULL;
					lpHeadFile2=NULL;
					UI_AfterClear();
					EnableWindow(GetDlgItem(hWnd,IDC_CLEAR1),FALSE);
					return(TRUE);
				case	IDM_CLEARSHOT1:
					UI_BeforeClear();
					FreeAllKeyContent1();
					FreeAllCompareResults();
					FreeAllFileHead(lpHeadFile1);
					lpHeadFile1=NULL;
					ClearKeyMatchTag(lpHeadLocalMachine2); //we clear shot2's tag
					ClearKeyMatchTag(lpHeadUsers2);
					ClearHeadFileMatchTag(lpHeadFile2);
					UI_AfterClear();
					return(TRUE);
				case	IDM_CLEARSHOT2:
					UI_BeforeClear();
					FreeAllKeyContent2();
					FreeAllCompareResults();
					FreeAllFileHead(lpHeadFile2);
					lpHeadFile2=NULL;
					ClearKeyMatchTag(lpHeadLocalMachine1); //we clear shot1's tag
					ClearKeyMatchTag(lpHeadUsers1);
					ClearHeadFileMatchTag(lpHeadFile1);
					UI_AfterClear();
					return(TRUE);
					/*
					case	IDM_CLEARRESULT:
					UI_BeforeClear();
					FreeAllCompareResults();
					ClearKeyMatchTag(lpHeadLocalMachine1);
					ClearKeyMatchTag(lpHeadLocalMachine2);
					ClearKeyMatchTag(lpHeadUsers1);
					ClearKeyMatchTag(lpHeadUsers2);
					ClearHeadFileMatchTag(lpHeadFile1);
					ClearHeadFileMatchTag(lpHeadFile2);
					UI_AfterClear();
					return(TRUE);
					*/

				case	IDC_CHECKDIR:
					if(SendMessage(GetDlgItem(hDlg,IDC_CHECKDIR),BM_GETCHECK,(WPARAM)0,(LPARAM)0)==1) {
						EnableWindow(GetDlgItem(hDlg,IDC_EDITDIR),TRUE);
						EnableWindow(GetDlgItem(hDlg,IDC_BROWSE1),TRUE);
					} else {
						EnableWindow(GetDlgItem(hDlg,IDC_EDITDIR),FALSE);
						EnableWindow(GetDlgItem(hDlg,IDC_BROWSE1),FALSE);
					}
					return(TRUE);
				case	IDC_CANCEL1:
				case	IDCANCEL:
					/*					SetCurrentDirectory(lpStartDir);
										hFile = CreateFile(REGSHOTDATFILE,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
										if( hFile != INVALID_HANDLE_VALUE)
										{

											nFlag=(BYTE)(SendMessage(GetDlgItem(hDlg,IDC_RADIO1),BM_GETCHECK,(WPARAM)0,(LPARAM)0)|
												SendMessage(GetDlgItem(hDlg,IDC_CHECKDIR),BM_GETCHECK,(WPARAM)0,(LPARAM)0)<<1);
											WriteFile(hFile,&nFlag,1,&NBW,NULL);
											WriteFile(hFile,&nMask,4,&NBW,NULL);
											nLengthofStr = GetDlgItemText(hDlg,IDC_EDITDIR,lpExtDir,EXTDIRLEN+2);
											WriteFile(hFile,&nLengthofStr,sizeof(nLengthofStr),&NBW,NULL);
											WriteFile(hFile,lpExtDir,nLengthofStr,&NBW,NULL);
											nLengthofStr = GetDlgItemText(hDlg,IDC_EDITPATH,lpOutputpath,MAX_PATH);
											WriteFile(hFile,&nLengthofStr,sizeof(nLengthofStr),&NBW,NULL);
											WriteFile(hFile,lpOutputpath,nLengthofStr,&NBW,NULL);

											CloseHandle(hFile);
										}
					*/
					SetSnapRegs(hDlg);//tfx
					PostQuitMessage(0);
					return(TRUE);

				case	IDC_BROWSE1: {

					LPITEMIDLIST lpidlist;
					size_t	nWholeLen;
					BrowseInfo1.hwndOwner=hDlg;
					BrowseInfo1.pszDisplayName=MYALLOC0(MAX_PATH+1);
					//BrowseInfo1.lpszTitle="Select:";
					BrowseInfo1.ulFlags =0; //3lines added in 1.8.2
					BrowseInfo1.lpfn = NULL;
					BrowseInfo1.lParam = 0;

					lpidlist=SHBrowseForFolder(&BrowseInfo1);
					if (lpidlist!=NULL) {
						SHGetPathFromIDList(lpidlist,BrowseInfo1.pszDisplayName);
						nLengthofStr = GetDlgItemText(hDlg,IDC_EDITDIR,lpExtDir,EXTDIRLEN+2);
						nWholeLen=nLengthofStr+strlen(BrowseInfo1.pszDisplayName);

						if (nWholeLen<EXTDIRLEN+1) {
							strcat(lpExtDir,";");
							strcat(lpExtDir,BrowseInfo1.pszDisplayName);

						} else {
							strcpy(lpExtDir,BrowseInfo1.pszDisplayName);
						}

						SetDlgItemText(hDlg,IDC_EDITDIR,lpExtDir);
						MYFREE(lpidlist);
					}

					MYFREE(BrowseInfo1.pszDisplayName);
				}
				return(TRUE);

				case	IDC_BROWSE2: {

					LPITEMIDLIST lpidlist;
					BrowseInfo1.hwndOwner=hDlg;
					BrowseInfo1.pszDisplayName=MYALLOC0(MAX_PATH+1);
					//BrowseInfo1.lpszTitle="Select:";

					//-----------------
					// Added by Youri in 1.8.2 ,Thanks!
					// if you add this code, the browse dialog will be expand path and have button "Create Folder"
					BrowseInfo1.ulFlags |= 0x0040; //BIF_NEWDIALOGSTYLE;    // button "Create Folder" and resizable
					BrowseInfo1.lpfn = SelectBrowseFolder;        // function for expand path
					BrowseInfo1.lParam = (LPARAM)BrowseInfo1.pszDisplayName;
					// Initilize selection path
					GetDlgItemText(hDlg,IDC_EDITPATH,BrowseInfo1.pszDisplayName,MAX_PATH);
					//-----------------

					lpidlist=SHBrowseForFolder(&BrowseInfo1);
					if (lpidlist!=NULL) {
						SHGetPathFromIDList(lpidlist,BrowseInfo1.pszDisplayName);
						SetDlgItemText(hDlg,IDC_EDITPATH,BrowseInfo1.pszDisplayName);
						MYFREE(lpidlist);
					}

					MYFREE(BrowseInfo1.pszDisplayName);
				}
				return(TRUE);
				case	IDC_COMBOLANGUAGE:
					GetLanguageStrings(hDlg);
					return(TRUE);

				case	IDC_ABOUT: {
					LPSTR	lpAboutBox;
					//_asm int 3;
					lpAboutBox=MYALLOC0(SIZEOF_ABOUTBOX);
					//it is silly that when wsprintf encounter a NULL strings, it will write the whole string to NULL!
					sprintf(lpAboutBox,"%s%s%s%s%s%s",str_aboutme,"[",(strlen(lpCurrentLanguage)==0)?str_DefaultLanguage:lpCurrentLanguage,"]"," by:",lpCurrentTranslator);
					MessageBox(hDlg,lpAboutBox,lan_about,MB_OK);
					MYFREE(lpAboutBox);
					return(TRUE);
				}
			}

	}
	return(FALSE);
}

/*
BOOL	SetPrivilege(HANDLE hToken,LPCTSTR pString,BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES	tp;
	LUID	luid;
	TOKEN_PRIVILEGES	tpPrevious;
	DWORD	cbSize=sizeof(TOKEN_PRIVILEGES);
	if	(!LookupPrivilegeValue(NULL,pString,&luid))
		return FALSE;
	tp.PrivilegeCount=1;
	tp.Privileges[0].Luid=luid;
	tp.Privileges[0].Attributes=0;
	if	(!AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),&tpPrevious,&cbSize))
		return FALSE;
	tpPrevious.PrivilegeCount=1;
	tpPrevious.Privileges[0].Luid=luid;
	if	(bEnablePrivilege)
		tpPrevious.Privileges[0].Attributes|=(SE_PRIVILEGE_ENABLED);
	else
		tpPrevious.Privileges[0].Attributes^=((tpPrevious.Privileges[0].Attributes)&(SE_PRIVILEGE_ENABLED));
	if	(!AdjustTokenPrivileges(hToken,FALSE,&tpPrevious,cbSize,NULL,NULL))
		return FALSE;
	return TRUE;
}
*/

//////////////////////////////////////////////////////////////////
int	WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
				   LPSTR lpszCmdLine,int nCmdShow)
{
	/*
	BOOL	bWinNTDetected;
	HANDLE	hToken=0;
	OSVERSIONINFO winver;
	winver.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	GetVersionEx(&winver);
	bWinNTDetected=(winver.dwPlatformId==VER_PLATFORM_WIN32_NT) ? TRUE : FALSE;
	//hWndMonitor be created first for the multilanguage interface.

	//FARPROC		lpfnDlgProc;
	//lpfnDlgProc	=	MakeProcInstance((FARPROC)DialogProc,hInstance); //old style of create dialogproc
	*/

	hHeap=GetProcessHeap(); //1.8.2
	hWnd=CreateDialog(hInstance,MAKEINTRESOURCE(IDD_DIALOG1),NULL,(WNDPROC)DialogProc);

	SetClassLongPtr(hWnd,GCLP_HICON,(LONG_PTR)LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MAINICON)));

	SetWindowText(hWnd, str_prgname);
	ShowWindow(hWnd,nCmdShow);
	UpdateWindow(hWnd);
	//SetPriorityClass(hInstance,31);
	/*
	if	(bWinNTDetected)
	{
		if	(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken)==TRUE)
		{
			if	(SetPrivilege(hToken,"SeSystemProfilePrivilege",TRUE)==TRUE)
			{
				MessageBox(hWnd,"We are in system level,enjoy!","Info:",MB_OK);
			}
			CloseHandle(hToken);
		}
	}
	*/
	while(GetMessage(&msg,NULL,(WPARAM)NULL,(LPARAM)NULL)) {
		if(!IsDialogMessage(hWnd,&msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return(int)(msg.wParam);
}
