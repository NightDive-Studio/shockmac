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
//====================================================================================
//
//		System Shock - й1994-1995 Looking Glass Technologies, Inc.
//
//		MoviePlay.c	-	Plays movies in Shock fashion, changing palettes on the fly.
//
//====================================================================================


//--------------------
//  Includes
//--------------------
#include <stdio.h>
#include <string.h>
#include <Movies.h>

#include "Shock.h"
#include "InitMac.h"
#include "ShockBitmap.h"
#include "MoviePlay.h"
#include "Prefs.h"


typedef struct
{
	short		language;
	long		startingTime;
	Str255	subtitle;
} SubtitleEntry;

//--------------------
//  Private Globals
//--------------------
Rect				gSubRect;							// Rectangle for showing sub-titles.
Rect 				gMovieBox;						// Rectangle of the movie box.
CTabHandle		gCTab[16];						// Color tables for the movie.
long				gChgTime[16];					// When to change colors.
long				gPalIndex;							// Which palette to change to.
Boolean			gShowSubs;
SubtitleEntry	*gCurrSub;

//--------------------
//  Internal Prototypes
//--------------------
pascal OSErr DrawDoneProc(Movie theMovie, long refCon);
void DoSubtitle(TimeValue time);
void DrawSubtitle(char *title);
void GetNextSubtitle(void);


