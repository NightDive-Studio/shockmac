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
 * $Source: r:/prj/cit/src/RCS/mlimbs.c $
 * $Revision: 1.32 $
 * $Author: dc $
 * $Date: 1994/11/23 00:13:15 $
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <timer.h>

//���#include <ail.h>
#include "mlimbs.h"
#include "musicai.h"

#define CHANNEL_MAP

#define LOCK_ALL_CHANNELS


bool   mlimbs_on=FALSE;
static   char   mlimbs_status=0;    // could make this one bitfield of status, on/off, enable/not, so on
static   int    mlimbs_timer_id;    // what our timer handle is

static   uchar *mlimbs_theme=NULL;  // data about the current theme
volatile int    mlimbs_master_slot = -1;
static   int    mlimbs_cur_theme_id= -1;

volatile struct mlimbs_piece_info xseq_info[MAX_SEQUENCES];	// Sequence specific information
volatile struct mlimbs_request_info current_request[MLIMBS_MAX_SEQUENCES - 1]; // Request information
volatile struct mlimbs_channel_info channel_info[MLIMBS_MAX_CHANNELS]; // MIDI channel information
volatile struct mlimbs_playing_info userID[MLIMBS_MAX_SEQUENCES - 1]; // Sequence instance specific information
volatile bool   mlimbs_update_requests = FALSE;

volatile uchar   num_XMIDI_sequences = 0;
volatile uchar   num_master_measures;
volatile uchar   voices_used = 0;
volatile uchar   max_voices = 0;
                 
volatile void  (*mlimbs_AI)(void) = NULL;
volatile ulong   mlimbs_counter = 0;
volatile long    mlimbs_error;
volatile bool    mlimbs_semaphore = FALSE;

int     master_volume = 100;

char curr_play_list[10];
char loop_list[10];
char cpl_num;

// gruesome hacks to try and get this working for now
int mlimbs_priority[MLIMBS_MAX_SEQUENCES];

// convienience psuedo-function defines
#define _uiD_seq(i) ((SEQUENCE *)snd_get_sequence(userID[i].seq_id))
#define _mlimbs_top if (mlimbs_status==0) return -1



//���LONG mlimbs_timbre_callback(MDI_DRIVER *mdi, LONG bank, LONG patch);

/////////////////////////////////////////////////////////////////
//	mlimbs_init (void)
//
//	purpose:
//		This routine initializes the MLIMBS system.  This also
//		must be used if you want to switch the SndMIDIDevice that
//		MLIMBS uses.  This must be called before using mlimbs.
//
//	inputs:
//    now uses the default global midi device always
int mlimbs_init (void)
{
	int i;
   
//���   if (!music_card) return -1;

	if (mlimbs_status!=0) return 1;
	for (i=0; i < 10; i++)
	{
		curr_play_list[i] = -1;
		loop_list[i] = -1;
	}
	cpl_num = 0;

/*��� What is max_voices???
	// Determine maximum number of voices
#ifdef AIL_2
			case SOUNDBLASTER:
			case SOUNDBLASTERPRO:
			case ADLIB:
				max_voices = 9;
				break;
			case SOUNDBLASTERPRO2:
				max_voices = 18;
			case MT32:
				// Switch max_voices to 32 when Roland specific dat files are
				// available, since voices usage should be tracked using partials on a Roland.
//				max_voices = 32;	// Note that this here is actually the # of partials available.
				max_voices = 9;	// For now, since we're using only SB/ADLIB dat files, set voice limit to 9.
				master_volume = 90;	// Reduces distortion in Roland MT-32's
				break;
			default:
				max_voices = 9;
				break;
		}
#else
   max_voices=18;
#endif

   snd_set_midi_sequences(MLIMBS_MAX_SEQUENCES);

#ifdef CALLBACK_ON
// Install mlimbs_callback, which will be called by the master sequence twice per loop.
// each one has a different value, one means half done, get ready, the other means do the switch
   seq_miditrig=mlimbs_callback;
   AIL_register_timbre_callback((MDI_DRIVER *)snd_midi,mlimbs_timbre_callback);

//   seq_finish=mlimbs_seq_done_call;

//	Since we cannot stop and start XMIDI sequences from within a MIDI callback, use 
// a timer, running at 100 hz to start and stop XMIDI sequences. The mlimbs_callback
// simply sets a flag, letting the timer know when to update sequence status.
	if ((mlimbs_timer_id = tm_add_process(mlimbs_timer_callback, 0, (TMD_FREQ/MLIMBS_TIMER_FREQUENCY))) == -1)
		goto die;
#endif

#ifdef LOCK_ALL_CHANNELS
//	Lock all MIDI channels on the current mlimbs_device, and
//	initialize the channel_info[] array.
	for (i = 0; i < MLIMBS_MAX_CHANNELS; i++)
	{
		channel_info[i].mchannel = AIL_lock_channel((MDI_DRIVER *)snd_midi);
		channel_info[i].usernum = -1;	// This indicates what sequence is using this channel. -1 is stale handle.
		channel_info[i].status = MLIMBS_STOPPED;	// Current status of the channel.
	}
#endif
���*/
	mlimbs_status=1;
	return 1;

die:
	mlimbs_shutdown();
	return -1;
}

/////////////////////////////////////////////////////////////////
//	mlimbs_shutdown
//
//	purpose:
//		This shuts down the mlimbs system.  After calling this,
//		call mlimbs_init again to restart it.
//
/////////////////////////////////////////////////////////////////
void mlimbs_shutdown(void)
{
   if (mlimbs_status==0) return;

/*��� later, man
	mlimbs_purge_theme();

#ifdef CALLBACK_ON
   seq_miditrig=NULL;
//   seq_finish=NULL;
   AIL_register_timbre_callback((MDI_DRIVER *)snd_midi,NULL);
   tm_remove_process(mlimbs_timer_id);
#endif

#ifdef LOCK_ALL_CHANNELS
   {	// Release all the locked MIDI channels, and clear the channel_info[] array
      int i;
		for (i = 0; i < MLIMBS_MAX_CHANNELS; i++)
		{
			if (channel_info[i].mchannel >= 0) AIL_release_channel((MDI_DRIVER *)snd_midi,channel_info[i].mchannel);
			channel_info[i].mchannel = -1;
			channel_info[i].usernum = -1;
			channel_info[i].status = MLIMBS_STOPPED;
	   }
   }
#endif
*/
   mlimbs_status=0;
}

#ifdef NOT_YET //���

