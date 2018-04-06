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
//===========================================================================
//
//	SndCaps.c -	Program to display the sound capabilities of the Macintosh
//				the program is running on.
//
//===========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GestaltEqu.h>
#include <Sound.h>

//------------
// Prototypes
//------------
void main(void);
SndChannelPtr CreateSndChannel(void);

//------------
// Globals
//------------
int				gNumSC;
SndChannelPtr 	gSCPtr[100];


//--------------------------------------------------------------------------
//	main -	Lists all the sound capabilities of this Macintosh.
//--------------------------------------------------------------------------
void main(void)
{
	long			resp;
	OSErr		myErr;
	NumVersion	smVer;
	int 			i;

	printf("\nGetting sound capabilities…\n\n");

	// Show all the Gestalt info
	
	myErr = Gestalt(gestaltSoundAttr, &resp);
	if (myErr != noErr)
	{
		printf("Error in Gestalt call: %d\n", myErr);
		return;
	}
	if (resp & (1 << gestaltStereoCapability))
		printf("Stereo capable\n");
	if (resp & (1 << gestaltStereoMixing))
		printf("Mixes stereo to mono\n");
	if (resp & (1 << gestaltSoundIOMgrPresent))
		printf("Sound Input Manager is present\n");
	if (resp & (1 << gestaltBuiltInSoundInput))
		printf("Built-in sound input hardware\n");
	if (resp & (1 << gestaltHasSoundInputDevice))
		printf("Sound input device present\n");
	if (resp & (1 << gestaltPlayAndRecord))
		printf("Can play and record sounds at same time\n");
	if (resp & (1 << gestalt16BitSoundIO))
		printf("16 bit sound supported\n");
	if (resp & (1 << gestaltStereoInput))
		printf("Can record stereo sounds\n");
	if (resp & (1 << gestaltLineLevelInput))
		printf("Built-in input hardware needs line level\n");
	if (resp & (1 << gestaltSndPlayDoubleBuffer))
		printf("Play-from-disk supported\n");
	if (resp & (1 << gestaltMultiChannels))
		printf("Multiple sound channels supported\n");
	if (resp & (1 << gestalt16BitAudioSupport))
		printf("Can play 16 bit audio data\n");
	
	// Get Sound Manager and MACE versions.
	
	smVer = SndSoundManagerVersion();
	printf("\nSound Manager version: %X.%2.2X\n", smVer.majorRev, smVer.minorAndBugRev);
	smVer = MACEVersion();
	printf("\MACE version: %X.%2.2X\n", smVer.majorRev, smVer.minorAndBugRev);
	
	// Allocate all possible channels.
	
	SndChannelPtr	scPtr;
	gNumSC = 0;
	do {
		scPtr = CreateSndChannel();
		if (scPtr)
			gSCPtr[gNumSC++] = scPtr;
	} while (scPtr);
	printf("\nNumber of sound channels allocated: %d\n", gNumSC);
	
	// Play a sound on all of the allocated channels.
	
	printf("\nPlaying a sound on each channel.\n");

	Handle	sndHdl = GetResource('snd ', 208);				// Get the sound resource to play.
	if (sndHdl)
	{
		DetachResource(sndHdl);
		HLock(sndHdl);
		
		for (i = 0; i < gNumSC; i++)
		{
			myErr = SndPlay(gSCPtr[i], (SndListHandle)sndHdl, TRUE);		// Play the sound
			if (myErr != noErr)
				printf("Couldn't play a sound for channel %d: %d\n", i, myErr);

			Delay(15, &resp);								// Wait 1/4 second before playing next sound.
		}
		
		// Wait until the last sound is finished playing before continuing.
		
		SCStatus	stat;
		do {
			SndChannelStatus(gSCPtr[gNumSC-1], sizeof(SCStatus), &stat);
		} while (stat.scChannelBusy);
		
		HUnlock(sndHdl);									// Dispose of the sound resource.
		DisposeHandle(sndHdl);
	}
	else
		printf("Couldn't get the sound resource.\n");
	
	// Dispose of the sound channels.
	
	for (i = 0; i < gNumSC; i++)
	{
		myErr = SndDisposeChannel(gSCPtr[i], TRUE);
		if (myErr != noErr)
			printf("Error in SndDisposeChannel call for channel %d: %d\n", i, myErr);
		DisposePtr((Ptr)gSCPtr[i]);
	}
	
	printf("\nDone.\n");
}


//--------------------------------------------------------------------------
//	CreateSndChannel -	Create a sound channel and return its ptr.
//--------------------------------------------------------------------------
SndChannelPtr CreateSndChannel(void)
{
	SndChannelPtr	scPtr;
	OSErr			err;
	
	scPtr = SndChannelPtr(NewPtr(sizeof(SndChannel)));
	if (scPtr)
	{
		scPtr->qLength = 16;
		err = SndNewChannel(&scPtr, sampledSynth, initStereo, NULL);
		if (err != noErr)
		{
			DisposePtr(Ptr(scPtr));
			scPtr = NULL;
		}
		else
			scPtr->userInfo = 0;
	}
	
	return(scPtr);
}
