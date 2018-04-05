/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
*/
//		Restest.C	Resource system tester
//		Rex E. Bradford (REX)
/*
* $Header: n:/project/lib/src/res/rcs/restest.c 1.7 1994/05/26 13:54:14 rex Exp $
* $log$
*/

//#include <fcntl.h>
//#include <sys\stat.h>
//#include <io.h>
//#include <conio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#include <lg.h>
#include "res.h"
//#include <mprintf.h>
//#include <_res.h>

//--------------------------------------
//  Prototypes
//--------------------------------------
void TestCreateFile(short filenum);
void TestListRes(short filenum);

	
//--------------------------------------
//  Globals
//--------------------------------------
Handle	textHdl;
Handle	pictHdl;
Ptr		pRef1;
Ptr		pRef2;
Ptr		pRef3;

//----------------------------------------------------------------------------------
//  Main routine.
//----------------------------------------------------------------------------------
void main()
{
	StandardFileReply	reply;
	short 					filenum;

	textHdl = GetResource('TEXT', 128);
	if (textHdl == NULL)
	{
		printf("Couldn't get the text resource.\n");
		return;
	}
	pictHdl = GetResource('PICT', 128);
	if (pictHdl == NULL)
	{
		printf("Couldn't get the picture resource.\n");
		return;
	}
	
	pRef1 = NewPtr(100);
	if (pRef1 == NULL)
	{
		printf("Couldn't allocate pRef1.\n");
		return;
	}
	memset(pRef1, '1', 100);
	
	pRef2 = NewPtr(200);
	if (pRef2 == NULL)
	{
		printf("Couldn't allocate pRef2.\n");
		return;
	}
	memset(pRef2, '2', 200);
	
	pRef3 = NewPtr(300);
	if (pRef3 == NULL)
	{
		printf("Couldn't allocate pRef3.\n");
		return;
	}
	memset(pRef3, '3', 300);

	ResInit();

	printf("Create some compressed resources...\n");
	
	StandardPutFile("\pCreate a resource file:", "\pLZW Create Test", &reply);
	if (reply.sfGood)
	{
		filenum = ResCreateFile(&reply.sfFile);
		printf("Created file, filenum = %d\n", filenum);

		TestCreateFile(filenum);
		TestListRes(filenum);

//		ResWriteDir(filenum);
//		printf("Resource table written.\n");
		ResCloseFile(filenum);
		printf("Resource file closed.\n");
	}
	
	ResTerm();
}

//----------------------------------------------------------------------------------
//  Create a test file, add some resources to it.
//----------------------------------------------------------------------------------
void TestCreateFile(short filenum)
{
	HLock(textHdl);
	ResMake(1000, *textHdl, GetHandleSize(textHdl), RTYPE_UNKNOWN, filenum, RDF_LZW);
	HUnlock(textHdl);
	printf("Text resource made.\n");

	HLock(pictHdl);
	ResMake(1001, *pictHdl, GetHandleSize(pictHdl), RTYPE_PICT, filenum, RDF_LZW);
	HUnlock(pictHdl);
	printf("Picture resource made.\n");

	ResMakeCompound(1002, RTYPE_IMAGE, filenum, RDF_LZW);
	ResAddRef(MKREF(1002,0), pRef1, 100);
	ResAddRef(MKREF(1002,1), pRef2, 200);
	ResAddRef(MKREF(1002,2), pRef3, 300);
	printf("Compound resource made.\n");

	ResWrite(1000);
	ResWrite(1001);
	ResWrite(1002);
	printf("Resources written.\n");
}

//----------------------------------------------------------------------------------
//  Dump the bytes in each resource.
//----------------------------------------------------------------------------------
void TestListRes(short filenum)
{
	Id 				id;
	int 			i, rs;
	ResDesc		*prd;

	printf("Resource list...\n\n");

	for (id = ID_MIN; id <= resDescMax; id++)
	{
		prd = RESDESC(id);
		if (prd->filenum == filenum)
		{
//			ResLock(id);
			rs = ResSize(id);
//			ResUnlock(id);
			printf("%6d    %4.4s   0x%08x    %6d   0x%04x\n", 
						id, (char *)&resMacTypes[prd->type], prd->hdl, rs, prd->flags);
		}
	}
}
