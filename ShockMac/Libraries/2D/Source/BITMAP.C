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
 * $Source: n:/project/lib/src/2d/RCS/bitmap.c $
 * $Revision: 1.11 $
 * $Author: baf $
 * $Date: 1994/02/14 22:14:42 $
 *
 * Constants for bitmap flags & type fields.
 *
 * Bitmap handling routines.
 *
 * $Log: bitmap.c $
 * Revision 1.11  1994/02/14  22:14:42  baf
 * Pruned vestigial 16/bit translucency
 * 
 * Revision 1.10  1994/01/20  16:49:31  kaboom
 * Changed temporary bitmap to static so can be called in interrupt.
 * 
 * Revision 1.9  1993/11/19  17:22:02  unknown
 * Added 8/bit translucent bitmaps
 * 
 * Revision 1.8  1993/10/21  00:28:40  baf
 * Changed TRANSLUCENT to TLUC, for consistency's sake.
 * 
 * Revision 1.7  1993/10/19  09:49:53  kaboom
 * Replaced #include <grd.h> with new headers split from grd.h.
 * 
 * Revision 1.6  1993/10/15  12:19:25  baf
 * Added support for translucent bitmaps
 * 
 * Revision 1.5  1993/10/08  01:14:52  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.4  1993/09/09  00:17:41  kaboom
 * Added gr_alloc_bitmap() routine.
 * 
 * Revision 1.3  1993/07/08  23:02:24  kaboom
 * Added code in gr_init_bm() to set wlog and hlog fields.
 * 
 * Revision 1.2  1993/04/29  16:10:30  kaboom
 * Fixed up 24-bit case for init_bm.  Updated gr_sub_bm to gr_sub_bitmap.
 * 
 * Revision 1.1  1993/02/04  16:58:51  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log entries from old gr.c
 * Revision 1.6  1992/12/11  20:31:13  kaboom
 * Changed gr_init_bm from macro to function; now it calculates row from
 * width and assumes align is 0.  Added gr_init_sub_bm to initialize a
 * subsection of an existing bitmap.  Updated references from BMT_DRIVER to
 * BMT_DEVICE.
 */

#include <string.h>

#include "lg.h"
#include "grs.h"
#include "bitmap.h"
#include "grbm.h"
#include "grmalloc.h"

extern long		gScreenRowbytes;

/* initialize a new bitmap structure. set bits, type, flags, w, and h from
   arguments. set align to 0 and calculate row from width depending on what
   type of bitmap. */
void gr_init_bm (grs_bitmap *bm, uchar *p, uchar type, ushort flags,
                 short w, short h)
 {
	int row;
  int v;

  /* calculate row from type and w. */
  switch (type) {
		case BMT_DEVICE: row = gScreenRowbytes; break; 	// row = gr_calc_row (w); break;
   	case BMT_MONO: row = (w+7)/8; break;
  	case BMT_FLAT8: case BMT_TLUC8: row = w; break;
   	case BMT_FLAT24: row = 3*w; break;
   	case BMT_RSD8: row = 0; break;
   	default: break;
   }

	bm->bits = p;
	bm->type = type;
	bm->flags = flags;
	bm->align = 0;
	bm->w = w;
	bm->h = h;
	bm->row = row;
	bm->wlog = bm->hlog = 0;

	for (v=w>>1; v!=0; v>>=1)
		bm->wlog++;
	for (v=h>>1; v!=0; v>>=1)
		bm->hlog++;
}

/* set up a new bitmap structure to be a subsection of an existing bitmap.
   sbm is source bm, dbm destination. (0,0) of dbm maps to (x,y) of sbm,
   and dbm is w x h in size. */
void gr_init_sub_bm (grs_bitmap *sbm, grs_bitmap *dbm, short x, short y,
                     short w, short h)
{
	*dbm = *sbm; 	// memcpy (dbm, sbm, sizeof (*sbm));
	dbm->w = w; dbm->h = h;

	switch (sbm->type) {
	case BMT_DEVICE:
		/* chain to device sub bm. */
		gr_sub_bitmap (dbm, x, y, w, h);
		break;
	case BMT_MONO:
		dbm->bits += y*dbm->row + x/8;
		if ((dbm->align+=x%8) > 7)  {
			dbm->align -= 8;
			dbm->bits++;
		}
		break;
	case BMT_FLAT8:
	case BMT_TLUC8:
		dbm->bits += y*dbm->row + x;
		break;
	case BMT_FLAT24:
		dbm->bits += y*dbm->row + 3*x;
		break;
	case BMT_RSD8: break;
	default: break;
	}
}

/* allocate memory for a bitmap structure and the data for a bitmap of
   the specified type and flags of size w x h.  returns a pointer to the
   new bitmap structure.  the returned pointer can be freed in order to
   free both the structure and data memory. */
grs_bitmap *gr_alloc_bitmap (uchar type, ushort flags, short w, short h)
{
  grs_bitmap tmp_bm;	  /* temporary space for bitmap init */
	uchar *p;             /* pointer to allocated buffer */

	gr_init_bitmap(&tmp_bm, NULL, type, flags, w, h);
//	p=(uchar *)gr_malloc(sizeof(tmp_bm)+(tmp_bm.row*tmp_bm.h));
	p=(uchar *) NewPtr(sizeof(tmp_bm)+(tmp_bm.row*tmp_bm.h));
	if (p) 
	 {
		tmp_bm.bits = p+sizeof(tmp_bm);
		* (grs_bitmap *) p = tmp_bm;	// LG_memcpy(p, &tmp_bm, sizeof(tmp_bm));
	 }
	return (grs_bitmap *) p;
}
