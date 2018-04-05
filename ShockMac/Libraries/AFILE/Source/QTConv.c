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
//	==============================================================
//	 Convert a raw QT movie into a nice compressed movie.  Set the movie's color table.
//	==============================================================

#include <stdlib.h>
#include <string.h>

#include "lg.h"
#include "rect.h"
#include "fix.h"
#include "quiktime.h"
#include "2d.h"

#include "InitMac.h"
#include "ShockBitmap.h"
extern Ptr	gScreenAddress;
extern long	gScreenRowbytes;

#include <Movies.h>
#include <ImageCompression.h>
#include <QuickTimeComponents.h>
#include <Sound.h>


//  Globals
ComponentInstance	ci = nil;
WindowPtr		gMainWindow;
extern short		gMainVRef;
short			gResNum;
ulong			*gSampleTimes;
ulong			*gChunkOffsets;
long				gNumFrames;
long				gFakeIndex;
QT_ChunkInfo chunkInfo[] =
{
	QT_CLIP,FALSE,
	QT_CRGN,TRUE,
	QT_DINF,FALSE,
	QT_DREF,TRUE,
	QT_EDTS,FALSE,
	QT_ELST,TRUE,
	QT_HDLR,TRUE,
	QT_KMAT,TRUE,
	QT_MATT,FALSE,
	QT_MDAT,TRUE,
	QT_MDIA,FALSE,
	QT_MDHD,TRUE,
	QT_MINF,FALSE,
	QT_MOOV,FALSE,
	QT_MVHD,TRUE,
	QT_SMHD,TRUE,
	QT_STBL,FALSE,
	QT_STCO,TRUE,
	QT_STSC,TRUE,
	QT_STSD,TRUE,
	QT_STSH,TRUE,
	QT_STSS,TRUE,
	QT_STSZ,TRUE,
	QT_STTS,TRUE,
	QT_TKHD,TRUE,
	QT_TRAK,FALSE,
	QT_UDTA,FALSE,
	QT_VMHD,TRUE,
	0,0
};
TrackType currTrackType;

//  Prototypes
void main(void);
void	 CheckError(OSErr error, Str255 displayString);
void SetInputSpecs(void);
void SetPalette(short palID);

/*
bool QuikReadChunkHdr(FILE *fp, QT_ChunkHdr *phdr);
QT_ChunkInfo *QuikFindChunkInfo(QT_ChunkHdr *phdr);
void QuikSkipChunk(FILE *fp, QT_ChunkHdr *phdr);
*/
bool QuikReadChunkHdr(Ptr &p, QT_ChunkHdr *phdr);
QT_ChunkInfo *QuikFindChunkInfo(QT_ChunkHdr *phdr);
void QuikSkipChunk(Ptr &p, QT_ChunkHdr *phdr);


//	---------------------------------------------------------------
//		MAIN PROGRAM
//	---------------------------------------------------------------

