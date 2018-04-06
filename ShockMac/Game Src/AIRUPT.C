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
 * $Source: r:/prj/cit/src/RCS/airupt.c $
 * $Revision: 1.10 $
 * $Author: dc $
 * $Date: 1994/11/19 20:35:14 $
 */

// The interrupt-crunchy part of the music ai.

#include <stdlib.h>

#include "MacTune.h"
#include "musicai.h"
#include "player.h"
#include "map.h"
#include "mapflags.h"
#include "tools.h"

extern errtype musicai_reset(bool runai);
extern errtype musicai_shutdown();

struct mlimbs_request_info default_request =
   { 0, 0, 4, 100, 0, 64, TRUE, 0, 0 };

extern int ext_rp ;
extern char mlimbs_machine;
extern char cyber_play;

#define NUM_SNDMIDIDEVICES		2
#define NUM_SNDMIDICHANNELS	16
#define NUM_SNDSPEECHCHANNELS	32
#define NUM_SNDSOUNDSOURCES	1

#define NUM_SCORES                  8
#define NUM_LAYERABLE_SUPERCHUNKS   22
#define FIRST_SUPERCHUNK_LAYER      16
#define NUM_LAYERS                  32
#define LAYER_BASE                  32

#define SUPERCHUNKS_PER_SCORE       4
//#define NUM_TRANSITIONS           8
#define MAX_KEYS                    10
#define KEY_BAR_RESOLUTION          2

#define NUM_PARK_SOUNDS             10
#define PARK_LAYER_BASE             32

#define CYBERSPACE_SCORE_BASE       10
#define NUM_NODE_THEMES             2

#define DANGER_LAYER_BASE           10    // actually one less than Danger1 since a minimum of 1 gets added to it...
#define SUCCESS_LAYER_BASE         (DANGER_LAYER_BASE + 2)
#define DECONSTRUCT_LAYER           15
#define TRANSITION_LAYER_BASE       16

#define STANDARD_RAMP_TIME          2000
#define SHORT_RAMP_TIME             1

#define PITCHBEND_CHUNK             15

extern uchar track_table[NUM_SCORES][SUPERCHUNKS_PER_SCORE]; 
extern uchar transition_table[NUM_TRANSITIONS];
extern uchar layering_table[NUM_LAYERS][MAX_KEYS];
extern uchar key_table[NUM_LAYERABLE_SUPERCHUNKS][KEY_BAR_RESOLUTION];

extern char peril_bars;

extern int new_theme;
extern int new_x,new_y;
extern int old_bore;
extern short mai_override ;

extern int layer_danger ;
extern int layer_success ;
extern int layer_transition ;
extern int transition_count ;
extern char tmode_time ;
extern int actual_score ;
extern uchar decon_count ;
extern uchar decon_time ;
extern bool in_deconst , old_deconst ;
extern bool in_peril ;
extern bool just_started ;
extern int score_playing ;
extern short curr_ramp_time, curr_ramp;
extern char curr_prioritize, curr_crossfade;

extern errtype mai_transition(int new_trans);
extern errtype make_request(int chunk_num, int piece_ID);
extern int digifx_volume_shift(short x, short y, short z, short phi, short theta, short basevol);
extern int digifx_pan_shift(short x, short y, short z, short phi, short theta);
extern bool mai_semaphor;

extern uchar park_random ;
extern uchar park_playing ;
extern uchar access_random ;

extern ulong last_damage_sum ;
extern ulong last_vel_time ;

// Damage taken decay & quantity of decay
extern int danger_hp_level ;
extern int danger_damage_level ;
extern int damage_decay_time ;
extern int damage_decay_amount ;
extern int mai_damage_sum ;

// How long an attack keeps us in combat music mode
extern int mai_combat_length ;

extern bool bad_digifx;

extern bool mlimbs_semaphore;

bool run_asynch_music_ai = FALSE;
bool mai_semaphor = FALSE;

errtype check_asynch_ai(bool new_score_ok);
void grind_music_ai(void);

