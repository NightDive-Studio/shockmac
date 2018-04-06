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
//		AFILE.C		Read/write anim files
//		Rex E. Bradford (REX)
//
/*
* $Header: r:/prj/lib/src/afile/RCS/afile.c 1.9 1994/10/18 16:00:29 rex Exp $
* $Log: afile.c $
 * Revision 1.9  1994/10/18  16:00:29  rex
 * Removed warning if can't set frame pal
 * 
 * Revision 1.8  1994/10/04  20:30:41  rex
 * Removed warning if no frame pal
 * 
 * Revision 1.7  1994/10/03  18:06:33  rex
 * Added warning if w,h of bitmap changes from frame to frame
 * 
 * Revision 1.6  1994/09/29  10:29:42  rex
 * Added time arg to read/write frame
 * 
 * Revision 1.5  1994/09/27  17:21:42  rex
 * Added qtmMethods to list of supported file types (Quicktime movies)
 * 
 * Revision 1.4  1994/09/22  16:42:41  rex
 * Took out warning when read past last frame
 * 
 * Revision 1.3  1994/09/13  12:22:32  rex
 * Added lots of spew
 * 
 * Revision 1.2  1994/08/04  11:39:48  rex
 * When reset afile, clear compose buffer
 * 
 * Revision 1.1  1994/07/22  13:20:00  rex
 * Initial revision
 * 
*/

#include <string.h>
#include <stdlib.h>

#include "lg.h"
#include "afile.h"
#include "compose.h"
//#include <rsd24.h>

//#include <_2d.h>

static char *afExts[] = {"FLC","FLI","CEL","ANM","qtm","mov",NULL};
static AfileType afTypes[] = {
	AFILE_FLC,AFILE_FLC,AFILE_FLC,AFILE_ANM,AFILE_QTM,AFILE_MOV};

//extern Amethods flcMethods;
//extern Amethods anmMethods;
extern Amethods qtmMethods;
extern Amethods movMethods;

static Amethods *methods[] = {
//	&flcMethods,		// AFILE_FLC
//	&anmMethods,		// AFILE_ANM
	NULL,		// AFILE_FLC
	NULL,		// AFILE_ANM
	&qtmMethods,		// AFILE_QTM
	&movMethods,		// AFILE_MOV
};

//	Allocate enough room in case RSD goes overboard

#define BM_PLENTY_SIZE(szuncomp) (((szuncomp) * 2) + 512)

//	-------------------------------------------------------
//		GENERAL ACCESS ROUTINES - READING
//	-------------------------------------------------------
//
//	AfileOpen() opens an anim file and begins reading from it.
//
//		paf      = ptr to (unused) animation file struct
//		filename = ptr to filename
//		pdp      = ptr to datapath
//
//	Returns:
//				0  = ok
//				-1 = bad extension
//				-2 = can't open file
//				-3 = bad file format

int AfileOpen(Afile *paf, char *filename)
{
	AfileType aftype;
	uchar bmtype;
	FILE *fp;
	char *p;

//	Spew

//	Spew(DSRC_2D_Afile, ("AfileOpen: trying to open: %s\n", filename));

//	Extract file extension, get type

	aftype = AFILE_BAD;
	p = strchr(filename, '.');
	if (p)
	{
		p++;
		*(p+3) = 0;
		aftype = AfileLookupType(p);
	}
	if (aftype == AFILE_BAD)
	{
		printf("AfileOpen: unknown extension\n");
		return(-1);
	}

//	Open file

//	fp = DatapathOpen(pdp, filename, "rb");
	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		printf("AfileOpen: can't open file\n");
		return -2;
	}

//	If opened successfully, set up afile struct

	memset(paf, 0, sizeof(Afile));
	paf->fp = fp;
	paf->type = aftype;
	paf->writing = FALSE;
	paf->pm = methods[paf->type];
	paf->currFrame = 0;

//	Call method to read header

//	Spew(DSRC_2D_Afile, ("AfileOpen: reading header\n"));

	if ((*paf->pm->f_ReadHeader)(paf) < 0)
	{
		printf("AfileOpen: bad header\n");
		return -3;
	}