/////////////////////////////////////////////////////////////////
// int mlimbs_load_theme
//
//	purpose:
//		Load theme will load an XMIDI file into theme, stopping
//		playback and purging the previous XMIDI file from theme.
//		It will also allocate state tables if needed and preload all
//		timbres used by any sequence in the XMIDI file.
//
//	inputs:
//		char *xname				Filename for the XMIDI theme file.
//		char *xinfo				Filename for the file to be used in filling out xseq_info[].
//		char *GTL_filename	Global Timbre Library filename, for preloading all timbres.
//	return:
//		1 if successful
//		-1 if unsuccessful
// 	Also set mlimbs_error to the following values if unsuccessful:
//			-2 if it failed to load the XMIDI file
//			-3 if it failed to load the data file.
/////////////////////////////////////////////////////////////////
int mlimbs_load_theme (char *xname, char *xinfo, int thmid)
{
	int i;
	FILE *fil;

   //secret_sprint((ss_temp,"try to load themed %s (%d) (%d)\n",xname,thmid,mlimbs_status));
   _mlimbs_top;
	mlimbs_purge_theme();	// Purge any previously loaded mlimbs theme
   if ((mlimbs_theme=snd_load_raw(xname,NULL))==NULL)
      return -2;
	if ((fil = fopen(xinfo, "rb")) == NULL)
      return -3;
   mlimbs_cur_theme_id=thmid;
	fread(&num_XMIDI_sequences, sizeof(uchar), 1, fil);	// Number of sequences in this XMIDI file, NOT COUNTING THE MASTER SEQUENCE
	fread(&num_master_measures, sizeof(uchar), 1, fil);	// Number of measures in the master sequence - i.e. # of measures per loop
	num_XMIDI_sequences = min (num_XMIDI_sequences,MAX_SEQUENCES);
	for (i = 0; i < num_XMIDI_sequences; i++)
	{	
		fread (&(xseq_info[i].max_voices), sizeof(uchar), 1, fil);	   // Maximum # of simultaneous voices
		fread (&(xseq_info[i].avg_voices), sizeof(uchar), 1, fil);     // Normal # of simultaneous voices
		fread (&(xseq_info[i].channel_map),sizeof(ushort), 1, fil);    // Bitmap of channel usage
		fread (&(xseq_info[i].num_measures),sizeof(uchar), 1, fil);    // # of measures
		xseq_info[i].num_measures = max(1,xseq_info[i].num_measures);  // Don't allow 0 measure chunks (prevents divide by 0 in mlimbs_timer_callback)
		fread(&(xseq_info[i].priority),sizeof(int), 1, fil);           // Default priority
  		fread(xseq_info[i].channel_voices,sizeof(uchar),7,fil);  		// Now read in channel voice usage
	}
	fclose(fil);
   //secret_sprint((ss_temp,"load themed %s (%d) a-ok\n",xname,thmid));
	return 1;
}

/////////////////////////////////////////////////////////////////
// int mlimbs_start_theme
//
//	purpose:
//		This begins playback of the theme by playing the
//		master_sequence (sequence 0).  NOTE:  This doesn't clear
//		the current_request array and doesn't reset mlimbs_counter.
//
//	inputs:
//
//	return:
//	1 if successful
//		-1 if  not
//	Sets mlimbs_error to the following error codes if unsuccessful.
//		-1 if no mlimbs device present
//		-2 if it failed to load the master track.
//		-3 if it failed to start the master track.
//
/////////////////////////////////////////////////////////////////
int mlimbs_start_theme (void)
{
	if ((mlimbs_status==0)||(mlimbs_theme == NULL)) return -1;     // no can do
   if (mlimbs_master_slot>=0) return 1;   // already can done

   if ((mlimbs_master_slot = snd_sequence_play(mrefBuild(mlimbs_cur_theme_id,CALLBACK_SEQ_NUM),mlimbs_theme,CALLBACK_SEQ_NUM,NULL))==SND_PERROR)
	{   // better make sure this is looping, eh...
      return -2;
	}
   AIL_set_sequence_loop_count(snd_sequence_ptr_from_id(mlimbs_master_slot),0);

#ifdef CALLBACK_ON
	// Start the mlimbs timer callback which will now start and stop 
	//	various tracks according to how the current_request structures are set
   tm_activate_process(mlimbs_timer_id);
#endif

   //secret_sprint((ss_temp,"start theme %d callback %d\n",mlimbs_cur_theme_id,mlimbs_timer_id));

	return 1;
}

// i hate these, thank you
void _mlimbs_clear_req(int i)
{
	current_request[i].pieceID = -1;
	current_request[i].rel_vol = DEFAULT_REL_VOL;
	current_request[i].ramp_time = DEFAULT_RAMP_TIME;
	current_request[i].ramp = 0;
	current_request[i].priority = 0;
	current_request[i].loops = -1;
	current_request[i].pan = -1;
	current_request[i].channel_prioritize = FALSE;
	current_request[i].crossfade = 0;
}

void _mlimbs_clear_uid(int i)
{
   int j;
   userID[i].pieceID = -1;
	userID[i].current_channel_map = 0;
	userID[i].seq_id = -1;
	userID[i].rel_vol = DEFAULT_REL_VOL;
	userID[i].channel_prioritize = FALSE;
	userID[i].crossfade_status = 11;
   for (j = 0; j < 7; j++)
      userID[i].sequence_channel_status[j] = SEQUENCE_CHANNEL_UNUSED;
}


/////////////////////////////////////////////////////////////////
// void mlimbs_stop_theme
//
//	purpose:
//		This routine stops all mlimbs XMIDI playback.  After 
//		calling this routine, a call to mlimbs_start_theme is
//		needed to restart playback.  Note that this doesn't
//		clear the current_request[] array and doesn't reset the
//		mlimbs_counter.
//
//
//	inputs:
//
//	
/////////////////////////////////////////////////////////////////
void mlimbs_stop_theme (void)
{
	int i;

   if (mlimbs_status==0) return;

	/* Stop all sequences */
#ifdef CALLBACK_ON
	tm_deactivate_process (mlimbs_timer_id);
#endif

	mlimbs_update_requests = FALSE;

	for (i = 0; i < MLIMBS_MAX_SEQUENCES-1; i++)
	{
		mlimbs_punt_piece(i);
      _mlimbs_clear_req(i);
	}
	mlimbs_master_slot = -1;
	voices_used = 0;

   //secret_sprint((ss_temp,"stop theme %d callback %d\n",mlimbs_cur_theme_id,mlimbs_timer_id));
}

