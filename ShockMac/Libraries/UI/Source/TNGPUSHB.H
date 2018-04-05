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
#ifndef __TNGPUSHB_H
#define __TNGPUSHB_H

/*
 * $Source: n:/project/lib/src/ui/RCS/tngpushb.h $
 * $Revision: 1.5 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:40 $
 *
 * $Log: tngpushb.h $
 * Revision 1.5  1993/10/11  20:27:40  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.4  1993/04/28  14:40:25  mahk
 * Preparing for second exodus
 * 
 * Revision 1.3  1993/04/27  16:38:36  xemu
 * general improvement
 * 
 * Revision 1.2  1993/04/22  15:05:01  xemu
 * removed font macro
 * 
 * Revision 1.1  1993/04/21  11:31:04  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "tng.h"

// Typedefs
typedef struct {
   TNG *tng_data;
   LGPoint size;
   int type;
   bool pressed;
   void *disp_data;
} TNG_pushbutton;

#define TNG_NULL_TYPE       -1
#define TNG_RESOURCE_TYPE    0
#define TNG_TEXT_TYPE        1
#define TNG_TRANSPARENT_TYPE 2
#define TNG_COLORED_TYPE     3

// Prototypes

// Initializes the TNG 
errtype tng_pushbutton_init(void *ui_data, TNG *ptng, TNGStyle *sty, int button_type, void *display_data, LGPoint size);

// Deallocate all memory used by the TNG 
errtype tng_pushbutton_destroy(TNG *ptng);

// Draw the specified parts (may be all) of the TNG at screen coordinates loc
// assumes all appropriate setup has already been done!
errtype tng_pushbutton_2d_draw(TNG *ptng, ushort partmask, LGPoint loc);

// Fill in ppt with the size of the TNG 
errtype tng_pushbutton_size(TNG *ptng, LGPoint *ppt);

// Returns the current "value" of the TNG
int tng_pushbutton_getvalue(TNG *ptng);

// React appropriately for receiving the specified cooked key
bool tng_pushbutton_keycooked(TNG *ptng, ushort key);

// React appropriately for receiving the specified mouse button event
bool tng_pushbutton_mousebutt(TNG *ptng, uchar type, LGPoint loc);

// Handle incoming signals
bool tng_pushbutton_signal(TNG *ptng, ushort signal);

errtype tng_pushbutton_pressed(TNG_pushbutton *ppbtng);
errtype tng_pushbutton_released(TNG_pushbutton *ppbtng);

// Macros
#define TNG_PB(ptng) ((TNG_pushbutton *)(ptng->type_data))

#endif // __TNGPUSHB_H

