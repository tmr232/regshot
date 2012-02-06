/*
    Copyright 1999-2003,2007,2011 TiANWEi
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

#ifndef REGSHOT_GLOBAL_H
#define REGSHOT_GLOBAL_H


#ifdef __GNUC__
#include <unistd.h>
#endif
#include <windows.h>
#include <stdio.h>
#include <shlobj.h>
#include <stddef.h>  // for "offsetof" macro
#include <string.h>
#include <tchar.h>
#include "resource.h"

#if defined(_MSC_VER) && (_MSC_VER < 1300)  // before VS 2002 .NET (e.g. VS 6)
#define SetClassLongPtr SetClassLong
#ifndef GCLP_HICON
#define GCLP_HICON GCL_HICON
#endif
#if !defined(LONG_PTR)
typedef long LONG_PTR;
#endif
#endif  // _MSC_VER && (_MSC_VER < 1300)

// added in 1.8.2 to gain a slightly faster speed but it is danger!
#define USEHEAPALLOC_DANGER
//#define DEBUGLOG

#ifdef USEHEAPALLOC_DANGER

// MSDN doc say use HEAP_NO_SERIALIZE is not good for process heap :( so change fromm 1 to 0 20111216, slower than using 1
#define MYALLOC(x)  HeapAlloc(hHeap,0,x)
#define MYALLOC0(x) HeapAlloc(hHeap,8,x) // (HEAP_NO_SERIALIZE|) HEAP_ZERO_MEMORY ,1|8
#define MYFREE(x)   HeapFree(hHeap,0,x)

#else

#define MYALLOC(x)  GlobalAlloc(GMEM_FIXED,x)
#define MYALLOC0(x) GlobalAlloc(GPTR,x)
#define MYFREE(x)   GlobalFree(x)

#endif


// Some definitions
#define NOTMATCH        0             // Define modification type in comparison results
#define ISMATCH         1
#define ISDEL           2
#define ISADD           3
#define ISMODI          4

#define KEYADD          1
#define KEYDEL          2
#define VALADD          3
#define VALDEL          4
#define VALMODI         5
#define FILEADD         6
#define FILEDEL         7
#define FILEMODI        8
#define DIRADD          9
#define DIRDEL          10
#define DIRMODI         11

#define ESTIMATE_VALUEDATA_LENGTH 1024*1024 //Define estimated value data in scan
#define REFRESHINTERVAL 110          // Define progress refresh rate
#define MAXPBPOSITION   100          // Define progress bar length
#define COMMENTLENGTH   50           // Define commentfield length on the MainForm
#define HTMLWRAPLENGTH  1000         // Define html out put wrap length
#define MAXAMOUNTOFFILE 10000        // Define out put file counts
#define EXTDIRLEN       MAX_PATH * 4 // Define searching directory field length
#define OLD_COMPUTERNAMELEN 64       // Define COMPUTER name length, do not change


// Some definitions of MutiLanguage strings [Free space length]
#define SIZEOF_LANGUAGE_SECTIONNAMES_BUFFER 2048
#define SIZEOF_SINGLE_LANGUAGENAME          64
#define MAX_INI_SECTION_CHARS               32767
#define SIZEOF_ABOUTBOX                     4096


// Struct used for Windows Registry Key
struct _KEYCONTENT {
    LPTSTR lpKeyName;                         // Pointer to key's name
    struct _VALUECONTENT FAR *lpFirstVC;   // Pointer to key's first value
    struct _KEYCONTENT FAR *lpFirstSubKC;  // Pointer to key's first sub key
    struct _KEYCONTENT FAR *lpBrotherKC;   // Pointer to key's brother
    struct _KEYCONTENT FAR *lpFatherKC;    // Pointer to key's father
    DWORD  bkeymatch;                         // Flags used when comparing, until 1.8.2 was byte
};
typedef struct _KEYCONTENT KEYCONTENT, FAR *LPKEYCONTENT;


// Struct used for Windows Registry Value
struct _VALUECONTENT {
    DWORD  typecode;                          // Type of value [DWORD,STRING...]
    DWORD  datasize;                          // Value data size in bytes
    LPTSTR lpValueName;                       // Pointer to value's name
    LPBYTE lpValueData;                       // Pointer to value's data
    struct _VALUECONTENT FAR *lpBrotherVC;    // Pointer to value's brother
    struct _KEYCONTENT FAR *lpFatherKC;       // Pointer to value's father key
    DWORD  bvaluematch;                       // Flags used when comparing, until 1.8.2 was byte
};
typedef struct _VALUECONTENT VALUECONTENT, FAR *LPVALUECONTENT;


// Struct used for Windows File System
struct _FILECONTENT {
    LPTSTR lpFileName;                        // Pointer to file's name
    DWORD  writetimelow;                      // File write time [LOW  DWORD]
    DWORD  writetimehigh;                     // File write time [HIGH DWORD]
    DWORD  filesizelow;                       // File size [LOW  DWORD]
    DWORD  filesizehigh;                      // File size [HIGH DWORD]
    DWORD  fileattr;                          // File attributes (e.g. directory)
    DWORD  chksum;                            // File checksum (planned for the future, currently not used)
    struct _FILECONTENT FAR *lpFirstSubFC;    // Pointer to file's first sub file
    struct _FILECONTENT FAR *lpBrotherFC;     // Pointer to file's brother
    struct _FILECONTENT FAR *lpFatherFC;      // Pointer to file's father
    DWORD  bfilematch;                        // Flags used when comparing, until 1.8.2 was byte
};
typedef struct _FILECONTENT FILECONTENT, FAR *LPFILECONTENT;


// Adjusted for filecontent saving. 1.8
struct _HEADFILE {
    struct _HEADFILE FAR *lpBrotherHF;        // Pointer to head file's brother
    LPFILECONTENT   lpFirstFC;                // Pointer to head file's first file
};
typedef struct  _HEADFILE HEADFILE, FAR *LPHEADFILE;


// Struct used for comparing result output
struct _COMRESULT {
    LPSTR  lpresult;                          // Pointer to result string
    struct _COMRESULT FAR *lpnextresult;      // Pointer to next _COMRESULT
};
typedef struct _COMRESULT COMRESULT, FAR *LPCOMRESULT;

// Struct for shot,2012.
struct _REGSHOT {
    LPKEYCONTENT  lpHKLM;        // Pointer to Shot's HKLM registry keys
    LPKEYCONTENT  lpHKU;         // Pointer to Shot's HKU registry keys
    LPHEADFILE    lpHF;          // Pointer to Shot's head files
    LPTSTR        computername;  // Pointer to Shot's computer name
    LPTSTR        username;      // Pointer to Shot's user name
    SYSTEMTIME    systemtime;
};
typedef struct _REGSHOT REGSHOT, FAR *LPREGSHOT;


// Struct for file header, used in saving and loading
// when accessing fields of this structure always put a version check around them, e.g. "if version >= 2 then use ofsComputerName"
struct _FILEHEADER {
    char signature[12]; // ofs 0 len 12: never convert to Unicode, always use char type and SBCS/MBCS
    DWORD nFHSize;      // (v2) ofs 12 len 4: size of file header incl. signature and header size field

    DWORD ofsHKLM;      // ofs 16 len 4
    DWORD ofsHKU;       // ofs 20 len 4
    DWORD ofsHF;        // ofs 24 len 4: added in 1.8
    DWORD reserved;     // ofs 28 len 4

    // next two fields are kept and filled for <= 1.8.2 compatibility, see substituting fields in file header format version 2
    char computername[OLD_COMPUTERNAMELEN];  // ofs 32 len 64: DO NOT CHANGE, keep as SBCS/MBCS, name maybe truncated or missing NULL char
    char username[OLD_COMPUTERNAMELEN];      // ofs 96 len 64: DO NOT CHANGE, keep as SBCS/MBCS, name maybe truncated or missing NULL char

    SYSTEMTIME systemtime;  // ofs 160 len 16

    // extended values exist only since structure version 2
    // new since file header version 2
    DWORD nFHVersion;        // (v2) ofs 176 len 4: File header structure version
    DWORD nCharSize;         // (v2) ofs 180 len 4: sizeof(TCHAR), allows to separate between SBCS/MBCS (1 for char) and Unicode (2 for UTF-16 WCHAR), may become 4 for UTF-32 in the future

    DWORD nKCVersion;        // (v2) ofs 184 len 4: Key content structure version
    DWORD nKCSize;           // (v2) ofs 188 len 4: Size of key content

    DWORD nVCVersion;        // (v2) ofs 192 len 4: Value content structure version
    DWORD nVCSize;           // (v2) ofs 196 len 4: Size of value content

    DWORD nHFVersion;        // (v2) ofs 200 len 4: Head File structure version
    DWORD nHFSize;           // (v2) ofs 204 len 4: Size of file content

    DWORD nFCVersion;        // (v2) ofs 208 len 4: File content structure version
    DWORD nFCSize;           // (v2) ofs 212 len 4: Size of file content

    DWORD ofsComputerName;   // (v2) ofs 216 len 4
    DWORD nComputerNameLen;  // (v2) ofs 220 len 4: length in chars incl. NULL char

    DWORD ofsUserName;       // (v2) ofs 224 len 4
    DWORD nUserNameLen;      // (v2) ofs 228 len 4: length in chars incl. NULL char

    // ^^^ here the file header structure can be extended
    // * increase the version number for the new file header format
    // * place a comment with a reference to the new version before the new fields
    // * check all usages and version checks
    // * remember that older versions can not use these additional data
};
typedef struct _FILEHEADER FILEHEADER, FAR *LPFILEHEADER;

#define FILEHEADER_VERSION_EMPTY 0
#define FILEHEADER_VERSION_1 1
#define FILEHEADER_VERSION_2 2
#define FILEHEADER_VERSION_CURRENT FILEHEADER_VERSION_2

// Struct for reg key, used in saving and loading
// when accessing fields of this structure always put a version check around them, e.g. "if version >= 2 then use nKeyNameLen"
struct _SAVEKEYCONTENT {
    DWORD ofsKeyName;      // Position of key's name
    DWORD ofsFirstValue;   // Position of key's first value
    DWORD ofsFirstSubKey;  // Position of key's first sub key
    DWORD ofsBrotherKey;   // Position of key's brother key
    DWORD ofsFatherKey;    // Position of key's father key

    // extended values exist only since structure version 2
    // new since key content version 2
    DWORD nKeyNameLen;  // (v2) Length of key's name in chars incl. NULL char (up to 1.8.2 there a was single byte for internal bkeymatch field that was always zero)

    // ^^^ here the key content structure can be extended
    // * increase the version number for the new key content format
    // * place a comment with a reference to the new version before the new fields
    // * check all usages and version checks
    // * remember that older versions can not use these additional data
};
typedef struct _SAVEKEYCONTENT SAVEKEYCONTENT, FAR *LPSAVEKEYCONTENT;

#define KEYCONTENT_VERSION_EMPTY 0
#define KEYCONTENT_VERSION_1 1
#define KEYCONTENT_VERSION_2 2
#define KEYCONTENT_VERSION_CURRENT KEYCONTENT_VERSION_2


// Struct for reg value, used in saving and loading
// when accessing fields of this structure always put a version check around them, e.g. "if version >= 2 then use nValueNameLen"
struct _SAVEVALUECONTENT {
    DWORD typecode;         // Type of value [DWORD,STRING...]
    DWORD datasize;         // Value data size in bytes
    DWORD ofsValueName;     // Position of value's name
    DWORD ofsValueData;     // Position of value's data
    DWORD ofsBrotherValue;  // Position of value's brother value
    DWORD ofsFatherKey;     // Position of value's father key

    // extended values exist only since structure version 2
    // new since value content version 2
    DWORD nValueNameLen;  // (v2) Length of value's name in chars incl. NULL char (up to 1.8.2 there a was single byte for internal bkeymatch field that was always zero)

    // ^^^ here the value content structure can be extended
    // * increase the version number for the new value content format
    // * place a comment with a reference to the new version before the new fields
    // * check all usages and version checks
    // * remember that older versions can not use these additional data
};
typedef struct _SAVEVALUECONTENT SAVEVALUECONTENT, FAR *LPSAVEVALUECONTENT;

#define VALUECONTENT_VERSION_EMPTY 0
#define VALUECONTENT_VERSION_1 1
#define VALUECONTENT_VERSION_2 2
#define VALUECONTENT_VERSION_CURRENT KEYCONTENT_VERSION_2


// Struct for dirs and files, used in saving and loading
// when accessing fields of this structure always put a version check around them, e.g. "if version >= 2 then use nFileNameLen"
struct _SAVEFILECONTENT {
    DWORD ofsFileName;      // Position of file name
    DWORD writetimelow;     // File write time [LOW  DWORD]
    DWORD writetimehigh;    // File write time [HIGH DWORD]
    DWORD filesizelow;      // File size [LOW  DWORD]
    DWORD filesizehigh;     // File size [HIGH DWORD]
    DWORD fileattr;         // File attributes
    DWORD chksum;           // File checksum (planned for the future, currently not used)
    DWORD ofsFirstSubFile;  // Position of file's first sub file
    DWORD ofsBrotherFile;   // Position of file's brother file
    DWORD ofsFatherFile;    // Position of file's father file

    // extended values exist only since structure version 2
    // new since file content version 2
    DWORD nFileNameLen;  // (v2) Length of file's name in chars incl. NULL char (up to 1.8.2 there a was single byte for internal bkeymatch field that was always zero)

    // ^^^ here the file content structure can be extended
    // * increase the version number for the new file content format
    // * place a comment with a reference to the new version before the new fields
    // * check all usages and version checks
    // * remember that older versions can not use these additional data
};
typedef struct _SAVEFILECONTENT SAVEFILECONTENT, FAR *LPSAVEFILECONTENT;

#define FILECONTENT_VERSION_EMPTY 0
#define FILECONTENT_VERSION_1 1
#define FILECONTENT_VERSION_2 2
#define FILECONTENT_VERSION_CURRENT KEYCONTENT_VERSION_2


// Adjusted for filecontent saving. 1.8
struct _SAVEHEADFILE {
    DWORD ofsBrotherHeadFile;   // Position of head file's brother head file
    DWORD ofsFirstFileContent;  // Position of head file's first file content

    // extended values exist only since structure version 2

    // ^^^ here the head file structure can be extended
    // * increase the version number for the new head file format
    // * place a comment with a reference to the new version before the new fields
    // * check all usages and version checks
    // * remember that older versions can not use these additional data
};
typedef struct  _SAVEHEADFILE SAVEHEADFILE, FAR *LPSAVEHEADFILE;

#define HEADFILE_VERSION_EMPTY 0
#define HEADFILE_VERSION_1 1
#define HEADFILE_VERSION_CURRENT HEADFILE_VERSION_1


// Pointers to compare result [see above]
LPCOMRESULT lpKEYADD;
LPCOMRESULT lpKEYDEL;
LPCOMRESULT lpVALADD;
LPCOMRESULT lpVALDEL;
LPCOMRESULT lpVALMODI;
LPCOMRESULT lpFILEADD;
LPCOMRESULT lpFILEDEL;
LPCOMRESULT lpFILEMODI;
LPCOMRESULT lpDIRADD;
LPCOMRESULT lpDIRDEL;
LPCOMRESULT lpDIRMODI;


LPCOMRESULT lpKEYADDHEAD;
LPCOMRESULT lpKEYDELHEAD;
LPCOMRESULT lpVALADDHEAD;
LPCOMRESULT lpVALDELHEAD;
LPCOMRESULT lpVALMODIHEAD;
LPCOMRESULT lpFILEADDHEAD;
LPCOMRESULT lpFILEDELHEAD;
LPCOMRESULT lpFILEMODIHEAD;
LPCOMRESULT lpDIRADDHEAD;
LPCOMRESULT lpDIRDELHEAD;
LPCOMRESULT lpDIRMODIHEAD;


// Number of Modification detected
DWORD   nKEYADD;
DWORD   nKEYDEL;
DWORD   nVALADD;
DWORD   nVALDEL;
DWORD   nVALMODI;
DWORD   nFILEADD;
DWORD   nFILEDEL;
DWORD   nFILEMODI;
DWORD   nDIRADD;
DWORD   nDIRDEL;
DWORD   nDIRMODI;


// Some DWORD used to show the progress bar and etc
DWORD   nGettingValue;
DWORD   nGettingKey;
DWORD   nComparing;
DWORD   nRegStep;
DWORD   nFileStep;
DWORD   nSavingKey;
DWORD   nGettingTime;
DWORD   nBASETIME;
DWORD   nBASETIME1;
DWORD   nGettingFile;
DWORD   nGettingDir;
DWORD   nSavingFile;
//DWORD   nMask = 0xf7fd;     // not used now, but should be added
//DWORD   nRegMessageCount = 0;
DWORD   NBW;                // that is: NumberOfBytesWritten;


extern REGSHOT Shot1;
extern REGSHOT Shot2;
// Pointers to Registry Key
/*
LPKEYCONTENT    lpHeadLocalMachine1;    // Pointer to HKEY_LOCAL_MACHINE 1
LPKEYCONTENT    lpHeadLocalMachine2;    // Pointer to HKEY_LOCAL_MACHINE 2
LPKEYCONTENT    lpHeadUsers1;           // Pointer to HKEY_USERS 1
LPKEYCONTENT    lpHeadUsers2;
LPHEADFILE      lpHeadFile1;            // Pointer to headfile
LPHEADFILE      lpHeadFile2;
LPBYTE          lpTempHive1;            // Pointer for loading hive files
LPBYTE          lpTempHive2;
LPSTR           lpComputerName1;
LPSTR           lpComputerName2;
LPSTR           lpUserName1;
LPSTR           lpUserName2;
SYSTEMTIME FAR *lpSystemtime1, * lpSystemtime2;
*/

