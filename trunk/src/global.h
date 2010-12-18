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

#ifdef __GNUC__
#include <unistd.h>
#endif
#include <windows.h>
#include <stdio.h>
#include <shlobj.h>
#include "resource.h"


//!!!WARNING!!! HEAP_NO_SERIALIZE mean we can not use this in multithread.
//added in 1.8.2 to gain a slightly faster speed but it is danger!
//#define	USEHEAPALLOC_DANGER

#ifdef USEHEAPALLOC_DANGER

#define	MYALLOC(x)	HeapAlloc(hHeap,1,x)
#define	MYALLOC0(x)	HeapAlloc(hHeap,9,x) //HEAP_NO_SERIALIZE|HEAP_ZERO_MEMORY ,1|8
#define	MYFREE(x)	HeapFree(hHeap,1,x)

#else

#define MYALLOC(x)	GlobalAlloc(GMEM_FIXED,x)
#define MYALLOC0(x)	GlobalAlloc(GPTR,x)
#define MYFREE(x)	GlobalFree(x)

#endif

//#define WIN32_LEAN_AND_MEAN

//#define DEBUGLOG
//Some definations
#define SIZEOFREG_DWORD	4		//In current windows ,reg_dword's size =4
#define NOTMATCH		0		//Define modification type in compare results
#define ISMATCH			1
#define ISDEL			2
#define ISADD			3
#define ISMODI			4

#define KEYADD			1
#define KEYDEL			2
#define VALADD			3
#define VALDEL			4
#define VALMODI			5
#define FILEADD			6
#define FILEDEL			7
#define FILEMODI		8
#define DIRADD			9
#define DIRDEL			10
#define DIRMODI			11

#define REFRESHINTERVAL	110		//Define progress refresh rate
#define MAXPBPOSITION	100		//Define progress bar length
#define COMMENTLENGTH	50		//Define commentfield length on the MainForm
#define HTMLWRAPLENGTH	1000	//Define html out put wrap length
#define MAXAMOUNTOFFILE	10000	//Define out put file counts
#define	EXTDIRLEN	MAX_PATH*3	//Define Searching Directory field length
#define COMPUTERNAMELEN	64		//Define COMPUTER name length
#define HIVEBEGINOFFSET	512		//Hive file out put header computerlen*2+sizeof(systemtime)+32 must <hivebeginoffset!!!!!!!!!!!!!!


//Some definations of MutiLanguage strings [Free space length]
#define SIZEOF_LANGUAGE_SECTIONNAMES_BUFFER 2048
#define SIZEOF_SINGLE_LANGUAGENAME	64
#define SIZEOF_FREESTRINGS 16384
#define SIZEOF_ABOUTBOX 2048


//Struct used for Windows Registry Key
struct	_KEYCONTENT {
	LPSTR	lpkeyname;							//Pointer to Key Name
	struct	_VALUECONTENT FAR * lpfirstvalue;	//Pointer to Key's first Value
	struct	_KEYCONTENT	FAR * lpfirstsubkey;	//Pointer to Key's first Sub Key
	struct	_KEYCONTENT	FAR * lpbrotherkey;		//Pointer to Key's brother
	struct	_KEYCONTENT	FAR * lpfatherkey;		//Pointer to Key's father
	BYTE	bkeymatch;							//Flag used at comparing

};
typedef struct _KEYCONTENT KEYCONTENT,FAR * LPKEYCONTENT;


//Struct used for Windows Registry Value
struct	_VALUECONTENT {
	DWORD	typecode;							//Type of Value [DWORD,STRING...]
	DWORD	datasize;							//Value Data size in bytes
	LPSTR	lpvaluename;						//Pointer to Value Name
	LPBYTE	lpvaluedata;						//Pointer to Value Data
	struct	_VALUECONTENT FAR * lpnextvalue;	//Pointer to Value's brother
	struct	_KEYCONTENT	FAR * lpfatherkey;		//Pointer to Value's father[Key]
	BYTE	bvaluematch;						//Flag used at comparing
};
typedef struct _VALUECONTENT VALUECONTENT,FAR * LPVALUECONTENT;


//Struct used for Windows File System
struct	_FILECONTENT {
	LPSTR	lpfilename;							//Pointer to File Name
	DWORD	writetimelow;						//File write time [LOW  DWORD]
	DWORD	writetimehigh;						//File write time [HIGH DWORD]
	DWORD	filesizelow;						//File size	 [LOW  DWORD]
	DWORD	filesizehigh;						//File size	 [HIGH DWORD]
	DWORD	fileattr;							//File Attributes
	DWORD	cksum;								//File checksum(plan to add in 1.8 not used now)
	struct	_FILECONTENT FAR * lpfirstsubfile;	//Pointer to Files[DIRS] first sub file
	struct	_FILECONTENT FAR * lpbrotherfile;	//Pointer to Files[DIRS] brother
	struct	_FILECONTENT FAR * lpfatherfile;	//Pointer to Files father
	BYTE	bfilematch;							//Flag used at comparing
};
typedef struct _FILECONTENT FILECONTENT,FAR * LPFILECONTENT;