void music_ai()
{
//   mlimbs_semaphore = TRUE;
   ai_cycle = TRUE;
//   if ((run_asynch_music_ai) && (!mai_semaphor))
   if (!mai_semaphor)
   {
      mai_semaphor = TRUE;
      check_asynch_ai(FALSE);
      mai_semaphor = FALSE;
      ai_cycle = FALSE;
   }
}

void grind_credits_music_ai(void)
{
   int i;
   if (mai_semaphor)
   {
      current_request[0] = default_request;
      current_request[0].pieceID = mlimbs_boredom;
   }
   else
      make_request(0, mlimbs_boredom);
//   if (mlimbs_counter == 4)						//KLC - Was 16
//      mlimbs_counter = 0;
//   if ((mlimbs_counter % 4) == 3)
//   {
      mlimbs_boredom++;
      if (mlimbs_boredom == 8)
         mlimbs_boredom = 0;
//   }
   // Clear out other requests
   for (i=1; i < MLIMBS_MAX_CHANNELS -1; i++)
      current_request[i].pieceID = 255;						// KLC - was -1
}


#define MAIN_THEME_WEIGHT 0
void grind_music_ai(void)
{
   int i, open_track, r;
   int current_key;
   short play_me = -1, seq;

   if (mlimbs_counter == 4)		//KLC - was 16
   {
      mlimbs_counter = 0;
      boring_count++;
   }

   // Set the default values for the chunk parameters this time around
   if (in_deconst)
   {
      decon_count++;
      if (decon_count > decon_time)
         curr_crossfade = 0;
      else
      	curr_crossfade = 1;
   }
   else if (old_deconst)
   {
   	curr_crossfade = -1;
      decon_count = 0;
   }
   else
   {
   	curr_crossfade = 0;
      decon_count = 0;
   }

   if (new_theme > 1)
   {
      new_theme--;
      curr_ramp = -1;
      curr_ramp_time = STANDARD_RAMP_TIME;
   }
   else if (new_theme==1)
   {
      new_theme = -1;
   }
   else if (new_theme == -2)
   {
      new_theme = 0;
      curr_ramp = 1;
      curr_ramp_time = STANDARD_RAMP_TIME;
   }
   else if (new_theme != -1)
   {
      curr_ramp = 0;
      curr_ramp_time = 0;
   }

   if (score_playing == ELEVATOR_ZONE)
   {
      curr_crossfade = 0;
      in_deconst = FALSE;
      old_deconst = FALSE;
      grind_credits_music_ai();
      return;
   }
   if (current_transition == TRANS_DEATH)
   {
      make_request(0, transition_table[TRANS_DEATH]);
      return;
   }

   // Change  major score if required
   if (!mai_semaphor)
   {
      // We need a hack for inner bridge.  Inner bridge?  But I just met her!
      if ((mlimbs_combat) && ((actual_score == PERIL_SCORE) || (actual_score == COMBAT_SCORE)) && (peril_bars > 2))
      {
         current_score = COMBAT_SCORE;
      }
      else if ((mlimbs_peril > PERIL_THRESHOLD) || mlimbs_combat)
      {
         current_score = PERIL_SCORE;
      }
      else
         current_score = WALKING_SCORE;
   }
   switch (actual_score)
   {
      case COMBAT_SCORE:
         break;
      case PERIL_SCORE:
         peril_bars++;
         break;
      default:
         peril_bars = 0;
         break;
   }


   // Minor change maybe
   if (!mai_semaphor)
   {
      if ((boring_count > 0) && (mlimbs_counter == 0))
      {
         int rand_poss = 0;
         int max_rand;
         if (mlimbs_boredom > 0)
            mlimbs_boredom = 0;
         else
         {
            if (actual_score == WALKING_SCORE)
               max_rand = 3;
            else 
               max_rand = 1;
            for (i=1; i<=max_rand; i++)
               if (track_table[actual_score + i][0] != 255)
                  rand_poss++;
            ext_rp = rand_poss;
            if (rand_poss > 0)
            {
               mlimbs_boredom = (rand() % (rand_poss + MAIN_THEME_WEIGHT + 1)) - MAIN_THEME_WEIGHT;
               if (mlimbs_boredom < 0)
                  mlimbs_boredom = 0;
            }
            else
               mlimbs_boredom = 0;
            if (track_table[actual_score + mlimbs_boredom][0] == 255)  //KLC - was -1
               mlimbs_boredom = 0;
         }
      }
   }

   // Major score transition?
   if ((last_score != current_score) && (!mai_semaphor))
   {
      boring_count = 0;
      switch(current_score)
      {
         case WALKING_SCORE:
            if (last_score == PERIL_SCORE)
               mai_transition(TRANS_PERIL_TO_WALK);
            else if (last_score == COMBAT_SCORE)
               mai_transition(TRANS_COMB_TO_WALK);
            break;
         case PERIL_SCORE:
            if (last_score == WALKING_SCORE)
               mai_transition(TRANS_WALK_TO_PERIL);
            else if (last_score == COMBAT_SCORE)
               mai_transition(TRANS_COMB_TO_PERIL);
            break;
         case COMBAT_SCORE:
            if (last_score == WALKING_SCORE)
               mai_transition(TRANS_WALK_TO_COMB);
            else if (last_score == PERIL_SCORE)
               mai_transition(TRANS_PERIL_TO_COMB);
            break;
      }

      // if we aren't doing a layered transition, just jump
      // over to new score if possible
      if (transition_count == 0)
      {
//         if (mlimbs_counter % 4 == 0)		KLC - do it every time
//         {
            mlimbs_boredom = 0;
            last_score = current_score;
            actual_score = current_score;
//         }
      }
   }

   if ((next_mode) /*&& ((mlimbs_counter %  4) == 0)*/)		//KLC - do it every time.
   {
      // If we are coming out of death, shut down music
      // Eventually this will segue back into the journey screen music, perhaps.
      {
         current_mode = next_mode;
         next_mode = 0;
      }
   }
   switch (current_mode)
   {
      case TRANSITION_MODE:
         if (!mai_semaphor)
         {
            play_me = transition_table[current_transition];
            tmode_time--;
            if (tmode_time == 0)
            {
               next_mode = NORMAL_MODE;
               mlimbs_counter = 3;  // so that next_mode will take effect next ai loop   KLC - was 15
            }
         }
         break;
      case NORMAL_MODE:
         // Play basic superchunk
         seq = mlimbs_counter;		//KLC - was mlimbs_counter / 4
         play_me = track_table[actual_score + mlimbs_boredom][seq];

         break;
   }
   open_track = 0;

/* KLC - no pb12 track in our stuff
   // Play the pitchbend track if we are just starting out
   if ((just_started) && (!mai_semaphor))
   {
      make_request(open_track++, PITCHBEND_CHUNK);
      just_started = FALSE;
   }
*/

   if (mai_semaphor)
   {
      current_request[0] = default_request;
      current_request[0].pieceID = play_me;
   }
   else
   {
      if (global_fullmap->cyber)
      {
/* KLC - moved to mlimbs_do_ai.
         MapElem *pme;

         // Deal with pitch bend??
         pme = MAP_GET_XY(PLAYER_BIN_X, PLAYER_BIN_Y);
         if (!me_bits_peril(pme))
            play_me = NUM_NODE_THEMES + me_bits_music(pme);
         else
            play_me = me_bits_music(pme);
         if (play_me != cyber_play)
         {
            musicai_shutdown();
            make_request(open_track++, play_me);
            musicai_reset(FALSE);
            MacTuneStartCurrentTheme();
         }
         else
            make_request(open_track++, play_me);
         cyber_play = play_me;
*/
      }
      else
      {
         if (decon_count < decon_time)
         {
            make_request(open_track++,play_me);
         }

         // Don't layer over fullmode transitions
         if (current_mode == NORMAL_MODE)
         {
            current_key = key_table[play_me][mlimbs_counter / 2] - 1;		// was (mlimbs_counter % 4) / 2

            // Most layering is mutually exclusive with deconstructing
            if (in_deconst && (layering_table[DECONSTRUCT_LAYER][current_key] != 0xFF))
            {
               make_request(open_track++,layering_table[DECONSTRUCT_LAYER][current_key]);
//               if (layering_table[DECONSTRUCT_LAYER][current_key] == 0xFF)
//                  Warning(("playing decon layer, %d!\n",layering_table[DECONSTRUCT_LAYER][current_key]));
            }
            else
            {
               // Do layering
               // Monsters...
               if (actual_score != COMBAT_SCORE)
               {
                  if ((play_me < NUM_LAYERABLE_SUPERCHUNKS) && (mlimbs_monster != NO_MONSTER) &&
                     (layering_table[mlimbs_monster][current_key] != 0xFF))
                  {
                     make_request(open_track++,layering_table[mlimbs_monster][current_key]);
                  }
               }

               // Object-based "machine" layers
//               if (mlimbs_machine != 0)
//                  Warning(("mlimbs_machine = %d  table = %d!\n",mlimbs_machine,layering_table[mlimbs_machine][current_key]));
               if ((mlimbs_machine != 0) && (layering_table[mlimbs_machine][current_key] != 255))
               {
            		//KLC - no layering, force to track 0.
                  make_request(0,layering_table[mlimbs_machine][current_key]);
               }
	
               // Transitions...
               if (transition_count > 0)
               {
                  if (layering_table[TRANSITION_LAYER_BASE + current_transition][current_key] != 255)
                  {
//KLC                     if ((((mlimbs_counter % 4) == 0) && (transition_count == 2)) ||
//KLC                        (((mlimbs_counter % 4) == 1) && (transition_count == 1)))
                     {
                        if (current_score == actual_score)
                        {
                           transition_count = 0;
                        }
                        else
                        {
                           make_request(open_track++,layering_table[TRANSITION_LAYER_BASE + current_transition][current_key]);
                           transition_count--;
                           if (transition_count == 0)
                           {
                              last_score = actual_score = current_score;
                              mlimbs_counter = 3;		//KLC was 15
                              mlimbs_boredom = 0;
                           }
                        }
                     }
                  }
               }

               // Feedback (Danger & Success)
               if (layer_danger && (layering_table[DANGER_LAYER_BASE + layer_danger][current_key] != 255))
               {
                  make_request(open_track++,layering_table[DANGER_LAYER_BASE + layer_danger][current_key]);
               }
               if (layer_success && (layering_table[SUCCESS_LAYER_BASE + layer_success][current_key] != 255))
               {
                  make_request(open_track++,layering_table[SUCCESS_LAYER_BASE + layer_success][current_key]);
               }
            }

            // Some layers always play, independant of deconstruct

            // Current park sounds occur in peril, combat, and walking.  Should this be just walking?
            if (park_playing)
            {
               make_request(open_track++, park_playing);
               park_playing = 0;
            }
            else if ((score_playing == PARK_ZONE) && (rand()%100 < park_random))
//KLC                  && ((mlimbs_counter % 4)  == 0))
            {
               r = rand()%NUM_PARK_SOUNDS;
               park_playing = r + PARK_LAYER_BASE;
               make_request(open_track++,park_playing);
            }
         }
      }
   }

   // why!!!! why at the end, why randomly, hate hate hate
   // Clear out requests
   for (i=open_track; i < MLIMBS_MAX_CHANNELS -1; i++)
      current_request[i].pieceID = 255;				// was -1

   // Some end of cycle admin stuff...
   old_deconst = in_deconst;
}

errtype check_asynch_ai(bool /*new_score_ok*/)
{
//   extern bool mlimbs_semaphore;
//���   if (ai_cycle)
//���   {
      ai_cycle = 0;
      grind_music_ai();
//���      if (!run_asynch_music_ai)
//���         mlimbs_preload_requested_timbres();

//      mlimbs_semaphore = FALSE;

/*���
      // We need new theme loaded...
      if (!mai_semaphor)
      {
         if (new_score_ok && (new_theme==-1))
         {
            new_theme = -2;
            load_score_for_location(new_x,new_y);
         }
      }
*/
//���   }
   return(OK);
}

