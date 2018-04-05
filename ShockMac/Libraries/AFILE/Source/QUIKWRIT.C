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
//		QUIKWRIT.C		QuickTime file writing
//		Rex E. Bradford

/*
 * $Source: r:/prj/lib/src/afile/RCS/quikwrit.c $
 * $Revision: 1.6 $
 * $Author: rex $
 * $Date: 1994/10/18 16:01:53 $
 * $Log: quikwrit.c $
 * Revision 1.6  1994/10/18  16:01:53  rex
 * Got 16-bit depth working
 * 
 * Revision 1.5  1994/10/04  20:42:01  rex
 * Fixed bugs related to variable-timed frames, put in basic support
 * for 16-bit depth frames, but doesn't work because rgb bitorder unknown
 * 
 * Revision 1.4  1994/10/03  19:43:38  rex
 * Modified to respect varying frame rate
 * 
 * Revision 1.3  1994/10/03  18:05:20  rex
 * Modified QuikAddVideoSample() to handle bitmaps where w != row
 * 
 * Revision 1.2  1994/09/30  17:00:18  rex
 * Made it actually work
 * 
 * Revision 1.1  1994/09/29  10:36:52  rex
 * Initial revision
 * 
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lg.h"
#include "quiktime.h"

//	Handy macros

#define FIX8(v) ((v)>>8)
#define DEFAULT_MACTIME 2855653576
#define DEFAULT_TIMESCALE 600
#define FRAME_ALLOC_INCR 128
#define MDHD_TIMESCALE 30

//	Internal prototypes

void QuikStartSubChunk(QTM *pqtm, FILE *fp, QT_Ctype ctype, void *data,
	long len);
void QuikEndSubChunk(QTM *pqtm, FILE *fp);
void QuikWriteMVHD(QTM *pqtm, FILE *fp, ulong timeTot);
void QuikWriteVideoTrack(QTM *pqtm, FILE *fp, ulong timeTot);
void WriteChunkWithString(FILE *fp, QT_Ctype ctype, void *data,
	long len, char *str);
ulong QuikComputeTotalTime(QTM *pqtm);
long QuikSampleLenMsec(fix t, fix tlast);
fix QuikSampleLenMsecBackToFix(long msec);

//	-------------------------------------------------------------
//		MOVIE WRITING
//	-------------------------------------------------------------
//
//	QuikCreateMovie() creates a quiktime movie ready for writing.

void QuikCreateMovie(QTM *pqtm, FILE *fp)
{
	memset(pqtm, 0, sizeof(QTM));

	QuikStartSubChunk(pqtm, fp, QT_MDAT, NULL, 0);
}
/*
//	-------------------------------------------------------------
//
//	QuikSetPal() sets the video track's palette

void QuikSetPal(QTM *pqtm, uchar *pal)
{
	if (pqtm->pVideoTrack == NULL)
		pqtm->pVideoTrack = &pqtm->track[pqtm->numTracks++];

	pqtm->pVideoTrack->palette = Malloc(768);
	memcpy(pqtm->pVideoTrack->palette, pal, 768);
}
*/
//	-------------------------------------------------------------
//
//	QuikAddVideoSample() adds a video sample