//------------------------------------------------------------------------------------
//		Play a movie with palette changes, either cut-scenes or v-mail.
//------------------------------------------------------------------------------------
void PlayCutScene(FSSpec *movieSpec, Boolean showSubs, Boolean allowHalt)
{
	RGBColor			black = {0, 0, 0};
	RGBColor			white = {0xffff, 0xffff, 0xffff};
	OSErr				err;
	short 				movieResFile;
	long					i;
	Movie				theMovie;
	Handle				subHdl = nil;
	Handle				qpalHdl = nil;
	Track				vidTrack;
	Media				vidMedia;
	Fixed					mr;
	TimeValue			tv;
	MovieDrawingCompleteUPP	drawCompProc;
	
	// Turn off sound if soundFX is off.
	if (!gShockPrefs.soSoundFX)
		showSubs = TRUE;

	// Initialize the palette change times array and color handles array.
	for (i=0; i < 16; i++)
	{
		gChgTime[i] = -1;
		gCTab[i] = NULL;
	}
	
	// Setup for drawing subtitles.
	gSubRect = gActiveArea;
	gSubRect.top = gSubRect.bottom - 80;
	RGBForeColor(&white);
	RGBBackColor(&black);
	TextFont(geneva);
	TextSize(14);
	TextFace(bold);

	// Open the movie file and prepare it for playing.
	err = OpenMovieFile(movieSpec, &movieResFile, fsRdPerm);
	if (err == noErr) 
	{
		short 		movieResID = 0;
		Str255 		movieName;
		Boolean 		wasChanged;
																			// Load the 'moov' resource.
		err = NewMovieFromFile(&theMovie, movieResFile, &movieResID,
						movieName, newMovieActive, &wasChanged);
		
		subHdl = GetResource('subt', 128);					// Load the subtitles resource.
		if (subHdl)
		{
			DetachResource(subHdl);
			HLock(subHdl);
			gCurrSub = (SubtitleEntry *)*subHdl;
			GetNextSubtitle();
		}
		else
			showSubs = FALSE;
		gShowSubs = showSubs;
		
		qpalHdl = GetResource('qpal', 128);					// Load the palette change times resource
		if (qpalHdl)													// and copy its information out.
		{
			BlockMove(*qpalHdl, gChgTime, GetHandleSize(qpalHdl));
			ReleaseResource(qpalHdl);
		}

		CloseMovieFile (movieResFile);						// Close the resource fork.
	}
	else
	{
		return;
	}
	
	// Get a reference to the video track and media, and load all the palettes for the movie.
	long	tc = GetMovieTrackCount(theMovie);
	for (long t = 1; t <= tc; t++)
	{
		vidTrack = GetMovieIndTrack(theMovie, t);
		if (vidTrack)
		{
			vidMedia = GetTrackMedia(vidTrack);
			if (vidMedia)
			{
				OSType	medType;
				
				GetMediaHandlerDescription(vidMedia, &medType, nil, nil);
				if (medType == VideoMediaType)
				{
					ImageDescriptionHandle	idh = (ImageDescriptionHandle)NewHandle(sizeof(ImageDescription));
					
					long	sdc = GetMediaSampleDescriptionCount(vidMedia);
					for (long s = 1; s <= sdc; s++)
					{
						GetMediaSampleDescription(vidMedia, s, (SampleDescriptionHandle)idh);
						GetImageDescriptionCTable(idh, &gCTab[s-1]);
					}
					
					DisposeHandle((Handle)idh);
					break;
				}
			}
		}
	}

	// Get the movie box and center it on the screen.	
	GetMovieBox (theMovie, &gMovieBox);		
	OffsetRect (&gMovieBox, -gMovieBox.left, -gMovieBox.top);
	OffsetRect(&gMovieBox, (gActiveArea.right - gMovieBox.right) / 2,
									  (gActiveArea.bottom - gMovieBox.bottom) / 2);
	SetMovieBox (theMovie, &gMovieBox);
	
	// Set the movie GWorld to the screen.
	SetMovieGWorld (theMovie, (CGrafPtr)gMainWindow, nil);

	// Get ready to play the movie.
	GoToBeginningOfMovie(theMovie);
	mr = GetMoviePreferredRate(theMovie);
	PrerollMovie(theMovie, 0, mr);
	
	HideCursor();

	// When playing cut-scenes, blank the screen first.
	EraseRect(&gMainWindow->portRect);
	
	// Set the palette to the first entry.
	SetEntries(1, 254, &(**(gCTab[0])).ctTable[1]);
	gPalIndex = 1;
	
	// Setup the callback routine for subsequent palette changes.
	drawCompProc = NewMovieDrawingCompleteProc(DrawDoneProc);
	SetMovieDrawingCompleteProc(theMovie, movieDrawingCallAlways, drawCompProc, 0);
	
	// Turn off sound if soundFX is off.
	if (!gShockPrefs.soSoundFX)
		SetMovieVolume(theMovie, 0);

	// Play the movie until it's done, or a mouse click occurs.
	StartMovie (theMovie);
	FlushEvents(keyDownMask | autoKeyMask | mDownMask, 0);
	while(!IsMovieDone(theMovie)) 
	{
		if (allowHalt)
		{
			EventRecord	theEvent;
			if (OSEventAvail(keyDownMask | autoKeyMask | mDownMask, &theEvent))
			{
				StopMovie(theMovie);
				FlushEvents(keyDownMask | autoKeyMask | mDownMask, 0);
				break;
			}
		}
		
		MoviesTask (theMovie, 0);									// Process the movie (quickly now!)
		tv = GetMovieTime(theMovie, NULL);					// See where we're at.
		
		if (showSubs && subHdl)										// If the movie has sub-titles, 
			DoSubtitle(tv);												// see if it's time to change.
	}
	DisposeMovie(theMovie);

	for (i=0; i < 16; i++)												// Dispose all the color tables
		if (gCTab[i])														// read in from the movie.
			DisposCTable(gCTab[i]);

	if (subHdl)
	{
		HUnlock(subHdl);
		DisposeHandle(subHdl);
	}
	
	DisposeRoutineDescriptor(drawCompProc);				// Dispose the call-back UPP

	ShowCursor();
	RGBForeColor(&black);
	RGBBackColor(&white);
}

