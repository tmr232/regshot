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

#include "global.h"

// ISDIR, ISFILE added in 1.8.0
#define ISDIR(x)  ( (x&FILE_ATTRIBUTE_DIRECTORY) != 0 )
#define ISFILE(x) ( (x&FILE_ATTRIBUTE_DIRECTORY) == 0 )

SAVEFILECONTENT sFC;
SAVEHEADFILE sHF;

// Some DWORDs used to show the progress bar and etc
DWORD nGettingFile;
DWORD nGettingDir;
DWORD nSavingFile;


//-------------------------------------------------------------
// Routine to get Whole File Name[root dir] from a FILECONTENT
//-------------------------------------------------------------
LPSTR GetWholeFileName(LPFILECONTENT lpStartFC)
{
    LPFILECONTENT lpFC;
    LPSTR   lpName;  // TODO: LPTSTR
    LPSTR   lpTail;  // TODO: LPTSTR
    size_t  nLen;

    nLen = 0;
    for (lpFC = lpStartFC; NULL != lpFC; lpFC = lpFC->lpFatherFC) {
        if (NULL != lpFC->lpFileName) {
            nLen += strlen(lpFC->lpFileName) + 1;  // +1 char for backslash or NULL char  // TODO: _tcslen
        }
    }
    if (0 == nLen) {  // at least create an empty string with NULL char
        nLen++;
    }
    lpName = MYALLOC(nLen * sizeof(TCHAR));

    lpTail = lpName + nLen - 1;
    *lpTail = 0;

    for (lpFC = lpStartFC; NULL != lpFC; lpFC = lpFC->lpFatherFC) {
        if (NULL != lpFC->lpFileName) {
            nLen = strlen(lpFC->lpFileName);
            memcpy(lpTail -= nLen, lpFC->lpFileName, nLen);  // TODO: _tcsncpy
            if (lpTail > lpName) {
                *--lpTail = '\\';    // 0x5c;  // TODO: check if works for Unicode
            }
        }
    }

    return lpName;
}


//-------------------------------------------------------------
// Routine to walk through all sub tree of current directory [File system]
//-------------------------------------------------------------
VOID GetAllSubFile(
    BOOL    needbrother,
    DWORD   typedir,
    DWORD   typefile,
    LPDWORD lpcountdir,
    LPDWORD lpcountfile,
    LPFILECONTENT lpFC
)
{
    //LPSTR   lpTemp;

    if (ISDIR(lpFC->fileattr)) {
        //lpTemp = lpFC->lpFileName;
        if ((NULL != lpFC->lpFileName) && (0 != strcmp(lpFC->lpFileName, ".")) && (0 != strcmp(lpFC->lpFileName, "..")))  { // tfx   added in 1.7.3 fixed at 1.8.0 we should add here 1.8.0
            //if (*(unsigned short *)lpTemp != 0x002E && !(*(unsigned short *)lpTemp == 0x2E2E && *(lpTemp + 2) == 0x00)) {     // 1.8.2
            LogToMem(typedir, lpcountdir, lpFC);
        }
    } else {
        LogToMem(typefile, lpcountfile, lpFC);
    }

    if (NULL != lpFC->lpFirstSubFC)    {
        GetAllSubFile(TRUE, typedir, typefile, lpcountdir, lpcountfile, lpFC->lpFirstSubFC);
    }
    if (TRUE == needbrother) {
        if (NULL != lpFC->lpBrotherFC) {
            GetAllSubFile(TRUE, typedir, typefile, lpcountdir, lpcountfile, lpFC->lpBrotherFC);
        }
    }
}


