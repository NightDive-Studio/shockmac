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
* $Header: r:/prj/lib/src/res/rcs/res.c 1.25 1994/09/22 10:47:56 rex Exp $
* $Log: res.c $
 * Revision 1.25  1994/09/22  10:47:56  rex
 * Added secondary resdesc table, in shared buffer with first
 * 
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
#include <stdlib.h>
#include <string.h>

#include "lg.h"
#include "res.h"
#include "res_.h"
//#include <lzw.h>
//#include <memall.h>
//#include <_res.h>

//	The resource descriptor table

ResDesc *gResDesc;					// ptr to array of resource descriptors
ResDesc2 *gResDesc2;					// secondary array, shared buff with resdesc
Id resDescMax;							// max id in res desc
#define DEFAULT_RESMAX 1023		// default max resource id
#define DEFAULT_RESGROW 1024		// grow by blocks of 1024 resources
											// must be power of 2!
//	Some variables

ResStat resStat;						// stats held here
static bool resPushedAllocators;	// did we push our allocators?


//	---------------------------------------------------------
long SwapLongBytes(long in)
{
	long	out;
	*(uchar*)&out = *(((uchar *)&in)+3);
	*(((uchar*)&out)+1) = *(((uchar *)&in)+2);
	*(((uchar*)&out)+2) = *(((uchar *)&in)+1);
	*(((uchar*)&out)+3) = *(uchar *)&in;
	return(out);
}

short SwapShortBytes(short in)
{
	short out;
	*(uchar*)&out = *(((uchar *)&in)+1);
	*(((uchar*)&out)+1) = *(uchar *)&in;
	return(out);
}


//	---------------------------------------------------------
//		INITIALIZATION AND TERMINATION
//	---------------------------------------------------------
//
//	ResInit() initializes resource manager.

void ResInit()
{
	char *p;
	int i;
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
	gResDesc = (ResDesc *)malloc( (DEFAULT_RESMAX + 1) *
		(sizeof(ResDesc) + sizeof(ResDesc2)) );
	gResDesc2 = (ResDesc2 *) (gResDesc + (DEFAULT_RESMAX + 1));

	gResDesc[ID_HEAD].prev = 0;
	gResDesc[ID_HEAD].next = ID_TAIL;
	gResDesc[ID_TAIL].prev = ID_HEAD;
	gResDesc[ID_TAIL].next = 0;

//	Clear file descriptor array

	for (i = 0; i <= MAX_RESFILENUM; i++)
		resFile[i].fd = NULL;

//	Add directory pointed to by RES env var to search path
/*
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
	int i;

//	Close all open resource files

	for (i = 0; i <= MAX_RESFILENUM; i++)
		{
		if (resFile[i].fd != NULL)
			ResCloseFile(i);
		}

//	Spew out cumulative stats if want them

//	DBG(DSRC_RES_CumStat, {ResSpewCumStats();});

//	Free up resource descriptor table

	if (gResDesc)
		{
		free(gResDesc);
		gResDesc = NULL;
		gResDesc2 = NULL;
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
	long newAmt,currAmt;
	ResDesc2 *pNewResDesc2;

//	Calculate size of new table and size of current

	newAmt = (id + DEFAULT_RESGROW) & ~(DEFAULT_RESGROW - 1);
	currAmt = resDescMax + 1;

//	If need to grow, do it

	if (newAmt > currAmt)
		{
//		Spew(DSRC_RES_General,
//			("ResGrowResDescTable: extending to $%x entries\n", newAmt));

//	Realloc double-array buffer and check for error

		gResDesc = (ResDesc *)realloc(gResDesc, newAmt *
			(sizeof(ResDesc) + sizeof(ResDesc2)));
		if (gResDesc == NULL)
			{
//			Warning(("ResGrowDescTable: RES DESCRIPTOR TABLE BAD!!!\n"));
			return;
			}

//	Compute new location for gResDesc2[] array at top of buffer,
//	and move the gResDesc2[] array up there

		gResDesc2 = (ResDesc2 *) (gResDesc + currAmt);
		pNewResDesc2 = (ResDesc2 *) (gResDesc + newAmt);
		memmove(pNewResDesc2, gResDesc2, currAmt * sizeof(ResDesc2));
		gResDesc2 = pNewResDesc2;

//	Clear extra entries in both tables

		memset(gResDesc + currAmt, 0, (newAmt - currAmt) * sizeof(ResDesc));
		memset(gResDesc2 + currAmt, 0, (newAmt - currAmt) * sizeof(ResDesc2));

//	Set new max id limit

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

/*
//	---------------------------------------------------------
//
//	ResShrinkResDescTable() resizes the descriptor table to be
//	the minimum allowable size with the currently in-use resources.
//

void ResShrinkResDescTable()
{
	long newAmt,currAmt;
   // id is the largest used ID
   Id id;
	ResDesc2 *pNewResDesc2;

// Calculate largest used ID
   id = resDescMax;
   while ((id > ID_MIN) && (!ResInUse(id)))
      id--;
   Spew(DSRC_RES_General, ("largest ID in use is %x.\n",id));

//	Calculate size of new table and size of current

	newAmt = (id + DEFAULT_RESGROW) & ~(DEFAULT_RESGROW - 1);
	currAmt = resDescMax + 1;

//	If need to shrink do it
// note that we don't decrease the stat table

	if (currAmt > newAmt)
		{
		Spew(DSRC_RES_General,
			("ResGrowResDescTable: extending to $%x entries\n", newAmt));

//	Move gResDesc2[] array down to new place in buffer

		pNewResDesc2 = (ResDesc2 *) (gResDesc + newAmt);
		memmove(pNewResDesc2, gResDesc2, newAmt * sizeof(ResDesc2));

//	Shrink down double-array buffer

		gResDesc = Realloc(gResDesc, newAmt * sizeof(ResDesc));
		if (gResDesc == NULL)
			{
			Warning(("ResGrowDescTable: RES DESCRIPTOR TABLE BAD!!!\n"));
			return;
			}

//	Set new gResDesc2 ptr

		gResDesc2 = (ResDesc2 *) (gResDesc + newAmt);

//	Set new max id limit

		resDescMax = newAmt - 1;
      }
}
*/
