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
#ifndef __WRAPPER_H
#define __WRAPPER_H

/*
 * $Source: n:/project/cit/src/inc/RCS/wrapper.h $
 * $Revision: 1.7 $
 * $Author: dc $
 * $Date: 1994/05/12 00:30:52 $
 *
 * $Log: wrapper.h $
 * Revision 1.7  1994/05/12  00:30:52  dc
 * comment defines
 * 
 * Revision 1.6  1994/05/11  21:31:04  dc
 * some externals, if i remember correctly
 * 
 * Revision 1.5  1994/04/07  22:37:53  xemu
 * new wrapper paradigm
 * 
 * Revision 1.4  1994/02/25  15:43:58  mahk
 * Added stupid terseness questbit.
 * 
 * Revision 1.3  1993/09/02  23:08:56  xemu
 * angle me baby
 * 
 * Revision 1.2  1993/07/22  17:31:25  xemu
 * some #defines
 * 
 * Revision 1.1  1993/06/14  15:37:04  xemu
 * Initial revision
 * 
 *
 */

// Includes

// Defines
#define MAIN_PANEL       0  
#define SAVELOAD_PANEL   1

// questbit for terse text.
#define TERSENESS_QBIT 0x180

// Prototypes

// Replaces the inventory panel with a wrapper input paneloid thing,
// which is 2 by width text buttons for the user to click on.  When clicked,
// the passed callback is called with the number of the button clicked
// as an argument.
bool wrapper_options_func(short keycode, ulong context, void* data);

#define NUM_SAVE_SLOTS    8
#define SAVE_COMMENT_LEN 32

// Globals
#ifndef __WRAPPER_SRC
extern char save_game_name[];
extern char comments[NUM_SAVE_SLOTS][SAVE_COMMENT_LEN];
#endif
#define Poke_SaveName(game_num) { save_game_name[6]='0'+(game_num>>3); save_game_name[7]='0'+(game_num&7); }

#endif // __WRAPPER_H

