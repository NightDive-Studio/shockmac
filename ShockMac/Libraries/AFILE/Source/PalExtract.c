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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lg.h"
#include "fix.h"

#include "movie.h"

long SwapLongBytes(long in);
short SwapShortBytes(short in);

MovieHeader mh;
char buff[768];

typedef struct
{
	long 	frameNum;
	short	palID;
} PalChange;


//	--------------------------------------------------------------
//		MAIN PROGRAM
//	--------------------------------------------------------------

void main(void)
{
static char *chunkNames[] = {
   "END  ","VIDEO","AUDIO","TEXT ","PAL  ","TABLE","?????","?????"};
static char *bmTypeNames[] = {
   "DEVICE","MONO","FLAT8","FLAT24","RSD8","TLUC8","SPAN","GEN",
	"","","","","","","","4X4",
};
static char *palNames[] = {
	"SET","BLACK","???","???","???","???","???","???",
};
static char *tableNames[] = {
	"COLORSET","HUFFTAB","???","???","???","???","???","???",
	"???","???","???","???","???","???","???","???",
};

	FILE *fpi;
	int iarg;
	bool dumpChunkHdrs;
	long length;
	MovieChunk *pmc,*pmcBase;
	char infile[128];
	
	short	outResNum;
	short	resID = 128;

	long	frames = 0;
	dumpChunkHdrs = TRUE;

	Handle		changesHdl;
	PalChange	*pcp;
	
// Prompt for input file

	printf ("File to dump: ");
	fgets (infile, sizeof(infile), stdin);
	infile[strlen(infile)-1] = 0;
	if (strchr(infile, '.') == NULL)
		strcat(infile, ".mov");

//	Open input file

	fpi = fopen(infile, "rb");
	if (fpi == NULL)
		{
		printf("Can't open: %s\n", infile);
		exit(1);
		}

// Open the output resource file for saving the palette resources into.
	
	{
		StandardFileReply	reply;
		OSErr				err;
	
		StandardPutFile("\pPalette output file:", "\pMovie Palettes", &reply);
		if (!reply.sfGood)
			return;
	
		if (reply.sfReplacing)							// Delete the file if it exists.
			FSpDelete(&reply.sfFile);
	
		FSpCreateResFile(&reply.sfFile, 'Shok', 'rsrc', nil);	// Create the resource file.
		err = ResError();
		if (err != 0)
		{
			printf("Can't create file.\n");
			return;
		}	
		outResNum = FSpOpenResFile(&reply.sfFile, fsRdWrPerm);	// Open the file for writing.
		if (outResNum == -1)
		{
			printf("Can't open file.\n");
			return;
		}	
	}

//	Get movie header, check for valid movie file

	fread(&mh, sizeof(mh), 1, fpi);
	if (mh.magicId != MOVI_MAGIC_ID)
		{
		printf("%s not a valid .mov file!\n", infile);
		exit(1);
		}

// Swap all those bytes around.

	mh.numChunks = SwapLongBytes(mh.numChunks);
	mh.sizeChunks = SwapLongBytes(mh.sizeChunks);
	mh.sizeData = SwapLongBytes(mh.sizeData);
	mh.totalTime = SwapLongBytes(mh.totalTime);
	mh.frameRate = SwapLongBytes(mh.frameRate);
	mh.frameWidth = SwapShortBytes(mh.frameWidth);
	mh.frameHeight = SwapShortBytes(mh.frameHeight);
	mh.gfxNumBits = SwapShortBytes(mh.gfxNumBits);
	mh.isPalette = SwapShortBytes(mh.isPalette);
	mh.audioNumChans = SwapShortBytes(mh.audioNumChans);
	mh.audioSampleSize = SwapShortBytes(mh.audioSampleSize);
	mh.audioSampleRate = SwapLongBytes(mh.audioSampleRate);
	
//	Dump movie header

	printf("Movie header:\n");
	fix_sprint (buff, mh.totalTime);
	printf("   total time:     %s\n", buff);
	fix_sprint (buff, mh.frameRate);
	printf("   frame rate:     %s\n", buff);
	printf("   Video bits/pix: %d\n", mh.gfxNumBits);
	printf("   Palette:        %s\n\n", mh.isPalette ? "YES" : "NO");

//	If the movie has a palette in the header, write it out.

	if (mh.isPalette)
	{
		printf("Writing palette from header: %d\n\n", resID);
		Handle	palHdl = NewHandle(768);
		HLock(palHdl);
		BlockMove(mh.palette, *palHdl, 768);
		HUnlock(palHdl);

		AddResource(palHdl, 'mpal', resID++, "\ppal");
		WriteResource(palHdl);
		ReleaseResource(palHdl);

		// Allocate a handle to hold the palette change info.
		
		changesHdl = NewHandle(16 * sizeof(PalChange));
		HLock(changesHdl);
		pcp = (PalChange *)*changesHdl;
		
		// Go through and dump the chunks.
		
		pmc = (MovieChunk *)malloc(mh.sizeChunks);
		fread(pmc, mh.sizeChunks, 1, fpi);
      	pmcBase = pmc;
		while (TRUE)
		{
			uchar	s1, s2;
			
			// Swap bytes around.
			s1 = *((uchar *)pmc);
			s2 = *(((uchar *)pmc)+2);
			*(((uchar*)pmc)+2) = s1;
			*((uchar*)pmc) = s2;
 			pmc->offset = SwapLongBytes(pmc->offset);
			
			// Print info for each chunk type.
			if (pmc->chunkType != MOVIE_CHUNK_END)
 			{
   				if (pmc->chunkType == MOVIE_CHUNK_VIDEO)
   				{
   					frames++;
   				}
   				else if (pmc->chunkType == MOVIE_CHUNK_PALETTE)
   				{
					fix_sprint (buff, pmc->time);
					printf("time: %s  before frame: %d  ", buff, frames);
					printf("%s", palNames[pmc->flags & MOVIE_FPAL_EFFECTMASK]);
					if (pmc->flags & MOVIE_FPAL_CLEAR)
						printf(" [CLEAR]");
					printf("\n");
					
					if ((pmc->flags & MOVIE_FPAL_EFFECTMASK) == MOVIE_FPAL_SET)
					{
						fseek(fpi, pmc->offset, SEEK_SET);		// Read the palette
						fread(buff, 768, 1, fpi);
						
						pcp->frameNum = frames;					// Save change info
						pcp->palID = resID;
						pcp++;

						printf("Writing palette: %d\n\n", resID);
						
						Handle	palHdl = NewHandle(768);		// Add to output file
						HLock(palHdl);
						BlockMove(buff, *palHdl, 768);
						HUnlock(palHdl);
						
						AddResource(palHdl, 'mpal', resID++, "\ppal");
						WriteResource(palHdl);
						ReleaseResource(palHdl);
					}
				}
            }
			else
			{
				fix_sprint (buff, pmc->time);
				printf("END:     time: %s\n", buff);
			}

			if (pmc->chunkType == MOVIE_CHUNK_END)
				break;
			++pmc;
		}
	}

// 	Add final changes entry and write changes resource.

	pcp->frameNum = 0;
	pcp->palID = 0;
	HUnlock(changesHdl);
	AddResource(changesHdl, 'pchg', 128, "\pchanges");
	WriteResource(changesHdl);
	ReleaseResource(changesHdl);
	
//	Close file

	fclose(fpi);
	CloseResFile(outResNum);
}


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
