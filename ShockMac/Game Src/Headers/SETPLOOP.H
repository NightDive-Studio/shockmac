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
#ifndef __SETPLOOP_H
#define __SETPLOOP_H

/*
 * $Source: n:/project/cit/src/inc/RCS/setploop.h $
 * $Revision: 1.4 $
 * $Author: dc $
 * $Date: 1994/06/13 23:24:03 $
 *
 * $Log: setploop.h $
 * Revision 1.4  1994/06/13  23:24:03  dc
 * doug is a bonehead
 * 
 * Revision 1.3  1993/09/02  23:08:34  xemu
 * angle me baby
 * 
 * Revision 1.2  1993/07/27  18:34:53  xemu
 * SETUP_ANIM_UPDATE
 * 
 * Revision 1.1  1993/05/14  15:46:41  xemu
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
#define SETUP_ANIM_UPDATE        LL_CHG_BASE<<0

// loop id's
#define SETUP_ANIM         0
#define SETUP_JOURNEY      1 
#define SETUP_DIFFICULTY   2
#define SETUP_CREDITS      3
#define SETUP_CONTINUE     4

// Prototypes
void setup_loop(void);
void journey_credits_done(void);

// Globals
extern int setup_mode;

#endif // __SETPLOOP_H

