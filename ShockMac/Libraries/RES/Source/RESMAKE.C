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
//		ResMake.c		Resource making
//		Rex E. Bradford
/*
* $Header: n:/project/lib/src/res/rcs/resmake.c 1.2 1994/06/16 11:08:04 rex Exp $
* $Log: resmake.c $
 * Revision 1.2  1994/06/16  11:08:04  rex
 * Modified LRU list handling, lock resource made with ResMake() instead of
 * setting RDF_NODROP flag
 * 
 * Revision 1.1  1994/02/17  11:23:57  rex
 * Initial revision
 * 
*/

#include <string.h>

#include "res.h"
#include "res_.h"

//	--------------------------------------------------------
//
//	ResMake() makes a resource from a data block.
//
//		Id      = id of resource
//		ptr     = ptr to memory block (resource is not copied; this should
//					point to storage where the resource can live indefinitely)
//		size    = size of resource in bytes
//		type    = resource type (RTYPE_XXX)
//		filenum = file number
//		flags   = flags (RDF_XXX)
//	--------------------------------------------------------
//  For Mac version, use Resource Manager to add the resource to indicated res file.

void ResMake(Id id, void *ptr, long size, uchar type, short filenum, uchar flags)
{
	Handle		resHdl;
	ResDesc 	*prd;
	Str255		resName;
	
	// Check for resource at that id.  If the handle exists, then just change the
	// handle (adjusting for size if needed, of course).

	prd = RESDESC(id);
	if (prd->hdl)
	{
		ResLoadResource(id);
		if (GetHandleSize(prd->hdl) != size)
		{
			SetHandleSize(prd->hdl, size);
		}
		BlockMoveData(ptr, *prd->hdl, size);
	}
	else
	{
		// Make a handle out of the data and add the resource.

		UseResFile(filenum);							// Use the res file indicated.
	
		PtrToHand(ptr, &resHdl, size);			// Turn the pointer into a handle.
		
		resName[0] = 1;
		if ((flags & RDF_LZW) == 0)				// Figure out the resource name.
		{
			if (flags & RDF_COMPOUND)
				resName[1] = 'c';
			else
				resName[1] = 'n';
		}
		else
		{
			if (flags & RDF_COMPOUND)
				resName[1] = 'x';
			else
				resName[1] = 'z';
		}
		AddResource(resHdl, resMacTypes[type], id, resName);
		if (ResError())
			DebugStr("\pResMake: Can't add a resource.\n");
	
		ResExtendDesc(id);								//	Extend res desc table if need to
	
	//	Spew(DSRC_RES_Make, ("ResMake: making resource $%x\n", id));
	
		//	Add us to the soup, set lock so doesn't get swapped out
	
		prd->hdl = resHdl;
		prd->filenum = filenum;
		prd->lock = 1;
		prd->flags = flags;
		prd->type = type;
	}
}


//	---------------------------------------------------------------
//
//	ResMakeCompound() makes an empty compound resource
//
//		id      = id of resource
//		type    = resource type (RTYPE_XXX)
//		filenum = file number
//		flags   = flags (RDF_XXX, RDF_COMPOUND automatically added)

void ResMakeCompound(Id id, uchar type, short filenum, uchar flags)
{
	RefTable *prt;
	long sizeTable;

	//	Build empty compound resource in allocated memory

//	Spew(DSRC_RES_Make, ("ResMake: making compound resource $%x\n", id));

	sizeTable = REFTABLESIZE(0);
	prt = (RefTable *)NewPtrClear(sizeTable);
	prt->numRefs = 0;
	prt->offset[0] = sizeTable;

	//	Make a resource out of it

	ResMake(id, prt, sizeTable, type, filenum, flags | RDF_COMPOUND);
}

//	---------------------------------------------------------------
//
//	ResAddRef() adds an item to a compound resource.
//
//		ref      = reference
//		pitem    = ptr to item's data (copied from here, unlike simple resource)
//		itemSize = size of item
//	---------------------------------------------------------------
//  For Mac version:  Change references from 'ptr' to 'hdl'.  Use Mac memory allocating
//  routines.

