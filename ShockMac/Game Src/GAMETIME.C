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
 * $Source: r:/prj/cit/src/RCS/gametime.c $
 * $Revision: 1.16 $
 * $Author: minman $
 * $Date: 1994/09/11 07:42:01 $
 *
 */

#include <stdlib.h>

#include "gametime.h"
#include "faketime.h"
#include "player.h"
#include "schedule.h"
#include "drugs.h"
#include "lvldata.h"
#include "dirac.h"
#include "framer8.h"

ulong last_real_time = 0;
char reflex_remainder=0;

#define MAX_DELTAT  (CIT_CYCLE/MIN_FRAME_RATE)
#define MIN_DELTAT  (CIT_CYCLE/MAX_FRAME_RATE)


void update_level_gametime(void);


errtype update_state(bool time_running)
{
   bool update = TRUE;
   if (time_running)
   {
      ulong deltat;
      if (player_struct.drug_status[DRUG_REFLEX] > 0 && !global_fullmap->cyber)
      {
         // So we effectively downshift deltat by 2 to divide by 4
         // we keep the remainder around so that we don't screw up the universe badly
         deltat = *tmd_ticks + reflex_remainder - last_real_time;
         reflex_remainder = deltat & 0x3;
         deltat = deltat >> 2;
      }
      else
         deltat = *tmd_ticks - last_real_time;
      if (deltat > MAX_DELTAT) deltat = MAX_DELTAT;
      if (deltat < MIN_DELTAT)
      {
         deltat = 0;
         update = FALSE;
      }
      // update game time.
      player_struct.deltat =  deltat;
      player_struct.game_time += deltat;
   }
   if (update)
      last_real_time = *tmd_ticks;
   run_schedules();
   return(OK);
}


// static ulong time_at_suspend = 0;

void suspend_game_time(void)
{
//   if (time_at_suspend == 0)
//      time_at_suspend = *tmd_ticks;
}

void resume_game_time(void)
{
//   last_real_time += *tmd_ticks - time_at_suspend;
   last_real_time = *tmd_ticks;
//   time_at_suspend = 0;
}


#define MAX_PHYSICS_RUNTIME fix_make(10,0)

void update_level_gametime(void)
{
   ulong oldtime = level_gamedata.exit_time;
   ulong deltat = player_struct.game_time - oldtime;
   run_schedules();
   // run ai's 
//KLC   ai_time_passes(&deltat);
//KLC   ai_freeze_tag();
   // run physics 
   if (global_fullmap->cyber)
   {
      EDMS_control_Dirac_frame(PLAYER_PHYSICS,0,0,0,0);
   }
   else
   {
      EDMS_control_pelvis(PLAYER_PHYSICS,0,0,0,0,0,0);
   }
   EDMS_soliton_vector(min(MAX_PHYSICS_RUNTIME,deltat/CIT_CYCLE));
}