//Struct use for file tree compare
/* <=1.7.3
struct	_HEADFILE
{
	LPFILECONTENT	lpfilecontent1;				//Pointer to filecontent at 1st shot
	LPFILECONTENT	lpfilecontent2;				//Pointer to filecontent at 2nd shot
	struct _HEADFILE	FAR *	lpnextheadfile;	//Pointer to next headfile struc
};
*/


//Adjusted for filecontent saving. 1.8
struct	_HEADFILE {
	struct _HEADFILE	FAR *	lpnextheadfile;	//Pointer to next headfile struc
	LPFILECONTENT	lpfilecontent;				//Pointer to filecontent
};
typedef	struct	_HEADFILE	HEADFILE,FAR * LPHEADFILE;


//Struct use for compare result output
struct  _COMRESULT {
	LPSTR	lpresult;							//Pointer to result string
	struct	_COMRESULT FAR * lpnextresult;		//Pointer to next _COMRESULT
};
typedef struct _COMRESULT COMRESULT,FAR * LPCOMRESULT;


//Pointers to compare result [see above.]
LPCOMRESULT	lpKEYADD;
LPCOMRESULT	lpKEYDEL;
LPCOMRESULT	lpVALADD;
LPCOMRESULT	lpVALDEL;
LPCOMRESULT	lpVALMODI;
LPCOMRESULT	lpFILEADD;
LPCOMRESULT	lpFILEDEL;
LPCOMRESULT	lpFILEMODI;
LPCOMRESULT	lpDIRADD;
LPCOMRESULT	lpDIRDEL;
LPCOMRESULT	lpDIRMODI;


LPCOMRESULT	lpKEYADDHEAD;
LPCOMRESULT	lpKEYDELHEAD;
LPCOMRESULT	lpVALADDHEAD;
LPCOMRESULT	lpVALDELHEAD;
LPCOMRESULT	lpVALMODIHEAD;
LPCOMRESULT	lpFILEADDHEAD;
LPCOMRESULT	lpFILEDELHEAD;
LPCOMRESULT	lpFILEMODIHEAD;
LPCOMRESULT	lpDIRADDHEAD;
LPCOMRESULT	lpDIRDELHEAD;
LPCOMRESULT	lpDIRMODIHEAD;

//Number of Modification detected
DWORD	nKEYADD;
DWORD	nKEYDEL;
DWORD	nVALADD;
DWORD	nVALDEL;
DWORD	nVALMODI;
DWORD	nFILEADD;
DWORD	nFILEDEL;
DWORD	nFILEMODI;
DWORD	nDIRADD;
DWORD	nDIRDEL;
DWORD	nDIRMODI;


//Some DWORD used to show the progress bar and etc
DWORD	nGettingValue;
DWORD	nGettingKey,nComparing,nRegStep,nFileStep,nSavingKey;
DWORD	nGettingTime,nBASETIME,nBASETIME1;
DWORD	nGettingFile,nGettingDir,nSavingFile;
//DWORD	nMask=0xf7fd; //not used now ,but should be added
//DWORD	nRegMessageCount=0;
DWORD	NBW; //that is: NumberOfBytesWritten;

//Pointers to Registry Key
LPKEYCONTENT	lpHeadLocalMachine1;		//Pointer to HKEY_LOCAL_MACHINE	1
LPKEYCONTENT	lpHeadLocalMachine2;		//Pointer to HKEY_LOCAL_MACHINE 2
LPKEYCONTENT	lpHeadUsers1;				//Pointer to HKEY_USERS 1
LPKEYCONTENT	lpHeadUsers2;				//Pointer to HKEY_USERS 1
LPHEADFILE		lpHeadFile1,lpHeadFile2;	//Pointer to Headfile
LPSTR			lpTempHive1,lpTempHive2;	//Pointer for load hive files
LPSTR			lpComputerName1,lpComputerName2,lpUserName1,lpUserName2;
SYSTEMTIME FAR * lpSystemtime1,* lpSystemtime2;

//Some pointers need to allocate enough space to working
LPSTR	lpKeyName,lpMESSAGE,lpExtDir,lpOutputpath,lpLastSaveDir,lpLastOpenDir,lpCurrentLanguage;
LPSTR	lpWindowsDirName,lpTempPath,lpStartDir,lpIni,lpFreeStrings,lpCurrentTranslator;

//LPSTR	REGSHOTDATFILE		="rgst152.dat";
LPSTR	lpProgramDir;
LPDWORD	lpSnapRegs, lpSnapFiles;
LPSTR	lpRegshotIni;
LPSTR	lpSnapRegsStr,lpSnapFilesStr,lpSnapKey,lpSnapReturn;

LPDWORD	ldwTempStrings;