//	Figure bitmap type and frame length

	if (paf->v.numBits == 8)
	{
		bmtype = BMT_FLAT8;
		paf->frameLen = (long) paf->v.width * paf->v.height;
	}
	else
	{
		bmtype = BMT_FLAT24;
		paf->frameLen = (long) paf->v.width * paf->v.height * 3;
	}

//	Spew(DSRC_2D_Afile, ("AfileOpen: numBits: %d  w,h: %d,%d  frameLen: %d\n",
//		paf->v.numBits, paf->v.width, paf->v.height, paf->frameLen));

//	Set up work buffer, compose buffer, and prev buffer

//	Spew(DSRC_2D_Afile, ("AfileOpen: initing work buffer of size: %d\n",
//		BM_PLENTY_SIZE(paf->frameLen)));

	gr_init_bitmap(&paf->bmWork, (uchar *)malloc(BM_PLENTY_SIZE(paf->frameLen)),
		bmtype, 0, paf->v.width, paf->v.height);

//	Spew(DSRC_2D_Afile, ("AfileOpen: initing compose buffer and prev buffer\n"));

	ComposeInit(&paf->bmCompose, bmtype, paf->v.width, paf->v.height);
	ComposeInit(&paf->bmPrev, bmtype, paf->v.width, paf->v.height);

//	Return ok

//	Spew(DSRC_2D_Afile, ("AfileOpen: successful open\n"));

	return 0;
}

//	-------------------------------------------------------------
//
//	AfileReadFullFrame() reads the next frame in the sequence, full style.
//
//		paf    = ptr to animfile struct
//		pbm    = ptr to bitmap struct (if ptr NULL, will alloc)
//		ptime  = ptr to time field (if ptr NULL, no time returned)
//
//	Returns: size of frame, or -1 if error

long AfileReadFullFrame(Afile *paf, grs_bitmap *pbm, fix *ptime)
{
	long len;
	fix time;

//	Hey, did we hit end?

//	Spew(DSRC_2D_Afile, ("AfileReadFullFrame: reading frame: %d\n",
//		paf->currFrame));

	if (paf->currFrame >= paf->v.numFrames)
	{
		return(-1);
	}

//	Read bitmap from reader into working buffer

	len = (*paf->pm->f_ReadFrame)(paf, &paf->bmWork, &time);
	if (ptime)
		*ptime = time;
	if (len <= 0)
	{
		printf("AfileReadFullFrame: problem reading frame\n");
		return(len);
	}
//	Spew(DSRC_2D_Afile, ("AfileReadFullFrame: read frame, len: %d\n", len));

//	Add to compose buffer

//	Spew(DSRC_2D_Afile, ("AfileReadFullFrame: adding to compose buffer\n"));
	ComposeAdd(&paf->bmCompose, &paf->bmWork);

//	Make sure bitmap has memory

	if (pbm->bits == NULL)
	{
//		Spew(DSRC_2D_Afile, ("AfileReadFullFrame: mallocing bitmap\n"));
		pbm->bits = (uchar *)malloc(paf->frameLen);
		if (pbm->bits == NULL)
		{
			printf("AfileReadFullFrame: can't find memory for bitmap\n");
			return(0);
		}
	}

//	Copy current compose buffer to caller

	gr_init_bm(pbm, pbm->bits, paf->bmCompose.type, 0, paf->v.width,
		paf->v.height);
	memcpy(pbm->bits, paf->bmCompose.bits, paf->frameLen);

//	Return length

	paf->currFrame++;
	return(paf->frameLen);
}
/*
//	-------------------------------------------------------------
//
//	AfileReadDiffFrame() reads the next frame in the sequence, diff style.
//
//		paf    = ptr to animfile struct
//		pbm    = ptr to bitmap struct (if ptr NULL, will alloc)
//		ptime  = ptr to time field (if ptr NULL, no time returned)
//
//	Returns: size of frame, or -1 if error

long AfileReadDiffFrame(Afile *paf, grs_bitmap *pbm, fix *ptime)
{
	long len;
	fix time;

//	Hey, did we hit end?

	Spew(DSRC_2D_Afile, ("AfileReadDiffFrame: reading frame: %d\n",
		paf->currFrame));

	if (paf->currFrame >= paf->v.numFrames)
		{
		return(-1);
		}

//	Read bitmap from reader into working buffer

	len = (*paf->pm->f_ReadFrame)(paf, &paf->bmWork, &time);
	if (ptime)
		*ptime = time;
	if (len <= 0)
		{
		Warning(("AfileReadDiffFrame: problem reading frame\n"));
		return(len);
		}
	Spew(DSRC_2D_Afile, ("AfileReadDiffFrame: read frame, len: %d\n", len));

//	Move compose buffer to previous

	if (paf->currFrame > 0)
		memcpy(paf->bmPrev.bits, paf->bmCompose.bits, paf->frameLen);

//	Add to compose buffer

	ComposeAdd(&paf->bmCompose, &paf->bmWork);

//	Make sure bitmap has memory, init it

	if (pbm->bits == NULL)
		{
		Spew(DSRC_2D_Afile, ("AfileReadDiffFrame: mallocing bitmap\n"));
		pbm->bits = Malloc(paf->frameLen);
		if (pbm->bits == NULL)
			{
			Warning(("AfileReadDiffFrame: can't find memory for bitmap\n"));
			return(0);
			}
		}

//	Extract difference into bitmap

	Spew(DSRC_2D_Afile, ("AfileReadDiffFrame: finding diff with compose buff\n"));
	len = ComposeDiff(&paf->bmPrev, &paf->bmCompose, pbm);

//	Return length

	paf->currFrame++;
	return(len);
}
*/
//	--------------------------------------------------------------
//
//	AfileGetFramePal() gets a (partial) palette associated with this
//	frame only.  Call AFTER AfileReadFrame().
//
//		paf  = ptr to animfile struct
//		ppal = ptr to palette struct
//
//	Returns: TRUE if palette for this frame, FALSE if none

