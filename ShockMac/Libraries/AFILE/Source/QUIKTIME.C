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
//		QUIKTIME.C		QuickTime file reading
//		Rex E. Bradford

/*
 * $Source: r:/prj/lib/src/afile/RCS/quiktime.c $
 * $Revision: 1.6 $
 * $Author: rex $
 * $Date: 1994/11/21 12:09:36 $
 * $Log: quiktime.c $
 * Revision 1.6  1994/11/21  12:09:36  rex
 * If unknown sample rate, use 4K audio blocks
 * 
 * Revision 1.5  1994/10/31  17:36:44  rex
 * Made it work for 24-bit Quicktime input
 * 
 * Revision 1.4  1994/10/03  18:04:12  rex
 * Fixed bug by getting rid of rectangle before bitmap when reading
 * 
 * Revision 1.3  1994/09/30  16:59:54  rex
 * Fixed bug in Flip2Val() routine
 * 
 * Revision 1.2  1994/09/29  10:34:28  rex
 * Revamped like made to put globals into a single struct
 * 
 * Revision 1.1  1994/09/27  17:23:06  rex
 * Initial revision
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lg.h"
#include "rect.h"
//#include <fname.h>
#include "quiktime.h"
#include "2d.h"
//#include <voc.h>

#define QT_RAW MAKE4('r','a','w',' ')
#define QT_RLE MAKE4('r','l','e',' ')

extern uchar std8Palette[];	// Mac ROM standard 8-bit color palette
										// (it's in mac8pal.c)

//	Internal prototypes

void ReadVarChunk(FILE *fpi, QT_ChunkHdr *phdr, void **p);
ushort Flip2Val(ushort v);
fix ComputeTotalTime();
void AllocateVideoBuffers(int width, int height);
uchar *GetVideoFrame(QTM *pqtm, FILE *fpi, long size, ushort *pbmtype,
	long *plength, LGRect *pUpdateArea);

/*
//	-------------------------------------------------------------
//		MOVIE READING
//	-------------------------------------------------------------
//
//	QuikReadMovie() reads in a QuickTime movie, building necessary data
//	structures.

void QuikReadMovie(QTM *pqtm, FILE *fpi)
{
static QTS_STSD *pStsd;
	QT_ChunkHdr chunkHdr;
	QT_ChunkInfo *pinfo;
	MovieTrackStatus tkStat;
	int itrack;

//	Clear QTM structure

	memset(pqtm, 0, sizeof(QTM));

//	We're waiting for our first TRAK chunk

	itrack = -1;

//	Read chunks till we're exhausted

	while (TRUE)
		{

//	Read chunk header, if length is 0 then bail, else look up chunk type
//	and switch on it.

		if (!QuikReadChunkHdr(fpi, &chunkHdr))
			break;
		if (chunkHdr.length == 0)
			break;
		pinfo = QuikFindChunkInfo(&chunkHdr);

		switch (chunkHdr.ctype)
			{

//	MVHD: read in single movie header chunk

			case QT_MVHD:
				QuikReadChunk(fpi, &chunkHdr, &pqtm->qt_mvhd,
					sizeof(pqtm->qt_mvhd));
				break;

//	TRAK: if not 1st track, make sure last track got all necessary
//	subchunks.  Then bump track number and reset these flags.  Set
//	track type to TRACK_OTHER (may be overridden by subsequent chunks).

			case QT_TRAK:
				if ((itrack >= 0) &&
					(!tkStat.gotTKHD || !tkStat.gotMDHD || !tkStat.gotSTBL))
						{
						Warning(("QuikReadMovie: Track with missing TKHD or MDHD or STBL!\n"));
						return;
						}
				if (++itrack >= QTM_MAX_TRACKS)
					{
					Warning(("QuikReadMovie: Too many tracks in movie!\n"));
					return;
					}
				tkStat.gotTKHD = FALSE;
				tkStat.gotMDHD = FALSE;
				tkStat.gotSTBL = FALSE;
				pqtm->track[itrack].type = TRACK_OTHER;
				break;

//	TKHD: If already got one this track, skip over it.  Else read in the
//	chunk and marked as gotten.

			case QT_TKHD:
				if (tkStat.gotTKHD)
					{
					Warning(("QuikReadMovie: Extra TKHD in TRAK!\n"));
					QuikSkipChunk(fpi, &chunkHdr);
					}
				else
					{
					QuikReadChunk(fpi, &chunkHdr, &pqtm->track[itrack].qt_tkhd,
						sizeof(QTS_TKHD));
					tkStat.gotTKHD = TRUE;
					}
				break;

//	MDHD: Do similar to TKHD.

			case QT_MDHD:
				if (tkStat.gotMDHD)
					{
					Warning(("QuikReadMovie: Extra MDHD in TRAK!\n"));
					QuikSkipChunk(fpi, &chunkHdr);
					}
				else
					{
					QuikReadChunk(fpi, &chunkHdr, &pqtm->track[itrack].qt_mdhd,
						sizeof(QTS_MDHD));
					tkStat.gotMDHD = TRUE;
					}
				break;

//	VMHD: Mark current track as a VIDEO track.

			case QT_VMHD:
				pqtm->track[itrack].type = TRACK_VIDEO;
				QuikSkipChunk(fpi, &chunkHdr);
				break;

//	SMHD: Mark current track as an AUDIO track.

			case QT_SMHD:
				pqtm->track[itrack].type = TRACK_AUDIO;
				QuikSkipChunk(fpi, &chunkHdr);
				break;

//	STBL: Mark as gotten.

			case QT_STBL:
				if (tkStat.gotSTBL)
					Warning(("QuikReadMovie: Extra STBL in TRAK!\n"));
				else
					tkStat.gotSTBL = TRUE;
				break;

//	STSD: Allocate what is hopefully enough room for this chunk.
//	If VIDEO chunk, look for 'std' palette (clutId == 8) or custom
//	palette (clutId == 0).
//	If AUDIO chunk or OTHER chunk, just grab descriptor.

			case QT_STSD:
				if (pStsd == NULL)
					pStsd = Malloc(3000);
				QuikReadChunk(fpi, &chunkHdr, pStsd, 3000);
				switch (pqtm->track[itrack].type)
					{
					case TRACK_VIDEO:
						memcpy(&pqtm->track[itrack].qt_stsd, pStsd,
							sizeof(QTS_STSD_Base) + sizeof(QT_ImageDesc));
						if (pqtm->track[itrack].qt_stsd.idesc.depth == 24)
							{
							}
						else if (pqtm->track[itrack].qt_stsd.idesc.depth == 8)
							{
							if (pqtm->track[itrack].qt_stsd.idesc.clutId == 8)
								{
								pqtm->track[itrack].palette = Malloc(768);
								memcpy(pqtm->track[itrack].palette, std8Palette, 768);
								}
							else if (pqtm->track[itrack].qt_stsd.idesc.clutId == 0)
								{
								ushort *psrc;
								int i,index;
								uchar *ppall;
	
								pqtm->track[itrack].palette = Malloc(768);
								psrc = (ushort *) (((uchar *) pStsd) +
									sizeof(QTS_STSD_Base) + sizeof(QT_ImageDesc) + 8);
								for (i = 0; i < 256; i++)
									{
									index = Flip2Val(*psrc++);
									ppall = pqtm->track[itrack].palette + (index * 3);
									*ppall++ = Flip2Val(*psrc++) >> 8;
									*ppall++ = Flip2Val(*psrc++) >> 8;
									*ppall++ = Flip2Val(*psrc++) >> 8;
									}
								}
							}
						else
							Warning(("QuikReadMovie: Video track not 8-bit or 24-bit!\n"));
						break;

					case TRACK_AUDIO:
						memcpy(&pqtm->track[itrack].qt_stsd, pStsd,
							sizeof(QTS_STSD_Base) + sizeof(QT_SoundDesc));
						break;

					case TRACK_OTHER:
						memcpy(&pqtm->track[itrack].qt_stsd, pStsd,
							sizeof(QTS_STSD_Base) + sizeof(QT_TextDesc));
						break;
					}
				if (pqtm->track[itrack].qt_stsd.base.numEntries > 1)
					Warning(("QuikReadMovie: STSD chunk with more than 1 entry!\n"));
				break;

//	STTS: Grab time->sample table chunk.

			case QT_STTS:
				ReadVarChunk(fpi, &chunkHdr, &pqtm->track[itrack].qt_stts);
				break;

//	STSC: Grab sample->chunk table chunk.

			case QT_STSC:
				ReadVarChunk(fpi, &chunkHdr, &pqtm->track[itrack].qt_stsc);
				break;

//	STSZ: Grab sample size table chunk.

			case QT_STSZ:
				ReadVarChunk(fpi, &chunkHdr, &pqtm->track[itrack].qt_stsz);
				break;

//	STCO: Grab chunk->offset table chunk.

			case QT_STCO:
				ReadVarChunk(fpi, &chunkHdr, &pqtm->track[itrack].qt_stco);
				break;

//	All other chunks:  if leaf chunk, skip, else do sub-chunks

			default:
				if (pinfo->isleaf)
					QuikSkipChunk(fpi, &chunkHdr);
				break;
			}
		}

//	Before return, set total number of tracks, also test for right
//	number of each type.

	pqtm->numTracks = itrack + 1;

	for (itrack = 0; itrack < pqtm->numTracks; itrack++)
		{
		if (pqtm->track[itrack].type == TRACK_VIDEO)
			{
			if (pqtm->pVideoTrack)
				Warning(("QuikReadMovie: More than 1 VIDEO track in movie!\n"));
			pqtm->pVideoTrack = &pqtm->track[itrack];
			}
		else if (pqtm->track[itrack].type == TRACK_AUDIO)
			{
			if (pqtm->pAudioTrack)
				Warning(("QuikReadMovie: More than 1 AUDIO track in movie!\n"));
			pqtm->pAudioTrack = &pqtm->track[itrack];
			if (pqtm->pAudioTrack->qt_stsd.sdesc.sampRate == SAMPRATE_11KHZ)
				pqtm->numAudioSamplesPerBlock = 4096;
			else if (pqtm->pAudioTrack->qt_stsd.sdesc.sampRate == SAMPRATE_22KHZ)
				pqtm->numAudioSamplesPerBlock = 8192;
			else
            {
				Warning(("QuikReadMovie: Unknown audio sampling rate! (using 4K blocks)\n"));
            pqtm->numAudioSamplesPerBlock = 4096;
            }
			}
		}

	if ((pqtm->pVideoTrack == NULL) && (pqtm->pAudioTrack == NULL))
		{
		Warning(("QuikReadMovie: No VIDEO or AUDIO tracks in movie!\n"));
		return;
		}

//	Process movie

	QuikProcessMovie(pqtm, fpi);
}
*/

