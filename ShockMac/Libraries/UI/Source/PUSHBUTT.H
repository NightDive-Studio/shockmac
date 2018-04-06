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
#ifndef __PUSHBUTT_H
#define __PUSHBUTT_H

/*
 * $Source: n:/project/lib/src/ui/RCS/pushbutt.h $
 * $Revision: 1.8 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:25 $
 *
 * $Log: pushbutt.h $
 * Revision 1.8  1993/10/11  20:27:25  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.7  1993/04/28  14:40:18  mahk
 * Preparing for second exodus
 * 
 * Revision 1.6  1993/04/22  15:05:23  xemu
 * creation from tng
 * 
 * Revision 1.5  1993/04/21  11:32:16  xemu
 * Revised to TNG
 * 
 * Revision 1.4  1993/04/12  15:17:52  xemu
 * use better default font
 * ..
 * 
 * 
 * Revision 1.3  1993/04/08  23:57:04  xemu
 * 2d fonts
 * 
 * Revision 1.2  1993/03/31  12:49:04  xemu
 * keyboard equivalents
 * 
 * Revision 1.1  1993/03/25  23:34:14  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "tngpushb.h"
#include "gadgets.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines

#define PB_TYPE(x) ((TNG_pushbutton *)((x)->tng_data->type_data))->type
#define PB_DISPDATA(x) ((TNG_pushbutton *)((x)->tng_data->type_data))->disp_data
#define PB_FONTBUF(x) ((TNG_pushbutton *)((x)->tng_data->type_data))->font_buf
#define PB_PRESSED(x) ((TNG_pushbutton *)((x)->tng_data->type_data))->pressed

// Prototypes

// Creates a pushbutton, which can display stuff and respond to basic events.
Gadget *gad_pushbutton_create(Gadget *parent, LGRect *dim, int z, int type, void *disp_data, TNGStyle *sty, char *name);

Gadget *gad_pushbutton_create_from_tng(void *ui_data, LGPoint loc, TNG **pptng, TNGStyle *sty, int button_type, void *display_data, LGPoint size);

// Globals

#endif //  __PUSHBUTT_H
