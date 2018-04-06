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
//		QUIKTIME.H		QuickTime file access
//		Rex E. Bradford

/*
 * $Source: r:/prj/lib/src/afile/RCS/quiktime.h $
 * $Revision: 1.4 $
 * $Author: rex $
 * $Date: 1994/10/04 20:31:57 $
 * $Log: quiktime.h $
 * Revision 1.4  1994/10/04  20:31:57  rex
 * Added depth16 flag and macro to set it
 * 
 * Revision 1.3  1994/09/30  16:57:34  rex
 * Added stuff for writing quicktime movies
 * 
 * Revision 1.2  1994/09/29  10:34:57  rex
 * Put globals into a single struct, revamped prototypes, added writing
 * 
 * Revision 1.1  1994/09/27  17:23:17  rex
 * Initial revision
 * 
*/

#ifndef __QUIKTIME_H
#define __QUIKTIME_H

#include <stdio.h>

#ifndef __TYPES_H
#include "lg_types.h"
#endif
#ifndef __FIX_H
#include "fix.h"
#endif
#ifndef __2D_H
#include "2d.h"
#endif

//	Data types

typedef short fix8;		// 8.8 fixed-point number
typedef ulong QT_Ctype;	// quicktime chunk type

typedef struct {
	ulong length;			// chunk length
	QT_Ctype ctype;		// chunk type
} QT_ChunkHdr;

typedef struct {
	ulong ctype;			// chunk type
	bool isleaf;			// is leaf chunk, or are there subchunks
} QT_ChunkInfo;

typedef struct {
	uchar len;				// pascal strings start with length byte
	char str[1];			// followed by string
} PStr;

//	4-char chunk mnemonics

#define MAKE4(c0,c1,c2,c3) ((((ulong)c0)<<24)|(((ulong)c1)<<16)|(((ulong)c2)<<8)|((ulong)c3))

#define QT_CLIP	MAKE4('c','l','i','p')
#define QT_CRGN	MAKE4('c','r','g','n')
#define QT_DINF	MAKE4('d','i','n','f')
#define QT_DREF	MAKE4('d','r','e','f')
#define QT_EDTS	MAKE4('e','d','t','s')
#define QT_ELST	MAKE4('e','l','s','t')
#define QT_HDLR	MAKE4('h','d','l','r')
#define QT_KMAT	MAKE4('k','m','a','t')
#define QT_MATT	MAKE4('m','a','t','t')
#define QT_MDAT	MAKE4('m','d','a','t')
#define QT_MDIA	MAKE4('m','d','i','a')
#define QT_MDHD	MAKE4('m','d','h','d')
#define QT_MINF	MAKE4('m','i','n','f')
#define QT_MOOV	MAKE4('m','o','o','v')
#define QT_MVHD	MAKE4('m','v','h','d')
#define QT_SMHD	MAKE4('s','m','h','d')
#define QT_STBL	MAKE4('s','t','b','l')
#define QT_STCO	MAKE4('s','t','c','o')
#define QT_STSC	MAKE4('s','t','s','c')
#define QT_STSD	MAKE4('s','t','s','d')
#define QT_STSH	MAKE4('s','t','s','h')
#define QT_STSS	MAKE4('s','t','s','s')
#define QT_STSZ	MAKE4('s','t','s','z')
#define QT_STTS	MAKE4('s','t','t','s')
#define QT_TKHD	MAKE4('t','k','h','d')
#define QT_TRAK	MAKE4('t','r','a','k')
#define QT_UDTA	MAKE4('u','d','t','a')
#define QT_VMHD	MAKE4('v','m','h','d')

//	Auxiliary structures

//typedef struct {
//	uchar len;
//	uchar str[31];
//} Str31;

typedef struct {
	uchar version;
	uchar flags[3];
	ulong numEntries;
} QTS_STSD_Base;

typedef struct {
	ulong descSize;
	ulong dataFormat;
	uchar reserved1[6];
	short dataRefIndex;
	short version;
	short revLevel;
	ulong vendor;
	short numChans;
	short sampSize;
	short compId;
	short packetSize;
	fix sampRate;
} QT_SoundDesc;