//	------------------------------------------------------------
//		MOVIE FREEING
//	------------------------------------------------------------
//
//	QuikFreeMovie() frees up structs malloced in qtm struct

void QuikFreeMovie(QTM *pqtm)
{
	int itrack;
	MovieTrack *ptrack;

//	Free up stuff alloced inside each track

	for (itrack = 0, ptrack = pqtm->track; itrack < pqtm->numTracks;
		itrack++, ptrack++)
		{
		if (ptrack->palette)
			free(ptrack->palette);

		if (ptrack->qt_stts)
			free(ptrack->qt_stts);
		if (ptrack->qt_stsc)
			free(ptrack->qt_stsc);
		if (ptrack->qt_stsz)
			free(ptrack->qt_stsz);
		if (ptrack->qt_stco)
			free(ptrack->qt_stco);

		if (ptrack->sampBuff)
			free(ptrack->sampBuff);
		if (ptrack->sampTime)
			free(ptrack->sampTime);
		if (ptrack->sampSize)
			free(ptrack->sampSize);
		if (ptrack->sampOffset)
			free(ptrack->sampOffset);
		}

//	Free up video buffers

	if (pqtm->pFrameCurr)
		{
		free(pqtm->pFrameCurr);
		free(pqtm->pFrameCompQT);
		}

//	Clear qtm struct so can't Free() twice

	memset(pqtm, 0, sizeof(QTM));
}

