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
#include "stddef.h"  // for "offsetof" macro

// ISDIR, ISFILE added in 1.8.0
#define ISDIR(x)  ( (x&FILE_ATTRIBUTE_DIRECTORY) != 0 )
#define ISFILE(x) ( (x&FILE_ATTRIBUTE_DIRECTORY) == 0 )

SAVEFILECONTENT sFC;

extern LPBYTE lan_dir;
extern LPBYTE lan_file;


//-------------------------------------------------------------
// Routine to get Whole File Name[root dir] from a FILECONTENT
//-------------------------------------------------------------
LPSTR GetWholeFileName(LPFILECONTENT lpFileContent)
{
    LPFILECONTENT lpf;
    LPSTR   lpName;
    LPSTR   lptail;
    size_t  nLen = 0;

    for (lpf = lpFileContent; lpf != NULL; lpf = lpf->lpfatherfile) {
        nLen += strlen(lpf->lpfilename) + 1;
    }
    if (nLen == 0) {
        nLen++;
    }
    lpName = MYALLOC(nLen);

    lptail = lpName + nLen - 1;
    *lptail = 0x00;

    for (lpf = lpFileContent; lpf != NULL; lpf = lpf->lpfatherfile) {
        nLen = strlen(lpf->lpfilename);
        memcpy(lptail -= nLen, lpf->lpfilename, nLen);
        if (lptail > lpName) {
            *--lptail = '\\';    // 0x5c;
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
    LPFILECONTENT lpFileContent
)
{
    //LPSTR   lpTemp;

    if (ISDIR(lpFileContent->fileattr)) {
        //lpTemp = lpFileContent->lpfilename;
        if (strcmp(lpFileContent->lpfilename, ".") != 0  && strcmp(lpFileContent->lpfilename, "..") != 0)  { // tfx   added in 1.7.3 fixed at 1.8.0 we should add here 1.8.0
            //if (*(unsigned short *)lpTemp != 0x002E && !(*(unsigned short *)lpTemp == 0x2E2E && *(lpTemp + 2) == 0x00)) {     // 1.8.2
            LogToMem(typedir, lpcountdir, lpFileContent);
        }
    } else {
        LogToMem(typefile, lpcountfile, lpFileContent);
    }

    if (lpFileContent->lpfirstsubfile != NULL)    {
        GetAllSubFile(TRUE, typedir, typefile, lpcountdir, lpcountfile, lpFileContent->lpfirstsubfile);
    }
    if (needbrother == TRUE) {
        if (lpFileContent->lpbrotherfile != NULL) {
            GetAllSubFile(TRUE, typedir, typefile, lpcountdir, lpcountfile, lpFileContent->lpbrotherfile);
        }
    }


}


//------------------------------------------------------------
// File Shot Engine
//------------------------------------------------------------
VOID GetFilesSnap(LPFILECONTENT lpFatherFile)
{
    LPSTR   lpFilename;
    LPSTR   lpTemp;
    HANDLE  filehandle;
    WIN32_FIND_DATA finddata;
    LPFILECONTENT   lpFileContent;
    LPFILECONTENT   lpFileContentTemp;

    lpTemp = GetWholeFileName(lpFatherFile);
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

    lpFileContent = MYALLOC0(sizeof(FILECONTENT));
    lpFileContent->lpfilename = MYALLOC0(strlen(finddata.cFileName) + 1);   // must add one!
    strcpy(lpFileContent->lpfilename, finddata.cFileName);
    lpFileContent->writetimelow = finddata.ftLastWriteTime.dwLowDateTime;
    lpFileContent->writetimehigh = finddata.ftLastWriteTime.dwHighDateTime;
    lpFileContent->filesizelow = finddata.nFileSizeLow;
    lpFileContent->filesizehigh = finddata.nFileSizeHigh;
    lpFileContent->fileattr = finddata.dwFileAttributes;
    lpFileContent->lpfatherfile = lpFatherFile;
    lpFatherFile->lpfirstsubfile = lpFileContent;
    lpFileContentTemp = lpFileContent;

    if (ISDIR(lpFileContent->fileattr)) {
        if (strcmp(lpFileContent->lpfilename, ".") != 0 && strcmp(lpFileContent->lpfilename, "..") != 0
                && !IsInSkipList(lpFileContent->lpfilename, lplpFileSkipStrings)) { // tfx

            nGettingDir++;
            GetFilesSnap(lpFileContent);
        }
    } else {
        nGettingFile++;
    }


    for (; FindNextFile(filehandle, &finddata) != FALSE;) {
        lpFileContent = MYALLOC0(sizeof(FILECONTENT));
        lpFileContent->lpfilename = MYALLOC0(strlen(finddata.cFileName) + 1);
        strcpy(lpFileContent->lpfilename, finddata.cFileName);
        lpFileContent->writetimelow = finddata.ftLastWriteTime.dwLowDateTime;
        lpFileContent->writetimehigh = finddata.ftLastWriteTime.dwHighDateTime;
        lpFileContent->filesizelow = finddata.nFileSizeLow;
        lpFileContent->filesizehigh = finddata.nFileSizeHigh;
        lpFileContent->fileattr = finddata.dwFileAttributes;
        lpFileContent->lpfatherfile = lpFatherFile;
        lpFileContentTemp->lpbrotherfile = lpFileContent;
        lpFileContentTemp = lpFileContent;

        if (ISDIR(lpFileContent->fileattr)) {
            if (strcmp(lpFileContent->lpfilename, ".") != 0 && strcmp(lpFileContent->lpfilename, "..") != 0
                    && !IsInSkipList(lpFileContent->lpfilename, lplpFileSkipStrings)) { // tfx
                nGettingDir++;
                GetFilesSnap(lpFileContent);
            }
        } else {
            nGettingFile++;
        }

    }
    FindClose(filehandle);

    nGettingTime = GetTickCount();
    if ((nGettingTime - nBASETIME1) > REFRESHINTERVAL) {
        UpdateCounters(lan_dir, lan_file, nGettingDir, nGettingFile);
    }

    return ;
}


//-------------------------------------------------------------
// File comparison engine (lp1 and lp2 run parallel)
//-------------------------------------------------------------
VOID CompareFirstSubFile(LPFILECONTENT lpHead1, LPFILECONTENT lpHead2)
{
    LPFILECONTENT   lp1;
    LPFILECONTENT   lp2;

    for (lp1 = lpHead1; lp1 != NULL; lp1 = lp1->lpbrotherfile) {
        for (lp2 = lpHead2; lp2 != NULL; lp2 = lp2->lpbrotherfile) {
            if ((lp2->bfilematch == NOTMATCH) && strcmp(lp1->lpfilename, lp2->lpfilename) == 0) { // 1.8.2 from lstrcmp to strcmp
                // Two files have the same name, but we are not sure they are the same, so we compare them!
                if (ISFILE(lp1->fileattr) && ISFILE(lp2->fileattr))
                    //(lp1->fileattr&FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY && (lp2->fileattr&FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
                {
                    // Lp1 is file, lp2 is file
                    if (lp1->writetimelow == lp2->writetimelow && lp1->writetimehigh == lp2->writetimehigh &&
                            lp1->filesizelow == lp2->filesizelow && lp1->filesizehigh == lp2->filesizehigh && lp1->fileattr == lp2->fileattr) {
                        // We found a match file!
                        lp2->bfilematch = ISMATCH;
                    } else {
                        // We found a dismatch file, they will be logged
                        lp2->bfilematch = ISMODI;
                        LogToMem(FILEMODI, &nFILEMODI, lp1);
                    }

                } else {
                    // At least one file of the pair is directory, so we try to determine
                    if (ISDIR(lp1->fileattr) && ISDIR(lp2->fileattr))
                        // (lp1->fileattr&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY && (lp2->fileattr&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                    {
                        // The two 'FILE's are all dirs
                        if (lp1->fileattr == lp2->fileattr) {
                            // Same dir attributes, we compare their subfiles
                            lp2->bfilematch = ISMATCH;
                            CompareFirstSubFile(lp1->lpfirstsubfile, lp2->lpfirstsubfile);
                        } else {
                            // Dir attributes changed, they will be logged
                            lp2->bfilematch = ISMODI;
                            LogToMem(DIRMODI, &nDIRMODI, lp1);
                        }
                        //break;
                    } else {
                        // One of the 'FILE's is dir, but which one?
                        if (ISFILE(lp1->fileattr) && ISDIR(lp2->fileattr))
                            //(lp1->fileattr&FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY && (lp2->fileattr&FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
                        {
                            // lp1 is file, lp2 is dir
                            lp1->bfilematch = ISDEL;
                            LogToMem(FILEDEL, &nFILEDEL, lp1);
                            GetAllSubFile(FALSE, DIRADD, FILEADD, &nDIRADD, &nFILEADD, lp2);
                        } else {
                            // lp1 is dir, lp2 is file
                            lp2->bfilematch = ISADD;
                            LogToMem(FILEADD, &nFILEADD, lp2);
                            GetAllSubFile(FALSE, DIRDEL, FILEDEL, &nDIRDEL, &nFILEDEL, lp1);
                        }
                    }
                }
                break;
            }
        }

        if (lp2 == NULL) {
            // lp2 looped to the end, that is, we can not find a lp2 matches lp1, so lp1 is deleted!
            if (ISDIR(lp1->fileattr)) {
                GetAllSubFile(FALSE, DIRDEL, FILEDEL, &nDIRDEL, &nFILEDEL, lp1); // if lp1 is dir
            } else {
                LogToMem(FILEDEL, &nFILEDEL, lp1);  // if lp1 is file
            }
        }
    }

    // We loop to the end, then we do an extra loop of lp2 use flag we previous made
    for (lp2 = lpHead2; lp2 != NULL; lp2 = lp2->lpbrotherfile) {
        nComparing++;
        if (lp2->bfilematch == NOTMATCH) {
            // We did not find a lp1 matches a lp2, so lp2 is added!
            if (ISDIR(lp2->fileattr)) {
                GetAllSubFile(FALSE, DIRADD, FILEADD, &nDIRADD, &nFILEADD, lp2);
            } else {
                LogToMem(FILEADD, &nFILEADD, lp2);
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


//-------------------------------------------------------------
// Routines to free all file tree
//-------------------------------------------------------------
VOID FreeAllFileContent(LPFILECONTENT lpFile)
{
    if (lpFile != NULL) {
        FreeAllFileContent(lpFile->lpfirstsubfile);
        FreeAllFileContent(lpFile->lpbrotherfile);
        MYFREE(lpFile->lpfilename);
        MYFREE(lpFile);
    }
}


//-------------------------------------------------------------
// Routines to free head files
//-------------------------------------------------------------
VOID FreeAllFileHead(LPHEADFILE lp)
{
    if (lp != NULL) {
        FreeAllFileHead(lp->lpnextheadfile);
        FreeAllFileContent(lp->lpfilecontent);
        //FreeAllFileContent(lp->lpfilecontent2);
        MYFREE(lp);
    }

}


//--------------------------------------------------
// Save File Information to a File
//
// This routine is called recursively to store the entries of the file/dir tree
// Therefore temporary vars are put in a local block to reduce stack usage
//--------------------------------------------------
VOID SaveFileContent(LPFILECONTENT lpFileContent, DWORD nFPCurrentFatherFile, DWORD nFPCaller)
{
    DWORD nFPHeader;

    // Get current file position
    // put in a separate var for later use
    nFPHeader = SetFilePointer(hFileWholeReg, 0, NULL, FILE_CURRENT);

    // Filename will always be stored behind the structure, so its position is already known
    sFC.fpos_filename = nFPHeader + sizeof(SAVEFILECONTENT);

    // Set file positions of the relatives inside the tree
    sFC.fpos_firstsubfile = 0;  // not known yet, may be re-written by another recursive call
    sFC.fpos_brotherfile = 0;   // not known yet, may be re-written by another recursive call
    sFC.fpos_fatherfile = nFPCurrentFatherFile;

    // Copy the rest
    sFC.writetimelow = lpFileContent->writetimelow;
    sFC.writetimehigh = lpFileContent->writetimehigh;
    sFC.filesizelow = lpFileContent->filesizelow;
    sFC.filesizehigh = lpFileContent->filesizehigh;
    sFC.fileattr = lpFileContent->fileattr;
    sFC.chksum = lpFileContent->chksum;

    // new since file content version 2
    sFC.version = 2;

    // Get filename length in bytes
    {
        size_t nLen;

        nLen = strlen(lpFileContent->lpfilename);
        nLen += 1;  // add trailing \0
        nLen *= sizeof(lpFileContent->lpfilename[0]);  // multiply with byte size of a char

        // TODO: Assert when nLen exceeding size of DWORD

        sFC.len_filename = (DWORD)nLen;
    }

    // Write file information to file
    // Make sure that ALL fields have been initialized/set, otherwise you will write data from the previous call
    WriteFile(hFileWholeReg, &sFC, sizeof(sFC), &NBW, NULL);

    // Write file name to file
    WriteFile(hFileWholeReg, lpFileContent->lpfilename, sFC.len_filename, &NBW, NULL);

    // Pad to DWORD
    {
        DWORD nPad;

        nPad = (sFC.len_filename % sizeof(DWORD) == 0) ? 0 : (sizeof(DWORD) - sFC.len_filename % sizeof(DWORD));

        if (nPad > 0) {
            DWORD nFPTemp4Write;

            nFPTemp4Write = 0;
            WriteFile(hFileWholeReg, &nFPTemp4Write, nPad, &NBW, NULL);
        }
    }

    // ATTENTION!!! sFC is INVALID from this point on, due to recursive calls

    // Store position of current entry in caller's field
    if (nFPCaller > 0) {
        DWORD nFPCurrent;

        nFPCurrent = SetFilePointer(hFileWholeReg, 0, NULL, FILE_CURRENT);

        SetFilePointer(hFileWholeReg, nFPCaller, NULL, FILE_BEGIN);
        WriteFile(hFileWholeReg, &nFPHeader, sizeof(nFPHeader), &NBW, NULL);

        SetFilePointer(hFileWholeReg, nFPCurrent, NULL, FILE_BEGIN);
    }

    // If the entry has childs, then do a recursive call for the first child
    // Pass this entry as father and "fpos_firstsubfile" position for storing the first child's position
    if (lpFileContent->lpfirstsubfile != NULL) {
        SaveFileContent(lpFileContent->lpfirstsubfile, nFPHeader, nFPHeader + offsetof(SAVEFILECONTENT, fpos_firstsubfile));
    }

    // If the entry has a following brother, then do a recursive call for the following brother
    // Pass father as father and "fpos_brotherfile" position for storing the next brother's position
    if (lpFileContent->lpbrotherfile != NULL) {
        SaveFileContent(lpFileContent->lpbrotherfile, nFPCurrentFatherFile, nFPHeader + offsetof(SAVEFILECONTENT, fpos_brotherfile));
    }

    // Need adjust progress bar para!!
    nSavingFile++;

    if (nGettingFile != 0) {
        if (nSavingFile % nGettingFile > nFileStep) {
            nSavingFile = 0;
            SendDlgItemMessage(hWnd, IDC_PBCOMPARE, PBM_STEPIT, (WPARAM)0, (LPARAM)0);
            UpdateWindow(hWnd);
            PeekMessage(&msg, hWnd, WM_ACTIVATE, WM_ACTIVATE, PM_REMOVE);
        }
    }
}


#ifdef _WIN64
//-------------------------------------------------------------
// Rebuild file snap from file buffer
//-------------------------------------------------------------
VOID RebuildFromHive_file(LPSAVEFILECONTENT lpFile, LPFILECONTENT lpFatherFC, LPFILECONTENT lpFC, LPBYTE lpHiveFileBase)
{
    LPFILECONTENT lpsubfile;

    if (lpFile->fpos_filename != 0) {
        lpFC->lpfilename = (LPSTR)(lpHiveFileBase + lpFile->fpos_filename);
    }
    lpFC->writetimelow = lpFile->writetimelow;
    lpFC->writetimehigh = lpFile->writetimehigh;
    lpFC->filesizelow = lpFile->filesizelow;
    lpFC->filesizehigh = lpFile->filesizehigh;
    lpFC->fileattr = lpFile->fileattr;
    lpFC->chksum = lpFile->chksum;
    lpFC->lpfatherfile = lpFatherFC;
    if (ISDIR(lpFC->fileattr)) {
        nGettingDir++;
    } else {
        nGettingFile++;
    }

    if (lpFile->fpos_firstsubfile != 0) {
        lpsubfile = MYALLOC0(sizeof(FILECONTENT));
        lpFC->lpfirstsubfile = lpsubfile;
        RebuildFromHive_file((LPSAVEFILECONTENT)(lpHiveFileBase + lpFile->fpos_firstsubfile), lpFC, lpsubfile, lpHiveFileBase);
    }

    if (lpFile->fpos_brotherfile != 0) {
        lpsubfile = MYALLOC0(sizeof(FILECONTENT));
        lpFC->lpbrotherfile = lpsubfile;
        RebuildFromHive_file((LPSAVEFILECONTENT)(lpHiveFileBase + lpFile->fpos_brotherfile), lpFatherFC, lpsubfile, lpHiveFileBase);
    }


    nGettingTime = GetTickCount();
    if ((nGettingTime - nBASETIME1) > REFRESHINTERVAL) {
        UpdateCounters(lan_dir, lan_file, nGettingDir, nGettingFile);
    }

}
//-------------------------------------------------------------
// Rebuild filehead from file buffer
//-------------------------------------------------------------
VOID RebuildFromHive_filehead(LPSAVEHEADFILE lpSHF, LPHEADFILE lpHeadFile, LPBYTE lpHiveFileBase)
{
    LPSAVEHEADFILE  lpshf;
    LPHEADFILE lpHF;
    LPHEADFILE lpHFLast;

    for (lpshf = lpSHF, lpHF = lpHeadFile; lpHiveFileBase != (LPBYTE)lpshf; lpshf = (LPSAVEHEADFILE)(lpHiveFileBase + lpshf->fpos_nextheadfile)) {

        if (lpshf->fpos_filecontent != 0) {
            lpHF->lpfilecontent = MYALLOC0(sizeof(FILECONTENT));
            RebuildFromHive_file((LPSAVEFILECONTENT)(lpHiveFileBase + lpshf->fpos_filecontent), NULL, lpHF->lpfilecontent, lpHiveFileBase);
        }
        if (lpshf->fpos_nextheadfile != 0) {
            lpHF->lpnextheadfile = MYALLOC0(sizeof(HEADFILE));
            lpHFLast = lpHF;
            lpHF = lpHF->lpnextheadfile;
        }

    }
}

#else
//--------------------------------------------------
// Realign filecontent, called by ReAlignFile()
//--------------------------------------------------
VOID ReAlignFileContent(LPFILECONTENT lpFC, size_t nBase)
{

    if (lpFC->lpfilename != NULL) {
        lpFC->lpfilename += nBase;
    }
    if (lpFC->lpfirstsubfile != NULL) {
        lpFC->lpfirstsubfile = (LPFILECONTENT)((LPBYTE)lpFC->lpfirstsubfile + nBase);
    }
    if (lpFC->lpbrotherfile != NULL) {
        lpFC->lpbrotherfile = (LPFILECONTENT)((LPBYTE)lpFC->lpbrotherfile + nBase);
    }
    if (lpFC->lpfatherfile != NULL) {
        lpFC->lpfatherfile = (LPFILECONTENT)((LPBYTE)lpFC->lpfatherfile + nBase);
    }


    nGettingFile++; // just for the progress bar

    if (lpFC->lpfirstsubfile != NULL) {
        ReAlignFileContent(lpFC->lpfirstsubfile, nBase);
    }

    if (lpFC->lpbrotherfile != NULL) {
        ReAlignFileContent(lpFC->lpbrotherfile, nBase);
    }

}


//--------------------------------------------------
// Realign file, walk through chain
//--------------------------------------------------
VOID ReAlignFile(LPHEADFILE lpHF, size_t nBase)
{
    LPHEADFILE  lphf;

    for (lphf = lpHF; lphf != NULL; lphf = lphf->lpnextheadfile) {


        if (lphf->lpnextheadfile != NULL) {
            lphf->lpnextheadfile = (LPHEADFILE)((LPBYTE)lphf->lpnextheadfile + nBase);
        }
        if (lphf->lpfilecontent != NULL) {
            lphf->lpfilecontent = (LPFILECONTENT)((LPBYTE)lphf->lpfilecontent + nBase);
            ReAlignFileContent(lphf->lpfilecontent, nBase);
        }
    }
}

#endif

//--------------------------------------------------
// Walkthrough lpHF and find lpname matches
//--------------------------------------------------
LPFILECONTENT SearchDirChain(LPSTR lpname, LPHEADFILE lpHF)
{
    LPHEADFILE lphf;

    if (lpname != NULL)
        for (lphf = lpHF; lphf != NULL; lphf = lphf->lpnextheadfile) {
            if (lphf->lpfilecontent != NULL && lphf->lpfilecontent->lpfilename != NULL)
                if (_stricmp(lpname, lphf->lpfilecontent->lpfilename) == 0) {
                    return lphf->lpfilecontent;
                }
        }
    return NULL;
}


//--------------------------------------------------
// Walkthrough lpheadfile chain and collect it's first dirname to lpDir
//--------------------------------------------------
VOID FindDirChain(LPHEADFILE lpHF, LPSTR lpDir, size_t nMaxLen)
{
    size_t      nLen;
    LPHEADFILE  lphf;
    *lpDir = 0x00;
    nLen   = 0;

    for (lphf = lpHF; lphf != NULL; lphf = lphf->lpnextheadfile) {
        if (lphf->lpfilecontent != NULL && nLen < nMaxLen) {
            nLen += strlen(lphf->lpfilecontent->lpfilename) + 1;
            strcat(lpDir, lphf->lpfilecontent->lpfilename);
            strcat(lpDir, ";");
        }
    }
}


//--------------------------------------------------
// if two dir chains are the same
//--------------------------------------------------
BOOL DirChainMatch(LPHEADFILE lphf1, LPHEADFILE lphf2)
{
    char lpDir1[EXTDIRLEN + 4];
    char lpDir2[EXTDIRLEN + 4];
    ZeroMemory(lpDir1, sizeof(lpDir1));
    ZeroMemory(lpDir2, sizeof(lpDir2));
    FindDirChain(lphf1, lpDir1, EXTDIRLEN);
    FindDirChain(lphf2, lpDir2, EXTDIRLEN);

    if (_stricmp(lpDir1, lpDir2) != 0) {
        return FALSE;
    } else {
        return TRUE;
    }
}