typedef struct {
	ulong descSize;
	ulong dataFormat;
	uchar reserved1[6];
	short dataRefIndex;
	short version;
	short revLevel;
	ulong vendor;
	ulong temporalQuality;
	ulong spatialQuality;
	short width;
	short height;
	fix hRes;
	fix vRes;
	long dataSize;
	short frameCount;
	Str31 name;
	short depth;
	short clutId;
} QT_ImageDesc;

typedef struct {
	ulong descSize;
	ulong dataFormat;
	// more stuff, but who cares?
} QT_TextDesc;

typedef struct {
	ulong trackDuration;
	long mediaTime;
	fix mediaRate;
} QT_EditList;

typedef struct {
	ulong count;
	ulong duration;
} QT_Time2Samp;

typedef struct {
	ulong firstChunk;
	ulong sampsPerChunk;
	ulong sampDescId;
} QT_Samp2Chunk;

typedef struct {
	ulong frameDiffSampNum;
	ulong syncSampNum;
} QT_ShadowSync;

//	Chunk structures

typedef struct {
	uchar version;
	uchar flags[3];
	ulong numEntries;
	QT_EditList editList[1];
} QTS_ELST;

typedef struct {
	uchar version;
	uchar flags[3];
	ulong compType;
	ulong compSubtype;
	ulong compManufacturer;
	ulong compFlags;
	ulong compFlagsMask;
	PStr compName;
} QTS_HDLR;

typedef struct {
	uchar version;
	uchar flags[3];
	ulong createTime;
	ulong modTime;
	ulong timeScale;
	ulong duration;
	short language;
	short quality;
} QTS_MDHD;

typedef struct {
	uchar version;
	uchar flags[3];
	ulong createTime;
	ulong modTime;
	ulong timeScale;
	ulong duration;
	fix preferredRate;
	fix8 preferredVol;
	uchar reserved[10];
	fix matrix[9];
	ulong previewTime;
	ulong previewDur;
	ulong posterTime;
	ulong selTime;
	ulong selDur;
	ulong currTime;
	ulong nextTrackId;
} QTS_MVHD;

typedef struct {
	uchar version;
	uchar flags[3];
	short balance;
	short reserved;
} QTS_SMHD;

typedef struct {
	uchar version;
	uchar flags[3];
	ulong numEntries;
	ulong offset[1];
} QTS_STCO;

typedef struct {
	uchar version;
	uchar flags[3];
	ulong numEntries;
	QT_Samp2Chunk samp2chunk[1];
} QTS_STSC;

typedef struct {
	QTS_STSD_Base base;
	union {
		QT_SoundDesc sdesc;
		QT_ImageDesc idesc;
		QT_TextDesc tdesc;
		};
} QTS_STSD;

typedef struct {
	uchar version;
	uchar flags[3];
	ulong numEntries;
	QT_ShadowSync shadowSync[1];
} QTS_STSH;

typedef struct {
	uchar version;
	uchar flags[3];
	ulong numEntries;
	ulong sample[1];
} QTS_STSS;

typedef struct {
	uchar version;
	uchar flags[3];
	ulong sampSize;
	ulong numEntries;
	ulong sampSizeTab[1];
} QTS_STSZ;

typedef struct {
	uchar version;
	uchar flags[3];
	ulong numEntries;
	QT_Time2Samp time2samp[1];
} QTS_STTS;

typedef struct {
	uchar version;
	uchar flags[3];
	ulong createTime;
	ulong modTime;
	ulong trackId;
	uchar reserved1[4];
	ulong duration;
	uchar reserved2[8];
	short layer;
	short altGroup;
	fix8 volume;
	uchar reserved3[2];
	fix matrix[9];
	fix trackWidth;
	fix trackHeight;
} QTS_TKHD;

typedef struct {
	uchar version;
	uchar flags[3];
	short graphicsMode;
	ushort opColor[3];
} QTS_VMHD;

//	Quicktime movie structures for reading and writing whole movies

typedef enum {TRACK_VIDEO,TRACK_AUDIO,TRACK_OTHER} TrackType;

