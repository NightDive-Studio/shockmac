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
 * $Source: n:/project/lib/src/2d/RCS/rsdcvt.c $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/05/19 15:10:27 $
 * 
 * Routine for unpacking an rsd bitmap to a flat8 bitmap.
 * Uses memory provided externally.
 *
 * This file is part of the 2d library.
 *
 * $Log: rsdcvt.c $
 * Revision 1.2  1994/05/19  15:10:27  kevin
 * Check if BMF_TLUC8 flag is set and if so, set
 * uncompressed bitmap type to BMT_TLUC8.
 * 
 * Revision 1.1  1993/12/28  16:32:09  kevin
 * Initial revision
 * 
 */

#include <string.h>
#include "grs.h"
#include "bitmap.h"
#include "rsd.h"
#define _RSDCVT_C
#include "rsdunpck.h"
#include "lg.h"

uchar *grd_unpack_buf=NULL;

/*************************************************/
/* Puts 0's in place of skips and pads with 0's. */
/* i.e., grd_unpack_buf is entirely overwritten. */
/*************************************************/
int gr_rsd8_convert(grs_bitmap *sbm, grs_bitmap *dbm)
{
   short x_right,y_bot;                /* opposite edges of bitmap */
   short x,y;                          /* current position */
   int over_run;                       /* unwritten right edge */
   uchar *p_dst;
   uchar *rsd_src;                     /* rsd source buffer */
   short rsd_code;                     /* last rsd opcode */
   short rsd_count;                    /* count for last opcode */
   short op_count;                     /* operational count */

   if (grd_unpack_buf==NULL) return GR_UNPACK_RSD8_NOBUF;
   if (sbm->type != BMT_RSD8) return GR_UNPACK_RSD8_NOTRSD;
   *dbm = *sbm;	// LG_memcpy (dbm, sbm, sizeof (*sbm));
   if (sbm->flags&BMF_TLUC8)
      dbm->type = BMT_TLUC8;
   else
      dbm->type = BMT_FLAT8;
   dbm->bits = grd_unpack_buf;
   if (dbm->w==dbm->row) p_dst=gr_rsd8_unpack(sbm->bits,dbm->bits);
   else {
      rsd_src = sbm->bits;
      x = y = rsd_count = 0;
      x_right = sbm->w;
      y_bot = sbm->h;
      p_dst = dbm->bits;
      over_run=dbm->row-sbm->w;
     
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
                  LG_memset (p_dst, *rsd_src, rsd_count);
                  rsd_src++;
                  break;
               case RSD_SKIP:
                  LG_memset (p_dst, kSkipColor, rsd_count);
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
               /* code goes over to next scanline, do the amount that will fit
                  on this scanline, put off rest till next line. */
               op_count = x_right-x;
               switch (rsd_code) {
               case RSD_RUN:
                  LG_memset (p_dst, *rsd_src, op_count);
                  break;
               case RSD_SKIP:
                  LG_memset (p_dst, kSkipColor, op_count);
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
         x -= sbm->w;
         if (over_run) {
            LG_memset (p_dst, kSkipColor, over_run);
            p_dst += over_run;
         }
         y++;
      }
   }
rsd_done:
   if (over_run=(dbm->bits+dbm->row*dbm->h)-p_dst)
      LG_memset (p_dst, kSkipColor, over_run);
   return GR_UNPACK_RSD8_OK;
}
   
