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

char str_DefResPre[]="~res";
char str_filter[]= {"Regshot hive files [*.hiv]\0*.hiv\0All files\0*.*\0\0"};
char str_RegFileSignature[]="REGSHOTHIVE"; //Need [] to use with sizeof()


extern u_char * lan_errorcreatefile;
extern u_char * lan_comments;
extern u_char * lan_datetime;
extern u_char * lan_computer;
extern u_char * lan_username;
extern u_char * lan_keydel;
extern u_char * lan_keyadd;
extern u_char * lan_valdel;
extern u_char * lan_valadd;
extern u_char * lan_valmodi;
extern u_char * lan_filedel;
extern u_char * lan_fileadd;
extern u_char * lan_filemodi;
extern u_char * lan_diradd;
extern u_char * lan_dirdel;
extern u_char * lan_dirmodi;
extern u_char * lan_total;
extern u_char * lan_key;
extern u_char * lan_value;
extern u_char * lan_errorexecviewer;
extern u_char * lan_erroropenfile;

extern char *str_prgname; // be careful of extern ref! must be the same when declare them,otherwise pointer would mis-point!
extern char	str_CR[];


//-------------------------------------------------------------
//Routine to Get Whole Key Name from KEYCONTENT
//-------------------------------------------------------------
LPSTR	GetWholeKeyName(LPKEYCONTENT lpKeyContent)
{
	LPKEYCONTENT lpf;
	LPSTR	lpName,lptail;
	int nLen=0;

	for(lpf=lpKeyContent; lpf!=NULL; lpf=lpf->lpfatherkey) {
		nLen+=strlen(lpf->lpkeyname)+1;
	}
	if(nLen==0) {
		nLen++;
	}
	lpName=MYALLOC(nLen);

	lptail=lpName+nLen-1;
	*lptail=0x00;

	for(lpf=lpKeyContent; lpf!=NULL; lpf=lpf->lpfatherkey) {
		nLen=strlen(lpf->lpkeyname);
		memcpy(lptail-=nLen,lpf->lpkeyname,nLen);
		if (lptail>lpName) {
			*--lptail='\\';    //0x5c='\\'
		}
	}
	return lpName;
}


//-------------------------------------------------------------
//Routine to Get Whole VALUE Name from VALUECONTENT
//-------------------------------------------------------------
LPSTR	GetWholeValueName(LPVALUECONTENT lpValueContent)
{
	LPKEYCONTENT lpf;
	int nWholeLen,nLen;
	LPSTR	lpName,lptail;
	nLen=strlen(lpValueContent->lpvaluename);
	nWholeLen=nLen+1;
	for(lpf=lpValueContent->lpfatherkey; lpf!=NULL; lpf=lpf->lpfatherkey) {
		nWholeLen+=strlen(lpf->lpkeyname)+1;
	}

	lpName=MYALLOC(nWholeLen);
	lptail=lpName+nWholeLen-1;

	strcpy(lptail-=nLen,lpValueContent->lpvaluename);
	*--lptail='\\'; //0x5c='\\'

	for(lpf=lpValueContent->lpfatherkey; lpf!=NULL; lpf=lpf->lpfatherkey) {
		nLen=strlen(lpf->lpkeyname);
		memcpy(lptail-=nLen,lpf->lpkeyname,nLen);
		if (lptail>lpName) {
			*--lptail='\\';    //0x5c= '\\'
		}
	}
	return lpName;
}


//-------------------------------------------------------------
//Routine Trans VALUECONTENT.data[which in binary] into strings
//-------------------------------------------------------------
LPSTR	TransData(LPVALUECONTENT lpValueContent,DWORD type)
{
	LPSTR	lpvaluedata=NULL;
	DWORD	c,size=lpValueContent->datasize;
	switch(type) {

		case	REG_SZ:
			//case	REG_EXPAND_SZ: Not used any more,they all included in [default],because some nonregular value would corrupt this.
			lpvaluedata=MYALLOC0(size+5); //5 is enough
			strcpy(lpvaluedata,": \"");
			if(lpValueContent->lpvaluedata!=NULL) { //added in 1.62? not compiled
				strcat(lpvaluedata,lpValueContent->lpvaluedata);
			}
			strcat(lpvaluedata,"\"");
			//wsprintf has a bug that can not print string too long one time!);
			//wsprintf(lpvaluedata,"%s%s%s",": \"",lpValueContent->lpvaluedata,"\"");
			break;
		case	REG_MULTI_SZ:
			//Be sure to add below line outside of following "if",
			//for that GlobalFree(lp) must had lp already located!
			lpvaluedata=MYALLOC0(size+5);//5 is enough
			for(c=0; c<size; c++) {
				if (*((LPBYTE)(lpValueContent->lpvaluedata+c))==0) {
					if (*((LPBYTE)(lpValueContent->lpvaluedata+c+1))!=0) {
						*((LPBYTE)(lpValueContent->lpvaluedata+c))=0x20;    ////???????
					} else {
						break;
					}
				}
			}
			//*((LPBYTE)(lpValueContent->lpvaluedata+size))=0x00; // for some illegal multisz
			strcpy(lpvaluedata,": '");
			strcat(lpvaluedata,lpValueContent->lpvaluedata);
			strcat(lpvaluedata,"'");
			//wsprintf(lpvaluedata,"%s%s%s",": \"",lpValueContent->lpvaluedata,"\"");
			break;
		case	REG_DWORD:
			//case	REG_DWORD_BIG_ENDIAN: Not used any more,they all included in [default]
			lpvaluedata=MYALLOC0(13); //13 is enough
			sprintf(lpvaluedata,"%s%08X",": 0x",*(LPDWORD)(lpValueContent->lpvaluedata));
			break;
		default	:
			lpvaluedata=MYALLOC0(3*(size+1)); //3*(size+1) is enough
			*lpvaluedata=0x3a;
			//for the resttype lengthofvaluedata doesn't contains the 0!
			for	(c=0; c<size; c++) {
				sprintf(lpvaluedata+3*c+1," %02X",*(lpValueContent->lpvaluedata+c));
			}
	}
	return lpvaluedata;
}


//-------------------------------------------------------------
//Routine to Get Whole Value Data from VALUECONTENT
//-------------------------------------------------------------
LPSTR	GetWholeValueData(LPVALUECONTENT lpValueContent)
{
	LPSTR	lpvaluedata=NULL;
	DWORD	c,size=lpValueContent->datasize;
	switch(lpValueContent->typecode) {

		case	REG_SZ:
		case	REG_EXPAND_SZ:
			if(lpValueContent->lpvaluedata!=NULL) { //added in 1.62 not compiled
				if(size==(DWORD)strlen(lpValueContent->lpvaluedata)+1) {
					lpvaluedata=TransData(lpValueContent,REG_SZ);
				} else {
					lpvaluedata=TransData(lpValueContent,REG_BINARY);
				}
			} else {
				lpvaluedata=TransData(lpValueContent,REG_SZ);
			}
			break;
		case	REG_MULTI_SZ:
			if(*((LPBYTE)(lpValueContent->lpvaluedata))!=0x00) {
				for(c=0;; c++) {
					if (*((LPWORD)(lpValueContent->lpvaluedata+c))==0) {
						break;
					}
				}
				if(size==c+2) {
					lpvaluedata=TransData(lpValueContent,REG_MULTI_SZ);
				} else {
					lpvaluedata=TransData(lpValueContent,REG_BINARY);
				}
			} else {
				lpvaluedata=TransData(lpValueContent,REG_BINARY);
			}
			break;
		case	REG_DWORD:
		case	REG_DWORD_BIG_ENDIAN:
			if(size==SIZEOFREG_DWORD) {
				lpvaluedata=TransData(lpValueContent,REG_DWORD);
			} else {
				lpvaluedata=TransData(lpValueContent,REG_BINARY);
			}
			break;
		default	:
			lpvaluedata=TransData(lpValueContent,REG_BINARY);
	}
	return lpvaluedata;
}


