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
 * $Source: n:/project/lib/src/2d/RCS/genvcply.c $
 * $Revision: 1.4 $
 * $Author: baf $
 * $Date: 1994/02/14 20:38:31 $
 *
 * This file is part of the 2d library.
 *
 * $Log: genvcply.c $
 * Revision 1.4  1994/02/14  20:38:31  baf
 * Added dummy parameter to cpoly and spoly routines, for uniformity needed by 3D.
 * 
 * Revision 1.3  1993/10/19  10:14:04  kaboom
 * Updated calls to polygon routines for new arguments.
 * 
 * Revision 1.2  1993/10/08  01:15:52  kaboom
 * Changed quotes in #include lines to angle brackets for Watcom.
 * 
 * Revision 1.1  1993/08/19  21:52:29  jaemz
 * Initial revision
 */

#include "grcply.h"
#include "grrend.h"
#include "plytyp.h"
#include "scrdat.h"
#include "general.h"

void gen_vox_cpoly(fix x[4],fix y[4],fix dz[3],int near_ver,grs_bitmap *col,grs_bitmap *ht)
{
   int du,dv,initu,endu,initv,endv;
   int i,j;
   int c;
   long z;
   int far_ver;

   /* Test of broadcasting system */
   fix fdxdu,fdydu,fdxdv,fdydv;
   fix fcurx,fcury;
   fix fdxdz,fdydz;
   fix oldcurx;
   fix oldcury;
   fix vlist[20];
   grs_vertex *vpl[4];
//   fix vlist[8];
//   grs_rgb clist[4];
   int xp[2][80];
   int yp[2][80];
   ulong pp[2][80];
   int currow = 0;

   vpl[0]=(grs_vertex *)vlist; vpl[1]=(grs_vertex *)(vlist+5);
   vpl[2]=(grs_vertex *)(vlist+10); vpl[3]=(grs_vertex *)(vlist+15);
   far_ver = (near_ver+2)%4;

   /* How to scan given near_ver */
   if (near_ver == 0 || near_ver == 3) { initu = col->w -1; du = -1; endu = -1; }
   else { initu = 0; du = 1; endu = col->w ; }
   if (near_ver < 2) { initv = col->h-1; dv = -1; endv = -1; }
   else { initv = 0; dv = 1; endv = col->h; }

   fdxdu = (x[1]-x[0])/(col->w - 1);
   fdydu = (y[1]-y[0])/(col->w - 1);
   fdxdv = (x[2]-x[1])/(col->w - 1);
   fdydv = (y[2]-y[1])/(col->w - 1);

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
         c = *( col->bits +i +j*col->row);
         //c = get_pal(col,i,j);
         if (c != 0) {
            z = *(ht->bits + i + j*col->row);
            //z = get_pal(ht,i,j);
            /* Put in array */
            xp[currow][i] = (fcurx + fdxdz*z);
            yp[currow][i] = (fcury + fdydz*z);
            pp[currow][i] = c;
         }
         else {
            pp[currow][i] = 0;
         }
         fcurx += fdxdu;
         fcury += fdydu;
      }
      if (currow==1) {
         /* plot the triangles */
         for(i=initu;i!=endu;i+=du) {
            /* check for any transparents */
            if (pp[0][i]*pp[1][i]*pp[1][i+du]*pp[0][i+du] != 0) {
//               vlist[0] = xp[0][i];
//               vlist[1] = yp[0][i];
//               clist[0] = grd_bpal[pp[0][i]];
               vpl[0]->x = xp[0][i];
               vpl[0]->y = yp[0][i];
               vpl[0]->u = fix_make(grd_pal[pp[0][i]+0],0x8000);
               vpl[0]->v = fix_make(grd_pal[pp[0][i]+1],0x8000);
               vpl[0]->w = fix_make(grd_pal[pp[0][i]+2],0x8000);

//               vlist[4] = xp[1][i+du];
//               vlist[5] = yp[1][i+du];
//               clist[2] = grd_bpal[pp[1][i+du]];
               vpl[2]->x = xp[1][i+du];
               vpl[2]->y = yp[1][i+du];
               vpl[2]->u = fix_make(grd_pal[pp[1][i+du]+0],0x8000);
               vpl[2]->v = fix_make(grd_pal[pp[1][i+du]+1],0x8000);
               vpl[2]->w = fix_make(grd_pal[pp[1][i+du]+2],0x8000);

               if (du*dv > 0) {
//                  vlist[2] = xp[0][i+du];
//                  vlist[3] = yp[0][i+du];
//                  clist[1] = grd_bpal[pp[0][i+du]];
                  vpl[1]->x = xp[0][i+du];
                  vpl[1]->y = yp[0][i+du];
                  vpl[1]->u = fix_make(grd_pal[pp[0][i+du]+0],0x8000);
                  vpl[1]->v = fix_make(grd_pal[pp[0][i+du]+1],0x8000);
                  vpl[1]->w = fix_make(grd_pal[pp[0][i+du]+2],0x8000);

//                  vlist[6] = xp[1][i];
//                  vlist[7] = yp[1][i];
//                  clist[3] = grd_bpal[pp[1][i]];
                  vpl[3]->x = xp[1][i+du];
                  vpl[3]->y = yp[1][i+du];
                  vpl[3]->u = fix_make(grd_pal[pp[1][i+du]+0],0x8000);
                  vpl[3]->v = fix_make(grd_pal[pp[1][i+du]+1],0x8000);
                  vpl[3]->w = fix_make(grd_pal[pp[1][i+du]+2],0x8000);
               }
               else {
//                  vlist[6] = xp[0][i+du];
//                  vlist[7] = yp[0][i+du];
//                  clist[3] = grd_bpal[pp[0][i+du]];
                  vpl[3]->x = xp[0][i+du];
                  vpl[3]->y = yp[0][i+du];
                  vpl[3]->u = fix_make(grd_pal[pp[0][i+du]+0],0x8000);
                  vpl[3]->v = fix_make(grd_pal[pp[0][i+du]+1],0x8000);
                  vpl[3]->w = fix_make(grd_pal[pp[0][i+du]+2],0x8000);

//                  vlist[2] = xp[1][i];
//                  vlist[3] = yp[1][i];
//                  clist[1] = grd_bpal[pp[1][i]];
                  vpl[1]->x = xp[1][i];
                  vpl[1]->y = yp[1][i];
                  vpl[1]->u = fix_make(grd_pal[pp[1][i]+0],0x8000);
                  vpl[1]->v = fix_make(grd_pal[pp[1][i]+1],0x8000);
                  vpl[1]->w = fix_make(grd_pal[pp[1][i]+2],0x8000);
                }

               gr_ucpoly(0,4,vpl);
            }
         }
         /* copy from 1 to 0 */
         for(i=initu;i!=endu;i+=du) {
            xp[0][i] = xp[1][i];
            yp[0][i] = yp[1][i];
            pp[0][i] = pp[1][i];
         }
      }
      else ++currow;
      fcurx = (oldcurx += fdxdv);
      fcury = (oldcury += fdydv);
   }
}


