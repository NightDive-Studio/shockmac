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
 * $Source: n:/project/lib/src/2d/RCS/genustr.c $
 * $Revision: 1.7 $
 * $Author: lmfeeney $
 * $Date: 1994/06/15 01:18:55 $
 *
 * Generic unclipped string drawer.
 *
 * This file is part of the 2d library.
 *
 * $Log: genustr.c $
 * Revision 1.7  1994/06/15  01:18:55  lmfeeney
 * support extended ascii (c > 127) w\ uchars, don't change fn i\f
 * 
 * Revision 1.6  1994/04/09  07:24:51  lmfeeney
 * added grs_font * as first argument, #define for compatibility in str.h
 * 
 * Revision 1.5  1993/10/19  09:57:55  kaboom
 * Replaced #include   new headers.
 * 
 * Revision 1.4  1993/10/08  01:15:51  kaboom
 * Changed quotes in #include lines to angle brackets for Watcom.
 * 
 * Revision 1.3  1993/06/02  16:21:27  kaboom
 * Now handles hard and soft carriage returns and soft spaces, as
 * inserted by line wrapping.
 * 
 * Revision 1.2  1993/04/29  18:40:58  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.1  1993/04/08  16:26:22  kaboom
 * Initial revision
 */

#include "bitmap.h"
#include "ctxmac.h"
#include "grdbm.h"
#include "chr.h"
#include "general.h"


/* draw a string s in the specified font at (x0,y0).  does not perform
   any clipping. */

void gen_font_ustring (grs_font *f, char *s, short x0, short y0)
{
   grs_bitmap bm;             /* character bitmap */
   short *offset_tab;         /* table of character offsets */
   uchar *char_buf;           /* font pixel data */
   short offset;              /* offset of current character */
   short x, y;                /* position of current character */
   uchar c;                    /* current character */

   char_buf = (uchar *)f + f->buf;
   offset_tab = f->off_tab;
   gr_init_bm (&bm, NULL, (f->id==0xcccc)? BMT_FLAT8: BMT_MONO,
               BMF_TRANS, 0, f->h);
   bm.row = f->w;

   x = x0; y = y0;
   while ((c=(uchar) (*s++)) != '\0') {
      if (c=='\n' || c==CHAR_SOFTCR) {
         x = x0;
         y += f->h;
         continue;
      }
      if (c>f->max || c<f->min || c==CHAR_SOFTSP)
         continue;
      offset = offset_tab[c-f->min];
      bm.w = offset_tab[c-f->min+1]-offset;
      if (bm.type == BMT_MONO) {
         bm.bits = char_buf + (offset>>3);
         bm.align = offset&7;
         gr_mono_ubitmap (&bm, x, y);
      }
      else {
         bm.bits = char_buf + offset;
         gr_flat8_ubitmap (&bm, x, y);
      }
      x += bm.w;
   }
}
