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
 * $Source: n:/project/lib/src/2d/RCS/pixtab.h $
 * $Revision: 1.1 $
 * $Author: lmfeeney $
 * $Date: 1994/06/11 00:55:52 $
*/

#ifndef __PIXFILL_H
#define __PIXFILL_H

#include "pixfill.h"

/* these tables are used in the 2d, but are not visible, except though the
    current fill ptr (see pixfill.h) 
*/

// MLA (this won't compile?) extern grt_pixel_fill grd_pixel_fill_table[];

/* fill type functions living in the table -- wrappers around gr_set_upixel */

extern void gri_pixel_ns (long color, long fill_parm, int x, int y);
extern void gri_pixel_clut (long color, long fill_parm, int x, int y);
extern void gri_pixel_xor (long color, long fill_parm, int x, int y);
extern void gri_pixel_blend (long color, long fill_parm, int x, int y);

#endif
