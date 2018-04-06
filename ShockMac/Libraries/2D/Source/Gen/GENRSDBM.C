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
 * $Source: n:/project/lib/src/2d/RCS/genrsdbm.c $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/03/15 13:17:09 $
 * 
 * Generic routines for unpacking and then drawing/scaling rsd bitmaps.
 *
 * This file is part of the 2d library.
 *
 * $Log: genrsdbm.c $
 * Revision 1.3  1994/03/15  13:17:09  kevin
 * Added clut_bitmap routines.
 * 
 * Revision 1.2  1994/01/19  18:41:55  kaboom
 * Fixed typo in clipped clut scale.
 * 
 * Revision 1.1  1993/12/28  19:34:41  kevin
 * Initial revision
 */

#include "grs.h"
#include "grrend.h"
#include "clpcon.h"
#include "grdbm.h"
#include "grcbm.h"
#include "general.h"
#include "rsdunpck.h"

void unpack_rsd8_ubitmap(grs_bitmap *bm, short x, short y)
{
   if (grd_unpack_buf!=NULL) {
      grs_bitmap tbm;
      if (gr_rsd8_convert(bm,&tbm)==GR_UNPACK_RSD8_OK)
         gr_ubitmap(&tbm,x,y);
   }
}

int unpack_rsd8_bitmap(grs_bitmap *bm, short x, short y)
{
   if (grd_unpack_buf!=NULL) {
      grs_bitmap tbm;
      if (gr_rsd8_convert(bm,&tbm)==GR_UNPACK_RSD8_OK)
         return gr_bitmap(&tbm,x,y);
   }
   return CLIP_ALL;
}

void unpack_rsd8_scale_ubitmap(grs_bitmap *bm, short x, short y, short w, short h)
{
   if (grd_unpack_buf!=NULL) {
      grs_bitmap tbm;
      if (gr_rsd8_convert(bm,&tbm)==GR_UNPACK_RSD8_OK)
         gr_scale_ubitmap(&tbm,x,y,w,h);
   }
}

int unpack_rsd8_scale_bitmap(grs_bitmap *bm, short x, short y, short w, short h)
{
   if (grd_unpack_buf!=NULL) {
      grs_bitmap tbm;
      if (gr_rsd8_convert(bm,&tbm)==GR_UNPACK_RSD8_OK)
         return gr_scale_bitmap(&tbm,x,y,w,h);
   }
   return CLIP_ALL;
}

void unpack_rsd8_clut_scale_ubitmap(grs_bitmap *bm, short x, short y, short w, short h, uchar *cl) 
{
   if (grd_unpack_buf!=NULL) {
      grs_bitmap tbm;
      if (gr_rsd8_convert(bm,&tbm)==GR_UNPACK_RSD8_OK)
         gr_clut_scale_ubitmap(&tbm,x,y,w,h,cl);
   }
}

int unpack_rsd8_clut_scale_bitmap(grs_bitmap *bm, short x, short y, short w, short h, uchar *cl) 
{
   if (grd_unpack_buf!=NULL) {
      grs_bitmap tbm;
      if (gr_rsd8_convert(bm,&tbm)==GR_UNPACK_RSD8_OK)
         return gr_clut_scale_bitmap(&tbm,x,y,w,h,cl);
   }
   return CLIP_ALL;
}

void unpack_rsd8_clut_ubitmap(grs_bitmap *bm, short x, short y, uchar *cl) 
{
   if (grd_unpack_buf!=NULL) {
      grs_bitmap tbm;
      if (gr_rsd8_convert(bm,&tbm)==GR_UNPACK_RSD8_OK)
         gr_clut_ubitmap(&tbm,x,y,cl);
   }
}

int unpack_rsd8_clut_bitmap(grs_bitmap *bm, short x, short y, uchar *cl) 
{
   if (grd_unpack_buf!=NULL) {
      grs_bitmap tbm;
      if (gr_rsd8_convert(bm,&tbm)==GR_UNPACK_RSD8_OK)
         return gr_clut_bitmap(&tbm,x,y,cl);
   }
   return CLIP_ALL;
}
