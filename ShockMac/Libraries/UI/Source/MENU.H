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
#ifndef __MENU_H
#define __MENU_H

/*
 * $Source: n:/project/lib/src/ui/RCS/menu.h $
 * $Revision: 1.4 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:21 $
 *
 * $Log: menu.h $
 * Revision 1.4  1993/10/11  20:27:21  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.3  1993/07/06  11:39:29  xemu
 * popup_at_mouse
 * 
 * Revision 1.2  1993/06/01  13:37:39  xemu
 * popdown
 * 
 * Revision 1.1  1993/05/18  13:58:04  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "error.h"
#include "hotkey.h"
#include "tng.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines
typedef struct {
   char *name;
   int z;
} menu_struct;

// Prototypes
Gadget *gad_menu_create(Gadget *parent, LGPoint *coord, int z, TNGStyle *sty, int width, char *name);

errtype gad_menu_add_line(Gadget *menu, char *label, hotkey_callback f, short keycode, ulong context, void *user_data, char *help_text);
errtype gad_menu_add_submenu(Gadget *menu, char *label, Gadget *sub_menu);
errtype gad_menu_popup(Gadget *menu);
errtype gad_menu_popup_at_mouse(Gadget *menu);
errtype gad_menu_popdown(Gadget *menu);

// Globals

#endif // __MENU_H

