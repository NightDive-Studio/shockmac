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
 * $Source: r:/prj/cit/src/RCS/audiolog.c $
 * $Revision: 1.17 $
 * $Author: dc $
 * $Date: 1994/11/19 20:35:27 $
 */
 //	Mac version by Ken Cobb,  2/9/95
 

#include <stdio.h>
#include <Movies.h>		// QuickTime header

#include "Shock.h"
//#include <movie.h>
#include "audiolog.h"
#include "citalog.h"
#include "criterr.h"
#include "faketime.h"
#include "map.h"
#include "tools.h"
#include "MacTune.h"
#include "musicai.h"


//-------------------
//  PROTOTYPES
//-------------------
bool audiolog_cancel_func(short , ulong , void* );

//-------------------
//  GLOBALS
//-------------------
int 		curr_alog = -1;
//int 		alog_fn = -1;
uchar 	audiolog_setting = 1;
Movie 	alog = NULL;

// this can become RES_alog_email0 once there is one
#define AUDIOLOG_BASE_ID      2741
#define AUDIOLOG_BARK_BASE_ID 3100

//#define MIN_ALOG_SIZE   20000

//#define AUDIOLOG_BLOCK_LEN MOVIE_DEFAULT_BLOCKLEN
//#define AUDIOLOG_BUFFER_SIZE  (64 * 1024)
//uchar audiolog_buffer[AUDIOLOG_BUFFER_SIZE];


//-------------------------------------------------------------
//  Sorry excuse for a function.
//-------------------------------------------------------------
errtype audiolog_init()
{
   return(OK);
}

//extern bool set_sample_pan_gain(snd_digi_parms *sdp);

//#define ALOG_PAN 64
//char *bark_files[] = { "citbark.res", "frnbark.res", "gerbark.res" };
//char *alog_files[] = { "citalog.res", "frnalog.res", "geralog.res" };
//-------------------------------------------------------------
//  Play an audiolog or bark file.
//-------------------------------------------------------------
errtype audiolog_play(int email_id)
{
	extern uchar	curr_alog_vol;
	extern char	which_lang;
//	snd_digi_parms *sdp;
//	int new_alog_fn;
	char				buff[64];
	FSSpec			fSpec;
	OSErr			err;
	short 			movieResFile;
	Fixed				mr;
	Size				dummy;

//KLC - no sfx_card global in Mac version
//	if ((!sfx_on) || (!sfx_card) || (!audiolog_setting))
//		return(ERR_NOEFFECT);
	if ((!sfx_on) || (!audiolog_setting))
		return(ERR_NOEFFECT);
	
	// KLC - Big-time hack to prevent bark #389 from trying to play twice (and thus skipping).
	if ((email_id == 389) && (alog != NULL) && (curr_alog == email_id))
		return(ERR_NOEFFECT);
	
	// KLC - Another big-time hack to prevent the Shodan bark #448 from playing before
	// bark #389 has finished.
	if ((email_id == 448) && (alog != NULL) && (curr_alog == 389))
	{
		while (alog != NULL)
			audiolog_loop_callback();
	}
		
	// Stop any currently playing alogs.
	audiolog_stop();

	// woo hoo, what a hack!
	// this is for the player's log-to-self which has no audiolog
	if (email_id == 0x44)
		return(ERR_NOEFFECT);
	
	begin_wait();
	MaxMem(&dummy);							// Compact heap before loading the alog.

	// Open up the appropriate sound-only movie file.

	if (email_id > (AUDIOLOG_BARK_BASE_ID - AUDIOLOG_BASE_ID))
	{
		sprintf(buff, "BARK %x", AUDIOLOG_BASE_ID + email_id);
		FSMakeFSSpec(gBarkVref, gBarkDirID, c2pstr(buff), &fSpec);
	}
	else
	{
		sprintf(buff, "ALOG %x", AUDIOLOG_BASE_ID + email_id);
		FSMakeFSSpec(gAlogVref, gAlogDirID, c2pstr(buff), &fSpec);
	}
	err = OpenMovieFile(&fSpec, &movieResFile, fsRdPerm);
	if (err == noErr) 
	{
		short 		movieResID = 0;
		Str255 		movieName;
		Boolean 		wasChanged;
																			// Load the 'moov' resource.
		err = NewMovieFromFile(&alog, movieResFile, &movieResID,
						movieName, newMovieActive, &wasChanged);
		CloseMovieFile (movieResFile);						// Close the resource fork.
	}
	else
	{
		end_wait();
		return(ERR_FREAD);
	}

/*
   // Make sure this is a thing we have an audiolog for...
   if ((!ResInUse(AUDIOLOG_BASE_ID + email_id)) || (ResSize(AUDIOLOG_BASE_ID + email_id) < MIN_ALOG_SIZE))
   {
      ResCloseFile(new_alog_fn);
      end_wait();
      return(ERR_FREAD);
   }
   audiolog_stop();
   alog_fn = new_alog_fn;

   palog = MoviePrepareRes(AUDIOLOG_BASE_ID + email_id, audiolog_buffer, sizeof(audiolog_buffer), AUDIOLOG_BLOCK_LEN);
   sdp=snd_sample_parms(palog->as.smp_id);
   sdp->vol=curr_alog_vol;
   sdp->pan=ALOG_PAN;
   sdp->snd_ref=0;
   set_sample_pan_gain(sdp);

   // check failure
   if (palog == NULL)
   {
      Warning(("MoviePrepareRes returned NULL! email_id = %x\n",email_id));
      end_wait();
      return(ERR_NOMEM);
   }
*/

	// rock and roll
//   MovieReadAhead(palog, 2);
//  MoviePlay(palog, NULL);
	SetMovieGWorld (alog, nil, nil);
	GoToBeginningOfMovie(alog);
	mr = GetMoviePreferredRate(alog);
	PrerollMovie(alog, 0, mr);		
	StartMovie(alog);

	end_wait();

   // bureaucracy
   curr_alog = email_id;

   return(OK);
}

