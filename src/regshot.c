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
#include "version.h"
#include "LMCons.h"  // for UNLEN define

char szDefResPre[] = REGSHOT_RESULT_FILE;

char szFilter[] = TEXT("Regshot hive files (*.hiv;*.hiv2)\0*.hiv;*.hiv2\0All files\0*.*\0\0");

char szRegshotFileSignatureSBCS[] = "REGSHOTHIVE";
char szRegshotFileSignatureUTF16[] = "REGSHOTHIV2";  // use a number to define a file format not compatible with older releases (e.g. "3" could be UTF-32 or Big Endian)
#ifdef _UNICODE
#define szRegshotFileSignature szRegshotFileSignatureUTF16
#else
#define szRegshotFileSignature szRegshotFileSignatureSBCS
#endif

TCHAR szRegshotFileDefExt[] =
#ifdef _UNICODE
    TEXT("hiv2");
#else
    TEXT("hiv");
#endif

TCHAR szValueDataIsNULL[] = TEXT(": (NULL!)");

#ifndef _UNICODE
TCHAR szEmpty[] = TEXT("");
#endif

FILEHEADER fileheader;
SAVEKEYCONTENT sKC;
SAVEVALUECONTENT sVC;

LPBYTE lpFileBuffer;
LPTSTR lpStringBuffer;
size_t nStringBufferSize;
LPBYTE lpDataBuffer;
size_t nDataBufferSize;
size_t nSourceSize;

#define MAX_SIGNATURE_LENGTH 12
#define REGSHOT_READ_BLOCK_SIZE 8192

// Pointers to compare result
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

// Number of Modifications detected
DWORD nKEYADD;
DWORD nKEYDEL;
DWORD nVALADD;
DWORD nVALDEL;
DWORD nVALMODI;
DWORD nFILEADD;
DWORD nFILEDEL;
DWORD nFILEMODI;
DWORD nDIRADD;
DWORD nDIRDEL;
DWORD nDIRMODI;

// Some DWORDs used to show the progress bar and etc
DWORD nGettingValue;
DWORD nGettingKey;
DWORD nComparing;
DWORD nRegStep;
DWORD nFileStep;
DWORD nSavingKey;
DWORD nGettingTime;
DWORD nBASETIME;
DWORD nBASETIME1;
//DWORD nMask = 0xf7fd;     // not used now, but should be added; TODO: what for?
DWORD NBW;                // that is: NumberOfBytesWritten;

LPSTR lpKeyName;
LPSTR lpValueName;
LPBYTE lpValueData;
LPBYTE lpValueDataS;

HANDLE   hFileWholeReg;  // Handle of file regshot use
FILETIME ftLastWrite;    // Filetime struct

TCHAR LOCALMACHINESTRING[]      = TEXT("HKLM");
TCHAR LOCALMACHINESTRING_LONG[] = TEXT("HKEY_LOCAL_MACHINE");
TCHAR USERSSTRING[]             = TEXT("HKU");          // in regshot.ini, "UseLongRegHead" to control this
TCHAR USERSSTRING_LONG[]        = TEXT("HKEY_USERS");   // 1.6 using long name, so in 1.8.1 add an option


// ----------------------------------------------------------------------
// Get whole registry key name from KEYCONTENT
// ----------------------------------------------------------------------
LPTSTR GetWholeKeyName(LPKEYCONTENT lpStartKC)
{
    LPKEYCONTENT lpKC;
    LPTSTR lpszName;
    LPTSTR lpszTail;
    size_t nLen;

    nLen = 0;
    for (lpKC = lpStartKC; NULL != lpKC; lpKC = lpKC->lpFatherKC) {
        if (NULL != lpKC->lpKeyName) {
            nLen += _tcslen(lpKC->lpKeyName) + 1;  // +1 char for backslash or NULL char
        }
    }
    if (0 == nLen) {  // at least create an empty string with NULL char
        nLen++;
    }

    lpszName = MYALLOC(nLen * sizeof(TCHAR));

    lpszTail = &lpszName[nLen];
    *lpszTail = 0;

    for (lpKC = lpStartKC; NULL != lpKC; lpKC = lpKC->lpFatherKC) {
        if (NULL != lpKC->lpKeyName) {
            nLen = _tcslen(lpKC->lpKeyName);
            _tcsncpy(lpszTail -= nLen, lpKC->lpKeyName, nLen);
            if (lpszTail > lpszName) {
                *--lpszTail = (TCHAR)'\\';    // 0x5c = '\\'  // TODO: check if works for Unicode
            }
        }
    }

    return lpszName;
}


// ----------------------------------------------------------------------
// Get whole value name from VALUECONTENT
// ----------------------------------------------------------------------
LPTSTR GetWholeValueName(LPVALUECONTENT lpVC)
{
    LPKEYCONTENT lpKC;
    LPTSTR lpszName;
    LPTSTR lpszTail;
    size_t nLen;
    size_t nWholeLen;

    nLen = 0;
    if (NULL != lpVC->lpValueName) {
        nLen += _tcslen(lpVC->lpValueName);
    }
    nWholeLen = nLen + 1;  // +1 char for NULL char

    for (lpKC = lpVC->lpFatherKC; lpKC != NULL; lpKC = lpKC->lpFatherKC) {
        if (NULL != lpKC->lpKeyName) {
            nWholeLen += _tcslen(lpKC->lpKeyName) + 1;  // +1 char for backslash
        }
    }

    lpszName = MYALLOC(nWholeLen * sizeof(TCHAR));

    lpszTail = &lpszName[nWholeLen];
    *lpszTail = 0;

    if (NULL != lpVC->lpValueName) {
        _tcscpy(lpszTail -= nLen, lpVC->lpValueName);
        *--lpszTail = (TCHAR)'\\'; // 0x5c = '\\'  // TODO: check if works for Unicode
    }

    for (lpKC = lpVC->lpFatherKC; NULL != lpKC; lpKC = lpKC->lpFatherKC) {
        if (NULL != lpKC->lpKeyName) {
            nLen = _tcslen(lpKC->lpKeyName);
            _tcsncpy(lpszTail -= nLen, lpKC->lpKeyName, nLen);
            if (lpszTail > lpszName) {
                *--lpszTail = (TCHAR)'\\';    // 0x5c = '\\'  // TODO: check if works for Unicode
            }
        }
    }

    return lpszName;
}


// ----------------------------------------------------------------------
// Transform VALUECONTENT.data from binary into string
// Called by GetWholeValueData()
// ----------------------------------------------------------------------
LPTSTR TransData(LPVALUECONTENT lpVC, DWORD type)
{
    LPTSTR lpValueData;
    DWORD nCount;
    DWORD nSize;

    lpValueData = NULL;
    nSize = lpVC->datasize;

    if (NULL == lpVC->lpValueData) {
        lpValueData = MYALLOC0(sizeof(szValueDataIsNULL));
        _tcscpy(lpValueData, szValueDataIsNULL);
    } else {
        switch (type) {
            case REG_SZ:
                // case REG_EXPAND_SZ: Not used any more, is included in [default], because some non-regular value would corrupt this.
                lpValueData = MYALLOC0(((3 + 2) * sizeof(TCHAR)) + nSize);  // format  ": \"<string>\"\0"
                _tcscpy(lpValueData, TEXT(": \""));
                if (NULL != lpVC->lpValueData) {
                    _tcscat(lpValueData, (LPTSTR)lpVC->lpValueData);
                }
                _tcscat(lpValueData, TEXT("\""));
                // wsprintf has a bug that can not print string too long one time!);
                //wsprintf(lpValueData,"%s%s%s",": \"",lpVC->lpValueData,"\"");
                break;
            case REG_MULTI_SZ:
                // Be sure to add below line outside of following "if",
                // for that GlobalFree(lp) must had lp already located!
                lpValueData = MYALLOC0(((3 + 2) * sizeof(TCHAR)) + nSize);  // format  ": \"<string>\"\0"
                nSize /= sizeof(TCHAR);  // convert bytes to chars
                nSize--;  // account for last NULL char
                for (nCount = 0; nCount < nSize; nCount++) {
                    if (0 == ((LPTSTR)lpVC->lpValueData)[nCount]) {        // look for a NULL char before the end of the data
                        ((LPTSTR)lpVC->lpValueData)[nCount] = (TCHAR)' ';  // then overwrite with space  // TODO: check if works with Unicode
                    }
                }
                _tcscpy(lpValueData, TEXT(": \""));
                if (NULL != lpVC->lpValueData) {
                    _tcscat(lpValueData, (LPTSTR)lpVC->lpValueData);
                }
                _tcscat(lpValueData, TEXT("\""));
                //wsprintf(lpValueData,"%s%s%s",": \"",lpVC->lpValueData,"\"");
                break;
            case REG_DWORD:
                // case REG_DWORD_BIG_ENDIAN: Not used any more, they all included in [default]
                lpValueData = MYALLOC0((4 + 8 + 1) * sizeof(TCHAR));  // format  ": 0xXXXXXXXX\0"
                _tcscpy(lpValueData, TEXT(": "));
                if (NULL != lpVC->lpValueData) {
                    _stprintf(lpValueData + 2, TEXT("%s%08X"), TEXT("0x"), *(LPDWORD)(lpVC->lpValueData));
                }
                break;
            default:
                lpValueData = MYALLOC0((2 + (nSize * 3) + 1) * sizeof(TCHAR));  // format ": [ xx][ xx]...[ xx]\0"
                _tcscpy(lpValueData, TEXT(": "));
                // for the resttype lengthofvaluedata doesn't contains the 0!
                for (nCount = 0; nCount < nSize; nCount++) {
                    _stprintf(lpValueData + (2 + (nCount * 3)), TEXT(" %02X"), *(lpVC->lpValueData + nCount));
                }
        }
    }

    return lpValueData;
}


// ----------------------------------------------------------------------
// Get value data from VALUECONTENT as string
// ----------------------------------------------------------------------
LPTSTR GetWholeValueData(LPVALUECONTENT lpVC)
{
    LPTSTR lpValueData;
    DWORD nCount;
    DWORD nSize;

    lpValueData = NULL;
    nSize = lpVC->datasize;

    if (NULL == lpVC->lpValueData) {
        lpValueData = MYALLOC0(sizeof(szValueDataIsNULL));
        _tcscpy(lpValueData, szValueDataIsNULL);
    } else {
        switch (lpVC->typecode) {
            case REG_SZ:
            case REG_EXPAND_SZ:
                if ((DWORD)((_tcslen((LPTSTR)lpVC->lpValueData) + 1) * sizeof(TCHAR)) == nSize) {
                    lpValueData = TransData(lpVC, REG_SZ);
                } else {
                    lpValueData = TransData(lpVC, REG_BINARY);
                }
                break;
            case REG_MULTI_SZ:
                if (0 != ((LPTSTR)lpVC->lpValueData)[0]) {
                    for (nCount = 0; ; nCount++) {
                        if (0 == ((LPTSTR)lpVC->lpValueData)[nCount]) {
                            break;
                        }
                    }
                    if (((nCount + 1) * sizeof(TCHAR)) == nSize) {
                        lpValueData = TransData(lpVC, REG_MULTI_SZ);
                    } else {
                        lpValueData = TransData(lpVC, REG_BINARY);
                    }
                } else {
                    lpValueData = TransData(lpVC, REG_BINARY);
                }
                break;
            case REG_DWORD:
            case REG_DWORD_BIG_ENDIAN:
                if (sizeof(DWORD) == nSize) {
                    lpValueData = TransData(lpVC, REG_DWORD);
                } else {
                    lpValueData = TransData(lpVC, REG_BINARY);
                }
                break;
            default :
                lpValueData = TransData(lpVC, REG_BINARY);
        }
    }

    return lpValueData;
}