//------------------------------------------------------------
// File Shot Engine
//------------------------------------------------------------
VOID GetFilesSnap(LPFILECONTENT lpFatherFC)
{
    LPSTR   lpFilename;
    LPSTR   lpTemp;
    HANDLE  filehandle;
    WIN32_FIND_DATA finddata;
    LPFILECONTENT   lpFC;
    LPFILECONTENT   lpFCTemp;

    lpTemp = GetWholeFileName(lpFatherFC);
    //Not used
    //if (bWinNTDetected)
    //{
    //  lpFilename = MYALLOC(strlen(lpTemp) + 5 + 4);
    //  strcpy(lpFilename,"\\\\?\\");
    //  strcat(lpFilename,lpTemp);
    //}
    //else
    {
        lpFilename = MYALLOC(strlen(lpTemp) + 5);
        strcpy(lpFilename, lpTemp);
    }
    strcat(lpFilename, "\\*.*");

    MYFREE(lpTemp);
    //_asm int 3;
    filehandle = FindFirstFile(lpFilename, &finddata);
    MYFREE(lpFilename);
    if (filehandle == INVALID_HANDLE_VALUE) {
        return;
    }

    //lpTemp = finddata.cFileName; // 1.8

    lpFC = MYALLOC0(sizeof(FILECONTENT));
    lpFC->lpFileName = MYALLOC0(strlen(finddata.cFileName) + 1);   // must add one!
    strcpy(lpFC->lpFileName, finddata.cFileName);
    lpFC->writetimelow = finddata.ftLastWriteTime.dwLowDateTime;
    lpFC->writetimehigh = finddata.ftLastWriteTime.dwHighDateTime;
    lpFC->filesizelow = finddata.nFileSizeLow;
    lpFC->filesizehigh = finddata.nFileSizeHigh;
    lpFC->fileattr = finddata.dwFileAttributes;
    lpFC->lpFatherFC = lpFatherFC;
    lpFatherFC->lpFirstSubFC = lpFC;
    lpFCTemp = lpFC;

    if (ISDIR(lpFC->fileattr)) {
        if ((NULL != lpFC->lpFileName)
                && (0 != strcmp(lpFC->lpFileName, "."))
                && (0 != strcmp(lpFC->lpFileName, ".."))
                && !IsInSkipList(lpFC->lpFileName, lplpFileSkipStrings)) {  // tfx
            nGettingDir++;
            GetFilesSnap(lpFC);
        }
    } else {
        nGettingFile++;
    }

    for (; FindNextFile(filehandle, &finddata) != FALSE;) {
        lpFC = MYALLOC0(sizeof(FILECONTENT));
        lpFC->lpFileName = MYALLOC0(strlen(finddata.cFileName) + 1);
        strcpy(lpFC->lpFileName, finddata.cFileName);
        lpFC->writetimelow = finddata.ftLastWriteTime.dwLowDateTime;
        lpFC->writetimehigh = finddata.ftLastWriteTime.dwHighDateTime;
        lpFC->filesizelow = finddata.nFileSizeLow;
        lpFC->filesizehigh = finddata.nFileSizeHigh;
        lpFC->fileattr = finddata.dwFileAttributes;
        lpFC->lpFatherFC = lpFatherFC;
        lpFCTemp->lpBrotherFC = lpFC;
        lpFCTemp = lpFC;

        if (ISDIR(lpFC->fileattr)) {
            if ((NULL != lpFC->lpFileName)
                    && (0 != strcmp(lpFC->lpFileName, "."))
                    && (0 != strcmp(lpFC->lpFileName, ".."))
                    && !IsInSkipList(lpFC->lpFileName, lplpFileSkipStrings)) {  // tfx
                nGettingDir++;
                GetFilesSnap(lpFC);
            }
        } else {
            nGettingFile++;
        }

    }
    FindClose(filehandle);

    nGettingTime = GetTickCount();
    if ((nGettingTime - nBASETIME1) > REFRESHINTERVAL) {
        UpdateCounters(asLangTexts[iszTextDir].lpString, asLangTexts[iszTextFile].lpString, nGettingDir, nGettingFile);
    }

    return ;
}


