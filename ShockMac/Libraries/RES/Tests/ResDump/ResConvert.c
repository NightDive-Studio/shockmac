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
//==================================================================================
//
// ResConvert - Program to convert System Shock resources from the PC to the Mac.
//              by Ken Cobb.
//
//==================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lg.h"
#include "res.h"
#include "res_.h"
#include "lzw.h"

//---------------------------------------------------------------------------------
void main(void);
short CreateMacFile(char *macfname);
void ConvertData(Ptr resPtr, long size, uchar dtype);

void ConvertIMAGE(Ptr dataPtr, long size);
void ConvertFONT(Ptr dataPtr, long size);


//---------------------------------------------------------------------------------
char *res_files[] = { "vidmail.res", "texture.res", "objart.res", "objart2.res",
					  "objart3.res", "splash.res", "gamepal.res", "gamescr.res",
					  "obj3d.res", "citmat.res", "cutspal.res", "intro.res",
					  "handart.res", "mfdart.res", "mfdfrn.res",
					  "mfdger.res", "cybstrng.res", "frnstrng.res", "gerstrng.res",
					  "death.res", "start1.res", "win1.res" };
short fn;
FSSpec	outSpec;


//---------------------------------------------------------------------------------
// Main function - Convert all the files in the 'res_files' list.
//---------------------------------------------------------------------------------
void main(void)
{
	int i,j,filenum;
	FILE* fd;
	long totsize,totcsize,compPct,numErased,sizeErased;
	ResFileHeader fileHead;
	ResDirHeader dirHead;
	ResDirEntry *pentry;
	char flagLzw,flagComp,flagLoad;
	RefIndex numEntries;
	long *offset;
	ulong dataOffset,sizeRef;
	char fname[128];
	char buff[160];
	char tname[16];
	char blankPad[9] = {"        "};

	uchar	s1, s2;
	char*	tabPtr;
	long	tabSize;
	Handle	resHdl;
	Ptr		resPtr;
	long	sizeToRead;
	char 	macfname[64];
	short	macfilenum;
	Str255	resName;
	short	rerr;
	int		numResWritten = 0;
	
	// Init the resource system.

	ResInit();
	LzwInit();

	// Loop through each file in the res_files list.
	
//for(fn = 0; fn < 22; fn++)
//{
//	strcpy(fname, res_files[fn]);

//	strcpy(fname, res_files[8]);		// Do obj3d.res
	
	printf ("File to convert: ");
	fgets (fname, sizeof(fname), stdin);
	fname[strlen(fname)-1] = 0;
	if (strchr(fname, '.') == NULL)
		strcat(fname, ".RES");

	// Open the input PC file.
	filenum = ResOpenFile(fname);
	if (filenum < 0)
	{
		printf("ResConvert: can't open: %s\n", fname);
		return;
	}

	// Create the output Mac file.
//	strcpy(macfname, "Mac vidmail.res");			// Set output file name to "Mac xxxx.res".
//	strcat(macfname, fname);
	printf ("Output file: ");
	fgets (macfname, sizeof(macfname), stdin);
	macfname[strlen(macfname)-1] = 0;
	macfilenum = CreateMacFile(macfname);
	if (macfilenum < 0)
	{
		ResCloseFile(filenum);
		printf("ResConvert: can't create Mac res file: %s\n", macfname);
		return;
	}	

	printf("\nConverting file '%s' to Mac file '%s'...\n", fname, macfname);
	printf("\n");
	printf("  ID     TYPE     SIZE    CSIZE  PCT  LZW COMP   Converted to\n");
	printf("-----  --------  ------  ------  ---  --- ----   ------------------\n");

	//	Grab header and directory head and directory.  Swap necessary numbers.

	fd = resFile[filenum].fd;
	fseek(fd, 0L, SEEK_SET);
	fread(&fileHead, sizeof(ResFileHeader), 1, fd);
fileHead.dirOffset = SwapLongBytes(fileHead.dirOffset);

	fseek(fd, fileHead.dirOffset, SEEK_SET);
	fread(&dirHead, sizeof(ResDirHeader), 1, fd);
dirHead.numEntries = SwapShortBytes(dirHead.numEntries);
dirHead.dataOffset = SwapLongBytes(dirHead.dataOffset);

	pentry = malloc(dirHead.numEntries * sizeof(ResDirEntry));
	fread(pentry, dirHead.numEntries * sizeof(ResDirEntry), 1, fd);
	dataOffset = dirHead.dataOffset;

	//	Go thru directory, converting each resource.

	totsize = totcsize = 0;
	numErased = sizeErased = 0;

	for (i = 0; i < dirHead.numEntries; i++)
	{
		// Adjust the id.
		pentry->id = SwapShortBytes(pentry->id);

		// Swap bytes in the size and csize parts of pentry.  Because these
		// just use 24 bits of a long, I can't use SwapLongBytes.
		
		s1 = *(((uchar *)pentry)+2);	// size
		s2 = *(((uchar *)pentry)+4);
		*(((uchar*)pentry)+4) = s1;
		*(((uchar*)pentry)+2) = s2;
		
		s1 = *(((uchar *)pentry)+6);	// csize
		s2 = *(((uchar *)pentry)+8);
		*(((uchar*)pentry)+8) = s1;
		*(((uchar*)pentry)+6) = s2;

		// Figure the compression percentage (if any)
		compPct = ((pentry->size - pentry->csize) * 100) / pentry->size;

		// Is it compressed and/or compound?
		flagLzw = (pentry->flags & RDF_LZW) ? '*' : ' ';
		flagComp = (pentry->flags & RDF_COMPOUND) ? '*' : ' ';

		// If there is an id in this resource slot, then get its type name.
		// If not then it's an erased entry (not compacted??).
		if (pentry->id)
		{
			sprintf(tname, "%s%s", resTypeNames[pentry->type], blankPad +
				strlen(resTypeNames[pentry->type]));
		}
		else
		{
			strcpy(tname, "ERASED  ");
			numErased++;
			sizeErased += pentry->csize;
		}

		// If a compound resource, read in the offsets table and convert all
		// the numbers.
		if (pentry->flags & RDF_COMPOUND)
		{
			// First get the number of entries.
			fseek(fd, dataOffset, SEEK_SET);				
			fread(&numEntries, sizeof(RefIndex), 1, fd);
numEntries = SwapShortBytes(numEntries);
			
			// Then allocate space for and read the offsets.
			tabSize = REFTABLESIZE(numEntries);
			tabPtr = malloc(tabSize);
			*(RefIndex *)tabPtr = numEntries;
			tabPtr += 2;
			offset = (long *)tabPtr;
			tabPtr -= 2;
			fread(offset, sizeof(long) * (numEntries + 1), 1, fd);

			// Go through the offsets and swap their bytes.  Note there is one
			// more offset than indicated by numEntries.
			for (j = 0; j <= numEntries; j++)
			{
				offset[j] = SwapLongBytes(offset[j]);
			}
		}
		else
			fseek(fd, dataOffset, SEEK_SET);				

		// Allocate a new handle to place the resource into.
		resHdl = NewHandle(pentry->size);
		if (resHdl == NULL)
		{
			rerr = MemError();
			Debugger();
		}
		HLock(resHdl);
		resPtr = *resHdl;
		sizeToRead = pentry->size;

		// If it's a compound resource, copy over the converted table.		
		if (pentry->flags & RDF_COMPOUND)
		{
			BlockMove(tabPtr, resPtr, tabSize);
			resPtr += tabSize;
			sizeToRead -= tabSize;
			free(tabPtr);
		}

		// If compressed, then uncompress it before writing out.
		if (pentry->flags & RDF_LZW)
		{
			LzwExpandFp2Buff(fd, resPtr, 0, 0);
		}
		
		// Non-compressed resource, so just read in the resource into the
		// new handle.
		else
		{
			fread(resPtr, sizeToRead, 1, fd);
		}

		// Convert the type, and write it out as a Mac resource.  For
		// compound resources, convert each resource within the handle.
		if (pentry->flags & RDF_COMPOUND)
		{
			resPtr = *resHdl + 2;				// Set resPtr to beginning of table
			offset = (long *)resPtr;			// and look at it as longs.
			for (j = 0; j < numEntries; j++)
			{
				resPtr = *resHdl + offset[j];
				ConvertData(resPtr, offset[j+1] - offset[j], pentry->type);
			}
		}
		else
		{
			ConvertData(resPtr, pentry->size, pentry->type);
		}

		// Finally, write out the Mac resource.  Free the resource after writing.
		HUnlock(resHdl);
		resName[0] = 1;
		if (pentry->flags & RDF_COMPOUND)		// Set the resource name to indicate 'n'ormal
			resName[1] = 'c';					// or 'c'ompound.
		else
			resName[1] = 'n';
		AddResource(resHdl, resMacTypes[pentry->type], pentry->id, resName);
		if (rerr = ResError())
			Debugger();
		WriteResource(resHdl);
		if (rerr = ResError())
			Debugger();
		ReleaseResource(resHdl);
		if (rerr = ResError())
			Debugger();
		if ((numResWritten % 20) == 0)
		{
			CloseResFile(macfilenum);
			if (rerr = ResError())
				Debugger();
			macfilenum = FSpOpenResFile(&outSpec, fsRdWrPerm);
			if (rerr = ResError())
				Debugger();
		}
		
		// Print out the vitals for this resource.
		sprintf(buff, "$%4x  %s  %6d  %6d  %3d   %c    %c    '%4.4s' #%d",
				pentry->id,
				tname,
				pentry->size,
				pentry->csize,
				compPct,
				flagLzw, flagComp,
				(char *)&resMacTypes[pentry->type],
				pentry->id);
		puts(buff);

		// Total up some stats.
		numResWritten++;
		totsize += pentry->size;
		totcsize += pentry->csize;
		dataOffset += pentry->csize + RES_OFFSET_PADBYTES(pentry->csize);
		pentry++;
	}

	//	Report total statistics

	printf("\nNumber of resource items: %d\n", dirHead.numEntries);
	if (numErased)
		printf("Num erased: %d, total size erased: %d\n", numErased,
			sizeErased);
	if (totsize)
	{
		printf("Total size: %d, total csize: %d, compression = %d%%\n",
			totsize, totcsize, ((totsize - totcsize) * 100) / totsize);
	}
	else
		printf("Resource file empty!\n");
	//	Close file & exit

	CloseResFile(macfilenum);
	ResCloseFile(filenum);
}


