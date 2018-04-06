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
int RestestHeapWalk();
void TestCreateFile(FSSpec *specPtr);
void TestDumpFile(FSSpec *specPtr);
void TestEditFile(FSSpec *specPtr);
void TestSpin(FSSpec *specPtr);
void TestRefExtract(FSSpec *specPtr);
void DumpBlock(Ptr p, short psize);

//----------------------------------------------------------------------------------
//  Main routine.
//----------------------------------------------------------------------------------
void main()
{
	char				ans[10];
	char				c;
	StandardFileReply	reply;
	SFTypeList			typeList;

	typeList[0] = 'Sgam';

	ResInit();

LOOP:
	printf("\n(C)reate, (D)ump, (E)dit, (R)ef Extract, (S)pin, (Q)uit : ");
	fgets (ans, sizeof(ans), stdin);
	c = toupper(ans[0]);
	switch (c)
	{
		case 'C':
			StandardPutFile("\pCreate a resource file:", "\pTest Res File", &reply);
			if (reply.sfGood)
				TestCreateFile(&reply.sfFile);
			break;
		case 'D':
			StandardGetFile(nil, 1, typeList, &reply);
			if (reply.sfGood)
				TestDumpFile(&reply.sfFile);
			break;
		case 'E':
			StandardGetFile(nil, 1, typeList, &reply);
			if (reply.sfGood)
				TestEditFile(&reply.sfFile);
			break;
		case 'R':
			StandardGetFile(nil, 1, typeList, &reply);
			if (reply.sfGood)
				TestRefExtract(&reply.sfFile);
			break;
		case 'S':
			StandardGetFile(nil, 1, typeList, &reply);
			if (reply.sfGood)
				TestSpin(&reply.sfFile);
			break;
		case 'Q':
		case 27:
			ResTerm();
			exit(0);
	}
	goto LOOP;
}

//----------------------------------------------------------------------------------
//  Create a test file, add some resources to it.
//----------------------------------------------------------------------------------
void TestCreateFile(FSSpec *specPtr)
{
static uchar data1[] = {0x10,0x11,0x12,0x13,0x14,0x15};
static uchar data2[] = {
	0x99,0x98,0x97,0x96,0x99,0x98,0x97,0x96,
	0x99,0x98,0x97,0x96,0x99,0x98,0x97,0x96,
	0x99,0x98,0x97,0x96,0x99,0x98,0x97,0x96,
	0x99,0x98,0x97,0x96,0x99,0x98,0x97,0x96,
	0x99,0x98,0x97,0x96,0x99,0x98,0x97,0x96,
	0x99,0x98,0x97,0x96,0x99,0x98,0x97,0x96,
	0x99,0x98,0x97,0x96,0x99,0x98,0x97,0x96,
	0x99,0x98,0x97,0x96,0x99,0x98,0x97,0x96,
	0x99,0x98,0x97,0x96,0x99,0x98,0x97,0x96,
};
static uchar data3[] = {
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,
	0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
	0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,
};
static uchar data4[] = {0x48,0x48,0x48,0x48};
//static uchar data5[] = {0x38,0x38};
static uchar data5[] = {0x25,0x25,0x25,0x26,0x26,0x26};

	short 	filenum;
	Ptr		p;

	filenum = ResCreateFile(specPtr);
	printf("filenum = %d\n", filenum);

	ResSetComment(filenum, "This is a test\nresource file\n");

	p = NewPtr(sizeof(data1));
	memcpy(p, data1, sizeof(data1));
	ResMake(0x100, p, sizeof(data1), RTYPE_UNKNOWN, filenum, 0);

	p = NewPtr(sizeof(data2));
	memcpy(p, data2, sizeof(data2));
	ResMake(0x101, p, sizeof(data2), RTYPE_UNKNOWN, filenum, RDF_LZW);
	printf("resources added\n");

	ResMakeCompound(0x102, RTYPE_IMAGE, filenum, 0);
	ResAddRef(MKREF(0x102,0), data3, sizeof(data3));
	ResAddRef(MKREF(0x102,14), data4, sizeof(data4));
	ResAddRef(MKREF(0x102,14), data5, sizeof(data5));
	ResAddRef(MKREF(0x102,16), data4, sizeof(data4));
	printf("compound resource made\n");

	ResWrite(0x100);
	ResWrite(0x101);
	ResWrite(0x102);
	printf("resources written\n");

	ResCloseFile(filenum);
	printf("resource file closed\n");
}
/*
void TestDumpBlockDumper(void *buff, long numBytes, long iblock)
{
	char *p = buff;

	printf("block: %d (num: %d)   bytes: $%x $%x $%x $%x ...\n",
		iblock, numBytes, *p, *(p+1), *(p+2), *(p+3));
}
*/