//-------------------------------------------------------------
//Routine to create new compare result,distribute to different lp???MODI
//-------------------------------------------------------------
VOID	CreateNewResult(DWORD actiontype,LPDWORD lpcount,LPSTR lpresult)
{
	LPCOMRESULT	lpnew;
	lpnew=(LPCOMRESULT)MYALLOC0(sizeof(COMRESULT));
	lpnew->lpresult=lpresult;

	switch(actiontype) {
		case KEYADD:
			*lpcount==0 ? (lpKEYADDHEAD=lpnew):(lpKEYADD->lpnextresult=lpnew);
			lpKEYADD=lpnew;
			break;
		case KEYDEL:
			*lpcount==0 ? (lpKEYDELHEAD=lpnew):(lpKEYDEL->lpnextresult=lpnew);
			lpKEYDEL=lpnew;
			break;
		case VALADD:
			*lpcount==0 ? (lpVALADDHEAD=lpnew):(lpVALADD->lpnextresult=lpnew);
			lpVALADD=lpnew;
			break;
		case VALDEL:
			*lpcount==0 ? (lpVALDELHEAD=lpnew):(lpVALDEL->lpnextresult=lpnew);
			lpVALDEL=lpnew;
			break;
		case VALMODI:
			*lpcount==0 ? (lpVALMODIHEAD=lpnew):(lpVALMODI->lpnextresult=lpnew);
			lpVALMODI=lpnew;
			break;
		case FILEADD:
			*lpcount==0 ? (lpFILEADDHEAD=lpnew):(lpFILEADD->lpnextresult=lpnew);
			lpFILEADD=lpnew;
			break;
		case FILEDEL:
			*lpcount==0 ? (lpFILEDELHEAD=lpnew):(lpFILEDEL->lpnextresult=lpnew);
			lpFILEDEL=lpnew;
			break;
		case FILEMODI:
			*lpcount==0 ? (lpFILEMODIHEAD=lpnew):(lpFILEMODI->lpnextresult=lpnew);
			lpFILEMODI=lpnew;
			break;
		case DIRADD:
			*lpcount==0 ? (lpDIRADDHEAD=lpnew):(lpDIRADD->lpnextresult=lpnew);
			lpDIRADD=lpnew;
			break;
		case DIRDEL:
			*lpcount==0 ? (lpDIRDELHEAD=lpnew):(lpDIRDEL->lpnextresult=lpnew);
			lpDIRDEL=lpnew;
			break;
		case DIRMODI:
			*lpcount==0 ? (lpDIRMODIHEAD=lpnew):(lpDIRMODI->lpnextresult=lpnew);
			lpDIRMODI=lpnew;
			break;

	}
	(*lpcount)++;
}


//-------------------------------------------------------------
//Write compare results into memory and call CreateNewResult()
//-------------------------------------------------------------
VOID	LogToMem(DWORD actiontype,LPDWORD lpcount,LPVOID lp)
{
	LPSTR	lpname,lpdata,lpall;
	if(actiontype==KEYADD||actiontype==KEYDEL) {
		lpname=GetWholeKeyName(lp);
		CreateNewResult(actiontype,lpcount,lpname);
	} else {
		if(actiontype==VALADD||actiontype==VALDEL||actiontype==VALMODI) {

			lpname=GetWholeValueName(lp);
			lpdata=GetWholeValueData(lp);
			lpall=MYALLOC(strlen(lpname)+strlen(lpdata)+2);
			//do not use:wsprintf(lpall,"%s%s",lpname,lpdata); !!! strlen limit!
			strcpy(lpall,lpname);
			strcat(lpall,lpdata);
			MYFREE(lpname);
			MYFREE(lpdata);
			CreateNewResult(actiontype,lpcount,lpall);
		} else {
			lpname=GetWholeFileName(lp);
			CreateNewResult(actiontype,lpcount,lpname);
		}

	}
}


//-------------------------------------------------------------
//Routine to walk through sub keytree of current Key
//-------------------------------------------------------------
VOID	GetAllSubName(
	BOOL needbrother,
	DWORD typekey,DWORD typevalue,
	LPDWORD lpcountkey,LPDWORD lpcountvalue,
	LPKEYCONTENT lpKeyContent
)
{

	LPVALUECONTENT lpv;
	LogToMem(typekey,lpcountkey,lpKeyContent);

	if(lpKeyContent->lpfirstsubkey!=NULL) {
		GetAllSubName(TRUE,typekey,typevalue,lpcountkey,lpcountvalue,lpKeyContent->lpfirstsubkey);
	}

	if(needbrother==TRUE)
		if(lpKeyContent->lpbrotherkey!=NULL) {
			GetAllSubName(TRUE,typekey,typevalue,lpcountkey,lpcountvalue,lpKeyContent->lpbrotherkey);
		}

	for(lpv=lpKeyContent->lpfirstvalue; lpv!=NULL; lpv=lpv->lpnextvalue) {
		LogToMem(typevalue,lpcountvalue,lpv);
	}
}


//-------------------------------------------------------------
//Routine to walk through all values of current key
//-------------------------------------------------------------
VOID	GetAllValue(DWORD typevalue,LPDWORD lpcountvalue,LPKEYCONTENT lpKeyContent)
{
	LPVALUECONTENT lpv;
	for(lpv=lpKeyContent->lpfirstvalue; lpv!=NULL; lpv=lpv->lpnextvalue) {
		LogToMem(typevalue,lpcountvalue,lpv);
	}
}


//-------------------------------------------------------------
//Routine to Free All Compare Results [Release Memory]
//-------------------------------------------------------------
VOID	FreeAllCom(LPCOMRESULT lpComResult)
{
	LPCOMRESULT lp,lpold;
	for(lp=lpComResult; lp!=NULL;) {
		if(lp->lpresult!=NULL) {
			MYFREE(lp->lpresult);
		}
		lpold=lp;
		lp=lp->lpnextresult;
		MYFREE(lpold);
	}

}


//-------------------------------------------------------------
//Routine to Free All Keys and Values
//-------------------------------------------------------------
VOID FreeAllKey(LPKEYCONTENT lpKey)
{
	LPVALUECONTENT lpv,lpvold;
	if(lpKey!=NULL) {
		FreeAllKey(lpKey->lpfirstsubkey);
		FreeAllKey(lpKey->lpbrotherkey);
		for(lpv=lpKey->lpfirstvalue; lpv!=NULL;) {
			MYFREE(lpv->lpvaluename);
			if(lpv->lpvaluedata!=NULL) {
				MYFREE(lpv->lpvaluedata);
			}
			lpvold=lpv;
			lpv=lpv->lpnextvalue;
			MYFREE(lpvold);
		}
		MYFREE(lpKey->lpkeyname);
		MYFREE(lpKey);
	}

}


//-------------------------------------------------------------
//Clear RegFlag previous made by Compare Routine for the next compare
//-------------------------------------------------------------
VOID ClearKeyMatchTag(LPKEYCONTENT lpKey)
{
	LPVALUECONTENT lpv;
	if(lpKey!=NULL) {
		lpKey->bkeymatch=0;
		for(lpv=lpKey->lpfirstvalue; lpv!=NULL; lpv=lpv->lpnextvalue) {
			lpv->bvaluematch=0;
		}

		//if(lpKey->lpfirstsubkey!=NULL) //notused in 1.8
		{
			ClearKeyMatchTag(lpKey->lpfirstsubkey);
		}

		//if(lpKey->lpbrotherkey!=NULL) //notused in 1.8
		{
			ClearKeyMatchTag(lpKey->lpbrotherkey);
		}
	}
}


