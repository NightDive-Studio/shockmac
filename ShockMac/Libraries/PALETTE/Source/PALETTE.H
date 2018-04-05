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
#ifndef __PALETTE_H
#define __PALETTE_H

/*
 * $Source: r:/prj/lib/src/palette/RCS/palette.h $
 * $Revision: 1.16 $
 * $Author: minman $
 * $Date: 1994/08/15 04:51:11 $
 *
 *
 */

// Do yourself a favor and quickly read the docs, found in
//   n:\project\lib\docs\palette.txt, before you use the palette
//   library.  It may save you some confusion.

#include "lg.h"
#include "fix.h"
#include "error.h"
#include "2d.h"
#include <stdlib.h>

/*
 * STRUCTS and TYPEDEFS
 */

typedef enum {						// Different palette f(x) types:
   SHIFT, CYCLE, CBANK			//   shift, single color cycle,
}  PAL_TYPE;							//   color banking

typedef enum {						// Status of loaded palette f(x)'s
   EMPTY, ACTIVE, DELAYED, FROZEN
}  PAL_STATUS;

typedef enum {						// Different cycling/banking modes
   REAL_TIME, STEADY
}  PAL_MODE;

// This is the structure for an entry in the Palette Special Effects table.
//   It supports different types of effects, and will hold different bits
//   of information according to what type of entry it is.

typedef struct {
   PAL_STATUS	status;
   PAL_TYPE		effect;
   PAL_MODE		mode;
   short      			dsteps;				// Holds delay
   short       		stages;				// Holds steps just for shift
   short       		curr_dstep;		// Holds where we are in delay
   short       		curr_stage;		// Holds where we are in shift
   short       		entry_1;			// Doubles to hold cmap_index for Cycles
   short       		entry_n;			// Doubles to hold curr_color for Cycles
   short       		range;				// Cmap range used (shift/bank)/#cols (cycle)
   uchar       		*from_pal;		// Doubles to hold colors array for Cycles
   uchar       		*to_pal;			// sometimes useless
} PAL_TABLE_ENTRY;

/*
 * MACROS
 */

#define palette_install_fade(m, i, j, d, s, p, q) palette_install_effect(SHIFT, m, i, j, d, s, p, q)
#define palette_install_cycle(m, i, nc, d, c) palette_install_effect(CYCLE, m, i, nc, d, 0, c, NULL)
#define palette_install_cbank(m, i, j, d) palette_install_effect(CBANK, m, i, j, d, 0, NULL, NULL)

/*
 * ROUTINE PROTOTYPES
 */

extern void palette_initialize(short table_size);
extern void palette_set_rate(short time_units_per_step);
extern void palette_shutdown();
extern void palette_init_smap(short first, short last, uchar *from, uchar *to,
                              short num_steps);

extern byte palette_install_effect(PAL_TYPE type,
                                   PAL_MODE mode,	// SHIFT  CYCLE  CBANK
                                   short b1,         		// first  index  first
                                   short b2,         		// last   #cols  last
                                   short b3,         		// delay  delay  delay
                                   short b4,         		// #steps --     --
                                   uchar *ptr1,      	// from   colors --
                                   uchar *ptr2);     	// to     --     --

extern errtype palette_remove_effect(byte id);
extern errtype palette_freeze_effect(byte id);
extern errtype palette_unfreeze_effect(byte id);
extern void palette_advance_effect(byte id, int steps); // DON'T call this
extern void palette_advance_all_fx(long timestamp);  // Call this, rather...
extern PAL_STATUS palette_query_effect(byte id);
extern void palette_change_delay(byte id, short delay);
extern void palette_swap_shadow(int s, int n, int d);

extern void palette_print_table();

extern byte num_installed_shifts;

#endif // __PALETTE_H









