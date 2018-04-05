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
 * $Source: r:/prj/lib/src/2d/RCS/fl8rsd8.c $
 * $Revision: 1.6 $
 * $Author: kevin $
 * $Date: 1994/10/27 18:26:56 $
 * 
 * Routines for drawing rsd bitmaps into a flat 8 canvas.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8rsd8.c $
 * Revision 1.6  1994/10/27  18:26:56  kevin
 * fixed gr_rsd8_blit prototype.
 * 
 * Revision 1.5  1994/10/25  15:13:14  kevin
 * Renamed funcs, use fast rsd blitter when possible.
 * 
 * Revision 1.4  1993/10/19  09:50:54  kaboom
 * Replaced #include <grd.h" with new headers split from grd.h.
 * 
 * Revision 1.3  1993/10/01  16:00:47  kaboom
 * Converted to include clpcon.h instead of clip.h
 * 
 * Revision 1.2  1993/07/13  17:47:13  kaboom
 * Fixed bugs in clipping case off right edge.  Updated for padded clipping
 * rectangle.
 * 
 * Revision 1.1  1993/02/16  14:17:21  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log from old flat8.c:
 * Revision 1.7  1992/12/14  18:08:40  kaboom
 * Fixed bug in flat8_rsd8_[u]bitmap() -- were omitting bottom lines.
 */

#include <string.h>
#include "clpcon.h"
#include "cnvdat.h"
#include "rsd.h"
#include "grdbm.h"
#include "fl8tf.h"
#include "lg.h"
#include "general.h"


// prototypes
void gr_rsd8_blit (uchar *rsd_src, uchar *dst, int grd_bm_row, int bm_w);


//### MLA- not supposed to be used (PC code is in RSDBLT.ASM)
void gr_rsd8_blit (uchar *rsd_src, uchar *dst, int grd_bm_row, int bm_w)
 {
	DebugStr("\pask mark");
 }
 

void gri_flat8_rsd8_ubitmap(grs_bitmap *bm, short x, short y)
{
/*   uchar *p_dst;
   uchar *rsd_src;                     // rsd source buffer 

   rsd_src = bm->bits;
   p_dst = grd_bm.bits + grd_bm.row*y + x;
   gr_rsd8_blit(rsd_src,p_dst,grd_bm.row,bm->w);*/
	unpack_rsd8_ubitmap(bm,x,y);
}
          