//-------------------------------------------------------------
//Clear Filematch Flag (core)
//-------------------------------------------------------------
VOID ClearFileContentMatchTag(LPFILECONTENT lpFC)
{
	if(lpFC!=NULL) {
		lpFC->bfilematch=0;
		//if(lpFC->lpfirstsubfile!=NULL)
		ClearFileContentMatchTag(lpFC->lpfirstsubfile);
		//if(lpFC->lpbrotherfile!=NULL)
		ClearFileContentMatchTag(lpFC->lpbrotherfile);
	}
}


//-------------------------------------------------------------
//Clear Filematch Flag previous made by Compare Routine for the next compare
//-------------------------------------------------------------
VOID ClearHeadFileMatchTag(LPHEADFILE lpHF)
{
	LPHEADFILE lphf;
	for(lphf=lpHF; lphf!=NULL; lphf=lphf->lpnextheadfile) {
		ClearFileContentMatchTag(lphf->lpfilecontent);
	}
}

//////////////////////////////////////
VOID FreeAllKeyContent1(void)
{

	if(is1LoadFromHive) {
		MYFREE(lpTempHive1); //Note,together we free the filecontent!
		lpTempHive1=NULL;
		lpHeadFile1=NULL; //We add here
	} else {
		FreeAllKey(lpHeadLocalMachine1);
		FreeAllKey(lpHeadUsers1);
	}
	lpHeadLocalMachine1=NULL;
	lpHeadUsers1=NULL;
	*lpComputerName1=0;
	*lpUserName1=0;

}

VOID FreeAllKeyContent2(void)
{

	if(is2LoadFromHive) {
		MYFREE(lpTempHive2); //Note,together we free the filecontent!
		lpTempHive2=NULL;
		lpHeadFile2=NULL; //We add here!
	} else {
		FreeAllKey(lpHeadLocalMachine2);
		FreeAllKey(lpHeadUsers2);
	}
	lpHeadLocalMachine2=NULL;
	lpHeadUsers2=NULL;
	*lpComputerName2=0;
	*lpUserName2=0;

}

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


	nKEYADD=0;
	nKEYDEL=0;
	nVALADD=0;
	nVALDEL=0;
	nVALMODI=0;
	nFILEADD=0;
	nFILEDEL=0;
	nFILEMODI=0;
	nDIRADD=0;
	nDIRDEL=0;
	nDIRMODI=0;
	lpKEYADDHEAD=NULL;
	lpKEYDELHEAD=NULL;
	lpVALADDHEAD=NULL;
	lpVALDELHEAD=NULL;
	lpVALMODIHEAD=NULL;
	lpFILEADDHEAD=NULL;
	lpFILEDELHEAD=NULL;
	lpFILEMODIHEAD=NULL;
	lpDIRADDHEAD=NULL;
	lpDIRDELHEAD=NULL;
	lpDIRMODIHEAD=NULL;
}


//-------------------------------------------------------------
//Registry Compare Engine
//-------------------------------------------------------------
VOID * CompareFirstSubKey(LPKEYCONTENT lpHead1,LPKEYCONTENT lpHead2)
{
	LPKEYCONTENT	lp1,lp2;
	LPVALUECONTENT	lpvalue1,lpvalue2;
	//DWORD	i;

	for(lp1=lpHead1; lp1!=NULL; lp1=lp1->lpbrotherkey) {
		for(lp2=lpHead2; lp2!=NULL; lp2=lp2->lpbrotherkey) {
			if((lp2->bkeymatch==NOTMATCH)&&strcmp(lp1->lpkeyname,lp2->lpkeyname)==0) { //1.8.2 from lstrcmp to strcmp
				//Same key found! we compare their values and their subkeys!

				lp2->bkeymatch=ISMATCH;
				if (lp1->lpfirstvalue==NULL&&lp2->lpfirstvalue!=NULL) {
					//Key1 has no values,so lpvalue2 is added! we find all values belongs to lp2!
					GetAllValue(VALADD,&nVALADD,lp2);
				} else {
					if (lp1->lpfirstvalue!=NULL&&lp2->lpfirstvalue==NULL) {
						//Key2 has no values,so lpvalue1 is delted! we find all values belongs to lp1!
						GetAllValue(VALDEL,&nVALDEL,lp1);
					} else {
						//Two keys all has values,so we loop them

						for(lpvalue1=lp1->lpfirstvalue; lpvalue1!=NULL; lpvalue1=lpvalue1->lpnextvalue) {
							for(lpvalue2=lp2->lpfirstvalue; lpvalue2!=NULL; lpvalue2=lpvalue2->lpnextvalue) {
								//Loop lp2 to find a value matchs lp1's
								if((lpvalue2->bvaluematch==NOTMATCH)&&(lpvalue1->typecode==lpvalue2->typecode)) {
									//Same valuedata type
									if(strcmp(lpvalue1->lpvaluename,lpvalue2->lpvaluename)==0) { //1.8.2 from lstrcmp to strcmp
										//Same valuename
										if(lpvalue1->datasize==lpvalue2->datasize) {
											//Same size of valuedata
											/*for(i=0;i<lpvalue1->datasize;i++)
											{
												if (*((lpvalue1->lpvaluedata)+i)!=*((lpvalue2->lpvaluedata)+i))
													break;
											}
											if (i==lpvalue1->datasize)*/
											if(memcmp(lpvalue1->lpvaluedata,lpvalue2->lpvaluedata,lpvalue1->datasize)==0) { //1.8.2
												//Same valuedata,keys are the same!

												lpvalue2->bvaluematch=ISMATCH;
												break;//Be sure not to do lp2==NULL
											} else {
												//Valuedata not match due to data mismatch!,we found a modified valuedata!*****
												lpvalue2->bvaluematch=ISMODI;
												LogToMem(VALMODI,&nVALMODI,lpvalue1);
												LogToMem(VALMODI,&nVALMODI,lpvalue2);
												nVALMODI--;
												break;
											}
										} else {
											//Waluedata does not match due to size,we found a modified valuedata!******
											lpvalue2->bvaluematch=ISMODI;
											LogToMem(VALMODI,&nVALMODI,lpvalue1);
											LogToMem(VALMODI,&nVALMODI,lpvalue2);
											nVALMODI--;
											break;
										}
									}
								}
							}
							if(lpvalue2==NULL) {
								//We found a value in lp1 but not in lp2,we found a deleted value*****
								LogToMem(VALDEL,&nVALDEL,lpvalue1);
							}
						}
						//After we loop to end,we do extra loop use flag we previouse made to get added values
						for(lpvalue2=lp2->lpfirstvalue; lpvalue2!=NULL; lpvalue2=lpvalue2->lpnextvalue) {
							if(lpvalue2->bvaluematch!=ISMATCH&&lpvalue2->bvaluematch!=ISMODI) {
								//We found a value in lp2's but not in lp1's,we found a added value****
								LogToMem(VALADD,&nVALADD,lpvalue2);

							}
						}
					}
				}

				//////////////////////////////////////////////////////////////
				//After we walk through the values above,we now try to loop the sub keys of current key
				if(lp1->lpfirstsubkey==NULL&&lp2->lpfirstsubkey!=NULL) {
					//lp2's firstsubkey added!
					GetAllSubName(TRUE,KEYADD,VALADD,&nKEYADD,&nVALADD,lp2->lpfirstsubkey);
				}
				if(lp1->lpfirstsubkey!=NULL&&lp2->lpfirstsubkey==NULL) {
					//lp1's firstsubkey deleted!
					GetAllSubName(TRUE,KEYDEL,VALDEL,&nKEYDEL,&nVALDEL,lp1->lpfirstsubkey);
				}
				if(lp1->lpfirstsubkey!=NULL&&lp2->lpfirstsubkey!=NULL) {
					CompareFirstSubKey(lp1->lpfirstsubkey,lp2->lpfirstsubkey);
				}
				break;
			}
		}
		if(lp2==NULL) {
			//We did not find a lp2 matches a lp1,so lp1 is deleted!
			GetAllSubName(FALSE,KEYDEL,VALDEL,&nKEYDEL,&nVALDEL,lp1);
		}

	}

	//After we loop to end,we do extra loop use flag we previouse made to get added keys
	for(lp2=lpHead2; lp2!=NULL; lp2=lp2->lpbrotherkey) { //->lpbrotherkey
		nComparing++;
		if(lp2->bkeymatch==NOTMATCH) {
			//We did not find a lp1 matches a lp2,so lp2 is added!
			GetAllSubName(FALSE,KEYADD,VALADD,&nKEYADD,&nVALADD,lp2);
		}
	}

	// Progress bar update
	if (nGettingKey!=0)
		if (nComparing%nGettingKey>nRegStep) {
			nComparing=0;
			SendDlgItemMessage(hWnd,IDC_PBCOMPARE,PBM_STEPIT,(WPARAM)0,(LPARAM)0);
		}

	return NULL;
}