//----------------------------------------------------------------------------------
//  Dump the bytes in each resource.
//----------------------------------------------------------------------------------
void TestDumpFile(FSSpec *specPtr)
{
	int 	filenum;
	Id 		id;
	Ptr		p;
	int 	i, rs;
	ResDesc	*prd;

	printf("opening file\n");
	filenum = ResOpenFile(specPtr);
	printf("filenum = %d\n", filenum);

	for (id = ID_MIN; id <= resDescMax; id++)
	{
		prd = RESDESC(id);
		if (prd->filenum == filenum)
		{
			ResLock(id);
			rs = ResSize(id);
			p = NewPtr(rs);
			ResExtract(id, p);
			ResUnlock(id);
			printf("Res $%x (size %d):\n", id, rs);
			DumpBlock((Ptr)p, rs);
		}
	}


/*
	printf("Reading resource in blocks\n");
	ResExtractInBlocks(0x100, buff, 4, TestDumpBlockDumper);
*/
	ResCloseFile(filenum);
}


//----------------------------------------------------------------------------------
//  Edit a res file (add and delete resources).
//----------------------------------------------------------------------------------
void TestEditFile(FSSpec *specPtr)
{
	int		filenum;
	char	ans[10];
	int		c;
	Id 		id;
	Ptr 	buff;

	filenum = ResEditFile(specPtr, TRUE);
	if (filenum < 0)
	{
		printf("Error return: %d");
		return;
	}
	printf("File opened at filenum: %d\n", filenum);
	ResSetComment(filenum, "This file edited using ResEditFile");
//	ResAutoPackOff(filenum);

LOOP:
	printf("(A)dd, (K)ill, (C)lose : ");
	fgets (ans, sizeof(ans), stdin);
	switch (toupper(ans[0]))
	{
		case 'A':
			printf("Hit char for id : ");
			fgets(ans, sizeof(ans), stdin);
			c = atoi(ans);
			id = 0x100 + c;
			buff = NewPtr(16);
			memset(buff, c, 16);
			ResMake(id, buff, 13, 0, filenum, 0);
			c = ResWrite(id);
			printf("Wrote %d bytes\n", c);
			break;

		case 'K':
			printf("Hit char for id : ");
			fgets(ans, sizeof(ans), stdin);
			c = atoi(ans);
			id = 0x100 + c;
			ResKill(id);
			break;

		case 'C':
			ResCloseFile(filenum);
			return;
		}
	goto LOOP;
}


//----------------------------------------------------------------------------------
//  Who know what this does?  At least it doesn't crash.
//----------------------------------------------------------------------------------
void TestSpin(FSSpec *specPtr)
{
	int filenum,i;
	Id id;
	uchar *p;

	printf("opening file\n");
	filenum = ResOpenFile(specPtr);
	printf("filenum = %d\n", filenum);

	for (i = 0; i < 1000; i++)
	{
		for (id = 0x100; id <= 0x101; id++)
		{
			p = (uchar *)ResLock(id);
			if (i == 999)
				printf("$%x: $%x $%x ... (size %d)\n", id, *p, *(p+1), ResSize(id));
			ResUnlock(id);
		}
	}

	ResCloseFile(filenum);
}


