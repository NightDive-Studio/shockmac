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
#ifndef __TNGSLIDR_H
#define __TNGSLIDR_H

/*
 * $Source: n:/project/lib/src/ui/RCS/tngslidr.h $
 * $Revision: 1.9 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:42 $
 *
 * $Log: tngslidr.h $
 * Revision 1.9  1993/10/11  20:27:42  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.8  1993/06/29  10:28:14  xemu
 * sliding sliders
 * 
 * Revision 1.7  1993/06/10  13:34:36  xemu
 * stuff
 * 
 * Revision 1.6  1993/05/10  17:42:33  xemu
 * changed type of parameter to tng_slider_set
 * 
 * Revision 1.5  1993/04/29  19:04:37  xemu
 * removed resource.h include
 * 
 * Revision 1.4  1993/04/28  14:40:27  mahk
 * Preparing for second exodus
 * 
 * Revision 1.3  1993/04/27  16:38:29  xemu
 * general improvement
 * 
 * Revision 1.2  1993/04/22  15:04:47  xemu
 * more macros
 * 
 * Revision 1.1  1993/04/21  11:31:13  xemu
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
   int alignment;
   int min, max;
   int value, increm;
   bool dragging;
   Ref left_id, right_id, up_id, down_id;
   Ref slider_id;
} TNG_slider;

#define TNG_SL_HORIZONTAL  0
#define TNG_SL_VERTICAL    1

#define TNG_SL_INCREMENTER 0x0001
#define TNG_SL_DECREMENTER 0x0002
#define TNG_SL_SLIDER      0x0004
#define TNG_SL_ALLPARTS    TNG_ALLPARTS

#define TNG_SL_LEFT_KEY        0x1c
#define TNG_SL_RIGHT_KEY       0x1d
#define TNG_SL_UP_KEY          0x1e
#define TNG_SL_DOWN_KEY        0x1f

// Prototypes

// Initializes the TNG slider
errtype tng_slider_init(void *ui_data, TNG *ptng, TNGStyle *sty, int alignment, int min, int max, int value, int increm, LGPoint size);

// Initializes the TNG slider
errtype tng_slider_full_init(void *ui_data, TNG *ptng, TNGStyle *sty, int alignment, int min, int max, int value, int increm, LGPoint size,
   Ref left_id, Ref right_id, Ref up_id, Ref down_id, Ref slider_id);

// Deallocate all memory used by the TNG slider
errtype tng_slider_destroy(TNG *ptng);

// Draw the specified parts (may be all) of the TNG slider at screen coordinates loc
// assumes all appropriate setup has already been done!
errtype tng_slider_2d_draw(TNG *ptng, ushort partmask, LGPoint loc);

// Fill in ppt with the size of the TNG slider
errtype tng_slider_size(TNG *ptng, LGPoint *ppt);

// Returns the current "value" of the TNG slider
int tng_slider_getvalue(TNG *ptng);

// React appropriately for receiving the specified cooked key
bool tng_slider_keycooked(TNG *ptng, ushort key);

// React appropriately for receiving the specified mouse button event
bool tng_slider_mousebutt(TNG *ptng, uchar type, LGPoint loc);

// React to a click at the given location
bool tng_slider_apply_click(TNG *ptng, LGPoint loc);

// Handle incoming signals
bool tng_slider_signal(TNG *ptng, ushort signal);

bool tng_slider_increm(TNG_slider *ptng);
bool tng_slider_decrem(TNG_slider *ptng);
errtype tng_slider_set(TNG_slider *ptng, int perc);

// Macros
#define TNG_SL(ptng) ((TNG_slider *)ptng->type_data)
#define TNG_SL_VALFRAC(psltng) ((float)(psltng->value - psltng->min)) / ((float)(psltng->max - psltng->min))

#endif // __TNGSLIDR_H