/////////////////////////////////////////////////////////////////
// void mlimbs_purge_theme
//
//	purpose:
//		This routine, stops all XMIDI sequence playback, unloads 
//		the XMIDI sequence from theme, and clears the xseq_info[]
//		array.  Note that it also reinitializes the current_request[]
//		and xseq_info[] arrays.
//
//	inputs:
//
//	
/////////////////////////////////////////////////////////////////
void mlimbs_purge_theme (void)
{
	int i;

	mlimbs_update_requests = FALSE;
	/* Clear the current_request[] and arrays */

	for (i = 0; i < MLIMBS_MAX_SEQUENCES -1; i++)
	{
      _mlimbs_clear_req(i);
      _mlimbs_clear_uid(i);
	}

   memset(xseq_info,0,sizeof(xseq_info)); // make sure this is full size
	for (i = 0; i < MAX_SEQUENCES; i++)
		xseq_info[i].num_measures = 1;
	mlimbs_master_slot = -1;

   if (mlimbs_status==0) return;

#ifdef CALLBACK_ON
	tm_deactivate_process (mlimbs_timer_id);
#endif

   snd_kill_all_sequences();
   if (mlimbs_theme!=NULL)
	   Free(mlimbs_theme);
   mlimbs_theme=NULL;

	mlimbs_update_requests = FALSE;
#ifdef LOCK_ALL_CHANNELS
	for (i = 0; i < MLIMBS_MAX_CHANNELS; i++)
		{
			channel_info[i].usernum = -1;
			channel_info[i].status = MLIMBS_STOPPED;
		}
#endif

	num_XMIDI_sequences = 0;
	num_master_measures = 1;
	voices_used = 0;
	mlimbs_counter = 0;

   //secret_sprint((ss_temp,"purge theme %d callback %d\n",mlimbs_cur_theme_id,mlimbs_timer_id));
}

/////////////////////////////////////////////////////////////////
//
//	mlimbs_mute_sequence_channel
//
//	purpose: This will free a physical channel being used by a chunk's
//	sequence channel and will set the sequence channel's status to
//	SEQUENCE_CHANNEL_MUTED, or SEQUENCE_CHANNEL_PENDING.  This routine is
//	used when a chunk must give up a channel to a higher priority chunk,
//	or when a chunk must be muted.
//
//	inputs:
//		int usernum		// The sequence instance to give up a channel.
//		int x				// Which channel to relenquish.
//		bool mute			// If TRUE, mute the channel.  If FALSE merely,
//							//	relenquish the channel, and set the sequence channel's
//							// status to SEQUENCE_CHANNEL_PENDING
/////////////////////////////////////////////////////////////////

void mlimbs_mute_sequence_channel (int usernum, int x, bool mute)
{
	int val,seq_ch;
	int phys_ch;

	if (usernum < 0) return;
	if (userID[usernum].pieceID < 0) return;

   //secret_sprint((ss_temp,"want to mute %d's %d, %d\n",usernum,x,mute));

	if (mute == TRUE)
	{
		switch (userID[usernum].sequence_channel_status[x-10])
		{
		case SEQUENCE_CHANNEL_PENDING:
			userID[usernum].sequence_channel_status[x-10] = SEQUENCE_CHANNEL_MUTED;
		case SEQUENCE_CHANNEL_MUTED:
		case SEQUENCE_CHANNEL_UNUSED:
			return;
		default:
			break;
   	}
	}
	else if (userID[usernum].sequence_channel_status[x-10] < 0)
		return;

	phys_ch = userID[usernum].sequence_channel_status[x-10];

	if (channel_info[phys_ch].status == MLIMBS_PLAYING_PIECE)
	{	/* Remap the channel if it is playing in a sequence */
      //secret_sprint((ss_temp,"is playing, remapping it %d from %d\n",phys_ch,x));
		seq_ch = x;
		if (seq_ch >= 11)
		{	/* First check if the XMIDI bank select controller was set*/
		 	/* If it's < 0,this means the controller wasn't initialized.  Thus, we need to initialize it
		 		to 0 in order for the stop/resume trick to work */
		 	val = AIL_controller_value(_uiD_seq(usernum),seq_ch,XMIDI_BANK_SELECT);
#ifdef SND_CHANGE
         // hey, this isnt supported in AIL3
		 	if ((val < 0) || (val > 127))
		 		AIL_set_controller_value(_uiD_seq(usernum),seq_ch,XMIDI_BANK_SELECT,0);
#endif
		 	if ((val < 0) || (val > 127))
         {
            AIL_send_channel_voice_message(NULL,_uiD_seq(usernum),MIDI_CONTROL_CHANGE|(seq_ch-1),XMIDI_BANK_SELECT,0);
         }
		 	AIL_stop_sequence(_uiD_seq(usernum));
		 	AIL_map_sequence_channel(_uiD_seq(usernum),seq_ch,seq_ch);
		 	AIL_resume_sequence(_uiD_seq(usernum));
		 	channel_info[phys_ch].status = MLIMBS_STOPPED;
  	   }
   }

	/* Indicate that this physical channel is free */
	channel_info[phys_ch].usernum = -1;
	channel_info[phys_ch].sequence_channel = -1;
	voices_used -= xseq_info[userID[usernum].pieceID].channel_voices[x-10];
   userID[usernum].sequence_channel_status[x-10] = (mute == TRUE) ? SEQUENCE_CHANNEL_MUTED : SEQUENCE_CHANNEL_PENDING;
	userID[usernum].current_channel_map &= ~(0x0001 << phys_ch);
   //secret_sprint((ss_temp,"is now free\n"));
}

//////////////////////////////////////////////////////////////////
// mlimbs_unmute_sequence_channel
//
//	This routine attempts to map a sequence channel of a given
//	chunk to a physical channel.  It searches for physical channels
//	not currently used by other chunks.
//
//	NOTE: mlimbs_unmute_sequence_channel will not attempt to free
//	channels or voices.
//
//////////////////////////////////////////////////////////////////