//-------------------------------------------------------------
// Routine to create new comparison result, distribute to different lp???MODI
//-------------------------------------------------------------
VOID CreateNewResult(DWORD actiontype, LPDWORD lpcount, LPSTR lpresult)
{
    LPCOMRESULT lpnew;
    lpnew = (LPCOMRESULT)MYALLOC0(sizeof(COMRESULT));
    lpnew->lpresult = lpresult;

    switch (actiontype) {
        case KEYADD:
            *lpcount == 0 ? (lpKEYADDHEAD = lpnew) : (lpKEYADD->lpnextresult = lpnew);
            lpKEYADD = lpnew;
            break;
        case KEYDEL:
            *lpcount == 0 ? (lpKEYDELHEAD = lpnew) : (lpKEYDEL->lpnextresult = lpnew);
            lpKEYDEL = lpnew;
            break;
        case VALADD:
            *lpcount == 0 ? (lpVALADDHEAD = lpnew) : (lpVALADD->lpnextresult = lpnew);
            lpVALADD = lpnew;
            break;
        case VALDEL:
            *lpcount == 0 ? (lpVALDELHEAD = lpnew) : (lpVALDEL->lpnextresult = lpnew);
            lpVALDEL = lpnew;
            break;
        case VALMODI:
            *lpcount == 0 ? (lpVALMODIHEAD = lpnew) : (lpVALMODI->lpnextresult = lpnew);
            lpVALMODI = lpnew;
            break;
        case FILEADD:
            *lpcount == 0 ? (lpFILEADDHEAD = lpnew) : (lpFILEADD->lpnextresult = lpnew);
            lpFILEADD = lpnew;
            break;
        case FILEDEL:
            *lpcount == 0 ? (lpFILEDELHEAD = lpnew) : (lpFILEDEL->lpnextresult = lpnew);
            lpFILEDEL = lpnew;
            break;
        case FILEMODI:
            *lpcount == 0 ? (lpFILEMODIHEAD = lpnew) : (lpFILEMODI->lpnextresult = lpnew);
            lpFILEMODI = lpnew;
            break;
        case DIRADD:
            *lpcount == 0 ? (lpDIRADDHEAD = lpnew) : (lpDIRADD->lpnextresult = lpnew);
            lpDIRADD = lpnew;
            break;
        case DIRDEL:
            *lpcount == 0 ? (lpDIRDELHEAD = lpnew) : (lpDIRDEL->lpnextresult = lpnew);
            lpDIRDEL = lpnew;
            break;
        case DIRMODI:
            *lpcount == 0 ? (lpDIRMODIHEAD = lpnew) : (lpDIRMODI->lpnextresult = lpnew);
            lpDIRMODI = lpnew;
            break;

    }
    (*lpcount)++;
}


//-------------------------------------------------------------
// Write comparison results into memory and call CreateNewResult()
//-------------------------------------------------------------
VOID LogToMem(DWORD actiontype, LPDWORD lpcount, LPVOID lp)
{
    LPSTR   lpname;
    LPSTR   lpdata;
    LPSTR   lpall;

    if (actiontype == KEYADD || actiontype == KEYDEL) {
        lpname = GetWholeKeyName(lp);
        CreateNewResult(actiontype, lpcount, lpname);
    } else {
        if (actiontype == VALADD || actiontype == VALDEL || actiontype == VALMODI) {

            lpname = GetWholeValueName(lp);
            lpdata = GetWholeValueData(lp);
            lpall = MYALLOC(strlen(lpname) + strlen(lpdata) + 2);
            // do not use:wsprintf(lpall,"%s%s",lpname,lpdata); !!! strlen limit!
            strcpy(lpall, lpname);
            strcat(lpall, lpdata);
            MYFREE(lpname);
            MYFREE(lpdata);
            CreateNewResult(actiontype, lpcount, lpall);
        } else {
            lpname = GetWholeFileName(lp);
            CreateNewResult(actiontype, lpcount, lpname);
        }
    }
}


//-------------------------------------------------------------
// Routine to walk through sub keytree of current Key
//-------------------------------------------------------------
VOID GetAllSubName(
    BOOL    needbrother,
    DWORD   typekey,
    DWORD   typevalue,
    LPDWORD lpcountkey,
    LPDWORD lpcountvalue,
    LPKEYCONTENT lpKC
)
{
    LPVALUECONTENT lpVC;

    LogToMem(typekey, lpcountkey, lpKC);

    if (lpKC->lpFirstSubKC != NULL) {
        GetAllSubName(TRUE, typekey, typevalue, lpcountkey, lpcountvalue, lpKC->lpFirstSubKC);
    }

    if (needbrother == TRUE)
        if (lpKC->lpBrotherKC != NULL) {
            GetAllSubName(TRUE, typekey, typevalue, lpcountkey, lpcountvalue, lpKC->lpBrotherKC);
        }

    for (lpVC = lpKC->lpFirstVC; lpVC != NULL; lpVC = lpVC->lpBrotherVC) {
        LogToMem(typevalue, lpcountvalue, lpVC);
    }
}


//-------------------------------------------------------------
// Routine to walk through all values of current key
//-------------------------------------------------------------
VOID GetAllValue(DWORD typevalue, LPDWORD lpcountvalue, LPKEYCONTENT lpKC)
{
    LPVALUECONTENT lpVC;

    for (lpVC = lpKC->lpFirstVC; lpVC != NULL; lpVC = lpVC->lpBrotherVC) {
        LogToMem(typevalue, lpcountvalue, lpVC);
    }
}


//-------------------------------------------------------------
// Routine to free all comparison results (release memory)
//-------------------------------------------------------------
VOID FreeAllCom(LPCOMRESULT lpComResult)
{
    LPCOMRESULT lp;
    LPCOMRESULT lpold;

    for (lp = lpComResult; lp != NULL;) {
        if (lp->lpresult != NULL) {
            MYFREE(lp->lpresult);
        }
        lpold = lp;
        lp = lp->lpnextresult;
        MYFREE(lpold);
    }

}

// ----------------------------------------------------------------------
// Free all compare results
// ----------------------------------------------------------------------
VOID FreeAllCompareResults(void)
{
    FreeAllCom(lpKEYADDHEAD);
    FreeAllCom(lpKEYDELHEAD);
    FreeAllCom(lpVALADDHEAD);
    FreeAllCom(lpVALDELHEAD);
    FreeAllCom(lpVALMODIHEAD);
    FreeAllCom(lpFILEADDHEAD);
    FreeAllCom(lpFILEDELHEAD);
    FreeAllCom(lpFILEMODIHEAD);
    FreeAllCom(lpDIRADDHEAD);
    FreeAllCom(lpDIRDELHEAD);
    FreeAllCom(lpDIRMODIHEAD);


    nKEYADD = 0;
    nKEYDEL = 0;
    nVALADD = 0;
    nVALDEL = 0;
    nVALMODI = 0;
    nFILEADD = 0;
    nFILEDEL = 0;
    nFILEMODI = 0;
    nDIRADD = 0;
    nDIRDEL = 0;
    nDIRMODI = 0;
    lpKEYADDHEAD = NULL;
    lpKEYDELHEAD = NULL;
    lpVALADDHEAD = NULL;
    lpVALDELHEAD = NULL;
    lpVALMODIHEAD = NULL;
    lpFILEADDHEAD = NULL;
    lpFILEDELHEAD = NULL;
    lpFILEMODIHEAD = NULL;
    lpDIRADDHEAD = NULL;
    lpDIRDELHEAD = NULL;
    lpDIRMODIHEAD = NULL;
}


