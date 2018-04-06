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
 * $Source: r:/prj/lib/src/2d/RCS/fl8bldbl.c $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/12/01 14:59:38 $
 *
 * $Log: fl8bldbl.c $
 * Revision 1.3  1994/12/01  14:59:38  kevin
 * Added sub/bitmap blending routines.
 * 
 * Revision 1.2  1994/09/08  00:01:07  kevin
 * removed smooth_hv_doubler (replaced by asm version).
 * 
 * Revision 1.1  1994/03/14  17:51:09  kevin
 * Initial revision
 * 
 */

#include "lg.h"
#include "grs.h"
#include "blncon.h"
#include "blndat.h"
#include "blnfcn.h"
#include "cnvdat.h"
#include <string.h>
#include "flat8.h"

void flat8_flat8_v_double_ubitmap(grs_bitmap *bm)
{
   int   i,j,bpv, row, b_row;                      /* loop controls, bottom pixel value */
   uchar *src=bm->bits, *dst=grd_bm.bits, *src_nxt, *dst_nxt;
   int   dst_skip=grd_bm.row-bm->w,src_skip=bm->row-bm->w;
   uchar *local_grd_half_blend;
   
   local_grd_half_blend = grd_half_blend;
   row = grd_bm.row;
   b_row = bm->row;
   
   LG_memcpy(dst,src,bm->w);              /* first copy the top line */
   for (i=0; i<bm->h-1; i++)           /* for each row source, 2 destination */
   {
      src_nxt=src+b_row;             	/* next line of source */
      dst+=row;                 		/* interpolated row */
      dst_nxt=dst+row;          		/* next clone row */
      for (j=0; j<bm->w; j++)          /* all pixels in vertical clone */
      {
         *dst++=local_grd_half_blend[((bpv=*src_nxt++)<<8)|(*src++)];
         *dst_nxt++=bpv;               /* is this faster than another memcpy? in asm probably? */
      }
      dst+=dst_skip; src+=src_skip;    /* get to the next line */
   }
#ifdef FULL_FILL
   memset(dst+row,0,bm->w);
#endif
}

#define QSB_SIZE 4
#define LOG_QSB_SIZE 2
#define D_ROW 8*QSB_SIZE

uchar *grd_last_sub_bm;
uchar grd_sub_bm_buffer[D_ROW*D_ROW];

void gri_flat8_hv_quadruple_sub_bitmap(grs_bitmap *src_bm, grs_bitmap *dst_bm, int u, int v);
void gri_flat8_hv_quadruple_sub_bitmap(grs_bitmap *src_bm, grs_bitmap *dst_bm, int u, int v)
{
   int i,j,full_h_blend;
   uchar *src,*dst;

   if (grd_log_blend_levels!=2) {
      gr_free_blend();
      gr_init_blend(2);
   }

   /* initialize destination bitmap parameters. */
   dst_bm->bits=grd_sub_bm_buffer;
   dst_bm->h=D_ROW;
   dst_bm->row=dst_bm->w=D_ROW;
   dst_bm->hlog=dst_bm->wlog=LOG_QSB_SIZE+3;

   /* get pointer to sub bitmap bits */
   src=src_bm->bits+u+src_bm->row*v;

   /* If we just did this bitmap, no need to do it again! */
   if (src==grd_last_sub_bm) return;
   grd_last_sub_bm=src;

   /* Fill in the middle of the destination bitmap */
   dst=dst_bm->bits+(D_ROW+1)*(D_ROW/4);
   
   if (v+QSB_SIZE<src_bm->h)
      full_h_blend=1;
   else
      full_h_blend=0;

   /* First horizontally blend the source bitmap into every fourth destination bitmap row. */
   for (i=0; i<QSB_SIZE+full_h_blend; i++)
   {
      for (j=0; j<QSB_SIZE; j++)
      {
         if (u+j+1>=src_bm->w) {
            /* if we're at the right edge of the source bitmap, just copy the right pixel.*/
            dst[0]=dst[1]=dst[2]=dst[3]=src[j];
            dst+=4;
         } else {
            int k=(src[j+1])|(src[j]<<8);
            dst[0]=src[j];
            dst[1]=grd_blend[k];
            dst[2]=grd_blend[k+GR_BLEND_TABLE_SIZE];
            dst[3]=grd_blend[k+2*GR_BLEND_TABLE_SIZE];
            dst+=4;
         }
      }
      dst+=4*(D_ROW-QSB_SIZE);
      src+=src_bm->row;
   }

   /* Now verticaly blend the destination colums. */
   dst=dst_bm->bits+(D_ROW+1)*(D_ROW/4);
   for (i=0; i<QSB_SIZE+full_h_blend-1; i++)
   {
      for (j=0; j<4*QSB_SIZE; j++)
      {
         int k=(dst[j+4*D_ROW])|(dst[j]<<8);
         dst[j+D_ROW]=grd_blend[k];
         dst[j+2*D_ROW]=grd_blend[k+GR_BLEND_TABLE_SIZE];
         dst[j+3*D_ROW]=grd_blend[k+2*GR_BLEND_TABLE_SIZE];
      }
      dst+=4*D_ROW;
   }

   /* if we're at the bottom edge of the source bitmap, just copy the bottom row 3 times. */
   if (full_h_blend==0) {
      LG_memcpy(dst+D_ROW,dst,4*QSB_SIZE);
      LG_memcpy(dst+2*D_ROW,dst,4*QSB_SIZE);
      LG_memcpy(dst+3*D_ROW,dst,4*QSB_SIZE);
   }

   /* copy the top row to fill out the top of the dest. */
   dst=dst_bm->bits+(D_ROW+1)*(D_ROW/4);
   for (i=0;i<D_ROW/4;i++) {
      LG_memcpy(dst-D_ROW,dst,4*QSB_SIZE);
      dst-=D_ROW;
   }
   /* copy the bottom row to fill out the bottom of the dest. */
   dst=dst_bm->bits+(3*D_ROW+1)*(D_ROW/4);
   for (i=0;i<D_ROW/4;i++) {
      LG_memcpy(dst,dst-D_ROW,4*QSB_SIZE);
      dst+=D_ROW;
   }
   /* copy the right and left colums to fill out the right and left edges. */
   dst=dst_bm->bits;
   for (i=0;i<D_ROW;i++) {
      memset(dst,dst[D_ROW/4],D_ROW/4);
      memset(dst+3*D_ROW/4,dst[(3*D_ROW/4)-1],D_ROW/4);
      dst+=D_ROW;
   }
}

