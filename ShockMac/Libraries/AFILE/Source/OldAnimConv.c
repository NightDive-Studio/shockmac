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
#include "lg.h"
#include "rect.h"
#include "res.h"
#include "2d.h"
#include "InitMac.h"
#include "ShockBitmap.h"

#include <Movies.h>
#include <ImageCompression.h>
#include <QuickTimeComponents.h>

WindowPtr	gMainWindow;
short		gResNum;

void LoadShockPalette(void);
void LoadAnimRes(void);

typedef struct {
   grs_bitmap bm;       // embedded bitmap, bm.bits set to NULL
   union {
      LGRect updateArea;  // update area (for anims)
      LGRect anchorArea;  // area to anchor sub-bitmap
      LGPoint anchorPt;   // point to anchor from
      } u;
   long pallOff;        // offset to pallette
                        // bitmap's bits follow immediately
} FrameDesc;

/*
// 2624 - Shields On
TimeValue	ts[24] = {	100, 100, 100, 100, 100,
						400,
						1100,
						300,
						100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,100,
						400  };

// 2625 - Jettison Pod
TimeValue	ts[] = {	114, 114, 114, 114, 114,
						228, 228, 228, 228, 228, 228,
						114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,114,
						228, 228,
						114  };

// 2626 - Detach
TimeValue	ts[] = {	100, 100, 100, 100, 100,
					200,
					100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 
					100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100
					  };

// 2627 - Laser Malfunction 1
TimeValue	ts[] = {	142, 142, 142, 142, 142,
					284,
					994, 
					142, 142, 142, 142, 142, 142, 142, 142, 142, 142
					  };

// 2628 - Laser Malfunction 2
TimeValue	ts[] = {	142, 142, 142, 142, 142,142, 142, 142, 142, 142,142, 142,
					284,284,284,
					568 
					  };

// 2629 - Auto-destruct 1
TimeValue	ts[] = {	114, 114, 114, 114, 114, 
					228, 228, 228,
					114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114
					};

// 2630-2632 - Auto-destruct 2-4
TimeValue	ts[] = {	114, 114, 114, 114, 114 };
*/
// 2633 - Auto-destruct 5
TimeValue	ts[] = {	114, 114, 114 };
/*
// 2634 - Intro
TimeValue	ts[] = {	71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 
					71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 71, 
					142,
					71, 71, 71, 
					639 };

// 2635 - Citadel Status
TimeValue	ts[] = {	114, 114, 114, 114, 114, 
					342,
					114, 114, 114, 114, 114, 114, 114, 114, 114, 114,
					114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 
					114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114, 114,
					};
*/

//	---------------------------------------------------------------
//		MAIN PROGRAM
//	---------------------------------------------------------------

