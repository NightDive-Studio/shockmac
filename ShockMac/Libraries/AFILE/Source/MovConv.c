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
//		Convert a raw QT movie (from the LG .MOV file) into a nice compressed movie.
//		Also, add the sound track.
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

typedef struct
{
	long 	frameNum;
	short	palID;
} PalChange;

//--------------------------
//  Globals the user should set
//--------------------------
//#define ADD_TEXT_TRACK	1
//#define INSERT_BLANK_PALCHG_FRAME	1

char		gInputMov[] = "INTRO.QTM";
FSSpec		gInputPal = { 0, 0, "\pIntro Palettes" };
FSSpec		gInputSnd = { 0, 0, "\pINTRO/GERMAN" };
ComponentInstance	ci = nil;

#define codec		'smc '
#define spatialQ		codecHighQuality
#define temporalQ	codecHighQuality
#define codecType	bestCompressionCodec
#define kPrevious	codecFlagUpdatePreviousComp

//  Globals
WindowPtr		gMainWindow;
extern short		gMainVRef;
short			gResNum;
Handle			gPalHdl;
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

bool QuikReadChunkHdr(FILE *fp, QT_ChunkHdr *phdr);
QT_ChunkInfo *QuikFindChunkInfo(QT_ChunkHdr *phdr);
void QuikSkipChunk(FILE *fp, QT_ChunkHdr *phdr);

void CreateMySoundTrack(Movie theMovie);
void CreateSoundDescription(Handle sndHandle, SoundDescriptionHandle	sndDesc,
							long *sndDataOffset, long *numSamples, long *sndDataSize);
long GetSndHdrOffset(Handle sndHandle);

#ifdef ADD_TEXT_TRACK
void MyCreateTextTrack(Movie theMovie);
#endif

//	---------------------------------------------------------------
//		MAIN PROGRAM
//	---------------------------------------------------------------