//-------------------------------------------------------------
// Registry comparison engine
//-------------------------------------------------------------
VOID *CompareFirstSubKey(LPKEYCONTENT lpHeadKC1, LPKEYCONTENT lpHeadKC2)
{
    LPKEYCONTENT    lpKC1;
    LPKEYCONTENT    lpKC2;
    LPVALUECONTENT  lpVC1;
    LPVALUECONTENT  lpVC2;
    //DWORD           i;

    for (lpKC1 = lpHeadKC1; lpKC1 != NULL; lpKC1 = lpKC1->lpBrotherKC) {
        for (lpKC2 = lpHeadKC2; lpKC2 != NULL; lpKC2 = lpKC2->lpBrotherKC) {
            if (NOTMATCH == lpKC2->bkeymatch) {
                if ((lpKC1->lpKeyName == lpKC2->lpKeyName)
                        || ((NULL != lpKC1->lpKeyName) && (NULL != lpKC2->lpKeyName) && (0 == strcmp(lpKC1->lpKeyName, lpKC2->lpKeyName)))) { // 1.8.2 from lstrcmp to strcmp
                    // Same key found! We compare their values and their sub keys!
                    lpKC2->bkeymatch = ISMATCH;

                    if ((NULL == lpKC1->lpFirstVC) && (NULL != lpKC2->lpFirstVC)) {
                        // Key1 has no values, so lpVC2 is added! We find all values that belong to lpKC2!
                        GetAllValue(VALADD, &nVALADD, lpKC2);
                    } else {
                        if ((NULL != lpKC1->lpFirstVC) && (NULL == lpKC2->lpFirstVC)) {
                            // Key2 has no values, so lpVC1 is deleted! We find all values that belong to lpKC1!
                            GetAllValue(VALDEL, &nVALDEL, lpKC1);
                        } else {
                            // Two keys, both have values, so we loop them
                            for (lpVC1 = lpKC1->lpFirstVC; lpVC1 != NULL; lpVC1 = lpVC1->lpBrotherVC) {
                                for (lpVC2 = lpKC2->lpFirstVC; lpVC2 != NULL; lpVC2 = lpVC2->lpBrotherVC) {
                                    // Loop lpKC2 to find a value matchs lpKC1's
                                    if ((NOTMATCH == lpVC2->bvaluematch) && (lpVC1->typecode == lpVC2->typecode)) {
                                        // Same valuedata type
                                        if ((lpVC1->lpValueName == lpVC2->lpValueName)
                                                || ((NULL != lpVC1->lpValueName) && (NULL != lpVC2->lpValueName) && (0 == strcmp(lpVC1->lpValueName, lpVC2->lpValueName)))) { // 1.8.2 from lstrcmp to strcmp
                                            // Same valuename
                                            if ((lpVC1->datasize == lpVC2->datasize)) {
                                                // Same size of valuedata
                                                if (0 == memcmp(lpVC1->lpValueData, lpVC2->lpValueData, lpVC1->datasize)) { // 1.8.2
                                                    // Same valuedata, keys are the same!
                                                    lpVC2->bvaluematch = ISMATCH;
                                                    break;  // Be sure not to do lpKC2 == NULL
                                                } else {
                                                    // Valuedata not match due to data mismatch, we found a modified valuedata!*****
                                                    lpVC2->bvaluematch = ISMODI;
                                                    LogToMem(VALMODI, &nVALMODI, lpVC1);
                                                    LogToMem(VALMODI, &nVALMODI, lpVC2);
                                                    nVALMODI--;
                                                    break;
                                                }
                                            } else {
                                                // Valuedata does not match due to size, we found a modified valuedata!******
                                                lpVC2->bvaluematch = ISMODI;
                                                LogToMem(VALMODI, &nVALMODI, lpVC1);
                                                LogToMem(VALMODI, &nVALMODI, lpVC2);
                                                nVALMODI--;
                                                break;
                                            }
                                        }
                                    }
                                }
                                if (NULL == lpVC2) {
                                    // We found a value in lpKC1 but not in lpKC2, we found a deleted value*****
                                    LogToMem(VALDEL, &nVALDEL, lpVC1);
                                }
                            }
                            // After we loop to end, we do extra loop use flag we previously made to get added values
                            for (lpVC2 = lpKC2->lpFirstVC; lpVC2 != NULL; lpVC2 = lpVC2->lpBrotherVC) {
                                if (lpVC2->bvaluematch != ISMATCH && lpVC2->bvaluematch != ISMODI) {
                                    // We found a value in lpKC2's but not in lpKC1's, we found a added value****
                                    LogToMem(VALADD, &nVALADD, lpVC2);
                                }
                            }
                        }
                    }

                    //////////////////////////////////////////////////////////////
                    // After we walk through the values above, we now try to loop the sub keys of current key
                    if ((NULL == lpKC1->lpFirstSubKC) && (NULL != lpKC2->lpFirstSubKC)) {
                        // lpKC2's firstsubkey added!
                        GetAllSubName(TRUE, KEYADD, VALADD, &nKEYADD, &nVALADD, lpKC2->lpFirstSubKC);
                    }
                    if ((NULL != lpKC1->lpFirstSubKC) && (NULL == lpKC2->lpFirstSubKC)) {
                        // lpKC1's firstsubkey deleted!
                        GetAllSubName(TRUE, KEYDEL, VALDEL, &nKEYDEL, &nVALDEL, lpKC1->lpFirstSubKC);
                    }
                    if ((NULL != lpKC1->lpFirstSubKC) && (NULL != lpKC2->lpFirstSubKC)) {
                        CompareFirstSubKey(lpKC1->lpFirstSubKC, lpKC2->lpFirstSubKC);
                    }
                    break;
                }
            }
        }
        if (NULL == lpKC2) {
            // We did not find a lpKC2 matches a lpKC1, so lpKC1 is deleted!
            GetAllSubName(FALSE, KEYDEL, VALDEL, &nKEYDEL, &nVALDEL, lpKC1);
        }
    }

    // After we loop to end, we do extra loop use flag we previously made to get added keys
    for (lpKC2 = lpHeadKC2; lpKC2 != NULL; lpKC2 = lpKC2->lpBrotherKC) {
        nComparing++;
        if (lpKC2->bkeymatch == NOTMATCH) {
            // We did not find a lpKC1 matches a lpKC2,so lpKC2 is added!
            GetAllSubName(FALSE, KEYADD, VALADD, &nKEYADD, &nVALADD, lpKC2);
        }
    }

    // Progress bar update
    if (nGettingKey != 0)
        if (nComparing % nGettingKey > nRegStep) {
            nComparing = 0;
            SendDlgItemMessage(hWnd, IDC_PBCOMPARE, PBM_STEPIT, (WPARAM)0, (LPARAM)0);
        }

    return NULL;
}


//------------------------------------------------------------
// Routine to call registry/file comparison engine
//------------------------------------------------------------
BOOL CompareShots(LPREGSHOT lpShot1, LPREGSHOT lpShot2)
{
    BOOL    fAsHTML;
    BOOL    bshot2isnewer;
    //BOOL    bSaveWithCommentName;
    LPSTR   lpstrcomp;
    LPSTR   lpExt;
    LPSTR   lpDestFileName;
    DWORD   buffersize = 2048;
    DWORD   nTotal;
    size_t  nLengthofStr;
    LPHEADFILE  lpHF1;
    LPHEADFILE  lpHF2;
    LPFILECONTENT lpfc1;
    LPFILECONTENT lpfc2;
    FILETIME ftime1;
    FILETIME ftime2;


    if (!DirChainMatch(lpShot1->lpHF, lpShot2->lpHF)) {
        MessageBox(hWnd, "Found two shots with different DIR chain! (or with different order)\r\nYou can continue, but file comparison result would be abnormal!", "Warning", MB_ICONWARNING);
    }

    InitProgressBar();

    SystemTimeToFileTime(&lpShot1->systemtime, &ftime1);
    SystemTimeToFileTime(&lpShot2->systemtime, &ftime2);

    bshot2isnewer = (CompareFileTime(&ftime1, &ftime2) <= 0) ? TRUE : FALSE;
    if (bshot2isnewer) {
        CompareFirstSubKey(lpShot1->lpHKLM, lpShot2->lpHKLM);
        CompareFirstSubKey(lpShot1->lpHKU, lpShot2->lpHKU);
    } else {
        CompareFirstSubKey(lpShot2->lpHKLM, lpShot1->lpHKLM);
        CompareFirstSubKey(lpShot2->lpHKU, lpShot1->lpHKU);
    }

    SendDlgItemMessage(hWnd, IDC_PBCOMPARE, PBM_SETPOS, (WPARAM)0, (LPARAM)0);

    // Dir comparison v1.8.1
    // determine newer
    if (bshot2isnewer) {
        lpHF1 = lpShot1->lpHF;
        lpHF2 = lpShot2->lpHF;
    } else {
        lpHF1 = lpShot2->lpHF;
        lpHF2 = lpShot1->lpHF;
    }
    // first loop
    for (; lpHF1 != NULL; lpHF1 = lpHF1->lpBrotherHF) {
        lpfc1 = lpHF1->lpFirstFC;
        if (lpfc1 != NULL) {
            if ((lpfc2 = SearchDirChain(lpfc1->lpFileName, lpHF2)) != NULL) {   // note lpHF2 should not changed here!
                CompareFirstSubFile(lpfc1, lpfc2);                              // if found, we do compare
            } else {    // cannot find matched lpfc1 in lpHF2 chain.
                GetAllSubFile(FALSE, DIRDEL, FILEDEL, &nDIRDEL, &nFILEDEL, lpfc1);
            }
        }
    }
    // reset pointers
    if (bshot2isnewer) {
        lpHF1 = lpShot1->lpHF;
        lpHF2 = lpShot2->lpHF;
    } else {
        lpHF1 = lpShot2->lpHF;
        lpHF2 = lpShot1->lpHF;
    }
    // second loop
    for (; lpHF2 != NULL; lpHF2 = lpHF2->lpBrotherHF) {
        lpfc2 = lpHF2->lpFirstFC;
        if (lpfc2 != NULL) {
            if ((lpfc1 = SearchDirChain(lpfc2->lpFileName, lpHF1)) == NULL) {   // in the second loop we only find those do not match
                GetAllSubFile(FALSE, DIRADD, FILEADD, &nDIRADD, &nFILEADD, lpfc2);
            }
        }
    }

    SendDlgItemMessage(hWnd, IDC_PBCOMPARE, PBM_SETPOS, (WPARAM)MAXPBPOSITION, (LPARAM)0);

    if (SendMessage(GetDlgItem(hWnd, IDC_RADIO1), BM_GETCHECK, (WPARAM)0, (LPARAM)0) == 1) {
        fAsHTML = FALSE;
        lpExt = ".txt";
    } else {
        fAsHTML = TRUE;
        lpExt = ".htm";
    }

    lpDestFileName = MYALLOC0(MAX_PATH * 4 + 4);
    lpstrcomp = MYALLOC0(buffersize); // buffersize must > commentlength + 10 .txt 0000
    GetDlgItemText(hWnd, IDC_EDITCOMMENT, lpstrcomp, COMMENTLENGTH);
    GetDlgItemText(hWnd, IDC_EDITPATH, lpOutputpath, MAX_PATH);  // length incl. NULL character

    nLengthofStr = strlen(lpOutputpath);

    if (nLengthofStr > 0 && *(lpOutputpath + nLengthofStr - 1) != '\\') {
        *(lpOutputpath + nLengthofStr) = '\\';
        *(lpOutputpath + nLengthofStr + 1) = 0x00;    // bug found by "itschy" <itschy@lycos.de> 1.61d->1.61e
        nLengthofStr++;
    }
    strcpy(lpDestFileName, lpOutputpath);

    //bSaveWithCommentName = TRUE;
    if (ReplaceInvalidFileNameChars(lpstrcomp)) {
        strcat(lpDestFileName, lpstrcomp);
    } else {
        strcat(lpDestFileName, szDefResPre);
    }

    nLengthofStr = strlen(lpDestFileName);
    strcat(lpDestFileName, lpExt);
    hFile = CreateFile(lpDestFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD filetail = 0;

        for (filetail = 0; filetail < MAXAMOUNTOFFILE; filetail++) {
            sprintf(lpDestFileName + nLengthofStr, "_%04u", filetail);
            //*(lpDestFileName+nLengthofStr + 5) = 0x00;
            strcpy(lpDestFileName + nLengthofStr + 5, lpExt);

            hFile = CreateFile(lpDestFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                if (GetLastError() == ERROR_FILE_EXISTS) {    // My God! I use stupid ERROR_ALREADY_EXISTS first!!
                    continue;
                } else {
                    ErrMsg(asLangTexts[iszTextErrorCreateFile].lpString);
                    return FALSE;
                }
            } else {
                break;
            }
        }
        if (filetail >= MAXAMOUNTOFFILE) {
            ErrMsg(asLangTexts[iszTextErrorCreateFile].lpString);
            return FALSE;
        }

    }

    if (fAsHTML == TRUE) {
        WriteHTMLBegin();
    } else {
        WriteFile(hFile, lpszProgramName, (DWORD)(_tcslen(lpszProgramName) * sizeof(TCHAR)), &NBW, NULL);
        WriteFile(hFile, szCRLF, (DWORD)(_tcslen(szCRLF) * sizeof(TCHAR)), &NBW, NULL);
    }

    //_asm int 3;
    GetDlgItemText(hWnd, IDC_EDITCOMMENT, lpstrcomp, COMMENTLENGTH);
    WriteTitle(asLangTexts[iszTextComments].lpString, lpstrcomp, fAsHTML);


    sprintf(lpstrcomp, "%d%s%d%s%d %02d%s%02d%s%02d %s %d%s%d%s%d %02d%s%02d%s%02d",
            lpShot1->systemtime.wYear, "/",
            lpShot1->systemtime.wMonth, "/",
            lpShot1->systemtime.wDay,
            lpShot1->systemtime.wHour, ":",
            lpShot1->systemtime.wMinute, ":",
            lpShot1->systemtime.wSecond, " , ",
            lpShot2->systemtime.wYear, "/",
            lpShot2->systemtime.wMonth, "/",
            lpShot2->systemtime.wDay,
            lpShot2->systemtime.wHour, ":",
            lpShot2->systemtime.wMinute, ":",
            lpShot2->systemtime.wSecond

           );

    WriteTitle(asLangTexts[iszTextDateTime].lpString, lpstrcomp, fAsHTML);


    *lpstrcomp = 0x00;    //ZeroMemory(lpstrcomp,buffersize);

    if (NULL != lpShot1->computername) {
        strcpy(lpstrcomp, lpShot1->computername);
    }
    strcat(lpstrcomp, " , ");
    if (NULL != lpShot2->computername) {
        strcat(lpstrcomp, lpShot2->computername);
    }
    WriteTitle(asLangTexts[iszTextComputer].lpString, lpstrcomp, fAsHTML);

    *lpstrcomp = 0x00;    //ZeroMemory(lpstrcomp,buffersize);

    if (NULL != lpShot1->username) {
        strcpy(lpstrcomp, lpShot1->username);
    }
    strcat(lpstrcomp, " , ");
    if (NULL != lpShot2->username) {
        strcat(lpstrcomp, lpShot2->username);
    }

    WriteTitle(asLangTexts[iszTextUsername].lpString, lpstrcomp, fAsHTML);

    MYFREE(lpstrcomp);

    // Write keydel part
    if (nKEYDEL != 0) {
        WriteTableHead(asLangTexts[iszTextKeyDel].lpString, nKEYDEL, fAsHTML);
        WritePart(lpKEYDELHEAD, fAsHTML, FALSE);
    }
    // Write keyadd part
    if (nKEYADD != 0) {
        WriteTableHead(asLangTexts[iszTextKeyAdd].lpString, nKEYADD, fAsHTML);
        WritePart(lpKEYADDHEAD, fAsHTML, FALSE);
    }
    // Write valdel part
    if (nVALDEL != 0) {
        WriteTableHead(asLangTexts[iszTextValDel].lpString, nVALDEL, fAsHTML);
        WritePart(lpVALDELHEAD, fAsHTML, FALSE);
    }
    // Write valadd part
    if (nVALADD != 0) {
        WriteTableHead(asLangTexts[iszTextValAdd].lpString, nVALADD, fAsHTML);
        WritePart(lpVALADDHEAD, fAsHTML, FALSE);
    }
    // Write valmodi part
    if (nVALMODI != 0) {
        WriteTableHead(asLangTexts[iszTextValModi].lpString, nVALMODI, fAsHTML);
        WritePart(lpVALMODIHEAD, fAsHTML, TRUE);
    }
    // Write file add part
    if (nFILEADD != 0) {
        WriteTableHead(asLangTexts[iszTextFileAdd].lpString, nFILEADD, fAsHTML);
        WritePart(lpFILEADDHEAD, fAsHTML, FALSE);
    }
    // Write file del part
    if (nFILEDEL != 0) {
        WriteTableHead(asLangTexts[iszTextFileDel].lpString, nFILEDEL, fAsHTML);
        WritePart(lpFILEDELHEAD, fAsHTML, FALSE);
    }
    // Write file modi part
    if (nFILEMODI != 0) {
        WriteTableHead(asLangTexts[iszTextFileModi].lpString, nFILEMODI, fAsHTML);
        WritePart(lpFILEMODIHEAD, fAsHTML, FALSE);
    }
    // Write directory add part
    if (nDIRADD != 0) {
        WriteTableHead(asLangTexts[iszTextDirAdd].lpString, nDIRADD, fAsHTML);
        WritePart(lpDIRADDHEAD, fAsHTML, FALSE);
    }
    // Write directory del part
    if (nDIRDEL != 0) {
        WriteTableHead(asLangTexts[iszTextDirDel].lpString, nDIRDEL, fAsHTML);
        WritePart(lpDIRDELHEAD, fAsHTML, FALSE);
    }
    // Write directory modi part
    if (nDIRMODI != 0) {
        WriteTableHead(asLangTexts[iszTextDirModi].lpString, nDIRMODI, fAsHTML);
        WritePart(lpDIRMODIHEAD, fAsHTML, FALSE);
    }

    nTotal = nKEYADD + nKEYDEL + nVALADD + nVALDEL + nVALMODI + nFILEADD + nFILEDEL  + nFILEMODI + nDIRADD + nDIRDEL + nDIRMODI;
    if (fAsHTML == TRUE) {
        WriteHTML_BR();
    }
    WriteTableHead(asLangTexts[iszTextTotal].lpString, nTotal, fAsHTML);
    if (fAsHTML == TRUE) {
        WriteHTMLEnd();
    }


    CloseHandle(hFile);

    if ((size_t)ShellExecute(hWnd, "open", lpDestFileName, NULL, NULL, SW_SHOW) <= 32) {
        ErrMsg(asLangTexts[iszTextErrorExecViewer].lpString);
    }
    MYFREE(lpDestFileName);


    return TRUE;
}


