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
* $Header: r:/prj/lib/src/res/rcs/resfile.c 1.5 1994/11/30 20:40:43 xemu Exp $
* $Log: resfile.c $
 * Revision 1.5  1994/11/30  20:40:43  xemu
 * cd spoofing support
 * 
 * Revision 1.4  1994/09/22  10:48:32  rex
 * Modified access to resdesc flags and type, which have moved
 * 
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

//	Resource files start with this signature

char resFileSignature[16] = {
	'L','G',' ','R','e','s',' ','F','i','l','e',' ','v','2',13,10};

//	The active resource file info table

ResFile resFile[MAX_RESFILENUM+1];

//	Global datapath for res files, other data modules may piggyback

//Datapath gDatapath;

// CD spoofing data
ResSpoofDrive gSpoofDrive;

//	Internal prototypes

int ResFindFreeFilenum();
void ResReadDirEntries(int filenum, ResDirHeader *pDirHead, uint add_flags);
void ResProcDirEntry(ResDirEntry *pDirEntry, int filenum, long dataOffset, uint add_flags);
void ResReadEditInfo(ResFile *prf);
void ResReadDir(ResFile *prf, int filenum);
void ResCreateEditInfo(ResFile *prf, int filenum);
void ResCreateDir(ResFile *prf);
void ResWriteDir(int filenum);
void ResWriteHeader(int filenum);

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

int ResOpenResFile(char *fname, ResOpenMode mode, bool auxinfo)
{
//static int openMode[] = {
//	O_RDONLY | O_BINARY,
//	O_RDWR | O_BINARY,
//	O_RDWR | O_BINARY};

	int filenum;
	FILE*	fd;
	ResFile *prf;
	ResFileHeader fileHead;
	ResDirHeader dirHead;
   bool cd_spoof = FALSE;

//	Find free file number, else return -1

	filenum = ResFindFreeFilenum();
	if (filenum < 0)
		{
//		Warning(("ResOpenResFile: no free filenum for: %s\n", fname));
		return(-1);
		}

//	If any mode but create, open along datapath.  If can't open,
//	return error except if mode 2 (edit/create), in which case
//	drop thru to create case by faking mode 3.

	if (mode != ROM_CREATE)
		{
//		fd = DatapathFDOpen(&gDatapath, fname, openMode[mode]);
		fd = fopen(fname, "rb");
		if (fd != NULL)
			{
//			read(fd, &fileHead, sizeof(ResFileHeader));
			fread(&fileHead, sizeof(ResFileHeader), 1, fd);
			fileHead.dirOffset = SwapLongBytes(fileHead.dirOffset);		//еее
			if (strncmp(fileHead.signature, resFileSignature,
				sizeof(resFileSignature)) != 0)
				{
//				close(fd);
				fclose(fd);
//				Warning(("ResOpenResFile: %s is not valid resource file\n", fname));
				return(-3);
				}
			}
		else
			{
			if (mode == ROM_EDITCREATE)
				mode = ROM_CREATE;
			else
				{
//				Warning(("ResOpenResFile: can't open file: %s\n", fname));
				return(-2);
				}
			}
		}

//	If create mode, or edit/create failed, try to open file for creation.

	if (mode == ROM_CREATE)
		{
//		fd = open(fname, O_CREAT | O_TRUNC | O_RDWR | O_BINARY,
//			S_IREAD | S_IWRITE);
		fd = fopen(fname, "wb");
		if (fd == NULL)
			{
//			Warning(("ResOpenResFile: Can't create file: %s\n", fname));
			return(-2);
			}
		}

//	If aux info, allocate space for it

	prf = &resFile[filenum];
	prf->pedit = NULL;
	if (mode || auxinfo)
		{
		prf->pedit = (ResEditInfo *)malloc(sizeof(ResEditInfo));
		if (prf->pedit == NULL)
			{
//			Warning(("ResOpenResFile: unable to allocate ResEditInfo\n"));
//			close(fd);
			fclose(fd);
			return(-4);
			}
		}

//	Record resFile[] file descriptor

	prf->fd = fd;
//	Spew(DSRC_RES_General, ("ResOpenResFile: opening: %s at filenum %d\n",
//		fname, filenum));

//	Switch based on mode

	switch (mode)
		{

//	If open existing file, read directory into edit info & process, or
//	if no edit info then process piecemeal.

		case ROM_READ:
		case ROM_EDIT:
		case ROM_EDITCREATE:
			if (prf->pedit)
				{
//				ResReadEditInfo(prf);
//				ResReadDir(prf, filenum);
				}
			else
				{
//				lseek(fd, fileHead.dirOffset, SEEK_SET);
				fseek(fd, fileHead.dirOffset, SEEK_SET);
//				read(fd, &dirHead, sizeof(ResDirHeader));
				fread(&dirHead, 1, sizeof(ResDirHeader), fd);
				dirHead.numEntries = SwapShortBytes(dirHead.numEntries); 	//еее
				dirHead.dataOffset = SwapLongBytes(dirHead.dataOffset);		//еее
//				ResReadDirEntries(filenum, &dirHead, (cd_spoof) ? RDF_CDSPOOF : 0);
				}
			break;

//	If open for create, initialize header & dir

//		case ROM_CREATE:
//			ResCreateEditInfo(prf, filenum);
//			ResCreateDir(prf);
//			break;
		}

//	Return filenum

	return(filenum);
}