void main(void)
{
	grs_screen 		*screen;
	Ptr				p;
	long				stupid;
	OSErr			err, result;
	
	FILE 			*fp;
	uchar 			*dbuff;
	ulong 			dbuffLen;
	QT_ChunkHdr 	chunkHdr;
	QT_ChunkInfo 	*pinfo;

	short			palResNum, sndResNum;
	Handle			palChgHdl;
	PalChange		*pcp;
	
	Handle			qpalHdl;								// Contains array of palette change times.
	long				*qpalEntry;							// Pointer to current change.
	short			numQpalEntries = 0;
	ulong			qpalCurrTime = 0;
	
	Point			dlgPos = {120,120};
	SFReply			sfr;
	FSSpec			mySpec;
	Str255			name = "\pQT Movie";
	Rect			movieRect, r;
	ImageDescription	**imageDescriptionH = 0L;		/* Contains info about the sample	*/
	ImageSequence	seq;
	short			resRefNum;
	Movie			gMovie = 0;							/* Our movie, track and media */
	Track			gTrack;
	Media			gMedia;
	long 			maxCompressedFrameSize;			/* Max size of compressed frame		*/
	long				compressedFrameSize;				/* Size of current compressed frame */
	Handle			compressedFrameBitsH = 0L;			/* Buffer for the compressed data	*/
#ifdef INSERT_BLANK_PALCHG_FRAME
	Boolean 			lastInSeq = FALSE;
#endif

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
 	SetRect(&movieRect,0,0,600,300);
	
	//---------------------
	//	Setup the input FileSpecs
	//---------------------
	SetInputSpecs();
	
	//---------------------
	// Setup Quicktime stuff.
	//---------------------
	if (EnterMovies() != noErr)	// Start up the movie tools
	{
		ParamText("\pCan't startup QuickTime.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		CleanupAndExit();
	}
	
	//----------------------
	//	Open the input QuickTime movie.
	//----------------------
	fp = fopen(gInputMov, "rb");
	if (fp == NULL)
		CheckError(1, "\pCan't open the input movie!!");

	dbuffLen = 64000;
	dbuff = (uchar *)malloc(dbuffLen);

	//----------------------
	//	Open the input Sound file.
	//----------------------
	sndResNum = FSpOpenResFile(&gInputSnd, fsRdPerm);
	if (sndResNum == -1)
		CheckError(1, "\pCan't open the sound file!!");

	//----------------------
	//	Open the input Palettes file, read the palette changes, and set the first palette.
	//----------------------
	palResNum = FSpOpenResFile(&gInputPal, fsRdPerm);
	if (palResNum == -1)
		CheckError(1, "\pCan't open the palette file!!");
	palChgHdl = GetResource('pchg', 128);
	if (!palChgHdl)
		CheckError(1, "\pCan't load the palette changes resource!!");
	SetPalette(128);

	//----------------------
	//	Setup a handle to hold the palette change times (16 max).
	//----------------------
	qpalHdl = NewHandle(4 * 16);
	if (!qpalHdl)
		CheckError(1, "\pCan't allocate palette change times handle!!");
	HLock(qpalHdl);
	qpalEntry = (long *)*qpalHdl;
	
	*qpalEntry = 0;						// First palette change at 0.
	qpalEntry++;
	numQpalEntries++;
	
	//----------------------
	//	Setup output file.
	//----------------------
	SFPutFile(dlgPos, "\pSave Movie as:",name,0L,&sfr);
	if (!sfr.good)
	 {
	 	ExitMovies();
		CleanupAndExit();
 	 }
 	 
//	imageDescriptionH = (ImageDescription **)NewHandle(sizeof(ImageDescription));
//	CheckError(MemError(), "\pCan't alloc description for video.");

	ClearMoviesStickyError();
	FSMakeFSSpec(sfr.vRefNum, 0, sfr.fName, &mySpec);
	err = CreateMovieFile(&mySpec, 'TVOD', 0, createMovieFileDeleteCurFile, &resRefNum, &gMovie);
	CheckError(err, "\pCan't create output movie file.");

	SetMovieColorTable(gMovie, gMainColorHand);

#ifdef ADD_TEXT_TRACK
	// Add the text track here.	
	MyCreateTextTrack(gMovie);
#endif

	gTrack = NewMovieTrack(gMovie, 600L<<16, 300L<<16, 0);
	CheckError (GetMoviesError(), "\pNew video track." );

	gMedia = NewTrackMedia(gTrack, VideoMediaType, 30, 0L, 0L);
	CheckError (GetMoviesError(), "\pNew Media for video track." );

	BeginMediaEdits(gMedia);		// We do this since we are adding samples to the media

	//-----------------------------------
	//  Setup the standard compression component stuff.
	//-----------------------------------
	ci = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubType);
	if (!ci)
		CheckError (-1, "\pCan't open the Standard Compression component." );
/*
	GetMaxCompressionSize(((CGrafPort *)(gMainWindow))->portPixMap,
							&movieRect,
							8, spatialQ, codec, codecType,
							&maxCompressedFrameSize);	

	compressedFrameBitsH = NewHandle(maxCompressedFrameSize);	
	CheckError(MemError(), "\pCan't allocate output frame buffer.");

	result = CompressSequenceBegin(&seq, ((CGrafPort *)(gMainWindow))->portPixMap, 0L,
			&movieRect, 0L, 8, codec, codecType,
			spatialQ, temporalQ, 15,
			0L, kPrevious, imageDescriptionH);
	CheckError(result, "\pCan't begin sequence.");
	result = SetImageDescriptionCTable(imageDescriptionH, gMainColorHand);
	CheckError(result, "\pCan't set the sequence's color table.");
*/	
	//-----------------------------------
	//	Get the Chunk Offset and Sample-to-Time tables.
	//-----------------------------------
	while (TRUE)
	{
		if (!QuikReadChunkHdr(fp, &chunkHdr))
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
				fread(dbuff, chunkHdr.length - sizeof(QT_ChunkHdr), 1, fp);

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
				QuikSkipChunk(fp, &chunkHdr);
		}
	}

	HideCursor();
	
	//----------------------------
	//	Show the movie, one frame at a time.
	//----------------------------
	{
		long			f, line;
		Ptr			frameBuff;
		uchar		*pp;
		Ptr			imgp, scrp;
		RGBColor	black = {0, 0, 0};
		RGBColor	white = {0xffff, 0xffff, 0xffff};
		char		buff[64];
		PalChange	*pc;
		ulong		sampTime;
		Boolean		subColor;
 		Handle		compHdl;
 		long			compSize;
 		short		notSyncFlag;
		
		frameBuff = NewPtr(600 * 300);
		CheckError(MemError(), "\pCan't allocate a frame buffer for input movie.");
		
		SetRect(&r, 0, -17, 300, 0);
		for (f = 0; f < gNumFrames; f++)
		{
			// Read the next frame from the input movie.
			fseek(fp, gChunkOffsets[f], SEEK_SET);
			fread(frameBuff, 600 * 300, 1, fp);
			
			// See if there's an 0xFF anywhere in this screen.
			subColor = FALSE;
			pp = (uchar *)frameBuff;
			for (int pix = 0; pix < 600*300; pix++)
			{
				if (*pp == 0xFF)
				{
					*pp = (uchar)gFakeIndex;
					subColor = TRUE;
				}
				pp++;
			}
			
			// See if the palette needs to change starting at this frame.
			HLock(palChgHdl);
			pc = (PalChange *)*palChgHdl;
			while (pc->frameNum)
			{
#ifdef INSERT_BLANK_PALCHG_FRAME
				if (pc->frameNum-1 == f)				// If this is the last frame before a palette
				{										// change, then reduce the time for this
					gSampleTimes[f]--;					// frame by one (to allow for blank frame)
					lastInSeq = TRUE;					// we insert afterwards.
					break;
				}
				else
#endif
				if (pc->frameNum == f)				// If this is the frame to change palette on, then
				{
//					CDSequenceEnd(seq);					// end the current output image sequence,
					SCCompressSequenceEnd(ci);
				
					SetPalette(pc->palID);				// set the palette for the screen,
					
					*qpalEntry = (qpalCurrTime-1) * 20;	// record the time of the palette change
					qpalEntry++;							// (which is (time * 600)/30, or *20)
					numQpalEntries++;
														// begin a new output image sequence and set
														// its color table to the new palette.
/*					result = CompressSequenceBegin(&seq, ((CGrafPort *)(gMainWindow))->portPixMap, 0L,
							&movieRect, 0L, 8, codec, codecType,
							spatialQ, temporalQ, 15,
							0L, kPrevious, imageDescriptionH);
					CheckError(result, "\pCan't begin sequence.");   */
					result = SCCompressSequenceBegin(ci, ((CGrafPort *)(gMainWindow))->portPixMap,
													  	&movieRect, &imageDescriptionH);
					CheckError(result, "\pCan't start a sequence.");
					result = SetImageDescriptionCTable(imageDescriptionH, gMainColorHand);
					CheckError(result, "\pCan't set the sequence's color table.");
					break;
				}
				pc++;
			}
			HUnlock(palChgHdl);
				
			imgp = frameBuff;
			scrp = gScreenAddress;
			for (line = 0; line < 300; line++)
			{
				BlockMove(imgp, scrp, 600);
				imgp += 600;
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
				for (line = 0; line < 300; line++)
				{
					BlockMove(imgp, scrp, 600);
					imgp += 600;
					scrp += gScreenRowbytes;
				}
				
				// Begin a compression sequence.
				result = SCCompressSequenceBegin(ci, ((CGrafPort *)(gMainWindow))->portPixMap,
												  	&movieRect, &imageDescriptionH);
				CheckError(result, "\pCan't start a sequence.");
			}
			
			// Display the frame number.
			
			sprintf(buff, "Frame: %d   Sequence:%d", f, numQpalEntries);
			RGBForeColor(&black);
			PaintRect (&r);
			MoveTo(1, -6);
			RGBForeColor(&white);
			DrawText(buff, 0, strlen(buff));
			if (subColor)
			{
				MoveTo(250, -6);
				DrawString("\pSubstitued color 0xFF");
			}

			// Add the frame to the QuickTime movie.
/*
			HLock(compressedFrameBitsH);
			result = CompressSequenceFrame(seq, 
				((CGrafPort *)(gMainWindow))->portPixMap, 
				&movieRect,
				kPrevious, *compressedFrameBitsH, &compressedFrameSize, 0L, 0L);
			CheckError(result, "\pCan't compress a frame.");
			HUnlock(compressedFrameBitsH);
*/	
			result = SCCompressSequenceFrame(ci, ((CGrafPort *)(gMainWindow))->portPixMap, 
								 				&movieRect, &compHdl, &compSize, &notSyncFlag);
			CheckError(result, "\pCan't compress a frame.");

			sampTime = gSampleTimes[f];
			if (f == gNumFrames-1)
				sampTime = 25;		// for Death
//				sampTime = 250;	// for Endgame
//				sampTime = 200;	// for Intro
			result = AddMediaSample(gMedia, compHdl, 0L, compSize, sampTime, 
									(SampleDescriptionHandle)imageDescriptionH, 1L, notSyncFlag, 0L);
			CheckError(result, "\pCan't add the frame sample.");

			qpalCurrTime += gSampleTimes[f];			// Adjust cumulate time for frame just added.

#ifdef INSERT_BLANK_PALCHG_FRAME
			if (lastInSeq)
			{
				RGBForeColor(&black);
				PaintRect(&movieRect);				// Add a blank frame to the movie.
/*				HLock(compressedFrameBitsH);
				result = CompressSequenceFrame(seq, 
					((CGrafPort *)(gMainWindow))->portPixMap, 
					&movieRect,
					kPrevious, *compressedFrameBitsH, &compressedFrameSize, 0L, 0L);
				CheckError(result, "\pCan't compress a frame.");
				HUnlock(compressedFrameBitsH);   */
				result = SCCompressSequenceFrame(ci, ((CGrafPort *)(gMainWindow))->portPixMap, 
									 				&movieRect, &compHdl, &compSize, &notSyncFlag);
				CheckError(result, "\pCan't compress the blank frame.");
		
/*				result = AddMediaSample(gMedia, compressedFrameBitsH, 0L, compressedFrameSize,
										1, (SampleDescriptionHandle)imageDescriptionH,1L,0, 0L);
				CheckError(result, "\pCan't add the frame sample.");    */
				result = AddMediaSample(gMedia, compHdl, 0L, compSize, 1, 
										(SampleDescriptionHandle)imageDescriptionH, 1L, notSyncFlag, 0L);
				CheckError(result, "\pCan't add the blank frame sample.");
				
				qpalCurrTime++;					// Adjust the cumulative time.
				
				lastInSeq = FALSE;
			}
#endif
		}
	}
	ShowCursor();
	
