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
 * $Source: n:/project/lib/src/2d/RCS/tluctab.c $
 * $Revision: 1.6 $
 * $Author: baf $
 * $Date: 1994/01/17 22:13:15 $
 *
 * This file is, for good or ill, part of the 2d library.
 *
 * $Log: tluctab.c $
 * Revision 1.6  1994/01/17  22:13:15  baf
 * Redid tluc8 spolys (again).
 * 
 * Revision 1.5  1994/01/14  12:41:09  baf
 * Lit translucency reform.
 * 
 * Revision 1.4  1993/12/09  17:39:23  baf
 * Added lighting
 * 
 * Revision 1.3  1993/11/29  20:32:03  baf
 * General revisions and tidying/up of translucency
 * and shading routines
 * 
 * Revision 1.2  1993/11/15  15:01:27  baf
 * Added this RCS header
 * 
 *
 */

/*
This is a routine to generate CLUTs to simulate
looking at colors through a translucent frammice.
To do this, we need to know three things:
The color of the frammice, how opaque it is, and
how effective it is as a color filter (purity).
Opacity and purity should vary from 0 to 1.

Essentially, we multiply the background color by
the frammice color, then take a weighted average
of the result and the background color, then take a
weighted average of the new result and the frammice
color. The two weights are purity and opacity,
respectively. (Apologia: Averaging the two factors in
sequentially like this may seem strange, but I found
it to be more intuitive than any other system I could
think of. Opacity means exactly what it sounds like, and
filter purity only applies to the light coming through.)

Increasing the opacity of the frammice will make it look
cloudier, whereas increasing the purity will make it
look more strongly colored.  Put a green frammice on a
red background. If it has high opacity, it will be
green. If it has low opacity but high purity, it will
be black, as no red light gets through.  If it has low
opacity and low purity, it will be transparent and
therefore look red. Somewhere in the middle of all this,
it will look brown, which is probably what you want.
*/

#include <string.h>
#include "fix.h"
#include "grs.h"
#include "scrdat.h"
#include "rgb.h"
#include "tlucdat.h"
#include "tluctab.h"
#include "lg.h"

// convert red in a glomped rgb to a fixed point 
#define rtof(b) ( ((b)&0x3ff)<<12)
// convert green in a glomped rgb to a fixed point 
#define gtof(b) ( ((b)&0x1ff800)<<1)
// convert blue in a glomped rgb to a fixed point 
#define btof(b) ( ((b)&0xffc00000)>>10)

#define WHITE (0x7fff)

/* Note: RGB values are 6-bit. */

/* Fills table with groovy values, and incedentally returns it. */
uchar *gr_init_translucency_table(uchar *p, fix opacity, fix purity, grs_rgb color)
{
   grs_rgb background;
   int i;
   fix r, g, b;
   fix baser, baseg, baseb; /* surface component */
   fix filterr, filterg, filterb; /* translucent component before addition of background */
   fix filter, clarity;
   long thingge;

   baser = fix_mul(rtof(color), opacity);
   baseg = fix_mul(gtof(color), opacity);
   baseb = fix_mul(btof(color), opacity);
   filter = fix_mul(0x10000 - opacity, purity);
   clarity = 0x10000 - opacity - filter;
   filterr = clarity + (fix_mul(rtof(color), filter) >> 6);
   filterg = clarity + (fix_mul(gtof(color), filter) >> 6);
   filterb = clarity + (fix_mul(btof(color), filter) >> 6);
   for (i=0; i<256; i++) {
      background = grd_bpal[i];
      r = fix_mul(rtof(background), filterr) + baser;
      g = fix_mul(gtof(background), filterg) + baseg;
      b = fix_mul(btof(background), filterb) + baseb;
      thingge = gr_index_rgb(r<<2, g<<2, b<<2);
      p[i] = grd_ipal[thingge];
   }
   return p;
}

uchar *gr_init_lit_translucency_table(uchar *p, fix opacity, fix purity, grs_rgb color, grs_rgb light)
{
   grs_rgb background;
   int i;
   fix r, g, b;
   fix baser, baseg, baseb; /* surface component */
   fix filterr, filterg, filterb; /* translucent component before addition of background */
   fix filter, clarity;
   long thingge;

   filter = fix_mul(0x10000 - opacity, purity);
   clarity = 0x10000 - opacity - filter;
   filterr = clarity + (fix_mul(rtof(color), filter) >> 6);
   filterg = clarity + (fix_mul(gtof(color), filter) >> 6);
   filterb = clarity + (fix_mul(btof(color), filter) >> 6);
   baser = fix_mul(fix_mul(rtof(color), rtof(light)), opacity) >> 6;
   baseg = fix_mul(fix_mul(gtof(color), gtof(light)), opacity) >> 6;
   baseb = fix_mul(fix_mul(btof(color), btof(light)), opacity) >> 6;
   for (i=0; i<256; i++) {
      background = grd_bpal[i];
      r = fix_mul(rtof(background), filterr) + baser;
      g = fix_mul(gtof(background), filterg) + baseg;
      b = fix_mul(btof(background), filterb) + baseb;
      thingge = gr_index_rgb(r<<2, g<<2, b<<2);
      p[i] = grd_ipal[thingge];
   }
   return p;
}

uchar *gr_init_lit_translucency_tables(uchar *p, fix opacity, fix purity, grs_rgb color, int n)
{
   int k;
   grs_rgb light;
   if (n == 0) return NULL;
   for (k=0; k<n; k++) {
      light = grd_bpal[grd_screen->ltab[k*256+grd_ipal[WHITE]]];
      gr_init_lit_translucency_table(p+256*k, opacity, purity, color, light);
   }
   return p;
}

int gr_dump_tluc8_table(uchar *buf, int nlit)
{
   uchar *p;
   int k, lsize = nlit*256;
   p = buf;
   p += sizeof(int);
   *(p++) = nlit;
   *(p++) = tluc8nstab;
   LG_memcpy(p, tluc8stab, tluc8nstab*256);
   for (k=0; k<256; k++) {
      if (tluc8tab[k]) {
         *(p++) = k;
         LG_memcpy(p, tluc8tab[k], 256);
         p += 256;
         LG_memcpy(p, tluc8ltab[k], lsize);
         p += lsize;
      }
   }
   *(int *)buf = p-buf;
   return p-buf;
}

void gr_read_tluc8_table(uchar *buf)
{
   uchar *p, *end;
   int lsize, k;
   end = buf + *(int *)buf;
   p = buf + sizeof(int);
   lsize = *(p++) * 256;
   tluc8nstab = *(p++);
   tluc8stab=p;
   p += 256*tluc8nstab;
   while (p < end) {
      k = *(p++);
      tluc8tab[k] = p;
      p += 256;
      if (lsize) tluc8ltab[k] = p;
      p += lsize;
   }
}

