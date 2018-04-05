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
 * $Source: r:/prj/lib/src/vox/RCS/vox2d.c $
 * $Revision: 1.6 $
 * $Author: jaemz $
 * $Date: 1994/08/16 05:45:28 $
 *
 * 2d Interface to the voxel library
 * This file is part of the vox library
 *
 * $Log: vox2d.c $
 * Revision 1.6  1994/08/16  05:45:28  jaemz
 * Found fix overflow problem and fixed it
 * 
 * Revision 1.5  1994/08/11  22:52:55  jaemz
 * Made fill mode work and put pix_size in real 3d points.
 * 
 * Revision 1.4  1994/07/15  21:13:12  jaemz
 * Added self clipping
 * 
 * Revision 1.3  1994/05/25  23:14:53  jaemz
 * Added more debug checking for out of bounds squares
 * 
 * Revision 1.2  1994/04/21  12:00:24  jaemz
 * Added bounds checking to the debug version
 * 
 * Revision 1.1  1994/04/21  10:52:33  jaemz
 * Initial revision
 * 
 */

#include <stdlib.h>
#include "2d.h"
//#include <mprintf.h>
#include "vox.h"

// static area for multiplication tables

extern fix *zdxdz;
extern fix *zdydz;

// maximum depth allocated for mult tables
// used for bounds checking in debug version
#ifdef DBG_ON
extern int vxd_maxd;
#endif

void vmap_dot(fix x0, fix y0, fix dxdu, fix dydu, fix dxdv, fix dydv, fix dxdz, fix dydz, 
			  int near_ver,vxs_vox *vx,int dotw,int doth,bool clip);

// We calculate fdxdu (where uv goes across bitmap)
// and xy goes across screen
// x,y seem to be nearest vertex coordinates
// dz is the amount
void vmap_dot(fix x0, fix y0, fix dxdu, fix dydu, fix dxdv, fix dydv, fix dxdz, fix dydz, 
			  int near_ver,vxs_vox *vx,int dotw,int doth,bool clip)
{
   int du,dv,initu,endu,initv,endv;
   int i,j;
   int c;
   int crow,hrow;
   int dcrow,dhrow;
   ubyte *crow2,*hrow2;
   long z;
   int far_ver;
   grs_bitmap *col;
   grs_bitmap *ht;
   int (* rect) (short x1,short y1,short x2,short y2);
   fix xp,yp;
   fix oldcurx;
   fix oldcury;
   fix curx,cury;
   #ifdef DBG_ON
   short xl,yt,xr,yb;
   #endif

   col = vx->col;
   ht = vx->ht;

   if (clip)
      rect = gr_rect;
   else
      rect = (int (*)(short, short, short, short)) gr_urect;
   
   far_ver = (near_ver+2)%4;

   //How to scan given near_ver, and this should be actually computed some day
   if (near_ver == 0 || near_ver == 3) {
      initu = col->w -1;
      du = -1;
      endu = -1;
   }
   else {
      initu = 0;
      du = 1;
      endu = col->w;
   } 
   if (near_ver < 2) {
      initv = col->h-1;
      dv = -1;
      endv = -1;
   }
   else {
      initv = 0;
      dv = 1;
      endv = col->h;
   }

   oldcurx = curx = x0 + dxdu*initu + dxdv*initv;
   oldcury = cury = y0 + dydu*initu + dydv*initv;

   dxdu *= du;
   dydu *= du;
   dxdv *= dv;
   dydv *= dv;

   // fill tables with values so we don't have to multiply in the inner loop
   xp = 0;
   yp = 0;

   #ifdef DBG_ON
   if (vx->d > vxd_maxd) {
      mprintf("voxel object depth z=%d\n",vx->d);
      mprintf("greater than max = %d\n",vxd_maxd);
      mprintf("voxel at %ld\n",vx);
      mprintf("color map at %ld htmap at %ld\n",col,ht);
      return;
    }
   #endif

   for (i=0;i<vx->d;++i) {
      zdxdz[i] = xp;
      zdydz[i] = yp;
      xp += dxdz;
      yp += dydz;
   }

   dcrow = dv*col->row;
   dhrow = dv*ht->row;

   crow = initv*col->row;
   hrow = initv*ht->row;

   for(j=initv;j!=endv;j+=dv) {
      crow2 = col->bits + crow + initu;
      hrow2 = ht->bits + hrow + initu;

      for(i=initu;i!=endu;i+=du) {
         c = *crow2;
         if (c != 0) {
            z = *hrow2;

            #ifdef DBG_ON
            if ((z>=vxd_maxd) || (z<0) ) {
               mprintf("voxel object depth (%d,%d)=%d\n",i,j,(char)z);
               mprintf("out of bounds [0,%d]\n",vxd_maxd);
               mprintf("color map at %ld  htmap at %ld\n\n",col,ht);
               return;
            }
            #endif

            xp = (curx + zdxdz[z])>>16;
            yp = (cury + zdydz[z])>>16;

            #ifdef DBG_ON
            if (!clip) {
               xl = xp;
               xr = xp+dotw;
               yt = yp;
               yb = yp+doth;
               if (gr_clip_rect(&xl,&yt,&xr,&yb) != 0) {
                  mprintf("vox: Thair's a rectungle oot uf elaignment, lahd.\n");
                  mprintf("vox: best tell Jaeeemz, thut ruscal!\n");
                  exit(1);
                  return;
               }
            }
            #endif

            // call the box routine only if its bigger than a point
            // until gr_point supports fill modes, do boxes always.
            if (doth>1) {
               gr_set_fcolor(c);
               rect(xp,yp,xp+dotw,yp+doth);
            } else {
               gr_set_fcolor(c);
               if (clip) gr_point(xp,yp);
               else gr_upoint(xp,yp);
               //*(grd_bm.bits + yp*grd_bm.row + xp) = c;
            }

         }
         curx += dxdu;
         cury += dydu;

         crow2 += du;
         hrow2 += du;
      }
      curx = (oldcurx += dxdv);
      cury = (oldcury += dydv);

      crow += dcrow;
      hrow += dhrow;
   }
}


