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
#ifndef __TNGAPP_H
#define __TNGAPP_H

/*
 * $Source: n:/project/lib/src/ui/RCS/tngapp.h $
 * $Revision: 1.13 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:37 $
 *
 * $Log: tngapp.h $
 * Revision 1.13  1993/10/11  20:27:37  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.12  1993/08/24  15:57:54  xemu
 * remove prototype not found error
 * 
 * Revision 1.11  1993/08/18  16:33:56  xemu
 * added foreign obscuration
 * 
 * Revision 1.10  1993/08/16  18:25:01  xemu
 * TNG_IF_UNOBSCURED added
 * 
 * Revision 1.9  1993/08/11  10:22:25  xemu
 * added extern prototypes
 * 
 * Revision 1.8  1993/08/10  19:12:10  xemu
 * TNG_IF_OBSCURED
 * 
 * Revision 1.7  1993/07/06  11:39:53  xemu
 * added some mouse & screen size functions
 * 
 * Revision 1.6  1993/06/29  10:28:05  xemu
 * grab & release focus
 * 
 * Revision 1.5  1993/06/17  18:57:40  mahk
 * Changed TNG_ABSLOC
 * 
 * Revision 1.4  1993/04/28  14:40:24  mahk
 * Preparing for second exodus
 * 
 * Revision 1.3  1993/04/27  16:37:37  xemu
 * more mods
 * 
 * Revision 1.2  1993/04/22  15:04:29  xemu
 * TNG creation macros
 * 
 * Revision 1.1  1993/04/21  11:30:44  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "region.h"
#include "gadgets.h"
//#include <pushbutt.h>
//#include <slider.h>

#define TNG_ALLPARTS 0xffff

// Macros!!

#define TNG_GADGET(ptng) ((Gadget *)((ptng)->ui_data))

#define TNG_DRAW(ptng) TNG_DRAWPART(ptng, TNG_ALLPARTS)
#define TNG_DRAWPART(ptng, pmask) gadget_display_part((Gadget *)(ptng->ui_data),((Gadget *)(ptng->ui_data))->rep->r, pmask)
#define TNG_DRAWRECT(ptng, r) gadget_display((Gadget *)((ptng)->ui_data), (r))

extern LGPoint tng_absloc(TNG* ptng);
#define TNG_ABSLOC(ptng) tng_absloc(ptng)

#define GUI_MALLOC(uid, size)   NewPtr(size)
#define GUI_DEALLOC(uid, victim) DisposePtr((Ptr)victim)

#define TNG_GRAB_FOCUS(ptng, evmask) uiGrabFocus(((Gadget *)((ptng)->ui_data))->rep, evmask)
#define TNG_RELEASE_FOCUS(ptng, evmask) uiReleaseFocus(((Gadget *)((ptng)->ui_data))->rep, evmask)

#define TNG_GET_MOUSE(px, py) mouse_get_xy((px),(py))
#define TNG_PUT_MOUSE(x, y) mouse_put_xy((x), (y))

#define TNG_SCREEN_SIZE_X(ptng) 320;
#define TNG_SCREEN_SIZE_Y(ptng) 200;

#define TNG_IF_OBSCURED(ptng) if (region_obscured(TNG_GADGET(ptng)->rep, TNG_GADGET(ptng)->rep->r) == COMPLETELY_OBSCURED)
#define TNG_IF_UNOBSCURED(ptng) if (region_obscured(TNG_GADGET(ptng)->rep, TNG_GADGET(ptng)->rep->r) == UNOBSCURED)

#define TNG_IF_FOREIGN_OBSCURED(ptng) if (foreign_region_obscured(TNG_GADGET(ptng)->rep, TNG_GADGET(ptng)->rep->r) == COMPLETELY_OBSCURED)
#define TNG_IF_FOREIGN_UNOBSCURED(ptng) if (foreign_region_obscured(TNG_GADGET(ptng)->rep, TNG_GADGET(ptng)->rep->r) == UNOBSCURED)
#define TNG_FOREIGN_OBSCURED(ptng) (foreign_region_obscured(TNG_GADGET(ptng)->rep, TNG_GADGET(ptng)->rep->r))

// Macros for creating supported TNGs....

extern Gadget *gad_pushbutton_create_from_tng(void *ui_data, LGPoint loc, TNG **pptng, TNGStyle *sty, int button_type,
   void *display_data, LGPoint size);
#define TNG_CREATE_PUSHBUTTON(ui_data, loc, pptng, sty, button_type, display_data, size) gad_pushbutton_create_from_tng(ui_data, loc, pptng, sty, button_type, display_data, size)
   
extern Gadget *gad_slider_create_from_tng(void *ui_data, LGPoint loc, TNG **pptng, TNGStyle *sty, int alignment, int min, int max,
   int value, int increment, LGPoint size);
#define TNG_CREATE_SLIDER(ui_data, loc, pptng, sty, alignment, min, max, value, increm, size) gad_slider_create_from_tng(ui_data, loc, pptng, sty, alignment, min, max, value, increm, size)

extern Gadget *gad_textgadget_create_from_tng(void *ui_data, LGPoint loc, TNG **pptng, TNGStyle *sty, ulong options, LGPoint size);
#define TNG_CREATE_TEXT(ui_data, loc, pptng, sty, options, size)  gad_textgadget_create_from_tng(ui_data, loc, pptng, sty, options, size)

#endif // __TNGAPP_H

