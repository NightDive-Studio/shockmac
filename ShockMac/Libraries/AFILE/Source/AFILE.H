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
//		AFILE.H		Generic animation file access
//		Rex E. Bradford (REX)
/*
* $Header: r:/prj/lib/src/afile/RCS/afile.h 1.2 1994/09/29 10:29:58 rex Exp $
* $Log: afile.h $
 * Revision 1.2  1994/09/29  10:29:58  rex
 * Added time arg to read/write frame
 * 
 * Revision 1.1  1994/07/22  13:20:59  rex
 * Initial revision
 * 
*/

#ifndef __AFILE_H
#define __AFILE_H

#ifndef STDIO_H
#include <stdio.h>
#endif
#ifndef __2D_H 
#include "2d.h"
#endif
//#ifndef DATAPATH_H
//#include <datapath.h>
//#endif
#ifndef __FIX_H
#include "fix.h"
#endif

//	Animation file types

typedef enum {
	AFILE_FLC,				// .flc, .fli, .cel (Autodesk Animator Pro)
	AFILE_ANM,				// .anm (DeluxePaint Anim)
	AFILE_QTM,				// .qtm (QuickTime, brought over from Mac)
	AFILE_MOV,				// .mov (LookingGlass movie)
	AFILE_GFILE,			// set of gfiles (.pcx, .gif, etc.)
	AFILE_BAD				// unknown/bad type
} AfileType;

//	Structures used in dealing with anim files

typedef struct {
	short index;			// index at which to start writing
	short numcols;			// number of colors to write
	uchar rgb[3 * 256];		// 0-256 rgb entries
} Apalette;

typedef struct {
	long numFrames;			// number of frames of video
	fix frameRate;			// frame rate in frames per second
	short width;			// width in pixels
	short height;			// height in pixels
	short numBits;			// number of bits per pixel (8, 15, 24)
	Apalette pal;			// (base) palette
} AvideoInfo;

typedef struct {
	short numChans;			// 0 = no audio, 1 = mono, 2 = stereo
	short sampleSize;		// 1 = 8-bit, 2 = 16-bit
	fix sampleRate;			// in Khz
	long numSamples;		// total number of samples (per channel)
} AaudioInfo;

struct Amethods_;

typedef struct Afile_ {
	FILE *fp;				// file access ptr
	AfileType type;			// file type
	bool writing;			// if TRUE, opened for write (FALSE = read)
	bool writerWantsRsd;	// if TRUE, writer wants rsd frames
	struct Amethods_ *pm; 	// ptr to access method ptrs
	AvideoInfo v;			// video info
	AaudioInfo a;			// audio info
	void *pspec;			// ptr to type-specific info
	int currFrame;			// current frame index
	long frameLen;			// length of frame (width & height * sizeof(pixel))
	grs_bitmap bmCompose;	// compose buffer
	grs_bitmap bmPrev;		// previous compose buffer
	grs_bitmap bmWork;		// working buffer bitmap
} Afile;

typedef struct Amethods_ {
	int (*f_ReadHeader)(Afile *paf);
	long (*f_ReadFrame)(Afile *paf, grs_bitmap *pbm, fix *ptime);
	int (*f_ReadFramePal)(Afile *paf, Apalette *ppal);
	int (*f_ReadAudio)(Afile *paf, void *paudio);
	int (*f_ReadReset)(Afile *paf);
	int (*f_ReadClose)(Afile *paf);
	int (*f_WriteBegin)(Afile *paf);
	int (*f_WriteAudio)(Afile *paf, void *paudio);
	int (*f_WriteFrame)(Afile *paf, grs_bitmap *pbm, long bmlength, fix time);
	int (*f_WriteFramePal)(Afile *paf, Apalette *ppal);
	int (*f_WriteClose)(Afile *paf);
} Amethods;

//	Function prototypes: reading anim files

int AfileOpen(Afile *paf, char *filename /*Datapath *pdp*/);
long AfileReadFullFrame(Afile *paf, grs_bitmap *pbm, fix *ptime);
long AfileReadDiffFrame(Afile *paf, grs_bitmap *pbm, fix *ptime);
bool AfileGetFramePal(Afile *paf, Apalette *ppal);
int AfileGetAudio(Afile *paf, void *paudio);
int AfileReadReset(Afile *paf);
void AfileClose(Afile *paf);		// write also

//	Function prototypes: writing anim files

int AfileCreate(Afile *paf, char *filename, fix frameRate);
int AfilePutAudio(Afile *paf, AaudioInfo *pai, void *paudio);
int AfileWriteFrame(Afile *paf, grs_bitmap *pbm, fix time);
void AfileSetPal(Afile *paf, Apalette *ppal);
int AfileSetFramePal(Afile *paf, Apalette *ppal);

//	Function prototypes: information & miscellaneous

AfileType AfileLookupType(char *ext);
long AfileBitmapLength(Afile *paf);
long AfileAudioLength(Afile *paf);

// Misc functions

long SwapLongBytes(long in);
short SwapShortBytes(short in);

#endif