void main(void)
{
	grs_screen 	*screen;
	grs_bitmap  unpackBM;
	FrameDesc	*srcBM;
	RefTable	*pRefTbl;
	Ref			pRefId;
	Ptr			p;
	long			stupid;
	
	OSErr			err, result;
	Point			dlgPos = {150,120};
	SFReply			sfr;
	FSSpec			mySpec;
	Str255			name = "\pQT Movie";
	Rect			movieRect;
	ImageDescriptionHandle	imageDescriptionH = 0L;		/* Contains info about the sample	*/
	ImageSequence	seq;
	short			resRefNum;
	Movie			gMovie = 0;							/* Our movie, track and media */
	Track			gTrack;
	Media			gMedia;
	long 			maxCompressedFrameSize;				/* Max size of compressed frame		*/
	long				compressedFrameSize;					/* Size of current compressed frame */
	Handle			compressedFrameBitsH = 0L;		/* Buffer for the compressed data	*/

//	Init graphics system

	InitMac();
	CheckConfig();

	SetupWindows(&gMainWindow);								// setup everything
	SetupOffscreenBitmaps();			

	gr_init();
	gr_set_mode (GRM_640x480x8, TRUE);
	screen = gr_alloc_screen (grd_cap->w, grd_cap->h);
	gr_set_screen (screen);
	gr_set_unpack_buf((uchar *)NewPtr(640 * 480));
	
	LoadShockPalette();
	LoadAnimRes();
	
	// Setup unpacking bitmap.
//	gr_init_bm (&unpackBM, (uchar *)malloc(640 * 480), BMT_FLAT8, BMT_TRANS, 640, 480);
	
	//---------------------
	// Setup Quicktime stuff.
	//---------------------
	if (EnterMovies() != noErr)	// Start up the movie tools
	{
		ParamText("\pCan't startup QuickTime.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		CleanupAndExit();
	}

	// get name & location for movie file
	SFPutFile(dlgPos, "\pSave Movie as:",name,0L,&sfr);
	if (!sfr.good)
	 {
	 	ExitMovies();
		CleanupAndExit();
 	 }
 	 
 	SetRect(&movieRect,0,0,400,200);
/*
	imageDescriptionH = (ImageDescriptionHandle)NewHandle(4);
	if (imageDescriptionH == nil)
	 {
		ParamText("\pCan't alloc image description.", "\p", "\p", "\p");
		StopAlert(1000, nil);
	 	ExitMovies();
		CleanupAndExit();
	 }
*/
	ClearMoviesStickyError();
	FSMakeFSSpec(sfr.vRefNum, 0, sfr.fName, &mySpec);
	err = CreateMovieFile(&mySpec, 'TVOD', 0, createMovieFileDeleteCurFile, &resRefNum, &gMovie);
 	if (err!=noErr)
 	 {
		ParamText("\pCan't create movie file.", "\p", "\p", "\p");
		StopAlert(1000, nil);
 	 	ExitMovies();
		CleanupAndExit();
 	 }

	gTrack = NewMovieTrack(gMovie, 400L<<16, 200L<<16, 0);
	gMedia = NewTrackMedia(gTrack, VideoMediaType, 1000, 0L, 0L);
	BeginMediaEdits(gMedia);		// We do this since we are adding samples to the media
	
	//-----------------------------------
	//  Setup the standard compression component stuff.
	//-----------------------------------
	ComponentInstance	ci;
	ci = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubType);
	if (!ci)
 	 {
		ParamText("\pCan't open the Standard Compression component.", "\p", "\p", "\p");
		StopAlert(1000, nil);
 	 	ExitMovies();
		CleanupAndExit();
 	 }
/*
	GetMaxCompressionSize(((CGrafPort *)(gMainWindow))->portPixMap,
							&movieRect,
							8, codecHighQuality, 'smc ', anyCodec,
							&maxCompressedFrameSize);	

	compressedFrameBitsH = NewHandle(maxCompressedFrameSize);	
	if (compressedFrameBitsH == nil)
	 {
		ParamText("\pCan't allocate frame buffer.", "\p", "\p", "\p");
		StopAlert(1000, nil);
 	 	ExitMovies();
		CleanupAndExit();
	 }
	 MoveHHi(compressedFrameBitsH);
	 HLock(compressedFrameBitsH);

	if ((result = CompressSequenceBegin(&seq, ((CGrafPort *)(gMainWindow))->portPixMap,0L,
			&movieRect, &movieRect, 8, 'smc ', anyCodec,
			codecHighQuality, codecHighQuality, 10,
			0L, codecFlagUpdatePreviousComp, imageDescriptionH)) != 0 )
	 {
		ParamText("\pCan't start sequence.", "\p", "\p", "\p");
		StopAlert(1000, nil);
 	 	ExitMovies();
		CleanupAndExit();
	 }
*/
	
	//---------------------
	// Convert the movies.
	//---------------------
	p = NewPtrClear(320 * 200);
	
//	for (short r = 2624; r <= 2635; r++)
//	{
		gr_clear(0xFF);
//		Delay(20, &stupid);
	
		// Load the initial image from the compound resource.
		short r = 2633;
		pRefId = MKREF(r, 0);	
	 	pRefTbl = ResReadRefTable(REFID(pRefId));
	 	
	 	for (short i = 0; i < pRefTbl->numRefs; i++)
	 	{
	 		Handle	compHdl;
	 		long		compSize;
	 		short	notSyncFlag;
	 		
			pRefId = MKREF(r, i);	
			RefExtract(pRefTbl, pRefId, p);
		
			srcBM = (FrameDesc *)p;
			srcBM->bm.bits = (uchar *)(srcBM+1);
			gr_rsd8_convert((grs_bitmap *)srcBM, &unpackBM);
			unpackBM.flags = BMF_TRANS;
//		  	gr_bitmap (&unpackBM, 0, 0);
			gr_scale_ubitmap(&unpackBM, 0,0,400,200);
		
//			while (!Button());
//			Delay(10, &stupid);

			// The first time through the loop (after displaying the first image), set the compression 
			// parameters for the output movie and begin a compression sequence.
			if (i == 0)
			{
				result = SCDefaultPixMapSettings(ci, ((CGrafPort *)(gMainWindow))->portPixMap, TRUE);
			 	result = SCRequestSequenceSettings(ci);
			 	if (result == scUserCancelled)
			 	{
					CloseComponent(ci);
			 	 	ExitMovies();
					CleanupAndExit();
			 	}
				if (result < 0)
			 	{
					ParamText("\pError in sequence settings.", "\p", "\p", "\p");
					StopAlert(1000, nil);
					CloseComponent(ci);
			 	 	ExitMovies();
					CleanupAndExit();
			 	}
				
				// Redraw the first frame on the screen.
				gr_clear(0xFF);
				gr_scale_ubitmap(&unpackBM, 0,0,400,200);
				
				// Begin a compression sequence.
				result = SCCompressSequenceBegin(ci, ((CGrafPort *)(gMainWindow))->portPixMap,
												  	&movieRect, &imageDescriptionH);
				if (result)
				 {
					ParamText("\pCan't start a sequence.", "\p", "\p", "\p");
					StopAlert(1000, nil);
					CloseComponent(ci);
			 	 	ExitMovies();
					CleanupAndExit();
				 }
			}

			// Compress this frame.
			result = SCCompressSequenceFrame(ci, ((CGrafPort *)(gMainWindow))->portPixMap, 
								 				&movieRect, &compHdl, &compSize, &notSyncFlag);
			if (result)
			 {
				ParamText("\pCan't compress a frame.", "\p", "\p", "\p");
				StopAlert(1000, nil);
				CloseComponent(ci);
		 	 	ExitMovies();
				CleanupAndExit();
			 }
/*
			// Add the frame to the QuickTime movie.

			result = CompressSequenceFrame(seq, 
				((CGrafPort *)(gMainWindow))->portPixMap, 
				&movieRect,
				codecFlagUpdatePreviousComp, *compressedFrameBitsH, &compressedFrameSize, 0L, 0L);
			if (result)
			 {
				ParamText("\pCan't compress a frame.", "\p", "\p", "\p");
				StopAlert(1000, nil);
		 	 	ExitMovies();
				CleanupAndExit();
			 }
*/	
			result = AddMediaSample(gMedia, compHdl, 0L, compSize,
									ts[i], (SampleDescriptionHandle)imageDescriptionH,1L,
									notSyncFlag, 0L);
			if (result)
			 {
				ParamText("\pCan't add the frame sample.", "\p", "\p", "\p");
				StopAlert(1000, nil);
				CloseComponent(ci);
		 	 	ExitMovies();
				CleanupAndExit();
			 }
		}
		  
//		ResFreeRefTable(pRefTbl);					// Free the existing refTable
//	}
	
//	CDSequenceEnd(seq);
	SCCompressSequenceEnd(ci);
	EndMediaEdits( gMedia );				

	result = InsertMediaIntoTrack(gTrack,0L,0L,GetMediaDuration(gMedia),1L<<16);
	if ( result )
	 {
		ParamText("\pCan't insert media into track.", "\p", "\p", "\p");
		StopAlert(1000, nil);
 	 	ExitMovies();
		CleanupAndExit();
	 }
		
	result = AddMovieResource(gMovie, resRefNum, 0L,0L);
	if ( result )
	 {
		ParamText("\pCan't add the movie resource.", "\p", "\p", "\p");
		StopAlert(1000, nil);
 	 	ExitMovies();
		CleanupAndExit();
	 }

	CloseMovieFile( resRefNum );

//	if (imageDescriptionH)
//		DisposeHandle((Handle)imageDescriptionH);
		
//	if (compressedFrameBitsH)
//	{
//		HUnlock(compressedFrameBitsH);
//		DisposeHandle(compressedFrameBitsH);
//	}

	if ( gMovie ) 
		DisposeMovie(gMovie);	

	CloseComponent(ci);
 	ExitMovies();

	gr_clear(0xFF);
	ResCloseFile(gResNum);
	CleanupAndExit();
}