void QuikAddVideoSample(QTM *pqtm, FILE *fp, grs_bitmap *pbm, fix time)
{
	MovieTrack *ptrack;
	uchar *p;
	uchar *ppal;
	uchar *pbuff;
	uchar *pd;
	uchar *prgb;
	int x,y;
	ushort color16;

//	Can't handle anything but 8-bit raw

	if (pbm->type != BMT_FLAT8)
	{
		printf("QuikAddVideoSample: only uncompressed bitmaps allowed\n");
		return;
	}

//	Write out frame bits in 8-bit of 16-bit mode

	if (pqtm->depth16)
	{
		if ((pqtm->pVideoTrack == NULL) || (pqtm->pVideoTrack->palette == NULL))
		{
			printf("QuikAddVideoSample: need pal to convert to 16-bits!\n");
			return;
		}
		ppal = pqtm->pVideoTrack->palette;
		pbuff = (uchar *)malloc(pbm->w * sizeof(ushort));
		for (y = 0, p = pbm->bits; y < pbm->h; y++, p += (pbm->row - pbm->w))
		{
			for (x = 0, pd = pbuff; x < pbm->w; x++)
			{
				prgb = &ppal[((ushort) *p++) * 3];
				color16 = (ushort)(*prgb++ >> 3) << 10;
				color16 |= (ushort)(*prgb++ >> 3) << 5;
				color16 |= (ushort)(*prgb++ >> 3);
				*pd++ = color16 >> 8;
				*pd++ = color16 & 0xFF;
			}
			fwrite(pbuff, pbm->w * sizeof(ushort), 1, fp);
		}
		free(pbuff);
	}
	else
	{
		if (pbm->w == pbm->row)
			fwrite(pbm->bits, (long) pbm->w * (long) pbm->h, 1, fp);
		else
		{
			for (y = 0, p = pbm->bits; y < pbm->h; y++, p += pbm->row)
				fwrite(p, pbm->w, 1, fp);
		}
	}

//	If video track not assigned, assign it

	if (pqtm->pVideoTrack == NULL)
		pqtm->pVideoTrack = &pqtm->track[pqtm->numTracks++];

	ptrack = pqtm->pVideoTrack;

//	If video track not set up, set it up

	if (pqtm->compTypeQT == 0)
	{
		pqtm->compTypeQT = MAKE4('r','a','w',' ');
		pqtm->frameWidth = pbm->w;
		pqtm->frameHeight = pbm->h;

		ptrack->qt_tkhd.flags[2] = 0x0F;
		ptrack->qt_tkhd.createTime = DEFAULT_MACTIME;
		ptrack->qt_tkhd.modTime = DEFAULT_MACTIME;
		ptrack->qt_tkhd.trackId = (ptrack - &pqtm->track[0]) + 1;
		ptrack->qt_tkhd.matrix[0] = FIX_UNIT;
		ptrack->qt_tkhd.matrix[4] = FIX_UNIT;
		ptrack->qt_tkhd.matrix[8] = fix_make(16384,0);
		ptrack->qt_tkhd.trackWidth = fix_make(pbm->w, 0);
		ptrack->qt_tkhd.trackHeight = fix_make(pbm->h, 0);

		ptrack->qt_mdhd.createTime = DEFAULT_MACTIME;
		ptrack->qt_mdhd.modTime = DEFAULT_MACTIME;

		ptrack->qt_stsd.base.numEntries = 1;
		if (pqtm->depth16)
			ptrack->qt_stsd.idesc.descSize = 86;
		else
			ptrack->qt_stsd.idesc.descSize = 2142;
		ptrack->qt_stsd.idesc.dataFormat = MAKE4('r','a','w',' ');
		ptrack->qt_stsd.idesc.dataRefIndex = 1;
		ptrack->qt_stsd.idesc.version = 1;
		ptrack->qt_stsd.idesc.revLevel = 1;
		ptrack->qt_stsd.idesc.vendor = MAKE4('a','p','p','l');
		ptrack->qt_stsd.idesc.temporalQuality = 0;
		ptrack->qt_stsd.idesc.spatialQuality = fix_make(0, 65536 / 50);
		ptrack->qt_stsd.idesc.width = pbm->w;
		ptrack->qt_stsd.idesc.height = pbm->h;
		ptrack->qt_stsd.idesc.hRes = fix_make(72,0);
		ptrack->qt_stsd.idesc.vRes = fix_make(72,0);
		ptrack->qt_stsd.idesc.frameCount = 1;
		strcpy((char *)ptrack->qt_stsd.idesc.name, (char *)"\pNone");
		if (pqtm->depth16)
		{
			ptrack->qt_stsd.idesc.depth = 16;
			ptrack->qt_stsd.idesc.clutId = -1;
		}
		else
		{
			ptrack->qt_stsd.idesc.depth = 8;
			if (ptrack->palette)
				ptrack->qt_stsd.idesc.clutId = 0;
			else
				ptrack->qt_stsd.idesc.clutId = 8;
		}

		ptrack->numSamplesAlloced = FRAME_ALLOC_INCR;
		ptrack->pSampleTime = (fix *)malloc(ptrack->numSamplesAlloced * sizeof(fix));
	}

//	Grow sample if need to

	ptrack->pSampleTime[ptrack->numSamps++] = time;
	if (ptrack->numSamps >= ptrack->numSamplesAlloced)
	{
		ptrack->numSamplesAlloced += FRAME_ALLOC_INCR;
		ptrack->pSampleTime = (fix *)realloc(ptrack->pSampleTime, 
									 ptrack->numSamplesAlloced * sizeof(fix));
	}
}

