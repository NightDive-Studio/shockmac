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
#ifndef __BUTARRAY_H
#define __BUTARRAY_H

/*
 * $Source: n:/project/lib/src/ui/RCS/butarray.h $
 * $Revision: 1.11 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:11 $
 *
 * $Log: butarray.h $
 * Revision 1.11  1993/10/11  20:27:11  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.10  1993/05/03  18:54:18  xemu
 * Checkbox mode
 * 
 * Revision 1.9  1993/04/28  14:40:13  mahk
 * Preparing for second exodus
 * 
 * Revision 1.8  1993/04/22  15:09:25  xemu
 * Convert to TNG
 * 
 * Revision 1.7  1993/04/12  15:17:45  xemu
 * use better default font
 * ..
 * 
 * 
 * Revision 1.6  1993/04/08  23:57:08  xemu
 * 2d fonts
 * 
 * Revision 1.5  1993/04/02  14:41:01  xemu
 * Style defines
 * 
 * Revision 1.4  1993/03/31  15:40:00  xemu
 * Keyboard equivalents
 * 
 * Revision 1.3  1993/03/27  18:08:31  unknown
 * Better #define names
 * 
 * Revision 1.2  1993/03/26  19:54:09  xemu
 * radio behavior, other fun stuff
 * 
 * Revision 1.1  1993/03/25  23:34:30  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "slider.h"
#include "tngbarry.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines

// Prototypes

// Creates a button array.  The initial button array is empty, each of the buttons must be added with
// _addbutton or _addbutton_at.  To delete a button, just add a button of type NULL_TYPE there.  The msize_
// and window_ values  indicate the size of the button matrix and the window onto it, respectively.  The units
// of those values is in "buttons".  The spacing parameter determines how much blank space is between the buttons,
// and the bsize_ parameters describe the size of each of the individual buttons.  The overall size of the
// buttonarray is automatically computed from these parameters.  The upper left of the button array is
// indicated by the coord parameter.  If NULL is passed as the style parameter, the default style is used.
Gadget *gad_buttonarray_create(Gadget *parent, LGPoint coord, int z, int msize_x, int msize_y, int window_x, int window_y,
   int bsize_x, int bsize_y, int num_sel, ushort options, TNGStyle *sty, char *name);

// Fills in a slot in the button array with a button of type type, with display information disp_data.
// The button will occupy the first empty slot.
errtype gad_buttonarray_addbutton(Gadget *g, int type, void *disp_data);

// Like gad_buttonarray_addbutton but allows specification of the x and y coordinates within the 
// button matrix for the new button.
errtype gad_buttonarray_addbutton_at(Gadget *g, int type, void *disp_data, int coord_x, int coord_y);

// Set the offset of the buttonarray to an arbitrary x & y coordinate
errtype gad_buttonarray_setoffset(Gadget *g, int offset_x, int offset_y);

// Globals

#endif // __BUTARRAY_H