//-------------------------------------------------------------
// File comparison engine (lp1 and lp2 run parallel)
//-------------------------------------------------------------
VOID CompareFirstSubFile(LPFILECONTENT lpFCHead1, LPFILECONTENT lpFCHead2)
{
    LPFILECONTENT lpFC1;
    LPFILECONTENT lpFC2;

    for (lpFC1 = lpFCHead1; lpFC1 != NULL; lpFC1 = lpFC1->lpBrotherFC) {
        for (lpFC2 = lpFCHead2; lpFC2 != NULL; lpFC2 = lpFC2->lpBrotherFC) {
            if ((lpFC2->bfilematch == NOTMATCH) && strcmp(lpFC1->lpFileName, lpFC2->lpFileName) == 0) { // 1.8.2 from lstrcmp to strcmp
                // Two files have the same name, but we are not sure they are the same, so we compare them!
                if (ISFILE(lpFC1->fileattr) && ISFILE(lpFC2->fileattr))
                    //(lpFC1->fileattr&FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY && (lpFC2->fileattr&FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
                {
                    // Lp1 is file, lpFC2 is file
                    if (lpFC1->writetimelow == lpFC2->writetimelow && lpFC1->writetimehigh == lpFC2->writetimehigh &&
                            lpFC1->filesizelow == lpFC2->filesizelow && lpFC1->filesizehigh == lpFC2->filesizehigh && lpFC1->fileattr == lpFC2->fileattr) {
                        // We found a match file!
                        lpFC2->bfilematch = ISMATCH;
                    } else {
                        // We found a dismatch file, they will be logged
                        lpFC2->bfilematch = ISMODI;
                        LogToMem(FILEMODI, &nFILEMODI, lpFC1);
                    }

                } else {
                    // At least one file of the pair is directory, so we try to determine
                    if (ISDIR(lpFC1->fileattr) && ISDIR(lpFC2->fileattr))
                        // (lpFC1->fileattr&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY && (lpFC2->fileattr&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                    {
                        // The two 'FILE's are all dirs
                        if (lpFC1->fileattr == lpFC2->fileattr) {
                            // Same dir attributes, we compare their subfiles
                            lpFC2->bfilematch = ISMATCH;
                            CompareFirstSubFile(lpFC1->lpFirstSubFC, lpFC2->lpFirstSubFC);
                        } else {
                            // Dir attributes changed, they will be logged
                            lpFC2->bfilematch = ISMODI;
                            LogToMem(DIRMODI, &nDIRMODI, lpFC1);
                        }
                        //break;
                    } else {
                        // One of the 'FILE's is dir, but which one?
                        if (ISFILE(lpFC1->fileattr) && ISDIR(lpFC2->fileattr))
                            //(lpFC1->fileattr&FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY && (lpFC2->fileattr&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                        {
                            // lpFC1 is file, lpFC2 is dir
                            lpFC1->bfilematch = ISDEL;
                            LogToMem(FILEDEL, &nFILEDEL, lpFC1);
                            GetAllSubFile(FALSE, DIRADD, FILEADD, &nDIRADD, &nFILEADD, lpFC2);
                        } else {
                            // lpFC1 is dir, lpFC2 is file
                            lpFC2->bfilematch = ISADD;
                            LogToMem(FILEADD, &nFILEADD, lpFC2);
                            GetAllSubFile(FALSE, DIRDEL, FILEDEL, &nDIRDEL, &nFILEDEL, lpFC1);
                        }
                    }
                }
                break;
            }
        }

        if (lpFC2 == NULL) {
            // lpFC2 looped to the end, that is, we can not find a lpFC2 matches lpFC1, so lpFC1 is deleted!
            if (ISDIR(lpFC1->fileattr)) {
                GetAllSubFile(FALSE, DIRDEL, FILEDEL, &nDIRDEL, &nFILEDEL, lpFC1); // if lpFC1 is dir
            } else {
                LogToMem(FILEDEL, &nFILEDEL, lpFC1);  // if lpFC1 is file
            }
        }
    }

    // We loop to the end, then we do an extra loop of lpFC2 use flag we previous made
    for (lpFC2 = lpFCHead2; lpFC2 != NULL; lpFC2 = lpFC2->lpBrotherFC) {
        nComparing++;
        if (lpFC2->bfilematch == NOTMATCH) {
            // We did not find a lpFC1 matches a lpFC2, so lpFC2 is added!
            if (ISDIR(lpFC2->fileattr)) {
                GetAllSubFile(FALSE, DIRADD, FILEADD, &nDIRADD, &nFILEADD, lpFC2);
            } else {
                LogToMem(FILEADD, &nFILEADD, lpFC2);
            }
        }
    }

    // Progress bar update
    if (nGettingFile != 0)
        if (nComparing % nGettingFile > nFileStep) {
            nComparing = 0;
            SendDlgItemMessage(hWnd, IDC_PBCOMPARE, PBM_STEPIT, (WPARAM)0, (LPARAM)0);
        }
}


