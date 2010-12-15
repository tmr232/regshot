/*
    Copyright 1999-2003,2007 TiANWEi
    Copyright 2004 tulipfan

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

#include "global.h"
extern LPSTR lan_time;
extern LPSTR lan_key;
extern LPSTR lan_value;
extern LPSTR lan_dir;
extern LPSTR lan_file;
extern LPSTR lan_menushot;
extern LPSTR lan_menushotsave;
extern LPSTR lan_menuload;


char USERSSTRING_LONG[]="HKEY_USERS";  //1.6 using long name, so in 1.8.1 an option in regshot.ini "UseLongRegHead" control this
char LOCALMACHINESTRING_LONG[]="HKEY_LOCAL_MACHINE";
char USERSSTRING[]="HKU"; //="HKEY_USERS"; // 1.7 using short name tfx, a good idea
char LOCALMACHINESTRING[]="HKLM"; //="HKEY_LOCAL_MACHINE";


void ShowHideCounters(int nCmdShow) //1.8.2
{
	ShowWindow(GetDlgItem(hWnd,IDC_TEXTCOUNT1),nCmdShow);
	ShowWindow(GetDlgItem(hWnd,IDC_TEXTCOUNT2),nCmdShow);
	ShowWindow(GetDlgItem(hWnd,IDC_TEXTCOUNT3),nCmdShow);
}

//////////////////////////////////////////////////////////////////
VOID InitProgressBar(VOID)
{
	//Following are not so good,but they works
	nSavingKey=0;
	nComparing=0;
	nRegStep=nGettingKey/MAXPBPOSITION;
	nFileStep=nGettingFile/MAXPBPOSITION;
	ShowHideCounters(SW_HIDE); //1.8.2
	SendDlgItemMessage(hWnd,IDC_PBCOMPARE,PBM_SETRANGE,(WPARAM)0,MAKELPARAM(0,MAXPBPOSITION));
	SendDlgItemMessage(hWnd,IDC_PBCOMPARE,PBM_SETPOS,(WPARAM)0,(LPARAM)0);
	SendDlgItemMessage(hWnd,IDC_PBCOMPARE,PBM_SETSTEP,(WPARAM)1,(LPARAM)0);
	ShowWindow(GetDlgItem(hWnd,IDC_PBCOMPARE),SW_SHOW);
}

////////////////////////////////////////////////////////////////// renamed from showcounters() at 1.8.2
void UpdateCounters(LPSTR title1,LPSTR title2,DWORD count1,DWORD count2)
{
	//nGettingTime=GetTickCount();
	nBASETIME1=nGettingTime;
	sprintf(lpMESSAGE,"%s%d%s%d%s",lan_time,(nGettingTime-nBASETIME)/1000,"s",(nGettingTime-nBASETIME)%1000,"ms");
	SendDlgItemMessage(hWnd,IDC_TEXTCOUNT3,WM_SETTEXT,(WPARAM)0,(LPARAM)lpMESSAGE);
	sprintf(lpMESSAGE,"%s%d",title1,count1);
	SendDlgItemMessage(hWnd,IDC_TEXTCOUNT1,WM_SETTEXT,(WPARAM)0,(LPARAM)lpMESSAGE);
	sprintf(lpMESSAGE,"%s%d",title2,count2);
	SendDlgItemMessage(hWnd,IDC_TEXTCOUNT2,WM_SETTEXT,(WPARAM)0,(LPARAM)lpMESSAGE);

	UpdateWindow(hWnd);
	PeekMessage(&msg,hWnd,WM_ACTIVATE,WM_ACTIVATE,PM_REMOVE);
	//SetForegroundWindow(hWnd);
}


//--------------------------------------------------
// Prepare the GUI for the Shot about to be taken
//--------------------------------------------------
VOID	UI_BeforeShot(DWORD id)
{
	hHourGlass=LoadCursor(NULL,IDC_WAIT);
	hSaveCursor=SetCursor(hHourGlass);
	EnableWindow(GetDlgItem(hWnd,id),FALSE);
	//Added 1.8.2
	strcpy(lpMESSAGE," ");	//clear counters
	SendDlgItemMessage(hWnd,IDC_TEXTCOUNT1,WM_SETTEXT,(WPARAM)0,(LPARAM)lpMESSAGE);
	SendDlgItemMessage(hWnd,IDC_TEXTCOUNT2,WM_SETTEXT,(WPARAM)0,(LPARAM)lpMESSAGE);
	SendDlgItemMessage(hWnd,IDC_TEXTCOUNT3,WM_SETTEXT,(WPARAM)0,(LPARAM)lpMESSAGE);
	ShowHideCounters(SW_SHOW);
}
//--------------------------------------------------
// Reset the GUI after the shot had been taken
//--------------------------------------------------
VOID	UI_AfterShot(VOID)
{
	DWORD	iddef;
	if(lpHeadLocalMachine1==NULL) {
		iddef=IDC_1STSHOT;
	} else if(lpHeadLocalMachine2==NULL) {
		iddef=IDC_2NDSHOT;
	} else {
		iddef=IDC_COMPARE;
	}
	EnableWindow(GetDlgItem(hWnd,IDC_CLEAR1),TRUE);
	EnableWindow(GetDlgItem(hWnd,iddef),TRUE);
	SendMessage(hWnd,DM_SETDEFID,(WPARAM)iddef,(LPARAM)0);
	SetFocus(GetDlgItem(hWnd,iddef));
	SetCursor(hSaveCursor);
	MessageBeep(0xffffffff);
}
//--------------------------------------------------
// Prepare the GUI for Clearing
//--------------------------------------------------
VOID	UI_BeforeClear(VOID)
{
	//EnableWindow(GetDlgItem(hWnd,IDC_CLEAR1),FALSE);
	hHourGlass=LoadCursor(NULL,IDC_WAIT);
	hSaveCursor=SetCursor(hHourGlass);
	ShowHideCounters(SW_HIDE);
	UpdateWindow(hWnd);
}
//--------------------------------------------------
// Reset the GUI after the clearing
//--------------------------------------------------
VOID	UI_AfterClear(VOID)
{
	DWORD iddef=0;
	//BOOL  bChk; //used for file scan disable
	if(lpHeadLocalMachine1==NULL) {
		iddef=IDC_1STSHOT;
	} else if(lpHeadLocalMachine2==NULL) {
		iddef=IDC_2NDSHOT;
	}
	EnableWindow(GetDlgItem(hWnd,iddef),TRUE);
	EnableWindow(GetDlgItem(hWnd,IDC_COMPARE),FALSE);

	if(lpHeadLocalMachine1==NULL&&lpHeadLocalMachine2==NULL) {
		EnableWindow(GetDlgItem(hWnd,IDC_2NDSHOT),FALSE);
		EnableWindow(GetDlgItem(hWnd,IDC_CLEAR1),FALSE);
		//bChk=TRUE;
	}
	//else  I fogot to comment out this,fixed at 1.8.2
	//bChk=FALSE;

	//EnableWindow(GetDlgItem(hWnd,IDC_CHECKDIR),bChk); //Not used 1.8 //we only enable chk when clear all
	//SendMessage(hWnd,WM_COMMAND,(WPARAM)IDC_CHECKDIR,(LPARAM)0);

	SetFocus(GetDlgItem(hWnd,iddef));
	SendMessage(hWnd,DM_SETDEFID,(WPARAM)iddef,(LPARAM)0);
	SetCursor(hSaveCursor);
	MessageBeep(0xffffffff);
}

VOID	Shot1(VOID)
{
	UINT	nLengthofStr;
	lpHeadLocalMachine1=(LPKEYCONTENT)MYALLOC0(sizeof(KEYCONTENT));
	lpHeadUsers1=(LPKEYCONTENT)MYALLOC0(sizeof(KEYCONTENT));

	if(bUseLongRegHead) { //1.8.1
		lpHeadLocalMachine1->lpkeyname=MYALLOC(sizeof(LOCALMACHINESTRING_LONG));
		lpHeadUsers1->lpkeyname=MYALLOC(sizeof(USERSSTRING_LONG));
		strcpy(lpHeadLocalMachine1->lpkeyname,LOCALMACHINESTRING_LONG);
		strcpy(lpHeadUsers1->lpkeyname,USERSSTRING_LONG);
	} else {
		lpHeadLocalMachine1->lpkeyname=MYALLOC(sizeof(LOCALMACHINESTRING));
		lpHeadUsers1->lpkeyname=MYALLOC(sizeof(USERSSTRING));
		strcpy(lpHeadLocalMachine1->lpkeyname,LOCALMACHINESTRING);
		strcpy(lpHeadUsers1->lpkeyname,USERSSTRING);
	}


	nGettingKey=2;
	nGettingValue=0;
	nGettingTime=0;
	nGettingFile=0,nGettingDir=0;
	nBASETIME=GetTickCount();
	nBASETIME1=nBASETIME;
	UI_BeforeShot(IDC_1STSHOT);

	GetRegistrySnap(HKEY_LOCAL_MACHINE,lpHeadLocalMachine1);
	GetRegistrySnap(HKEY_USERS,lpHeadUsers1);
	nGettingTime=GetTickCount();
	UpdateCounters(lan_key,lan_value,nGettingKey,nGettingValue);

	if(SendMessage(GetDlgItem(hWnd,IDC_CHECKDIR),BM_GETCHECK,(WPARAM)0,(LPARAM)0)==1) {
		DWORD nSubExtDirLen,i;
		LPSTR lpSubExtDir;
		LPHEADFILE lphf,lphftemp;

		GetDlgItemText(hWnd,IDC_EDITDIR,lpExtDir,EXTDIRLEN+2);
		nLengthofStr=strlen(lpExtDir);

		lphf=lphftemp=lpHeadFile1; // changed in 1.8
		lpSubExtDir=lpExtDir;

		if(nLengthofStr>0)
			for(i=0; i<=nLengthofStr; i++) {
				//This is the stupid File Name Detect Routine,[seperate with ";"]
				if(*(lpExtDir+i)==0x3b||*(lpExtDir+i)==0x00) {
					*(lpExtDir+i)=0x00;

					if(*(lpExtDir+i-1)=='\\'&&i>0) {
						*(lpExtDir+i-1)=0x00;
					}

					if(*lpSubExtDir!=0x00) {
						lphf=(LPHEADFILE)MYALLOC0(sizeof(HEADFILE));
						if(lpHeadFile1==NULL) {
							lpHeadFile1=lphf;
						} else {
							lphftemp->lpnextheadfile=lphf;
						}

						lphftemp=lphf;
						lphf->lpfilecontent=(LPFILECONTENT)MYALLOC0(sizeof(FILECONTENT));
						//lphf->lpfilecontent2=(LPFILECONTENT)MYALLOC0(sizeof(FILECONTENT));

						nSubExtDirLen=strlen(lpSubExtDir)+1;
						lphf->lpfilecontent->lpfilename=MYALLOC(nSubExtDirLen);
						//lphf->lpfilecontent2->lpfilename=MYALLOC(nSubExtDirLen);

						strcpy(lphf->lpfilecontent->lpfilename,lpSubExtDir);
						//strcpy(lphf->lpfilecontent2->lpfilename,lpSubExtDir);

						lphf->lpfilecontent->fileattr=FILE_ATTRIBUTE_DIRECTORY;
						//lphf->lpfilecontent2->fileattr=FILE_ATTRIBUTE_DIRECTORY;

						GetFilesSnap(lphf->lpfilecontent);
						nGettingTime=GetTickCount();
						UpdateCounters(lan_dir,lan_file,nGettingDir,nGettingFile);
					}
					lpSubExtDir=lpExtDir+i+1;

				}

			}
	}

	NBW=COMPUTERNAMELEN;
	GetSystemTime(lpSystemtime1);
	GetComputerName(lpComputerName1,&NBW);
	GetUserName(lpUserName1,&NBW);

	UI_AfterShot();


}

// -----------------------------
VOID	Shot2(VOID)
{
	UINT	nLengthofStr;
	lpHeadLocalMachine2=(LPKEYCONTENT)MYALLOC0(sizeof(KEYCONTENT));
	lpHeadUsers2=(LPKEYCONTENT)MYALLOC0(sizeof(KEYCONTENT));

	if(bUseLongRegHead) { //1.8.1
		lpHeadLocalMachine2->lpkeyname=MYALLOC(sizeof(LOCALMACHINESTRING_LONG));
		lpHeadUsers2->lpkeyname=MYALLOC(sizeof(USERSSTRING_LONG));
		strcpy(lpHeadLocalMachine2->lpkeyname,LOCALMACHINESTRING_LONG);
		strcpy(lpHeadUsers2->lpkeyname,USERSSTRING_LONG);
	} else {
		lpHeadLocalMachine2->lpkeyname=MYALLOC(sizeof(LOCALMACHINESTRING));
		lpHeadUsers2->lpkeyname=MYALLOC(sizeof(USERSSTRING));
		strcpy(lpHeadLocalMachine2->lpkeyname,LOCALMACHINESTRING);
		strcpy(lpHeadUsers2->lpkeyname,USERSSTRING);
	}


	nGettingKey=2;
	nGettingValue=0;
	nGettingTime=0;
	nGettingFile=0,nGettingDir=0;
	nBASETIME=GetTickCount();
	nBASETIME1=nBASETIME;
	UI_BeforeShot(IDC_2NDSHOT);

	GetRegistrySnap(HKEY_LOCAL_MACHINE,lpHeadLocalMachine2);
	GetRegistrySnap(HKEY_USERS,lpHeadUsers2);
	nGettingTime=GetTickCount();
	UpdateCounters(lan_key,lan_value,nGettingKey,nGettingValue);

	if(SendMessage(GetDlgItem(hWnd,IDC_CHECKDIR),BM_GETCHECK,(WPARAM)0,(LPARAM)0)==1) {
		DWORD nSubExtDirLen,i;
		LPSTR lpSubExtDir;
		LPHEADFILE lphf,lphftemp;

		GetDlgItemText(hWnd,IDC_EDITDIR,lpExtDir,EXTDIRLEN+2);
		nLengthofStr=strlen(lpExtDir);

		lphf=lphftemp=lpHeadFile1; // changed in 1.8
		lpSubExtDir=lpExtDir;

		if(nLengthofStr>0)
			for(i=0; i<=nLengthofStr; i++) {
				//This is the stupid File Name Detect Routine,[seperate with ";"]
				if(*(lpExtDir+i)==0x3b||*(lpExtDir+i)==0x00) {
					*(lpExtDir+i)=0x00;

					if(*(lpExtDir+i-1)=='\\'&&i>0) {
						*(lpExtDir+i-1)=0x00;
					}

					if(*lpSubExtDir!=0x00) {
						lphf=(LPHEADFILE)MYALLOC0(sizeof(HEADFILE));
						if(lpHeadFile2==NULL) {
							lpHeadFile2=lphf;
						} else {
							lphftemp->lpnextheadfile=lphf;
						}

						lphftemp=lphf;
						lphf->lpfilecontent=(LPFILECONTENT)MYALLOC0(sizeof(FILECONTENT));

						nSubExtDirLen=strlen(lpSubExtDir)+1;
						lphf->lpfilecontent->lpfilename=MYALLOC(nSubExtDirLen);

						strcpy(lphf->lpfilecontent->lpfilename,lpSubExtDir);

						lphf->lpfilecontent->fileattr=FILE_ATTRIBUTE_DIRECTORY;

						GetFilesSnap(lphf->lpfilecontent);
						nGettingTime=GetTickCount();
						UpdateCounters(lan_dir,lan_file,nGettingDir,nGettingFile);
					}
					lpSubExtDir=lpExtDir+i+1;

				}

			}
	}



	NBW=COMPUTERNAMELEN;
	GetSystemTime(lpSystemtime2);
	GetComputerName(lpComputerName2,&NBW);
	GetUserName(lpUserName2,&NBW);
	UI_AfterShot();
}


//--------------------------------------------------
//Show popup shortcut menu
//--------------------------------------------------
VOID CreateShotPopupMenu(VOID)
{
	hMenu=CreatePopupMenu();
	AppendMenu(hMenu,MF_STRING,IDM_SHOTONLY,lan_menushot);
	AppendMenu(hMenu,MF_STRING,IDM_SHOTSAVE,lan_menushotsave);
	AppendMenu(hMenu,MF_SEPARATOR,IDM_BREAK,NULL);
	AppendMenu(hMenu,MF_STRING,IDM_LOAD,lan_menuload);
	SetMenuDefaultItem(hMenu,IDM_SHOTONLY,FALSE);
}


