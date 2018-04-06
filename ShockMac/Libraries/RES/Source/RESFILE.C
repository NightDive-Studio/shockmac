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
//		ResFile.C		Resource Manager file access
//		Rex E. Bradford (REX)
/*
* $Header: r:/prj/lib/src/res/rcs/resfile.c 1.3 1994/08/07 20:17:31 xemu Exp $
* $Log: resfile.c $
 * Revision 1.3  1994/08/07  20:17:31  xemu
 * generate a warning on resource collision
 * 
 * Revision 1.2  1994/06/16  11:07:04  rex
 * Added item to tail of LRU when loadonopen
 * 
 * Revision 1.1  1994/02/17  11:23:23  rex
 * Initial revision
 * 
*/

//#include <fcntl.h>
//#include <sys\stat.h>
//#include <io.h>
#include <string.h>

#include "res.h"
#include "res_.h"

//#include <_res.h>
/*
//	Resource files start with this signature

char resFileSignature[16] = {
	'L','G',' ','R','e','s',' ','F','i','l','e',' ','v','2',13,10};

//	The active resource file info table

ResFile resFile[MAX_RESFILENUM+1];

//	Global datapath for res files, other data modules may piggyback

Datapath gDatapath;
*/
//	Internal prototypes

void AddResDesc(Handle resHdl, short resID, ResType macResType, short filenum, char cFlag);
/*
int ResFindFreeFilenum();
void ResReadDirEntries(int filenum, ResDirHeader *pDirHead);
void ResProcDirEntry(ResDirEntry *pDirEntry, int filenum, long dataOffset);
void ResReadEditInfo(ResFile *prf);
void ResReadDir(ResFile *prf, int filenum);
void ResCreateEditInfo(ResFile *prf, int filenum);
void ResCreateDir(ResFile *prf);
void ResWriteDir(int filenum);
void ResWriteHeader(int filenum);
*/

//	---------------------------------------------------------
//
//	ResAddPath() adds a path to the resource manager's list.
//
//		path = name of directory to add
/*
void ResAddPath(char *path)
{
	DatapathAdd(&gDatapath, path);

	Spew(DSRC_RES_General, ("ResAddPath: added %s\n", path));
}
*/

//	---------------------------------------------------------
//
//	ResOpenResFile() opens for read/edit/create.
//
//		fname   = ptr to filename
//		mode    = ROM_XXX (see res.h)
//		auxinfo = if TRUE, allocate aux info, including directory
//						(applies to mode 0, other modes automatically get it)
//
//	Returns:
//
//		-1 = couldn't find free filenum
//		-2 = couldn't open, edit, or create file
//		-3 = invalid resource file
//		-4 = memory allocation failure
//	---------------------------------------------------------
//  For Mac version:  Use the ResourceMgr routines to open and create Res files.
//  Skip all the EditInfo and Dir stuff.

short ResOpenResFile(FSSpec *specPtr, ResOpenMode mode, bool /*auxinfo*/)
{
	short  		filenum, fd;
	FInfo			fi;
	short			perm;
	short			numTypes, numRes;
	short			ti, ri;
	ResType		aResType;
	Handle		resHdl;
	short			resID;
	Str255		resName;

	//	If any mode but create, open along datapath.  If can't open,
	//	return error except if mode 2 (edit/create), in which case
	//	drop thru to create case by faking mode 3.

	if (mode != ROM_CREATE)
	{
		fd = FSpGetFInfo(specPtr, &fi);					// See if file exists
		if (fd != noErr)											// If not,
		{
			if (mode == ROM_EDITCREATE)				// If this mode, drop through
				mode = ROM_CREATE;							// to create the file.
			else
			{
				DebugStr("\pResOpenResFile: can't open file.\n");
				return(-2);
			}
		}
	}
		
	//	If create mode, or edit/create failed, try to create the file.

	if (mode == ROM_CREATE)
	{
		// If the file already exists, delete it.
		fd = FSpGetFInfo(specPtr, &fi);
		if (fd == noErr)
			FSpDelete(specPtr);
		
		// Create the file.
		FSpCreateResFile(specPtr, 'Shok', 'Sgam', nil);
		fd = ResError();
		if (fd != 0)
		{
			DebugStr("\pResOpenResFile: Can't create file.\n");
			return(-2);
		}
	}

	// Finally, open the thang.
	
	if (mode == ROM_READ)
		perm = fsRdPerm;
	else
		perm = fsRdWrPerm;
	filenum = FSpOpenResFile(specPtr, perm);
	if (filenum == -1)
	{
		DebugStr("\pResOpenResFile: Can't open file.\n");
		return(-2);
	}
	
	// Read in resource map info into the array of resource descriptions

	numTypes =  Count1Types();
	for (ti = 1; ti <= numTypes; ti++)
	{
		Get1IndType(&aResType, ti);
		
		numRes = Count1Resources(aResType);
		SetResLoad(false);
		for (ri = 1; ri <= numRes; ri++)
		{
			resHdl = Get1IndResource(aResType, ri);
			GetResInfo(resHdl, &resID, &aResType, resName);
			
			AddResDesc(resHdl, resID, aResType, filenum, resName[1]);
		}
		SetResLoad(true);
	}
	
	return(filenum);
}