// Some pointers need to allocate enough space to working
LPSTR   lpKeyName;   // following used in scan engine
LPSTR   lpValueName;
LPBYTE  lpValueData;
LPBYTE  lpValueDataS;


#define REGSHOT_MESSAGE_LENGTH 256
extern LPTSTR lpszMessage;
LPSTR   lpExtDir;
LPTSTR  lpOutputpath;
LPTSTR  lpLastSaveDir;
LPTSTR  lpLastOpenDir;
extern LPTSTR lpszLanguage;
LPSTR   lpWindowsDirName;
LPSTR   lpTempPath;
LPSTR   lpStartDir;
LPSTR   lpLanguageIni;  // For language.ini
LPSTR   lpCurrentTranslator;
LPSTR   lpRegshotIni;

LPTSTR *lplpRegSkipStrings;
LPTSTR *lplpFileSkipStrings;
extern LPTSTR lpgrszLangSection;
//LPSTR REGSHOTDATFILE = "rgst152.dat";
//LPSTR   lpProgramDir;   // tfx define
//LPSTR   lpSnapKey;
//LPSTR   lpSnapReturn;



// Former definations used at Dynamic Monitor Engine. Not Used NOW
//#define DIOCPARAMSSIZE 20      // 4+4+4+8 bytes DIOCParams size!
//#define MAXLISTBOXLEN  1024
//#define RING3TDLEN     8       // ring3 td name length
//LPSTR str_errorini = "Error create Dialog!";
//INT   tabarray[] = {40,106,426,466};      // the tabstops is the len addup!
//BOOL  bWinNTDetected;
//UINT  WM_REGSHOT = 0;

