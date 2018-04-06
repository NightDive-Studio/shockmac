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
//		AQTM.C		Animfile handler for Quicktime movie files
//		Rex E. Bradford (REX)
//
/*
* $Header: r:/prj/lib/src/afile/RCS/aqtm.c 1.4 1994/10/18 16:00:44 rex Exp $
* $Log: aqtm.c $
 * Revision 1.4  1994/10/18  16:00:44  rex
 * Added ability to "write" frame pal for 16-bit movies, so pal changes
 * can cause 16-bit colors to be written correctly
 * 
 * Revision 1.3  1994/09/30  16:57:53  rex
 * Added routines for movie writing
 * 
 * Revision 1.2  1994/09/29  10:32:14  rex
 * Revamped with new quiktime.c api
 * 
 * Revision 1.1  1994/09/27  17:22:51  rex
 * Initial revision
 * 
 * 
*/

#include <string.h>
#include <stdlib.h>

#include "lg.h"
//#include <rsd.h>
#include "rect.h"
#include "afile.h"
#include "compose.h"
#include "quiktime.h"

//	--------------------------------------------------------------
//		VIRTUAL FUNCTION TABLE AND RELATED INFO
//	--------------------------------------------------------------

//	Type-specific information

typedef struct {
	QTM qtm;
} AqtmInfo;

//	Methods

int AqtmReadHeader(Afile *paf);
long AqtmReadFrame(Afile *paf, grs_bitmap *pbm, fix *ptime);
int AqtmReadReset(Afile *paf);
int AqtmReadClose(Afile *paf);
int AqtmWriteBegin(Afile *paf);
int AqtmWriteFrame(Afile *paf, grs_bitmap *pbm, long bmlength, fix time);
int AqtmWriteFramePal(Afile *paf, Apalette *ppal);
int AqtmWriteClose(Afile *paf);

Amethods qtmMethods = {
	AqtmReadHeader,
	AqtmReadFrame,
	NULL,					// f_ReadFramePal
	NULL,					// f_ReadAudio
	AqtmReadReset,
	AqtmReadClose,
	AqtmWriteBegin,
	NULL,					// f_WriteAudio
	AqtmWriteFrame,
   AqtmWriteFramePal,
	AqtmWriteClose,
};

//	------------------------------------------------------
//		READER METHODS
//	------------------------------------------------------
//
//	AqtmReadHeader() reads in quiktime chunks & verifies.

int AqtmReadHeader(Afile *paf)
{
	printf("AqtmReadHeader not implemented yet!\n");
/*
	AqtmInfo *pqi;
	QTM *pqtm;

	paf->pspec = Calloc(sizeof(AqtmInfo));
	pqi = paf->pspec;
	pqtm = &pqi->qtm;

//	Read in Quiktime header chunks

	QuikReadMovie(pqtm, paf->fp);

//	Record header information

	if (pqtm->pVideoTrack)
		{
		paf->v.frameRate = QuikComputeFrameRate(pqtm);
		paf->v.width = pqtm->pVideoTrack->qt_stsd.idesc.width;
		paf->v.height = pqtm->pVideoTrack->qt_stsd.idesc.height;
		paf->v.numBits = pqtm->pVideoTrack->qt_stsd.idesc.depth;
		paf->v.numFrames = pqtm->pVideoTrack->numSamps;
		if (pqtm->pVideoTrack->palette)
			{
			paf->v.pal.index = 0;
			paf->v.pal.numcols = 256;
			memcpy(paf->v.pal.rgb, pqtm->pVideoTrack->palette, 256 * 3);
			}
		}

//	Return
*/
	return(0);
}

//	----------------------------------------------------------
//
//	AqtmReadFrame() reads the next frame.

long AqtmReadFrame(Afile *paf, grs_bitmap *pbm, fix *ptime)
{
	printf("AqtmReadFrame not implemented yet!\n");
/*
	AqtmInfo *pqi;
	QTM *pqtm;
	void *p;
	long length;
	fix time;
	uchar bmtype;

	pqi = paf->pspec;
	pqtm = &pqi->qtm;

	p = QuikGetVideoSample(pqtm, paf->currFrame, paf->fp, &length, &bmtype,
		&time);

	pbm->type = bmtype;
	memcpy(pbm->bits, p, length);

	*ptime = time;

	return(length);
*/
}

//	----------------------------------------------------------
//
//	AqtmReadReset() resets the movie for reading.

int AqtmReadReset(Afile *paf)
{
	return(0);
}

//	----------------------------------------------------------
//
//	AqtmReadClose() does cleanup and closes file.

int AqtmReadClose(Afile *paf)
{
	printf("AqtmReadClose not implemented yet!\n");
/*
	AqtmInfo *pqi;

	pqi = paf->pspec;
	QuikFreeMovie(&pqi->qtm);

	fclose(paf->fp);
*/
	return(0);
}

//	------------------------------------------------------
//		WRITER METHODS
//	------------------------------------------------------
//
//	AqtmWriteBegin() starts up writer.

int AqtmWriteBegin(Afile *paf)
{
	AqtmInfo *pqi;

//	Allocate type-specific info

	paf->pspec = malloc(sizeof(AqtmInfo));
	pqi = (AqtmInfo *)paf->pspec;

//	Start creating movie

	QuikCreateMovie(&pqi->qtm, paf->fp);

//	We don't need no stinkin' rsd!

	paf->writerWantsRsd = FALSE;

//	Return

	return(0);
}

//	------------------------------------------------------
//
//	AqtmWriteFrame() writes out next frame.

int AqtmWriteFrame(Afile *paf, grs_bitmap *pbm, long bmlength, fix time)
{
	AqtmInfo *pqi;

	pqi = (AqtmInfo *)paf->pspec;
	if ((paf->currFrame == 0) && paf->v.pal.numcols)
		QuikSetPal(&pqi->qtm, paf->v.pal.rgb);
	QuikAddVideoSample(&pqi->qtm, paf->fp, pbm, time);

	return(0);
}

// -------------------------------------------------------
//
// AqtmWriteFramePal() handles palette changes for 16-bit movies.

int AqtmWriteFramePal(Afile *paf, Apalette *ppal)
{
	AqtmInfo *pqi;
	QTM *pqtm;

	pqi = (AqtmInfo *)paf->pspec;
	pqtm = &pqi->qtm;
	if (pqtm->depth16 && pqtm->pVideoTrack && pqtm->pVideoTrack->palette)
	{
		memcpy(pqtm->pVideoTrack->palette + (ppal->index * 3),
		ppal->rgb + (ppal->index * 3), ppal->numcols * 3);
	}

   return(0);
}

//	-------------------------------------------------------
//
//	AqtmWriteClose() closes output movie

int AqtmWriteClose(Afile *paf)
{
	AqtmInfo *pqi;

	pqi = (AqtmInfo *)paf->pspec;

	QuikWriteMovieAndClose(&pqi->qtm, paf->fp);
	free(pqi);
	fclose(paf->fp);

	return(0);
}

