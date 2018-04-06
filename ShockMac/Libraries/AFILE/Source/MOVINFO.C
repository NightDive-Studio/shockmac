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
//		MOVINFO		Dump info about movie
//		Rex E. Bradford

/*
 * $Source: r:/prj/lib/src/afile/RCS/movinfo.c $
 * $Revision: 1.5 $
 * $Author: rex $
 * $Date: 1994/10/24 12:53:52 $
 * $Log: movinfo.c $
 * Revision 1.5  1994/10/24  12:53:52  rex
 * Added chunk numbers
 * 
 * Revision 1.4  1994/10/20  14:11:54  rex
 * Added chunk # to printout
 * 
 * Revision 1.3  1994/10/18  20:15:52  rex
 * Reduced flags to 4 bits, adjusted tables accordingly
 * 
 * Revision 1.2  1994/09/01  11:07:46  rex
 * Print out PALETTE chunk now
 * 
 * Revision 1.1  1994/08/22  17:38:12  rex
 * Initial revision
 * 
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
	char buff[128];

	dumpChunkHdrs = TRUE;

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
	printf("   num chunks:     %d\n", mh.numChunks);
	printf("   size chunks:    %dK\n", mh.sizeChunks >> 10);
	printf("   size data:      %d\n", mh.sizeData);
	fix_sprint (buff, mh.totalTime);
	printf("   total time:     %s\n", buff);
	fix_sprint (buff, mh.frameRate);
	printf("   frame rate:     %s\n", buff);
	printf("   frame size:     %d x %d\n", mh.frameWidth, mh.frameHeight);
	printf("   Video bits/pix: %d\n", mh.gfxNumBits);
	printf("   Palette:        %s\n", mh.isPalette ? "YES" : "NO");
	printf("   Audio channels: %d\n", mh.audioNumChans);
	printf("   Audio sampsize: %d\n", mh.audioSampleSize);
	fix_sprint (buff, mh.audioSampleRate);
	printf("   Audio rate:     %s\n", buff);

//	If dumping chunks, do them

	if (dumpChunkHdrs)
	{
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
 				ulong nxoff = SwapLongBytes((pmc + 1)->offset);
				length = nxoff - pmc->offset;

				fix_sprint (buff, pmc->time);
				printf("[%d] %s  time: %s offset: %d ($%x) len: %d ",
					pmc - pmcBase, chunkNames[pmc->chunkType], buff,
					pmc->offset, pmc->offset, length);

   				switch (pmc->chunkType)
   				{
   				case MOVIE_CHUNK_VIDEO:
					printf("%s\n", bmTypeNames[pmc->flags & MOVIE_FVIDEO_BMTMASK]);
					break;

   				case MOVIE_CHUNK_PALETTE:
					printf("%s", palNames[pmc->flags & MOVIE_FPAL_EFFECTMASK]);
					if (pmc->flags & MOVIE_FPAL_CLEAR)
						printf(" [CLEAR]");
					printf("\n", buff);
					break;

   				case MOVIE_CHUNK_TABLE:
					printf("%s\n", tableNames[pmc->flags]);
					break;

   				case MOVIE_CHUNK_AUDIO:
	   			case MOVIE_CHUNK_TEXT:
		        default:
					printf("\n");
					break;
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

//	Close file

	fclose(fpi);
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
