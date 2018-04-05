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
 * $Source: n:/project/lib/src/2d/RCS/genvrect.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/10/19 09:57:56 $
 *
 * This file is part of the 2d library.
 *
 * $Log: genvrect.c $
 * Revision 1.3  1993/10/19  09:57:56  kaboom
 * Replaced #include   new headers.
 * 
 * Revision 1.2  1993/10/08  01:15:54  kaboom
 * Changed quotes in #include lines to angle brackets for Watcom.
 * 
 * Revision 1.1  1993/08/19  21:53:06  jaemz
 * Initial revision
 */

#include "ctxmac.h"
#include "grrect.h"
#include "general.h"

/* x and y are the screen x and y's in fixed point of the vertices of this voxel piece
   dz[0] is the difference in x for a delta one in z
   dz[1] is the difference in y for a delta one in z
   near_ver is which vertex is closest to the viewer, so it knows which way to traverse
   col is the color map
   ht is the height map, 0 means at the surface of the bounding box, positive goes in
   dotw and doth are the height and width of the rectangles to use at each point */

void gen_vox_rect(fix x[4],fix y[4],fix dz[3],int near_ver,grs_bitmap *col,grs_bitmap *ht,int dotw,int doth)
{
   int du,dv,initu,endu,initv,endv;
   int i,j;
   int c;
   long z;
   int far_ver;

   /* Test of broadcasting system */
   fix fdxdu,fdydu,fdxdv,fdydv;
   fix fcurx,fcury;
   long fxp,fyp;
   long fdxdz,fdydz;
   long oldcurx;
   long oldcury;
   
   far_ver = (near_ver+2)%4;

   /* How to scan given near_ver */
   if (near_ver == 0 || near_ver == 3) { initu = col->w -1; du = -1; endu = -1; }
   else { initu = 0; du = 1; endu = col->w ; }
   if (near_ver < 2) { initv = col->h-1; dv = -1; endv = -1; }
   else { initv = 0; dv = 1; endv = col->h; }

   fdxdu = (x[1]-x[0])/(col->w-1);
   fdydu = (y[1]-y[0])/(col->w-1);
   fdxdv = (x[2]-x[1])/(col->h-1);
   fdydv = (y[2]-y[1])/(col->h-1);

   fdxdz = dz[0];
   fdydz = dz[1];

   oldcurx = fcurx = x[0] + fdxdu*initu + fdxdv*initv;
   oldcury = fcury = y[0] + fdydu*initu + fdydv*initv;

   fdxdu *= du;
   fdydu *= du;
   fdxdv *= dv;
   fdydv *= dv;

   for(j=initv;j!=endv;j+=dv) {
      for(i=initu;i!=endu;i+=du) {
         c = *(col->bits + i + j*col->row);
         //c = get_pal(col,i,j);
         if (c != 0) {
            z = *(ht->bits + i+ j*ht->row);
            //z = get_pal(ht,i,j);
            fxp = (fcurx + fdxdz*z)>>16;
            fyp = (fcury + fdydz*z)>>16;

            gr_set_fcolor(c);
            gr_rect(fxp,fyp,fxp+dotw,fyp+doth);
         }
         fcurx += fdxdu;
         fcury += fdydu;
      }
      fcurx = (oldcurx += fdxdv);
      fcury = (oldcury += fdydv);
   }
}