//------------------------------------------------------------
// Routine to call Registry/File Compare Engine
//------------------------------------------------------------
BOOL CompareShots(void)
{
	BOOL	isHTML,bshot2isnewer;//,bSaveWithCommentName;
	LPSTR	lpstrcomp,lpExt,lpDestFileName;
	DWORD	buffersize=2048,nTotal,filetail=0,nLengthofStr;
	LPHEADFILE	lphf1,lphf2;
	LPFILECONTENT lpfc1,lpfc2;
	FILETIME ftime1,ftime2;

	if(!DirChainMatch(lpHeadFile1,lpHeadFile2)) {
		MessageBox(hWnd,"Found two shots with different DIR chain!(or with different order)\r\nYou can continue,but file compare result would be abnormal!","Warning",MB_ICONWARNING);
	}

	InitProgressBar();

	SystemTimeToFileTime(lpSystemtime1,&ftime1);
	SystemTimeToFileTime(lpSystemtime2,&ftime2);

	bshot2isnewer=(CompareFileTime(&ftime1,&ftime2)<=0) ? TRUE:FALSE;
	if(bshot2isnewer) {
		CompareFirstSubKey(lpHeadLocalMachine1,lpHeadLocalMachine2);
		CompareFirstSubKey(lpHeadUsers1,lpHeadUsers2);
	} else {
		CompareFirstSubKey(lpHeadLocalMachine2,lpHeadLocalMachine1);
		CompareFirstSubKey(lpHeadUsers2,lpHeadUsers1);
	}

	SendDlgItemMessage(hWnd,IDC_PBCOMPARE,PBM_SETPOS,(WPARAM)0,(LPARAM)0);

	//Dir compare v1.8.1
	//determine newer
	if(bshot2isnewer) {
		lphf1=lpHeadFile1;
		lphf2=lpHeadFile2;
	} else {
		lphf1=lpHeadFile2;
		lphf2=lpHeadFile1;
	}
	//first loop
	for(; lphf1!=NULL; lphf1=lphf1->lpnextheadfile) {
		if(lphf1->lpfilecontent!=NULL) {
			lpfc1=lphf1->lpfilecontent;
		} else {
			lpfc1=NULL;
		}

		if(lpfc1!=NULL) {
			if((lpfc2=SearchDirChain(lpfc1->lpfilename,lphf2))!=NULL) { //note lphf2 should not changed here!
				CompareFirstSubFile(lpfc1,lpfc2);    //if found ,we do compare
			} else { //can not find matched lpfc1 in lphf2 chain.
				GetAllSubFile(FALSE,DIRDEL,FILEDEL,&nDIRDEL,&nFILEDEL,lpfc1);
			}
		}
	}
	//reset pointers
	if(bshot2isnewer) {
		lphf1=lpHeadFile1;
		lphf2=lpHeadFile2;
	} else {
		lphf1=lpHeadFile2;
		lphf2=lpHeadFile1;
	}
	//second loop
	for(; lphf2!=NULL; lphf2=lphf2->lpnextheadfile) {
		if(lphf2->lpfilecontent!=NULL) {
			lpfc2=lphf2->lpfilecontent;
		} else {
			lpfc2=NULL;
		}
		if(lpfc2!=NULL) {
			if((lpfc1=SearchDirChain(lpfc2->lpfilename,lphf1))==NULL) { // in the second loop we only find those do not match
				GetAllSubFile(FALSE,DIRADD,FILEADD,&nDIRADD,&nFILEADD,lpfc2);
			}
		}
	}
	/*	silly one used in 1.8.0
		for(lphf1=lpHeadFile1,lphf2=lpHeadFile2;;)
		{
			//Normally,two lphf should run parallel,otherwise sth abnormal should happen :(
			if(lphf1!=NULL && lphf1->lpfilecontent!=NULL)
				lpfc1=lphf1->lpfilecontent;
			else
				lpfc1=NULL;
			if(lphf2!=NULL && lphf2->lpfilecontent!=NULL)
				lpfc2=lphf2->lpfilecontent;
			else
				lpfc2=NULL;
			if(lpfc1==NULL&&lpfc2==NULL)
				break;
			if(bshot2isnewer)
				CompareFirstSubFile(lpfc1,lpfc2);
			else
				CompareFirstSubFile(lpfc2,lpfc1);

			if(lphf1!=NULL)
				lphf1=lphf1->lpnextheadfile;
			if(lphf2!=NULL)
				lphf2=lphf2->lpnextheadfile;

		}
	*/
	SendDlgItemMessage(hWnd,IDC_PBCOMPARE,PBM_SETPOS,(WPARAM)MAXPBPOSITION,(LPARAM)0);

	if(SendMessage(GetDlgItem(hWnd,IDC_RADIO1),BM_GETCHECK,(WPARAM)0,(LPARAM)0)==1) {
		isHTML=FALSE;
		lpExt=".txt";
	} else {
		isHTML=TRUE;
		lpExt=".htm";
	}

	lpDestFileName=MYALLOC0(MAX_PATH*2);
	lpstrcomp=MYALLOC0(buffersize); //buffersize must> commentlength+10 .txt 0000
	GetDlgItemText(hWnd,IDC_EDITCOMMENT,lpstrcomp,COMMENTLENGTH);
	GetDlgItemText(hWnd,IDC_EDITPATH,lpOutputpath,MAX_PATH);

	nLengthofStr=strlen(lpOutputpath);

	if (nLengthofStr>0&&*(lpOutputpath+nLengthofStr-1)!='\\') {
		*(lpOutputpath+nLengthofStr)='\\';
		*(lpOutputpath+nLengthofStr+1)=0x00; //bug found by "itschy" <itschy@lycos.de> 1.61d->1.61e
		nLengthofStr++;
	}
	strcpy(lpDestFileName,lpOutputpath);

	//bSaveWithCommentName=TRUE;
	if	(ReplaceInValidFileName(lpstrcomp)) {
		strcat(lpDestFileName,lpstrcomp);
	} else {
		strcat(lpDestFileName,str_DefResPre);
	}

	nLengthofStr=strlen(lpDestFileName);
	strcat(lpDestFileName,lpExt);
	hFile = CreateFile(lpDestFileName,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
	if( hFile == INVALID_HANDLE_VALUE) {
		for (filetail=0; filetail<MAXAMOUNTOFFILE; filetail++) {
			sprintf(lpDestFileName+nLengthofStr,"%04d",filetail);
			//*(lpDestFileName+nLengthofStr+4)=0x00;
			strcpy(lpDestFileName+nLengthofStr+4,lpExt);

			hFile = CreateFile(lpDestFileName,GENERIC_READ | GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
			if( hFile == INVALID_HANDLE_VALUE) {
				if (GetLastError()==ERROR_FILE_EXISTS) {	//My God! I use stupid ERROR_ALREADY_EXISTS first!!
					continue;
				} else {
					ErrMsg(lan_errorcreatefile);
					return FALSE;
				}
			} else {
				break;
			}
		}
		if (filetail>=MAXAMOUNTOFFILE) {
			ErrMsg(lan_errorcreatefile);
			return FALSE;
		}

	}

	if(isHTML==TRUE) {
		WriteHtmlbegin();
	}

	WriteFile(hFile,str_prgname,strlen(str_prgname),&NBW,NULL);
	WriteFile(hFile,str_CR,strlen(str_CR),&NBW,NULL);

	//_asm int 3;
	GetDlgItemText(hWnd,IDC_EDITCOMMENT,lpstrcomp,COMMENTLENGTH);
	WriteTitle(lan_comments,lpstrcomp,isHTML);


	sprintf(lpstrcomp,"%d%s%d%s%d %02d%s%02d%s%02d %s %d%s%d%s%d %02d%s%02d%s%02d",
			lpSystemtime1->wYear,"/",
			lpSystemtime1->wMonth,"/",
			lpSystemtime1->wDay,
			lpSystemtime1->wHour,":",
			lpSystemtime1->wMinute,":",
			lpSystemtime1->wSecond," , ",
			lpSystemtime2->wYear,"/",
			lpSystemtime2->wMonth,"/",
			lpSystemtime2->wDay,
			lpSystemtime2->wHour,":",
			lpSystemtime2->wMinute,":",
			lpSystemtime2->wSecond

		   );

	WriteTitle(lan_datetime,lpstrcomp,isHTML);



	*lpstrcomp=0x00; //ZeroMemory(lpstrcomp,buffersize);
	//GetComputerName(lpstrcomp,&buffersize);
	strcpy(lpstrcomp,lpComputerName1);
	strcat(lpstrcomp," , ");
	strcat(lpstrcomp,lpComputerName2);
	WriteTitle(lan_computer,lpstrcomp,isHTML);

	*lpstrcomp=0x00;//ZeroMemory(lpstrcomp,buffersize);
	//GetUserName(lpstrcomp,&buffersize);
	strcpy(lpstrcomp,lpUserName1);
	strcat(lpstrcomp," , ");
	strcat(lpstrcomp,lpUserName2);

	WriteTitle(lan_username,lpstrcomp,isHTML);

	MYFREE(lpstrcomp);

	//Write keydel part
	if(nKEYDEL!=0) {
		WriteHead(lan_keydel,nKEYDEL,isHTML);
		WritePart(lpKEYDELHEAD,isHTML,FALSE);
	}
	//Write keyadd part
	if(nKEYADD!=0) {
		WriteHead(lan_keyadd,nKEYADD,isHTML);
		WritePart(lpKEYADDHEAD,isHTML,FALSE);
	}
	//Write valdel part
	if(nVALDEL!=0) {
		WriteHead(lan_valdel,nVALDEL,isHTML);
		WritePart(lpVALDELHEAD,isHTML,FALSE);
	}
	//Write valadd part
	if(nVALADD!=0) {
		WriteHead(lan_valadd,nVALADD,isHTML);
		WritePart(lpVALADDHEAD,isHTML,FALSE);
	}
	//Write valmodi part
	if(nVALMODI!=0) {
		WriteHead(lan_valmodi,nVALMODI,isHTML);
		WritePart(lpVALMODIHEAD,isHTML,TRUE);
	}
	//Write file add part
	if(nFILEADD!=0) {
		WriteHead(lan_fileadd,nFILEADD,isHTML);
		WritePart(lpFILEADDHEAD,isHTML,FALSE);
	}
	//Write file del part
	if(nFILEDEL!=0) {
		WriteHead(lan_filedel,nFILEDEL,isHTML);
		WritePart(lpFILEDELHEAD,isHTML,FALSE);
	}
	//Write file modi part
	if(nFILEMODI!=0) {
		WriteHead(lan_filemodi,nFILEMODI,isHTML);
		WritePart(lpFILEMODIHEAD,isHTML,FALSE);
	}
	//Write directory add part
	if(nDIRADD!=0) {
		WriteHead(lan_diradd,nDIRADD,isHTML);
		WritePart(lpDIRADDHEAD,isHTML,FALSE);
	}
	//Write directory del part
	if(nDIRDEL!=0) {
		WriteHead(lan_dirdel,nDIRDEL,isHTML);
		WritePart(lpDIRDELHEAD,isHTML,FALSE);
	}
	//Write directory modi part
	if(nDIRMODI!=0) {
		WriteHead(lan_dirmodi,nDIRMODI,isHTML);
		WritePart(lpDIRMODIHEAD,isHTML,FALSE);
	}

	nTotal=nKEYADD+nKEYDEL+nVALADD+nVALDEL+nVALMODI+nFILEADD+nFILEDEL+nFILEMODI+nDIRADD+nDIRDEL+nDIRMODI;
	if(isHTML==TRUE) {
		WriteHtmlbr();
	}
	WriteHead(lan_total,nTotal,isHTML);
	if(isHTML==TRUE) {
		WriteHtmlover();
	}


	CloseHandle(hFile);

	if((DWORD)ShellExecute(hWnd,"open",lpDestFileName,NULL,NULL,SW_SHOW)<=32) {
		ErrMsg(lan_errorexecviewer);
	}
	MYFREE(lpDestFileName);


	return TRUE;
}


//------------------------------------------------------------
//Registry Shot Engine
//------------------------------------------------------------
VOID	GetRegistrySnap(HKEY hkey,LPKEYCONTENT lpFatherKeyContent)
{

	HKEY  Subhkey;
	DWORD i,NTr;
	DWORD TypeCode;
	DWORD LengthOfKeyName;
	DWORD LengthOfValueName;
	DWORD LengthOfValueData;
	DWORD LengthOfLongestValueName;
	DWORD LengthOfLongestValueData;
	DWORD LengthOfLongestSubkeyName;
	LPSTR lpValueName;
	LPSTR lpValueData;
	LPKEYCONTENT	lpKeyContent,lpKeyContentLast;
	LPVALUECONTENT	lpValueContent,lpValueContentLast;

	//To detemine MAX length
	if(RegQueryInfoKey(
				hkey,
				NULL,						//lpClassName_nouse,
				NULL,						//&nClassName_nouse_length,
				NULL,
				NULL,						//&NumberOfSubkeys,
				&LengthOfLongestSubkeyName, //chars
				NULL,						//&nClassName_nouse_longestlength,
				NULL,						//&NumberOfValue,
				&LengthOfLongestValueName,	//chars
				&LengthOfLongestValueData,	//bytes
				NULL,						//&nSecurity_length_nouse,
				NULL						//&ftLastWrite
			)!=ERROR_SUCCESS) {
		return ;
	}
	LengthOfLongestSubkeyName =LengthOfLongestSubkeyName*2+3; //yeah,may be x+1 is enought! x=chars
	LengthOfLongestValueName  =LengthOfLongestValueName*2+3; //yeah,may be x+1 is enought! x=chars
	LengthOfLongestValueData  =LengthOfLongestValueData+1;
	lpValueName=MYALLOC(LengthOfLongestValueName);
	lpValueData=MYALLOC(LengthOfLongestValueData);

	//Get Values
	for(i=0;; i++) {

		*(LPBYTE)lpValueName=(BYTE)0x00;//That's the bug in 2000! thanks zhangl@digiark.com!
		*(LPBYTE)lpValueData=(BYTE)0x00;
		//DebugBreak();
		LengthOfValueName=LengthOfLongestValueName;
		LengthOfValueData=LengthOfLongestValueData;
		NTr=RegEnumValue(hkey,i,lpValueName,&LengthOfValueName,NULL,&TypeCode,lpValueData,&LengthOfValueData);
		if	(NTr==ERROR_NO_MORE_ITEMS) {
			break;
		} else {
			if	(NTr!=ERROR_SUCCESS) {
				continue;
			}
		}

#ifdef DEBUGLOG
		DebugLog("debug_trytogetvalue.log","trying:",hWnd,FALSE);
		DebugLog("debug_trytogetvalue.log",lpValueName,hWnd,TRUE);
#endif

		lpValueContent=MYALLOC0(sizeof(VALUECONTENT));
		//I had done if(i==0) in 1.50b- ! thanks fisttk@21cn.com and non-standard
		if(lpFatherKeyContent->lpfirstvalue==NULL) {
			lpFatherKeyContent->lpfirstvalue=lpValueContent;
		} else {
			lpValueContentLast->lpnextvalue=lpValueContent;
		}
		lpValueContentLast=lpValueContent;
		lpValueContent->typecode=TypeCode;
		lpValueContent->datasize=LengthOfValueData;
		lpValueContent->lpfatherkey=lpFatherKeyContent;
		lpValueContent->lpvaluename=MYALLOC(strlen(lpValueName)+1);
		strcpy(lpValueContent->lpvaluename,lpValueName);

		if(LengthOfValueData!=0) {
			lpValueContent->lpvaluedata=MYALLOC(LengthOfValueData);
			CopyMemory(lpValueContent->lpvaluedata,lpValueData,LengthOfValueData);
			//	*(lpValueContent->lpvaluedata+LengthOfValueData)=0x00;
		}
		nGettingValue++;

#ifdef DEBUGLOG
		lstrdb1=MYALLOC0(100);
		sprintf(lstrdb1,"LGVN:%08d LGVD:%08d VN:%08d VD:%08d",LengthOfLongestValueName,LengthOfLongestValueData,LengthOfValueName,LengthOfValueData);
		DebugLog("debug_valuenamedata.log",lstrdb1,hWnd,TRUE);
		DebugLog("debug_valuenamedata.log",GetWholeValueName(lpValueContent),hWnd,FALSE);
		DebugLog("debug_valuenamedata.log",GetWholeValueData(lpValueContent),hWnd,TRUE);
		//DebugLog("debug_valuenamedata.log",":",hWnd,FALSE);
		//DebugLog("debug_valuenamedata.log",lpValueData,hWnd,TRUE);
		MYFREE(lstrdb1);

#endif
	}

	MYFREE(lpValueName);
	MYFREE(lpValueData);

	for(i=0;; i++) {
		LengthOfKeyName=LengthOfLongestSubkeyName;
		*(LPBYTE)lpKeyName=(BYTE)0x00;
		NTr=RegEnumKeyEx(hkey,i,lpKeyName,&LengthOfKeyName,NULL,NULL,NULL,&ftLastWrite);
		if	(NTr==ERROR_NO_MORE_ITEMS) {
			break;
		} else {
			if	(NTr!=ERROR_SUCCESS) {
				continue;
			}
		}
		lpKeyContent=MYALLOC0(sizeof(KEYCONTENT));
		if	(lpFatherKeyContent->lpfirstsubkey==NULL) {
			lpFatherKeyContent->lpfirstsubkey=lpKeyContent;
		} else {
			lpKeyContentLast->lpbrotherkey=lpKeyContent;
		}
		lpKeyContentLast=lpKeyContent;
		lpKeyContent->lpkeyname=MYALLOC(strlen(lpKeyName)+1);
		strcpy(lpKeyContent->lpkeyname,lpKeyName);
		lpKeyContent->lpfatherkey=lpFatherKeyContent;
		//DebugLog("debug_getkey.log",lpKeyName,hWnd,TRUE);

#ifdef DEBUGLOG
		lstrdb1=MYALLOC0(100);
		sprintf(lstrdb1,"LGKN:%08d KN:%08d",LengthOfLongestSubkeyName,LengthOfKeyName);
		DebugLog("debug_key.log",lstrdb1,hWnd,TRUE);
		DebugLog("debug_key.log",GetWholeKeyName(lpKeyContent),hWnd,TRUE);
		MYFREE(lstrdb1);

#endif

		nGettingKey++;

		if(RegOpenKeyEx(hkey,lpKeyName,0,KEY_QUERY_VALUE|KEY_ENUMERATE_SUB_KEYS,&Subhkey)!=ERROR_SUCCESS) {
			continue;
		}
		if(IsInSkipList(lpKeyName,lpSnapRegs) ) {
			//tfx
			RegCloseKey(Subhkey);  //1.8.2 seprate
			continue;
		}

		GetRegistrySnap(Subhkey,lpKeyContent);
		RegCloseKey(Subhkey);
	}

	nGettingTime=GetTickCount();
	if ((nGettingTime-nBASETIME1)>REFRESHINTERVAL) {
		UpdateCounters(lan_key,lan_value,nGettingKey,nGettingValue);
	}


	return ;
}


//--------------------------------------------------
//Registry Save Engine (It is rather stupid!)
//--------------------------------------------------
VOID	SaveRegKey(LPKEYCONTENT lpKeyContent, DWORD nFPCurrentFatherKey,DWORD nFPCaller)
{

	DWORD	nFPHeader,nFPCurrent,nFPTemp4Write,nLenPlus1;
	LPVALUECONTENT lpv;


	nLenPlus1=strlen(lpKeyContent->lpkeyname)+1;							//get len+1
	nFPHeader=SetFilePointer(hFileWholeReg,0,NULL,FILE_CURRENT);			//save head fp
	nFPTemp4Write=nFPHeader+21;												//5*4+1
	WriteFile(hFileWholeReg,&nFPTemp4Write,4,&NBW,NULL);					//save location of lpkeyname
	nFPTemp4Write=(lpKeyContent->lpfirstvalue!=NULL) ? (nFPHeader+21+nLenPlus1):0;			//We write lpkeyname plus a "\0"
	WriteFile(hFileWholeReg,&nFPTemp4Write,4,&NBW,NULL);					//save location of lpfirstvalue
	WriteFile(hFileWholeReg,(LPBYTE)lpKeyContent+8,8,&NBW,NULL);			//save lpfirstsubkey and lpbrotherkey
	WriteFile(hFileWholeReg,&nFPCurrentFatherKey,4,&NBW,NULL);				//save nFPCurrentFatherKey passed by caller
	nFPTemp4Write=0;
	WriteFile(hFileWholeReg,&nFPTemp4Write,1,&NBW,NULL);					//clear and save bkeymatch
	WriteFile(hFileWholeReg,lpKeyContent->lpkeyname,nLenPlus1,&NBW,NULL);	//Save the current keyname



	//Save the sub-value of current KeyContent
	for(lpv=lpKeyContent->lpfirstvalue; lpv!=NULL; lpv=lpv->lpnextvalue) {
		nLenPlus1=strlen(lpv->lpvaluename)+1;
		nFPCurrent=SetFilePointer(hFileWholeReg,0,NULL,FILE_CURRENT);		//save  fp
		WriteFile(hFileWholeReg,(LPBYTE)lpv,8,&NBW,NULL);
		nFPTemp4Write=nFPCurrent+25;										//6*4+1
		WriteFile(hFileWholeReg,&nFPTemp4Write,4,&NBW,NULL);				//save location of lpvaluename
		nFPTemp4Write=(lpv->datasize>0)?(nFPCurrent+25+nLenPlus1):0;		//if no lpvaluedata,we write 0
		WriteFile(hFileWholeReg,&nFPTemp4Write,4,&NBW,NULL);				//save location of lpvaluedata
		nFPTemp4Write=(lpv->lpnextvalue!=NULL)?(nFPCurrent+25+nLenPlus1+lpv->datasize):0;	//if no nextvalue we write 0
		WriteFile(hFileWholeReg,&nFPTemp4Write,4,&NBW,NULL);				//save location of next subvalue
		nFPTemp4Write=nFPHeader;
		WriteFile(hFileWholeReg,&nFPTemp4Write,4,&NBW,NULL);				//save location of current key
		nFPTemp4Write=0;
		WriteFile(hFileWholeReg,&nFPTemp4Write,1,&NBW,NULL);				//clear and save bvaluematch
		WriteFile(hFileWholeReg,lpv->lpvaluename,nLenPlus1,&NBW,NULL);		//save lpvaluename
		WriteFile(hFileWholeReg,lpv->lpvaluedata,lpv->datasize,&NBW,NULL);	//save lpvaluedata

	}

	if(lpKeyContent->lpfirstsubkey!=NULL) {
		//pass this keycontent's position as subkey's fatherkey's position and pass the "lpfirstsubkey field"
		SaveRegKey(lpKeyContent->lpfirstsubkey,nFPHeader,nFPHeader+8);
	}

	if(lpKeyContent->lpbrotherkey!=NULL) {
		// pass this key's fatherkey's position as brother's father and pass "lpbrotherkey field"
		SaveRegKey(lpKeyContent->lpbrotherkey,nFPCurrentFatherKey,nFPHeader+12);
	}

	if(nFPCaller>0) { //save position of current key in current father key
		nFPCurrent=SetFilePointer(hFileWholeReg,0,NULL,FILE_CURRENT);
		SetFilePointer(hFileWholeReg,nFPCaller,NULL,FILE_BEGIN);
		WriteFile(hFileWholeReg,&nFPHeader,4,&NBW,NULL);
		SetFilePointer(hFileWholeReg,nFPCurrent,NULL,FILE_BEGIN);
	}
	nSavingKey++;
	if (nGettingKey!=0)
		if (nSavingKey%nGettingKey>nRegStep) {
			nSavingKey=0;
			SendDlgItemMessage(hWnd,IDC_PBCOMPARE,PBM_STEPIT,(WPARAM)0,(LPARAM)0);
			UpdateWindow(hWnd);
			PeekMessage(&msg,hWnd,WM_ACTIVATE,WM_ACTIVATE,PM_REMOVE);
		}

}


//--------------------------------------------------
//Routine to call Registry Save Engine and file save engine
//--------------------------------------------------
VOID	SaveHive(LPKEYCONTENT lpKeyHLM,LPKEYCONTENT lpKeyUSER,
				 LPHEADFILE lpHF,LPSTR computer,LPSTR user,LPVOID time)
{
	DWORD nFPcurrent,nFPcurrent1;
	LPHEADFILE lphf;
	if(lpKeyHLM!=NULL||lpKeyUSER!=NULL) {

		opfn.lStructSize=sizeof(opfn);
		opfn.hwndOwner=hWnd;
		opfn.lpstrFilter=str_filter;
		opfn.lpstrFile=MYALLOC0(MAX_PATH+1);
		opfn.nMaxFile=MAX_PATH*2;
		opfn.lpstrInitialDir=lpLastSaveDir;
		opfn.lpstrDefExt="hiv";
		opfn.Flags=OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;
		if(GetSaveFileName(&opfn)) {
			hFileWholeReg = CreateFile(opfn.lpstrFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
			if( hFileWholeReg!= INVALID_HANDLE_VALUE) {

				UI_BeforeClear();
				InitProgressBar();
				WriteFile(hFileWholeReg,str_RegFileSignature,sizeof(str_RegFileSignature)-1,&NBW,NULL); //save lpvaluedata
				//0   signature(<=12) last 4 bytes may be used in furture
				//16  startoflpkeyhlm (512)
				//20  startoflpkeyuser(???)
				//24  fileheadchain begin (new) ->(file -> nextfilehead(4) next4bytes is filecontent) added in 1.8
				//28  future use!
				//32  computer name
				//96  username
				//160 systemtime

				//Save the position of H_L_M
				nFPcurrent=HIVEBEGINOFFSET; //computerlen*2+sizeof(systemtime)+32 must <hivebeginoffset
				SetFilePointer(hFileWholeReg,16,NULL,FILE_BEGIN);
				WriteFile(hFileWholeReg,&nFPcurrent,4,&NBW,NULL);

				SetFilePointer(hFileWholeReg,HIVEBEGINOFFSET,NULL,FILE_BEGIN);


				if(lpKeyHLM!=NULL) {
					SaveRegKey(lpKeyHLM,0,0);
				}

				//Save the position of hkeyUsr
				nFPcurrent=SetFilePointer(hFileWholeReg,0,NULL,FILE_CURRENT);
				SetFilePointer(hFileWholeReg,20,NULL,FILE_BEGIN);
				WriteFile(hFileWholeReg,&nFPcurrent,4,&NBW,NULL);
				SetFilePointer(hFileWholeReg,nFPcurrent,NULL,FILE_BEGIN);

				if(lpKeyUSER!=NULL) {
					SaveRegKey(lpKeyUSER,0,0);
				}

				if(lpHF!=NULL) {
					//Write start position of file chain
					nFPcurrent=SetFilePointer(hFileWholeReg,0,NULL,FILE_CURRENT);
					SetFilePointer(hFileWholeReg,24,NULL,FILE_BEGIN);
					WriteFile(hFileWholeReg,&nFPcurrent,4,&NBW,NULL);  //write start pos at 24
					SetFilePointer(hFileWholeReg,nFPcurrent,NULL,FILE_BEGIN);

					for(lphf=lpHF; lphf!=NULL;) {
						nFPcurrent=SetFilePointer(hFileWholeReg,0,NULL,FILE_CURRENT); //save place for next filehead in chain
						SetFilePointer(hFileWholeReg,4,NULL,FILE_CURRENT); //move 4 bytes,leave space for lpnextfilecontent
						nFPcurrent1=nFPcurrent+8;
						WriteFile(hFileWholeReg,&nFPcurrent1,4,&NBW,NULL); //write lpfilecontent

						SaveFileContent(lphf->lpfilecontent,0,0);
						nFPcurrent1=SetFilePointer(hFileWholeReg,0,NULL,FILE_CURRENT);
						SetFilePointer(hFileWholeReg,nFPcurrent,NULL,FILE_BEGIN);
						lphf=lphf->lpnextheadfile;
						if(lphf!=NULL) {
							WriteFile(hFileWholeReg,&nFPcurrent1,4,&NBW,NULL);
						} else {
							nFPcurrent1=0;
							WriteFile(hFileWholeReg,&nFPcurrent1,4,&NBW,NULL);
							break;
						}
						SetFilePointer(hFileWholeReg,nFPcurrent1,NULL,FILE_BEGIN);
					}
				}


				SetFilePointer(hFileWholeReg,32,NULL,FILE_BEGIN);
				WriteFile(hFileWholeReg,computer,strlen(computer)+1,&NBW,NULL);
				SetFilePointer(hFileWholeReg,COMPUTERNAMELEN+32,NULL,FILE_BEGIN);
				WriteFile(hFileWholeReg,user,strlen(user)+1,&NBW,NULL);
				SetFilePointer(hFileWholeReg,COMPUTERNAMELEN*2+32,NULL,FILE_BEGIN);
				WriteFile(hFileWholeReg,time,sizeof(SYSTEMTIME),&NBW,NULL);

				ShowWindow(GetDlgItem(hWnd,IDC_PBCOMPARE),SW_HIDE);

				SetCursor(hSaveCursor);
				MessageBeep(0xffffffff);
				CloseHandle(hFileWholeReg);
			} else {
				ErrMsg(lan_errorcreatefile);
			}

		}
		*(opfn.lpstrFile+opfn.nFileOffset)=0x00;
		strcpy(lpLastSaveDir,opfn.lpstrFile);
		MYFREE(opfn.lpstrFile);
	}
}


//--------------------------------------------------
//ReAlign key&value content after Loading from hive file
//--------------------------------------------------
VOID ReAlignReg(LPKEYCONTENT lpKey,DWORD nBase)
{
	LPDWORD lp;
	LPVALUECONTENT lpv;
	lp=(LPDWORD)lpKey;

	if((*lp)!=0) {
		(*lp)+=nBase;
	}
	lp++;
	if((*lp)!=0) {
		(*lp)+=nBase;
	}
	lp++;
	if((*lp)!=0) {
		(*lp)+=nBase;
	}
	lp++;
	if((*lp)!=0) {
		(*lp)+=nBase;
	}
	lp++;
	if((*lp)!=0) {
		(*lp)+=nBase;
	}
	nGettingKey++;

	for(lpv=lpKey->lpfirstvalue; lpv!=NULL; lpv=lpv->lpnextvalue) {
		lp=(LPDWORD)lpv+2;
		if((*lp)!=0) {
			(*lp)+=nBase;
		}
		lp++;
		if((*lp)!=0) {
			(*lp)+=nBase;
		}
		lp++;
		if((*lp)!=0) {
			(*lp)+=nBase;
		}
		lp++;
		if((*lp)!=0) {
			(*lp)+=nBase;
		}
	}

	if(lpKey->lpfirstsubkey!=NULL) {
		ReAlignReg(lpKey->lpfirstsubkey,nBase);
	}

	if(lpKey->lpbrotherkey!=NULL) {
		ReAlignReg(lpKey->lpbrotherkey,nBase);
	}
}


//----------------------------------------------------------------------------------------------------
//Load Registry From HIVE file (After this,We should realign the data in memory)
//----------------------------------------------------------------------------------------------------
BOOL LoadHive(LPKEYCONTENT FAR * lplpKeyHLM,LPKEYCONTENT FAR * lplpKeyUSER,
			  LPHEADFILE FAR * lplpHeadFile,LPSTR FAR * lpHive)
{
	DWORD	nFileSize,nOffSet=0,nBase,i,j,nRemain,nReadSize;
	BOOL	bRet=FALSE;
	opfn.lStructSize=sizeof(opfn);
	opfn.hwndOwner=hWnd;
	opfn.lpstrFilter=str_filter;
	opfn.lpstrFile=MYALLOC0(MAX_PATH+1);
	opfn.nMaxFile=MAX_PATH*2;
	opfn.lpstrInitialDir=lpLastOpenDir;
	opfn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
	opfn.lpstrDefExt="hiv";
	if(GetOpenFileName(&opfn)) {
		hFileWholeReg=CreateFile(opfn.lpstrFile,GENERIC_READ ,FILE_SHARE_READ ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
		if (hFileWholeReg!=INVALID_HANDLE_VALUE) {
			*lpHive=MYALLOC0(16);
			ReadFile(hFileWholeReg,*lpHive,16,&NBW,NULL);

			if(strcmp(str_RegFileSignature,*lpHive)!=0) {
				ErrMsg("It is not a valid Regshot hive file!");
				bRet=FALSE;
			} else {
				nGettingKey=0;
				nGettingFile=0;
				if(is1) {
					UI_BeforeShot(IDC_1STSHOT);
				} else {
					UI_BeforeShot(IDC_2NDSHOT);
				}
				nFileSize=GetFileSize(hFileWholeReg,NULL);
				MYFREE(*lpHive);

				*lpHive=MYALLOC(nFileSize);
				nBase=(DWORD)(*lpHive);
				ReadFile(hFileWholeReg,&nOffSet,4,&NBW,NULL);
				*lplpKeyHLM=(LPKEYCONTENT)(nBase+nOffSet);

				ReadFile(hFileWholeReg,&nOffSet,4,&NBW,NULL);
				*lplpKeyUSER=(LPKEYCONTENT)(nBase+nOffSet);

				ReadFile(hFileWholeReg,&nOffSet,4,&NBW,NULL);
				if(nOffSet==0) { //!!!
					*lplpHeadFile=NULL;
				} else {
					*lplpHeadFile=(LPHEADFILE)(nBase+nOffSet);
				}

				SetFilePointer(hFileWholeReg,0,NULL,FILE_BEGIN);

				InitProgressBar();
#define READ_BATCH_SIZE 8192
				nFileStep=nFileSize/READ_BATCH_SIZE/MAXPBPOSITION;

				for(i=0,j=0,nRemain=nFileSize;; i+=READ_BATCH_SIZE,j++) {
					if(nRemain>=READ_BATCH_SIZE) {
						nReadSize=READ_BATCH_SIZE;
					} else {
						nReadSize=nRemain;
					}
					//Crash bug made in 1.8.0 tianwei ,fixed in 1.8.1 tianwei
					ReadFile(hFileWholeReg,(*lpHive)+i,nReadSize,&NBW,NULL); //read the whole file now
					if(NBW!=nReadSize) {
						ErrMsg("Reading ERROR!");
						break;
					}
					nRemain-=nReadSize;
					if(nRemain==0) {
						break;
					}
					if(j%(nFileSize/READ_BATCH_SIZE)>nFileStep) {
						j=0;
						SendDlgItemMessage(hWnd,IDC_PBCOMPARE,PBM_STEPIT,(WPARAM)0,(LPARAM)0);
						UpdateWindow(hWnd);
						PeekMessage(&msg,hWnd,WM_ACTIVATE,WM_ACTIVATE,PM_REMOVE);
					}
				}
#undef READ_BATCH_SIZE
				ShowWindow(GetDlgItem(hWnd,IDC_PBCOMPARE),SW_HIDE);


				ReAlignReg(*lplpKeyHLM,nBase);
				ReAlignReg(*lplpKeyUSER,nBase);

				if(*lplpHeadFile!=NULL) {
					SendMessage(GetDlgItem(hWnd,IDC_CHECKDIR),BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
					SendMessage(hWnd,WM_COMMAND,(WPARAM)IDC_CHECKDIR,(LPARAM)0);
					ReAlignFile(*lplpHeadFile,nBase);
					FindDirChain(*lplpHeadFile,lpExtDir,EXTDIRLEN); //Get new chains,must do this after ReAlignFile!
					SetDlgItemText(hWnd,IDC_EDITDIR,lpExtDir);
				} else {
					SetDlgItemText(hWnd,IDC_EDITDIR,"");
				}


				if(is1) {
					//Use copymemory in 1.8,old version direct point to ,which is wrong
					CopyMemory(lpComputerName1,*lpHive+32,COMPUTERNAMELEN);
					CopyMemory(lpUserName1,*lpHive+COMPUTERNAMELEN+32,COMPUTERNAMELEN);
					CopyMemory(lpSystemtime1,(SYSTEMTIME FAR *)(*lpHive+COMPUTERNAMELEN*2+32),sizeof(SYSTEMTIME));
				} else {
					CopyMemory(lpComputerName2,*lpHive+32,COMPUTERNAMELEN);
					CopyMemory(lpUserName2,*lpHive+COMPUTERNAMELEN+32,COMPUTERNAMELEN);
					CopyMemory(lpSystemtime2,(SYSTEMTIME FAR *)(*lpHive+COMPUTERNAMELEN*2+32),sizeof(SYSTEMTIME));
				}

				UI_AfterShot();
				bRet=TRUE;

			}
			CloseHandle(hFileWholeReg);
		} else {
			ErrMsg(lan_erroropenfile);
			bRet=FALSE;
		}

	} else {
		bRet=FALSE;
	};

	*(opfn.lpstrFile+opfn.nFileOffset)=0x00;
	strcpy(lpLastOpenDir,opfn.lpstrFile);

	MYFREE(opfn.lpstrFile);
	return(bRet);

}