//------------------------------------------------------------------------
//  Open a file containing palette resources.
//------------------------------------------------------------------------
void LoadShockPalette(void)
{
	StandardFileReply	reply;
	SFTypeList			typeList;
	Ptr					p = NULL;
	short				resNum;

	typeList[0] = 'Sres';
	StandardGetFile(nil, 1, typeList, &reply);
	if (!reply.sfGood)
		return;

	resNum = ResOpenFile(&reply.sfFile);
	if (resNum < 0)
	{
		ParamText("\pCan't open res file.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		return;
	}
	
	{
		Id 		id = 702;
		long		rfs;
		ResLock(id);
		rfs = ResSize(id);
		p = NewPtrClear(rfs);
		ResExtract(id, p);
		ResUnlock(id);
	}
	if (p == NULL)
	{
		ParamText("\pCouldn't get a palette resource.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		return;
	}
	
	
 	gr_set_pal(0, 256, (uchar *)p);
	DisposePtr(p);
	
	ResCloseFile(resNum);
}

//------------------------------------------------------------------------
//  Open a file containing palette resources.
//------------------------------------------------------------------------
void LoadAnimRes(void)
{
	StandardFileReply	reply;
	SFTypeList			typeList;
	Ptr					p = NULL;
	short				resNum;

	typeList[0] = 'Sres';
	StandardGetFile(nil, 1, typeList, &reply);
	if (!reply.sfGood)
		return;

	gResNum = ResOpenFile(&reply.sfFile);
	if (gResNum < 0)
	{
		ParamText("\pCan't open res file.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		return;
	}
}