//	------------------------------------------------------------
//		OTHER PUBLIC FUNCTIONS
//	------------------------------------------------------------
//
//	QuikComputeFrameRate() computes the video frame rate.  Currently,
//	it just computes this from the duration of the first frame.
//	There may be a better way, for instance reading from the MDHD
//	media header for the video track (?).

fix QuikComputeFrameRate(QTM *pqtm)
{
	fix t;

	if (pqtm->pVideoTrack && (pqtm->pVideoTrack->numSamps > 1))
		{
		t = pqtm->pVideoTrack->sampTime[1] - pqtm->pVideoTrack->sampTime[0];
		if (t)
			return(fix_div(FIX_UNIT,t));
		}
	return 0;
}
/*
//	-----------------------------------------------------------------
//
//	QuikGetVideoSample() gets the next video sample.  If the sample is
//	compressed, the sample is decompressed.  If the compress flag is set,
//	the sample is then recompressed, currently using RSD8 encoding.

void *QuikGetVideoSample(QTM *pqtm, int isample, FILE *fpi, long *plength,
	uchar *pbmtype, fix *ptime)
{
static uchar *buff = NULL;
static ulong buffLen = 0;

	long length;
	ushort bmtype;
	uchar *p;
	Rect area;

//	VIDEO sample: seek to proper spot in file, call getVideoFrame() to
//	decompress and optionally recompress the frame.  Set the chunk subType
//	to the returned bitmap type.  Length is rounded up to a integral number
//	of longwords.  Grow the return buffer if needed, and copy the frame into
//	it.  Set the update area rectangle too.

	if (pqtm->pVideoTrack == NULL)
		{
		*plength = 0;
		return(NULL);
		}

	fseek(fpi, pqtm->pVideoTrack->sampOffset[isample], SEEK_SET);
	p = GetVideoFrame(pqtm, fpi, pqtm->pVideoTrack->sampSize[isample],
		&bmtype, &length, &area);
	if (pbmtype)
		*pbmtype = bmtype;
	if (ptime)
		*ptime = pqtm->pVideoTrack->sampTime[isample];

	*plength = length;
	*plength = (*plength + 3) & 0xFFFFFFFCL;	// round to 4
	if (*plength > buffLen)
		{
		buffLen = *plength;
		if (buff)
			buff = Realloc(buff, buffLen);
		else
			buff = Malloc(buffLen);
		}

	memcpy(buff, p, length);

	return(buff);
}

//	-----------------------------------------------------------------
//
//	QuikGetAudioSample() gets the next audio sample.

void *QuikGetAudioSample(QTM *pqtm, int isample, long *plength, fix *ptime)
{
//	AUDIO sample: set chunk length, return ptr into audio sample buffer
//	for this track.

	if (pqtm->pAudioTrack == NULL)
		{
		*plength = 0;
		return(NULL);
		}

	if (ptime)
		*ptime = pqtm->pAudioTrack->sampTime[isample];
	*plength = pqtm->pAudioTrack->sampSize[isample];
	return(pqtm->pAudioTrack->sampBuff +
		pqtm->pAudioTrack->sampOffset[isample]);
}

//	-------------------------------------------------------------
//		INTERNAL ROUTINES
//	-------------------------------------------------------------
//		MOVIE PROCESSING
//	-------------------------------------------------------------
//
//	QuikProcessMovie() movie scans thru the tracks, doing special processing
//	for video and audio tracks.  For each of these, we want to parse
//	the mildly complicated QuickTime sample tables, and just come up
//	with 3 arrays for each track:
//
//		sampTime[]   - time of each sample
//		sampSize[]   - size of each sample (source sample, may change
//		               size when output)
//		sampOffset[] - offset in QuickTime file of each sample

static void QuikProcessMovie(QTM *pqtm, FILE *fpi)
{
	int itrack;
	MovieTrack *ptrack;

//	The looping statement is an important part of any computer language

	for (itrack = 0, ptrack = pqtm->track; itrack < pqtm->numTracks;
		itrack++, ptrack++)
		{

//	If VIDEO track, allocate sample tables, then compute sample times,
//	sizes, and offsets.

		if (ptrack->type == TRACK_VIDEO)
			{
			ptrack->numSamps = ptrack->qt_stsz->numEntries;
			ptrack->sampBuff = NULL;
			ptrack->sampTime = Malloc(sizeof(fix) * ptrack->numSamps);
			ptrack->sampSize = Malloc(sizeof(ulong) * ptrack->numSamps);
			ptrack->sampOffset = Malloc(sizeof(ulong) * ptrack->numSamps);
			ComputeSampleTimesVideo(ptrack);
			ComputeSampleSizesVideo(ptrack);
			ComputeSampleOffsetsVideo(ptrack);

			pqtm->compTypeQT = pqtm->pVideoTrack->qt_stsd.idesc.dataFormat;
			pqtm->frameWidth = pqtm->pVideoTrack->qt_stsd.idesc.width;
			pqtm->frameHeight = pqtm->pVideoTrack->qt_stsd.idesc.height;
			}

//	If AUDIO track, read entire audio track into big buffer, allocate
//	sample tables, and compute sample times, sizes, and offsets.

		else if (ptrack->type == TRACK_AUDIO)
			{
			ptrack->numSamps =
				(ptrack->qt_stsz->numEntries +
					pqtm->numAudioSamplesPerBlock - 1) /
					pqtm->numAudioSamplesPerBlock;
			// SOMEDAY THIS SHOULD TAKE INTO ACCOUNT 8-BIT VS. 16-BIT SAMPLES
			ptrack->audioBlockSize = ptrack->qt_stsd.sdesc.numChans *
				pqtm->numAudioSamplesPerBlock;
			ptrack->sampBuff = Calloc(ptrack->numSamps * ptrack->audioBlockSize);
			ReadAudioSamples(ptrack, fpi);
			ptrack->sampTime = Malloc(sizeof(fix) * ptrack->numSamps);
			ptrack->sampSize = Malloc(sizeof(ulong) * ptrack->numSamps);
			ptrack->sampOffset = Malloc(sizeof(ulong) * ptrack->numSamps);
			ComputeSampleTimesAudio(pqtm, ptrack);
			ComputeSampleSizesAudio(ptrack);
			ComputeSampleOffsetsAudio(ptrack);
			}

//	If OTHER track, we're just gonna toss it

		else
			{
			}
		}
}

//	---------------------------------------------------------------
//
//	ComputeSampleTimesVideo() computes the sample time for each video frame.

static void ComputeSampleTimesVideo(MovieTrack *ptrack)
{
	fix timeScale,currTime;
	int isample,i,j;

//	Read time scale from MDHD media header chunk
//	Assume track starts at beginning of movie (???)

	timeScale = fix_make(ptrack->qt_mdhd.timeScale,0);
	currTime = 0;

//	Go thru table entries in time->sample STTS chunk.  This chunk gives
//	the number of frames with a given duration, and said duration.
//	Then assign frame sample times based on that duration for the given
//	number of samples, before moving on to next STTS table entry.

	for (i = 0, isample = 0; i < ptrack->qt_stts->numEntries; i++)
		{
		QT_Time2Samp *pt2s = &ptrack->qt_stts->time2samp[i];
		for (j = 0; j < pt2s->count; j++)
			{
			ptrack->sampTime[isample++] = currTime;
			currTime += fix_div(fix_make(pt2s->duration, 0), timeScale);
			}
		}

//	If we haven't processed the number of samples (frames) we're expecting
//	(read from the STSZ chunk), something went wrong.

	if (isample != ptrack->numSamps)
		Warning(("Computed # video samples: %d, actual: %d\n",
			isample, ptrack->numSamps));
}

//	--------------------------------------------------------------
//
//	ComputeSampleSizesVideo() computes the source sample size for each
//	video frame.  The STSZ chunk is used to do this.

static void ComputeSampleSizesVideo(MovieTrack *ptrack)
{
	int i;

//	If the sampSize field is set, all samples are the same size.
//	Just set them all to this field.

	if (ptrack->qt_stsz->sampSize)
		{
		for (i = 0; i < ptrack->numSamps; i++)
			ptrack->sampSize[i] = ptrack->qt_stsz->sampSize;
		}

//	Else there is a table of sample sizes.  Copy them into our table.

	else
		{
		for (i = 0; i < ptrack->numSamps; i++)
			ptrack->sampSize[i] = ptrack->qt_stsz->sampSizeTab[i];
		}
}

//	------------------------------------------------------------
//
//	ComputeSampleOffsetsVideo() computes the table of file offsets
//	to each source sample (frame).  The STSC and STCO chunks are needed
//	to compute this, along with the sample size table previously computed.
//	The STCO table gives the offset of each chunk in the file.  The
//	STSC gives the sample->chunk table, which determines number of samples
//	in a given chunk.  The sample size table previously computed is used
//	to determine the offset within a chunk.

static void ComputeSampleOffsetsVideo(MovieTrack *ptrack)
{
	int ichunk,isample,scTableIndex,numSampsThisChunk;
	ulong chunkOffset,sampleOffset;
	QT_Samp2Chunk *psamp2chunk;

//	Initialize:
//		Start with an index of 0 in the chunk->offset table (STCO).
//		Get the ptr to first sample->chunk table entry (STSC).
//		Get the offset of the first chunk.
//		Set the sample offset within this first chunk to 0.
//		Get the number of samples in this chunk from the STSC table.

	ichunk = 0;
	scTableIndex = 0;
	psamp2chunk = &ptrack->qt_stsc->samp2chunk[0];
	chunkOffset = ptrack->qt_stco->offset[0];
	sampleOffset = 0;
	numSampsThisChunk = psamp2chunk->sampsPerChunk;

//	Loop thru all samples

	for (isample = 0; isample < ptrack->numSamps; isample++)
		{

//	Set current sample's offset equal to chunk's offset + sample's offset
//	within this chunk.

		ptrack->sampOffset[isample] = chunkOffset + sampleOffset;

//	Advance sample offset within this chunk by the sample size.

		sampleOffset += ptrack->sampSize[isample];

//	Count down number of samples in this chunk.

		if (--numSampsThisChunk <= 0)
			{

//	If done with samples in this chunk, bump chunk number and get its
//	offset, and set sample offset within chunk to 0.

			++ichunk;
			chunkOffset = ptrack->qt_stco->offset[ichunk];
			sampleOffset = 0;

//	Then test to see if can advance in sample->chunk table, if so do it

			if (((scTableIndex + 1) < ptrack->qt_stsc->numEntries) &&
				((ichunk + 1) >= (psamp2chunk+1)->firstChunk))
					{
					++scTableIndex;
					++psamp2chunk;
					}

//	Finally, get the number of samples in this chunk out of the
//	current table entry, whether advanced or not.

			numSampsThisChunk = psamp2chunk->sampsPerChunk;
			}
		}
}

//	---------------------------------------------------------------
//
//	ReadAudioSamples() reads all the audio samples for a track into
//	one big buffer.  This implementation assumes that any audio track
//	lasts for the full duration of the movie.

static void ReadAudioSamples(MovieTrack *ptrack, FILE *fpi)
{
	int ichunk;
	long index,isample,sampleSize,numBytes;

//	All movies encountered so far has audio STTS tables with one
//	time->sample table entry.  If this does not hold true for further
//	movies, this routine will have to be modified.

	if (ptrack->qt_stts->numEntries > 1)
		{
		Warning(("Can't handle audio track with > 1 time2samp table!\n"));
		return;
		}

//	Go thru the sample->chunk entries, for each:
//		1. Seek to the chunk
//		2. Read the sample data into the current indexed loc in the buffer
//		3. Advance the current index

	index = 0;
	isample = 0;
	sampleSize = ptrack->qt_stsc->samp2chunk[0].sampsPerChunk;
	for (ichunk = 0; ichunk < ptrack->qt_stco->numEntries; ichunk++)
		{
		fseek(fpi, ptrack->qt_stco->offset[ichunk], SEEK_SET);
		if ((isample < (ptrack->qt_stsc->numEntries - 1)) &&
			((ichunk + 1) >= ptrack->qt_stsc->samp2chunk[isample + 1].firstChunk))
				{
				sampleSize = ptrack->qt_stsc->samp2chunk[++isample].sampsPerChunk;
				}
		numBytes = sampleSize * ptrack->qt_stsd.sdesc.numChans;
		fread(ptrack->sampBuff + index, numBytes, 1, fpi);
		index += numBytes;
		}
}

//	---------------------------------------------------------------
//
//	ComputeSampleTimesAudio() computes the sample time of each audio
//	chunk.  Since each audio chunk is fixed size, this is easy.  If
//	audio tracks are not constrained to play for the full length of the
//	movie, this will get considerably more complex.

static void ComputeSampleTimesAudio(QTM *pqtm, MovieTrack *ptrack)
{
	fix currTime;
	int i;

//	Set each chunk time, then advance time by the amount of time it takes
//	to play the fixed-size chunk at the sampling rate.

	currTime = 0;
	for (i = 0; i < ptrack->numSamps; i++)
		{
		ptrack->sampTime[i] = currTime;
		currTime += fix_div(fix_make(pqtm->numAudioSamplesPerBlock,0),
			ptrack->qt_stsd.sdesc.sampRate);
		}
}

//	---------------------------------------------------------------
//
//	ComputeSampleSizesAudio() computes the sample size of each audio
//	chunk.  They are all constant size currently.

static void ComputeSampleSizesAudio(MovieTrack *ptrack)
{
	int i;
	long totBytes;

	totBytes = ptrack->qt_stsz->numEntries * ptrack->qt_stsd.sdesc.numChans;

	for (i = 0; i < ptrack->numSamps; i++)
		{
		ptrack->sampSize[i] = min(ptrack->audioBlockSize, totBytes);
		totBytes -= ptrack->sampSize[i];
		}
}

//	---------------------------------------------------------------
//
//	ComputeSampleOffsetsAudio() computes the byte offset of each
//	sample chunk.  These offsets are into the allocated audio buffer
//	for this track.  Again, for now they are all fixed size, so it's easy.

static void ComputeSampleOffsetsAudio(MovieTrack *ptrack)
{
	int i;
	ulong offset;

	for (i = 0, offset = 0; i < ptrack->numSamps; i++)
		{
		ptrack->sampOffset[i] = offset;
		offset += ptrack->audioBlockSize;
		}
}

//	-----------------------------------------------------------------
//		READING, DECOMPRESSING, AND COMPRESSING SAMPLES
//	---------------------------------------------------------------
//
//	GetVideoFrame() gets the current video frame.  It decompresses it
//	if necessary.

static uchar *GetVideoFrame(QTM *pqtm, FILE *fpi, long size, ushort *pbmtype,
	long *plength, Rect *pUpdateArea)
{
   long u_frame_size;
   int numBytesPix;

//	Figure # bytes per pixel

	if (pqtm->pVideoTrack->qt_stsd.idesc.depth == 24)
		numBytesPix = 3;
	else
		numBytesPix = 1;
   u_frame_size = (long) pqtm->frameWidth * (long) pqtm->frameHeight *
      numBytesPix;

//	Allocate buffers

	if (pqtm->pFrameCurr == NULL)
		{
		pqtm->pFrameCompQT = Malloc(u_frame_size * 2);	// enough so always fits???
		pqtm->pFrameCurr = Malloc(u_frame_size);
		}

//	Read and decompress video frame

	fread(pqtm->pFrameCompQT, size, 1, fpi);
	switch (pqtm->compTypeQT)
		{
		case QT_RAW:
			DecompressRaw(pqtm, pqtm->pFrameCurr, pqtm->pFrameCompQT, size,
				numBytesPix);
			break;

		case QT_RLE:
			DecompressRle(pqtm, pqtm->pFrameCurr, pqtm->pFrameCompQT, size);
			break;

		default:
			Warning(("Unknown video compression type: %c%c%c%c\n",
				pqtm->compTypeQT >> 24, (pqtm->compTypeQT >> 16) & 0xFF,
				(pqtm->compTypeQT >> 8) & 0xFF, pqtm->compTypeQT & 0xFF));
		}

//	FIX THIS TEMPORARY STUFFING

	pUpdateArea->ul.x = 0;
	pUpdateArea->ul.y = 0;
	pUpdateArea->lr.x = pqtm->frameWidth;
	pUpdateArea->lr.y = pqtm->frameHeight;

//	Set bitmap type and return it

	if (pqtm->pVideoTrack->qt_stsd.idesc.depth == 24)
		*pbmtype = BMT_FLAT24;
   else
   	*pbmtype = BMT_FLAT8;

	*plength = u_frame_size;
	return(pqtm->pFrameCurr);
}
                   
//	------------------------------------------------------------------
//		QUIKTIME DECOMPRESSORS
//	------------------------------------------------------------------
//
//	DecompressRaw() "decompresses" raw uncompressed frames.  This can
//	be more than a simple copy, since source data appears to always be
//	in widths of multiples of 4, and our destination does not have
//	such a restriction.

#pragma off(unreferenced);

static void DecompressRaw(QTM *pqtm, uchar *pd, uchar *ps, long csize,
	int numBytesPix)
{
	int y,widthSrc;

	if ((pqtm->frameWidth & 3) == 0)
		{
		memcpy(pd, ps, csize);
      if (numBytesPix == 3)
         SwapRgb(pd, (int) (csize / 3));
		}
	else
		{
		widthSrc = ((pqtm->frameWidth + 3) & 0xFFFC) * numBytesPix;
		if ((widthSrc * pqtm->frameHeight) != csize)
			{
			Warning(("Raw video data frame does not seem to be proper size!\n"));
			return;
			}
		for (y = 0; y < pqtm->frameHeight; y++)
			{
			memcpy(pd, ps, pqtm->frameWidth * numBytesPix);
         if (numBytesPix == 3)
            SwapRgb(pd, (int) pqtm->frameWidth);
			pd += pqtm->frameWidth * numBytesPix;
			ps += widthSrc;
			}
		}
}

void SwapRgb(uchar *p, int n)
{
   while (n-- > 0)
      {
      uchar r = *p;
      uchar g = *(p+1);
      uchar b = *(p+2);

      *p = g;
      *(p+1) = b;
      *(p+2) = r;

      p += 3;
      }
}

//	------------------------------------------------------------------
//
//	DecompressRle() decompresses frames encoded by the ANIM codec.


static void DecompressRle(QTM *pqtm, uchar *pd, uchar *ps, long csize)
{
	uchar *spoint=ps+15;
	uchar *dpoint=pd;
	int i,j,k;
	int width_count=0,height_count=0;
	uchar quit_flag=0;
	char command;

	while (!quit_flag)
		{
		command = *(char *)(spoint++);
		if (command == -1)   
			{
			if (*(spoint++) == 1)
				{
				if (width_count != pqtm->frameWidth)
					{
					Warning(("Error in decompressing rle, width mismatch"));
					return;
					}
				width_count = 0;
				height_count++;
				}
			else
				quit_flag=1;
			}
		else if (command < 0)
			{
			if (width_count >= pqtm->frameWidth)
				{
				Warning(("Error in decompressing rle, width mismatch"));
				return;
				}
			for (i = command; i < 0; i++)
				for (j = 0; j < 4; j++)
					*(dpoint++) = *(spoint + j);
			width_count += -4 * command;
			spoint += 4;
			}
		else
			{
			if (width_count >= pqtm->frameWidth)
				{
				Warning(("Error in decompressing rle, width mismatch"));
				return;
				}
			for (i = 0; i < (command * 4); i++)
				{
				if (width_count < pqtm->frameWidth)
					{
					*(dpoint++) = *spoint;
					width_count++;
					}
				spoint++;        
				}
			}
		}

	if (height_count != (pqtm->frameHeight - 1))
		{
		Warning(("Error in decompressing rle, height mismatch"));
		return;
		}
}

#pragma on(unreferenced);

//	--------------------------------------------------------------
//		MISCELLANEOUSH INTERNAL ROUTINES
//	--------------------------------------------------------------
//
//	ReadVarChunk() allocates memory and reads a variable-sized chunk
//	into it.

static void ReadVarChunk(FILE *fpi, QT_ChunkHdr *phdr, void **p)
{
	long length;

	length = phdr->length - sizeof(QT_ChunkHdr);
	*p = Malloc(length);
	QuikReadChunk(fpi, phdr, *p, length);
}

static ushort Flip2Val(ushort v)
{
	return(((v & 0xFF) << 8) | (v >> 8));
}
*/