//	-------------------------------------------------------------
//
//	QuikWriteMovieAndClose() writes out movie and closes file

void QuikWriteMovieAndClose(QTM *pqtm, FILE *fp)
{
	ulong timeTot;

//	Close 'mdat' chunk

	QuikEndSubChunk(pqtm, fp);

//	Write 'moov' chunk, which has everything else

	QuikStartSubChunk(pqtm, fp, QT_MOOV, NULL, 0);
	timeTot = QuikComputeTotalTime(pqtm);
	QuikWriteMVHD(pqtm, fp, timeTot);
	QuikWriteVideoTrack(pqtm, fp, timeTot);

//	Unpop stacked chunk lengths

	while (pqtm->indexOffsetStack > 0)
		QuikEndSubChunk(pqtm, fp);

//	Close file and clean up

	fclose(fp);
	QuikFreeMovie(pqtm);
	memset(pqtm, 0, sizeof(QTM));
}

//	-------------------------------------------------------------
//		INTERNAL ROUTINES
//	-------------------------------------------------------------
//
//	QuikStartSubChunk() starts a new subchunk.

void QuikStartSubChunk(QTM *pqtm, FILE *fp, QT_Ctype ctype, void *data,
	long len)
{
	if (pqtm->indexOffsetStack >= QTM_MAX_CHUNK_NESTING)
	{
		printf("QuikStartSubChunk: nesting too deep\n");
		return;
	}

	pqtm->offsetStack[pqtm->indexOffsetStack++] = ftell(fp);

	QuikWriteChunk(fp, ctype, data, len);
}

//	-------------------------------------------------------------
//
//	QuikEndSubChunk() ends a subchunk.

void QuikEndSubChunk(QTM *pqtm, FILE *fp)
{
	long currPos,length;

	if (pqtm->indexOffsetStack == 0)
	{
		printf("QuikEndSubChunk: chunk nesting underflow\n");
		return;
	}

	pqtm->indexOffsetStack--;

	currPos = ftell(fp);
	fseek(fp, pqtm->offsetStack[pqtm->indexOffsetStack], SEEK_SET);
	length = currPos - ftell(fp);
	QuikWriteChunkLength(fp, length);
	fseek(fp, currPos, SEEK_SET);
}

//	-------------------------------------------------------------
//
//	QuikWriteMVHD() writes MVHD chunk

void QuikWriteMVHD(QTM *pqtm, FILE *fp, ulong timeTot)
{
	pqtm->qt_mvhd.createTime = DEFAULT_MACTIME;
	pqtm->qt_mvhd.modTime = DEFAULT_MACTIME;
	pqtm->qt_mvhd.timeScale = DEFAULT_TIMESCALE;
	if (pqtm->pVideoTrack)
		pqtm->qt_mvhd.duration = timeTot;
	pqtm->qt_mvhd.preferredRate = FIX_UNIT;
	pqtm->qt_mvhd.preferredVol = FIX8(FIX_UNIT);
	pqtm->qt_mvhd.matrix[0] = FIX_UNIT;
	pqtm->qt_mvhd.matrix[4] = FIX_UNIT;
	pqtm->qt_mvhd.matrix[8] = fix_make(16384,0);
	pqtm->qt_mvhd.nextTrackId = 1 + (pqtm->pVideoTrack != NULL) +
		(pqtm->pAudioTrack != NULL);

	QuikWriteChunk(fp, QT_MVHD, &pqtm->qt_mvhd, sizeof(pqtm->qt_mvhd));
}

