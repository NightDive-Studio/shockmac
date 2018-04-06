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
#ifndef __GAMETIME_H
#define __GAMETIME_H

/*
 * $Source: n:/project/cit/src/inc/RCS/gametime.h $
 * $Revision: 1.5 $
 * $Author: mahk $
 * $Date: 1994/02/28 12:16:57 $
 *
 * $Log: gametime.h $
 * Revision 1.5  1994/02/28  12:16:57  mahk
 * Added suspend_ and resume_game_time
 * 
 * Revision 1.4  1993/09/02  23:07:42  xemu
 * angle me baby
 * 
 * Revision 1.3  1993/07/12  16:03:42  mahk
 * Changed proto to update_state
 * 
 * Revision 1.2  1993/07/01  13:57:02  mahk
 * Added run_fatigue_rate
 * 
 * Revision 1.1  1993/05/12  14:20:32  xemu
 * Initial revision
 * 
 *
 */

// Includes

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes


// Defines

// Prototypes

// Increment game time by one frame
errtype update_state(bool run_time);

// suspend and resume game time, so that sim doesn't 
// get run while doing non-real-time stuff.
void suspend_game_time(void);
void resume_game_time(void);


// Globals
extern int run_fatigue_rate;


#endif // __GAMETIME_H

