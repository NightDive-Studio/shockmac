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
//		ResLoad.c	Load resource from resfile
//		Rex E. Bradford
/*
* $Header: n:/project/lib/src/res/rcs/resload.c 1.5 1994/06/16 11:07:44 rex Exp $
* $Log: resload.c $
 * Revision 1.5  1994/06/16  11:07:44  rex
 * Took LRU list adding out of ResLoadResource()
 * 
 * Revision 1.4  1994/05/26  13:52:32  rex
 * Surrounded Malloc() for loading resource with setting of idBeingLoaded,
 * so installable pager can make use of this.
 * 
 * Revision 1.3  1994/04/19  16:40:28  rex
 * Added check for 0-size resource
 * 
 * Revision 1.2  1994/03/14  16:10:47  rex
 * Added id to spew in ResLoadResource()
 * 
 * Revision 1.1  1994/02/17  11:23:39  rex
 * Initial revision
 * 
*/

//#include <io.h>

#include "res.h"
#include "res_.h"
#include "lzw.h"
//#include <_res.h>


//-------------------------------
//  Private Prototypes
//-------------------------------
void LoadCompressedResource(ResDesc *prd, Id id);


//	-----------------------------------------------------------
//
//	ResLoadResource() loads a resource object, decompressing it if it is
//		compressed.
//
//		id = resource id
//	-----------------------------------------------------------
//  For Mac version:  Call Resource Mgr call LoadResource.

void *ResLoadResource(Id id)
{
	ResDesc *prd = RESDESC(id);

	//	If doesn't exit, forget it

//	DBG(DSRC_RES_ChkIdRef, {if (!ResInUse(id)) return NULL;});
//	DBG(DSRC_RES_ChkIdRef, {if (!ResCheckId(id)) return NULL;});

//	Spew(DSRC_RES_Read, ("ResLoadResource: loading $%x\n", id));

	//	Allocate memory, setting magic id so pager can tell who it is if need be.

//	idBeingLoaded = id;
//	prd->ptr = Malloc(prd->size);
//	idBeingLoaded = ID_NULL;
//	if (prd->ptr == NULL)
//		return(NULL);

	//	Tally memory allocated to resources

//	DBG(DSRC_RES_Stat, {resStat.totMemAlloc += prd->size;});
//	Spew(DSRC_RES_Stat, ("ResLoadResource: loading id: $%x, alloc %d, total now %d bytes\n",
//		id, prd->size, resStat.totMemAlloc));

	//	Add to cumulative stats

//	CUMSTATS(id,numLoads);

	//	Load from disk

	if (prd->flags & RDF_LZW)
	{
		LoadCompressedResource(prd, id);
	}
	else
	{
		if (prd->hdl == nil)
			prd->hdl = GetResource(resMacTypes[prd->type], id);
		else if (*prd->hdl == nil)
			LoadResource(prd->hdl);
	}
//	ResRetrieve(id, nil);

	//	Tally stats

//	DBG(DSRC_RES_Stat, {resStat.numLoaded++;});

	//	Return handle

	return(prd->hdl);
}

//	---------------------------------------------------------
//  Load a compressed resource.  It's different enough for the Mac version to
//  warrant its own function.
//
//  In order to keep prd->hdl as the resource handle (that is, the handle associated
//  with the resource map), we have to be a little non-obvious.  First, we'll get
//  the compressed data from the resource file.  Next we copy the compressed data
//  into another handle of the same size.  Then we determine how large the expanded
//  data will be, and set prd->hdl to that size.  Finally, we expand the data into
//  prd->hdl.
//	---------------------------------------------------------
void LoadCompressedResource(ResDesc *prd, Id id)
{
	Handle		mirrorHdl;
	Ptr			resPtr, expPtr;
	long			exlen;
	long			tableSize = 0;
	ushort		numRefs;
	
	if (prd->hdl != NULL && *prd->hdl != NULL)	// If everything's still in
		return;														// memory, there's no need to load.
			
	// Get the compressed resource from disk.
	prd->hdl = GetResource(resMacTypes[prd->type], id);
	if (prd->hdl == NULL)
	{
		DebugStr("\pLoadCompressedResource: Can't get compressed resource.\n");
		return;
	}
	
	// Copy the compressed info into the mirror handle.
	exlen = GetHandleSize(prd->hdl);
	mirrorHdl = NewHandle(exlen);
	if (mirrorHdl == NULL)
	{
		DebugStr("\pLoadCompressedResource: Can't allocate mirror handle.\n");
		return;
	}
	BlockMoveData(*prd->hdl, *mirrorHdl, exlen);

	// Point to the beginning of the compressed data in the mirror handle.
	HLock(mirrorHdl);
	resPtr = *mirrorHdl;
	
	// Determine the expanded buffer size.
	if (prd->flags & RDF_COMPOUND)
	{
		numRefs = *(short *)resPtr;
		tableSize = REFTABLESIZE(numRefs);
		resPtr += tableSize;
	}
	exlen = LzwExpandBuff2Null(resPtr, 0, 0);

	// Set prd->hdl to the expanded buffer size.
	SetHandleSize(prd->hdl, exlen + tableSize);
	if (MemError())
	{
		DebugStr("\pLoadCompressedResource: Can't resize resource handle for expansion.\n");
		return;
	}
	
	// Point to the beginning of the expanded data handle.
	HLock(prd->hdl);
	expPtr = *(prd->hdl);
	
	// Expand-o-rama!  If a compound resource, copy the uncompressed refTable
	// over first.
	if (prd->flags & RDF_COMPOUND)
	{
		BlockMoveData(resPtr-tableSize, expPtr, tableSize);
		expPtr += tableSize;
	}
	exlen = LzwExpandBuff2Buff(resPtr, expPtr, 0, 0);
	if (exlen < 0)
	{
		DebugStr("\pLoadCompressedResource: Can't expand resource.\n");
		return;
	}
	
	HUnlock(prd->hdl);								// Unlock the buffers.
	HUnlock(mirrorHdl);
	DisposeHandle(mirrorHdl);						// Free the mirror buffer.
}

/*
//	---------------------------------------------------------
//
//	ResRetrieve() retrieves a resource from disk.
//
//		id     = id of resource
//		buffer = ptr to buffer to load into (must be big enough)
//
//	Returns: TRUE if retrieved, FALSE if problem

bool ResRetrieve(Id id, void *buffer)
{
	ResDesc *prd;
//	int fd;
//	uchar *p;
//	long size;
//	RefIndex numRefs;

	//	Check id and file number

//	DBG(DSRC_RES_ChkIdRef, {if (!ResCheckId(id)) return FALSE;});
	prd = RESDESC(id);
//	fd = resFile[prd->filenum].fd;
//	DBG(DSRC_RES_ChkIdRef, {if (fd < 0) { \
//		Warning(("ResRetrieve: id $%x doesn't exist\n", id)); \
//		return FALSE; \
//		}});
	
	//	Seek to data, set up

	lseek(fd, RES_OFFSET_DESC2REAL(prd->offset), SEEK_SET);
	p = buffer;
	size = prd->size;

	//	If compound, read in ref table

	if (prd->flags & RDF_COMPOUND)
		{
		read(fd, p, sizeof(short));
		numRefs = *(short *)p;
		p += sizeof(short);
		read(fd, p, sizeof(long) * (numRefs + 1));
		p += sizeof(long) * (numRefs + 1);
      size -= REFTABLESIZE(numRefs);
		}

	//	Read in data

	if (prd->flags & RDF_LZW)
		LzwExpandFd2Buff(fd, p, 0, 0);
	else
		read(fd, p, size);

	return TRUE;
}
*/
