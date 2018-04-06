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
 * $Source: r:/prj/lib/src/2d/RCS/genrsd8.c $
 * $Revision: 1.7 $
 * $Author: kevin $
 * $Date: 1994/10/25 15:14:10 $
 *
 * Generic rsd 8 bitmap routines.
 *
 * $Log: genrsd8.c $
 * Revision 1.7  1994/10/25  15:14:10  kevin
 * Renamed funcs.
 * 
 * Revision 1.6  1993/10/19  09:51:26  kaboom
 * Replaced #include "grd.h" with new headers split from grd.h.
 * 
 * Revision 1.5  1993/10/02  01:17:31  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.4  1993/09/06  19:43:34  kaboom
 * Fixed bug in completely clipped case---now checks for >= on right and bottom.
 * 
 * Revision 1.3  1993/07/13  17:48:33  kaboom
 * Fixed bugs in clipping case off right edge.  Updated for padded clipping
 * rectangle.
 * 
 * Revision 1.2  1993/04/29  18:40:46  kaboom
 * Changed include of gr.h to smaller more specific grxxx.h.
 * 
 * Revision 1.1  1993/02/16  16:00:01  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log from old general.c:
 *
 * Revision 1.9  1992/12/14  18:13:37  kaboom
 * Fixed bug in gen_rsd8_[u]bitmap --
 * was omitting bottom line.  
 *
 * Revision 1.7  1992/12/11  13:58:09  kaboom
 * Fixed bug in clipping off right and bottom in gen_rsd8_bitmap.
 */

#include "bitmap.h"
#include "clpcon.h"
#include "cnvdat.h"
#include "ctxmac.h"
#include "grpix.h"
#include "grrect.h"
#include "rsd.h"
#include "general.h"

/* draw an unclipped rsd bitmap. since rsd bitmaps have skips, the trans
   field of the flags byte is ignored. */
void gri_gen_rsd8_ubitmap (grs_bitmap *bm, short x, short y)
{
   short x_right,y_bot;       /* opposite edges of bitmap */
   uchar *rsd_src;            /* rsd source buffer */
   short rsd_code;            /* last rsd opcode */
   short rsd_count;           /* count for last opcode */
   short op_count;            /* operational count */
   int i;

   rsd_src = bm->bits;
   rsd_count = 0;
   x_right = x+bm->w;
   y_bot = y+bm->h;

   /* process each scanline, keeping track of x and splitting opcodes that
      span across more than one line. */
   while (y < y_bot) {
      /* do enough opcodes to get to the end of the current scanline. */
      while (x < x_right) {
         if (rsd_count == 0)
            RSD_GET_TOKEN ();
         if (x+rsd_count <= x_right) {
            /* current code is all on this scanline. */
            switch (rsd_code) {
            case RSD_RUN:
               gr_set_fcolor (*rsd_src);
               gr_uhline (x, y, x+rsd_count-1);
               rsd_src++;
               break;
            case RSD_SKIP:
               break;
            default: /* RSD_DUMP */
               for (i=0; i<rsd_count; i++)
                  gr_fill_pixel (rsd_src[i],x+i,y);
               rsd_src += rsd_count;
               break;
            }
            x += rsd_count;
            rsd_count = 0;
         }
         else {
            /* code goes over to next scanline, do the amount that will fit
               on this scanline, put off rest till next line. */
            op_count = x_right-x;
            switch (rsd_code) {
            case RSD_RUN:
               gr_set_fcolor (*rsd_src);
               gr_uhline (x, y, x+op_count-1);
               break;
            case RSD_SKIP:
               break;
            default: /* RSD_DUMP */
               for (i=0; i<op_count; i++)
                  gr_fill_pixel (rsd_src[i],x+i,y);
               rsd_src += op_count;
               break;
            }
            x += op_count;
            rsd_count -= op_count;
         }
      }

      /* reset x to be beginning of line and set y to next scanline. */
      x -= bm->w;
      y++;
   }
rsd_done:
   return;
}

int gri_gen_rsd8_bitmap (grs_bitmap *bm, short x_left, short y_top)
{
   short x,y;                 /* current destination position */
   short x_right,y_bot;       /* opposite edges of bitmap */
   short x_off,y_off;         /* x,y offset for clip */
   ulong start_byte;          /* byte to start drawing */
   ulong cur_byte;            /* current position within rsd */
   uchar *rsd_src;            /* rsd source buffer */
   short rsd_code;            /* last rsd opcode */
   short rsd_count;           /* count for last opcode */
   short op_count;            /* operational count */
   int code;                  /* clip code to return */
   int i;

   rsd_src = bm->bits;
   x = x_left; y = y_top;
   x_off = y_off = cur_byte = rsd_count = 0;
   x_right = x_left+bm->w;
   y_bot = y_top +bm->h;

   /* clip bitmap to rectangular clipping window. */
   if (x_left>grd_canvas->gc.clip.i.right ||
		 x_right<=grd_canvas->gc.clip.i.left ||
       y_top>grd_canvas->gc.clip.i.bot ||
		 y_bot<=grd_canvas->gc.clip.i.top)
      /* completely clipped, forget it. */
      return CLIP_ALL;

   code = CLIP_NONE;
   if (x_left < grd_canvas->gc.clip.i.left) {
      /* clipped off left edge. */
      x_off = grd_clip.left-x_left;
      x = grd_clip.left;
      code |= CLIP_LEFT;
   }
   if (y < grd_clip.top) {
      /* clipped off top edge. */
      y_off = grd_canvas->gc.clip.i.top-y_top;
      y = grd_clip.top;
      code |= CLIP_TOP;
   }
   if (x_right > grd_canvas->gc.clip.i.right) {
      /* clipped off right edge. */
      x_right = grd_clip.right;
      code |= CLIP_RIGHT;
   }
   if (y_bot > grd_canvas->gc.clip.i.bot) {
      /* clipped off bottom edge. */
      y_bot = grd_clip.bot;
      code |= CLIP_BOT;
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
               gr_set_fcolor (*rsd_src);
               gr_uhline (x, y, x+rsd_count-1);
               rsd_src++;
               break;
            case RSD_SKIP:
               break;
            default: /* RSD_DUMP */
               for (i=0; i<rsd_count; i++)
                  gr_fill_pixel (rsd_src[i],x+i,y);
               rsd_src += rsd_count;
               break;
            }
            x += rsd_count;
            rsd_count = 0;
         }
         else {
            op_count = x_right-x;
            switch (rsd_code) {
            case RSD_RUN:
               gr_set_fcolor (*rsd_src);
               gr_uhline (x, y, x+op_count-1);
               break;
            case RSD_SKIP:
               break;
            default: /* RSD_DUMP */
               for (i=0; i<op_count; i++)
                  gr_fill_pixel (rsd_src[i],x+i,y);
               rsd_src += op_count;
               break;
            }
            x += op_count;
            rsd_count -= op_count;
         }
      }

      /* reset x to be beginning of line and set y to next scanline. */
      x -= bm->w;
      y++;
   }
rsd_done:
   return code;
}