int gri_flat8_rsd8_bitmap (grs_bitmap *bm, short x_left, short y_top)
{
   short x,y;                          /* current destination position */
   short x_right,y_bot;                /* opposite edges of bitmap */
   short x_off,y_off;                  /* x,y offset for clip */
   ulong start_byte;                   /* byte to start drawing */
   ulong cur_byte;                     /* current position within rsd */
   uchar *p_dst;
   uchar *rsd_src;                     /* rsd source buffer */
   short rsd_code;                     /* last rsd opcode */
   short rsd_count;                    /* count for last opcode */
   short op_count;                     /* operational count */
   int code;                           /* clip code to return */

   rsd_src = bm->bits;
   x = x_left; y = y_top;
   x_off = y_off = cur_byte = rsd_count = 0;
   x_right = x_left+bm->w;
   y_bot = y_top +bm->h;

   /* clip bitmap to rectangular clipping window. */
   if (x_left>grd_clip.right || x_right<=grd_clip.left ||
       y_top>grd_clip.bot || y_bot<=grd_clip.top)
      /* completely clipped, forget it. */
      return CLIP_ALL;

   code = CLIP_NONE;
   if (x_left < grd_clip.left) {
      /* clipped off left edge. */
      x_off = grd_clip.left-x_left;
      x = grd_clip.left;
      code |= CLIP_LEFT;
   }
   if (y < grd_clip.top) {
      /* clipped off top edge. */
      y_off = grd_clip.top-y_top;
      y = grd_clip.top;
      code |= CLIP_TOP;
   }
   if (x_right >= grd_clip.right) {
      /* clipped off right edge. */
      x_right = grd_clip.right;
      code |= CLIP_RIGHT;
   }
   if (y_bot > grd_clip.bot) {
      /* clipped off bottom edge. */
      y_bot = grd_clip.bot;
      code |= CLIP_BOT;
   }

   if (code==CLIP_NONE) {
      gr_ubitmap(bm,x_left,y_top);
      return CLIP_NONE;
   }

   if (y_off>0 || x_off>0) {
      /* been clipped of left and/or top, so we need to skip from beginning
         of rsd buffer to be at x_off,y_off within rsd bitmap. */
      start_byte = y_off*bm->row + x_off;
      while (cur_byte < start_byte) {
         if (rsd_count == 0)
            /* no pending opcodes, get a new one. */
            RSD_GET_TOKEN ();
         if (cur_byte+rsd_count <= start_byte) {
            /* current code doesn't hit start_byte yet, so skip all of it. */
            switch (rsd_code) {
            case RSD_RUN:
               /* advance past 1 byte of run color. */
               rsd_src++;
               break;
            case RSD_SKIP:
               break;
            default: /* RSD_DUMP */
               /* advance past rsd_count bytes of dump pixel data. */
               rsd_src += rsd_count;
               break;
            }
            cur_byte += rsd_count;
            rsd_count = 0;
         }
         else {
            /* current code goes past start_byte, so skip only enough to get
               to start_byte. */
            op_count = start_byte-cur_byte;
            switch (rsd_code) {
            case RSD_RUN:
               break;
            case RSD_SKIP:
               break;
            default: /* RSD_DUMP */
               rsd_src += op_count;
               break;
            }
            cur_byte += op_count;
            rsd_count -= op_count;
         }
      }
   }

   p_dst = grd_bm.bits + y*grd_bm.row + x;

   /* process each scanline in two chunks. the first is the clipped section
      from the right edge, wrapping around to to the left. the second is the
      unclipped area in the middle. */
   while (y < y_bot) {
      /* clipped section. */
      while (x < x_left+x_off) {
         if (rsd_count == 0)
            RSD_GET_TOKEN ();
         if (x+rsd_count <= x_left+x_off) {
            switch (rsd_code) {
            case RSD_RUN:
               rsd_src++;
               break;
            case RSD_SKIP:
               break;
            default: /* RSD_DUMP */
               rsd_src += rsd_count;
               break;
            }
            x += rsd_count;
            rsd_count = 0;
         }
         else {
            op_count = x_left+x_off-x;
            switch (rsd_code) {
            case RSD_RUN:
               break;
            case RSD_SKIP:
               break;
            default: /* RSD_DUMP */
               rsd_src += op_count;
               break;
            }
            rsd_count -= op_count;
            x += op_count;
         }
      }

      /* section to draw. */
      while (x < x_right) {
         if (rsd_count == 0)
            RSD_GET_TOKEN ();
         if (x+rsd_count <= x_right) {
            switch (rsd_code) {
            case RSD_RUN:
               LG_memset (p_dst, *rsd_src, rsd_count);
               rsd_src++;
               break;
            case RSD_SKIP:
               break;
            default: /* RSD_DUMP */
               LG_memcpy (p_dst, rsd_src, rsd_count);
               rsd_src += rsd_count;
               break;
            }
            x += rsd_count;
            p_dst += rsd_count;
            rsd_count = 0;
         }
         else {
            op_count = x_right-x;
            switch (rsd_code) {
            case RSD_RUN:
               LG_memset (p_dst, *rsd_src, op_count);
               break;
            case RSD_SKIP:
               break;
            default: /* RSD_DUMP */
               LG_memcpy (p_dst, rsd_src, op_count);
               rsd_src += op_count;
               break;
            }
            x += op_count;
            p_dst += op_count;
            rsd_count -= op_count;
         }
      }

      /* reset x to be beginning of line and set y to next scanline. */
      x -= bm->w;
      p_dst += grd_bm.row-(x_right-x_left)+x_off;
      y++;
   }
rsd_done:
   return code;
}