//	-------------------------------------------------------------
//
//	QuikWriteVideoTrack() writes video track out.

void QuikWriteVideoTrack(QTM *pqtm, FILE *fp, ulong timeTot)
{
	MovieTrack *ptrack;
	uchar *pStsd;
	int i,ilast;
	ushort *ppw;
	uchar *ppr;
	ushort r,g,b;
	long len;
	ulong offset;
	long tlen,tlenLast;
	fix tlast;
	long sampSize;
	QTS_ELST elst;
	QTS_HDLR hdlr;
	QTS_VMHD vmhd;
	uchar dref[20];

//	If no video track, don't write it obviously

	ptrack = pqtm->pVideoTrack;
	if (ptrack == NULL)
		return;

//	Write TRAK chunk hdr and TKHD chunk

	QuikStartSubChunk(pqtm, fp, QT_TRAK, NULL, 0);
	ptrack->qt_tkhd.duration = timeTot;
	QuikWriteChunk(fp, QT_TKHD, &ptrack->qt_tkhd, sizeof(QTS_TKHD));

//	Write EDTS chunk hdr and ELST chunk

	QuikStartSubChunk(pqtm, fp, QT_EDTS, NULL, 0);
	memset(&elst, 0, sizeof(elst));
	elst.numEntries = 1;
	elst.editList[0].trackDuration = timeTot;
	elst.editList[0].mediaRate = FIX_UNIT;
	QuikStartSubChunk(pqtm, fp, QT_ELST, &elst, sizeof(elst));
	QuikEndSubChunk(pqtm, fp);
	QuikEndSubChunk(pqtm, fp);

//	Write MDIA chunk hdr and MDHD chunk

	QuikStartSubChunk(pqtm, fp, QT_MDIA, NULL, 0);
	ptrack->qt_mdhd.timeScale = MDHD_TIMESCALE;
	ptrack->qt_mdhd.duration = (timeTot * MDHD_TIMESCALE) / DEFAULT_TIMESCALE;
	QuikWriteChunk(fp, QT_MDHD, &ptrack->qt_mdhd, sizeof(QTS_MDHD));

//	Write HDLR chunk

	memset(&hdlr, 0, sizeof(hdlr));
	hdlr.compType = MAKE4('m','h','l','r');
	hdlr.compSubtype = MAKE4('v','i','d','e');
	hdlr.compManufacturer = MAKE4('a','p','p','l');
	hdlr.compFlags = 0x40000000;
	if (pqtm->depth16)
		hdlr.compFlagsMask = 0x20072;
	else
		hdlr.compFlagsMask = 0x10011;
	WriteChunkWithString(fp, QT_HDLR, &hdlr, sizeof(hdlr),
		"Apple Video Media Handler");

//	Write MINF chunk hdr and VMHD chunk

	QuikStartSubChunk(pqtm, fp, QT_MINF, NULL, 0);
	memset(&vmhd, 0, sizeof(vmhd));
	vmhd.flags[2] = 1;
	vmhd.graphicsMode = 0x40;
	vmhd.opColor[0] = 0x8000;
	vmhd.opColor[1] = 0x8000;
	vmhd.opColor[2] = 0x8000;
	QuikWriteChunk(fp, QT_VMHD, &vmhd, sizeof(vmhd));

//	Write HDLR chunk

	memset(&hdlr, 0, sizeof(hdlr));
	hdlr.compType = MAKE4('d','h','l','r');
	hdlr.compSubtype = MAKE4('a','l','i','s');
	hdlr.compManufacturer = MAKE4('a','p','p','l');
	if (pqtm->depth16)
		{
		hdlr.compFlags = 0xC0000000;
		hdlr.compFlagsMask = 0x1005D;
		}
	else
		{
		hdlr.compFlags = 0x40000000;
		hdlr.compFlagsMask = 0x10016;
		}
	WriteChunkWithString(fp, QT_HDLR, &hdlr, sizeof(hdlr),
		"Apple Alias Data Handler");

//	Write DINF chunk hdr and DREF chunk

	QuikStartSubChunk(pqtm, fp, QT_DINF, NULL, 0);
	memset(dref, 0, sizeof(dref));
	dref[7] = 1;
	dref[11] = 0x0C;
	dref[12] = 'a';
	dref[13] = 'l';
	dref[14] = 'i';
	dref[15] = 's';
	dref[19] = 1;
	QuikWriteChunk(fp, QT_DREF, dref, sizeof(dref));
	QuikEndSubChunk(pqtm, fp);

//	Write STBL chunk hdr and STSD chunk

	QuikStartSubChunk(pqtm, fp, QT_STBL, NULL, 0);
	pStsd = (uchar *)malloc(sizeof(ptrack->qt_stsd) + 0x0800 + 8);
	memcpy(pStsd, &ptrack->qt_stsd, sizeof(ptrack->qt_stsd));
	if (ptrack->palette && !pqtm->depth16)
		{
		ppw = (ushort *) (pStsd + sizeof(QTS_STSD_Base) +
			sizeof(QT_ImageDesc) + 8);
		*(ppw - 1) = 0xFF00;
		ppr = ptrack->palette;
		for (i = 0; i < 256; i++)
			{
			*ppw++ = i << 8;
			r = *ppr++;
			*ppw++ = (r << 8) | r;
			g = *ppr++;
			*ppw++ = (g << 8) | g;
			b = *ppr++;
			*ppw++ = (b << 8) | b;
			}
		}
	QuikWriteChunk(fp, QT_STSD, pStsd, sizeof(ptrack->qt_stsd) + 0x0800 + 8);
	free(pStsd);

//	Write STTS chunk

	ptrack->qt_stts = (QTS_STTS *)malloc(sizeof(QTS_STTS) + 
								  (sizeof(QT_Time2Samp) * ptrack->numSamps));
	tlenLast = 0;
	tlast = 0;
	ilast = 1;
	for (i = 1; i < ptrack->numSamps; i++)
		{
		tlen = QuikSampleLenMsec(ptrack->pSampleTime[i], tlast);
		if (tlen != tlenLast)
			{
			if (i != 1)
				{
				ptrack->qt_stts->time2samp[ptrack->qt_stts->numEntries].count =
					i - ilast;
				ptrack->qt_stts->time2samp[ptrack->qt_stts->numEntries].duration =
					tlenLast;
				ptrack->qt_stts->numEntries++;
				ilast = i;
				}
			tlenLast = tlen;
			}
		tlast += QuikSampleLenMsecBackToFix(tlen);
		}
	ptrack->qt_stts->time2samp[ptrack->qt_stts->numEntries].count =
		(i - ilast) + 1;
	ptrack->qt_stts->time2samp[ptrack->qt_stts->numEntries].duration =
		tlen;
	ptrack->qt_stts->numEntries++;
	QuikWriteChunk(fp, QT_STTS, ptrack->qt_stts, sizeof(QTS_STTS) +
		(sizeof(QT_Time2Samp) * (ptrack->qt_stts->numEntries - 1)));

//	Write STSC chunk

	len = sizeof(QTS_STSC);
	ptrack->qt_stsc = (QTS_STSC *)malloc(len);
	ptrack->qt_stsc->numEntries = 1;
	ptrack->qt_stsc->samp2chunk[0].firstChunk = 1;
	ptrack->qt_stsc->samp2chunk[0].sampsPerChunk = 1;
	ptrack->qt_stsc->samp2chunk[0].sampDescId = 1;
	QuikWriteChunk(fp, QT_STSC, ptrack->qt_stsc, len);

//	Write STSZ chunk

	len = sizeof(QTS_STSZ) - sizeof(ulong);	// don't include sampSizeTab[]
	ptrack->qt_stsz = (QTS_STSZ *)malloc(len);
	sampSize = (long) pqtm->frameWidth * (long) pqtm->frameHeight;
	if (pqtm->depth16)
		sampSize *= 2;
	ptrack->qt_stsz->sampSize = sampSize;
	ptrack->qt_stsz->numEntries = ptrack->numSamps;
	QuikWriteChunk(fp, QT_STSZ, ptrack->qt_stsz, len);

//	Write STCO chunk

	len = sizeof(QTS_STCO) + ((ptrack->numSamps - 1) * sizeof(ulong));
	ptrack->qt_stco = (QTS_STCO *)malloc(len);
	ptrack->qt_stco->numEntries = ptrack->numSamps;
	offset = sizeof(QT_ChunkHdr);
	for (i = 0; i < ptrack->numSamps; i++)
		{
		ptrack->qt_stco->offset[i] = offset;
		offset += sampSize;
		}
	QuikWriteChunk(fp, QT_STCO, ptrack->qt_stco, len);
}

