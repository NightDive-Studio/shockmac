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
/*
 * $Source: r:/prj/lib/src/afile/RCS/movie.h $
 * $Revision: 1.16 $
 * $Author: dc $
 * $Date: 1994/12/01 04:17:50 $
 */

#ifndef __MOVIE_H
#define __MOVIE_H

#ifndef __TYPES_H
#include "lg_types.h"
#endif
#ifndef __FIX_H
#include "fix.h"
#endif
#ifndef __2D_H
#include "2d.h"
#endif
#ifndef __RES_H
#include "res.h"
#endif
#ifndef __CIRCBUFF_H
#include "circbuff.h"
#endif
/*
#ifndef AIL_H
#include <ail.h>
#endif
*/
//	Movie file-format structures:
//
//	1. MovieHeader (1K) at head of file (check MOVI_MAGIC_ID)
//	2. MovieChunk[] array, padded to 1K/3K/5K (so array + hdr mult of 2K)
//	3. Actual chunks, as pointed at in MovieChunk[] array

//	Movie chunk format

typedef struct {
	ulong time: 24;		// fixed-point time since movie start
    ulong played: 1;     // has this chunk been clocked out?
	ulong flags: 4;		// chunkType-specific
	ulong chunkType: 3;	// MOVIE_CHUNK_XXX
	ulong offset;			// byte offset to chunk start
} MovieChunk;

//	Movie chunk types

#define MOVIE_CHUNK_END		0
#define MOVIE_CHUNK_VIDEO	1
#define MOVIE_CHUNK_AUDIO	2
#define MOVIE_CHUNK_TEXT	3
#define MOVIE_CHUNK_PALETTE	4
#define MOVIE_CHUNK_TABLE	5

//	Movie chunk flags

#define MOVIE_FVIDEO_BMTMASK	0x0F	// video chunk, 4 bits of flags is bmtype
#define MOVIE_FVIDEO_BMF_4X4	0x0F	// 4x4 movie format

#define MOVIE_FPAL_EFFECTMASK	0x07	// pal chunk, 4 bits of flags is effect
#define MOVIE_FPAL_SET			0x00	// set palette from data
#define MOVIE_FPAL_BLACK		0x01	// set palette to black
#define MOVIE_FPAL_CLEAR		0x08	// if bit set, also clear screen

#define MOVIE_FTABLE_COLORSET	0		// table chunk, table is color set
#define MOVIE_FTABLE_HUFFTAB	1		// huffman table (compressed)

//	Movie header layout

typedef struct {
	ulong magicId;			// 'MOVI' (MOVI_MAGIC_ID)
	long numChunks;		// number of chunks in movie
	long sizeChunks;		// size in bytes of chunk array
	long sizeData;			// size in bytes of chunk data
	fix totalTime;			// total playback time
	fix frameRate;			// frames/second, for info only
	short frameWidth;		// frame width in pixels
	short frameHeight;	// frame height in pixels
	short gfxNumBits;		// 8, 15, 24
	short isPalette;		// is palette present?
	short audioNumChans;	// 0 = no audio, 1 = mono, 2 = stereo
	short audioSampleSize;	// 1 = 8-bit, 2 = 16-bit
	fix audioSampleRate;	// in Khz
	uchar reserved[216];	// so chunk is 1K in size
	uchar palette[768];	// palette
} MovieHeader;

#ifndef SAMPRATE_11KHZ	// also appear in voc.h
#define SAMPRATE_11KHZ fix_make(11127,0)
#define SAMPRATE_22KHZ fix_make(22254,0)
#endif

#define MOVI_MAGIC_ID	0x4D4F5649

//	Movie text chunk begins with a 0-terminated array of these:

typedef struct {
	ulong tag;			// 'XXXX'
	ulong offset;		// offset to text string
} MovieTextItem;

#define MOVIE_TEXTITEM_MAKETAG(c1,c2,c3,c4) ((((ulong)c4)<<24)|(((ulong)c3)<<16)|(((ulong)c2)<<8)|(c1))
#define MOVIE_TEXTITEM_TAG(pmti,index) ((pmti+(index))->tag)
#define MOVIE_TEXTITEM_PTR(pmti,index) ((char *)(pmti) + (pmti+(index))->offset)
#define MOVIE_TEXTITEM_EXISTS(pmti,index) MOVIE_TEXTITEM_TAG(pmti,index)