#define DSB_SIZE 8
#define LOG_DSB_SIZE 3

void gri_flat8_hv_double_sub_bitmap(grs_bitmap *src_bm, grs_bitmap *dst_bm, int u, int v);
void gri_flat8_hv_double_sub_bitmap(grs_bitmap *src_bm, grs_bitmap *dst_bm, int u, int v)
{
   int i,j,full_h_blend;
   uchar *src,*dst;

   if (grd_log_blend_levels!=2) {
      gr_free_blend();
      gr_init_blend(2);
   }

   /* initialize destination bitmap parameters. */
   dst_bm->bits=grd_sub_bm_buffer;
   dst_bm->h=D_ROW;
   dst_bm->row=dst_bm->w=D_ROW;
   dst_bm->hlog=dst_bm->wlog=LOG_DSB_SIZE+2;

   /* get pointer to sub bitmap bits */
   src=src_bm->bits+u+src_bm->row*v;

   /* If we just did this bitmap, no need to do it again! */
   if (src==grd_last_sub_bm) return;
   grd_last_sub_bm=src;

   /* Fill in the middle of the destination bitmap */
   dst=dst_bm->bits+(D_ROW+1)*(D_ROW/4);
   
   if (v+DSB_SIZE<src_bm->h)
      full_h_blend=1;
   else
      full_h_blend=0;

   /* First horizontally blend the source bitmap into every other destination bitmap row. */
   for (i=0; i<DSB_SIZE+full_h_blend; i++)
   {
      for (j=0; j<DSB_SIZE; j++)
      {
         if (u+j+1>=src_bm->w) {
            /* if we're at the right edge of the source bitmap, just copy the right pixel.*/
            dst[0]=dst[1]=src[j];
            dst+=2;
         } else {
            int k=(src[j+1])|(src[j]<<8);
            dst[0]=src[j];
            dst[1]=grd_half_blend[k];
            dst+=2;
         }
      }
      dst+=2*(D_ROW-DSB_SIZE);
      src+=src_bm->row;
   }

   /* Now verticaly blend the destination colums. */
   dst=dst_bm->bits+(D_ROW+1)*(D_ROW/4);
   for (i=0; i<DSB_SIZE+full_h_blend-1; i++)
   {
      for (j=0; j<2*DSB_SIZE; j++)
      {
         int k=(dst[j+2*D_ROW])|(dst[j]<<8);
         dst[j+D_ROW]=grd_half_blend[k];
      }
      dst+=2*D_ROW;
   }

   /* if we're at the bottom edge of the source bitmap, just copy the bottom row 3 times. */
   if (full_h_blend==0)
      LG_memcpy(dst+D_ROW,dst,4*QSB_SIZE);

// all this is unnecessary if we're just doing linear maps.
//   /* copy the top row to fill out the top of the dest. */
//   dst=dst_bm->bits+(D_ROW+1)*(D_ROW/4);
//   for (i=0;i<D_ROW/4;i++) {
//      memcpy(dst-D_ROW,dst,2*DSB_SIZE);
//      dst-=D_ROW;
//   }
//   /* copy the bottom row to fill out the bottom of the dest. */
//   dst=dst_bm->bits+(3*D_ROW+1)*(D_ROW/4);
//   for (i=0;i<D_ROW/4;i++) {
//      memcpy(dst,dst-D_ROW,2*DSB_SIZE);
//      dst+=D_ROW;
//   }
//   /* copy the right and left colums to fill out the right and left edges. */
//   dst=dst_bm->bits;
//   for (i=0;i<D_ROW;i++) {
//      memset(dst,dst[D_ROW/4],D_ROW/4);
//      memset(dst+3*D_ROW/4,dst[(3*D_ROW/4)-1],D_ROW/4);
//      dst+=D_ROW;
//   }
}