#ifdef DEBUGLOG
LPSTR   lstrdb1;
#endif

MSG             msg;                // Windows MSG struct
HWND            hWnd;               // The handle of REGSHOT
HMENU           hMenu;              // The handles of shortcut menus
HMENU           hMenuClear;         // The handles of shortcut menus
HANDLE          hFile;              // Handle of file regshot use
HANDLE          hFileWholeReg;      // Handle of file regshot use
HCURSOR         hHourGlass;         // Handle of cursor
HCURSOR         hSaveCursor;        // Handle of cursor
BOOL            is1;                // Flag to determine is the 1st shot
//BOOL            is1LoadFromHive;    // Flag to determine are shots load from hive files
//BOOL            is2LoadFromHive;    // Flag to determine are shots load from hive files
RECT            rect;               // Window RECT
FILETIME        ftLastWrite;        // Filetime struct
BROWSEINFO      BrowseInfo1;        // BrowseINFO struct
BOOL            bUseLongRegHead;    // 1.8.1 for compatibility with 1.61e5 and undoreg1.46
HANDLE          hHeap;              // 1.8.2

VOID    LogToMem(DWORD actiontype, LPDWORD lpcount, LPVOID lp);
BOOL    LoadSettingsFromIni(HWND hDlg);
BOOL    SaveSettingsToIni(HWND hDlg);
BOOL    IsInSkipList(LPTSTR lpStr, LPTSTR *lpSkipList);
VOID    UpdateCounters(LPTSTR lpszTitle1, LPTSTR lpszTitle2, DWORD nCount1, DWORD nCount2);
LPTSTR  FindKeyInIniSection(LPTSTR lpgrszSection, LPTSTR lpszSearch, size_t cchSectionLen, size_t cchSearchLen);
VOID    SetTextsToDefaultLanguage(VOID);
VOID    LoadAvailableLanguagesFromIni(HWND hDlg);
BOOL    GetSelectedLanguage(HWND hDlg);
VOID    SetTextsToSelectedLanguage(HWND hDlg);
VOID    CreateShotPopupMenu(VOID);
VOID    UI_BeforeShot(DWORD nID);
VOID    UI_AfterShot(VOID);
VOID    UI_BeforeClear(VOID);
VOID    UI_AfterClear(VOID);

