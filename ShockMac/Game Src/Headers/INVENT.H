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
#ifndef __INVENT_H
#define __INVENT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/invent.h $
 * $Revision: 1.14 $
 * $Author: xemu $
 * $Date: 1994/07/18 21:31:31 $
 *
 */

// Includes
#include "invdims.h"

// C Library Includes

// System Library Includes
#include "objects.h"

// Master Game Includes

// Game Library Includes

// Game Object Includes


#define MAX_GENERAL_INVENTORY 12

// Prototypes

// creates and initializes the inventory region
LGRegion* create_invent_region(LGRegion* parent, LGRegion **pbuttons, LGRegion **pinvent);

// Draw the inventory area.  Keeps information on most recent draw so only does 
// incremental updates.
errtype inventory_draw(void);

// Force the inventory panel to draw, no matter what
errtype inventory_full_redraw(void);

// switch the inventory page to pgnum and redraw
errtype inventory_draw_new_page(int pgnum);


// clears the inventory region
errtype inventory_clear(void);

// Add the appropriate kind of object to the player's inventory.   Returns whether
// or not the action succeded (typical failure reason being not enough inventory
// slots remaining).
bool inventory_add_object (ObjID new_object,bool select);

// Removes the specified object from the player's inventory.  Does not do any 
// correlation with the rest of the Universe -- this needs to be handed by
// the dropping/consuming/destroying code.
errtype inventory_remove_object(ObjID new_object);

// Globals
extern short inventory_page;
extern short inv_last_page;

#endif // __INVENT_H

