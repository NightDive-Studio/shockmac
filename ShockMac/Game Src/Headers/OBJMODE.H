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
#ifndef __OBJMODE_H
#define __OBJMODE_H

/*
 * $Source: n:/project/cit/src/inc/RCS/objmode.h $
 * $Revision: 1.14 $
 * $Author: xemu $
 * $Date: 1994/01/06 10:34:20 $
 *
 * $Log: objmode.h $
 * Revision 1.14  1994/01/06  10:34:20  xemu
 * camera fun
 * 
 * Revision 1.13  1993/09/25  16:58:12  xemu
 * resized stuff
 * 
 * Revision 1.12  1993/09/19  19:07:05  xemu
 * cameras in editor
 * 
 * Revision 1.11  1993/09/02  23:08:16  xemu
 * angle me baby
 * 
 * Revision 1.10  1993/07/20  11:08:05  xemu
 * removed bitmap triple function
 * 
 * Revision 1.9  1993/07/11  12:56:55  xemu
 * hack bitmap func for Mahk
 * 
 * Revision 1.8  1993/07/08  23:49:42  xemu
 * object properties
 * 
 * Revision 1.7  1993/06/28  12:57:51  xemu
 * soem fixes
 * 
 * Revision 1.6  1993/06/24  00:05:35  xemu
 * minor move around
 * 
 * Revision 1.5  1993/06/03  20:05:15  xemu
 * browser stuff
 * 
 * Revision 1.4  1993/05/24  15:19:19  xemu
 * moved to own slab/loop
 * 
 * Revision 1.3  1993/05/23  16:35:37  xemu
 * added highlight func and mode brush
 * 
 * Revision 1.2  1993/05/21  17:46:04  xemu
 * edit buttons, classes
 * 
 * Revision 1.1  1993/05/20  20:58:16  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "objects.h"
#include "map.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines

// Prototypes

// Creates all appropriate gadgets for the object palette area,
// then hides them under the magic curtain of the palette region.
errtype object_palette_create(void);

// Brings the object palette up to the front of the object palette area.
errtype object_palette_popup(void);

errtype object_load_subclass(ObjClass loadclass);
errtype object_load_type(ObjClass loadclass, ubyte subclass);

bool object_find_func(int highlight_num);

void object_mode_brush(MapElem *paint, FullMap *map, LGRect *r, LGPoint square, void *brushdata);

// Globals
#ifdef __OBJMODE_SRC
ObjRefID current_ref = OBJ_REF_NULL;
int curr_x = 0, curr_y = 0;
ubyte curr_z = 0;
int curr_int = 0, curr_class = 0, curr_subclass = 0;
Point current_loc = {0,0};
#else
extern ObjRefID current_ref;
extern int curr_x, curr_y;
extern ubyte curr_z;
extern int curr_int,curr_class, curr_subclass;
#endif

#endif // __OBJMODE_H