// ----------------------------------------------------------------------
// Clear comparison match flags in all files
// ----------------------------------------------------------------------
VOID ClearFileContentMatchTag(LPFILECONTENT lpFC)
{
    if (NULL != lpFC) {
        lpFC->bfilematch = 0;
        ClearFileContentMatchTag(lpFC->lpFirstSubFC);
        ClearFileContentMatchTag(lpFC->lpBrotherFC);
    }
}

// ----------------------------------------------------------------------
// Clear comparison match flags in all head files
// ----------------------------------------------------------------------
VOID ClearHeadFileMatchTag(LPHEADFILE lpStartHF)
{
    LPHEADFILE lpHF;

    for (lpHF = lpStartHF; NULL != lpHF; lpHF = lpHF->lpBrotherHF) {
        ClearFileContentMatchTag(lpHF->lpFirstFC);
    }
}


// ----------------------------------------------------------------------
// Free all files
// ----------------------------------------------------------------------
VOID FreeAllFileContent(LPFILECONTENT lpFC)
{
    if (NULL != lpFC) {
        if (NULL != lpFC->lpFileName) {
            MYFREE(lpFC->lpFileName);
        }
        FreeAllFileContent(lpFC->lpFirstSubFC);
        FreeAllFileContent(lpFC->lpBrotherFC);
        MYFREE(lpFC);
    }
}

// ----------------------------------------------------------------------
// Free all head files
// ----------------------------------------------------------------------
VOID FreeAllFileHead(LPHEADFILE lpHF)
{
    if (NULL != lpHF) {
        FreeAllFileContent(lpHF->lpFirstFC);
        FreeAllFileHead(lpHF->lpBrotherHF);
        MYFREE(lpHF);
    }
}