bool AfileGetFramePal(Afile *paf, Apalette *ppal)
{
//	Spew(DSRC_2D_Afile, ("AfileGetFramePal: getting pal\n"));

	if (paf->pm->f_ReadFramePal == NULL)
		return FALSE;

	(*paf->pm->f_ReadFramePal)(paf, ppal);
	return(ppal->numcols != 0);
}

//	--------------------------------------------------------------
//
//	AfileGetAudio() gets giant block of audio for entire animation.
//	Use AudioFileLength() for sizing when allocate buffer.
//
//		paf    = ptr to animfile struct
//		paudio = ptr to audio buffer
//
//	Returns: 0 if ok, -1 if error

int AfileGetAudio(Afile *paf, void *paudio)
{
//	Spew(DSRC_2D_Afile, ("AfileGetAudio: getting audio\n"));

	if (paf->pm->f_ReadAudio == NULL)
	{
		printf("AfileGetAudio: anim file format doesn't support audio\n");
		return(-1);
	}

	return((*paf->pm->f_ReadAudio)(paf, paudio));
}
/*
//	--------------------------------------------------------------
//
//	AfileReadReset() resets to frame 0.

int AfileReadReset(Afile *paf)
{
	Spew(DSRC_2D_Afile, ("AfileReadReset: resetting\n"));

	paf->currFrame = 0;
	memset(paf->bmCompose.bits, 0, paf->bmCompose.row * paf->bmCompose.h);
	memset(paf->bmPrev.bits, 0, paf->bmPrev.row * paf->bmPrev.h);
	return((*paf->pm->f_ReadReset)(paf));
}
*/
//	--------------------------------------------------------------
//
//	AfileClose() closes animfile.
//
//		paf = ptr to animfile struct

void AfileClose(Afile *paf)
{
//	Close properly based on whether writing or reading

//	Spew(DSRC_2D_Afile, ("AfileClose: closing file\n"));

	if (paf->writing)
	{
		paf->v.numFrames = paf->currFrame;
		(*paf->pm->f_WriteClose)(paf);
	}
	else
		(*paf->pm->f_ReadClose)(paf);

//	Free up buffers

	if (paf->bmCompose.bits)
		free(paf->bmCompose.bits);
	if (paf->bmWork.bits)
		free(paf->bmWork.bits);
	if (paf->bmPrev.bits)
		free(paf->bmPrev.bits);

//	Close file

	fclose(paf->fp);
}

//	--------------------------------------------------------------
//		INFORMATIONAL AND HELPER ROUTINES
//	--------------------------------------------------------------
//
//	AfileLookupType() looks up anim file type given extension.

