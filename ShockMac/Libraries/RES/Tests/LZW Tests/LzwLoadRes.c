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
//=========================================================
//  PurgeTest.cp - Program to test the memory purging capabilities of the resource
//						    library.
//=========================================================

#include <stdlib.h>
#include <stdio.h>

#include "res.h"

//--------------------------------------
//  Prototypes
//--------------------------------------
void TestListRes(short filenum);
void TestExtractRefs(short filenum);
void TestGetRefs(short filenum);
void main(void);
void DumpBlock(Ptr p, short psize);


//---------------------------------------------------------
//  Main routine.
//---------------------------------------------------------
void main(void)
{
	StandardFileReply	reply;
	SFTypeList				typeList;
	short						filenum;

	ResInit();
	
	printf("Open a resource file...\n");
	
	typeList[0] = 'Sgam';
	typeList[1] = 'rsrc';
	StandardGetFile(nil, 2, typeList, &reply);
	if (!reply.sfGood)
		return;
	else
	{
		filenum = ResOpenFile(&reply.sfFile);
		printf("Opened file, filenum = %d\n", filenum);
	}

	printf("\nLoaded Resources...\n\n");
	TestListRes(filenum);
	
	printf("\nGetting refs...\n\n");
	TestGetRefs(filenum);
	
	printf("\nExtracting refs...\n\n");
	TestExtractRefs(filenum);
	
	printf("\nClosing file.\n");
	ResCloseFile(filenum);
	ResTerm();
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
			ResLock(id);
			rs = ResSize(id);
			ResUnlock(id);
			printf("%6d    %4.4s   0x%08x    %6d   0x%04x\n", 
						id, (char *)&resMacTypes[prd->type], prd->hdl, rs, prd->flags);
		}
	}
}

//----------------------------------------------------------------------------------
//  Get and dump the references in the compound resource.
//----------------------------------------------------------------------------------
void TestGetRefs(short filenum)
{
	int		i;
	Ptr		p;

	for (i = 0; i < 3; i++)
	{
		p = RefGet(MKREF(1002, i));
		printf("Got Ref %d\n", i);
		DumpBlock(p, 100);
	}
}

//----------------------------------------------------------------------------------
//  Extract the references in the compound resource.
//----------------------------------------------------------------------------------
void TestExtractRefs(short filenum)
{
	int		i, rfs;
	Ptr		p;
	RefTable *rt;

	rt = ResReadRefTable(1002);
	
	for (i = 0; i < 3; i++)
	{
		rfs = RefSize(rt, i);
		p = NewPtrClear(rfs);
		RefExtract(rt, MKREF(1002, i), p);
		printf("Ref %d\n", i);
		DumpBlock(p, rfs);
		DisposePtr(p);
	}
	
	ResFreeRefTable(rt);
}

//----------------------------------------------------------------------------------
//  Dumps the contents of a pointer.
//----------------------------------------------------------------------------------
void DumpBlock(Ptr p, short psize)
{
	short 	c = 1;
	Ptr		cur = p;
	uchar	ch;
	
	while ((cur - p) < psize)
	{
		ch = *cur;
		printf("$%x ", ch);
		c++;
		cur++;
		if (c > 8)
		{ 
			printf("\n");
			c = 1;
		}
		else if ((cur - p) >= psize)
			printf("\n");
	}
}