// ----------------------------------------------------------------------
// Clear comparison match flags in all registry keys
// ----------------------------------------------------------------------
VOID ClearKeyMatchTag(LPKEYCONTENT lpKC)
{
    LPVALUECONTENT lpVC;

    if (NULL != lpKC) {
        lpKC->bkeymatch = 0;
        for (lpVC = lpKC->lpFirstVC; NULL != lpVC; lpVC = lpVC->lpBrotherVC) {
            lpVC->bvaluematch = 0;
        }
        ClearKeyMatchTag(lpKC->lpFirstSubKC);
        ClearKeyMatchTag(lpKC->lpBrotherKC);
    }
}


// ----------------------------------------------------------------------
// Free all registry keys and values
// ----------------------------------------------------------------------
VOID FreeAllValueContent(LPVALUECONTENT lpVC)
{
    if (NULL != lpVC) {
        if (NULL != lpVC->lpValueName) {
            MYFREE(lpVC->lpValueName);
        }
        if (NULL != lpVC->lpValueData) {
            MYFREE(lpVC->lpValueData);
        }
        FreeAllValueContent(lpVC->lpBrotherVC);
        MYFREE(lpVC);
    }
}

// ----------------------------------------------------------------------
// Free all registry keys and values
// ----------------------------------------------------------------------
VOID FreeAllKeyContent(LPKEYCONTENT lpKC)
{
    if (NULL != lpKC) {
        if (NULL != lpKC->lpKeyName) {
            if ((lpKC->lpKeyName != LOCALMACHINESTRING)
                    && (lpKC->lpKeyName != LOCALMACHINESTRING_LONG)
                    && (lpKC->lpKeyName != USERSSTRING)
                    && (lpKC->lpKeyName != USERSSTRING_LONG)) {
                MYFREE(lpKC->lpKeyName);
            }
        }
        FreeAllValueContent(lpKC->lpFirstVC);
        FreeAllKeyContent(lpKC->lpFirstSubKC);
        FreeAllKeyContent(lpKC->lpBrotherKC);
        MYFREE(lpKC);
    }
}

// ----------------------------------------------------------------------
// Free shot completely and initialize
// ----------------------------------------------------------------------
VOID FreeShot(LPREGSHOT lpShot)
{
    if (NULL != lpShot->computername) {
        MYFREE(lpShot->computername);
    }

    if (NULL != lpShot->username) {
        MYFREE(lpShot->username);
    }

    FreeAllKeyContent(lpShot->lpHKLM);
    FreeAllKeyContent(lpShot->lpHKU);
    FreeAllFileHead(lpShot->lpHF);

    ZeroMemory(lpShot, sizeof(REGSHOT));
}


