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
//		Add a sound track to a QuickTime movie.  Remember to use MoviePlayer to flatten
//		the resulting movie.
//	==============================================================

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "quiktime.h"

#include <Movies.h>
#include <ImageCompression.h>
#include <QuickTimeComponents.h>
#include <Sound.h>


//  Prototypes
void main(void);
void	 CheckError(OSErr error, Str255 displayString);
void SetInputSpecs(void);

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
	Ptr				p;
	long				stupid;
	OSErr			err, result;
	
	short			sndResNum;
		
	Point			dlgPos = {120,120};
	StandardFileReply	reply;
	SFTypeList			typeList;
	FSSpec			outSpec;
	Str255			name = "\pQT Movie";
	Rect			r;
	short			resRefNum;
	Movie			gMovie = 0;			// Our movie, track and media
	
	printf("\n");
	
	//---------------------
	// Setup Quicktime stuff.
	//---------------------
	if (EnterMovies() != noErr)	// Start up the movie tools
	{
		ParamText("\pCan't startup QuickTime.", "\p", "\p", "\p");
		StopAlert(1000, nil);
		return;
	}
	
	//----------------------
	//	Open the QuickTime movie.
	//----------------------
	printf("Open the QuickTime movie...\n\n");
	typeList[0] = 'MooV';
	StandardGetFilePreview(nil, 1, typeList, &reply);
	if (!reply.sfGood)
	{
	 	ExitMovies();
		return;
 	}
	err = OpenMovieFile(&reply.sfFile, &resRefNum, fsRdPerm);
	if (err == noErr) 
	{
		short 		movieResID = 0;		// get first movie
		Str255 		movieName;
		Boolean 		wasChanged;
	
		err = NewMovieFromFile(&gMovie, resRefNum, &movieResID,
						movieName, newMovieActive, &wasChanged);
		CloseMovieFile( resRefNum );
	}
	else
		CheckError(1, "\pCan't open the movie file!!");
	
	// Setup an FSSpec for the output file.
	outSpec = reply.sfFile;
	BlockMove("\pOutput Movie", outSpec.name, 20);
	
	//----------------------
	//	Open the input Sound file.
	//----------------------
	printf("Open the Sound file...\n\n");
	typeList[0] = 'sfil';
	StandardGetFilePreview(nil, 1, typeList, &reply);
	if (!reply.sfGood)
	{
	 	ExitMovies();
		return;
 	}
	sndResNum = FSpOpenResFile(&reply.sfFile, fsRdPerm);
	if (sndResNum == -1)
		CheckError(1, "\pCan't open the sound file!!");
	
	ClearMoviesStickyError();
	
	// Add the sound track here.
	CreateMySoundTrack(gMovie);
	
	// Save the new movie.
	FlattenMovie(gMovie, 0, &outSpec, 'TVOD', smCurrentScript, createMovieFileDeleteCurFile, NULL, NULL);
	CheckError (GetMoviesError(), "\pCouldn't save output movie." );

	// Cleanup.
	if ( gMovie ) 
		DisposeMovie(gMovie);	
	CloseResFile(sndResNum);
 	ExitMovies();
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
 	ExitMovies();
	ExitToShell();
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