// ----------------------------------------------------------------------
// Save file to HIVE File
//
// This routine is called recursively to store the entries of the file/dir tree
// Therefore temporary vars are put in a local block to reduce stack usage
// ----------------------------------------------------------------------
VOID SaveFileContent(LPFILECONTENT lpFC, DWORD nFPFatherFile, DWORD nFPCaller)
{
    DWORD nFPFile;

    // Get current file position
    // put in a separate var for later use
    nFPFile = SetFilePointer(hFileWholeReg, 0, NULL, FILE_CURRENT);

    // Write position of current file in caller's field
    if (0 < nFPCaller) {
        SetFilePointer(hFileWholeReg, nFPCaller, NULL, FILE_BEGIN);
        WriteFile(hFileWholeReg, &nFPFile, sizeof(nFPFile), &NBW, NULL);

        SetFilePointer(hFileWholeReg, nFPFile, NULL, FILE_BEGIN);
    }

    // Initialize file content
    ZeroMemory(&sFC, sizeof(sFC));

    // Copy values
    sFC.writetimelow = lpFC->writetimelow;
    sFC.writetimehigh = lpFC->writetimehigh;
    sFC.filesizelow = lpFC->filesizelow;
    sFC.filesizehigh = lpFC->filesizehigh;
    sFC.fileattr = lpFC->fileattr;
    sFC.chksum = lpFC->chksum;

    // Set file positions of the relatives inside the tree
    sFC.ofsFileName = 0;      // not known yet, may be re-written in this call
    sFC.ofsFirstSubFile = 0;  // not known yet, may be re-written by another recursive call
    sFC.ofsBrotherFile = 0;   // not known yet, may be re-written by another recursive call
    sFC.ofsFatherFile = nFPFatherFile;

    // New since file content version 2
    sFC.nFileNameLen = 0;
    if (NULL != lpFC->lpFileName) {
        sFC.nFileNameLen = (DWORD)_tcslen(lpFC->lpFileName);
#ifdef _UNICODE
        sFC.nFileNameLen++;  // account for NULL char
        // File name will always be stored behind the structure, so its position is already known
        sFC.ofsFileName = nFPFile + sizeof(sFC);
#endif
    }
#ifndef _UNICODE
    sFC.nFileNameLen++;  // account for NULL char
    // File name will always be stored behind the structure, so its position is already known
    sFC.ofsFileName = nFPFile + sizeof(sFC);
#endif

    // Write file content to file
    // Make sure that ALL fields have been initialized/set
    WriteFile(hFileWholeReg, &sFC, sizeof(sFC), &NBW, NULL);

    // Write file name to file
    if (NULL != lpFC->lpFileName) {
        WriteFile(hFileWholeReg, lpFC->lpFileName, sFC.nFileNameLen * sizeof(TCHAR), &NBW, NULL);
#ifndef _UNICODE
    } else {
        // Write empty string for backward compatibility
        WriteFile(hFileWholeReg, szEmpty, sFC.nFileNameLen * sizeof(TCHAR), &NBW, NULL);
#endif
    }

    // ATTENTION!!! sFC is INVALID from this point on, due to recursive calls

    // If the entry has childs, then do a recursive call for the first child
    // Pass this entry as father and "ofsFirstSubFile" position for storing the first child's position
    if (NULL != lpFC->lpFirstSubFC) {
        SaveFileContent(lpFC->lpFirstSubFC, nFPFile, nFPFile + offsetof(SAVEFILECONTENT, ofsFirstSubFile));
    }

    // If the entry has a following brother, then do a recursive call for the following brother
    // Pass father as father and "ofsBrotherFile" position for storing the next brother's position
    if (NULL != lpFC->lpBrotherFC) {
        SaveFileContent(lpFC->lpBrotherFC, nFPFatherFile, nFPFile + offsetof(SAVEFILECONTENT, ofsBrotherFile));
    }

    // TODO: Need to adjust progress bar para!!
    nSavingFile++;
    if (0 != nGettingFile) {
        if (nSavingFile % nGettingFile > nFileStep) {
            nSavingFile = 0;
            SendDlgItemMessage(hWnd, IDC_PBCOMPARE, PBM_STEPIT, (WPARAM)0, (LPARAM)0);
            UpdateWindow(hWnd);
            PeekMessage(&msg, hWnd, WM_ACTIVATE, WM_ACTIVATE, PM_REMOVE);
        }
    }
}

//--------------------------------------------------
// Save head file to HIVE file
//--------------------------------------------------
VOID SaveHeadFile(LPHEADFILE lpStartHF, DWORD nFPCaller)
{
    LPHEADFILE lpHF;
    DWORD nFPHFCaller;
    DWORD nFPHF;

    // Write all head files and their file contents
    nFPHFCaller = nFPCaller;
    for (lpHF = lpStartHF; NULL != lpHF; lpHF = lpHF->lpBrotherHF) {
        nFPHF = SetFilePointer(hFileWholeReg, 0, NULL, FILE_CURRENT);

        // Write position of current head file in caller's field
        if (0 < nFPHFCaller) {
            SetFilePointer(hFileWholeReg, nFPHFCaller, NULL, FILE_BEGIN);
            WriteFile(hFileWholeReg, &nFPHF, sizeof(nFPHF), &NBW, NULL);

            SetFilePointer(hFileWholeReg, nFPHF, NULL, FILE_BEGIN);
        }
        nFPHFCaller = nFPHF + offsetof(SAVEHEADFILE, ofsBrotherHeadFile);

        // Initialize head file
        ZeroMemory(&sHF, sizeof(sHF));

        // Write head file to file
        // Make sure that ALL fields have been initialized/set
        WriteFile(hFileWholeReg, &sHF, sizeof(sHF), &NBW, NULL);

        // Write all file contents of head file
        if (NULL != lpHF->lpFirstFC) {
            SaveFileContent(lpHF->lpFirstFC, 0, nFPHF + offsetof(SAVEHEADFILE, ofsFirstFileContent));
        }
    }
}