AfileType AfileLookupType(char *ext)
{
	int itype;

	itype = 0;
	while (afExts[itype])
	{
		if (strcmp(ext, afExts[itype]) == 0)
			return(afTypes[itype]);
		++itype;
	}
	return(AFILE_BAD);
}

//	--------------------------------------------------------------
//
//	AfileBitmapLength() returns amount of space needed to read bitmaps.

long AfileBitmapLength(Afile *paf)
{
	return(paf->frameLen);
}

//	-------------------------------------------------------------
//
//	AfileAudioLength() computes the length of the buffer needed
//	to store audio data.  Hope it all fits into ram!

long AfileAudioLength(Afile *paf)
{
	if (paf->a.numChans == 0)
		return(0);
	else
		return(paf->a.numChans * paf->a.sampleSize * paf->a.numSamples);
}

//	-------------------------------------------------------------
//		GENERAL ACCESS ROUTINES - WRITING
//	-------------------------------------------------------------
//
//	AfileCreate() creates a new anim file.
//
//		paf       = ptr to (unused) animation file struct
//		filename  = ptr to filename
//		frameRate = video frame rate
//
//	Returns:
//				0  = ok
//				-1 = bad extension
//				-2 = no writer for this type
//				-3 = can't open file

int AfileCreate(Afile *paf, char *filename, fix frameRate)
{
	AfileType aftype;
	FILE *fp;
	char *p;

//	Extract file extension, get type

//	Spew(DSRC_2D_Afile, ("AfileCreate: creating %s\n", filename));

	aftype = AFILE_BAD;
	p = strchr(filename, '.');
	if (p)
	{
		p++;
		*(p+3) = 0;
		aftype = AfileLookupType(p);
	}
	if (aftype == AFILE_BAD)
	{
		printf("AfileCreate: unknown extension\n");
		return(-1);
	}

//	Check if writer

	if (methods[aftype]->f_WriteBegin == NULL)
	{
		printf("AfileCreate: anim file format doesn't support writing\n");
		return(-2);
	}

//	Open file

	fp = fopen(filename, "wb");
	if (fp == NULL)
	{
		printf("AfileCreate: can't open file\n");
		return(-3);
	}

//	If opened successfully, set up afile struct

	memset(paf, 0, sizeof(Afile));
	paf->fp = fp;
	paf->type = aftype;
	paf->writing = TRUE;
	paf->pm = methods[paf->type];

	paf->v.frameRate = frameRate;

//	Call method to begin writing

//	Spew(DSRC_2D_Afile, ("AfileCreate: initializing anim file\n"));

	(*paf->pm->f_WriteBegin)(paf);

//	Return ok

//	Spew(DSRC_2D_Afile, ("AfileCreate: successful create\n"));

	return 0;
}

//	-----------------------------------------------------------
//
//	AfilePutAudio() hands off audio buffer to writer.
//	Call this before writing any frames.

int AfilePutAudio(Afile *paf, AaudioInfo *pai, void *paudio)
{
//	Can we do audio?

//	Spew(DSRC_2D_Afile, ("AfilePutAudio: putting audio\n"));

	if (*paf->pm->f_WriteAudio == NULL)
	{
		printf("AfilePutAudio: anim file doesn't support writing audio\n");
		return(-1);
	}

//	Set audio section of animfile struct

	paf->a = *pai;

//	Hand off to function

	return((*paf->pm->f_WriteAudio)(paf, paudio));
}

//	------------------------------------------------------------
//
//	AfileWriteFrame() writes frame out to writer.