int mlimbs_unmute_sequence_channel (int usernum, int x)
{
	int i;

	if (usernum < 0) return 1;
	if (userID[usernum].pieceID < 0) return 1;

   //secret_sprint((ss_temp,"unmuting %d of %d\n",usernum,x));

	switch (userID[usernum].sequence_channel_status[x-10])
		{
			case SEQUENCE_CHANNEL_MUTED:
				userID[usernum].sequence_channel_status[x-10] = SEQUENCE_CHANNEL_PENDING;
			case SEQUENCE_CHANNEL_PENDING:
				break;
			case SEQUENCE_CHANNEL_UNUSED:
			default:
				return 1;
		}


	/* First, make sure there are enough voices */
	if (xseq_info[userID[usernum].pieceID].channel_voices[x-10] + voices_used > max_voices)
		return -1;

	/* Now, look for unused channels */
	for (i = 0; i < MLIMBS_MAX_CHANNELS; i++)
		{
			if (channel_info[i].status == MLIMBS_STOPPED)
				{
               //secret_sprint((ss_temp,"undoing %d (%d)\n",i,channel_info[i].mchannel));

					AIL_stop_sequence(_uiD_seq(usernum));
					AIL_map_sequence_channel(_uiD_seq(usernum),x,channel_info[i].mchannel);
					AIL_resume_sequence(_uiD_seq(usernum));

					channel_info[i].status = MLIMBS_PLAYING_PIECE;
					channel_info[i].usernum = usernum;
					channel_info[i].sequence_channel = x;

					voices_used += xseq_info[userID[usernum].pieceID].channel_voices[x-10];

					userID[usernum].sequence_channel_status[x-10] = i;
					userID[usernum].current_channel_map |= (0x0001 << i);
					return 1;
				}
		}
   //secret_sprint((ss_temp,"we lost, no joy\n"));
	return -1;
}

/////////////////////////////////////////////////////////////////
// mlimbs_channel_prioritize
//
// This routine attempts to acquire free channels and voices
//	from lower priority chunks for a given chunk.
// For channels used by lower priority chunks, this routine simply
//	frees them. Call mlimbs_assign_channels to give them to a 
//	chunk.  The minimum number of voices required is the number of
//	voices the chunk plays on channel 10.  If not enough voices
//	can be freed, then nothing	is punted, and the chunk is not played.
//	If channel_prioritize is set to FALSE, then mlimbs_channel_prioritize
//	will attempt to acquire all necessary free channels and voices,else
//	fail.
//
/////////////////////////////////////////////////////////////////

int mlimbs_channel_prioritize (int priority, int pieceID,int voices_needed,bool crossfade,bool channel_prioritize)
{
	int i,j;
	int channels_needed, num_free_channels;
	int seq_punted;
	int voices_punted;
	int punt_list[MLIMBS_MAX_SEQUENCES];
	int min;
	int min_voices_needed;

	if (pieceID >= num_XMIDI_sequences) return -1;

	channels_needed = num_free_channels = seq_punted = 0;
	voices_punted = 0;

	/* the punt_list is a list of userID[] entries */
	for (i = 0; i < MLIMBS_MAX_SEQUENCES; i++)
		punt_list[i] = -1;

	/* First, count free physical_channels */
	for (i = 0; i < MLIMBS_MAX_CHANNELS; i++)
		if (channel_info[i].usernum == -1)
			num_free_channels++;

	/* Count # of channels needed */
	if (crossfade != TRUE || channel_prioritize == FALSE)
	{
		for (i = 11,channels_needed = 0; i < 16; i++)
			if ((xseq_info[pieceID].channel_map >> i) & (0x0001))
				channels_needed++;
	}
	else channels_needed = 0; // If we're crossfading, mlimbs_channel_prioritize
									  // doesn't need to get channels for this piece, right now.

	// Minimum # of voices needed to play = # of voices on channel 10 since MIDI on
	//	channel 10 is never remapped to a nonphysical channel

	min_voices_needed = xseq_info[pieceID].channel_voices[0] - (max_voices - voices_used);

	// If we're crossfading, we only needed the minimum # of voices.
	if (crossfade == TRUE && channel_prioritize == TRUE)
		voices_needed = min_voices_needed;

	if (voices_needed <= 0)
		if (channels_needed <= num_free_channels)
			return 1;

	/*************************************/
	/* Find all the channels we can punt */
	/* and voices we can free.				 */
	/*************************************/
	for (i = 0; i < MLIMBS_MAX_SEQUENCES - 1; i++)
		{
			if (userID[i].pieceID >= 0)
				{
					// index mlimbs_priority[] with i+1 because the master track is
					// already in slot 0, while userID[0] corresponds to slot 1.

					if (mlimbs_priority[i + 1] > priority)
						continue;

					punt_list[seq_punted] = i;
					seq_punted++;


					// For channel_prioritized chunks, we can steal individual channels and voices.  For non
					// channel prioritized chunks, you have to punt the entire piece.
					if (userID[i].channel_prioritize == TRUE)
						{
							for (j = 16; j > 10 ; j--)
								{
									/* If this sequence channel is currently using a physical channel...*/
									if (userID[i].sequence_channel_status[j - 10] >= 0)
										{
											num_free_channels++;
											voices_punted += xseq_info[userID[i].pieceID].channel_voices[j-10];
										}
								}
							voices_punted += xseq_info[userID[i].pieceID].channel_voices[0];
						}
					else
						{
#ifdef USE_MAX_VOICES
							voices_punted += xseq_info[userID[i].pieceID].max_voices;
#else
							voices_punted += xseq_info[userID[i].pieceID].avg_voices;
#endif
							for (j = 0; j < MLIMBS_MAX_CHANNELS; j++)
							if ((userID[i].current_channel_map>>j) & (0x0001))
								num_free_channels++;
						}
							

				}
		}

	// If there aren't enough voices or channels, then
	// do not punt anything.
	if (channel_prioritize == TRUE)
		{
			if ((voices_punted < min_voices_needed) || (num_free_channels== 0))
				return -1;
		}
	else
		{
			if ((voices_punted < voices_needed) || (num_free_channels < channels_needed))
				return -1;
		}

	// Now, punt as many voices and channels that we can
	voices_punted = 0;
	while ((voices_punted < voices_needed)  || (num_free_channels < channels_needed))
		{

			// Each time through, find the lowest priority thing in the punt list
			min = -1;
			for (i = 0; i < seq_punted; i++)
				{
					if (punt_list[i] >= 0)
						{
							if (min < 0) min = i;
							if (mlimbs_priority[punt_list[i] + 1] < mlimbs_priority[punt_list[min] + 1])
								min = i;
						}
				}

			if (min < 0)
				{
					if (channel_prioritize == TRUE)
						{
							if (voices_punted < min_voices_needed)
								return -1;
							else
								return 1;	// Its o.k. not to have all necessary channels
						}
					else
						return -1;
				}

			// For channel_prioritized chunks, we can steal individual channels and voices.  For non
			// channel prioritized chunks, you have to punt the entire piece.
			if (userID[punt_list[min]].channel_prioritize == TRUE)
				{
					for (j = 16; j > 10; j--)
						{
							if (userID[punt_list[min]].sequence_channel_status[j - 10] >= 0)
								{
									num_free_channels++;
									voices_punted += xseq_info[userID[punt_list[min]].pieceID].channel_voices[j-10];
									mlimbs_mute_sequence_channel(punt_list[min],j,FALSE);
								}
							if ((voices_punted >= voices_needed) && (num_free_channels >= channels_needed))
								break;
						}
		
					// If punting channels 11-16 wasn't enough, punt entire
					//	piece, thus freeing channel 10 voices as well
					if (j == 10)
						{
							mlimbs_punt_piece(punt_list[min]);
							voices_punted += xseq_info[userID[punt_list[min]].pieceID].channel_voices[0];
						}
					else
						{
						// If we got enough voices and channels, only punt entire
						//	piece if nothing is playing on channel 10
							for (j = 16; j > 10; j--)
								{
									if (userID[punt_list[min]].sequence_channel_status[j-10] >= 0)
										break;
								}
							if ((j == 10) && (xseq_info[userID[punt_list[min]].pieceID].channel_voices[0] <= 0))
								{
									mlimbs_punt_piece(punt_list[min]);
									voices_punted += xseq_info[userID[punt_list[min]].pieceID].channel_voices[0];
								}
						}
				}

			else
				{
					for (j = 0; j < MLIMBS_MAX_CHANNELS; j++)
						if ((userID[punt_list[min]].current_channel_map >> j) & (0x0001))
							num_free_channels++;
#ifdef USE_MAX_VOICES
					voices_punted += xseq_info[userID[punt_list[min]].pieceID].max_voices;
#else
					voices_punted += xseq_info[userID[punt_list[min]].pieceID].avg_voices;
#endif
					mlimbs_punt_piece(punt_list[min]);

				}

			punt_list[min] = -1;
		}
	return 1;
}