//	---------------------------------------------------------
//  Add a resource description
//	---------------------------------------------------------
void AddResDesc(Handle resHdl, short resID, ResType macResType, short filenum, char cFlag)
{
	Id				id = resID;
	ResDesc 	*prd;
	uchar		ind;
	uchar		flags = 0;

	ResExtendDesc(id);								// Grow table if need to

//	If already a resource at this id, warning
//
//	Spew(DSRC_RES_Read, ("ResProcDirEntry: reading entry for id $%x\n",
//		pDirEntry->id));

	prd = RESDESC(id);
	if (prd->hdl)
	{
		DebugStr("\pAddResDesc: RESOURCE ID COLLISION AT ID ?!!\n");
//		CUMSTATS(pDirEntry->id,numOverwrites);
		ResDelete(id);
	}

	// Set the flags based on the cFlag character.
	
	switch (cFlag)
	{
		case 'c':
			flags = RDF_COMPOUND;
			break;
		case 'z':
			flags = RDF_LZW;
			break;
		case 'x':
			flags = RDF_COMPOUND | RDF_LZW;
			break;
	}	

	for (ind = 0; ind < NUM_RESTYPENAMES; ind++)	// Find associated Shock type.
		if (resMacTypes[ind] == macResType)
			break;
	
	prd->hdl = resHdl;								// Fill in resource descriptor
	prd->filenum = filenum;
	prd->lock = 0;
	prd->flags = flags;
	prd->type = ind;

//	If loadonopen flag set, load resource
/*
	if (pDirEntry->flags & RDF_LOADONOPEN)
		{
		currOffset = tell(resFile[filenum].fd);
		ResLoadResource(pDirEntry->id);
		ResAddToTail(prd);
		lseek(resFile[filenum].fd, currOffset, SEEK_SET);
		}
*/
}

//	---------------------------------------------------------
//
//	ResCloseFile() closes an open resource file.
//
//		filenum = file number used when opening file
//	---------------------------------------------------------
//  For Mac version:  Real simple.  See notes in code.

void ResCloseFile(short filenum)
{
	Id 			id;
	ResDesc	*prd;

	CloseResFile(filenum);
	
	// Since CloseResFile free all the resource handles associated with that
	// file, all we have to do is zero the refs in our ResDesc array for the
	// associated file.
	
	for (id = ID_MIN; id <= resDescMax; id++)
	{
		prd = RESDESC(id);
		if (prd->filenum == filenum)
			LG_memset(prd, 0, sizeof(ResDesc));
	}
/*
//	Make sure file is open

	if (resFile[filenum].fd < 0)
		{
		Warning(("ResCloseFile: filenum %d not in use\n"));
		return;
		}

//	If file being created, flush it

	Spew(DSRC_RES_General, ("ResCloseFile: closing %d\n", filenum));

	if (resFile[filenum].pedit)
		{
		ResWriteDir(filenum);
		ResWriteHeader(filenum);
		}

//	Scan object list, delete any blocks associated with this file

	for (id = ID_MIN; id <= resDescMax; id++)
		{
		if (ResInUse(id) && (ResFilenum(id) == filenum))
			ResDelete(id);
		}

//	Free up memory

	if (resFile[filenum].pedit)
		{
		if (resFile[filenum].pedit->pdir)
			Free(resFile[filenum].pedit->pdir);
		Free(resFile[filenum].pedit);
		}

//	Close file

	close(resFile[filenum].fd);
	resFile[filenum].fd = -1;
*/
}

