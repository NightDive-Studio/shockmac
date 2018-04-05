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
/*
 * $Source: r:/prj/lib/src/2d/RCS/rgb.h $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1994/07/21 08:25:17 $
 *
 * Prototypes and macros for rgb manipulation routines.
 *
 * This file is part of the 2d library.
 *
 * $Log: rgb.h $
 * Revision 1.4  1994/07/21  08:25:17  kaboom
 * Added gr_index_lrgb() macro.
 * 
 * Revision 1.3  1993/10/19  10:31:44  kaboom
 * Changed gr_index_brgb to calculate an index from a bound rgb value
 * and gr_index_rgb to calculate an index from 3 fixed rgb values.
 * 
 * Revision 1.2  1993/10/15  14:42:56  baf
 * Removed offending semicolon from end of macro
 * 
 * Revision 1.1  1993/02/04  17:44:03  kaboom
 * Initial revision
 * 
 */

#ifndef __RGB_H
#define __RGB_H

#define RGB_OK (0)
#define RGB_OUT_OF_MEMORY (-1)
#define RGB_CANT_DEALLOCATE (-2)
#define RGB_IPAL_NOT_ALLOCATED (-3)

/* convert fixed-point (r,g,b) triplet into a 15-bit ipal index. */
#define gr_index_rgb(r,g,b) \
   (((r)>>19)&0x1f) | (((g)>>14)&0x3e0) | (((b)>>9)&0x7c00)

/* convert an 8-8-8 long rgb into a 15-bit ipal index. */
#define gr_index_lrgb(t) \
   ((((t)>>3)&0x1f) | (((t)>>6)&0x3e0) | (((t)>>9)&0x7c00))

/* convert 8-bit r,g,b into grs_rgb format. */
#define gr_bind_rgb(r,g,b) (((r)<<2)|((g)<<13)|((b)<<24))

/* convert a grs_rgb value into a 15-bit inverse palette table index. */
#define gr_index_brgb(c) \
   ((((c)>>5)&0x1f)|(((c)>>11)&0x3e0)|(((c)>>17)&0x7c00))

extern void gr_split_rgb (grs_rgb c, uchar *r, uchar *g, uchar *b);

/* Generate an inverse palette for the given screen palette */
int gr_alloc_ipal(void);

/* Reinitialize the ipal for the current palette */
int gr_init_ipal(void);

/* Destroy the current inverse palette, freeing memory */
int gr_free_ipal(void);

#endif /* !__RGB_H */
