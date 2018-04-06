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
#ifndef __TEXTGADG_H
#define __TEXTGADG_H

/*
 * $Source: n:/project/lib/src/ui/RCS/textgadg.h $
 * $Revision: 1.8 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:34 $
 *
 * $Log: textgadg.h $
 * Revision 1.8  1993/10/11  20:27:34  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.7  1993/04/28  14:40:22  mahk
 * Preparing for second exodus
 * 
 * Revision 1.6  1993/04/27  16:38:56  xemu
 * General improvmenet
 * 
 * Revision 1.5  1993/04/16  13:51:33  xemu
 * random fixes
 * 
 * Revision 1.4  1993/04/12  11:08:05  xemu
 * lots of new #defines
 * 
 * Revision 1.3  1993/04/07  21:40:29  xemu
 * Change style
 * 
 * Revision 1.2  1993/04/02  17:14:20  xemu
 * Style #defines
 * ,
 * 
 * Revision 1.1  1993/03/27  18:09:12  unknown
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "slider.h"
#include "texttool.h"
#include "tngtextg.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines

// Prototypes

// Creates a text gadget of size dim.
Gadget *gad_text_create(Gadget *parent, LGRect *dim, int z, ulong options, TNGStyle *sty, char *name);

Gadget *gad_textgadget_create_from_tng(void *ui_data, LGPoint loc, TNG **pptng, TNGStyle *sty, ulong options, LGPoint size);
// Globals

#endif // __TEXTGADG_H