/////////////////////////////////////////////////////////////////
// int mlimbs_assign_channels
//
//	purpose:
//		This routine will mark the userID[].sequence_channel_status[] array
//		according to what sequence channels need to use.  It then maps as
//		many sequence channels as it can to physical	channels, in order,
//    from 11 to 16, unless the crossfade argument is TRUE.
//
//	inputs:
//		int usernum
//		bool	crossfade	// If TRUE, all the sequence channels used by
//		      // the sequence are set to SEQUENCE_CHANNEL_MUTED so crossfade
//          // code can unmute channels as the sequence is crossfaded in.
//
/////////////////////////////////////////////////////////////////
int mlimbs_assign_channels (int usernum,bool crossfade)
{
	uint j;
	ushort c_map;

	if (usernum >= (MLIMBS_MAX_SEQUENCES - 1))
		return -1;
	c_map = xseq_info[userID[usernum].pieceID].channel_map;
	if (c_map == 0) return 1;	// If no channels are requested from dynamic channel allocation, return.
	for (j = 11; j < 17; j++)
	{
		if ((c_map >> (j - 1)) & 0x0001)
		{	// Mark the sequence channel as waiting for a physical channel
			// unless the sequence is to be crossfaded in, in which
		 	// case, mute all channels.
			if (crossfade == FALSE)
			{
				userID[usernum].sequence_channel_status[j - 10] = SEQUENCE_CHANNEL_PENDING;
				if (mlimbs_unmute_sequence_channel(usernum,j) < 0)
				{
					if (userID[usernum].channel_prioritize == FALSE)
						return -1;
				}
			}
   		else
				userID[usernum].sequence_channel_status[j - 10] = SEQUENCE_CHANNEL_MUTED;
		}
		else
			userID[usernum].sequence_channel_status[j-10] = SEQUENCE_CHANNEL_UNUSED;
	}
	return 1;
}

/////////////////////////////////////////////////////////////////
// int mlimbs_play_piece
//
//	purpose:
//		This routine will begin playback of a single 'snippet' in 
//		the theme.
//
//	inputs:
//		int pieceID			Index into the xseq_info array.  
//		int priority		Priority of the request.  If < 0, then
//								use the default priority (in xseq_info[]).
//		int loops			Number of times to play the piece
//		int rel_vol			Relative volume to start the piece at.  This will
//								be set to 0 for pieces that will ramp up.
//		bool channel_prioritize		- if TRUE, then its ok to play the
//								chunk even if not enough channels are available,
//								If FALSE, then only play chunk if all channels can
//								be played.
//		bool crossfade
//
//	return:
//		-1	if failed
//		usernum	- an integer index into the userID[] array
/////////////////////////////////////////////////////////////////

int mlimbs_play_piece (int pieceID, int priority, int loops, int rel_vol, bool channel_prioritize,bool crossfade)
{
	int i, slot, usernum, voices_needed, voices_available;

	if (pieceID >= num_XMIDI_sequences)	return -1;
	if (loops == 0) return 1;

   //secret_sprint((ss_temp,"play piece %d %d %d %d %d %d\n",pieceID, priority,loops,rel_vol,channel_prioritize,crossfade));

   // Check for duplicate piece IDs?
   if (cpl_num < 10)
   {	
      for (i=0; i < cpl_num; i++)
         if (curr_play_list[i] == pieceID)
         {
            //secret_sprint((ss_temp,"cur play list %d already has %d\n",i,pieceID));
            return 1;
         }
   }
   else
      Warning(("BADNESS!  cpl_num = %d!\n"));

	if (priority < 0)
		priority = xseq_info[pieceID].priority;

	voices_needed = 0;
	voices_available = max_voices - voices_used;

	// First, free as many voices and channels as possible.
	if (channel_prioritize)
	{  // Only need to count # of voices needed if we're not crossfading.  since channel 10 voices
		// are used by mlimbs_channel_prioritize as the # of voices needed if we are crossfading
		if (crossfade != TRUE)
		{
			for (i = 0, voices_needed = 0; i < 7; i++)
				voices_needed += xseq_info[pieceID].channel_voices[i];
			voices_needed -= voices_available;
		}
		if (mlimbs_channel_prioritize(priority,pieceID,voices_needed,crossfade,TRUE) < 0)
			return -1;
	}
	else
	{
#ifdef USE_MAX_VOICES
		voices_needed = -(voices_available - xseq_info[pieceID].max_voices);
#else
		voices_needed = -(voices_available - xseq_info[pieceID].avg_voices);
#endif
		if (mlimbs_channel_prioritize(priority,pieceID,voices_needed,crossfade,FALSE) < 0)
			return -1;
	}


	/***********************************/
	/* Now, attempt to load the piece. */
	/***********************************/
   slot = snd_sequence_play(mrefBuild(mlimbs_cur_theme_id,pieceID+1),mlimbs_theme,pieceID+1,NULL);
	usernum = slot - 1;
   //secret_sprint((ss_temp,"play piece got slot %d for %d\n",slot,pieceID));
	if (slot >= 0)
	{
		userID[usernum].pieceID = pieceID;              // What is going on here?
		userID[usernum].seq_id = slot;
		userID[usernum].channel_prioritize = channel_prioritize;
		userID[usernum].crossfade_status = 11;
		// Note: tallying of voice usage is now done in mlimbs_assign channels.  Only add channel 10 here
		voices_used += xseq_info[pieceID].channel_voices[0];
      if (cpl_num < 10)   curr_play_list[cpl_num++] = pieceID;
		if (mlimbs_assign_channels(usernum,crossfade) == -1)
		{  /* Try and assign the channels and voices freed above */
			mlimbs_punt_piece(usernum);
			return -1;
		}
		// Now set the initial volume
		AIL_set_sequence_volume (_uiD_seq(usernum),(unsigned) rel_vol * master_volume / 100,0);
		return (usernum);	// Return entry of userID[]
	}
	else
		return -1;
}

