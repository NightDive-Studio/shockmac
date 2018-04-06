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
 * $Source: n:/project/lib/src/2d/RCS/genf24.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/02 01:17:21 $
 *
 * Generic routines to draw a flat 24 bitmap.
 *
 * $Log: genf24.c $
 * Revision 1.3  1993/10/02  01:17:21  kaboom
 * Changed include of clip.h to include of clpcon.h and/or clpfcn.h.
 * 
 * Revision 1.2  1993/09/02  20:34:38  kaboom
 * Now gen_flat24_bitmap() returns clip code.
 * 
 * Revision 1.1  1993/09/02  20:07:31  kaboom
 * Initial revision
 */

#include "grs.h"
#include "grd.h"
#include "bitmap.h"
#include "clpcon.h"
#include "clpfcn.h"
#include "grdbm.h"
#include "grp24.h"
#include "general.h"

#if 0
// MLA - this doesn't appear to be used anywhere
void memmove (uchar *dst, uchar *src, int n);
#pragma aux memmove =      \
   "mov  eax,ecx" \
   "shr  ecx,2"   \
   "rep  movsd"   \
   "mov  ecx,eax" \
   "and  ecx,3"   \
   "rep  movsb"   \
   parm [edi] [esi] [ecx]  \
   modify [eax ecx edi esi];
#endif

void gen_flat24_ubitmap (grs_bitmap *bm, short x0, short y0)
{
   short x, y;
   uchar *p, *lp;

   p = bm->bits;
   if (bm->flags & BMF_TRANS)
      for (y=y0; y<y0+bm->h; y++) {
         lp = p;
         for (x=x0; x<x0+bm->w; x++, p+=3)
            if (*((long *)p) & 0x00ffffff)
               gr_set_upixel24 (*((long *)p)&0x00ffffff, x, y);
         p = lp+bm->row;
      }
   else
      for (y=y0; y<y0+bm->h; y++) {
         lp = p;
         for (x=x0; x<x0+bm->w; x++, p+=3)
            gr_set_upixel24 (*((long *)p)&0x00ffffff, x, y);
         p = lp+bm->row;
      }
}

int gen_flat24_bitmap (grs_bitmap *bm, short x0, short y0)
{
   int r;
   short w,h;
   uchar *b;

   b = bm->bits; w = bm->w; h = bm->h;
   r = gr_clip_flat24_bitmap (bm, &x0, &y0);
   if (r != CLIP_ALL)
      gr_flat24_ubitmap (bm, x0, y0);
   bm->bits = b; bm->w = w; bm->h = h;
   return r;
}