//------------------------------------------------------------------------
//  Callback routine to change the palette.
//------------------------------------------------------------------------
pascal OSErr DrawDoneProc(Movie theMovie, long )
{
	RGBColor	black = {0, 0, 0};
	TimeValue	tv;
	long			chkTime;

	chkTime = gChgTime[gPalIndex];						// Get the next time for a palette change.
	if (chkTime != -1)											// If it's a valid time,
	{
		tv = GetMovieTime(theMovie, NULL);
		if (tv >= chkTime)										// If we've reached the time in the movie,
		{
			EraseRect(&gMovieBox);							// Erase the movie box (and sub-title box).
			if (gShowSubs)
				EraseRect(&gSubRect);
			
//			saveSeed = (*(*gScreenPixMap)->pmTable)->ctSeed;
			SetEntries(1, 254, &(**(gCTab[gPalIndex])).ctTable[1]);	// Set the palette
//			(*(*gScreenPixMap)->pmTable)->ctSeed = saveSeed;

//			BlockMove(  &(**(gCTab[gPalIndex])).ctTable[1], 
//							   &(*(*gScreenPixMap)->pmTable)->ctTable[1],
//							   254 * sizeof(ColorSpec)   );

			gChgTime[gPalIndex] = -1;						// Get ready for the next change.
			gPalIndex++;
  		}
	}
	return (0);
}

//------------------------------------------------------------------------
//  See if we need to put up a subtitle.
//------------------------------------------------------------------------
void DoSubtitle(TimeValue time)
{
	if (gCurrSub->startingTime != -1 &&					// If it's a valid time
		 time >= gCurrSub->startingTime)					// and it's time to show it
	{
		DrawSubtitle((char *)gCurrSub->subtitle);		// Draw the thang.
		gCurrSub++;
		GetNextSubtitle();
	}
}

//------------------------------------------------------------------------
//  Draw a subtitle (centered and everything).
//------------------------------------------------------------------------
void DrawSubtitle(char *title)
{
	short		tl;
	
	EraseRect(&gSubRect);									// Blank out before drawing.
	
	tl = strlen(title);											// Center the text (look for new-lines).
	TextBox(title, tl, &gSubRect, teCenter);
}


//------------------------------------------------------------------------
//  Move to the next subtitle with the same language (stop if at end).
//------------------------------------------------------------------------
void GetNextSubtitle(void)
{
	while (gCurrSub->language != 0 && gCurrSub->language != -1)	//еее Fix this!
		gCurrSub++;
}



//------------------------------------------------------------------------------------
//	  Play a v-mail movie, always showing the intro first.
//------------------------------------------------------------------------------------
void PlayVMail(FSSpec *movieSpec, short orgx, short orgy)
{
	Size					dummy;
	RGBColor			black = {0, 0, 0};
	RGBColor			white = {0xffff, 0xffff, 0xffff};
	OSErr				err;
	short 				movieResFile;
	Rect					movieBox;
	Movie				theMovie[2];
	Fixed					mr;
	short 				i;
	
	MaxMem(&dummy);							// Compact heap before loading the movie.

	// Open the movie files and prepare them for playing.
	for (i=0; i<2; i++)
	{
		if (i == 1)
			BlockMove("\pV-Mail Intro", movieSpec->name, 63);
		err = OpenMovieFile(movieSpec, &movieResFile, fsRdPerm);
		if (err == noErr) 
		{
			short 		movieResID = 0;
			Str255 		movieName;
			Boolean 		wasChanged;
																				// Load the 'moov' resource.
			err = NewMovieFromFile(&theMovie[i], movieResFile, &movieResID,
							movieName, newMovieActive, &wasChanged);
			CloseMovieFile (movieResFile);						// Close the resource fork.
		}
		else
		{
			return;
		}

		// Figure out where to place the movie on the screen.
		GetMovieBox (theMovie[i], &movieBox);		
		OffsetRect (&movieBox, -movieBox.left, -movieBox.top);
		OffsetRect(&movieBox, orgx, orgy);
		SetMovieBox (theMovie[i], &movieBox);
	
		// Set the movie GWorld to the screen.
		SetMovieGWorld (theMovie[i], (CGrafPtr)gMainWindow, nil);

		// Get ready to play the movie.
		GoToBeginningOfMovie(theMovie[i]);
		mr = GetMoviePreferredRate(theMovie[i]);
		PrerollMovie(theMovie[i], 0, mr);		
	}

	HideCursor();

	// Erase the movie box.
	RGBForeColor(&black);
	PaintRect(&movieBox);

	for (i=1; i >= 0; i--)
	{
		// Turn off sound if soundFX is off.
		if (!gShockPrefs.soSoundFX)
			SetMovieVolume(theMovie[i], 0);
		
		// Play the movie until it's done, or a mouse click occurs.
		StartMovie (theMovie[i]);
		while(!IsMovieDone(theMovie[i])) 
		{
			MoviesTask (theMovie[i], 0);									// Process the movie (quickly now!)
		}
	}
	
	for (i=0; i<2; i++)
		DisposeMovie(theMovie[i]);

	ShowCursor();
}


