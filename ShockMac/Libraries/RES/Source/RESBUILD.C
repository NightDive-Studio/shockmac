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
//		RESBUILD.C		Resource-file building routines
//		Rex E. Bradford (REX)
/*
* $Header: n:/project/lib/src/res/rcs/resbuild.c 1.10 1994/06/16 11:06:30 rex Exp $
* $Log: resbuild.c $
 * Revision 1.10  1994/06/16  11:06:30  rex
 * Got rid of RDF_NODROP flag
 * 
 * Revision 1.9  1994/02/17  11:25:32  rex
 * Moved some stuff out to resmake.c and resfile.c
 * 
*/

//#include <io.h>
//#include <stdlib.h>
#include <string.h>

#include "res.h"
#include "res_.h"
#include "lzw.h"
//#include <_res.h>

/*
#define CTRL_Z 26		// make sure comment ends with one, so can type a file

//	Internal prototypes

bool ResEraseIfInFile(Id id);				// erase item from file
*/

//	-------------------------------------------------------
//
//	ResSetComment() sets comment in res header.
//	-------------------------------------------------------
//	For Mac version:  Does nothing.  May go back later and add comment via the
// desktop database, maybe.

void ResSetComment(int /*filenum*/, char* /*comment*/)
{
/*
	ResFileHeader *phead;

	DBG(DSRC_RES_ChkIdRef, {if (resFile[filenum].pedit == NULL) { \
		Warning(("ResSetComment: file %d not open for writing\n", filenum)); \
		return;}});

	Spew(DSRC_RES_General,
		("ResSetComment: setting comment for filenum %d to:\n%s\n",
		filenum, comment));

	phead = &resFile[filenum].pedit->hdr;
	memset(phead->comment, 0, sizeof(phead->comment));
	strncpy(phead->comment, comment, sizeof(phead->comment) - 2);
	phead->comment[strlen(phead->comment)] = CTRL_Z;
*/
}

//	-------------------------------------------------------
//
//	ResWrite() writes a resource to an open resource file.
//	This routine assumes that the file position is already set to
//	the current data position.
// Returns the total number of bytes written out, or -1 if there
// was a writing error.
//
//		id = id to write
//	-------------------------------------------------------
//	For Mac version:  This is why we're using the Mac Resource Mgr. 
//	Simply get the resource handle and write it out.  If resource is compressed,
// do that first before writing.
#define EXTRA 250

