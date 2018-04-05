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
 * $Source: r:/prj/lib/src/2d/RCS/fl8fl8m.c $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/10/04 18:46:16 $
 * 
 * Routines for masking flat 8 bitmaps into a flat 8 canvas
 * through a stencil.
 *
 * This file is part of the 2d library.
 *
 * $Log: fl8fl8m.c $
 * Revision 1.3  1994/10/04  18:46:16  kevin
 * added clut fill mode specific function.  Renamed old proc.
 * 
 * Revision 1.2  1994/08/16  18:29:35  kevin
 * fixed several bugs.
 * 
 * Revision 1.1  1994/08/16  13:14:22  kevin
 * Initial revision
 * 
 */

#include "bitmap.h"
#include "clip.h"
#include "cnvdat.h"
#include "grs.h"
#include <string.h>
#include "fl8tf.h"
#include "lg.h"

extern int gen_flat8_bitmap(grs_bitmap *bm, short x, short y);
int gri_flat8_mask_bitmap(grs_bitmap *bm, short x, short y, grs_stencil *sten)
{
   if (sten==NULL) return gen_flat8_bitmap(bm, x, y);
   else {
      int yf=y+bm->h;
      uchar *dst;
      uchar *src=bm->bits-x;
      if (yf>grd_clip.bot) yf=grd_clip.bot;
      if (y<grd_clip.top) {
         src+=(grd_clip.top-y)*bm->row;
         y=grd_clip.top;
      }
      dst=grd_bm.bits+y*grd_bm.row;
      for (;y<yf;y++) {
         int xi=x,xf=x+bm->w;
         grs_sten_elem *s;
#ifdef NEW_STENCILS
         s=&(sten[y]);
#else
         s=&((sten->elem)[y]);
#endif
         for (;s->r<=xi;) {
            s=s->n;
            if (s==NULL) break;
         }
         if (s!=NULL) {
            if (s->l>xi) xi=s->l;
            if (s->r<xf) xf=s->r;
            if (xf>xi) {
               if (bm->flags & BMF_TRANS) {
                  int i;
                  for (i=xi;i<xf;i++)
                     if (src[i]) dst[i]=src[i];
               } else {
                  LG_memmove(dst+xi,src+xi,xf-xi);
               }
            }
         }
         src+=bm->row;
         dst+=grd_bm.row;
      }
   }
   return CLIP_NONE;  /* actually, who knows? */
}

extern int gen_flat8_clut_bitmap(grs_bitmap *bm, short x, short y, uchar *clut);
int gri_flat8_mask_fill_clut_bitmap(grs_bitmap *bm, short x, short y, grs_stencil *sten)
{
   uchar *clut=(uchar *)(grd_gc.fill_parm);
   if (sten==NULL) return gen_flat8_clut_bitmap(bm, x, y, clut);
   else {
      int yf=y+bm->h;
      uchar *dst;
      uchar *src=bm->bits-x;
      if (yf>grd_clip.bot) yf=grd_clip.bot;
      if (y<grd_clip.top) {
         src+=(grd_clip.top-y)*bm->row;
         y=grd_clip.top;
      }
      dst=grd_bm.bits+y*grd_bm.row;
      for (;y<yf;y++) {
         int xi=x,xf=x+bm->w;
         grs_sten_elem *s;
#ifdef NEW_STENCILS
         s=&(sten[y]);
#else
         s=&((sten->elem)[y]);
#endif
         for (;s->r<=xi;) {
            s=s->n;
            if (s==NULL) break;
         }
         if (s!=NULL) {
            if (s->l>xi) xi=s->l;
            if (s->r<xf) xf=s->r;
            if (xf>xi) {
               int i;
               if (bm->flags & BMF_TRANS) {
                  for (i=xi;i<xf;i++)
                     if (src[i]) dst[i]=clut[src[i]];
               } else {
                  for (i=xi;i<xf;i++)
                     dst[i]=clut[src[i]];
               }
            }
         }
         src+=bm->row;
         dst+=grd_bm.row;
      }
   }
   return CLIP_NONE;  /* actually, who knows? */
}
