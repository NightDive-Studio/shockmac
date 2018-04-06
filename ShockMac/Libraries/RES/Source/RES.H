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
//		Res.H		Resource Manager header file
//		Rex E. Bradford (REX)
/*
* $Header: n:/project/lib/src/res/rcs/res.h 1.9 1994/06/16 11:56:34 rex Exp $
* $Log: res.h $
 * Revision 1.9  1994/06/16  11:56:34  rex
 * Got rid of RDF_NODROP
 * 
 * Revision 1.8  1994/05/26  13:54:27  rex
 * Added prototype ResInstallPager()
 * 
 * Revision 1.7  1994/03/09  19:31:48  jak
 * Res\RefExtractInBlocks transfers a variable/length
 * block of data in each pass.  The user/defined function
 * returns the amount that should be passed in NEXT time.
 * 
 * Revision 1.6  1994/02/17  11:25:02  rex
 * Massive overhaul, moved some private stuff out to res_.h
 * 
 * Revision 1.5  1993/09/01  16:02:10  rex
 * Added prototype for ResExtractRefTable().
 * 
 * Revision 1.4  1993/05/13  10:38:44  rex
 * Added prototype for ResUnmake()
 * 
 * Revision 1.3  1993/05/13  10:30:56  rex
 * Added Extract routines and macros
 * 
 * Revision 1.2  1993/03/08  10:06:12  rex
 * Changed resource directory entry format (reduced from 12 to 10 bytes)
 * 
 * Revision 1.1  1993/03/04  18:47:58  rex
 * Initial revision
 * 
 * Revision 1.6  1993/03/02  18:42:21  rex
 * Major revision, new system
 * 
*/

#ifndef __RES_H
#define __RES_H

//еее For now
//#define DBG_ON		1

#include <Files.h>
#include "lg.h"

//#ifndef DATAPATH_H
//#include <datapath.h>
//#endif

#ifndef __TYPES_H
#include "lg_types.h"
#endif

#ifndef __RESTYPES_H
#include "restypes.h"
#endif

//	---------------------------------------------------------
//		ID AND REF DEFINITIONS AND MACROS
//	---------------------------------------------------------

//	Id's refer to resources, Ref's refer to items in compound resources

typedef ushort Id;			// ID of a resource
typedef ulong Ref;			// high word is ID, low word is index
typedef ushort RefIndex;	// index part of ref

//	Here's how you get parts of a ref, or make a ref

#define REFID(ref) ((ref)>>16)									// get id from ref
#define REFINDEX(ref) ((ref)&0xFFFF)						// get index from ref
#define MKREF(id,index) ((((long)id)<<16)|(index))	// make ref

#define ID_NULL 0			// null resource id
#define ID_HEAD 1			// holds head ptr for LRU chain
#define ID_TAIL 2			// holds tail ptr for LRU chain
#define ID_MIN 3			// id's from 3 and up are valid

//	---------------------------------------------------------
//		ACCESS TO RESOURCES (ID'S)  (resacc.c)
//	---------------------------------------------------------

void *ResLock(Id id);								// lock resource & get ptr
void *ResLockHi(Id id);							// lock resource hi (Mac only)
void ResUnlock(Id id);								// unlock resource
void *ResGet(Id id);								// get ptr to resource (dangerous!)
void *ResExtract(Id id, void *buffer);		// extract resource into buffer
void ResDrop(Id id);								// drop resource from immediate use
void ResDelete(Id id);								// delete resource forever

//	------------------------------------------------------------
//		ACCESS TO ITEMS IN COMPOUND RESOURCES (REF'S)  (refacc.c)
//	------------------------------------------------------------

//	Each compound resource starts with a Ref Table

typedef struct {
	RefIndex numRefs;									// # items in compound resource
	long offset[1];										// offset to each item (numRefs + 1 of them)
} RefTable;

void *RefLock(Ref ref);								// lock compound res, get ptr to item
#define RefUnlock(ref) ResUnlock(REFID(ref))	// unlock compound res item
void *RefGet(Ref ref);									// get ptr to item in comp. res (dangerous!)

RefTable *ResReadRefTable(Id id);									// alloc & read ref table
#define ResFreeRefTable(prt) (DisposePtr((Ptr)prt))		// free ref table
//int ResExtractRefTable(Id id, RefTable *prt, long size); 	// extract reftable
void *RefExtract(RefTable *prt, Ref ref, void *buff);			// extract ref

#define RefIndexValid(prt,index) ((index) < (prt)->numRefs)
#define RefSize(prt,index) (prt->offset[(index)+1]-prt->offset[index])

