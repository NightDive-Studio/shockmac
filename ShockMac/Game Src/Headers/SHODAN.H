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
#include "objects.h"
#include "cybstrng.h"

#define CREATURE_SHODODMETER

#ifdef CREATURE_SHODOMETER
#define CYBORG_DRONE_TRIPLE_VALUE   2
#define WARRIOR_TRIPLE_VALUE        4
#define ASSASSIN_TRIPLE_VALUE       4
#define CYBERBABE_TRIPLE_VALUE      8
#define ELITE_GUARD_TRIPLE_VALUE    20   
#define CORTEX_REAVER_TRIPLE_VALUE  16
#define MUTANT_BORG_TRIPLE_VALUE    10
#define SECURITY_BOT1_TRIPLE_VALUE  5
#define SECURITY_BOT2_TRIPLE_VALUE  22
#define EXECBOT_TRIPLE_VALUE        6
#endif
#define CAMERA_TRIPLE_VALUE         5
#define SMALL_CPU_TRIPLE_VALUE      10
#define LARGCPU_TRIPLE_VALUE        50

#define SHODAN_INTERVAL_SHIFT    6
#define SHODAN_COLOR       0x4a

#define FIRST_SHODAN_QV    0x10
#define SHODAN_QV (FIRST_SHODAN_QV + player_struct.level)
#define MAX_SHODOMETER_LEVEL  13

#define SHODAN_BARK_CODE -1
#define SHODAN_BARK_TIMEOUT 3
#define DIEGO_BARK_CODE -2
#define FIRST_SHODAN_BARK  0x666
#define NUM_SHODAN_BARKS   4
#define SHODAN_MUG      17
#define DIEGO_MUG       4
#define SHODAN_MUG_2    23
#define FIRST_SHODAN_MUG 31
#define NUM_SHODAN_MUGS NUM_SHODAN_BARKS

#define FIRST_SHODAN_ANIM  0x3f
#define NUM_SHODAN_FRAMES  6

#define SPECIAL_SHODAN_FAIL_CODE 0xFF
#define SHODAN_FAILURE_STRING REF_STR_SHODANFail

short compute_shodometer_value(bool game_stuff);
short increment_shodan_value(ObjID oid, bool game_stuff);
short decrement_shodan_value(ObjID oid, bool game_stuff);