int ResWrite(Id id)
{
	ResDesc 	*prd;
	long 			compsize = -1;
	long 			size = 0;
	long 			sizeTable = 0;
	Handle		compHdl = NULL;
	Ptr			srcPtr, compPtr;
	
	prd = RESDESC(id);
	if (prd->hdl)
	{
		if (prd->flags & RDF_LZW)								// If this handle needs to be compressed, then...
		{
			size = GetHandleSize(prd->hdl);					// Get the size of the resource to compress.
			compHdl = NewHandle(size + EXTRA);			// Make a new handle to compress to.
			if (compHdl == NULL)
			{
				DebugStr("\pResWrite: Can't allocate LZW buffer for resource.\n");
			}
			else
			{
				HLock(prd->hdl);									// Lock handles and get pointers.
				HLock(compHdl);
				srcPtr = *prd->hdl;
				compPtr = *compHdl;

				if (prd->flags & RDF_COMPOUND)			// If it's a compound handle, copy the refTable
				{															// over uncompressed.
					sizeTable = REFTABLESIZE(((RefTable *)srcPtr)->numRefs);
					BlockMove(srcPtr, compPtr, sizeTable);
					size -= sizeTable;
					srcPtr += sizeTable;
					compPtr += sizeTable;
				}
				
				if (size > 0)											// Compress it!
					compsize = LzwCompressBuff2Buff(srcPtr, size, compPtr, size);

				HUnlock(compHdl);
				HUnlock(prd->hdl);
	
				if (compsize > 0)												// If compressed okay, then
				{
					SetHandleSize(prd->hdl, compsize+sizeTable);	// set resource handle to the compressed size,
					BlockMoveData(*compHdl, *prd->hdl, compsize+sizeTable);	// copy compressed data into it
					DisposeHandle(compHdl);								// and dispose of the compressed data.
				}
				else
				{
					DebugStr("\pResWrite: LZW compression did not work.\n");
				}
			}
		}
				
		// Now write out the changed resource.
		ChangedResource(prd->hdl);
		WriteResource(prd->hdl);

		return (GetHandleSize(prd->hdl));
	}
	else
		return (-1);
/*
static uchar pad[] = {0,0,0,0,0,0,0,0};
	ResDesc *prd;
	ResFile *prf;
	ResDirEntry *pDirEntry;
	uchar *p;
	long size,sizeTable;
	void *pcompbuff;
	long compsize;
	int padBytes;

//	Check for errors

	DBG(DSRC_RES_ChkIdRef, {if (!ResCheckId(id)) return;});

	prd = RESDESC(id);
	prf = &resFile[prd->filenum];

	DBG(DSRC_RES_Write, {if (prf->pedit == NULL) { \
		Warning(("ResWrite: file %d not open for writing\n", prd->filenum)); \
		return;}});

//	Check if item already in directory, if so erase it

	ResEraseIfInFile(id);

//	If directory full, grow it

	if (prf->pedit->pdir->numEntries == prf->pedit->numAllocDir)
		{
		Spew(DSRC_RES_Write, ("ResWrite: growing directory of filenum %d\n",
			prd->filenum));

		prf->pedit->numAllocDir += DEFAULT_RES_GROWDIRENTRIES;
		prf->pedit->pdir = Realloc(prf->pedit->pdir,
			sizeof(ResDirHeader) + (sizeof(ResDirEntry) * prf->pedit->numAllocDir));
		}

//	Set resource's file offset

	prd->offset = RES_OFFSET_REAL2DESC(prf->pedit->currDataOffset);

//	Fill in directory entry

	pDirEntry = ((ResDirEntry *) (prf->pedit->pdir + 1)) +
		prf->pedit->pdir->numEntries;

	pDirEntry->id = id;
	pDirEntry->flags = prd->flags;
	pDirEntry->type = prd->type;
	pDirEntry->size = prd->size;

	Spew(DSRC_RES_Write, ("ResWrite: writing $%x\n", id));

//	If compound, write out reftable without compression

	lseek(prf->fd, prf->pedit->currDataOffset, SEEK_SET);
	p = prd->ptr;
	sizeTable = 0;
	size = prd->size;
	if (prd->flags & RDF_COMPOUND)
		{
		sizeTable = REFTABLESIZE(((RefTable *) p)->numRefs);
		write(prf->fd, p, sizeTable);
		p += sizeTable;
		size -= sizeTable;
		}

//	If compression, try it (may not work out)

	if (pDirEntry->flags & RDF_LZW)
		{
		pcompbuff = Malloc(size + EXTRA);
		compsize = LzwCompressBuff2Buff(p, size, pcompbuff, size);
		if (compsize < 0)
			{
			pDirEntry->flags &= ~RDF_LZW;
			}
		else
			{
			pDirEntry->csize = sizeTable + compsize;
			write(prf->fd, pcompbuff, compsize);
			}
		Free(pcompbuff);
		}

//	If no compress (or failed to compress well), just write out

	if (!(pDirEntry->flags & RDF_LZW))
		{
		pDirEntry->csize = prd->size;
		write(prf->fd, p, size);
		}

//	Pad to align on data boundary

	padBytes = RES_OFFSET_PADBYTES(pDirEntry->csize);
	if (padBytes)
		write(prf->fd, pad, padBytes);

if (tell(prf->fd) & 3)
	Warning(("ResWrite: misaligned writing!\n"));

//	Advance dir num entries, current data offset

	prf->pedit->pdir->numEntries++;
	prf->pedit->currDataOffset =
		RES_OFFSET_ALIGN(prf->pedit->currDataOffset + pDirEntry->csize);
*/
}

//	-------------------------------------------------------------
//
//	ResKill() not only deletes a resource from memory, it removes it
//	from the file too.
//	-------------------------------------------------------------
//  For Mac version:  Use Resource Manager to remove resource from file.  Have to do
//  our own thing (instead of calling ResDelete()) because RmveResource turns the
//  resource handle into a normal handle.

void ResKill(Id id)
{
	Handle	resHdl;
	ResDesc *prd = RESDESC(id);

	if (prd->hdl)
	{
		resHdl = prd->hdl;
			
		RmveResource(resHdl);							// RmveResource turns it into a normal handle.
		DisposeHandle(resHdl);							// that we can dispose of.

//		if (prd->lock == 0)
//			ResRemoveFromLRU(prd);
	}
	LG_memset(prd, 0, sizeof(ResDesc));

/*
	//	Check for valid id

	DBG(DSRC_RES_ChkIdRef, {if (!ResCheckId(id)) return;});
	Spew(DSRC_RES_Write, ("ResKill: killing $%x\n", id));

	//	Delete it

	ResDelete(id);

	//	Make sure file is writeable

	prd = RESDESC(id);
	DBG(DSRC_RES_Write, {if (resFile[prd->filenum].pedit == NULL) { \
		Warning(("ResKill: file %d not open for writing\n", prd->filenum)); \
		return;}});

	//	If so, erase it

	ResEraseIfInFile(id);
*/
}