//----------------------------------------------------------------------------------
//  Extract data from a resource using two different methods.
//----------------------------------------------------------------------------------
void TestRefExtract(FSSpec *specPtr)
{
	int		filenum;
	char	ans[10];
	int		c, rfs;
	Ref		rid;
	Ptr		p, cur;
	RefTable *rt;
	
	filenum = ResEditFile(specPtr, TRUE);
	if (filenum < 0)
	{
		printf("Error return: %d");
		return;
	}
	printf("File opened at filenum: %d\n", filenum);

	printf("Extract method (1) or (2) or (L)ist table? ");
	fgets (ans, sizeof(ans), stdin);
	switch (ans[0])
	{
		case 'l':
		case 'L':
   			rt = ResReadRefTable(0x102);
			for (c = 0; c <= rt->numRefs; c++)
				printf("Index:%d, Offset:%d\n", c, rt->offset[c]);
			break;
		
		case '1':
			printf("Ref index: ");
			fgets(ans, sizeof(ans), stdin);
			c = atoi(ans);
			rid = MKREF(0x102,c);
			p = (Ptr)RefLock(rid);
			// Copy into your own buffer here.
			RefUnlock(rid);
			break;

		case '2':
			printf("Ref index: ");
			fgets(ans, sizeof(ans), stdin);
			c = atoi(ans);
			rid = MKREF(0x102,c);
			
   			rt = ResReadRefTable(REFID(rid));
   			rfs = RefSize(rt,REFINDEX(rid));
   			p = NewPtrClear(rfs);
  			RefExtract(rt, rid, p);
   			ResFreeRefTable(rt);
			
			DumpBlock(p, rfs);

			DisposePtr(p);
			break;
	}
	ResCloseFile(filenum);
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

/*
int RestestHeapWalk()
{
	struct _heapinfo hinfo;
	int hstat;

//	Walk heap, collecting info

	hinfo._pentry = NULL;
	while (TRUE)
		{
		hstat = _heapwalk(&hinfo);
		printf("Heapwalk: pentry: $%Fp, size: %d, use: %d (stat = %d)\n",
			hinfo._pentry, hinfo._size, hinfo._useflag, hstat);
		if (hstat != _HEAPOK)
			break;
		}

//	Return 0 or error (-1)

	if (hstat != _HEAPEND)
		{
		Warning(("MemStats: heap bad\n"));
		return(-1);
		}
	return(0);
}

#include "lzw.h"

void DoesItCompile()
{
	LzwCompressBuff2Buff(0,0,0,0);
	LzwCompressBuff2Fd(0,0,0);
	LzwCompressBuff2Fp(0,0,0);
	LzwCompressBuff2Null(0,0);
	LzwCompressBuff2User(0,0,0,0,0,0);
	LzwCompressFd2Buff(0,0,0,0);
	LzwCompressFd2Fd(0,0,0);
	LzwCompressFd2Fp(0,0,0);
	LzwCompressFd2Null(0,0);
	LzwCompressFd2User(0,0,0,0,0,0);
	LzwCompressFp2Buff(0,0,0,0);
	LzwCompressFp2Fd(0,0,0);
	LzwCompressFp2Fp(0,0,0);
	LzwCompressFp2Null(0,0);
	LzwCompressFp2User(0,0,0,0,0,0);
	LzwCompressUser2Buff(0,0,0,0,0,0);
	LzwCompressUser2Fd(0,0,0,0,0);
	LzwCompressUser2Fp(0,0,0,0,0);
	LzwCompressUser2Null(0,0,0,0);
	LzwCompressUser2User(0,0,0,0,0,0,0,0);

	LzwExpandBuff2Buff(0,0,0,0);
	LzwExpandBuff2Fd(0,0,0,0);
	LzwExpandBuff2Fp(0,0,0,0);
	LzwExpandBuff2Null(0,0,0);
	LzwExpandBuff2User(0,0,0,0,0,0);
	LzwExpandFd2Buff(0,0,0,0);
	LzwExpandFd2Fd(0,0,0,0);
	LzwExpandFd2Fp(0,0,0,0);
	LzwExpandFd2Null(0,0,0);
	LzwExpandFd2User(0,0,0,0,0,0);
	LzwExpandFp2Buff(0,0,0,0);
	LzwExpandFp2Fd(0,0,0,0);
	LzwExpandFp2Fp(0,0,0,0);
	LzwExpandFp2Null(0,0,0);
	LzwExpandFp2User(0,0,0,0,0,0);
	LzwExpandUser2Buff(0,0,0,0,0,0);
	LzwExpandUser2Fd(0,0,0,0,0,0);
	LzwExpandUser2Fp(0,0,0,0,0,0);
	LzwExpandUser2Null(0,0,0,0,0);
	LzwExpandUserUser(0,0,0,0,0,0,0,0);
}
*/
