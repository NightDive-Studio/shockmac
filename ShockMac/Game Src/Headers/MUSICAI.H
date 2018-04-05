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
#ifndef __MUSICAI_H
#define __MUSICAI_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/musicai.h $
 * $Revision: 1.28 $
 * $Author: dc $
 * $Date: 1994/11/19 20:44:54 $
 *
 *
 */

// Includes
#include "objects.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines
#define DEFAULT_PERIL_MAX  100
#define DEFAULT_PERIL_MIN  1
#define DEFAULT_POSIT_MAX  100
#define DEFAULT_POSIT_MIN  1
#define DEFAULT_MOTION_MAX 100
#define DEFAULT_MOTION_MIN 1

#define NO_MUSIC_ZONE      0
#define HOSPITAL_ZONE      1   // nee Residential
#define EXECUTIVE_ZONE     2
#define INDUSTRIAL_ZONE    3
#define METAL_ZONE         4
#define PARK_ZONE          5   // also used in access corridors
#define BRIDGE_ZONE        6   // once Outer bridge
#define ELEVATOR_ZONE      7

#define NUM_EVENTS 9

#define WALKING_SCORE    0
#define PERIL_SCORE      4
#define COMBAT_SCORE     6

#define PERIL_THRESHOLD    70

#define NO_MONSTER   -1
#define SMALL_ROBOT  0
#define LARGE_ROBOT  1
#define MUTANT       2

#define SAME_MODE    0
#define NORMAL_MODE  1
#define TRANSITION_MODE 2

#define NUM_TRANSITIONS       9

#define TRANS_INTRO           0
#define TRANS_WALK_TO_PERIL   1
#define TRANS_PERIL_TO_COMB   2
#define TRANS_DEATH           3
#define TRANS_VICTORY         4
#define TRANS_PERIL_TO_WALK   5
#define TRANS_COMB_TO_WALK    6
#define TRANS_COMB_TO_PERIL   7
#define TRANS_WALK_TO_COMB    8

#define MONSTER_MUSIC_MUTANT        0
#define MONSTER_MUSIC_ROBOT         1
#define MONSTER_MUSIC_CYBORG        2
#define MONSTER_MUSIC_SMALL_ROBOT   3

#define NUM_SCORES                  				8
#define NUM_LAYERABLE_SUPERCHUNKS	22
#define FIRST_SUPERCHUNK_LAYER      	16
#define NUM_LAYERS                  				32
#define LAYER_BASE                  				32

#define SUPERCHUNKS_PER_SCORE		4
#define MAX_KEYS    							10
#define KEY_BAR_RESOLUTION 			2

#define NUM_PARK_SOUNDS				10
#define PARK_LAYER_BASE 				32

#define CYBERSPACE_SCORE_BASE		10
#define NUM_NODE_THEMES				2

#define DANGER_LAYER_BASE				10    // actually one less than Danger1 since a minimum of 1 gets added to it...
#define SUCCESS_LAYER_BASE			(DANGER_LAYER_BASE + 2)
#define DECONSTRUCT_LAYER				15
#define TRANSITION_LAYER_BASE		16


// Prototypes

// Initialize the AI portion of the MLIMBS system.
errtype mlimbs_AI_init(void);
void music_ai(void);
errtype mai_monster_nearby(int monster_type);
errtype mai_attack();
errtype mai_intro();
errtype mai_monster_defeated();
errtype mai_player_death();
errtype fade_into_location(int x, int y);
errtype load_score_for_location(int x, int y);
errtype load_score_from_cfg(char *filename);
void    load_score_guts(char score_playing);
errtype music_init();
errtype digifx_init();
errtype stop_digi_fx();
void clear_digi_fx();
int play_digi_fx_master(int sfx_code, int num_loops, ObjID id, ushort x, ushort y);
#define play_digi_fx(sfx_code, loops)                 play_digi_fx_master(sfx_code,loops,OBJ_NULL,0,0)
#define play_digi_fx_obj(sfx_code, num_loops, id)     play_digi_fx_master(sfx_code,num_loops,id,0,0)
#define play_digi_fx_loc(sfx_code, num_loops, x, y)   play_digi_fx_master(sfx_code,num_loops,OBJ_NULL,x,y)
errtype play_sound_effect(char *filename);
bool digi_fx_playing(int fx_id, int *handle_ptr);
errtype output_text(char *);
extern void mlimbs_do_ai(void);

extern void digifx_EOS_callback(snd_digi_parms *sdp);

extern bool digi_pan_reverse;

void grind_credits_music_ai(void);

// Globals
#ifdef __MUSICAI_SRC
int mlimbs_peril, mlimbs_positive, mlimbs_motion, mlimbs_monster;
ulong mlimbs_combat;
int current_score, current_zone, current_mode, random_flag;
int current_transition, last_score;
int boring_count;
int mlimbs_boredom;
int *output_table;
bool wait_flag;
int next_mode, ai_cycle;
bool music_card=FALSE, music_on=FALSE;
//KLC no sfx_card, sfx_on moved to DIGIFX.C     bool sfx_card=FALSE, sfx_on=FALSE;
int cur_digi_channels=4;
#else
extern int mlimbs_peril, mlimbs_positive, mlimbs_motion, mlimbs_monster;
extern ulong mlimbs_combat;
extern int current_score, current_zone, current_mode, random_flag;
extern int current_transition, last_score;
extern int boring_count;
extern int mlimbs_boredom;
extern int *output_table;
extern bool wait_flag;
extern int next_mode, ai_cycle;
extern bool music_card, music_on;
extern bool /*sfx_card, */sfx_on;
extern int cur_digi_channels;
#endif

#endif // __MUSICAI_H

