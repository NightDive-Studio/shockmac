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
#ifndef __TNGPLAIN_H
#define __TNGPLAIN_H

/*
 * $Source: n:/project/lib/src/ui/RCS/tngplain.h $
 * $Revision: 1.2 $
 * $Author: dc $
 * $Date: 1993/10/11 20:27:39 $
 *
 * $Log: tngplain.h $
 * Revision 1.2  1993/10/11  20:27:39  dc
 * Angle is fun, fun fun fun
 * 
 * Revision 1.1  1993/05/12  16:29:59  xemu
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
} TNG_plain;

// Prototypes

// Initializes the TNG 
errtype tng_plain_init(void *ui_data, TNG *ptng, LGPoint size);

// Deallocate all memory used by the TNG 
errtype tng_plain_destroy(TNG *ptng);

// Fill in ppt with the size of the TNG 
errtype tng_plain_size(TNG *ptng, LGPoint *ppt);

// Returns the current "value" of the TNG
int tng_plain_getvalue(TNG *ptng);

// Macros
#define TNG_PL(ptng) ((TNG_plain *)(ptng->type_data))

#endif // __TNGPLAIN_H

