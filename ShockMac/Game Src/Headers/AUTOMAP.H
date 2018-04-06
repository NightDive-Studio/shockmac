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
#ifndef __AUTOMAP_H
#define __AUTOMAP_H

/*
 * $Source: q:/inc/RCS/automap.h $
 * $Revision: 1.5 $
 * $Author: xemu $
 * $Date: 1993/09/02 23:06:30 $
 *
 * $Log: automap.h $
 * Revision 1.5  1993/09/02  23:06:30  xemu
 * angle me baby
 * 
 * Revision 1.4  1993/08/09  20:45:21  spaz
 * Changed some #define's.
 * 
 * 
 * Revision 1.3  1993/08/09  14:33:12  mahk
 * Fixed syntax errors
 * 
 * Revision 1.2  1993/08/09  13:27:47  spaz
 * Linked automap stub functions to the MFD system.
 * 
 * Revision 1.1  1993/05/03  11:51:58  xemu
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
typedef struct _AutoText{
   char *letters;
   Point coordinate;
   int color;
   struct _AutoText *next_entry;
} AutoText;

#define AUTOMAP_TERRAIN       0x0001
#define AUTOMAP_ELEVATION     0x0002
#define AUTOMAP_SECURITY      0x0004   
#define AUTOMAP_CRITTERS      0x0008
#define AUTOMAP_INFORMATION   0x0010
#define AUTOMAP_ALIGNMENT     0x0020

#define AUTOMAP_ZOOMIN_X      16
#define AUTOMAP_ZOOMIN_Y      16
#define AUTOMAP_ZOOMOUT_X     32
#define AUTOMAP_ZOOMOUT_Y     32

// Prototypes

// Specify which level is the "current" automap level for viewing
errtype automap_current(int level);

// Set what kinds of information are shown by the automap
errtype automap_infotype(ulong map_info);

// Draw the automap, scaled to specified size, in the area with upper left corner
// as specified.
errtype automap_draw(int map_size, Point ul_coord);

// Get a handle to a new text entry field on the automap, at specified coords
errtype automap_new_entry(Point coord, AutoText *new_text);

// Delete a given text entry
errtype automap_delete_entry(AutoText *victim);

// Stubs for the MFD system
void mfd_map_expose(MFD *m, ubyte control);
errtype mfd_map_init(MFD_Func *);
bool mfd_map_handler(MFD *m, uiEvent *e);

// Globals

#endif // __AUTOMAP_H