/*
//	-------------------------------------------------------------
//
//	ResPack() removes holes from a resource file.
//
//		filenum = resource filenum (must already be open for create/edit)
//
//	Returns: # bytes reclaimed

long ResPack(int filenum)
{
	ResFile *prf;
	ResDirEntry *pDirEntry;
	long numReclaimed,sizeReclaimed;
	long dataRead,dataWrite;
	int i;
	ResDirEntry *peWrite;

//	Check for errors

	prf = &resFile[filenum];
	if (prf->pedit == NULL)
		{
		Warning(("ResPack: filenum %d not open for editing\n"));
		return(0);
		}

//	Set up

	sizeReclaimed = numReclaimed = 0;
	dataRead = dataWrite = prf->pedit->pdir->dataOffset;

//	Scan thru directory, copying over all empty entries

	pDirEntry = (ResDirEntry *) (prf->pedit->pdir + 1);
	for (i = 0; i < prf->pedit->pdir->numEntries; i++)
		{
		if (pDirEntry->id == 0)
			{
			numReclaimed++;
			sizeReclaimed += pDirEntry->csize;
			}
		else
			{
			if (gResDesc[pDirEntry->id].offset > RES_OFFSET_PENDING)
				gResDesc[pDirEntry->id].offset = RES_OFFSET_REAL2DESC(dataWrite);
			if (dataRead != dataWrite)
				ResCopyBytes(prf->fd, dataWrite, dataRead, pDirEntry->csize);
			dataWrite = RES_OFFSET_ALIGN(dataWrite + pDirEntry->csize);
			}
		dataRead = RES_OFFSET_ALIGN(dataRead + pDirEntry->csize);
		pDirEntry++;
		}

//	Now pack directory itself

	pDirEntry = (ResDirEntry *) (prf->pedit->pdir + 1);
	peWrite = pDirEntry;
	for (i = 0; i < prf->pedit->pdir->numEntries; i++)
		{
		if (pDirEntry->id)
			{
			if (pDirEntry != peWrite)
				*peWrite = *pDirEntry;
			peWrite++;
			}
		pDirEntry++;
		}
	prf->pedit->pdir->numEntries -= numReclaimed;

//	Set new current data offset

	prf->pedit->currDataOffset = dataWrite;
	lseek(prf->fd, dataWrite, SEEK_SET);
	prf->pedit->flags &= ~RFF_NEEDSPACK;

//	Truncate file to just header & data (will be extended later when
//	write directory on closing)

	chsize(prf->fd, dataWrite);

//	Return # bytes reclaimed

	Spew(DSRC_RES_Write, ("ResPack: reclaimed %d bytes\n", sizeReclaimed));

	return(sizeReclaimed);
}

#define SIZE_RESCOPY 32768

static void ResCopyBytes(int fd, long writePos, long readPos, long size)
{
	long sizeCopy;
	uchar *buff;

	buff = Malloc(SIZE_RESCOPY);

	while (size > 0)
		{
		sizeCopy = min(SIZE_RESCOPY, size);
		lseek(fd, readPos, SEEK_SET);
		read(fd, buff, sizeCopy);
		lseek(fd, writePos, SEEK_SET);
		write(fd, buff, sizeCopy);
		readPos += sizeCopy;
		writePos += sizeCopy;
		size -= sizeCopy;
		}

	Free(buff);
}

//	--------------------------------------------------------
//		INTERNAL ROUTINES
//	--------------------------------------------------------
//
//	ResEraseIfInFile() erases a resource if it's in a file's directory.
//
//		id = id of item
//
//	Returns: TRUE if found & erased, FALSE otherwise

bool ResEraseIfInFile(Id id)
{
	ResDesc *prd;
	ResFile *prf;
	ResDirEntry *pDirEntry;
	int i;

	prd = RESDESC(id);
	prf = &resFile[prd->filenum];
	pDirEntry = (ResDirEntry *) (prf->pedit->pdir + 1);

	for (i = 0; i < prf->pedit->pdir->numEntries; i++)
		{
		if (id == pDirEntry->id)
			{
			Spew(DSRC_RES_Write, ("ResEraseIfInFile: $%x being erased\n", id));
			pDirEntry->id = 0;
			prf->pedit->flags |= RFF_NEEDSPACK;
			if (prf->pedit->flags & RFF_AUTOPACK)
				ResPack(prd->filenum);
			return TRUE;
			}
		pDirEntry++;
		}

	return FALSE;
}
*/
