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
#ifndef __SLIDER_H
#define __SLIDER_H

/*
 * $Source: n:/project/lib/src/ui/RCS/slider.h $
 * $Revision: 1.12 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:33 $
 *
 * $Log: slider.h $
 * Revision 1.12  1993/10/11  20:27:33  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.11  1993/04/29  19:04:24  xemu
 * resource.h removed
 * 
 * Revision 1.10  1993/04/28  14:40:21  mahk
 * Preparing for second exodus
 * 
 * Revision 1.9  1993/04/27  16:38:19  xemu
 * resource ripout
 * 
 * Revision 1.8  1993/04/22  15:05:28  xemu
 * creation from mfd
 * 
 * Revision 1.7  1993/04/21  11:32:05  xemu
 * Revised to TNG
 * 
 * Revision 1.6  1993/04/12  12:08:13  xemu
 * SL_CHANGE
 * 
 * Revision 1.5  1993/04/02  14:40:30  xemu
 * Style defines
 * 
 * Revision 1.4  1993/04/01  13:46:01  xemu
 * made increm and decrem CBs public
 * 
 * Revision 1.3  1993/03/31  12:48:49  xemu
 * added keyboard constants
 * 
 * Revision 1.2  1993/03/27  18:08:58  unknown
 * Added SL_ANY_SLIDER
 * 
 * Revision 1.1  1993/03/26  17:13:02  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "tngslidr.h"
#include "gadgets.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines
// #define SL_RES_LEFT   REF_IMG_bmCursorLeft
// #define SL_RES_RIGHT  REF_IMG_bmCursorRight
// #define SL_RES_UP     REF_IMG_bmCursorUp
// #define SL_RES_DOWN   REF_IMG_bmCursorDown
// #define SL_RES_SLIDER REF_IMG_bmHelmTinyHead

// #define SL_RES_LEFT   REF_IMG_iconArrowLt
// #define SL_RES_RIGHT  REF_IMG_iconArrowRt
// #define SL_RES_UP     REF_IMG_iconArrowUp
// #define SL_RES_DOWN   REF_IMG_iconArrowDn
// #define SL_RES_SLIDER REF_IMG_bmHelmTinyHead

#define SL_VERTICAL  TNG_SL_VERTICAL
#define SL_HORIZONTAL TNG_SL_HORIZONTAL

#define SL_VALUE(x) ((TNG_slider *)((x)->tng_data->type_data))->value
#define SL_ALIGNMENT(x) ((TNG_slider *)((x)->tng_data->type_data))->alignment
#define SL_MAX(x) ((TNG_slider *)((x)->tng_data->type_data))->max
#define SL_MIN(x) ((TNG_slider *)((x)->tng_data->type_data))->min
#define SL_INCREM(x) ((TNG_slider *)((x)->tng_data->type_data))->increm
#define SL_STYLE(x) ((TNG_slider *)((x)->tng_data->type_data))->style
#define SL_CHANGE(x) ((TNG_slider *)((x)->tng_data->type_data))->change_flag

// Prototypes
Gadget *gad_slider_create(Gadget *parent, LGRect *dim, int z, int alignment, int increment,
   int min, int max, TNGStyle *sty, char *name);

Gadget *gad_slider_create_full(Gadget *parent, LGRect *dim, int z, int alignment, int increment,
   int min, int max, TNGStyle *sty, char *name, Ref res_left, Ref res_right, Ref res_up,
   Ref res_down, Ref res_slider);

Gadget *gad_slider_create_from_tng(void *ui_data, LGPoint loc, TNG **pptng, TNGStyle *sty, int alignemnt, int min, int max,
   int value, int increment, LGPoint size);

// Globals

#endif // __SLIDER_H