void ResAddRef(Ref ref, void *pitem, long itemSize)
{
	ResDesc *prd;
	RefTable *prt;
	RefIndex index,i;
	long sizeItemOffsets,oldSize,sizeDiff, hdlSize;

	//	Error check

//	DBG(DSRC_RES_ChkIdRef, {if (!RefCheckRef(ref)) return;});

	//	Get vital info (and get into memory if not already)

//	Spew(DSRC_RES_Make, ("ResAddRef: adding ref $%x\n", ref));

	prd = RESDESC(REFID(ref));
	if (prd->hdl == nil)										// If there is no compound resource handle,
	{																	// try loading it.
		prt = (RefTable *)RefGet(ref);
		if (prt == nil)
		{
			DebugStr("\pResAddRef: Can't get the compound resource handle.\n");
		}
	}
	else																// If there is, then make a RefTable ptr to it.
	{
		HLock(prd->hdl);
		prt = (RefTable *)*prd->hdl;
	}
	hdlSize = GetHandleSize(prd->hdl);
	
	//	If index within current range of compound resource, replace or insert

	index = REFINDEX(ref);
	if (index < prt->numRefs)
	{
		oldSize = RefSize(prt, index);

		//	If same size, just copy in

		if (itemSize == oldSize)
		{
//			Spew(DSRC_RES_Make, ("ResAddRef: replacing same size ref\n"));
			LG_memcpy(REFPTR(prt,index), pitem, itemSize);
		}

//	Else if new item smaller, reduce offsets, shift data, insert new data

		else if (itemSize < oldSize)
		{
//			Spew(DSRC_RES_Make, ("ResAddRef: replacing larger ref\n"));
			sizeDiff = oldSize - itemSize;

			for (i = index + 1; i <= prt->numRefs; i++)
				prt->offset[i] -= sizeDiff;
			hdlSize -= sizeDiff;
			LG_memmove(REFPTR(prt,index + 1), REFPTR(prt,index + 1) + sizeDiff,
				prt->offset[prt->numRefs] - prt->offset[index + 1]);
			LG_memcpy(REFPTR(prt,index), pitem, itemSize);

			HUnlock(prd->hdl);
			SetHandleSize(prd->hdl, hdlSize);
			HLock(prd->hdl);
			prt = (RefTable *)*prd->hdl;

		}
		else		// New item is larger.
		{
//			Spew(DSRC_RES_Make, ("ResAddRef: replacing smaller ref\n"));
			sizeDiff = itemSize - oldSize;

			hdlSize += sizeDiff;
			HUnlock(prd->hdl);
			SetHandleSize(prd->hdl, hdlSize);
			HLock(prd->hdl);
			prt = (RefTable *)*prd->hdl;
			
			LG_memmove(REFPTR(prt, index + 1) + sizeDiff, REFPTR(prt, index + 1),
				prt->offset[prt->numRefs] - prt->offset[index + 1]);
			for (i = index + 1; i <= prt->numRefs; i++)
				prt->offset[i] += sizeDiff;
			LG_memcpy(REFPTR(prt,index), pitem, itemSize);
		}
	}

	//	Else if index exceeds current range, expand

	else
	{
//		Spew(DSRC_RES_Make, ("ResAddRef: extending compound resource\n"));

		// Extend resource for new offset(s) and data item

		sizeItemOffsets = sizeof(long) * ((index + 1) - prt->numRefs);

		hdlSize += sizeItemOffsets + itemSize;
		HUnlock(prd->hdl);
		SetHandleSize(prd->hdl, hdlSize);
		HLock(prd->hdl);
		prt = (RefTable *)*prd->hdl;

		//	Shift data upwards to make room for new offset(s)

		LG_memmove(REFPTR(prt,0) + sizeItemOffsets, REFPTR(prt,0),
			hdlSize - REFTABLESIZE(index + 1));

		//	Advance old offsets, set new ones

		for (i = 0; i <= prt->numRefs; i++)
			prt->offset[i] += sizeItemOffsets;
		for (i = prt->numRefs + 1; i <= index; i++)
			prt->offset[i] = prt->offset[prt->numRefs];
		prt->offset[index + 1] = prt->offset[index] + itemSize;

		//	Copy data into place, set new numRefs

		LG_memcpy(REFPTR(prt,index), pitem, itemSize);
		prt->numRefs = index + 1;
	}
	
	HUnlock(prd->hdl);
}


//	-------------------------------------------------------------
//
//	ResUnmake() removes a resource from the LRU list and sets its
//		ptr to NULL.  In this way, a program may take over management
//		of the resource data, and the RES system forgets about it.
//		This is typically done when user-managed data needs to be
//		written to a resource file, using ResMake(),ResWrite(),ResUnmake().
//
//		id = id of resource to unmake
//	--------------------------------------------------------
//  For Mac version: use ReleaseResource to free the handle (the pointer that
//  the handle was made from will still be around).  

void ResUnmake(Id id)
{
	ResDesc *prd;

	prd = RESDESC(id);
	if (prd->hdl)
	{
		ReleaseResource(prd->hdl);
//		prd->hdl = NULL;
//		LG_memset(prd, 0, sizeof(ResDesc));
	}
}

