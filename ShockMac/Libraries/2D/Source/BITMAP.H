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
 * $Source: r:/prj/lib/src/2d/RCS/bitmap.h $
 * $Revision: 1.12 $
 * $Author: kevin $
 * $Date: 1994/08/16 15:33:32 $
 *
 * Constants for bitmap flags & type fields; prototypes for bitmap
 * functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: bitmap.h $
 * Revision 1.12  1994/08/16  15:33:32  kevin
 * Added REAL_BM_TYPES constant.
 * 
 * Revision 1.11  1994/05/19  15:08:20  kevin
 * Added flag BMF_TLUC8 for rsd8 bitmaps that want to be tluc8 bitmaps when uncompressed.
 * 
 * Revision 1.10  1994/02/14  22:15:25  baf
 * Pruned vestigial 16/bit translucency
 * 
 * Revision 1.9  1993/11/19  17:22:24  unknown
 * Added 8/bit translucent bitmaps.
 * 
 * Revision 1.8  1993/11/15  03:23:55  baf
 * Added BMT_GEN, for new generic canvas table.
 * (Methinks the name 'bitmap_types' is getting
 * somewhat strained?
 * 
 * Revision 1.7  1993/11/11  00:09:00  baf
 * Added BMT_TYPES to enum, to indicate number of
 * types of bitmap
 * 
 * Revision 1.6  1993/10/21  00:29:23  baf
 * Changed TRANSLUCENT to TLUC, for consistency's sake
 * 
 * Revision 1.5  1993/10/19  09:49:54  kaboom
 * Replaced #include <grd.h> with new headers split from grd.h.
 * 
 * Revision 1.4  1993/10/15  12:19:37  baf
 * Added support for translucent bitmaps
 * 
 * Revision 1.3  1993/09/09  00:17:56  kaboom
 * Added prototype for gr_alloc_bitmap().
 * 
 * Revision 1.2  1993/05/03  15:09:26  kaboom
 * Added BMT_SPAN to bitmap type list.
 * 
 * Revision 1.1  1993/02/04  16:59:15  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log entries from old 2d.h
 * Revision 1.15  1993/01/07  20:03:47  kaboom
 * Added new bitmap type, BMT_BANK8 for 8-bit bank-switched memory.
 * Changed prototype for gr_set_driver.
 * 
 * Revision 1.10  1992/12/11  20:24:22  kaboom
 * Changed name of BMT_DRIVER to BMT_DEVICE for distinction between
 * device drivers and bitmap drivers.
 */

#ifndef __BITMAP_H
#define __BITMAP_H
#include "grs.h"

/* bitmap types. */
enum {
   BMT_DEVICE,
   BMT_MONO,
   BMT_FLAT8,
   BMT_FLAT24,
   BMT_RSD8,
   BMT_TLUC8,
   BMT_SPAN,
   BMT_GEN,
   BMT_TYPES
};

/* BMT_GEN and BMT_SPAN are not true bitmap types. */
#define REAL_BMT_TYPES BMT_SPAN

/* bitmap flags. */
#define BMF_TRANS    1
#define BMF_TLUC8    2

/* function prototypes for bitmap routines. */
extern void gr_init_bitmap
   (grs_bitmap *bm, uchar *p, uchar type, ushort flags, short w, short h);
extern void gr_init_sub_bitmap
   (grs_bitmap *sbm, grs_bitmap *dbm, short x, short y, short w, short h);
extern grs_bitmap *gr_alloc_bitmap
   (uchar type, ushort flags, short w, short h);

/* compatibility defines. */
#define gr_init_bm gr_init_bitmap
#define gr_init_sub_bm gr_init_sub_bitmap
#define gr_alloc_bm gr_alloc_bitmap
#endif /* !__BITMAP_H */
