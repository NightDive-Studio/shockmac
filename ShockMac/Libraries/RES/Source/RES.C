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
//		Res.C		Resource Manager primary access routines
//		Rex E. Bradford (REX)
//
//		See the doc RESOURCE.DOC for information.
/*
* $Header: r:/prj/lib/src/res/rcs/res.c 1.24 1994/07/15 18:19:33 xemu Exp $
* $Log: res.c $
 * Revision 1.24  1994/07/15  18:19:33  xemu
 * added ResShrinkResDescTable
 * 
 * Revision 1.23  1994/05/26  13:51:55  rex
 * Added ResInstallPager(ResDefaultPager) to ResInit()
 * 
 * Revision 1.22  1994/02/17  11:24:51  rex
 * Moved most funcs out into other .c files
 * 
*/

//#include <io.h>
//#include <stdlib.h>
#include <string.h>

//#include <lg.h>
#include "res.h"
#include "res_.h"
//#include <lzw.h>
//#include <memall.h>
//#include <_res.h>

//	The resource descriptor table

ResDesc *gResDesc;								// ptr to array of resource descriptors
Id resDescMax;									// max id in res desc
#define DEFAULT_RESMAX 1023			// default max resource id
#define DEFAULT_RESGROW 1024		// grow by blocks of 1024 resources
														// must be power of 2!
//	Some variables
/*
ResStat resStat;						// stats held here
static bool resPushedAllocators;	// did we push our allocators?
*/

//	---------------------------------------------------------
//		INITIALIZATION AND TERMINATION
//	---------------------------------------------------------
//
//	ResInit() initializes resource manager.

void ResInit()
{
//	char *p;
//	int i;
/*
//	We must exit cleanly

	AtExit(ResTerm);

//	Set memory allocator, init LZW system

	MemPushAllocator(ResMalloc, ResRealloc, ResFree);
	resPushedAllocators = TRUE;
	LzwInit();
*/

//	Allocate initial resource descriptor table, default size (can't fail)

	resDescMax = DEFAULT_RESMAX;
	gResDesc = (ResDesc *)NewPtrClear( (DEFAULT_RESMAX + 1) * sizeof(ResDesc) );
	if (MemError())
		DebugStr("\pResInit: Can't allocate the global resource descriptor table.\n");
	
//	gResDesc[ID_HEAD].prev = 0;
//	gResDesc[ID_HEAD].next = ID_TAIL;
//	gResDesc[ID_TAIL].prev = ID_HEAD;
//	gResDesc[ID_TAIL].next = 0;

/*
//	Clear file descriptor array

	for (i = 0; i <= MAX_RESFILENUM; i++)
		resFile[i].fd = -1;

//	Add directory pointed to by RES env var to search path

	p = getenv("RES");
	if (p)
		ResAddPath(p);

	Spew(DSRC_RES_General, ("ResInit: res system initialized\n"));

//	Install default pager

	ResInstallPager(ResDefaultPager);
*/
}

//	---------------------------------------------------------
//
//	ResTerm() terminates resource manager.

void ResTerm()
{
//	int i;
/*
//	Close all open resource files

	for (i = 0; i <= MAX_RESFILENUM; i++)
		{
		if (resFile[i].fd >= 0)
			ResCloseFile(i);
		}

//	Spew out cumulative stats if want them

	DBG(DSRC_RES_CumStat, {ResSpewCumStats();});
*/

//	Free up resource descriptor table

	if (gResDesc)
	{
		DisposePtr((Ptr)gResDesc);
		gResDesc = NULL;
		resDescMax = 0;
	}
/*
//	Pop allocators

	if (resPushedAllocators)
		{
		MemPopAllocator();
		resPushedAllocators = FALSE;
		}

//	We're outta here

	Spew(DSRC_RES_General, ("ResTerm: res system terminated\n"));
*/
}

//	---------------------------------------------------------
//  For Mac version:  This is now a function, because we have to check a few things
//  before returning the size.
//	---------------------------------------------------------
long ResSize(Id id)
{
	ResDesc *prd = RESDESC(id);

	if (prd->flags & RDF_LZW)							// If it's compressed...
	{
		if (*prd->hdl)										// if it's a real handle
			return(GetHandleSize(prd->hdl));		// return the handle size
		else
			return (MaxSizeRsrc(prd->hdl));		// else return the res map size.
	}
	else
		return (MaxSizeRsrc(prd->hdl));			// For normal resources, 
}																	// return the res map size.

//	---------------------------------------------------------
//
//	ResGrowResDescTable() grows resource descriptor table to
//	handle a new id.
//
//	This routine is normally called internally, but a client
//	program may call it directly too.
//
//		id = id

void ResGrowResDescTable(Id id)
{
	long	newAmt, currAmt;
	Ptr	growPtr;

//	Calculate size of new table and size of current

	newAmt = (id + DEFAULT_RESGROW) & ~(DEFAULT_RESGROW - 1);
	currAmt = resDescMax + 1;

//	If need to grow, do it, clearing new entries

	if (newAmt > currAmt)
	{
//		Spew(DSRC_RES_General,
//			("ResGrowResDescTable: extending to $%x entries\n", newAmt));

//		SetPtrSize((Ptr)gResDesc, newAmt * sizeof(ResDesc));
		growPtr = NewPtr(newAmt * sizeof(ResDesc));
		if (MemError() != noErr)
		{
			DebugStr("\pResGrowDescTable: CANT GROW DESCRIPTOR TABLE!!!\n");
			return;
		}
		BlockMove(gResDesc, growPtr, currAmt * sizeof(ResDesc));
		DisposePtr((Ptr)gResDesc);
		gResDesc = (ResDesc *)growPtr;
		LG_memset(gResDesc + currAmt, 0, (newAmt - currAmt) * sizeof(ResDesc));
		resDescMax = newAmt - 1;

//	Grow cumulative stats table too
/*
		DBG(DSRC_RES_CumStat, {
			if (pCumStatId == NULL)
				ResAllocCumStatTable();
			else
				{
				pCumStatId = Realloc(pCumStatId, newAmt * sizeof(ResCumStat));
				if (pCumStatId == NULL)
					{
					Warning(("ResGrowDescTable: RES CUMSTAT TABLE BAD!!!\n"));
					return;
					}
				memset(pCumStatId + currAmt, 0, (newAmt - currAmt) *
					sizeof(ResCumStat));
				}
			});
*/
	}
}

//	---------------------------------------------------------
//
//	ResShrinkResDescTable() resizes the descriptor table to be
//	the minimum allowable size with the currently in-use resources.
//
/*
void ResShrinkResDescTable()
{
	long newAmt,currAmt;
   // id is the largest used ID
   Id id;

// Calculate largest used ID
   id = resDescMax;
   while ((id > ID_MIN) && (!ResInUse(id)))
      id--;
//   Spew(DSRC_RES_General, ("largest ID in use is %x.\n",id));

//	Calculate size of new table and size of current

	newAmt = (id + DEFAULT_RESGROW) & ~(DEFAULT_RESGROW - 1);
	currAmt = resDescMax + 1;

//	If need to shrink do it
// note that we don't increase the stat table

	if (currAmt > newAmt)
	{
//		Spew(DSRC_RES_General,
//			("ResGrowResDescTable: extending to $%x entries\n", newAmt));

		SetPtrSize(gResDesc, newAmt * sizeof(ResDesc));
		if (MemError() != noErr)
		{
//еее			Warning(("ResGrowDescTable: RES DESCRIPTOR TABLE BAD!!!\n"));
			return;
		}
		resDescMax = newAmt - 1;
      }
}
*/