// CHANGE - now resets pieceID as well (inside clearUid)
//  we can clearly change it back if we want to

// void mlimbs_punt_piece
// 	This routine will stop playback of a single 'snippet' in the theme.
//	inputs: usernum			Index into the userID array.
void mlimbs_punt_piece (int usernum)
{
	int i, slot, ch;

	if (userID[usernum].pieceID < 0 || userID[usernum].pieceID >= num_XMIDI_sequences)
      return;

	slot = usernum + 1;

   //secret_sprint((ss_temp,"looking to punt slot %d (unum %d)\n",slot,usernum));

	if (slot >= 0)
	{
      if (cpl_num < 10)
      {
         for (i=0; i < cpl_num; i++)
         {
            if (curr_play_list[i] == userID[usernum].pieceID)
            {
               if (i != (cpl_num -1))
               {
                  curr_play_list[i] = curr_play_list[cpl_num - 1];
                  loop_list[i] = loop_list[cpl_num - 1];
               }
               cpl_num--;
            }
         }
      }

      //secret_sprint((ss_temp,"end seq it is\n",slot,usernum));
      snd_end_sequence(slot);

		for (i = 11; i < 17; i++)
		{
			ch = userID[usernum].sequence_channel_status[i-10];
			if (ch >= 0)
			{
				voices_used -= xseq_info[userID[usernum].pieceID].channel_voices[i-10];
				channel_info[ch].usernum = -1;
				channel_info[ch].sequence_channel = -1;
				if (channel_info[ch].status == MLIMBS_PLAYING_PIECE)
					channel_info[ch].status = MLIMBS_STOPPED;
			}
   		userID[usernum].sequence_channel_status[i-10] = SEQUENCE_CHANNEL_UNUSED;
		}
		voices_used -= xseq_info[userID[usernum].pieceID].channel_voices[0];
      _mlimbs_clear_uid(usernum);
	}
}

////////////////////////////////////////////////////////////////
// mlimbs_get_crossfade_status
//
//	purpose:
//		Given a chunkID, it returns its current crossfade_status.
//		Whether it is getting crossfaded in or out depends on
//		the crossfade field in the current_request structure.  It
//		basically searches the array of userID[]'s for a userID that
//		is currently playing the given chunk.
//
//	inputs:
//		int pieceID		- id of the chunk to get the status of
//	outputs:
//		>= 0				- the correct crossfade_status 
//									0 - not crossfading)
//									10-16	- next channel to be crossfaded
//							
//		< 0				- the chunk is not currently playing.
//
///////////////////////////////////////////////////////////////

schar mlimbs_get_crossfade_status (int pieceID)
{
	int i;

	for (i = 0; i < MLIMBS_MAX_SEQUENCES - 1; i++)
		if (userID[i].pieceID == pieceID)
			return userID[i].crossfade_status;

	return -1;
}

////////////////////////////////////////////////////////////////
//	mlimbs_callback
//
//	purpose:
//		This is the routine that gets called by an XMIDI controller
//		119.  All the mlimbs_callback does is call the mlimbs_AI, and
//		then set mlimbs_update_requests to TRUE, which tells the mlimbs_timer_callback
//		to actual execute its code, when next it is called.
//
////////////////////////////////////////////////////////////////
#pragma disable_message(202)
void cdecl mlimbs_callback (snd_midi_parms *mprm, unsigned trigger_value)
{
   if (trigger_value)
   {
      mlimbs_update_requests = TRUE;
   }
   else
   {
      if (mlimbs_AI != NULL)   // This routine accesses the music AI which should set the structures in
      {
   		mlimbs_AI();          // current_request[] to tell mlimbs which pieces should be playing.
/*KLC
	      {
            extern schar curr_crossfade;
            extern int new_theme;
            extern uchar decon_count , decon_time ;
				extern bool in_deconst , old_deconst ;
            secret_sprint((ss_temp, "note in %d old %d, count %d time %d, cf %d, nt %d\n",
               in_deconst, old_deconst, decon_count, decon_time, curr_crossfade, new_theme));
	      }
*/
      }
   }
}
#pragma enable_message(202)

#ifdef COW
// the key, here, is we need to update all sequences which are done
//  ie. clear out their state and such....
void cdecl mlimbs_seq_done_call(snd_midi_parms *seq)
{
}
#endif

//////////////////////////////////////////////////////////////////////////////
// mlimbs_reassign_channels(void)
//
//	This routine assigns currently free channels to chunks that are playing,
//	and are lacking channels.
//
/////////////////////////////////////////////////////////////////////////////

void mlimbs_reassign_channels (void)
{
	int i,j;
	int highest;
	schar checked[7];

	for (i = 0; i < 7; i++)
		checked[i] = 0;

	highest = -1;
	do
		{
			// Check if the current highest priority chunk needs channels
			if (highest >= 0)
				{
					for (j = 11; j < 17; j++)
						{
							if (userID[highest].sequence_channel_status[j-10] == SEQUENCE_CHANNEL_PENDING)
								break;
						}
					if (j == 17)
						{
							checked[highest] = -1;
							highest = -1;
						}
				}

			// If the previous highest priority chunk needs no more channels, or
			//	a highest priority chunk hasn't been found yet, get the highest
			//	priority chunk

			if (highest < 0)
				{
					for (i = 0; i < MLIMBS_MAX_SEQUENCES - 1; i++)
						{
							// -1 means that this chunk doens't need more channels 
							if (checked[i] == -1)
								continue;
							if (userID[i].pieceID != -1)
								{
									/* Make sure the chunk needs a channel */
									if (checked[i] == 0)
										{
											for (j = 11; j < 17; j++)
												{
													if (userID[i].sequence_channel_status[j-10] == SEQUENCE_CHANNEL_PENDING)
														break;
												}
											if (j == 17)
												{
													checked[i] = -1;
													continue;
												}
											else
												checked[i] = 1;	// 1 means that this chunk needs channels
										}

									if (highest < 0)
										highest = i;
									else
										{
											if (mlimbs_priority[i+1] > mlimbs_priority[highest + 1])
												highest = i;
										}
								}
						}
				}

			if (highest >= 0)
				{
					if (mlimbs_unmute_sequence_channel(highest, j) < 0)
						break;	// Ran out of channels or voices to assign, so exit from the loop
				}
			else
				break;
		} while (1);
}