// ----------------------------------------------------------------------
// Get registry snap shot
// ----------------------------------------------------------------------
LPKEYCONTENT GetRegistrySnap(HKEY hRegKey, LPTSTR lpszRegKeyName, LPKEYCONTENT lpFatherKC, LPKEYCONTENT *lplpCaller)
{
    LPKEYCONTENT lpKC;
    DWORD nSubKeys;
    DWORD nMaxSubKeyNameLen;

    // Process registry key itself, then key values with data, then sub keys (see msdn.microsoft.com/en-us/library/windows/desktop/ms724256.aspx)

    // Extra local block to reduce stack usage due to recursive calls
    {
        LPTSTR lpszFullRegKeyName;
        DWORD nValues;
        DWORD nMaxValueNameLen;
        DWORD nMaxValueDataLen;

        // Create new key content
        // put in a separate var for later use
        lpKC = MYALLOC0(sizeof(KEYCONTENT));
        ZeroMemory(lpKC, sizeof(KEYCONTENT));

        // Write pointer to current key into caller's pointer
        if (NULL != lplpCaller) {
            *lplpCaller = lpKC;
        }

        // Set father of current key
        lpKC->lpFatherKC = lpFatherKC;

        // Set key name
        lpKC->lpKeyName = lpszRegKeyName;

        // Check if key is to be excluded
        lpszFullRegKeyName = GetWholeKeyName(lpKC);
        if (IsInSkipList(lpszFullRegKeyName, lprgszRegSkipStrings)) {
            MYFREE(lpszFullRegKeyName);
            FreeAllKeyContent(lpKC);
            return NULL;
        }
        MYFREE(lpszFullRegKeyName);

        nGettingKey++;

        // Examine key for values and sub keys, get counts and also maximum lengths of names plus value data
        if (ERROR_SUCCESS != RegQueryInfoKey(
                    hRegKey,             // key handle
                    NULL,                // LPTSTR lpClass
                    NULL,                // LPDWORD lpcClass
                    NULL,                // LPDWORD lpReserved
                    &nSubKeys,           // LPDWORD lpcSubKeys (count)
                    &nMaxSubKeyNameLen,  // LPDWORD lpcMaxSubKeyLen (in chars/TCHARs *not* incl. NULL char)
                    NULL,                // LPDWORD lpcMaxClassLen (in chars *not* incl. NULL char)
                    &nValues,            // LPDWORD lpcValues (count)
                    &nMaxValueNameLen,   // LPDWORD lpcMaxValueNameLen (in chars/TCHARs *not* incl. NULL char)
                    &nMaxValueDataLen,   // LPDWORD lpcMaxValueLen (count in bytes)
                    NULL,                // LPDWORD lpcbSecurityDescriptor (count in bytes)
                    NULL                 // PFILETIME lpftLastWriteTime
                )) {
            return NULL;
        }

        // Copy the registry values of the current key
        if (0 < nValues) {
            LPVALUECONTENT lpVC;
            LPVALUECONTENT *lplpVCPrev;
            DWORD i;
            DWORD nRetCode;
            DWORD nValueNameLen;
            DWORD nValueType;
            DWORD nValueDataLen;
#ifdef DEBUGLOG
            LPTSTR lpszDebugMsg;
#endif

            // Account for NULL char
            if (0 < nMaxValueNameLen) {
                nMaxValueNameLen++;
            }

            // Get buffer for maximum value name length
            nSourceSize = nMaxValueNameLen * sizeof(TCHAR);
            nStringBufferSize = AdjustBuffer(&lpStringBuffer, nStringBufferSize, nSourceSize, REGSHOT_BUFFER_BLOCK_BYTES);

            // Get buffer for maximum value data length
            nSourceSize = nMaxValueDataLen;
            nDataBufferSize = AdjustBuffer(&lpDataBuffer, nDataBufferSize, nSourceSize, REGSHOT_BUFFER_BLOCK_BYTES);

            // Get registry key values
            lplpVCPrev = &lpKC->lpFirstVC;
            for (i = 0; ; i++) {
                // Enumerate value
                nValueNameLen = nMaxValueNameLen;
                nValueDataLen = nMaxValueDataLen;
                nRetCode = RegEnumValue(hRegKey, i,
                                        lpStringBuffer,
                                        &nValueNameLen,   // in chars/TCHARs; out: *not* incl. NULL char
                                        NULL,             // LPDWORD lpReserved
                                        &nValueType,
                                        lpDataBuffer,
                                        &nValueDataLen);  // in bytes
                if (ERROR_NO_MORE_ITEMS == nRetCode) {
                    break;
                }
                if (ERROR_SUCCESS != nRetCode) {
                    // TODO: protocol issue in some way, do not silently ignore
                    continue;
                }
                lpStringBuffer[nValueNameLen] = 0;

#ifdef DEBUGLOG
                DebugLog(szDebugTryToGetValueLog, TEXT("trying: "), FALSE);
                DebugLog(szDebugTryToGetValueLog, lpStringBuffer, TRUE);
#endif

                // Create new value content
                lpVC = MYALLOC0(sizeof(VALUECONTENT));
                ZeroMemory(lpVC, sizeof(VALUECONTENT));

                // Write pointer to current value into previous value's next value pointer
                if (NULL != lplpVCPrev) {
                    *lplpVCPrev = lpVC;
                }
                lplpVCPrev = &lpVC->lpBrotherVC;

                // Set father key to current key
                lpVC->lpFatherKC = lpKC;

                // Copy values
                lpVC->typecode = nValueType;
                lpVC->datasize = nValueDataLen;

                // Copy value name
                if (0 < nValueNameLen) {
                    lpVC->lpValueName = MYALLOC((nValueNameLen + 1) * sizeof(TCHAR));
                    _tcscpy(lpVC->lpValueName, lpStringBuffer);
                }

                // Copy value data
                if (0 < nValueDataLen) {  // otherwise leave it NULL
                    lpVC->lpValueData = MYALLOC0(nValueDataLen);
                    CopyMemory(lpVC->lpValueData, lpDataBuffer, nValueDataLen);
                }

                nGettingValue++;

#ifdef DEBUGLOG
                lpszDebugMsg = MYALLOC0((REGSHOT_DEBUG_MESSAGE_LENGTH + 1) * sizeof(TCHAR));
                lpszDebugMsg[REGSHOT_DEBUG_MESSAGE_LENGTH] = 0;  // safety NULL char
                _sntprintf(lpszDebugMsg, REGSHOT_DEBUG_MESSAGE_LENGTH, TEXT("LGVN:%08d LGVD:%08d VN:%08d VD:%08d"), nMaxValueNameLen, nMaxValueDataLen, nValueNameLen, nValueDataLen);
                DebugLog(szDebugValueNameDataLog, lpszDebugMsg, TRUE);
                MYFREE(lpszDebugMsg);

                lpszDebugMsg = GetWholeValueName(lpVC);
                DebugLog(szDebugValueNameDataLog, lpszDebugMsg, FALSE);
                MYFREE(lpszDebugMsg);

                lpszDebugMsg = GetWholeValueData(lpVC);
                DebugLog(szDebugValueNameDataLog, lpszDebugMsg, TRUE);
                MYFREE(lpszDebugMsg);
#endif
            }
        }
    }  // End of extra local block

    nGettingTime = GetTickCount();
    if (REFRESHINTERVAL < (nGettingTime - nBASETIME1)) {
        UpdateCounters(asLangTexts[iszTextKey].lpString, asLangTexts[iszTextValue].lpString, nGettingKey, nGettingValue);
    }

    // Process sub keys
    if (0 < nSubKeys) {
        LPKEYCONTENT lpKCSub;
        LPKEYCONTENT *lplpKCPrev;
        DWORD i;
        LPTSTR lpszRegSubKeyName;
        HKEY hRegSubKey;

        // Account for NULL char
        if (0 < nMaxSubKeyNameLen) {
            nMaxSubKeyNameLen++;
        }

        // Get buffer for maximum sub key name length
        nSourceSize = nMaxSubKeyNameLen * sizeof(TCHAR);
        nStringBufferSize = AdjustBuffer(&lpStringBuffer, nStringBufferSize, nSourceSize, REGSHOT_BUFFER_BLOCK_BYTES);

        // Get registry sub keys
        lplpKCPrev = &lpKC->lpFirstSubKC;
        for (i = 0; ; i++) {
            // Extra local block to reduce stack usage due to recursive calls
            {
                DWORD nSubKeyNameLen;
                DWORD nRetCode;
#ifdef DEBUGLOG
                LPTSTR lpszDebugMsg;
#endif

                // Enumerate sub key
                nSubKeyNameLen = nMaxSubKeyNameLen;
                nRetCode = RegEnumKeyEx(hRegKey, i,
                                        lpStringBuffer,
                                        &nSubKeyNameLen,  // in chars/TCHARs; out: *not* incl. NULL char
                                        NULL,             // LPDWORD lpReserved
                                        NULL,             // LPTSTR lpClass
                                        NULL,             // LPDWORD lpcClass
                                        NULL);            // PFILETIME lpftLastWriteTime
                if (ERROR_NO_MORE_ITEMS == nRetCode) {
                    break;
                }
                if (ERROR_SUCCESS != nRetCode) {
                    // TODO: protocol issue in some way, do not silently ignore
                    continue;
                }
                lpStringBuffer[nSubKeyNameLen] = 0;

                // Copy sub key name
                lpszRegSubKeyName = NULL;
                if (0 < nSubKeyNameLen) {
                    lpszRegSubKeyName = MYALLOC((nSubKeyNameLen + 1) * sizeof(TCHAR));
                    _tcscpy(lpszRegSubKeyName, lpStringBuffer);
                }

#ifdef DEBUGLOG
                lpszDebugMsg = MYALLOC0((REGSHOT_DEBUG_MESSAGE_LENGTH + 1) * sizeof(TCHAR));
                lpszDebugMsg[REGSHOT_DEBUG_MESSAGE_LENGTH] = 0;  // safety NULL char
                _sntprintf(lpszDebugMsg, REGSHOT_DEBUG_MESSAGE_LENGTH, TEXT("LGKN:%08d KN:%08d"), nMaxSubKeyNameLen, nSubKeyNameLen);
                DebugLog(szDebugKeyLog, lpszDebugMsg, TRUE);
                MYFREE(lpszDebugMsg);

                lpszDebugMsg = GetWholeKeyName(lpKC);
                DebugLog(szDebugKeyLog, lpszDebugMsg, FALSE);
                MYFREE(lpszDebugMsg);

                DebugLog(szDebugKeyLog, TEXT("\\"), FALSE);
                DebugLog(szDebugKeyLog, lpszRegSubKeyName, TRUE);
#endif
            }  // End of extra local block

            if (ERROR_SUCCESS != RegOpenKeyEx(hRegKey, lpszRegSubKeyName, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hRegSubKey)) {
                // TODO: protocol issue in some way, do not silently ignore
                continue;
            }

            lpKCSub = GetRegistrySnap(hRegSubKey, lpszRegSubKeyName, lpKC, lplpKCPrev);
            RegCloseKey(hRegSubKey);

            if (NULL != lpKCSub) {
                lplpKCPrev = &lpKCSub->lpBrotherKC;
            }
        }
    }

    return lpKC;
}


