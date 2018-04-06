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
//		ResAcc.c		Resource access
//		Rex E. Bradford
/*
* $Header: r:/prj/lib/src/res/rcs/resacc.c 1.4 1994/08/30 15:18:20 rex Exp $
* $Log: resacc.c $
 * Revision 1.4  1994/08/30  15:18:20  rex
 * Made sure ResGet() returns NULL if ResLoadResource() did
 * 
 * Revision 1.3  1994/08/30  15:14:32  rex
 * Put in check for NULL return from ResLoadResource
 * 
 * Revision 1.2  1994/06/16  11:06:05  rex
 * Modified routines to handle LRU list better (keep locked and nodrop stuff out)
 * 
 * Revision 1.1  1994/02/17  11:23:31  rex
 * Initial revision
 * 
*/

#include <string.h>

#include "res.h"
#include "res_.h"

//	---------------------------------------------------------
//
//	ResLock() locks a resource and returns ptr.
//
//		id = resource id
//
//	Returns: ptr to locked resource
//	---------------------------------------------------------
//  For Mac version:  Change 'ptr' refs to 'hdl', lock resource handle and return ptr.

void *ResLock(Id id)
{
	ResDesc *prd;

	//	Check if valid id
//	DBG(DSRC_RES_ChkIdRef, {if (!ResCheckId(id)) return NULL;});

	//	Add to cumulative stats
//	CUMSTATS(id,numLocks);

	//	If resource not loaded, load it

	prd = RESDESC(id);
	if (ResLoadResource(id) == nil)
		return(nil);
//	else if (prd->lock == 0)
//		ResRemoveFromLRU(prd);

	//	Tally stats, check for over-lock

//	DBG(DSRC_RES_Stat, {if (prd->lock == 0) resStat.numLocked++;});

	//	Increment lock count, check for overlock

//	DBG(DSRC_RES_ChkLock, {if (prd->lock == RES_MAXLOCK) prd->lock--;});
	prd->lock++;

	//	Return ptr

	if (prd->lock == 1)
		HLock(prd->hdl);
	return(*prd->hdl);
}

//	---------------------------------------------------------
//
//	ResLockHi() Mac only!  Locks a resource, moves hi, and returns ptr.
//
//		id = resource id
//
//	Returns: ptr to locked resource
//	---------------------------------------------------------
void *ResLockHi(Id id)
{
	ResDesc *prd;

	//	If resource not loaded, load it.
	prd = RESDESC(id);
	if (ResLoadResource(id) == nil)
		return(nil);
	
	// Lock the handle.
	prd->lock++;
	if (prd->lock == 1)
		HLockHi(prd->hdl);
	
	return(*prd->hdl);
}

//	---------------------------------------------------------
//
//	ResUnlock() unlocks a resource.
//
//		id = resource id
//	---------------------------------------------------------
//  For Mac version:  Change 'ptr' refs to 'hdl', unlock resource handle.

void ResUnlock(Id id)
{
	ResDesc *prd;

	//	Check if valid id
//	DBG(DSRC_RES_ChkIdRef, {if (!ResCheckId(id)) return;});

	//	Check for under-lock

	prd = RESDESC(id);
//	DBG(DSRC_RES_ChkLock, {if (prd->lock == 0) { \
//		Warning(("ResUnlock: id $%x already unlocked\n", id)); return;} });

	//	Else decrement lock, if 0 move to tail and tally stats
	
	if (prd->lock > 0)
		prd->lock--;
	if (prd->lock == 0)
	{
		HUnlock(prd->hdl);
//		ResAddToTail(prd);
//		DBG(DSRC_RES_Stat, {resStat.numLocked--;});
	}
}

//	-------------------------------------------------------------
//
//	ResGet() gets a ptr to a resource
//
//		id = resource id
//
//	Returns: ptr to resource (ptr only guaranteed until next Malloc(),
//				Lock(), Get(), etc.
//	---------------------------------------------------------
//  For Mac version:  Change 'ptr' refs to 'hdl', lock resource handle and return ptr.

void *ResGet(Id id)
{
	ResDesc *prd = RESDESC(id);

//	Check if valid id

//	DBG(DSRC_RES_ChkIdRef, {if (!ResCheckId(id)) return NULL;});

//	Add to cumulative stats

//	CUMSTATS(id,numGets);

//	Load resource or move to tail

	if (ResLoadResource(id) == NULL)
		return(NULL);
	
//	ResAddToTail(prd);
//	if (prd->lock == 0)
//		ResMoveToTail(prd);

	//	Return ptr
	
	HLock(prd->hdl);
	return(*prd->hdl);
}