//---------------------------------------------------------------------------------
//  Create the output Mac resource file.
//---------------------------------------------------------------------------------
short CreateMacFile(char *macfname)
{
	short	launchWD;
	long	temp;
	OSErr	err;
	long	dirID;
	short	vRefNum;
	Str255	filename;
	FInfo	fi;
	
	GetVol(nil,	&launchWD);									// Where was I launched from?
 	GetWDInfo(launchWD, &vRefNum, &dirID, &temp);
	
	BlockMove(macfname, &filename[1], strlen(macfname));	// Make file name a pstr.
	filename[0] = strlen(macfname);
	
	err = FSMakeFSSpec(vRefNum, dirID, filename, &outSpec);	// Make the file's spec.
	if (err != 0 && err != -43)
		return(err);
	
	err = FSpGetFInfo(&outSpec, &fi);						// If file exists delete it.
	if (err == noErr)
		FSpDelete(&outSpec);

	FSpCreateResFile(&outSpec, 'Shok', 'Sdat', nil);		// Create the file.
	err = ResError();
	if (err)
		return (err);
	
	return(FSpOpenResFile(&outSpec, fsRdWrPerm));			// And open it as a resource file.
}

//---------------------------------------------------------------------------------
//  Convert the data based on its type.
//---------------------------------------------------------------------------------
void ConvertData(Ptr resPtr, long size, uchar dtype)
{
	char 	*p;
	short	*sp;
	int		i;
	
	switch(dtype)
	{
		case RTYPE_UNKNOWN:					// No conversion on these
		case RTYPE_OBJ3D:
		case RTYPE_STRING:
		case RTYPE_APP:
			p = resPtr;
			break;
		case RTYPE_IMAGE:					// Convert the image
			ConvertIMAGE(resPtr, size);
			break;
		case RTYPE_FONT:					// Convert the font
			ConvertFONT(resPtr, size);
			break;
		case RTYPE_RECT:
			sp = (short *)resPtr;
			for (i = 0; i < 8; i++)
			{
				*sp = SwapShortBytes(*sp);
				sp++;
			}
			break;
	}
}