// ----------------------------------------------------------------------
// Load file from HIVE file
// ----------------------------------------------------------------------
VOID LoadFile(DWORD ofsFileContent, LPFILECONTENT lpFatherFC, LPFILECONTENT *lplpCaller)
{
    LPFILECONTENT lpFC;
    DWORD ofsFirstSubFile;
    DWORD ofsBrotherFile;

    // Copy SAVEFILECONTENT to aligned memory block
    ZeroMemory(&sFC, sizeof(sFC));
    CopyMemory(&sFC, (lpFileBuffer + ofsFileContent), fileheader.nFCSize);

    // Create new file content
    // put in a separate var for later use
    lpFC = MYALLOC0(sizeof(FILECONTENT));
    ZeroMemory(lpFC, sizeof(FILECONTENT));

    // Write pointer to current file into caller's pointer
    if (NULL != lplpCaller) {
        *lplpCaller = lpFC;
    }

    // Set father of current file
    lpFC->lpFatherFC = lpFatherFC;

    // Copy values
    lpFC->writetimelow = sFC.writetimelow;
    lpFC->writetimehigh = sFC.writetimehigh;
    lpFC->filesizelow = sFC.filesizelow;
    lpFC->filesizehigh = sFC.filesizehigh;
    lpFC->fileattr = sFC.fileattr;
    lpFC->chksum = sFC.chksum;

    // Copy file name
    if (FILECONTENT_VERSION_2 > fileheader.nFCVersion) {
        sFC.nFileNameLen = (DWORD)strlen((const char *)(lpFileBuffer + sFC.ofsFileName));
        if (0 < sFC.nFileNameLen) {
            sFC.nFileNameLen++;  // account for NULL char
        }
    }
    if (0 < sFC.nFileNameLen) {  // otherwise leave it NULL
        // Copy string to an aligned memory block
        nSourceSize = sFC.nFileNameLen * fileheader.nCharSize;
        nStringBufferSize = AdjustBuffer(&lpStringBuffer, nStringBufferSize, nSourceSize, REGSHOT_BUFFER_BLOCK_BYTES);
        ZeroMemory(lpStringBuffer, nStringBufferSize);
        CopyMemory(lpStringBuffer, (lpFileBuffer + sFC.ofsFileName), nSourceSize);

        lpFC->lpFileName = MYALLOC0(sFC.nFileNameLen * sizeof(TCHAR));
        if (sizeof(TCHAR) == fileheader.nCharSize) {
            _tcscpy(lpFC->lpFileName, lpStringBuffer);
        } else {
#ifdef _UNICODE
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpStringBuffer, -1, lpFC->lpFileName, sFC.nFileNameLen);
#else
            WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR, (LPCWSTR)lpStringBuffer, -1, lpFC->lpFileName, sFC.nFileNameLen, NULL, NULL);
#endif
        }
    }

    ofsFirstSubFile = sFC.ofsFirstSubFile;
    ofsBrotherFile = sFC.ofsBrotherFile;

    if (ISDIR(lpFC->fileattr)) {
        nGettingDir++;
    } else {
        nGettingFile++;
    }

    nGettingTime = GetTickCount();
    if ((nGettingTime - nBASETIME1) > REFRESHINTERVAL) {
        UpdateCounters(asLangTexts[iszTextDir].lpString, asLangTexts[iszTextFile].lpString, nGettingDir, nGettingFile);
    }

    // ATTENTION!!! sFC is INVALID from this point on, due to recursive calls

    // If the entry has childs, then do a recursive call for the first child
    // Pass this entry as father and "lpFirstSubFC" pointer for storing the first child's pointer
    if (0 != ofsFirstSubFile) {
        LoadFile(ofsFirstSubFile, lpFC, &lpFC->lpFirstSubFC);
    }

    // If the entry has a following brother, then do a recursive call for the following brother
    // Pass father as father and "lpBrotherFC" pointer for storing the next brother's pointer
    if (0 != ofsBrotherFile) {
        LoadFile(ofsBrotherFile, lpFatherFC, &lpFC->lpBrotherFC);
    }
}

