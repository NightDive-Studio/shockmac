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
#ifndef __SCREEN_H
#define __SCREEN_H

/*
 * $Source: n:/project/cit/src/inc/RCS/screen.h $
 * $Revision: 1.20 $
 * $Author: dc $
 * $Date: 1994/05/11 21:42:04 $
 *
 *
 */

#define GADGET_GAMESCREEN

// C Library Includes

// System Library Includes
#include "frtypesx.h"

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines
#define SCREEN_VIEW_X   28
#define SCREEN_VIEW_Y   24
#define SCREEN_VIEW_HEIGHT 108
#define SCREEN_VIEW_WIDTH  268  

// Prototypes

// Initialize the main game screen.  This should only be called once, at
// the major initialization stage of the program.
errtype screen_init(void);

// Bring the main game screen to the monitor, doing appropriate cool
// looking tricks (palette fading, etc.) as necessary
void screen_start(void);

// Do appropriate stuff to indicate that we have left the game screen
void screen_exit(void);

// Force a draw of the whole durned screen
errtype screen_draw(void);

// Stop doing graphics things
errtype screen_shutdown(void);

// Handle keyboard input anywhere on the main screen
bool main_kb_callback(uiEvent *h, LGRegion *r, void *udata);

// rct NULL means fullscreen, slb NULL is no slabinit, key and maus are callbacks, NULL is no install
void generic_reg_init(bool create_reg, LGRegion *reg, LGRect *rct, uiSlab *slb, void *key_h, void *maus_h);

// Globals
#ifdef __SCREEN_SRC
uchar *default_font_buf;
LGRegion *root_region, *mainview_region, *status_region, *inventory_region_game;
LGRegion *pagebutton_region_game;
LGCursor globcursor, wait_cursor, fire_cursor;
frc *normal_game_fr_context;
#else
extern uchar *default_font_buf;
extern LGRegion *root_region, *mainview_region, *inventory_region_game, *status_region;
extern LGRegion *pagebutton_region_game;
extern LGCursor globcursor, wait_cursor, fire_cursor;
extern frc  *normal_game_fr_context;
#endif

#endif // __SCREEN_H