void main(void)
{
	grs_screen 		*screen;
	OSErr			err, result;
	
	StandardFileReply	reply;
	SFTypeList			typeList;

	FILE 			*fp;
	uchar 			*dbuff;
	ulong 			dbuffLen;
	QT_ChunkHdr 	chunkHdr;
	QT_ChunkInfo 	*pinfo;

	CTabHandle		ctab;
		
	Point			dlgPos = {120,120};
	SFReply			sfr;
	FSSpec			mySpec;
	Str255			name = "\pQT Movie";
	Rect			movieRect, r;
	ImageDescription	**imageDescriptionH = 0L;		// Contains info about the sample
	short			resRefNum;
	Movie			gMovie = 0;							// Our movie, track and media
	Track			gTrack;
	Media			gMedia;
	
	Handle			movieRsrc;

	//---------------------
	//	Init graphics system
	//---------------------
	InitMac();
	CheckConfig();

	SetupWindows(&gMainWindow);								// setup everything
	SetupOffscreenBitmaps();			

	gr_init();
	gr_set_mode (GRM_640x480x8, TRUE);
	screen = gr_alloc_screen (grd_cap->w, grd_cap->h);
	gr_set_screen (screen);
	gr_clear(0xff);
	
	//---------------------
	// Setup Quicktime stuff.
	//---------------------
	if (EnterMovies() != noErr)	// Start up the movie tools
	{
		ParamText("\pCan't startup QuickTime.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		CleanupAndExit();
	}
	
	//-------------------------------------------------------------
	//	Open the QuickTime movie as a QuickTime movie, and get information about the move.
	//-------------------------------------------------------------
	typeList[0] = 'MooV';
	StandardGetFilePreview(nil, 1, typeList, &reply);
	if (!reply.sfGood)
	{
	 	ExitMovies();
		CleanupAndExit();
 	}
	err = OpenMovieFile(&reply.sfFile, &resRefNum, fsRdPerm);
	if (err == noErr) 
	{
		movieRsrc = GetIndResource('moov', 1);
		if (!movieRsrc)
			CheckError(1, "\pCan't get movie resource!");
		DetachResource(movieRsrc);
		
		short 		movieResID = 0;		// get first movie
		Str255 		movieName;
		Boolean 		wasChanged;
	
		err = NewMovieFromFile(&gMovie, resRefNum, &movieResID,
						movieName, newMovieActive, &wasChanged);
		CloseMovieFile( resRefNum );
	}
	else
		CheckError(1, "\pCan't open the movie file!!");
	
	// Get movie rectangle
	GetMovieBox (gMovie, &movieRect);		
	OffsetRect (&movieRect, -movieRect.left, -movieRect.top);
	
	// Set movie palette
	GetMovieColorTable(gMovie, &ctab);
	SetEntries(1, 253, &(**ctab).ctTable[1]);

	DisposeMovie(gMovie);

	//------------------------------------------------
	//	Open the input QuickTime movie again, this time as a data fork file.
	//------------------------------------------------
	fp = fopen(p2cstr(reply.sfFile.name), "rb");
	if (fp == NULL)
		CheckError(1, "\pCan't open the input movie!!");

	dbuffLen = 64000;
	dbuff = (uchar *)malloc(dbuffLen);
	
	//----------------------
	//	Setup output file.
	//----------------------
	SFPutFile(dlgPos, "\pSave Movie as:",name,0L,&sfr);
	if (!sfr.good)
	 {
	 	ExitMovies();
		CleanupAndExit();
 	 }
 	 
	ClearMoviesStickyError();
	FSMakeFSSpec(sfr.vRefNum, 0, sfr.fName, &mySpec);
	err = CreateMovieFile(&mySpec, 'TVOD', 0, createMovieFileDeleteCurFile, &resRefNum, &gMovie);
	CheckError(err, "\pCan't create output movie file.");

	SetMovieColorTable(gMovie, ctab);

	gTrack = NewMovieTrack(gMovie, movieRect.right<<16, movieRect.bottom<<16, 0);
	CheckError (GetMoviesError(), "\pNew video track." );

	gMedia = NewTrackMedia(gTrack, VideoMediaType, 600, 0L, 0L);
	CheckError (GetMoviesError(), "\pNew Media for video track." );

	BeginMediaEdits(gMedia);		// We do this since we are adding samples to the media

	//-----------------------------------
	//  Setup the standard compression component stuff.
	//-----------------------------------
	ci = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubType);
	if (!ci)
		CheckError (-1, "\pCan't open the Standard Compression component." );

	//-----------------------------------
	//	Get the Chunk Offset and Sample-to-Time tables.
	//-----------------------------------
	HLock(movieRsrc);
	Ptr	mp = (Ptr)*movieRsrc;
	while (TRUE)
	{
		//if (!QuikReadChunkHdr(fp, &chunkHdr))
		if (!QuikReadChunkHdr(mp, &chunkHdr))
			break;
		if (chunkHdr.length == 0)
			break;
		pinfo = QuikFindChunkInfo(&chunkHdr);
		if (pinfo->isleaf)
		{
			if ((chunkHdr.ctype != QT_MDAT))
			{
				if (chunkHdr.length > dbuffLen)
				{
					dbuffLen = chunkHdr.length;
					dbuff = (uchar *)realloc(dbuff, dbuffLen);
				}
				//fread(dbuff, chunkHdr.length - sizeof(QT_ChunkHdr), 1, fp);
				BlockMove(mp, dbuff, chunkHdr.length - sizeof(QT_ChunkHdr));
				mp += chunkHdr.length - sizeof(QT_ChunkHdr);
				
				// For the sample-to-time table, create our own table giving a time
				// for each frame.
				if (chunkHdr.ctype == QT_STTS)
				{
					QTS_STTS	*p = (QTS_STTS *)dbuff;
					short		i, j, si;
					
					gSampleTimes = (ulong *)NewPtr(1200 * sizeof(ulong));
					si = 0;
					for (i = 0; i < p->numEntries; i++)
						for (j = 0; j < p->time2samp[i].count; j++)
							gSampleTimes[si++] = p->time2samp[i].duration;
				}
				
				// For the chunk offsets table, read it in to a memory block.
				if (chunkHdr.ctype == QT_STCO)
				{
					QTS_STCO 	*p = (QTS_STCO *)dbuff;
					
					gNumFrames = p->numEntries;
					gChunkOffsets = (ulong *)NewPtr(p->numEntries * sizeof(ulong));
					BlockMove(p->offset, gChunkOffsets, p->numEntries * sizeof(ulong));
				}
			}
			else
				//QuikSkipChunk(fp, &chunkHdr);
				QuikSkipChunk(mp, &chunkHdr);
		}
	}
	HUnlock(movieRsrc);

	HideCursor();
	
	//----------------------------
	//	Show the movie, one frame at a time.
	//----------------------------
	{
		long			f, line;
		Ptr			frameBuff;
		Ptr			imgp, scrp;
		RGBColor	black = {0, 0, 0};
		RGBColor	white = {0xffff, 0xffff, 0xffff};
		char		buff[64];
		ulong		sampTime;
		Boolean		subColor;
 		Handle		compHdl;
 		long			compSize;
 		short		notSyncFlag;
 		uchar		*pp;
		
		frameBuff = NewPtr(movieRect.right * movieRect.bottom);
		CheckError(MemError(), "\pCan't allocate a frame buffer for input movie.");
		
		SetRect(&r, 0, -17, 300, 0);
		for (f = 0; f < gNumFrames; f++)
		{
			if (f % 4 == 3)		// Skip every fourth frame
				continue;
			
			// Read the next frame from the input movie.
			fseek(fp, gChunkOffsets[f], SEEK_SET);
			fread(frameBuff, movieRect.right*movieRect.bottom, 1, fp);
			
			// See if there's an 0xFF anywhere in this screen.
			subColor = FALSE;
			pp = (uchar *)frameBuff;
			for (int pix = 0; pix < movieRect.right*movieRect.bottom; pix++)
			{
				if (*pp == 0x00)
				{
					*pp = 0xFF;
					subColor = TRUE;
				}
				pp++;
			}
				
			imgp = frameBuff;
			scrp = gScreenAddress;
			for (line = 0; line < movieRect.bottom; line++)
			{
				BlockMove(imgp, scrp, movieRect.right);
				imgp += movieRect.right;
				scrp += gScreenRowbytes;
			}
			
			// The first time through the loop (after displaying the first frame), set the compression 
			// parameters for the output movie and begin a compression sequence.
			if (f == 0)
			{
				result = SCDefaultPixMapSettings(ci, ((CGrafPort *)(gMainWindow))->portPixMap, TRUE);
			 	result = SCRequestSequenceSettings(ci);
			 	if (result == scUserCancelled)
			 	{
					CloseComponent(ci);
			 	 	ExitMovies();
					CleanupAndExit();
			 	}
				CheckError(result, "\pError in sequence settings.");
				
				// Redraw the first frame on the screen.
				gr_clear(0xFF);
				imgp = frameBuff;
				scrp = gScreenAddress;
				for (line = 0; line < movieRect.bottom; line++)
				{
					BlockMove(imgp, scrp, movieRect.right);
					imgp += movieRect.right;
					scrp += gScreenRowbytes;
				}
				
				// Begin a compression sequence.
				result = SCCompressSequenceBegin(ci, ((CGrafPort *)(gMainWindow))->portPixMap,
												  	&movieRect, &imageDescriptionH);
				CheckError(result, "\pCan't start a sequence.");
			}
			
			// Display the frame number.
			
			sprintf(buff, "Frame: %d", f);
			RGBForeColor(&black);
			PaintRect (&r);
			MoveTo(1, -6);
			RGBForeColor(&white);
			DrawText(buff, 0, strlen(buff));
			if (subColor)
			{
				MoveTo(250, -6);
				DrawString("\pSubstitued color 0x00");
			}

			// Add the frame to the QuickTime movie.

			result = SCCompressSequenceFrame(ci, ((CGrafPort *)(gMainWindow))->portPixMap, 
								 				&movieRect, &compHdl, &compSize, &notSyncFlag);
			CheckError(result, "\pCan't compress a frame.");

//			sampTime = gSampleTimes[f];
			sampTime = gSampleTimes[f] * 1.3333;
			result = AddMediaSample(gMedia, compHdl, 0L, compSize, sampTime, 
									(SampleDescriptionHandle)imageDescriptionH, 1L, notSyncFlag, 0L);
			CheckError(result, "\pCan't add the frame sample.");

		}
	}
	ShowCursor();
	
//	CDSequenceEnd(seq);
	SCCompressSequenceEnd(ci);
	EndMediaEdits( gMedia );				

	result = InsertMediaIntoTrack(gTrack,0L,0L,GetMediaDuration(gMedia),1L<<16);
	CheckError(result, "\pCan't insert media into track.");
	
	// Finally, we're done with the movie.
	result = AddMovieResource(gMovie, resRefNum, 0L,0L);
	CheckError(result, "\pCan't add the movie resource.");
	
	// Close the movie file.
	CloseMovieFile( resRefNum );

	if ( gMovie ) 
		DisposeMovie(gMovie);	

	fclose(fp);
	CloseComponent(ci);
 	ExitMovies();

	gr_clear(0xFF);
	CleanupAndExit();
}