//	---------------------------------------------------------
//
//	ResExtract() extracts a resource from an open resource file.
//
//		id   = id
//		buff = ptr to buffer (use ResSize() to compute needed buffer size)
//
//	Returns: ptr to supplied buffer, or NULL if problem
//	---------------------------------------------------------
//  For Mac version:  Copies information from resource handle into the buffer.

void *ResExtract(Id id, void *buffer)
{
	ResDesc *prd = RESDESC(id);

	if (ResLoadResource(id) == NULL)
		return(NULL);
	
	HLock(prd->hdl);
	BlockMove(*prd->hdl, buffer, GetHandleSize(prd->hdl));
	HUnlock(prd->hdl);
	return(buffer);
	
/*
	//	Retrieve the data into the buffer, please

	if (ResRetrieve(id, buffer))
		{
		CUMSTATS(id,numExtracts);
		return(buffer);
		}

	//	If ResRetreive failed, return NULL ptr

	return(NULL);
*/
}


//	----------------------------------------------------------
//
//	ResDrop() drops a resource from memory for awhile.
//
//		id = resource id
//	----------------------------------------------------------
//  For Mac version:  Calls Resource Mgr function EmptyHandle to purge the handle.

void ResDrop(Id id)
{
	ResDesc *prd;

	//	Check for locked

//	DBG(DSRC_RES_ChkIdRef, {if (!ResCheckId(id)) return;});

	prd = RESDESC(id);
//	DBG(DSRC_RES_ChkLock, {if (prd->lock) \
//		Warning(("ResDrop: Block $%x is locked, dropping anyway\n", id));});
//	DBG(DSRC_RES_ChkLock, {if (prd->flags & RDF_NODROP) \
//		Warning(("ResDrop: Block $%x has NODROP flag set, dropping anyway\n", id));});

//	Spew(DSRC_RES_DelDrop, ("ResDrop: dropping $%x\n", id));

	//	Remove from LRU chain

//	if (prd->lock == 0)
//		ResRemoveFromLRU(prd);

	//	Tally stats

//	DBG(DSRC_RES_Stat, {resStat.totMemAlloc -= prd->size;
//		resStat.numLoaded--;
//		Spew(DSRC_RES_Stat, ("ResDrop: free %d, total now %d bytes\n",
//			prd->size, resStat.totMemAlloc));});

	//	Free memory and set ptr to NULL

	if (prd->hdl)
	{
		EmptyHandle(prd->hdl);
	}
}

//	-------------------------------------------------------
//
//	ResDelete() deletes a resource forever.
//
//		Id = id of resource
//	-------------------------------------------------------
//  For Mac version:  Call ReleaseResource on the handle and set its ref to null.
//  The next ResLoadResource on the resource will load it back in.

void ResDelete(Id id)
{
	ResDesc *prd;

	//	If locked, issue warning

//	DBG(DSRC_RES_ChkIdRef, {if (!ResCheckId(id)) return;});

	prd = RESDESC(id);
//	DBG(DSRC_RES_ChkLock, {if (prd->lock) \
//		Warning(("ResDelete: Block $%x is locked!\n", id));});

	//	If in use: if in ram, free memory & LRU, then in any case zap entry

//	if (prd->offset)
//	{
//		Spew(DSRC_RES_DelDrop, ("ResDelete: deleting $%x\n", id));
		if (prd->hdl)
		{
//			Spew(DSRC_RES_DelDrop, ("ResDelete: freeing memory for $%x\n", id));
//			DBG(DSRC_RES_Stat, {resStat.totMemAlloc -= prd->size;
//				resStat.numLoaded--;
//				Spew(DSRC_RES_Stat, ("ResDelete: free %d, total now %d bytes\n",
//					prd->size, resStat.totMemAlloc));});

			ReleaseResource(prd->hdl);				// release the resource.
			prd->hdl = NULL;

//			if (prd->lock == 0)
//				ResRemoveFromLRU(prd);
		}
		LG_memset(prd, 0, sizeof(ResDesc));
//	}

//	Else if not in use, spew to whoever's listening

//	else
//		{
//		Spew(DSRC_RES_DelDrop, ("ResDelete: $%x not in use\n", id));
//		}
}

/*
//	--------------------------------------------------------
//		INTERNAL ROUTINES
//	--------------------------------------------------------
//
//	ResCheckId() checks if id valid.
//
//		id = id to be checked
//
//	Returns: TRUE if id ok, FALSE if invalid & prints warning

bool ResCheckId(Id id)
{
	if (id < ID_MIN)
		{
		Warning(("ResCheckId: id $%x invalid\n", id));
		return FALSE;
		}
	if (id > resDescMax)
		{
		Warning(("ResCheckId: id $%x exceeds table\n", id));
		return FALSE;
		}
	return TRUE;
}
*/