/*
//	---------------------------------------------------------
//  For Mac version, this function builds a resource table for the file and writes
//  it out as a 'hTbl' resource.
//	---------------------------------------------------------
void ResWriteDir(short filenum)
{
	Id 					id;
	short				numRes;
	ResDesc			*prd;
	Handle			tableHdl;
	ResDirEntry	*rdePtr;

	// Find out how many resources to place in the table.
	for (numRes = 0, id = ID_MIN; id <= resDescMax; id++)
	{
		prd = RESDESC(id);
		if (prd->filenum == filenum)
			numRes++;
	}

	// Fill in the table and write it out.	
	tableHdl = NewHandle(numRes * sizeof(ResDirEntry));
	HLock(tableHdl);
	rdePtr = (ResDirEntry *)*tableHdl;
	for (id = ID_MIN; id <= resDescMax; id++)
	{
		prd = RESDESC(id);
		if (prd->filenum == filenum)
		{
			rdePtr->id = id;
			rdePtr->flags = prd->flags;
			rdePtr->type = prd->type;
			
			rdePtr++;
		}
	}
	HUnlock(tableHdl);
	AddResource(tableHdl, 'hTbl', 128, "\pRes Table");
	WriteResource(tableHdl);
	ReleaseResource(tableHdl);
}

//	--------------------------------------------------------------
//		INTERNAL ROUTINES
//	---------------------------------------------------------
//
//	ResFindFreeFilenum() finds free file number

int ResFindFreeFilenum()
{
	int filenum;

	for (filenum = 0; filenum <= MAX_RESFILENUM; filenum++)
		{
		if (resFile[filenum].fd < 0)
			return(filenum);
		}
	return(-1);
}

//	----------------------------------------------------------
//
//	ResReadDirEntries() reads in entries in a directory.
//		(file seek should be set to 1st directory entry)
//
//		filenum  = file number
//		pDirHead = ptr to directory header

void ResReadDirEntries(int filenum, ResDirHeader *pDirHead)
{
#define NUM_DIRENTRY_BLOCK 64		// (12 bytes each)
	int entry,fd;
	long dataOffset;
	ResDirEntry *pDirEntry;
	ResDirEntry dirEntries[NUM_DIRENTRY_BLOCK];

//	Set up

	Spew(DSRC_RES_Read,
		("ResReadDirEntries: scanning directory, filenum %d\n", filenum));

	pDirEntry = &dirEntries[NUM_DIRENTRY_BLOCK];		// no dir entries read
	dataOffset = pDirHead->dataOffset;					// mark starting offset
	fd = resFile[filenum].fd;

//	Scan directory:

	for (entry = 0; entry < pDirHead->numEntries; entry++)
		{

//	If reached end of local directory buffer, refill it

		if (pDirEntry >= &dirEntries[NUM_DIRENTRY_BLOCK])
			{
			read(fd, dirEntries, sizeof(ResDirEntry) * NUM_DIRENTRY_BLOCK);
			pDirEntry = &dirEntries[0];
			}

//	Process entry

		ResProcDirEntry(pDirEntry, filenum, dataOffset);

//	Advance file offset and get next

		dataOffset = RES_OFFSET_ALIGN(dataOffset + pDirEntry->csize);
		pDirEntry++;
		}
}

//	-----------------------------------------------------------
//
//	ResProcDirEntry() processes directory entry, sets res desc.
//
//		pDirEntry  = ptr to directory entry
//		filenum    = file number
//		dataOffset = offset in file where data lives

void ResProcDirEntry(ResDirEntry *pDirEntry, int filenum, long dataOffset)
{
	ResDesc *prd;
	long currOffset;

//	Grow table if need to

	ResExtendDesc(pDirEntry->id);

//	If already a resource at this id, warning

	Spew(DSRC_RES_Read, ("ResProcDirEntry: reading entry for id $%x\n",
		pDirEntry->id));

	prd = RESDESC(pDirEntry->id);
	if (prd->ptr)
		{
      Warning(("RESOURCE ID COLLISION AT ID %x!!\n",pDirEntry->id));
		CUMSTATS(pDirEntry->id,numOverwrites);
		ResDelete(pDirEntry->id);
		}

//	Fill in resource descriptor

	prd->ptr = NULL;
	prd->size = pDirEntry->size;
	prd->filenum = filenum;
	prd->lock = 0;
	prd->offset = RES_OFFSET_REAL2DESC(dataOffset);
	prd->flags = pDirEntry->flags;
	prd->type = pDirEntry->type;
	prd->next = 0;
	prd->prev = 0;

//	If loadonopen flag set, load resource

	if (pDirEntry->flags & RDF_LOADONOPEN)
		{
		currOffset = tell(resFile[filenum].fd);
		ResLoadResource(pDirEntry->id);
		ResAddToTail(prd);
		lseek(resFile[filenum].fd, currOffset, SEEK_SET);
		}
}

//	--------------------------------------------------------------
//
//	ResReadEditInfo() reads edit info from file.

void ResReadEditInfo(ResFile *prf)
{
	ResEditInfo *pedit = prf->pedit;

//	Init flags to no autopack or anything else

	pedit->flags = 0;

//	Seek to start of file, read in header

	lseek(prf->fd, 0L, SEEK_SET);
	read(prf->fd, &pedit->hdr, sizeof(pedit->hdr));

//	Set no directory (yet, anyway)

	pedit->pdir = NULL;
	pedit->numAllocDir = 0;
	pedit->currDataOffset = 0L;
}

//	---------------------------------------------------------------
//
//	ResReadDir() reads directory for a file.

void ResReadDir(ResFile *prf, int filenum)
{
	ResEditInfo *pedit;
	ResFileHeader *phead;
	ResDirHeader *pdir;
	ResDirEntry *pDirEntry;
	ResDirHeader dirHead;

//	Read directory header

	pedit = prf->pedit;
	phead = &pedit->hdr;
	lseek(prf->fd, phead->dirOffset, SEEK_SET);
	read(prf->fd, &dirHead, sizeof(ResDirHeader));

//	Allocate space for directory, copy directory header into it

	pedit->numAllocDir =
		(dirHead.numEntries + DEFAULT_RES_GROWDIRENTRIES) &
		~(DEFAULT_RES_GROWDIRENTRIES - 1);
	pdir = pedit->pdir = Malloc(sizeof(ResDirHeader) +
		(sizeof(ResDirEntry) * pedit->numAllocDir));
	*pdir = dirHead;

//	Read in directory into allocated space (past header)

	read(prf->fd, RESFILE_DIRENTRY(pdir,0),
		dirHead.numEntries * sizeof(ResDirEntry));

//	Scan directory, setting resource descriptors & counting data bytes

	pedit->currDataOffset = pdir->dataOffset;

	RESFILE_FORALLINDIR(pdir, pDirEntry)
		{
		if (pDirEntry->id == 0)
			pedit->flags |= RFF_NEEDSPACK;
		else
			ResProcDirEntry(pDirEntry, filenum, pedit->currDataOffset);
		pedit->currDataOffset =
			RES_OFFSET_ALIGN(pedit->currDataOffset + pDirEntry->csize);
		}

//	Seek to current data location

	lseek(prf->fd, pedit->currDataOffset, SEEK_SET);
}

//	--------------------------------------------------------------
//
//	ResCreateEditInfo() creates new empty edit info.

void ResCreateEditInfo(ResFile *prf, int filenum)
{
	ResEditInfo *pedit = prf->pedit;

	pedit->flags = RFF_AUTOPACK;
	memcpy(pedit->hdr.signature, resFileSignature, sizeof(resFileSignature));
	ResSetComment(filenum, "");
	memset(pedit->hdr.reserved, 0, sizeof(pedit->hdr.reserved));
}

//	--------------------------------------------------------------
//
//	ResCreateDir() creates empty dir.

void ResCreateDir(ResFile *prf)
{
	ResEditInfo *pedit = prf->pedit;

	pedit->hdr.dirOffset = 0;
	pedit->numAllocDir = DEFAULT_RES_GROWDIRENTRIES;
	pedit->pdir = Malloc(sizeof(ResDirHeader) +
		(sizeof(ResDirEntry) * pedit->numAllocDir));
	pedit->pdir->numEntries = 0;
	pedit->currDataOffset = pedit->pdir->dataOffset = sizeof(ResFileHeader);
	lseek(prf->fd, pedit->currDataOffset, SEEK_SET);
}

//	-------------------------------------------------------------
//
//	ResWriteDir() writes directory to resource file.

void ResWriteDir(int filenum)
{
	ResFile *prf;

	DBG(DSRC_RES_ChkIdRef, {if (resFile[filenum].pedit == NULL) { \
		Warning(("ResWriteDir: file %d not open for writing\n", filenum)); \
		return;}});

	Spew(DSRC_RES_Write, ("ResWriteDir: writing directory for filenum %d\n",
		filenum));

	prf = &resFile[filenum];
	lseek(prf->fd, prf->pedit->currDataOffset, SEEK_SET);
	write(prf->fd, prf->pedit->pdir, sizeof(ResDirHeader) +
		(prf->pedit->pdir->numEntries * sizeof(ResDirEntry)));
}

//	--------------------------------------------------------
//
//	ResWriteHeader() writes header to resource file.

void ResWriteHeader(int filenum)
{
	ResFile *prf;

	DBG(DSRC_RES_ChkIdRef, {if (resFile[filenum].pedit == NULL) { \
		Warning(("ResWriteHeader: file %d not open for writing\n", filenum)); \
		return;}});

	Spew(DSRC_RES_Write, ("ResWriteHeader: writing header for filenum %d\n",
		filenum));

	prf = &resFile[filenum];
	prf->pedit->hdr.dirOffset = prf->pedit->currDataOffset;

	lseek(prf->fd, 0L, SEEK_SET);
	write(prf->fd, &prf->pedit->hdr, sizeof(ResFileHeader));
}
*/