//	CDSequenceEnd(seq);
	SCCompressSequenceEnd(ci);
	EndMediaEdits( gMedia );				

	result = InsertMediaIntoTrack(gTrack,0L,0L,GetMediaDuration(gMedia),1L<<16);
	CheckError(result, "\pCan't insert media into track.");
	
	// Add the sound track here.	
	CreateMySoundTrack(gMovie);
	
	// Finally, we're done with the movie.
	result = AddMovieResource(gMovie, resRefNum, 0L,0L);
	CheckError(result, "\pCan't add the movie resource.");

	// Add the palette change times resource to the movie file.
	HUnlock(qpalHdl);
	SetHandleSize(qpalHdl, 4 * numQpalEntries);

	UseResFile(resRefNum);
	AddResource(qpalHdl, 'qpal', 128, "\ppal chg times");
	WriteResource(qpalHdl);
	ReleaseResource(qpalHdl);
	
	// Close the movie file.
	CloseMovieFile( resRefNum );

//	if (imageDescriptionH)
//		DisposeHandle((Handle)imageDescriptionH);
		
//	if (compressedFrameBitsH)
//		DisposeHandle(compressedFrameBitsH);

	if ( gMovie ) 
		DisposeMovie(gMovie);	

	CloseResFile(palResNum);
	CloseResFile(sndResNum);
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