#pragma disable_message(202)
LONG cdecl mlimbs_timbre_callback(MDI_DRIVER *mdi, LONG bank, LONG patch)
{  // dont allow the timbre to load
   return 1;
}
#pragma enable_message(202)

////////////////////////////////////////////////////////////////
//	mlimbs_timer_callback
//
//	This timer callback is necessary since starting and stopping
//	XMIDI sequences from within an XMIDI callback is not a good
//	thing to do.  It is currently called at 100 hz.  Whenever it
//	is called, it first checks whether mlimbs_update_requests has
//	been set to TRUE by mlimbs_callback and whether mlimbs_semaphore
//	is FALSE.  If both the above conditions are met, then mlimbs_timer_callback
//	executes the main body of its code, then resets mlimbs_update_requests
//	to FALSE and increments the mlimbs_counter.
//
////////////////////////////////////////////////////////////////
void mlimbs_timer_callback (void)
{
	int i,j,k,loop,rvol;
	int usernum;
	
	if (mlimbs_update_requests == FALSE)
	{
		return;
	}
	if (mlimbs_semaphore == TRUE)
	{
		return;
	}

/*KLC
   // show current requests
   for (i=0; i < MLIMBS_MAX_SEQUENCES -1; i++)
      if (current_request[i].pieceID != -1)
      {
         secret_sprint((ss_temp,"request %d is %d (%d %d %d)\n",i,current_request[i].pieceID,
           current_request[i].ramp,current_request[i].crossfade,current_request[i].ramp_time));
      }
*/

	// First punt everything that is not requested.  Also crossfade out pieces here.
	k = 0;
	for (i = 0; i < MLIMBS_MAX_SEQUENCES - 1; i++)
	{
			if (userID[i].pieceID != -1)
			{
				for (j = 0; j < MLIMBS_MAX_SEQUENCES - 1; j++)
				{
					if (current_request[j].pieceID == userID[i].pieceID)
					{	// Begin rampout of this chunk, if desired.
						if (current_request[j].ramp < 0)
						{
							if (userID[i].rel_vol != 0)	// Check whether this chunk is already being ramped out.
							{
								userID[i].rel_vol = 0;
								AIL_set_sequence_volume (_uiD_seq(i),0,current_request[j].ramp_time);
                        //secret_sprint((ss_temp,"start ramp out for %d (req %d)\n",i,j));
							}
						}
                  if (cpl_num < 10)
                  {
                     for (loop = 0; loop < cpl_num; loop++)
                     {
                        if (current_request[j].pieceID == curr_play_list[loop])
                           loop_list[loop] = current_request[j].loops;
                     }
                  }
						if (current_request[j].loops > 0)
							current_request[j].loops--;
						if (current_request[j].loops == 0)
						{
                     //secret_sprint((ss_temp,"punt loops over for %d (req %d)\n",i,j));
							mlimbs_punt_piece(i);      // i, j, and k, eh?
                     _mlimbs_clear_req(j);
							k++;
						}
   					//	Crossfade out pieces here, so that their channels become
						//	available to pieces to be crossfaded in, later in the callback.
						else if (userID[i].channel_prioritize == TRUE)
						{
							if (current_request[j].crossfade < 0)
						   {
							   if (userID[i].crossfade_status > 16) userID[i].crossfade_status = 16;
   							while (userID[i].crossfade_status > 10)
   							{
									if (userID[i].sequence_channel_status[userID[i].crossfade_status - 10] >= 0)
									{
										mlimbs_mute_sequence_channel(i,userID[i].crossfade_status,TRUE);
										userID[i].crossfade_status--;
										break;
									}
									else if (userID[i].sequence_channel_status[userID[i].crossfade_status - 10] == SEQUENCE_CHANNEL_PENDING)
									{
										userID[i].sequence_channel_status[userID[i].crossfade_status - 10] = SEQUENCE_CHANNEL_MUTED;
										userID[i].crossfade_status--;
										break;
									}
									userID[i].crossfade_status--;
								}
                        //secret_sprint((ss_temp,"crossfade fun for %d (req %d) got %d\n",i,j,userID[i].crossfade_status));
   						}
         			}
         			break;
         		}
         	}
				if (j == MLIMBS_MAX_SEQUENCES - 1)	// If we get here, userID[i] matches none of the current_requests[]
				{
               //secret_sprint((ss_temp,"gonna punt: no current request for %d (seq %d)\n",userID[i].pieceID,i));
					mlimbs_punt_piece(i);
					k++;
				}
			}
		}		                           // Reassign channels to chunks that are missing channels
		mlimbs_reassign_channels();		// Play everything on the play list
		k = 0;
		for (i = 0; i < MLIMBS_MAX_SEQUENCES - 1; i++)
		{
   		if (current_request[i].pieceID >= 0)
			{
				if (current_request[i].pieceID >= num_XMIDI_sequences)
					continue;
				// Don't play empty sequences
				if (xseq_info[current_request[i].pieceID].channel_map == 0)
					continue;
				/* Make sure the piece isn't already playing */
				for (j = 0; j < MLIMBS_MAX_SEQUENCES - 1; j++)
				{	// If the piece is playing, see if we should fade any channels in.
					if (userID[j].pieceID == current_request[i].pieceID)
					{
						if (current_request[i].crossfade > 0)
						{
							if (userID[j].crossfade_status < 11) userID[j].crossfade_status = 11;
							while (userID[j].crossfade_status < 17)
							{
								if (userID[j].sequence_channel_status[userID[j].crossfade_status - 10] == SEQUENCE_CHANNEL_PENDING)
									break;
								else if (userID[j].sequence_channel_status[userID[j].crossfade_status - 10] == SEQUENCE_CHANNEL_MUTED)
									userID[j].sequence_channel_status[userID[j].crossfade_status - 10] = SEQUENCE_CHANNEL_PENDING;
								else
									userID[j].crossfade_status++;
							}
 	                  //secret_sprint((ss_temp,"%d already playing [%d req %d] cross %d\n",userID[j].pieceID,j,i,userID[j].crossfade_status));
            		}
						break;
					}
				}
	   		if (j < MLIMBS_MAX_SEQUENCES - 1)
               //secret_sprint((ss_temp,"no dup seq, new, bases %d %d, n_m %d for %d\n",
                //  mlimbs_counter, num_master_measures, xseq_info[current_request[i].pieceID].num_measures, current_request[i].pieceID));
// I took this out in order to solve the layering timing problem -- Rob F.
//					continue;
				/* For now, only start playing a piece if the number of measures played is a multiple
   				of the measure size of the piece. */
			 	if ((mlimbs_counter * num_master_measures) % xseq_info[current_request[i].pieceID].num_measures == 0)
			 	{ 	// If the chunk is going to be ramped in, set its initial volume to 0.
					if (current_request[i].ramp > 0) rvol = 0;
					else                             rvol = current_request[i].rel_vol;
					usernum = mlimbs_play_piece(current_request[i].pieceID,current_request[i].priority,current_request[i].loops,
    							 	   rvol, current_request[i].channel_prioritize,(current_request[i].crossfade > 0));
               //secret_sprint((ss_temp,"brought in %d [%d req %d] crossfade %d got %d (rv %d)\n",current_request[i].pieceID,j,i,current_request[i].crossfade,usernum,rvol));
					if (usernum >= 0)
					{
						userID[usernum].rel_vol = current_request[k].rel_vol;

						if (current_request[i].crossfade > 0)
							userID[usernum].crossfade_status = 11;	// If we're crossfading in, crossfade starting with channel 11
						else
							userID[usernum].crossfade_status = 16;	// If we're not crossfading in, set crossfade_status to 16
																				// so that crossfading out will work.
                  // wait, what if we _are_ crossfading out?
						if (current_request[i].ramp > 0)
							AIL_set_sequence_volume(_uiD_seq(usernum),(unsigned) (userID[usernum].rel_vol * master_volume / 100),
                          						   current_request[i].ramp_time);
						else
							AIL_set_sequence_volume(_uiD_seq(usernum),(unsigned) (userID[usernum].rel_vol * master_volume / 100),0);
					}
				}
			}
		}
		mlimbs_update_requests = FALSE;
		mlimbs_counter++;	// Only update the counter after the pieces playing have been updated
}