typedef struct {
	QTS_TKHD	qt_tkhd;		// track header (TKHD)
	QTS_MDHD qt_mdhd;		// media header (MDHD)
	QTS_STSD qt_stsd;		// sample descriptor (STSD)
	QTS_STTS *qt_stts;	// ptr to time->sample table (STTS)
	QTS_STSC *qt_stsc;	// ptr to sample->chunk table (STSC)
	QTS_STSZ *qt_stsz;	// ptr to sample size table (STSZ)
	QTS_STCO *qt_stco;	//	ptr to chunk->offset table (STCO)

	uchar *palette;		// 256-entry palette or NULL
	ulong numSamps;		// number of samples
	uchar *sampBuff;		// sample buffer (if NULL, then in file)
	fix *sampTime;			// array of sample->time
	ulong *sampSize;		// array of sample->size
	ulong *sampOffset;	// array of sample->fileoffset
	ulong audioBlockSize;	// # bytes per audio sample block
	TrackType type;		// TRACK_XXX
//	The following only used when writing movies
	short numSamplesAlloced;	// # sample entries allocated in buffs
	fix *pSampleTime;		// ptr to array of sample times
} MovieTrack;

typedef struct {			// info about current track, have we gotten
	bool gotTKHD;			// these important chunks? (TKHD, MDHD, STBL)
	bool gotMDHD;
	bool gotSTBL;
} MovieTrackStatus;

#define QTM_MAX_TRACKS 8			// hey, what's in this thing anyway??
#define QTM_MAX_CHUNK_NESTING 8	// max nesting of subchunks

typedef struct {
	QTS_MVHD qt_mvhd;			// global movie header
	MovieTrack track[QTM_MAX_TRACKS];	// here are all my tracks!
	MovieTrack *pVideoTrack;	// ptr to single video track, or NULL if no video
	MovieTrack *pAudioTrack;	// ptr to audio track, or NULL if no audio
	int numTracks;					// number of tracks read into track[] array
	int numAudioSamplesPerBlock;	// 4K if 11Khz, 8K if 22 Khz
	ulong compTypeQT;				// type of QuickTime VIDEO compression
	short frameWidth;				// frame width of video track
	short frameHeight;			// frame height of video track
	uchar *pFrameCurr;			// current frame in flat format
	uchar *pFrameCompQT;			// current frame in compressed format
//	The following used only when writing movies
	long offsetStack[QTM_MAX_CHUNK_NESTING];	// stack of lengths to update
	short indexOffsetStack;		// current index into offset stack
	bool depth16;					// convert to 16-bit depth when writing
} QTM;

//	Prototypes: quiktime.c (reading movie for processing/conversion)

void QuikReadMovie(QTM *pqtm, FILE *fpi);
void QuikFreeMovie(QTM *pqtm);
fix QuikComputeFrameRate(QTM *pqtm);
void *QuikGetVideoSample(QTM *pqtm, int isample, FILE *fpi, long *plength,
	uchar *pbmtype, fix *ptime);
void *QuikGetAudioSample(QTM *pqtm, int isample, long *plength, fix *ptime);

//	Protoypes: quikwrite.c (writing movie)

void QuikCreateMovie(QTM *pqtm, FILE *fp);
void QuikSetPal(QTM *pqtm, uchar *pal);
void QuikAddVideoSample(QTM *pqtm, FILE *fp, grs_bitmap *pbm, fix time);
void QuikWriteMovieAndClose(QTM *pqtm, FILE *fp);

#define QuikSetDepth16(pqtm) ((pqtm)->depth16 = TRUE)

//	Prototypes: quikconv.c (reading chunks for inspection/dumping)

FILE *QuikOpenFile(char *filename);
bool QuikReadChunkHdr(FILE *fp, QT_ChunkHdr *phdr);
void QuikSkipChunk(FILE *fp, QT_ChunkHdr *phdr);
bool QuikReadChunk(FILE *fp, QT_ChunkHdr *phdr, void *buff, ulong bufflen);
void QuikWriteChunkHdr(FILE *fp, QT_ChunkHdr chunkHdr);
void QuikWriteChunkLength(FILE *fp, long length);
void QuikWriteChunk(FILE *fp, QT_Ctype ctype, void *data, ulong len);
QT_ChunkInfo *QuikFindChunkInfo(QT_ChunkHdr *phdr);

#define Flip4(v) Flip4Func((ulong *)(v))
#define Flip2(v) Flip2Func((ushort *)(v))

void Flip4Func(ulong *pval4);
void Flip2Func(ushort *pval2);

//	Prototypes: quikprnt.c (printing chunks gotten via quikconv.c)

void QuikPrintChunk(QT_ChunkHdr *phdr, void *buff, char *indent);

#endif