int ResNumRefs(Id id); // returns the number of refs in a resource, extracting if necessary.

#define REFTABLESIZE(numrefs) (sizeof(RefIndex) + (((numrefs)+1) * sizeof(long)))
#define REFPTR(prt,index) (((uchar *) prt) + prt->offset[index])

/*
//	-----------------------------------------------------------
//		BLOCK-AT-A-TIME ACCESS TO RESOURCES  (resexblk.c)
//	-----------------------------------------------------------

void ResExtractInBlocks(Id id, void *buff, long blockSize,
	long (*f_ProcBlock)(void *buff, long numBytes, long iblock));
void RefExtractInBlocks(RefTable *prt, Ref ref, void *buff, long blockSize,
	long (*f_ProcBlock)(void *buff, long numBytes, long iblock));

#define REBF_FIRST 0x01		// set for 1st block passed to f_ProcBlock
#define REBF_LAST  0x02		// set for last block (may also be first!)
*/

//	-----------------------------------------------------------
//		IN-MEMORY RESOURCE DESCRIPTORS, AND INFORMATION ROUTINES
//	-----------------------------------------------------------
//  For Mac version, keep a handle (rather than ptr).  Most ResDesc info not needed
//  because the Mac Resource Mgr takes care of it.

//	Each resource id gets one of these resource descriptors

typedef struct 
{
	Handle	hdl;			// Mac resource handle.  NULL if not in memory (on disk)
	short	 	filenum;	// Mac resource file number
	uchar 	lock;			// lock count
	uchar	flags;			// misc flags (RDF_XXX, see below)
	uchar 	type;			// resource type (RTYPE_XXX, see restypes.h)
} ResDesc;

#define RESDESC(id) (&gResDesc[id])					// convert id to resource desc ptr
#define RESDESC_ID(prd) ((prd)-gResDesc)		// convert resdesc ptr to id

#define RDF_LZW				0x01		// if 1, LZW compressed
#define RDF_COMPOUND		0x02		// if 1, compound resource
#define RDF_RESERVED		0x04		// reserved
#define RDF_LOADONOPEN	0x08		// if 1, load block when open file

#define RES_MAXLOCK 255				// max locks on a resource

extern ResDesc *gResDesc;				// ptr to big array of ResDesc's
extern Id resDescMax;						// max id in res desc


//	Information about resources

#define ResInUse(id) (gResDesc[id].hdl)
#define ResPtr(id) (*(gResDesc[id].hdl))
long ResSize(Id id);										// It's a function now, in res.c
//#define ResSize(id) (MaxSizeRsrc(gResDesc[id].hdl))
#define ResLocked(id) (gResDesc[id].lock)
//#define ResType(id) (gResDesc[id].type)
//#define ResFilenum(id) (gResDesc[id].filenum)
#define ResFlags(id) (gResDesc[id].flags)
#define ResCompressed(id) (gResDesc[id].flags & RDF_LZW)
#define ResIsCompound(id) (gResDesc[id].flags & RDF_COMPOUND)

//	------------------------------------------------------------
//		RESOURCE MANAGER GENERAL ROUTINES  (res.c)
//	------------------------------------------------------------

void ResInit();						// init Res, allocate initial ResDesc[]
void ResTerm();						// term Res (done auto via atexit)

//	------------------------------------------------------------
//		RESOURCE FILE ACCESS (resfile.c)
//	------------------------------------------------------------

typedef enum
{
	ROM_READ,			// open for reading only
	ROM_EDIT,			// open for editing (r/w) only
	ROM_EDITCREATE,	// open for editing, create if not found
	ROM_CREATE			// open for creation (deletes existing)
} ResOpenMode;

void ResAddPath(char *path);		// add search path for resfiles
short ResOpenResFile(FSSpec *specPtr, ResOpenMode mode, bool auxinfo);
void ResCloseFile(short filenum);	// close res file

#define ResOpenFile(specPtr) ResOpenResFile(specPtr, ROM_READ, FALSE)
#define ResEditFile(specPtr,creat) ResOpenResFile(specPtr, \
	(creat) ? ROM_EDITCREATE : ROM_EDIT, TRUE)
#define ResCreateFile(specPtr) ResOpenResFile(specPtr, ROM_CREATE, TRUE)

//#define MAX_RESFILENUM 15			// maximum file number