//------------------------------------------------------------------------
//  Setup the input file specs.
//------------------------------------------------------------------------
void SetInputSpecs(void)
{
	short	vRefNum;
	long		temp, parID;

 	GetWDInfo(gMainVRef, &vRefNum, &parID, &temp);
 	
	gInputPal.vRefNum = vRefNum;
	gInputPal.parID = parID;

	gInputSnd.vRefNum = vRefNum;
	gInputSnd.parID = parID;
}

//------------------------------------------------------------------------
//  Load in the 'mpal' resource and set it.
//------------------------------------------------------------------------
void SetPalette(short palID)
{
	RGBColor	fakecol;
	
	gPalHdl = GetResource('mpal', palID);
	if (!gPalHdl)
	{
		ParamText("\pCan't load a palette resource!!", "\p", "\p", "\p");
		StopAlert(1000, nil);
	 	ExitMovies();
		CleanupAndExit();
	}
	gr_clear(0xFF);
	HLock(gPalHdl);
	gr_set_pal(0, 256, (uchar *)*gPalHdl);
	
	fakecol.red = *(*gPalHdl + 765) << 8;
	fakecol.green = *(*gPalHdl + 766) << 8;
	fakecol.blue = *(*gPalHdl + 767) << 8;
	gFakeIndex = Color2Index(&fakecol);
	
	HUnlock(gPalHdl);
}