//uchar *intro_files[] = { "\pIntro", "\pIntro (French)", "\pIntro (German)" } ;
//extern char which_lang;

//------------------------------------------------------------------------------------
//	  Play the intro cut-scene.
//------------------------------------------------------------------------------------
void PlayIntroCutScene()
{
	FSSpec	fSpec;
	Rect		r;
	
	HideMenuBar();
	
	FSMakeFSSpec(gCDDataVref, gCDDataDirID, "\pIntro", &fSpec);
	PlayCutScene(&fSpec, FALSE, TRUE);
	PaintRect(&gMainWindow->portRect);

	SetEntries(0, 255, (**(gMainColorHand)).ctTable);
	
	ShowMenuBar();
	SetRect(&r, 0, 0, 640, 480);
	InvalRect(&r);
}


//------------------------------------------------------------------------------------
//	  Play a startup movie.
//------------------------------------------------------------------------------------
void PlayStartupMovie(FSSpec *movieSpec, short orgx, short orgy)
{
	RGBColor			black = {0, 0, 0};
	RGBColor			white = {0xffff, 0xffff, 0xffff};
	OSErr				err;
	short 				movieResFile;
	Rect					movieBox;
	Movie				theMovie;
	Fixed					mr;
	CTabHandle			ctab;

	// Open the movie files and prepare it for playing.
	err = OpenMovieFile(movieSpec, &movieResFile, fsRdPerm);
	if (err == noErr) 
	{
		short 		movieResID = 0;
		Str255 		movieName;
		Boolean 		wasChanged;
																			// Load the 'moov' resource.
		err = NewMovieFromFile(&theMovie, movieResFile, &movieResID,
						movieName, newMovieActive, &wasChanged);
		CloseMovieFile (movieResFile);						// Close the resource fork.
	}
	else
	{
		return;
	}

	// Figure out where to place the movie on the screen.
	GetMovieBox (theMovie, &movieBox);		
	OffsetRect (&movieBox, -movieBox.left, -movieBox.top);
	OffsetRect(&movieBox, orgx, orgy);
	SetMovieBox (theMovie, &movieBox);

	// Set the movie GWorld to the screen.
	SetMovieGWorld (theMovie, (CGrafPtr)gMainWindow, nil);

	// Get ready to play the movie.
	GoToBeginningOfMovie(theMovie);
	mr = GetMoviePreferredRate(theMovie);
	PrerollMovie(theMovie, 0, mr);		

	// Erase the movie box.
	RGBForeColor(&black);
	PaintRect(&movieBox);

	// Get the movie's color table and set the palette with it.
	GetMovieColorTable(theMovie, &ctab);
	SetEntries(1, 253, &(**ctab).ctTable[1]);
	BlockMove(&(**(ctab)).ctTable[1], &(**(gMainColorHand)).ctTable[1], 254 * sizeof(ColorSpec));
	ResetCTSeed();
	ReleaseResource((Handle)ctab);
	
	// Turn off sound if soundFX is off.
	if (!gShockPrefs.soSoundFX)
		SetMovieVolume(theMovie, 0);

	// Play the movie until it's done, or a mouse click occurs.
	StartMovie (theMovie);
	while(!IsMovieDone(theMovie)) 
	{
		EventRecord	theEvent;
		if (OSEventAvail(keyDownMask | autoKeyMask | mDownMask, &theEvent))
		{
			StopMovie(theMovie);
			FlushEvents(keyDownMask | autoKeyMask | mDownMask, 0);
			break;
		}
		MoviesTask(theMovie, 0);			// Process the movie (quickly now!)
	}
	DisposeMovie(theMovie);
}