//------------------------------------------------------------------------
//  Exit in case of an error.
//------------------------------------------------------------------------
void	 CheckError(OSErr error, Str255 displayString)
{
	if (error == noErr)
		return;
	ParamText(displayString, "\p", "\p", "\p");
	StopAlert(1000, nil);
	if (ci) CloseComponent(ci);
 	ExitMovies();
	CleanupAndExit();
}



//	===============================================================
//	QuickTime file dumping routines.
//	===============================================================

//	--------------------------------------------------------------
//
//	QuikReadChunkHdr() reads in the next chunk header, returns TRUE if ok.

//bool QuikReadChunkHdr(FILE *fp, QT_ChunkHdr *phdr)
bool QuikReadChunkHdr(Ptr &p, QT_ChunkHdr *phdr)
{
//	fread(phdr, sizeof(QT_ChunkHdr), 1, fp);
	BlockMove(p, phdr, sizeof(QT_ChunkHdr));
	p += sizeof(QT_ChunkHdr);
	
	switch (phdr->ctype)
	{
		case QT_TRAK:
			currTrackType = TRACK_OTHER;
			break;

		case QT_VMHD:
			currTrackType = TRACK_VIDEO;
			break;

		case QT_SMHD:
			currTrackType = TRACK_AUDIO;
			break;
	}

//	return(feof(fp) == 0);
	return(TRUE);
}

//	--------------------------------------------------------------
//
//	QuikFindChunkInfo() finds info for a chunk.

QT_ChunkInfo *QuikFindChunkInfo(QT_ChunkHdr *phdr)
{
static QT_Ctype lastType = 0;
static QT_ChunkInfo *lastInfoPtr = NULL;

	QT_ChunkInfo *pinfo;

	if (lastType == phdr->ctype)
		return((QT_ChunkInfo *) lastInfoPtr);

	pinfo = chunkInfo;
	while (pinfo->ctype)
		{
		if (pinfo->ctype == phdr->ctype)
			{
			lastType = phdr->ctype;
			lastInfoPtr = pinfo;
			return((QT_ChunkInfo *) pinfo);
			}
		++pinfo;
		}
	return NULL;
}

//	--------------------------------------------------------------
//
//	QuikSkipChunk() skips over the data in the current chunk.

//void QuikSkipChunk(FILE *fp, QT_ChunkHdr *phdr)
void QuikSkipChunk(Ptr &p, QT_ChunkHdr *phdr)
{
//	fseek(fp, phdr->length - sizeof(QT_ChunkHdr), SEEK_CUR);
	p += phdr->length - sizeof(QT_ChunkHdr);
}