VOID    Shot(LPREGSHOT lpShot);
BOOL    CompareShots(LPREGSHOT lpShot1, LPREGSHOT lpShot2);
VOID    SaveHive(LPREGSHOT lpShot);
BOOL    LoadHive(LPREGSHOT lpShot);
VOID    FreeAllCompareResults(void);
VOID    FreeShot(LPREGSHOT lpShot);
VOID    FreeAllFileHead(LPHEADFILE lpHeadFile);
VOID    ClearKeyMatchTag(LPKEYCONTENT lpKC);
VOID    GetRegistrySnap(HKEY hRegKey, LPKEYCONTENT lpFatherKC);  // HWND hDlg, first para deleted in 1.8, return from void * to void
VOID    GetFilesSnap(LPFILECONTENT lpFatherFC);                  // HWND hDlg, first para deleted in 1.8
LPSTR   GetWholeFileName(LPFILECONTENT lpFileContent);
VOID    InitProgressBar(VOID);
VOID    CompareFirstSubFile(LPFILECONTENT lpHead1, LPFILECONTENT lpHead2);
BOOL    ReplaceInValidFileName(LPSTR lpf);
VOID    ErrMsg(LPCSTR note);
VOID    WriteHead(LPTSTR lpstr, DWORD count, BOOL isHTML);
VOID    WritePart(LPCOMRESULT lpcomhead, BOOL isHTML, BOOL usecolor);
VOID    WriteTitle(LPSTR lph, LPSTR lpb, BOOL isHTML);
VOID    ClearHeadFileMatchTag(LPHEADFILE lpHF);
VOID    FindDirChain(LPHEADFILE lpHF, LPSTR lpDir, size_t nMaxLen);
BOOL    DirChainMatch(LPHEADFILE lphf1, LPHEADFILE lphf2);
VOID    WriteHtmlbegin(void);
VOID    WriteHtmlover(void);
VOID    WriteHtmlbr(void);
VOID    GetAllSubFile(BOOL needbrother, DWORD typedir, DWORD typefile, LPDWORD lpcountdir, LPDWORD lpcountfile, LPFILECONTENT lpFileContent);
LPFILECONTENT SearchDirChain(LPSTR lpname, LPHEADFILE lpHF);

