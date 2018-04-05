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
 * $Source: n:/project/lib/src/2d/RCS/genuchr.c $
 * $Revision: 1.6 $
 * $Author: lmfeeney $
 * $Date: 1994/06/15 01:20:04 $
 *
 * Generic unclipped character drawer.
 *
 * This file is part of the 2d library.
 *
 * $Log: genuchr.c $
 * Revision 1.6  1994/06/15  01:20:04  lmfeeney
 * support extended ascii (c > 127) w\ uchars, don't change fn i\f
 * 
 * Revision 1.5  1994/04/09  07:24:11  lmfeeney
 * added grs_font * as first arguement, #define for compatibility in str.h
 * 
 * Revision 1.4  1993/10/19  09:57:55  kaboom
 * Replaced #include   new headers.
 * 
 * Revision 1.3  1993/10/08  01:15:51  kaboom
 * Changed quotes in #include lines to angle brackets for Watcom.
 * 
 * Revision 1.2  1993/04/29  18:40:56  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.1  1993/04/08  18:54:48  kaboom
 * Initial revision
 */

#include "bitmap.h"
#include "ctxmac.h"
#include "grdbm.h"
#include "general.h"

/* draw an unclipped character c from the current font at (x,y). */

void gen_font_uchar (grs_font *f, char c, short x, short y)
{
   grs_bitmap bm;                /* character bitmap */
   short *off_tab;               /* character offset table */
   uchar *data_buf;              /* pixel data buffer */
   short offset;                 /* current character offset */

   /* range check char, get font table pointers, offset. */
    if ((uchar)c > f->max || (uchar)c < f->min)
      return;
   data_buf = (uchar *)f + f->buf;
   off_tab = f->off_tab;
   offset = off_tab[(uchar)c - f->min];
   gr_init_bm (&bm, NULL, (f->id==0xcccc)? BMT_FLAT8: BMT_MONO,
               BMF_TRANS, off_tab[(uchar)c - f->min+1]-offset, f->h);
   bm.row = f->w;
   /* draw the character with no clipping. */
   if (bm.type == BMT_MONO)
   {
      bm.bits = data_buf + (offset>>3);
      bm.align = offset&7;
      gr_mono_ubitmap (&bm, x, y);
   }
   else
   {
      bm.bits = data_buf + offset;
      gr_flat8_ubitmap (&bm, x, y);
   }
}
