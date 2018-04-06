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
 * $Source: n:/project/lib/src/2d/RCS/genchr.c $
 * $Revision: 1.6 $
 * $Author: lmfeeney $
 * $Date: 1994/06/15 01:20:11 $
 *
 * Generic clipped character drawer.
 *
 * This file is part of the 2d library.
 *
 * $Log: genchr.c $
 * Revision 1.6  1994/06/15  01:20:11  lmfeeney
 * support extended ascii (c > 127) w\ uchars, don't change fn i\f
 * 
 * Revision 1.5  1994/04/09  07:20:12  lmfeeney
 * routine takes grs_font * as first arg, #define for compatibility in 
 * str.h
 * 
 * Revision 1.4  1993/10/19  09:57:52  kaboom
 * Replaced #include   new headers.
 * 
 * Revision 1.3  1993/10/02  01:17:16  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.2  1993/04/29  18:40:17  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.1  1993/04/08  18:54:36  kaboom
 * Initial revision
 */

#include "bitmap.h"
#include "clpcon.h"
#include "ctxmac.h"
#include "grdbm.h"
#include "general.h"


/* draw a clipped character c from the specified font at (x,y). */

int gen_font_char (grs_font *f, char c, short x, short y)
{
   grs_bitmap bm;             /* character bitmap */
   short *off_tab;            /* character offset table */
   uchar *data_buf;           /* pixel data buffer */
   short offset;              /* current character offset */

   /* range check char, get font table pointers, offset. */
   if ((uchar)c > f->max || (uchar)c < f->min)
      return CLIP_ALL;
   data_buf = (uchar *)f + f->buf;
   off_tab = f->off_tab;
   offset = off_tab[(uchar)c - f->min];
   gr_init_bm (&bm, NULL, (f->id==0xcccc)? BMT_FLAT8: BMT_MONO,
               BMF_TRANS, off_tab[(uchar)c - f->min+1]-offset, f->h);
   bm.row = f->w;
   /* draw the character with clipping. */
   if (bm.type == BMT_MONO) {
      bm.bits = data_buf + (offset>>3);
      bm.align = offset&7;
      return gr_mono_bitmap (&bm, x, y);
   }
   else {
      bm.bits = data_buf + offset;
      return gr_flat8_bitmap (&bm, x, y);
   }
}