VOID Shot(LPREGSHOT lpShot)
{
    FreeShot(lpShot);

    lpStringBuffer = NULL;
    lpDataBuffer = NULL;

    /*
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
    */

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

    GetRegistrySnap(HKEY_LOCAL_MACHINE, LOCALMACHINESTRING_LONG, NULL, &lpShot->lpHKLM);
    GetRegistrySnap(HKEY_USERS, USERSSTRING_LONG, NULL, &lpShot->lpHKU);

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

        if (nLengthofStr > 0) {
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

    if (NULL != lpStringBuffer) {
        MYFREE(lpStringBuffer);
        lpStringBuffer = NULL;
    }
    if (NULL != lpDataBuffer) {
        MYFREE(lpDataBuffer);
        lpDataBuffer = NULL;
    }
}


// ----------------------------------------------------------------------
// Save registry key with values to HIVE file
//
// This routine is called recursively to store the keys of the Registry tree
// Therefore temporary vars are put in a local block to reduce stack usage
// ----------------------------------------------------------------------
VOID SaveRegKey(LPKEYCONTENT lpKC, DWORD nFPFatherKey, DWORD nFPCaller)
{
    DWORD nFPKey;

    // Get current file position
    // put in a separate var for later use
    nFPKey = SetFilePointer(hFileWholeReg, 0, NULL, FILE_CURRENT);

    // Write position of current key in caller's field
    if (0 < nFPCaller) {
        SetFilePointer(hFileWholeReg, nFPCaller, NULL, FILE_BEGIN);
        WriteFile(hFileWholeReg, &nFPKey, sizeof(nFPKey), &NBW, NULL);

        SetFilePointer(hFileWholeReg, nFPKey, NULL, FILE_BEGIN);
    }

    // Initialize key content
    ZeroMemory(&sKC, sizeof(sKC));

    // Set file positions of the relatives inside the tree
    sKC.ofsKeyName = 0;      // not known yet, may be re-written in this call
    sKC.ofsFirstValue = 0;   // not known yet, may be re-written in this call
    sKC.ofsFirstSubKey = 0;  // not known yet, may be re-written by another recursive call
    sKC.ofsBrotherKey = 0;   // not known yet, may be re-written by another recursive call
    sKC.ofsFatherKey = nFPFatherKey;

    // New since key content version 2
    sKC.nKeyNameLen = 0;
    if (NULL != lpKC->lpKeyName) {
        sKC.nKeyNameLen = (DWORD)_tcslen(lpKC->lpKeyName);
#ifdef _UNICODE
        sKC.nKeyNameLen++;  // account for NULL char
        // Key name will always be stored behind the structure, so its position is already known
        sKC.ofsKeyName = nFPKey + sizeof(sKC);
#endif
    }
#ifndef _UNICODE
    sKC.nKeyNameLen++;  // account for NULL char
    // Key name will always be stored behind the structure, so its position is already known
    sKC.ofsKeyName = nFPKey + sizeof(sKC);
#endif

    // Write key content to file
    // Make sure that ALL fields have been initialized/set
    WriteFile(hFileWholeReg, &sKC, sizeof(sKC), &NBW, NULL);

    // Write key name to file
    if (NULL != lpKC->lpKeyName) {
        WriteFile(hFileWholeReg, lpKC->lpKeyName, sKC.nKeyNameLen * sizeof(TCHAR), &NBW, NULL);
#ifndef _UNICODE
    } else {
        // Write empty string for backward compatibility
        WriteFile(hFileWholeReg, szEmpty, sKC.nKeyNameLen * sizeof(TCHAR), &NBW, NULL);
#endif
    }

    // Save the values of current key
    if (NULL != lpKC->lpFirstVC) {
        LPVALUECONTENT lpVC;
        DWORD nFPValueCaller;
        DWORD nFPValue;

        // Write all values of key
        nFPValueCaller = nFPKey + offsetof(SAVEKEYCONTENT, ofsFirstValue);  // Write position of first value into key
        for (lpVC = lpKC->lpFirstVC; NULL != lpVC; lpVC = lpVC->lpBrotherVC) {
            nFPValue = SetFilePointer(hFileWholeReg, 0, NULL, FILE_CURRENT);

            // Write position of previous value content in value content field ofsBrotherValue
            if (0 < nFPValueCaller) {
                SetFilePointer(hFileWholeReg, nFPValueCaller, NULL, FILE_BEGIN);
                WriteFile(hFileWholeReg, &nFPValue, sizeof(nFPValue), &NBW, NULL);

                SetFilePointer(hFileWholeReg, nFPValue, NULL, FILE_BEGIN);
            }
            nFPValueCaller = nFPValue + offsetof(SAVEVALUECONTENT, ofsBrotherValue);

            // Initialize key content
            ZeroMemory(&sVC, sizeof(sVC));

            // Copy values
            sVC.typecode = lpVC->typecode;
            sVC.datasize = lpVC->datasize;

            // Set file positions of the relatives inside the tree
            sVC.ofsValueData = 0;       // not known yet, may be re-written in this iteration
            sVC.ofsBrotherValue = 0;    // not known yet, may be re-written in next iteration
            sVC.ofsFatherKey = nFPKey;

            // New since value content version 2
            sVC.nValueNameLen = 0;
            if (NULL != lpVC->lpValueName) {
                sVC.nValueNameLen = (DWORD)_tcslen(lpVC->lpValueName);
#ifdef _UNICODE
                sVC.nValueNameLen++;  // account for NULL char
                // Value name will always be stored behind the structure, so its position is already known
                sVC.ofsValueName = nFPValue + sizeof(sVC);
#endif
            }
#ifndef _UNICODE
            sVC.nValueNameLen++;  // account for NULL char
            // Value name will always be stored behind the structure, so its position is already known
            sVC.ofsValueName = nFPValue + sizeof(sVC);
#endif

            // Write value content to file
            // Make sure that ALL fields have been initialized/set
            WriteFile(hFileWholeReg, &sVC, sizeof(sVC), &NBW, NULL);

            // Write value name to file
            if (NULL != lpVC->lpValueName) {
                WriteFile(hFileWholeReg, lpVC->lpValueName, sVC.nValueNameLen * sizeof(TCHAR), &NBW, NULL);
#ifndef _UNICODE
            } else {
                // Write empty string for backward compatibility
                WriteFile(hFileWholeReg, szEmpty, sVC.nValueNameLen * sizeof(TCHAR), &NBW, NULL);
#endif
            }

            // Write value data to file
            if (0 < lpVC->datasize) {
                DWORD nFPValueData;

                // Write position of value data in value content field ofsValueData
                nFPValueData = SetFilePointer(hFileWholeReg, 0, NULL, FILE_CURRENT);

                SetFilePointer(hFileWholeReg, nFPValue + offsetof(SAVEVALUECONTENT, ofsValueData), NULL, FILE_BEGIN);
                WriteFile(hFileWholeReg, &nFPValueData, sizeof(nFPValueData), &NBW, NULL);

                SetFilePointer(hFileWholeReg, nFPValueData, NULL, FILE_BEGIN);

                // Write value data
                WriteFile(hFileWholeReg, lpVC->lpValueData, lpVC->datasize, &NBW, NULL);
            }
        }
    }

    // ATTENTION!!! sKC is INVALID from this point on, due to recursive calls

    // If the entry has childs, then do a recursive call for the first child
    // Pass this entry as father and "ofsFirstSubKey" position for storing the first child's position
    if (NULL != lpKC->lpFirstSubKC) {
        SaveRegKey(lpKC->lpFirstSubKC, nFPKey, nFPKey + offsetof(SAVEKEYCONTENT, ofsFirstSubKey));
    }

    // If the entry has a following brother, then do a recursive call for the following brother
    // Pass father as father and "ofsBrotherKey" position for storing the next brother's position
    if (NULL != lpKC->lpBrotherKC) {
        SaveRegKey(lpKC->lpBrotherKC, nFPFatherKey, nFPKey + offsetof(SAVEKEYCONTENT, ofsBrotherKey));
    }

    // TODO: Need to adjust progress bar para!!
    nSavingKey++;
    if (0 != nGettingKey) {
        if (nSavingKey % nGettingKey > nRegStep) {
            nSavingKey = 0;
            SendDlgItemMessage(hWnd, IDC_PBCOMPARE, PBM_STEPIT, (WPARAM)0, (LPARAM)0);
            UpdateWindow(hWnd);
            PeekMessage(&msg, hWnd, WM_ACTIVATE, WM_ACTIVATE, PM_REMOVE);
        }
    }
}

// ----------------------------------------------------------------------
// Save registry and files to HIVE file
// ----------------------------------------------------------------------
VOID SaveHive(LPREGSHOT lpShot)
{
    OPENFILENAME opfn;
    TCHAR filepath[MAX_PATH];
    DWORD nFPCurrent;

    // Check if there's anything to save
    if ((NULL == lpShot->lpHKLM) && (NULL == lpShot->lpHKU) && (NULL == lpShot->lpHF)) {
        return;  // leave silently
    }

    // Clear Save File Name result buffer
    ZeroMemory(filepath, sizeof(filepath));

    // Prepare Save File Name dialog
    ZeroMemory(&opfn, sizeof(opfn));
    opfn.lStructSize = sizeof(opfn);
    opfn.hwndOwner = hWnd;
    opfn.lpstrFilter = szFilter;
    opfn.lpstrFile = filepath;
    opfn.nMaxFile = MAX_PATH;  // incl. NULL character
    opfn.lpstrInitialDir = lpLastSaveDir;
    opfn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    opfn.lpstrDefExt = "hiv";

    // Display Save File Name dialog
    if (!GetSaveFileName(&opfn)) {
        return;  // leave silently
    }

    // Open file for writing
    hFileWholeReg = CreateFile(opfn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFileWholeReg) {
        ErrMsg(asLangTexts[iszTextErrorCreateFile].lpString);
        return;
    }

    // Setup GUI for saving...
    UI_BeforeClear();
    InitProgressBar();

    // Initialize file header
    ZeroMemory(&fileheader, sizeof(fileheader));

    // Copy SBCS signature to header (even in Unicode builds for backwards compatibility)
    strncpy(fileheader.signature, szRegshotFileSignature, MAX_SIGNATURE_LENGTH);

    // Set file positions of hives inside the file
    fileheader.ofsHKLM = 0;   // not known yet, may be empty
    fileheader.ofsHKU = 0;    // not known yet, may be empty
    fileheader.ofsHF = 0;  // not known yet, may be empty

    // Copy SBCS/MBCS strings to header (even in Unicode builds for backwards compatibility)
#ifdef _UNICODE
    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR, lpShot->computername, -1, fileheader.computername, OLD_COMPUTERNAMELEN, NULL, NULL);
    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR, lpShot->username, -1, fileheader.username, OLD_COMPUTERNAMELEN, NULL, NULL);
#else
    strncpy(fileheader.computername, lpShot->computername, OLD_COMPUTERNAMELEN);
    strncpy(fileheader.username, lpShot->username, OLD_COMPUTERNAMELEN);
#endif

    // Copy system time to header
    CopyMemory(&fileheader.systemtime, &lpShot->systemtime, sizeof(SYSTEMTIME));

    // new since header version 2
    fileheader.nFHSize = sizeof(fileheader);

    fileheader.nFHVersion = FILEHEADER_VERSION_CURRENT;
    fileheader.nCharSize = sizeof(TCHAR);

    fileheader.ofsComputerName = 0;  // not known yet, may be empty
    fileheader.nComputerNameLen = 0;
    if (NULL != lpShot->computername) {
        fileheader.nComputerNameLen = (DWORD)_tcslen(lpShot->computername);
        if (0 < fileheader.nComputerNameLen) {
            fileheader.nComputerNameLen++;  // account for NULL char
        }
    }

    fileheader.ofsUserName = 0;      // not known yet, may be empty
    fileheader.nUserNameLen = 0;
    if (NULL != lpShot->username) {
        fileheader.nUserNameLen = (DWORD)_tcslen(lpShot->username);
        if (0 < fileheader.nUserNameLen) {
            fileheader.nUserNameLen++;  // account for NULL char
        }
    }

    fileheader.nKCVersion = KEYCONTENT_VERSION_CURRENT;
    fileheader.nKCSize = sizeof(SAVEKEYCONTENT);

    fileheader.nVCVersion = VALUECONTENT_VERSION_CURRENT;
    fileheader.nVCSize = sizeof(SAVEVALUECONTENT);

    fileheader.nHFVersion = HEADFILE_VERSION_CURRENT;
    fileheader.nHFSize = sizeof(SAVEHEADFILE);

    fileheader.nFCVersion = FILECONTENT_VERSION_CURRENT;
    fileheader.nFCSize = sizeof(SAVEFILECONTENT);

    // Write header to file
    WriteFile(hFileWholeReg, &fileheader, sizeof(fileheader), &NBW, NULL);

    // new since header version 2
    // (v2) full computername
    if (0 < fileheader.nComputerNameLen) {
        // Write position in file header
        nFPCurrent = SetFilePointer(hFileWholeReg, 0, NULL, FILE_CURRENT);

        SetFilePointer(hFileWholeReg, offsetof(FILEHEADER, ofsComputerName), NULL, FILE_BEGIN);
        WriteFile(hFileWholeReg, &nFPCurrent, sizeof(nFPCurrent), &NBW, NULL);
        fileheader.ofsComputerName = nFPCurrent;  // keep track in memory too

        SetFilePointer(hFileWholeReg, nFPCurrent, NULL, FILE_BEGIN);

        // Write computername
        WriteFile(hFileWholeReg, lpShot->computername, fileheader.nComputerNameLen * sizeof(TCHAR), &NBW, NULL);
    }

    // (v2) full username
    if (0 < fileheader.nUserNameLen) {
        // Write position in file header
        nFPCurrent = SetFilePointer(hFileWholeReg, 0, NULL, FILE_CURRENT);

        SetFilePointer(hFileWholeReg, offsetof(FILEHEADER, ofsUserName), NULL, FILE_BEGIN);
        WriteFile(hFileWholeReg, &nFPCurrent, sizeof(nFPCurrent), &NBW, NULL);
        fileheader.ofsUserName = nFPCurrent;  // keep track in memory too

        SetFilePointer(hFileWholeReg, nFPCurrent, NULL, FILE_BEGIN);

        // Write username
        WriteFile(hFileWholeReg, lpShot->username, fileheader.nUserNameLen * sizeof(TCHAR), &NBW, NULL);
    }

    // Save HKLM
    if (NULL != lpShot->lpHKLM) {  // should always be present
        SaveRegKey(lpShot->lpHKLM, 0, offsetof(FILEHEADER, ofsHKLM));
    }

    // Save HKU
    if (NULL != lpShot->lpHKU) {  // should always be present
        SaveRegKey(lpShot->lpHKU, 0, offsetof(FILEHEADER, ofsHKU));
    }

    // Save HEADFILEs
    if (NULL != lpShot->lpHF) {
        SaveHeadFile(lpShot->lpHF, offsetof(FILEHEADER, ofsHF));
    }

    // Close file
    CloseHandle(hFileWholeReg);

    ShowWindow(GetDlgItem(hWnd, IDC_PBCOMPARE), SW_HIDE);
    SetCursor(hSaveCursor);
    MessageBeep(0xffffffff);

    // overwrite first letter of file name with NULL character to get path only, then create backup for initialization on next call
    *(opfn.lpstrFile + opfn.nFileOffset) = 0x00;
    strcpy(lpLastSaveDir, opfn.lpstrFile);
}