//--------------------------------------------------
// Load head file from HIVE file
//--------------------------------------------------
VOID LoadHeadFile(DWORD ofsHeadFile, LPHEADFILE *lplpCaller)
{
    LPHEADFILE *lplpHFCaller;
    LPHEADFILE lpHF;

    // Read all head files and their file contents
    lplpHFCaller = lplpCaller;
    for (; 0 != ofsHeadFile; ofsHeadFile = sHF.ofsBrotherHeadFile) {
        // Copy SAVEHEADFILE to aligned memory block
        ZeroMemory(&sHF, sizeof(sHF));
        CopyMemory(&sHF, (lpFileBuffer + ofsHeadFile), fileheader.nHFSize);

        // Create new head file
        // put in a separate var for later use
        lpHF = MYALLOC0(sizeof(SAVEHEADFILE));
        ZeroMemory(lpHF, sizeof(SAVEHEADFILE));

        // Write pointer to current head file into caller's pointer
        if (NULL != lplpHFCaller) {
            *lplpHFCaller = lpHF;
        }
        lplpHFCaller = &lpHF->lpBrotherHF;

        // If the entry has file contents, then do a call for the first file content
        if (0 != sHF.ofsFirstFileContent) {
            LoadFile(sHF.ofsFirstFileContent, NULL, &lpHF->lpFirstFC);
        }
    }
}


//--------------------------------------------------
// Walkthrough lpHF and find lpname matches
//--------------------------------------------------
LPFILECONTENT SearchDirChain(LPSTR lpName, LPHEADFILE lpStartHF)
{
    LPHEADFILE lpHF;

    if (NULL != lpName) {
        for (lpHF = lpStartHF; NULL != lpHF; lpHF = lpHF->lpBrotherHF) {
            if ((NULL != lpHF->lpFirstFC)
                    && (NULL != lpHF->lpFirstFC->lpFileName)
                    && (0 == _stricmp(lpName, lpHF->lpFirstFC->lpFileName))) {
                return lpHF->lpFirstFC;
            }
        }
    }
    return NULL;
}

//--------------------------------------------------
// Walkthrough lpFILES chain and collect it's first dirname to lpDir
//--------------------------------------------------
VOID FindDirChain(LPHEADFILE lpStartHF, LPSTR lpDir, size_t nMaxLen)
{
    LPHEADFILE  lpHF;
    size_t      nLen;

    *lpDir = 0;
    nLen = 0;
    for (lpHF = lpStartHF; NULL != lpHF; lpHF = lpHF->lpBrotherHF) {
        if ((NULL != lpHF->lpFirstFC)
                && (NULL != lpHF->lpFirstFC->lpFileName)
                && (nLen < nMaxLen)) {
            nLen += strlen(lpHF->lpFirstFC->lpFileName) + 1;
            strcat(lpDir, lpHF->lpFirstFC->lpFileName);
            strcat(lpDir, TEXT(";"));
        }
    }
}


//--------------------------------------------------
// if two dir chains are the same
//--------------------------------------------------
BOOL DirChainMatch(LPHEADFILE lpHF1, LPHEADFILE lpHF2)
{
    char lpDir1[EXTDIRLEN + 4];
    char lpDir2[EXTDIRLEN + 4];

    ZeroMemory(lpDir1, sizeof(lpDir1));
    ZeroMemory(lpDir2, sizeof(lpDir2));
    FindDirChain(lpHF1, lpDir1, EXTDIRLEN);
    FindDirChain(lpHF2, lpDir2, EXTDIRLEN);

    if (0 != _stricmp(lpDir1, lpDir2)) {
        return FALSE;
    } else {
        return TRUE;
    }
}
