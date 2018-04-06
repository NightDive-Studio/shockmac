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
#ifndef __PLAIN_H
#define __PLAIN_H

/*
 * $Source: n:/project/lib/src/ui/RCS/plain.h $
 * $Revision: 1.2 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:23 $
 *
 * $Log: plain.h $
 * Revision 1.2  1993/10/11  20:27:23  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.1  1993/05/12  16:29:53  xemu
 * Initial revision
 * 
 *
 */

// Includes
#include "lg.h"  // every file should have this
#include "tngplain.h"
#include "gadgets.h"

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes

// Defines

// Prototypes

// Creates a totally plain, boring TNG which is little more than
// a region in gadget's clothing
Gadget *gad_plain_create(Gadget *parent, LGRect *dim, int z, char *name);

Gadget *gad_plain_create_from_tng(void *ui_data, LGPoint loc, TNG **pptng, LGPoint size);

// Globals

#endif //  __PLAIN_H