//	-------------------------------------------------------------
//
//	SetPascalString() puts C string into PStr field.

static void SetPascalString(PStr *pstr, char *str)
{
	pstr->len = strlen(str);
	memcpy(pstr->str, str, pstr->len);
}

//	-------------------------------------------------------------
//
//	WriteChunkWithString() writes a var-length data chunk with trailing PStr.

static void WriteChunkWithString(FILE *fp, QT_Ctype ctype, void *data,
	long len, char *str)
{
	char buff[128];

	memcpy(buff, data, len - sizeof(PStr));
	SetPascalString((PStr *) (buff + len - sizeof(PStr)), str);
	QuikWriteChunk(fp, ctype, buff, (len - sizeof(PStr)) + strlen(str) + 1);
}

//	---------------------------------------------------------------
//
//	QuikComputeTotalTime() computes total time of movie.

ulong QuikComputeTotalTime(QTM *pqtm)
{
	MovieTrack *ptrack;
	fix fixtime;

	ptrack = pqtm->pVideoTrack;
	if ((ptrack == NULL) || (ptrack->pSampleTime == NULL))
		return(0);

	fixtime = ptrack->pSampleTime[ptrack->numSamps - 1] +
		(ptrack->pSampleTime[ptrack->numSamps - 1] -
			ptrack->pSampleTime[ptrack->numSamps - 2]);
	return(fix_mul(fixtime, DEFAULT_TIMESCALE));
}