//	---------------------------------------------------------
//
//	ResCloseFile() closes an open resource file.
//
//		filenum = file number used when opening file

void ResCloseFile(int filenum)
{
	Id id;

//	Make sure file is open

	if (resFile[filenum].fd == NULL)
		{
//		Warning(("ResCloseFile: filenum %d not in use\n"));
		return;
		}

//	If file being created, flush it
/*
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
*/
//	Free up memory

	if (resFile[filenum].pedit)
		{
		if (resFile[filenum].pedit->pdir)
			free(resFile[filenum].pedit->pdir);
		free(resFile[filenum].pedit);
		}

//	Close file

//	close(resFile[filenum].fd);
	fclose(resFile[filenum].fd);
	resFile[filenum].fd = NULL;
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
		if (resFile[filenum].fd == NULL)
			return(filenum);
		}
	return(-1);
}
/*
//	----------------------------------------------------------
//
//	ResReadDirEntries() reads in entries in a directory.
//		(file seek should be set to 1st directory entry)
//
//		filenum  = file number
//		pDirHead = ptr to directory header
//    add_flags = additional flags to OR into RDF flags for all
//                resources in this file.

void ResReadDirEntries(int filenum, ResDirHeader *pDirHead, uint add_flags)
{
#define NUM_DIRENTRY_BLOCK 64		// (12 bytes each)
	int entry;
	FILE *fd;
	long dataOffset;
	ResDirEntry *pDirEntry;
	ResDirEntry dirEntries[NUM_DIRENTRY_BLOCK];

//	Set up

//	Spew(DSRC_RES_Read,
//		("ResReadDirEntries: scanning directory, filenum %d\n", filenum));

	pDirEntry = &dirEntries[NUM_DIRENTRY_BLOCK];		// no dir entries read
	dataOffset = pDirHead->dataOffset;					// mark starting offset
	fd = resFile[filenum].fd;

//	Scan directory:

	for (entry = 0; entry < pDirHead->numEntries; entry++)
		{

//	If reached end of local directory buffer, refill it

		if (pDirEntry >= &dirEntries[NUM_DIRENTRY_BLOCK])
			{
//			read(fd, dirEntries, sizeof(ResDirEntry) * NUM_DIRENTRY_BLOCK);
			fread(dirEntries, sizeof(ResDirEntry) * NUM_DIRENTRY_BLOCK, 1, fd);
			pDirEntry = &dirEntries[0];
			}

//	Process entry

		ResProcDirEntry(pDirEntry, filenum, dataOffset, add_flags);

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
//    add_flags = additional flags to OR into RDF flags for all
//                resources in this file.

void ResProcDirEntry(ResDirEntry *pDirEntry, int filenum, long dataOffset, uint add_flags)
{
	ResDesc *prd;
	ResDesc2 *prd2;
	long currOffset;

//	Grow table if need to

	ResExtendDesc(pDirEntry->id);

//	If already a resource at this id, warning

//	Spew(DSRC_RES_Read, ("ResProcDirEntry: reading entry for id $%x\n",
//		pDirEntry->id));

	prd = RESDESC(pDirEntry->id);
	prd2 = RESDESC2(pDirEntry->id);
	if (prd->ptr)
		{
//      Warning(("RESOURCE ID COLLISION AT ID %x!!\n",pDirEntry->id));
		CUMSTATS(pDirEntry->id,numOverwrites);
//		ResDelete(pDirEntry->id);
		}

//	Fill in resource descriptor

	prd->ptr = NULL;
	prd->size = pDirEntry->size;
	prd->filenum = filenum;
	prd->lock = 0;
	prd->offset = RES_OFFSET_REAL2DESC(dataOffset);
	prd2->flags = pDirEntry->flags | add_flags;
	prd2->type = pDirEntry->type;
	prd->next = 0;
	prd->prev = 0;

//	If loadonopen flag set, load resource

//	if (pDirEntry->flags & RDF_LOADONOPEN)
//		{
//		currOffset = tell(resFile[filenum].fd);
//		ResLoadResource(pDirEntry->id);
//		ResAddToTail(prd);
//		lseek(resFile[filenum].fd, currOffset, SEEK_SET);
//		}

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

//	lseek(prf->fd, 0L, SEEK_SET);
//	read(prf->fd, &pedit->hdr, sizeof(pedit->hdr));
	fseek(prf->fd, 0L, SEEK_SET);
	fread(&pedit->hdr, sizeof(pedit->hdr), 1, prf->fd);

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
//	lseek(prf->fd, phead->dirOffset, SEEK_SET);
//	read(prf->fd, &dirHead, sizeof(ResDirHeader));
	fseek(prf->fd, phead->dirOffset, SEEK_SET);
	fread(&dirHead, sizeof(ResDirHeader), 1, prf->fd);

//	Allocate space for directory, copy directory header into it

	pedit->numAllocDir =
		(dirHead.numEntries + DEFAULT_RES_GROWDIRENTRIES) &
		~(DEFAULT_RES_GROWDIRENTRIES - 1);
	pdir = pedit->pdir = malloc(sizeof(ResDirHeader) +
		(sizeof(ResDirEntry) * pedit->numAllocDir));
	*pdir = dirHead;

//	Read in directory into allocated space (past header)

//	read(prf->fd, RESFILE_DIRENTRY(pdir,0),
//		dirHead.numEntries * sizeof(ResDirEntry));
	fread(RESFILE_DIRENTRY(pdir,0),
		dirHead.numEntries * sizeof(ResDirEntry), 1, prf->fd);

//	Scan directory, setting resource descriptors & counting data bytes

	pedit->currDataOffset = pdir->dataOffset;

	RESFILE_FORALLINDIR(pdir, pDirEntry)
		{
		if (pDirEntry->id == 0)
			pedit->flags |= RFF_NEEDSPACK;
		else
			ResProcDirEntry(pDirEntry, filenum, pedit->currDataOffset,0);
		pedit->currDataOffset =
			RES_OFFSET_ALIGN(pedit->currDataOffset + pDirEntry->csize);
		}

//	Seek to current data location

//	lseek(prf->fd, pedit->currDataOffset, SEEK_SET);
	fseek(prf->fd, pedit->currDataOffset, SEEK_SET);
}

//	--------------------------------------------------------------
//
//	ResCreateEditInfo() creates new empty edit info.

void ResCreateEditInfo(ResFile *prf, int filenum)
{
	ResEditInfo *pedit = prf->pedit;

	pedit->flags = RFF_AUTOPACK;
	memcpy(pedit->hdr.signature, resFileSignature, sizeof(resFileSignature));
//	ResSetComment(filenum, "");
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
	pedit->pdir = malloc(sizeof(ResDirHeader) +
		(sizeof(ResDirEntry) * pedit->numAllocDir));
	pedit->pdir->numEntries = 0;
	pedit->currDataOffset = pedit->pdir->dataOffset = sizeof(ResFileHeader);
//	lseek(prf->fd, pedit->currDataOffset, SEEK_SET);
	fseek(prf->fd, pedit->currDataOffset, SEEK_SET);
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

// Sets the path that the resource system will treat as a virtual CD
//
// path = path to treat as the virtual CD
// fix_delay = a delay, in milliseconds, to add every time the
//       virtual CD is accessed
// size_delay = a delay in milliseconds per KB to add when the
//       virtual CD is accessed, so it is proportional to transfer size

// right now, this only handles one spoof_drive, but obviously
// it could be tweaked to keep around an array of ResSpoofDrives...
void ResSetCDSpoof(char *path, void (*spoof_cb)(int size, Id id))
{
   gSpoofDrive.path = path;
   gSpoofDrive.callback = spoof_cb;
}
*/