// ----------------------------------------------------------------------
//
// ----------------------------------------------------------------------
size_t AdjustBuffer(LPVOID *lpBuffer, size_t nCurrentSize, size_t nWantedSize, size_t nAlign)
{
    if (NULL == *lpBuffer) {
        nCurrentSize = 0;
    }

    if (nWantedSize > nCurrentSize) {
        if (NULL != *lpBuffer) {
            MYFREE(*lpBuffer);
            *lpBuffer = NULL;
        }

        if (1 >= nAlign) {
            nCurrentSize = nWantedSize;
        } else {
            nCurrentSize = nWantedSize / nAlign;
            nCurrentSize *= nAlign;
            if (nWantedSize > nCurrentSize) {
                nCurrentSize +=  nAlign;
            }
        }

        *lpBuffer = MYALLOC0(nCurrentSize);
    }

    return nCurrentSize;
}

// ----------------------------------------------------------------------
// Load registry key with values from HIVE file
// ----------------------------------------------------------------------
VOID LoadRegKey(DWORD ofsKeyContent, LPKEYCONTENT lpFatherKC, LPKEYCONTENT *lplpCaller)
{
    LPKEYCONTENT lpKC;
    DWORD ofsFirstSubKey;
    DWORD ofsBrotherKey;

    // Copy SAVEKEYCONTENT to aligned memory block
    ZeroMemory(&sKC, sizeof(sKC));
    CopyMemory(&sKC, (lpFileBuffer + ofsKeyContent), fileheader.nKCSize);

    // Create new key content
    // put in a separate var for later use
    lpKC = MYALLOC0(sizeof(KEYCONTENT));
    ZeroMemory(lpKC, sizeof(KEYCONTENT));

    // Write pointer to current key into caller's pointer
    if (NULL != lplpCaller) {
        *lplpCaller = lpKC;
    }

    // Set father of current key
    lpKC->lpFatherKC = lpFatherKC;

    // Copy key name
    if (KEYCONTENT_VERSION_2 > fileheader.nKCVersion) {
        sKC.nKeyNameLen = (DWORD)strlen((const char *)(lpFileBuffer + sKC.ofsKeyName));
        if (0 < sKC.nKeyNameLen) {
            sKC.nKeyNameLen++;  // account for NULL char
        }
    }
    if (0 < sKC.nKeyNameLen) {  // otherwise leave it NULL
        // Copy string to an aligned memory block
        nSourceSize = sKC.nKeyNameLen * fileheader.nCharSize;
        nStringBufferSize = AdjustBuffer(&lpStringBuffer, nStringBufferSize, nSourceSize, REGSHOT_BUFFER_BLOCK_BYTES);
        ZeroMemory(lpStringBuffer, nStringBufferSize);
        CopyMemory(lpStringBuffer, (lpFileBuffer + sKC.ofsKeyName), nSourceSize);

        lpKC->lpKeyName = MYALLOC0(sKC.nKeyNameLen * sizeof(TCHAR));
        if (sizeof(TCHAR) == fileheader.nCharSize) {
            _tcscpy(lpKC->lpKeyName, lpStringBuffer);
        } else {
#ifdef _UNICODE
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpStringBuffer, -1, lpKC->lpKeyName, sKC.nKeyNameLen);
#else
            WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR, (LPCWSTR)lpStringBuffer, -1, lpKC->lpKeyName, sKC.nKeyNameLen, NULL, NULL);
#endif
        }
    }

    nGettingKey++;

    // Copy the value contents of the current key
    if (0 != sKC.ofsFirstValue) {
        LPVALUECONTENT lpVC;
        LPVALUECONTENT *lplpVCPrev;
        DWORD ofsValueContent;

        lplpVCPrev = &lpKC->lpFirstVC;
        for (ofsValueContent = sKC.ofsFirstValue; 0 != ofsValueContent; ofsValueContent = sVC.ofsBrotherValue) {
            // Copy SAVEVALUECONTENT to aligned memory block
            ZeroMemory(&sVC, sizeof(sVC));
            CopyMemory(&sVC, (lpFileBuffer + ofsValueContent), fileheader.nVCSize);

            // Create new value content
            lpVC = MYALLOC0(sizeof(VALUECONTENT));
            ZeroMemory(lpVC, sizeof(VALUECONTENT));

            // Write pointer to current value into previous value's next value pointer
            if (NULL != lplpVCPrev) {
                *lplpVCPrev = lpVC;
            }
            lplpVCPrev = &lpVC->lpBrotherVC;

            // Set father key to current key
            lpVC->lpFatherKC = lpKC;

            // Copy values
            lpVC->typecode = sVC.typecode;
            lpVC->datasize = sVC.datasize;

            // Copy value name
            if (VALUECONTENT_VERSION_2 > fileheader.nVCVersion) {
                sVC.nValueNameLen = (DWORD)strlen((const char *)(lpFileBuffer + sVC.ofsValueName));
                if (0 < sVC.nValueNameLen) {
                    sVC.nValueNameLen++;  // account for NULL char
                }
            }
            if (0 < sVC.nValueNameLen) {  // otherwise leave it NULL
                // Copy string to an aligned memory block
                nSourceSize = sVC.nValueNameLen * fileheader.nCharSize;
                nStringBufferSize = AdjustBuffer(&lpStringBuffer, nStringBufferSize, nSourceSize, REGSHOT_BUFFER_BLOCK_BYTES);
                ZeroMemory(lpStringBuffer, nStringBufferSize);
                CopyMemory(lpStringBuffer, (lpFileBuffer + sVC.ofsValueName), nSourceSize);

                lpVC->lpValueName = MYALLOC0(sVC.nValueNameLen * sizeof(TCHAR));
                if (sizeof(TCHAR) == fileheader.nCharSize) {
                    _tcscpy(lpVC->lpValueName, lpStringBuffer);
                } else {
#ifdef _UNICODE
                    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpStringBuffer, -1, lpVC->lpValueName, sVC.nValueNameLen);
#else
                    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR, (LPCWSTR)lpStringBuffer, -1, lpVC->lpValueName, sVC.nValueNameLen, NULL, NULL);
#endif
                }
            }

            // Copy value data
            if (0 < sVC.datasize) {  // otherwise leave it NULL
                lpVC->lpValueData = MYALLOC0(sVC.datasize);
                CopyMemory(lpVC->lpValueData, (lpFileBuffer + sVC.ofsValueData), sVC.datasize);
            }

            nGettingValue++;
        }
    }

    ofsFirstSubKey = sKC.ofsFirstSubKey;
    ofsBrotherKey = sKC.ofsBrotherKey;

    nGettingTime = GetTickCount();
    if (REFRESHINTERVAL < (nGettingTime - nBASETIME1)) {
        UpdateCounters(asLangTexts[iszTextKey].lpString, asLangTexts[iszTextValue].lpString, nGettingKey, nGettingValue);
    }

    // ATTENTION!!! sKC is INVALID from this point on, due to recursive calls

    // If the entry has childs, then do a recursive call for the first child
    // Pass this entry as father and "lpFirstSubKC" pointer for storing the first child's pointer
    if (0 != ofsFirstSubKey) {
        LoadRegKey(ofsFirstSubKey, lpKC, &lpKC->lpFirstSubKC);
    }

    // If the entry has a following brother, then do a recursive call for the following brother
    // Pass father as father and "lpBrotherKC" pointer for storing the next brother's pointer
    if (0 != ofsBrotherKey) {
        LoadRegKey(ofsBrotherKey, lpFatherKC, &lpKC->lpBrotherKC);
    }
}