char secret_pending_hack;
extern Boolean	gDeadPlayerQuit;
extern Boolean	gPlayingGame;

//-------------------------------------------------------------
//  Stop a current audiolog from playing.
//-------------------------------------------------------------
void audiolog_stop()
{
	// double check
	if (alog != NULL)
	{
//		extern uchar curr_vol_lev;
	
		// waste it!
		//MovieKill(palog);
		StopMovie(alog);
		DisposeMovie(alog);
		
		// Restore music volume
//		if (music_on)
//			mlimbs_change_master_volume(curr_vol_lev);
	}
//	if (alog_fn >= 0)
//	{
//		ResCloseFile(alog_fn);
//		alog_fn = -1;
//	}

	alog = NULL;
	curr_alog = -1;

	if (secret_pending_hack)
	{
		secret_pending_hack = 0;
     	gDeadPlayerQuit = TRUE;					// The player is dead.
		gPlayingGame = FALSE;					// Hop out of the game loop.
	}
}

//#define ALOG_MUSIC_VOLUME  65
//-------------------------------------------------------------
//  Task handler for audiologs and barks.
//-------------------------------------------------------------
errtype audiolog_loop_callback()
{
//   extern uchar curr_vol_lev;

	// check on things
	if (alog != NULL)
	{
		MoviesTask(alog, 0);
		if (IsMovieDone(alog))
		{
			audiolog_stop();
		}
//		MovieUpdate(palog);
//		if (!MoviePlaying(palog))
//			audiolog_stop();
//		else
//		{
//			if (music_on)
//				mlimbs_change_master_volume(min(ALOG_MUSIC_VOLUME,curr_vol_lev));
//		}
	}
	return(OK);
}


/*
// subsumed by new timer lib
#ifdef NOTDEF 
fix GetFixTimer()
{
   return(fix_make(*tmd_ticks / CIT_CYCLE, (*tmd_ticks % CIT_CYCLE) * 65536 / CIT_CYCLE ));
}
#endif
*/

//-------------------------------------------------------------
// if email_id is -1, returns whether or not anything is playing
// if email_id != -1, matches whether or not that specific email_id is playing
//-------------------------------------------------------------
bool audiolog_playing(int email_id)
{
	if (email_id == -1)
		return(curr_alog != -1);
	else
		return(curr_alog == email_id);
}

//-------------------------------------------------------------
//  Start playing a bark file.
//-------------------------------------------------------------
errtype audiolog_bark_play(int bark_id)
{
	if (global_fullmap->cyber)
		return(ERR_NOEFFECT);
	else
		return(audiolog_play(bark_id + (AUDIOLOG_BARK_BASE_ID - AUDIOLOG_BASE_ID)));
}

//-------------------------------------------------------------
//  Stop playing audiolog (in response to a hotkey).
//-------------------------------------------------------------
bool audiolog_cancel_func(short , ulong , void* )
{
	audiolog_stop();
	return(TRUE);
}