//extern Datapath gDatapath;			// res system's datapath (others may use)
/*
//	---------------------------------------------------------
//		RESOURCE MEMORY MANAGMENT ROUTINES  (resmem.c)
//	---------------------------------------------------------

void *ResMalloc(size_t size);
void *ResRealloc(void *p, size_t newsize);
void ResFree(void *p);
void *ResPage(long size);

void ResInstallPager(void *f(long size));

//	---------------------------------------------------------
//		RESOURCE STATS - ACCESSIBLE AT ANY TIME
//	---------------------------------------------------------

typedef struct {
	ushort numLoaded;					// # resources loaded in ram
	ushort numLocked;					// # resources locked
	long totMemAlloc;					// total memory alloted to resources
} ResStat;

extern ResStat resStat;				// stats computed if proper DBG bit set
*/

//	----------------------------------------------------------
//		PUBLIC INTERFACE FOR CREATORS OF RESOURCES
//	----------------------------------------------------------

//	----------------------------------------------------------
//		RESOURCE MAKING  (resmake.c)
//	----------------------------------------------------------

void ResMake(Id id, void *ptr, long size, uchar type, short filenum,
	uchar flags);										// make resource from data block
void ResMakeCompound(Id id, uchar type, short filenum,
	uchar flags);										// make empty compound resource
void ResAddRef(Ref ref, void *pitem, long itemSize);	// add item to compound
void ResUnmake(Id id);											// unmake a resource

//	----------------------------------------------------------
//		RESOURCE FILE LAYOUT
//	----------------------------------------------------------
/*
typedef struct					// For Mac version, an array of these are saved in the
{									// file's 'hTbl' 128 resource.
	Id 			id;					// resource id
	uchar 	flags;				// resource flags (RDF_XXX)
	uchar	type;				// resource type
} ResDirEntry;

void ResWriteDir(short filenum);	// Mac version: Write out a resource table for the file.
*/
/*
//	Resource-file disk format:  header, data, dir

typedef struct {
	char signature[16];		// "LG ResFile v2.0\n",
	char comment[96];			// user comment, terminated with '\z'
	uchar reserved[12];		// reserved for future use, must be 0
	long dirOffset;			// file offset of directory
} ResFileHeader;				// total 128 bytes (why not?)

typedef struct {
	ushort numEntries;		// # items referred to by directory
	long dataOffset;			// file offset at which data resides
									// directory entries follow immediately
									// (numEntries of them)
} ResDirHeader;

typedef struct {
	Id id;						// resource id (if 0, entry is deleted)
	long size: 24;				// uncompressed size (size in ram)
	long flags: 8;				// resource flags (RDF_XXX)
	long csize: 24;			// compressed size (size on disk)
									// (this size is valid disk size even if not comp.)
	long type: 8;				// resource type
} ResDirEntry;

//	Active resource file table

typedef struct {
	ushort flags;				// RFF_XXX
	ResFileHeader hdr;		// file header
	ResDirHeader *pdir;		// ptr to resource directory
	ushort numAllocDir;		// # dir entries allocated
	long currDataOffset;		// current data offset in file
} ResEditInfo;

typedef struct {
	int fd;						// file descriptor (from open())
	ResEditInfo *pedit;		// editing info, or NULL if read-only file
} ResFile;

#define RFF_NEEDSPACK	0x0001			// resfile has holes, needs packing
#define RFF_AUTOPACK		0x0002			// resfile auto-packs (default TRUE)

extern ResFile resFile[MAX_RESFILENUM+1];

//	Macros to get ptr to resfile's directory, & iterate across entries

#define RESFILE_HASDIR(filenum) (resFile[filenum].pedit)
#define RESFILE_DIRPTR(filenum) (resFile[filenum].pedit->pdir)
#define RESFILE_DIRENTRY(pdir,n) ((ResDirEntry *)((pdir) + 1) + (n))
#define RESFILE_FORALLINDIR(pdir,pde) for (pde = RESFILE_DIRENTRY(pdir,0); \
	pde < RESFILE_DIRENTRY(pdir,pdir->numEntries); pde++)

extern char resFileSignature[16];		// magic header
*/

//	--------------------------------------------------------
//		RESOURCE FILE BUILDING  (resbuild.c)
//	--------------------------------------------------------

void ResSetComment(int filenum, char *comment);	// set comment
int ResWrite(Id id);												// write resource to file
void ResKill(Id id);													// delete resource & remove from file
//long ResPack(int filenum);									// remove empty entries
#define ResPack(filenum) 

//#define ResAutoPackOn(filenum) (resFile[filenum].pedit->flags |= RFF_AUTOPACK)
//#define ResAutoPackOff(filenum) (resFile[filenum].pedit->flags &= ~RFF_AUTOPACK)
//#define ResNeedsPacking(filenum) (resFile[filenum].pedit->flags & RFF_NEEDSPACK)


#endif