//	===============================================================
//	QuickTime file dumping routines.
//	===============================================================

//	--------------------------------------------------------------
//
//	QuikReadChunkHdr() reads in the next chunk header, returns TRUE if ok.

bool QuikReadChunkHdr(FILE *fp, QT_ChunkHdr *phdr)
{
	fread(phdr, sizeof(QT_ChunkHdr), 1, fp);

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

	return(feof(fp) == 0);
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

void QuikSkipChunk(FILE *fp, QT_ChunkHdr *phdr)
{
	fseek(fp, phdr->length - sizeof(QT_ChunkHdr), SEEK_CUR);
}



//================================================================
//	QuickTime Sound Track routines.
//================================================================

#define	kSoundSampleDuration  	1
#define	kSyncSample 			0
#define	kTrackStart				0
#define	kMediaStart				0
#define	kFix1					0x00010000

//----------------------------------------------------------------
void CreateMySoundTrack(Movie theMovie)
{
	Track 					theTrack;
	Media 					theMedia;
	Handle					sndHandle = nil;
	SoundDescriptionHandle	sndDesc = nil;
	long 					sndDataOffset;
	long 					sndDataSize;
	long 					numSamples;
	OSErr					err = noErr;

	sndHandle = GetIndResource ('snd ', 1);
	CheckError (ResError(), "\pGetResource 'snd '" );
	if (sndHandle == nil) return;

	sndDesc = (SoundDescriptionHandle)NewHandle(4);
	CheckError (MemError(), "\pNewHandle for SoundDesc" );
	
	CreateSoundDescription (sndHandle, sndDesc, &sndDataOffset, &numSamples, &sndDataSize );
	
	theTrack = NewMovieTrack (theMovie, 0, 0, kFullVolume);
	CheckError (GetMoviesError(), "\pNew Sound Track" );
	
	theMedia = NewTrackMedia (theTrack, SoundMediaType, FixRound ((**sndDesc).sampleRate), nil, 0);
	CheckError (GetMoviesError(), "\pNew Media snd." );

	err = BeginMediaEdits (theMedia);
	CheckError( err, "\pBeginMediaEdits snd." );

	err = AddMediaSample(theMedia, sndHandle, sndDataOffset, sndDataSize,	1,
						   (SampleDescriptionHandle) sndDesc, numSamples, 0, nil);
	CheckError( err, "\pAddMediaSample snd." );
					
	err = EndMediaEdits (theMedia);
	CheckError( err, "\pEndMediaEdits snd." );

	err = InsertMediaIntoTrack (theTrack, 0, 0, GetMediaDuration (theMedia), kFix1);	
	CheckError( err, "\pInsertMediaIntoTrack snd." );

	if (sndDesc != nil) DisposeHandle( (Handle)sndDesc);
}

//----------------------------------------------------------------
void CreateSoundDescription(Handle sndHandle, SoundDescriptionHandle	sndDesc,
							long *sndDataOffset, long *numSamples, long *sndDataSize)
{
	long					sndHdrOffset = 0;
	long					sampleDataOffset;
	SoundHeaderPtr 		sndHdrPtr = nil;
	long					numFrames;
	long					samplesPerFrame;
	long					bytesPerFrame;
	SignedByte			sndHState;
	SoundDescriptionPtr	sndDescPtr;
	
	*sndDataOffset = 0;
	*numSamples = 0;
	*sndDataSize = 0;
	
	SetHandleSize( (Handle)sndDesc, sizeof(SoundDescription) );
	CheckError(MemError(),"\pSetHandleSize for sndDesc.");
	
	sndHdrOffset = GetSndHdrOffset (sndHandle);
	if (sndHdrOffset == 0) CheckError(-1,  "\pGetSndHdrOffset ");
	
	// we can use pointers since we don't move memory
	sndHdrPtr = (SoundHeaderPtr)(*sndHandle + sndHdrOffset);
	sndDescPtr = *sndDesc;
	sndDescPtr->descSize = sizeof (SoundDescription);			// total size of sound desc structure
	sndDescPtr->resvd1 = 0;							
	sndDescPtr->resvd2 = 0;
	sndDescPtr->dataRefIndex = 1;
	sndDescPtr->compressionID = 0;
	sndDescPtr->packetSize = 0;
	sndDescPtr->version = 0;								
	sndDescPtr->revlevel = 0;
	sndDescPtr->vendor = 0;  
	
	switch  (sndHdrPtr->encode) 
	{
		case stdSH:
			sndDescPtr->dataFormat = 'raw ';						// uncompressed offset-binary data
			sndDescPtr->numChannels = 1;						// number of channels of sound 
			sndDescPtr->sampleSize = 8;							// number of bits per sample
			sndDescPtr->sampleRate = sndHdrPtr->sampleRate;	// sample rate
			*numSamples = sndHdrPtr->length;				
			*sndDataSize = *numSamples;
			bytesPerFrame = 1;      
			samplesPerFrame = 1;
			sampleDataOffset = (Ptr)&sndHdrPtr->sampleArea - (Ptr)sndHdrPtr;
			break;			
	
		case extSH:
		{
			ExtSoundHeaderPtr	extSndHdrP = (ExtSoundHeaderPtr)sndHdrPtr;
			
			sndDescPtr->dataFormat = 'raw ';							// uncompressed offset-binary data
			sndDescPtr->numChannels = extSndHdrP->numChannels;	// number of channels of sound
			sndDescPtr->sampleSize = extSndHdrP->sampleSize;		// number of bits per sample
			sndDescPtr->sampleRate = extSndHdrP->sampleRate; 		// sample rate
			numFrames = extSndHdrP->numFrames;				
			*numSamples = numFrames;
			bytesPerFrame = extSndHdrP->numChannels * ( extSndHdrP->sampleSize / 8);
			samplesPerFrame = 1;
			*sndDataSize = numFrames * bytesPerFrame;
			sampleDataOffset = (Ptr)(&extSndHdrP->sampleArea) - (Ptr)extSndHdrP;
		}
			break;
					
		default:
			CheckError(-1, "\pCorrupt sound data or unsupported format." );
			break;
			
	}
	*sndDataOffset = sndHdrOffset + sampleDataOffset;  
}

//----------------------------------------------------------------
typedef SndCommand *SndCmdPtr;

typedef struct 
{
	short 			format;
	short 			numSynths;
} Snd1Header, *Snd1HdrPtr, **Snd1HdrHndl;

typedef struct 
{
	short 			format;
	short 			refCount;
} Snd2Header, *Snd2HdrPtr, **Snd2HdrHndl;
typedef struct 
{
	short 			synthID;
	long 			initOption;
} SynthInfo, *SynthInfoPtr;


long GetSndHdrOffset (Handle sndHandle)
{
	short	howManyCmds;
	long		sndOffset  = 0;
	Ptr		sndPtr;
	
	if (sndHandle == nil) return 0;
	sndPtr = *sndHandle;
	if (sndPtr == nil) return 0;
	
	if ((*(Snd1HdrPtr)sndPtr).format == firstSoundFormat) 
	{
		short synths = ((Snd1HdrPtr)sndPtr)->numSynths;
		sndPtr += sizeof(Snd1Header) + (sizeof(SynthInfo) * synths);
	}
	else 
	{
		sndPtr += sizeof(Snd2Header);
	}
	
	howManyCmds = *(short *)sndPtr;
	
	sndPtr += sizeof(howManyCmds);
	
	// sndPtr is now at the first sound command--cruise all
	// commands and find the first soundCmd or bufferCmd
	
	while (howManyCmds > 0) 
	{
		switch (((SndCmdPtr)sndPtr)->cmd) 
		{
			case (soundCmd + dataOffsetFlag):
			case (bufferCmd + dataOffsetFlag):
				sndOffset = ((SndCmdPtr)sndPtr)->param2;
				howManyCmds = 0;							// done, get out of loop
				break;
			default:											// catch any other type of commands
				sndPtr += sizeof(SndCommand);
				howManyCmds--;
				break;
		}
	}

	return sndOffset;
}


#ifdef ADD_TEXT_TRACK
/*	DEATH MOVIE

	// English.
	char					theText1[] = "They find your body and give it new life.";
	char					theText2[] = "As a cyborg, you will serve SHODAN well.";
	// German.
	char					theText1[] = "Sie finden deine Leiche und geben ihr neues Leben.";
	char					theText2[] = "Als Cyborg wirst du SHODAN treu dienen.";
	// French.
	char					theText1[] = "Ils ont trouvç ton corps et lui ont redonnç la vie.";
	char					theText2[] = "En tant que cyborgue, tu serviras bien SHODAN.";
	
	// Timing - 507 total
	blank	9
	text 1	150
	blank	17
	text 2	331
*/

/*	ENDGAME MOVIE

	// English.
	char					theText1[] = "It's over.";
	char					theText2[] = "They offered you a nice, boring job at Trioptimum; it never occured to you to take it.";
	char					theText3[] = "Old habits die hard.";
	// German.
	char					theText1[] = "Es ist vorbei.";
	char					theText2[] = "Sie haben dir einen netten Job bei Triop angeboten; es wÑre dir nie eingefallen, ihr Angebot anzunehmen.";
	char					theText3[] = "Du kannst es eben einfach nicht lassen.";
	// French.
	char					theText1[] = "C'est fini.";
	char					theText2[] = "Ils vous ont offert un bon boulot ennuyeux sur Triop; áa ne vous est jamais venu Ö l'idçe de l'accepter.";
	char					theText3[] = "On a du mal Ö se dçbarasser des mauvaises habitudes.";
	
	// Timing - 507 total
	blank	9
	text 1	180
	blank	17
	text 2	176
	blank	30
	text 2	1021
*/
void MyCreateTextTrack(Movie theMovie)
{
	Track 					theTrack;
	Media 					theMedia;
	OSErr					err = noErr;
	char					blankText[] = " ";
	char					theText1[] = "Ils ont trouvç ton corps et lui ont redonnç la vie.";
	char					theText2[] = "En tant que cyborgue, tu serviras bien SHODAN.";
	RGBColor				black = {0, 0, 0};
	RGBColor				white = {0xeeee, 0xeeee, 0xeeee};
	Rect					textBox;
	TimeValue				retTime;

	SetRect(&textBox, 0, 320, 600, 350);
	theTrack = NewMovieTrack(theMovie, 600L<<16, 350L<<16, 0);
	CheckError (GetMoviesError(), "\pNew text track." );

	theMedia = NewTrackMedia(theTrack, TextMediaType, 30, 0L, 0L);
	CheckError (GetMoviesError(), "\pNew Media for text track." );

	err = BeginMediaEdits (theMedia);
	CheckError( err, "\pBeginMediaEdits: text." );
	
	err = AddTextSample(GetMediaHandler(theMedia), (Ptr)blankText, strlen(blankText), geneva, 14, bold, 
						 &white, &black, teCenter, &textBox, 0, 0, 0, 0, nil, 9, &retTime);
	CheckError( err, "\pAddTextSample 1." );
	err = AddTextSample(GetMediaHandler(theMedia), (Ptr)theText1, strlen(theText1), geneva, 14, bold, 
						 &white, &black, teCenter, &textBox, 0, 0, 0, 0, nil, 150, &retTime);
	CheckError( err, "\pAddTextSample 2." );
	err = AddTextSample(GetMediaHandler(theMedia), (Ptr)blankText, strlen(blankText), geneva, 14, bold, 
						 &white, &black, teCenter, &textBox, 0, 0, 0, 0, nil, 17, &retTime);
	CheckError( err, "\pAddTextSample 3." );
	err = AddTextSample(GetMediaHandler(theMedia), (Ptr)theText2, strlen(theText2), geneva, 14, bold, 
						 &white, &black, teCenter, &textBox, 0, 0, 0, 0, nil, 331, &retTime);
	CheckError( err, "\pAddTextSample 4." );
	
	err = EndMediaEdits(theMedia);
	CheckError( err, "\pEndMediaEdits: text." );

	err = InsertMediaIntoTrack(theTrack, 0, 0, GetMediaDuration(theMedia), kFix1);	
	CheckError( err, "\pInsertMediaIntoTrack: text." );
}
#endif