#define REGSHOT_BUFFER_BLOCK_BYTES 1024

// List of all language strings
// NEVER CHANGE THE ORDER, LEAVE OLD ENTRIES UNTOUCHED
enum eLangTexts {
    iszTextKey = 0,
    iszTextValue,
    iszTextDir,
    iszTextFile,
    iszTextTime,
    iszTextKeyAdd,
    iszTextKeyDel,
    iszTextValAdd,
    iszTextValDel,
    iszTextValModi,
    iszTextFileAdd,
    iszTextFileDel,
    iszTextFileModi,
    iszTextDirAdd,
    iszTextDirDel,
    iszTextDirModi,
    iszTextTotal,
    iszTextComments,
    iszTextDateTime,
    iszTextComputer,
    iszTextUsername,
    iszTextAbout,
    iszTextError,
    iszTextErrorExecViewer,
    iszTextErrorCreateFile,
    iszTextErrorOpenFile,
    iszTextErrorMoveFP,
    //
    iszTextButtonShot1,
    iszTextButtonShot2,
    iszTextButtonCompare,
    iszTextButtonClear,
    iszTextButtonQuit,
    iszTextButtonAbout,
    iszTextTextMonitor,
    iszTextTextCompare,
    iszTextTextOutput,
    iszTextTextComment,
    iszTextRadioPlain,
    iszTextRadioHTML,
    iszTextTextScan,
    //
    iszTextMenuShot,
    iszTextMenuShotSave,
    iszTextMenuShotLoad,
    iszTextMenuClearAllShots,
    iszTextMenuClearShot1,
    iszTextMenuClearShot2,
    // ATTENTION: add new language strings before this line, the last line is used to determine the count
    cLangStrings
};

struct _LANGUAGETEXT {
    LPTSTR lpString;
    int nIDDlgItem;
};
typedef struct _LANGUAGETEXT LANGUAGETEXT, FAR *LPLANGUAGETEXT;

extern LANGUAGETEXT asLangTexts[];

extern FILEHEADER fileheader;
extern LPBYTE lpFileBuffer;
extern LPTSTR lpStringBuffer;
extern size_t nStringBufferSize;
extern size_t nSourceSize;

extern char LOCALMACHINESTRING[];
extern char LOCALMACHINESTRING_LONG[];
extern char USERSSTRING[];
extern char USERSSTRING_LONG[];

#ifndef _UNICODE
extern TCHAR szEmpty[];
#endif

size_t AdjustBuffer(PVOID *lpBuffer, size_t nCurrentSize, size_t nWantedSize, size_t nAlign);
VOID SaveHeadFile(LPHEADFILE lpHF, DWORD nFPCaller);
VOID LoadHeadFile(DWORD ofsHeadFile, LPHEADFILE *lplpCaller);

#endif // REGSHOT_GLOBAL_H