// ----------------------------------------------------------------------
// Load registry and files from HIVE file
// ----------------------------------------------------------------------
BOOL LoadHive(LPREGSHOT lpShot)
{
    OPENFILENAME opfn;
    TCHAR filepath[MAX_PATH];

    DWORD nFileSize;
    DWORD i, j;
    DWORD nRemain;
    DWORD nReadSize;

    // Clear Get File Name result buffer
    ZeroMemory(filepath, sizeof(filepath));

    // Prepare Open File Name dialog
    ZeroMemory(&opfn, sizeof(opfn));
    opfn.lStructSize = sizeof(opfn);
    opfn.hwndOwner = hWnd;
    opfn.lpstrFilter = szFilter;
    opfn.lpstrFile = filepath;
    opfn.nMaxFile = MAX_PATH;  // incl. NULL character
    opfn.lpstrInitialDir = lpLastOpenDir;
    opfn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    opfn.lpstrDefExt = szRegshotFileDefExt;

    // Display Open File Name dialog
    if (!GetOpenFileName(&opfn)) {
        return FALSE;
    }

    // Open file for reading
    hFileWholeReg = CreateFile(opfn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFileWholeReg) {
        ErrMsg(asLangTexts[iszTextErrorOpenFile].lpString);
        return FALSE;
    }

    nFileSize = GetFileSize(hFileWholeReg, NULL);
    if (sizeof(fileheader) > nFileSize) {
        CloseHandle(hFileWholeReg);
        ErrMsg(TEXT("wrong filesize"));
        return FALSE;
    }

    // Initialize file header
    ZeroMemory(&fileheader, sizeof(fileheader));

    // Read first part of file header from file (signature, nHeaderSize)
    ReadFile(hFileWholeReg, &fileheader, offsetof(FILEHEADER, ofsHKLM), &NBW, NULL);

    // Check file signature
    if ((0 != strncmp(szRegshotFileSignatureSBCS, fileheader.signature, MAX_SIGNATURE_LENGTH)) && (0 != strncmp(szRegshotFileSignatureUTF16, fileheader.signature, MAX_SIGNATURE_LENGTH))) {
        CloseHandle(hFileWholeReg);
        ErrMsg(TEXT("It is not a valid Regshot hive file!"));
        return FALSE;
    }

    // Clear shot
    FreeShot(lpShot);

    // Setup GUI for loading...
    nGettingKey   = 0;
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

    // Allocate memory to hold the complete file
    lpFileBuffer = MYALLOC(nFileSize);

    // Read file blockwise for progress bar
    InitProgressBar();
    nFileStep = nFileSize / REGSHOT_READ_BLOCK_SIZE / MAXPBPOSITION;  // TODO: does look wrong!?! PBM_SETSTEP message was in InitProgressBar()

    SetFilePointer(hFileWholeReg, 0, NULL, FILE_BEGIN);
    nRemain = nFileSize;  // 100% to go
    nReadSize = REGSHOT_READ_BLOCK_SIZE;  // next block length to read
    for (i = 0, j = 0; nRemain > 0; i += nReadSize, j++) {
        // If the rest is smaller than a block, then use the rest length
        if (REGSHOT_READ_BLOCK_SIZE > nRemain) {
            nReadSize = nRemain;
        }

        // Read the next block
        ReadFile(hFileWholeReg, lpFileBuffer + i, nReadSize, &NBW, NULL);
        if (NBW != nReadSize) {
            CloseHandle(hFileWholeReg);
            ErrMsg(TEXT("Reading ERROR!"));
            return FALSE;
        }

        // Determine how much to go, if zero leave the for loop
        nRemain -= nReadSize;
        if (0 == nRemain) {
            break;
        }

        // Handle progress bar
        if (j % (nFileSize / REGSHOT_READ_BLOCK_SIZE) > nFileStep) {  // TODO: does look wrong!?!
            j = 0;
            SendDlgItemMessage(hWnd, IDC_PBCOMPARE, PBM_STEPIT, (WPARAM)0, (LPARAM)0);
            UpdateWindow(hWnd);
            PeekMessage(&msg, hWnd, WM_ACTIVATE, WM_ACTIVATE, PM_REMOVE);
        }
    }

    CloseHandle(hFileWholeReg);

    ShowWindow(GetDlgItem(hWnd, IDC_PBCOMPARE), SW_HIDE);

    // Check size for copying file header
    nSourceSize = fileheader.nFHSize;
    if (0 == nSourceSize) {
        nSourceSize = offsetof(FILEHEADER, nFHVersion);
    } else if (sizeof(fileheader) < nSourceSize) {
        nSourceSize = sizeof(fileheader);
    }

    // Copy file header to structure
    CopyMemory(&fileheader, lpFileBuffer, nSourceSize);

    // Enhance data of old headers to be used with newer code
    if (FILEHEADER_VERSION_EMPTY == fileheader.nFHVersion) {
        if ((0 != fileheader.ofsHKLM) && (fileheader.ofsHKU == fileheader.ofsHKLM)) {
            fileheader.ofsHKLM = 0;
        }
        if ((0 != fileheader.ofsHKU) && (fileheader.ofsHF == fileheader.ofsHKU)) {
            fileheader.ofsHKU = 0;
        }

        fileheader.nFHVersion = FILEHEADER_VERSION_1;
        fileheader.nCharSize = 1;
        fileheader.nKCVersion = KEYCONTENT_VERSION_1;
        fileheader.nKCSize = offsetof(SAVEKEYCONTENT, nKeyNameLen);
        fileheader.nVCVersion = VALUECONTENT_VERSION_1;
        fileheader.nVCSize = offsetof(SAVEVALUECONTENT, nValueNameLen);
        fileheader.nHFVersion = HEADFILE_VERSION_1;
        fileheader.nHFSize = sizeof(SAVEHEADFILE);  // not changed yet, if it is then adopt to offsetof(SAVEHEADFILE, <first new field>)
        fileheader.nFCVersion = FILECONTENT_VERSION_1;
        fileheader.nFCSize = offsetof(SAVEFILECONTENT, nFileNameLen);
    }

    // Check for compatible char size
    if (sizeof(TCHAR) != fileheader.nCharSize) {
        if (2 < fileheader.nCharSize) {  // known: 1 = SBCS/MBCS, 2 = UTF-16
            ErrMsg(TEXT("Unsupported character size!"));
            return FALSE;
        }
    }

    // Check structure boundaries
    if (sizeof(SAVEKEYCONTENT) < fileheader.nKCSize) {
        fileheader.nKCSize = sizeof(SAVEKEYCONTENT);
    }
    if (sizeof(SAVEVALUECONTENT) < fileheader.nVCSize) {
        fileheader.nVCSize = sizeof(SAVEVALUECONTENT);
    }
    if (sizeof(SAVEHEADFILE) < fileheader.nHFSize) {
        fileheader.nHFSize = sizeof(SAVEHEADFILE);
    }
    if (sizeof(SAVEFILECONTENT) < fileheader.nFCSize) {
        fileheader.nFCSize = sizeof(SAVEFILECONTENT);
    }

    // ^^^ here the file header can be checked for additional extended content
    // * remember that files from older versions do not provide these additional data

    // New temporary string buffer
    lpStringBuffer = NULL;

    // Copy computer name from file header to shot data
    if (FILEHEADER_VERSION_2 <= fileheader.nFHVersion) {
        if (0 < fileheader.nComputerNameLen) {  // otherwise leave it NULL
            // Copy string to an aligned memory block
            nSourceSize = fileheader.nComputerNameLen * fileheader.nCharSize;
            nStringBufferSize = AdjustBuffer(&lpStringBuffer, nStringBufferSize, nSourceSize, REGSHOT_BUFFER_BLOCK_BYTES);
            ZeroMemory(lpStringBuffer, nStringBufferSize);
            CopyMemory(lpStringBuffer, (lpFileBuffer + fileheader.ofsComputerName), nSourceSize);

            lpShot->computername = MYALLOC0(fileheader.nComputerNameLen * sizeof(TCHAR));
            if (sizeof(TCHAR) == fileheader.nCharSize) {
                _tcscpy(lpShot->computername, lpStringBuffer);
            } else {
#ifdef _UNICODE
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpStringBuffer, -1, lpShot->computername, fileheader.nComputerNameLen);
#else
                WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR, (LPCWSTR)lpStringBuffer, -1, lpShot->computername, fileheader.nComputerNameLen, NULL, NULL);
#endif
            }
        }
    } else {
        // Copy string to an aligned memory block
        nSourceSize = strnlen((const char *)&fileheader.computername, OLD_COMPUTERNAMELEN);
        if (0 < nSourceSize) {  // otherwise leave it NULL
            nStringBufferSize = AdjustBuffer(&lpStringBuffer, nStringBufferSize, (nSourceSize + 1), REGSHOT_BUFFER_BLOCK_BYTES);
            ZeroMemory(lpStringBuffer, nStringBufferSize);
            CopyMemory(lpStringBuffer, &fileheader.computername, nSourceSize);

            lpShot->computername = MYALLOC0((nSourceSize + 1) * sizeof(TCHAR));
#ifdef _UNICODE
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpStringBuffer, (nSourceSize + 1), lpShot->computername, (nSourceSize + 1));
#else
            strncpy(lpShot->computername, lpStringBuffer, nSourceSize);
#endif
            lpShot->computername[nSourceSize] = 0;
        }
    }

    // Copy user name from file header to shot data
    if (FILEHEADER_VERSION_2 <= fileheader.nFHVersion) {
        if (0 < fileheader.nUserNameLen) {  // otherwise leave it NULL
            // Copy string to an aligned memory block
            nSourceSize = fileheader.nUserNameLen * fileheader.nCharSize;
            nStringBufferSize = AdjustBuffer(&lpStringBuffer, nStringBufferSize, nSourceSize, REGSHOT_BUFFER_BLOCK_BYTES);
            ZeroMemory(lpStringBuffer, nStringBufferSize);
            CopyMemory(lpStringBuffer, (lpFileBuffer + fileheader.ofsUserName), nSourceSize);

            lpShot->username = MYALLOC0(fileheader.nUserNameLen * sizeof(TCHAR));
            if (sizeof(TCHAR) == fileheader.nCharSize) {
                _tcscpy(lpShot->username, lpStringBuffer);
            } else {
#ifdef _UNICODE
                MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpStringBuffer, -1, lpShot->username, fileheader.nUserNameLen);
#else
                WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK | WC_DEFAULTCHAR, (LPCWSTR)lpStringBuffer, -1, lpShot->username, fileheader.nUserNameLen, NULL, NULL);
#endif
            }
        }
    } else {
        // Copy string to an aligned memory block
        nSourceSize = strnlen((const char *)&fileheader.username, OLD_COMPUTERNAMELEN);
        if (0 < nSourceSize) {  // otherwise leave it NULL
            nStringBufferSize = AdjustBuffer(&lpStringBuffer, nStringBufferSize, (nSourceSize + 1), REGSHOT_BUFFER_BLOCK_BYTES);
            ZeroMemory(lpStringBuffer, nStringBufferSize);
            CopyMemory(lpStringBuffer, &fileheader.username, nSourceSize);

            lpShot->username = MYALLOC0((nSourceSize + 1) * sizeof(TCHAR));
#ifdef _UNICODE
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (LPCSTR)lpStringBuffer, (nSourceSize + 1), lpShot->username, (nSourceSize + 1));
#else
            strncpy(lpShot->username, lpStringBuffer, nSourceSize);
#endif
            lpShot->username[nSourceSize] = 0;
        }
    }

    CopyMemory(&lpShot->systemtime, &fileheader.systemtime, sizeof(SYSTEMTIME));

    if (0 != fileheader.ofsHKLM) {
        LoadRegKey(fileheader.ofsHKLM, NULL, &lpShot->lpHKLM);
    }

    if (0 != fileheader.ofsHKU) {
        LoadRegKey(fileheader.ofsHKU, NULL, &lpShot->lpHKU);
    }

    if (0 != fileheader.ofsHF) {
        LoadHeadFile(fileheader.ofsHF, &lpShot->lpHF);
    }

    // Setup GUI for loading...
    if (NULL != lpShot->lpHF) {
        SendMessage(GetDlgItem(hWnd, IDC_CHECKDIR), BM_SETCHECK, (WPARAM)BST_CHECKED, (LPARAM)0);
        SendMessage(hWnd, WM_COMMAND, (WPARAM)IDC_CHECKDIR, (LPARAM)0);

        FindDirChain(lpShot->lpHF, lpExtDir, EXTDIRLEN);  // Get new chains, must do this after loading!
        SetDlgItemText(hWnd, IDC_EDITDIR, lpExtDir);
    } else {
        SetDlgItemText(hWnd, IDC_EDITDIR, TEXT(""));
    }

    if (NULL != lpStringBuffer) {
        MYFREE(lpStringBuffer);
        lpStringBuffer = NULL;
    }

    if (NULL != lpFileBuffer) {
        MYFREE(lpFileBuffer);
        lpFileBuffer = NULL;
    }

    UI_AfterShot();

    // overwrite first letter of file name with NULL character to get path only, then create backup for initialization on next call
    *(opfn.lpstrFile + opfn.nFileOffset) = 0x00;
    strcpy(lpLastOpenDir, opfn.lpstrFile);

    return TRUE;
}
