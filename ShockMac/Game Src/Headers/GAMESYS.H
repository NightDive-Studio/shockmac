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
#ifndef __GAMESYS_H
#define __GAMESYS_H

/*
 * $Source: n:/project/cit/src/inc/RCS/gamesys.h $
 * $Revision: 1.18 $
 * $Author: mahk $
 * $Date: 1993/12/29 16:16:52 $
 *
 * $Log: gamesys.h $
 * Revision 1.18  1993/12/29  16:16:52  mahk
 * Added some more fatigue defines
 * 
 * Revision 1.17  1993/11/24  21:04:57  mahk
 * Added a new game time schedule.
 * 
 * Revision 1.16  1993/11/15  19:33:51  xemu
 * level control box params
 * 
 * Revision 1.15  1993/11/14  18:51:00  minman
 * got rid of MAX_HP -cause we're using PLAYER_MAX_HP now
 * 
 * Revision 1.14  1993/10/21  21:25:32  xemu
 * MAX_ENERGY = 2555
 * errr, 255 that is
 * 
 * Revision 1.13  1993/09/02  23:07:41  xemu
 * angle me baby
 * 
 * Revision 1.12  1993/08/11  20:53:59  spaz
 * #define'd maximum energy,hp,accuracy
 * 
 * Revision 1.11  1993/08/06  16:01:41  minman
 * modified software numbers
 * 
 * Revision 1.10  1993/08/05  14:13:09  minman
 * forgot objwpn.h
 * 
 * Revision 1.9  1993/08/05  14:04:12  minman
 * changed to new object properties order
 * 
 * Revision 1.8  1993/07/29  20:16:42  minman
 * made it rely on objprop.h
 * 
 * Revision 1.7  1993/07/23  12:14:45  mahk
 * Changed numbers of combat & defense softs
 * 
 * Revision 1.6  1993/07/19  11:39:57  mahk
 * Moved #defines for numbers of things back here.
 * 
 * Revision 1.5  1993/07/02  14:16:23  mahk
 * Removed NUM_WEAPONZ
 * 
 * Revision 1.4  1993/07/01  17:18:58  spaz
 * Removed double definition of NUM_DRUGZ (now in drugs.h)
 * 
 * Revision 1.3  1993/06/29  09:53:46  mahk
 * Modes for fatigue
 * 
 * Revision 1.2  1993/06/16  23:01:45  xemu
 * more #defines
 * 
 * Revision 1.1  1993/05/12  14:20:51  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "objwpn.h"
#include "objwarez.h"
#include "schedtyp.h"

// Defines
#define NUM_LEVELZ            22
#define NUM_HARDWAREZ         NUM_HARDWARE         // Steal from Objwarez.h
#define NUM_COMBAT_SOFTS      NUM_OFFENSE_SOFTWARE
#define NUM_DEFENSE_SOFTS     NUM_DEFENSE_SOFTWARE
#define NUM_MISC_SOFTS        (NUM_ONESHOT_SOFTWARE + NUM_MISC_SOFTWARE)
#define NUM_GRENADEZ          7
#define NUM_DRUGZ             7
#define NUM_WEAPONZ           16

#define NUM_AMMO_TYPES        NUM_AMMO             // Steal from Objwpn.h
                                           
// just so you don't screw up. 
#define NUM_GRENADES NUM_GRENADEZ
#define NUM_LEVELS NUM_LEVELZ
#define NUM_DRUGS NUM_DRUGZ   
#define NUM_WEAPONS NUM_WEAPONZ

#define SPRINT_CONTROL_THRESHOLD 70 // threshold between sprint and jog

#define MAX_ENERGY   255
#define MAX_ACCURACY 100

#define LEVEL_GRAV_NORMAL     0
#define LEVEL_GRAV_LOW        1
#define LEVEL_GRAV_ZERO       2

#define LEVEL_MIST_NONE       0
#define LEVEL_MIST_LIGHT      1
#define LEVEL_MIST_HEAVY      2

#define LEVEL_BIOHAZARD_NONE     0
#define LEVEL_BIOHAZARD_PARTIAL  1
#define LEVEL_BIOHAZARD_SEVERE   2

#define GAME_SCHEDULE_SIZE 64

// Prototypes

// Run the game system for one frame
errtype gamesys_run(void);

// Globals
extern Schedule game_seconds_schedule;


#endif // __GAMESYS_H