//	---------------------------------------------------------------
//
//	QuikSampleLenMsec() returns sample length in msecs.

long QuikSampleLenMsec(fix t, fix tlast)
{
	fix fixtime;
	long msec;

	fixtime = t - tlast;
#if MDHD_TIMESCALE == 1000
	msec = ((fixtime * 125) + 0x1000) >> 13;	// overflows at about 8 minutes
#elif MDHD_TIMESCALE == 100
	msec = ((fixtime * 25) + 0x2000) >> 14;
#elif MDHD_TIMESCALE == 30
	msec = ((fixtime * 15) + 0x4000) >> 15;
#else
	msec = 0;
	Warning(("QuikSampleLenMsec: invalid MDHD_TIMESCALE\n"));
#endif
	return(msec);
}

//	--------------------------------------------------------------
//
//	QuikSampleLenMsecBackToFix() converts msec to fix

fix QuikSampleLenMsecBackToFix(long msec)
{
	fix tfix;

#if MDHD_TIMESCALE == 1000
	tfix = (msec << 13) / 125;
#elif MDHD_TIMESCALE == 100
	tfix = (msec << 14) / 25;
#elif MDHD_TIMESCALE == 30
	tfix = (msec << 15) / 15;
#else
	tfix = 0;
	Warning(("QuikSampleLenMsecBackToFix: invalid MDHD_TIMESCALE\n"));
#endif
	return(tfix);
}