SEQUENCE *_mlimbs_get_a_seq(void)
{
   extern int snd_find_free_sequence(uchar smp_pri, bool check_only);
   SEQUENCE *S;
   int seq_id;
   if ((seq_id=snd_find_free_sequence(1000,FALSE))==SND_PERROR)
      return NULL;
   S=snd_sequence_ptr_from_id(seq_id);
   return S;
}

extern bool run_asynch_music_ai;

// scan through all requested pieces, if not already playing, init_sequence them
void mlimbs_preload_requested_timbres(void)
{
   char *old;
   int i, j, piece;
   SEQUENCE *S;
//   mprintf("preload requested, stat %d them %x\n",mlimbs_status,mlimbs_theme);
   if ((mlimbs_status==0)||(mlimbs_theme==NULL)) return;
   if ((S=_mlimbs_get_a_seq())==NULL)
      Warning(("No Seq for preload\n"));
   old=AIL_register_timbre_callback((MDI_DRIVER *)snd_midi,NULL);
//   mprintf("Old is %x, set to null\n",old);
   for (i = 0; i < MLIMBS_MAX_SEQUENCES - 1; i++)
   {
      // check if it is already playing before reloading it, eh?
      piece=current_request[i].pieceID;
   	if ((piece>=0)&&(piece<num_XMIDI_sequences))
	   	if (xseq_info[piece].channel_map != 0)       // Don't bother wit empty sequences
         {
				for (j = 0; j < MLIMBS_MAX_SEQUENCES - 1; j++)
					if (userID[j].pieceID == piece)
               {
//                  mprintf("but we already have it...\n");
                  break;
               }
            if (j==MLIMBS_MAX_SEQUENCES - 1)
            {
		         AIL_init_sequence(S,mlimbs_theme,piece+1);
//               mprintf("Hey bob, requesting %d to load\n",piece);
            }
         }
   }
   old=AIL_register_timbre_callback((MDI_DRIVER *)snd_midi,mlimbs_timbre_callback);
//   mprintf("Old is %x, set to mtc\n",old);
}

void mlimbs_preload_full_timbres_and_go_asynch(void)
{
   int piece;
   SEQUENCE *S;
   char *old;
//   mprintf("preload full go asynch, stat %d them %x\n",mlimbs_status,mlimbs_theme);
   if ((mlimbs_status==0)||(mlimbs_theme==NULL)) return;
   if ((S=_mlimbs_get_a_seq())==NULL)
      Warning(("No Seq for full preload\n"));
   old=AIL_register_timbre_callback((MDI_DRIVER *)snd_midi,NULL);
//   mprintf("Old is %x, set to null\n",old);
   for (piece=0; piece<num_XMIDI_sequences; piece++)
   	if (xseq_info[piece].channel_map != 0)
	      AIL_init_sequence(S,mlimbs_theme,piece+1);
   old=AIL_register_timbre_callback((MDI_DRIVER *)snd_midi,mlimbs_timbre_callback);
//   mprintf("Old is %x, set to mtc\n",old);
   run_asynch_music_ai=TRUE;
}

void mlimbs_return_to_synch(void)
{
//   mprintf("return to synch\n");
   run_asynch_music_ai=FALSE;
}

///////////////////////////////////////////////////////
// mlimbs_change_master_volume
//
//	master_volume_scheme:
//		Here's how the master volume scheme works.  mlimbs
//		has an overall master_volume which is given as a
//		percentage.  This is normally set to 100.  Each
//		sequence that is playing has a relative volume (the
//		rel_vol field in the userID structures).  Whenever
//		the master_volume or a sequence relative volume
//		is changed, a call to AIL_set_relative_volume is
//		made for each affected sequence.  The volume is
//		passed to AIL_set_relative_volume as a percentage
//		by which all the actual XMIDI sequence volume controller
//		values are multiplied.  The percentage passed =
//		master_volume * userID[].rel_vol.
//		Since changing the master_volume itself affects all
//		sequences, a call to AIL_set_relative_volume is
//		made for all currently playing sequences.
//
//	inputs:
//		int vol		
//
///////////////////////////////////////////////////////
void mlimbs_change_master_volume (int vol)
{
	int i;
	int percent;

	master_volume = vol;
	for (i = 0; i < (MLIMBS_MAX_SEQUENCES - 1); i++)
	{
		percent = userID[i].rel_vol * master_volume / 100;
		if (userID[i].seq_id>=0)
		{
			AIL_set_sequence_volume (_uiD_seq(i),(unsigned)percent,0);
		}
	}
}

#endif //NOT_YET���
