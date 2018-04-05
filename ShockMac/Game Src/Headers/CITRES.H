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
#ifndef __CITRES_H
#define __CITRES_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/citres.h $
 * $Revision: 1.8 $
 * $Author: mahk $
 * $Date: 1994/09/01 18:31:03 $
 *
 */

// Includes

// Defines

// Typedefs

// Prototypes
#define lock_bitmap_from_ref(r) lock_bitmap_from_ref_anchor(r,NULL)
grs_bitmap *lock_bitmap_from_ref_anchor(Ref r, LGRect *anchor);
#define get_bitmap_from_ref(r) get_bitmap_from_ref_anchor(r,NULL)
grs_bitmap *get_bitmap_from_ref_anchor(Ref r, LGRect *anchor);
errtype load_bitmap_from_res(grs_bitmap *bmp, Id id_num, int i, RefTable *rt, bool transp, LGRect *anchor, uchar *p);

// loads in a bitmap or a bitmap cursor, malloc'ing the bits 
// field. 
errtype simple_load_res_bitmap(grs_bitmap* bmp, Ref rid); 
errtype simple_load_res_bitmap_cursor(LGCursor* c, grs_bitmap* bmp, Ref rid);

// loads a bitmap, specifying whether to malloc the bits or not.
errtype load_res_bitmap(grs_bitmap* bmp, Ref rid, bool alloc); 
errtype load_res_bitmap_cursor(LGCursor* c, grs_bitmap* bmp, Ref rid, bool alloc);
errtype load_hires_bitmap_cursor(LGCursor* c, grs_bitmap* bmp, Ref rid, bool alloc);

errtype extract_temp_res_bitmap(grs_bitmap* bmp, Ref rid); 


// Globals



#endif // __CITRES_H

