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
#ifndef __SETUP_H
#define __SETUP_H

/*
 * $Source: n:/project/cit/src/inc/RCS/setup.h $
 * $Revision: 1.11 $
 * $Author: xemu $
 * $Date: 1994/05/03 23:45:22 $
 *
 * $Log: setup.h $
 * Revision 1.11  1994/05/03  23:45:22  xemu
 * moved location controlling #defines
 * 
 * Revision 1.10  1994/03/14  22:28:20  xemu
 * setup_continue
 * 
 * Revision 1.9  1994/02/12  14:34:24  xemu
 * credits define
 * 
 * Revision 1.8  1994/01/30  15:35:42  minman
 * got rid of animation stuff
 * 
 * Revision 1.7  1994/01/15  03:52:15  minman
 * incremented number of cutscenes
 * 
 * Revision 1.6  1993/09/02  23:08:35  xemu
 * angle me baby
 * 
 * Revision 1.5  1993/08/12  10:47:31  xemu
 * allow playing of different cutscenes
 * 
 * Revision 1.4  1993/07/28  20:44:37  xemu
 * new art
 * 
 * Revision 1.3  1993/07/27  18:35:13  xemu
 * all sorts of #defines about difficulty and journey layout
 * 
 * Revision 1.2  1993/07/13  04:28:18  minman
 * added setup_init
 * 
 * Revision 1.1  1993/05/14  15:49:21  xemu
 * Initial revision
 * 
 *
 */

// Includes

// Prototypes

errtype setup_init(void);

// Do appropriate things upon first entering the setup loop
void setup_start(void);

// Do appropriate things for leaving the setup loop
void setup_exit(void);

// Displays the intro screen(s)
errtype setup_intro_draw();

// Call this when ready to start a new game.
void go_and_start_the_game_already(void);

// Call this when opening a saved game.
errtype load_that_thar_game(FSSpec *loadSpec);

// Globals

#endif // __SETUP_H

