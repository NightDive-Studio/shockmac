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
#include "player.h"

errtype exit_cyberspace_stuff();
errtype enter_cyberspace_stuff(char dest_level);
errtype check_cspace_death();

#define NUM_CS_EFFECTS 3

#define CS_TURBO_EFF    0
#define CS_DECOY_EFF    1
#define CS_MATCHBOX_EFF 2

#define CYBER_DIFF   (player_struct.difficulty[CYBER_DIFF_INDEX])

//#define BASE_CSPACE_TIME  ((CYBER_DIFF == 0) ? (CIT_CYCLE * 3600) : ((CYBER_DIFF == 1) ? (CIT_CYCLE * 720) : (CIT_CYCLE * 360)))
#define BASE_CSPACE_TIME   1800 * CIT_CYCLE

#define CSPACE_MIN_TIME          (CYBER_DIFF ? CIT_CYCLE * 90 : BASE_CSPACE_TIME)
#define CSPACE_MAX_TIME          ((CYBER_DIFF == 0) ? BASE_CSPACE_TIME : (CYBER_DIFF == 1) ? (BASE_CSPACE_TIME / 3) : (CYBER_DIFF == 2) ? (BASE_CSPACE_TIME / 6) : (BASE_CSPACE_TIME / 12))

// no time penalties at diff 0 and 1, harsh ones at 3
#define CSPACE_EXIT_PENALTY      ((CYBER_DIFF <= 2) ? 0 : (CIT_CYCLE * 5))
#define CSPACE_DEATH_PENALTY     ((CYBER_DIFF < 2) ? 0 : ((CYBER_DIFF == 2) ? (CIT_CYCLE * 3) : (CIT_CYCLE * 12)))
#define CSPACE_MUX_BONUS         CIT_CYCLE * 30

#define CYBERMINE_DAMAGE   15
#define CYBERHEAL_QUANTITY 70

extern void (*cspace_effect_turnoff[])(bool, bool);
extern ulong cspace_effect_times[NUM_CS_EFFECTS];
extern ulong cspace_effect_durations[NUM_CS_EFFECTS];
extern ObjID cspace_decoy_obj;
extern ObjLoc recall_objloc;