#define MOVIE_TEXTITEM_STDTAG		MOVIE_TEXTITEM_MAKETAG('S','T','D',' ')

//	Movie runtime structures

typedef struct {
	short sizeBuffers;		// size of each buffer
	uchar *pbuff[2];			// sound buffers (raw ptrs)
} MovieAudioBuffers;

typedef struct {
	CircBuff cb;					// circular data buffer
	long blockLen;					// # bytes to read in each block
	long ovfLen;					// # overflow bytes past circular buffer
	MovieChunk *pCurrChunk;		// ptr to current chunk to use
	long bytesLeft;				// bytes left to read
} MovieBuffInfo;

typedef struct {
   int   snd_in;
	short nextBuff;		// next buffer to load (0 or 1, -1 for none)
   short smp_id;        // snd lib id of the current sample
} MovieAudioState;
/*
typedef struct Movie_ {
	MovieHeader *pmh;		// ptr to movie header (read from 1st bytes of movie)
	MovieChunk *pmc;		// ptr to movie chunk array
	int fd;					// file being read from
	long fileOff;			// offset in file to start of movie
	grs_canvas *pcanvas;	// ptr to canvas being played into
	fix tStart;				// time movie started
	MovieBuffInfo bi;		// movie buffering info
	MovieAudioState as;	// current audio state for each channel
	uchar *pColorSet;		// ptr to color set table (4x4 codec)
	long lenColorSet;		// length of color set table
	uchar *pHuffTab;		// ptr to huffman table (4x4 codec)
	long lenHuffTab;		// length of huffman table
	void (*f_VideoCallback)(struct Movie_ *pmovie);	// video callback for composing
	void (*f_TextCallback)(struct Movie_ *pmovie, MovieTextItem *pitem);	// text chunk callback
	void *pTextCallbackInfo;	// info maintained by text callback
	bool playing;			// is movie playing?
 	bool processing;		// is movie processing?
	bool singleStep;		// single step movie
	bool clipCanvas;		// clip to canvas?
} Movie;

//	Prototypes

Movie *MoviePrepare(int fd, uchar *buff, long buffLen, long blockLen);
Movie *MoviePrepareRes(Id id, uchar *buff, long buffLen, long blockLen);
void MovieReadAhead(Movie *pmovie, int numBlocks);
void MoviePlay(Movie *pmovie, grs_canvas *pcanvas);
void MovieUpdate(Movie *pmovie);
void MovieAdvance(Movie *pmovie);
void MovieRestart(Movie *pmovie);
void MovieKill(Movie *pmovie);

#define TXTCB_FLAG_CENTER_X   0x01
#define TXTCB_FLAG_CENTER_Y   0x02
#define TXTCB_FLAG_CENTERED   TXTCB_FLAG_CENTER_X|TXTCB_FLAG_CENTER_Y

void MovieInstallStdTextCallback(Movie *pmovie, ulong lang, Id fontId,
	uchar color, uchar flags);

#define MovieChunkLength(pmc) (((pmc)+1)->offset-(pmc)->offset)
#define MoviePlaying(pmovie) ((pmovie)->playing)
#define MovieSetSingleStep(pmovie,on) ((pmovie)->singleStep=(on))
#define MovieSetVideoCallback(pmovie,f) ((pmovie)->f_VideoCallback=(f))
#define MovieSetTextCallback(pmovie,f) ((pmovie)->f_TextCallback=(f))
#define MovieSetAudioBuffers(pmab) movieAudioBuffers = *(pmab)
#define MovieClearCanvas(pmovie) { \
	gr_push_canvas((pmovie)->pcanvas); gr_clear(0); gr_pop_canvas(); }
#define MovieSetPal(pmovie,s,n) if ((pmovie)->pmh->isPalette) gr_set_pal(s, n, (pmovie)->pmh->palette)

extern MovieAudioBuffers movieAudioBuffers;

#define MOVIE_DEFAULT_BLOCKLEN 8192

// 4x4 cleanup routine (frees 

void Draw4x4FreeResources();
*/
#endif

