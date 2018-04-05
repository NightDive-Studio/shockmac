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
#ifndef __TNGTEXTG_H
#define __TNGTEXTG_H

/*
 * $Source: n:/project/lib/src/ui/RCS/tngtextg.h $
 * $Revision: 1.4 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:44 $
 *
 * $Log: tngtextg.h $
 * Revision 1.4  1993/10/11  20:27:44  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.3  1993/05/04  17:53:15  xemu
 * TNG_TX_ADDSTRING changed to use sequencing
 * 
 * Revision 1.2  1993/04/28  14:40:28  mahk
 * Preparing for second exodus
 * 
 * Revision 1.1  1993/04/27  16:37:25  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "texttool.h"
#include "tng.h"

typedef struct {
   TNG *tng_data;
   LGPoint size;
   short last_key;
   ulong options;
   TextTool *tt;
   TNG *hscroll_tng, *vscroll_tng;
} TNG_textgadget;

#define TNG_TG_TEXTAREA   0x0001
#define TNG_TG_HSCROLL  0x0002
#define TNG_TG_VSCROLL  0x0004

#define TNG_TG_BORDER_WIDTH   1
#define TNG_TG_SCROLL_X       14
#define TNG_TG_SCROLL_Y       14

#define TNG_TG_RETURN_KEY          0xd
#define TNG_TG_UP_KEY              0x48
#define TNG_TG_DOWN_KEY            0x50
#define TNG_TG_LEFT_KEY            0x4b
#define TNG_TG_RIGHT_KEY           0x4d
#define TNG_TG_SCROLL_LEFT_KEY     0x47
#define TNG_TG_SCROLL_RIGHT_KEY    0x4f
#define TNG_TG_SCROLL_UP_KEY       0x49
#define TNG_TG_SCROLL_DOWN_KEY     0x51

#define TNG_TG_SINGLE_LINE        0x0001
#define TNG_TG_LINE_SET           0x0002
#define TNG_TG_READ_ONLY          0x0004

#define TNG_TG_HORZ_SCROLL        0x1000
#define TNG_TG_VERT_SCROLL        0x2000

#define TNG_TG_SCROLLBARS   TNG_TG_HORZ_SCROLL | TNG_TG_VERT_SCROLL
// Prototypes

// Initializes the TNG 
// Note that both of these must be called!
// _init is called before the UI deals appropriately, _init2 is called afterwards.
errtype tng_textgadget_init(void *ui_data, TNG *ptng, TNGStyle *sty, ulong options, LGPoint size, LGPoint abs_loc);
errtype tng_textgadget_init2(TNG *ptng);

// Deallocate all memory used by the TNG 
errtype tng_textgadget_destroy(TNG *ptng);

// Draw the specified parts (may be all) of the TNG at screen coordinates loc
// assumes all appropriate setup has already been done!
errtype tng_textgadget_2d_draw(TNG *ptng, ushort partmask, LGPoint loc);

// Fill in ppt with the size of the TNG 
errtype tng_textgadget_size(TNG *ptng, LGPoint *ppt);

// Returns the current "value" of the TNG
int tng_textgadget_getvalue(TNG *ptng);

// React appropriately for receiving the specified cooked key
bool tng_textgadget_keycooked(TNG *ptng, ushort key);

// React appropriately for receiving the specified mouse button event
bool tng_textgadget_mousebutt(TNG *ptng, uchar type, LGPoint loc);

// Handle incoming signals
bool tng_textgadget_signal(TNG *ptng, ushort signal);

errtype tng_textgadget_scroll(TNG *ptng);

errtype tng_textgadget_addstring(TNG *ptng, char *s);

// Macros
#define TNG_TG(ptng) ((TNG_textgadget *)(ptng->type_data))
#define TNG_TG_SIZE(ptng) ((TNG_textgadget *)(ptng->type_data))->size
#define TNG_TG_LASTKEY(ptng) ((TNG_textgadget *)(ptng->type_data))->last_key
#define TNG_TG_TT(ptng) ((TNG_textgadget *)(ptng->type_data))->tt

#define TNG_TX_GETLINE(ptng,l) tt_get(TNG_TG_TT(ptng),(l))
#define TNG_TX_ADDSTRING(ptng,s) tng_textgadget_addstring(ptng, s); 
#define TNG_TX_CLEARLINE(ptng,l) tt_fill_line(TNG_TG_TT(ptng),TTF_REPLACE,(l),"\0"); _tt_do_event(TTEV_BOL)

#endif // __TNGTEXTG_H