int AfileWriteFrame(Afile *paf, grs_bitmap *pbm, fix time)
{
	int bmtype;
	long len;
	int ret;

//	Spew(DSRC_2D_Afile, ("AfileWriteFrame: writing frame: %d\n",
//		paf->currFrame));

//	If 1st frame, init some vars.
//	Set up work buffer, compose buffer, and prev buffer

	if (paf->currFrame == 0)
	{
		paf->v.width = pbm->w;
		paf->v.height = pbm->h;
		if ((pbm->type == BMT_FLAT8) || (pbm->type == BMT_RSD8))
		{
			paf->v.numBits = 8;
			bmtype = BMT_FLAT8;
			paf->frameLen = (long) paf->v.width * paf->v.height;
		}
		else
		{
			paf->v.numBits = 24;
			bmtype = BMT_FLAT24;
			paf->frameLen = (long) paf->v.width * paf->v.height * 3;
		}
//		Spew(DSRC_2D_Afile, ("AfileWriteFrame: numBits: %d  w,h: %d,%d  frameLen: %d\n",
//			paf->v.numBits, paf->v.width, paf->v.height, paf->frameLen));

//		Spew(DSRC_2D_Afile, ("AfileWriteFrame: initing work buffer of size: %d\n",
//			BM_PLENTY_SIZE(paf->frameLen)));

		gr_init_bitmap(&paf->bmWork, (uchar *)malloc(BM_PLENTY_SIZE(paf->frameLen)),
			bmtype, 0, paf->v.width, paf->v.height);

//		Spew(DSRC_2D_Afile, ("AfileWriteFrame: initing compose buffers\n"));

		ComposeInit(&paf->bmCompose, bmtype, paf->v.width, paf->v.height);
		ComposeInit(&paf->bmPrev, bmtype, paf->v.width, paf->v.height);
	}

//	Else for other frames, copy current to previous

	else
	{
//		if ((pbm->w != paf->v.width) || (pbm->h != paf->v.height))
//		{
//			Warning(("AfileWriteFrame: new w,h: %d,%d (was: %d,%d)\n",
//				pbm->w, pbm->h, paf->v.width, paf->v.height));
//		}
		memcpy(paf->bmPrev.bits, paf->bmCompose.bits, paf->frameLen);
		if (time == 0)
			time = paf->currFrame * fix_div(FIX_UNIT, paf->v.frameRate);
	}

//	Now put current frame into compose buffer

//	Spew(DSRC_2D_Afile, ("AfileWriteFrame: adding to compose buff\n"));

	ComposeAdd(&paf->bmCompose, pbm);

//	If writer wants rsd, give rsd-encoded frame (diff if past frame 0)

	if (paf->writerWantsRsd)
	{
//		Spew(DSRC_2D_Afile, ("AfileWriteFrame: converting to rsd\n"));

		if (paf->currFrame == 0)
		{
			if (paf->bmCompose.type == BMT_FLAT8)
				paf->bmWork.type = BMT_RSD8;
			else
//				paf->bmWork.type = BMT_RSD24;		// no rsd24 support yet
				paf->bmWork.type = BMT_FLAT24;
			len = ComposeConvert(&paf->bmCompose, &paf->bmWork);
		}
		else
		{
			// The next 3 lines should be unnecessary, but rsd24 broken
			if (paf->bmCompose.type == BMT_FLAT24)
				len = ComposeConvert(&paf->bmCompose, &paf->bmWork);
			else
				len = ComposeDiff(&paf->bmPrev, &paf->bmCompose, &paf->bmWork);
		}
//		Spew(DSRC_2D_Afile, ("AfileWriteFrame: writing, rsd len = %d\n", len));

		ret = (*paf->pm->f_WriteFrame)(paf, &paf->bmWork, len, time);
	}

//	Else just write flat frame

	else
	{
//		Spew(DSRC_2D_Afile, ("AfileWriteFrame: writing, len = %d\n",
//			paf->frameLen));
		ret = (*paf->pm->f_WriteFrame)(paf, &paf->bmCompose, paf->frameLen, time);
	}

//	Bump frame counter and return

	if (ret >= 0)
		paf->currFrame++;
	return(ret);
}

//	-------------------------------------------------------------
//
//	AfileSetPal() sets overall anim palette.
//	Call this before writing any frames.

void AfileSetPal(Afile *paf, Apalette *ppal)
{
//	Spew(DSRC_2D_Afile, ("AfileSetPal: setting palette\n"));

	memcpy(&paf->v.pal, ppal, sizeof(Apalette));
}

//	-------------------------------------------------------------
//
//	AfileSetFramePal() sets palette for upcoming frame.

int AfileSetFramePal(Afile *paf, Apalette *ppal)
{
//	Spew(DSRC_2D_Afile, ("AfileSetFramePal: setting frame pal\n"));

	if (paf->pm->f_WriteFramePal == NULL)
		return(-1);
	return((*paf->pm->f_WriteFramePal)(paf,ppal));
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