//Former definations used at Dynamic Monitor Engine.Not Used NOW
//#define	DIOCPARAMSSIZE	20		//4+4+4+8 bytes DIOCParams size!
//#define	MAXLISTBOXLEN	1024
//#define	RING3TDLEN		8		//ring3 td name length
//LPSTR		str_errorini="Error create Dialog!";
//INT		tabarray[]={40,106,426,466};		// the tabstops is the len addup!
//BOOL		bWinNTDetected;
//UINT		WM_REGSHOT=0;

#ifdef	DEBUGLOG
LPSTR	lstrdb1;
#endif

MSG				msg;							//Windows MSG struct
HWND			hWnd;							//The handle of REGSHOT
HMENU			hMenu,hMenuClear;				//The handles of shortcut menus
HANDLE			hFile,hFileWholeReg;			//Handle of file regshot use
HCURSOR			hHourGlass;						//Handle of cursor
HCURSOR			hSaveCursor;					//Handle of cursor
BOOL			is1;							//Flag to determine is the 1st shot
BOOL			is1LoadFromHive,is2LoadFromHive;//Flag to determine are shots load from hive files
RECT			rect;							//Window RECT
FILETIME		ftLastWrite;					//Filetime struct
BROWSEINFO		BrowseInfo1;					//BrowseINFO struct
OPENFILENAME	opfn;							//Openfilename struct
BOOL			bUseLongRegHead;				//1.8.1 for compatible to 1.61e5 and undoreg1.46
HANDLE			hHeap;							//1.8.2

VOID	LogToMem(DWORD actiontype,LPDWORD lpcount,LPVOID lp);
BOOL	GetSnapRegs(HWND hDlg);
BOOL	SetSnapRegs(HWND hDlg);
BOOL	IsInSkipList(LPSTR lpSnap, LPDWORD lpSkipList);
VOID	UpdateCounters(LPSTR title1,LPSTR title2,DWORD count1,DWORD count2);
LPSTR	AtPos(LPSTR lpMaster,LPSTR lp,DWORD size);
BOOL	GetLanguageType(HWND hDlg);
VOID	GetDefaultStrings(VOID);
VOID	PointToNewStrings(VOID);
BOOL	GetLanguageStrings(HWND hDlg);
VOID	CreateShotPopupMenu(VOID);
VOID	UI_BeforeShot(DWORD id);
VOID	UI_AfterShot(VOID);
VOID	UI_BeforeClear(VOID);
VOID	UI_AfterClear(VOID);

VOID	Shot1(void);
VOID	Shot2(void);
BOOL	CompareShots(void);
VOID	SaveHive(LPKEYCONTENT lpKeyHLM,LPKEYCONTENT lpKeyUSER, LPHEADFILE lpHF,LPSTR computer,LPSTR user,LPVOID time);
BOOL	LoadHive(LPKEYCONTENT FAR * lplpKeyHLM,LPKEYCONTENT FAR * lplpKeyUSER, LPHEADFILE FAR * lpHF,LPSTR FAR * lpHive);
VOID	FreeAllCompareResults(void);
VOID	FreeAllKeyContent1(void);
VOID	FreeAllKeyContent2(void);
VOID	FreeAllFileHead(LPHEADFILE lp);
VOID	ClearKeyMatchTag(LPKEYCONTENT lpKey);
VOID	GetRegistrySnap(HKEY hkey,LPKEYCONTENT lpFatherKeyContent);		//HWND hDlg,first para deleted in 1.8, return from void * to void
VOID	GetFilesSnap(LPFILECONTENT lpFatherFile);						//HWND hDlg,first para deleted in 1.8
LPSTR	GetWholeFileName(LPFILECONTENT lpFileContent);
VOID	InitProgressBar(VOID);
VOID	CompareFirstSubFile(LPFILECONTENT lpHead1,LPFILECONTENT lpHead2);
BOOL	ReplaceInValidFileName(LPSTR lpf);
VOID	ErrMsg(char * note);
VOID	WriteHead(u_char * lpstr,DWORD count,BOOL isHTML);
VOID	WritePart(LPCOMRESULT lpcomhead,BOOL isHTML,BOOL usecolor);
VOID	WriteTitle(LPSTR lph,LPSTR lpb,BOOL isHTML);
VOID	SaveFileContent(LPFILECONTENT lpFileContent, DWORD nFPCurrentFatherFile,DWORD nFPCaller);
VOID	ClearHeadFileMatchTag(LPHEADFILE lpHF);
VOID	FindDirChain(LPHEADFILE lpHF,LPSTR lpDir,int nMaxLen);
BOOL	DirChainMatch(LPHEADFILE lphf1,LPHEADFILE lphf2);
VOID	WriteHtmlbegin(void);
VOID	WriteHtmlover(void);
VOID	WriteHtmlbr(void);
VOID	ReAlignFile(LPHEADFILE lpHF,DWORD nBase);
LPFILECONTENT SearchDirChain(LPSTR lpname,LPHEADFILE lpHF);
VOID	GetAllSubFile(BOOL needbrother,DWORD typedir,DWORD typefile,LPDWORD lpcountdir,LPDWORD lpcountfile, LPFILECONTENT lpFileContent);
