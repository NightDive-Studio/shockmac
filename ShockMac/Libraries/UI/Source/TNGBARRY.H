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
#ifndef __TNGBARRY_H
#define __TNGBARRY_H

/*
 * $Source: n:/project/lib/src/ui/RCS/tngbarry.h $
 * $Revision: 1.12 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:37 $
 *
 * $Log: tngbarry.h $
 * Revision 1.12  1993/10/11  20:27:37  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.11  1993/08/10  17:48:43  xemu
 * fixed a macro bug
 * 
 * Revision 1.10  1993/07/11  21:59:11  xemu
 * added LOOSEPACK and TIGHTPACK as options
 * 
 * Revision 1.9  1993/07/11  17:16:33  mahk
 * moved key defs to barrykey.h
 * 
 * Revision 1.8  1993/06/29  11:53:44  xemu
 * smaller checkboxes
 * 
 * Revision 1.7  1993/06/16  22:10:08  xemu
 * shrinkify it
 * 
 * Revision 1.6  1993/05/24  20:03:51  mahk
 * Added data structure for callback button type. 
 * 
 * Revision 1.5  1993/05/06  14:58:51  xemu
 * smaller scrollbar size
 * 
 * Revision 1.4  1993/05/03  18:54:50  xemu
 * Checkbox mdoe
 * 
 * Revision 1.3  1993/04/28  14:40:24  mahk
 * Preparing for second exodus
 * 
 * Revision 1.2  1993/04/27  16:38:44  xemu
 * general imporvement
 * 
 * Revision 1.1  1993/04/22  15:03:50  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "tng.h"

// Typedefs
typedef struct {
   int type;
   void *disp_data;
} TNGButtonArrayElement;

typedef struct _butdrawcallback
{
   void (*func)(LGRect* r, LGPoint butcoord, void* data);
   void* data;
} ButtonDrawCallback;


typedef struct {
   TNG *tng_data;
   LGPoint bsize, msize, wsize, scroll_size;
   LGPoint size, offset, lsel;
   ubyte spacing;
   int num_selectable;
   short lastkey;
   ushort options;
   TNGButtonArrayElement *matrix;
   bool *selected;
   TNG *hscroll_tng, *vscroll_tng;
} TNG_buttonarray;

#define TNG_BA_MATRIX   0x0001
#define TNG_BA_HSCROLL  0x0002
#define TNG_BA_VSCROLL  0x0004

#define TNG_BA_SELECT_SIZE    1
#define TNG_BA_BORDER_WIDTH   1
#define TNG_BA_SCROLL_X       7
#define TNG_BA_SCROLL_Y       7


#define TNG_BA_NO_OPTIONS          0x00
#define TNG_BA_OUTLINE_MODE        0x01
#define TNG_BA_CHECKBOX_MODE       0x02
#define TNG_BA_TIGHTPACK           0x04
#define TNG_BA_LOOSEPACK           0x08

#define TNG_BA_CHECKBOX_SIZE       4
// Prototypes

// Initializes the TNG 
// Note that both of these must be called!
// _init is called before the UI deals appropriately, _init2 is called afterwards.
errtype tng_buttonarray_init(void *ui_data, TNG *ptng, TNGStyle *sty, ushort options, LGPoint msize, LGPoint wsize, LGPoint bsize, int num_sel);
errtype tng_buttonarray_init2(TNG *ptng);

// Deallocate all memory used by the TNG 
errtype tng_buttonarray_destroy(TNG *ptng);

// Draw the specified parts (may be all) of trdinates loc
// assumes all appropriate setup has already been done!
errtype tng_buttonarray_2d_draw(TNG *ptng, ushort partmask, LGPoint loc);

// Fill in ppt with the size of the TNG 
errtype tng_buttonarray_size(TNG *ptng, LGPoint *ppt);

// Returns the current "value" of the TNG
int tng_buttonarray_getvalue(TNG *ptng);

// React appropriately for receiving the specified cooked key
bool tng_buttonarray_keycooked(TNG *ptng, ushort key);

// React appropriately for receiving the specified mouse button event
bool tng_buttonarray_mousebutt(TNG *ptng, uchar type, LGPoint loc);

// Handle incoming signals
bool tng_buttonarray_signal(TNG *ptng, ushort signal);

errtype tng_buttonarray_select(TNG *ptng);
errtype tng_buttonarray_scroll(TNG *ptng);
errtype tng_buttonarray_addbutton_at(TNG *ptng, int type, void *disp_data, int coord_x, int coord_y);
errtype tng_buttonarray_addbutton(TNG *ptng, int type, void *disp_data);
errtype tng_buttonarray_setoffset(TNG *ptng, int offset_x, int offset_y);

// Draws button i,j of the buttonarray pointed to by ptng.
errtype tng_buttonarray_draw_button(TNG *ptng, int i, int j);

// Macros
#define TNG_BA(ptng) ((TNG_buttonarray *)(ptng->type_data))
#define TNG_BA_MSIZE(ptng) ((TNG_buttonarray *)(ptng->type_data))->msize
#define TNG_BA_BSIZE(ptng) ((TNG_buttonarray *)(ptng->type_data))->bsize
#define TNG_BA_WSIZE(ptng) ((TNG_buttonarray *)(ptng->type_data))->wsize
#define TNG_BA_LSEL(ptng) ((TNG_buttonarray *)(ptng->type_data))->lsel
#define TNG_BA_SPACING(ptng) ((TNG_buttonarray *)(ptng->type_data))->spacing
#define TNG_BA_OFFSET(ptng) ((TNG_buttonarray *)(ptng->type_data))->offset
#define TNG_BA_LASTKEY(ptng) ((TNG_buttonarray *)(ptng->type_data))->lastkey
#define TNG_BA_NUMSEL(ptng) ((TNG_buttonarray *)(ptng->type_data))->num_selectable
#define TNG_BA_SELECTED(ptng, x1, y) ((TNG_buttonarray *)((ptng)->type_data))->selected[(x1) + ((y) * TNG_BA_MSIZE(ptng).x)]
#define TNG_BA_INDEX(ptng,x1,y) ((TNG_buttonarray *)((ptng)->type_data))->matrix[(x1) + ((y) * TNG_BA_MSIZE(ptng).x)]

#endif // __TNGBARRY_H

