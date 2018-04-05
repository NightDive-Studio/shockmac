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
#ifndef __TNGMENU_H
#define __TNGMENU_H

/*
 * $Source: n:/project/lib/src/ui/RCS/tngmenu.h $
 * $Revision: 1.4 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:38 $
 *
 * $Log: tngmenu.h $
 * Revision 1.4  1993/10/11  20:27:38  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.3  1993/07/06  11:39:21  xemu
 * popup_at_mouse
 * 
 * Revision 1.2  1993/05/19  19:44:08  xemu
 * cleanup
 * 
 * Revision 1.1  1993/05/18  13:58:10  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "error.h"
#include "hotkey.h"
#include "texttool.h"
#include "tng.h"
#include "llist.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines
typedef struct {
   struct _llist *pnext;		// ptr to next node or NULL if at tail
   struct _llist *pprev;		// ptr to prev node or NULL if at head
   char *label;
   hotkey_callback f;
   void *user_data;
   short keycode;
   ulong context;
   TNG *submenu;
} MenuElement;

typedef struct {
   TNG *tng_data;
   void *ui_struct;
   LGPoint coord;
   LGPoint size;
   llist_head element_header;
   int slot_height;
   int num_lines;
   bool popped_up;
   MenuElement *current_selection;
   void (*popup_func)(TNG *ptng);
   void (*popdown_func)(TNG *ptng);
}  TNG_menu;

#define TNG_MENU_SPACING   2

// Prototypes
// Initializes the TNG 

errtype tng_menu_init(void *ui_data, TNG *ptng, TNGStyle *sty, LGPoint coord, int width, 
   void (*upfunc)(TNG *ptng), void (*downfunc)(TNG *ptng), void *ui_struct);

// Deallocate all memory used by the TNG 
errtype tng_menu_destroy(TNG *ptng);

// Draw the specified parts (may be all) of the TNG at screen coordinates loc
// assumes all appropriate setup has already been done!
errtype tng_menu_2d_draw(TNG *ptng, ushort partmask, LGPoint loc);

// Fill in ppt with the size of the TNG 
errtype tng_menu_size(TNG *ptng, LGPoint *ppt);

// Returns the current "value" of the TNG
int tng_menu_getvalue(TNG *ptng);

// React appropriately for receiving the specified cooked key
bool tng_menu_keycooked(TNG *ptng, ushort key);

// React appropriately for receiving the specified mouse button event
bool tng_menu_mousebutt(TNG *ptng, uchar type, LGPoint loc);

// Handle incoming signals
bool tng_menu_signal(TNG *ptng, ushort signal);

errtype tng_menu_add_line(TNG *ptng, char *label, hotkey_callback f, short keycode, ulong context,
   void *user_data, char *help_text);

errtype tng_menu_add_submenu(TNG *ptng, char *label, TNG *submenu);

MenuElement *tng_menu_add_basic(TNG *ptng, char *label);

errtype tng_menu_selection(TNG *ptng);

errtype tng_menu_popup(TNG *ptng);
errtype tng_menu_popup_at_mouse(TNG *ptng);
errtype tng_menu_popup_loc(TNG *ptng, LGPoint poploc);

// Macros
#define TNG_MN(ptng) ((TNG_menu *)(ptng->type_data))

// Globals

#endif // __TNGMENU_H

