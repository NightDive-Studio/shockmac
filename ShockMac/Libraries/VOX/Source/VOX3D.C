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
 * $Source: r:/prj/lib/src/vox/RCS/vox3d.c $
 * $Revision: 1.5 $
 * $Author: jaemz $
 * $Date: 1994/08/16 05:45:42 $
 *
 * 3d Interface to the voxel library
 * This file is part of the vox library
 *
 * $Log: vox3d.c $
 * Revision 1.5  1994/08/16  05:45:42  jaemz
 * Found fix overflow problem and fixed it
 * 
 * Revision 1.4  1994/08/11  22:53:20  jaemz
 * Made fill mode work and put pix_size in real 3d points.
 * 
 * Revision 1.3  1994/07/15  21:13:20  jaemz
 * Added self clipping
 * 
 * Revision 1.2  1994/04/21  12:00:36  jaemz
 * Changed interface to vox2d to facilitate bounds checking in
 * the debug version.
 * 
 * Revision 1.1  1994/04/21  10:52:42  jaemz
 * Initial revision
 * 
 */


#include <stdlib.h>
#include "vox.h"
#include "fix.h"
#include "3d.h"
//#include <mprintf.h>

//void vmap_rgbg(fix x[4],fix y[4],fix dz[3],int near_ver,grs_bitmap *col,grs_bitmap *ht);
//void vmap_poly(fix x[4],fix y[4],fix dz[3],int near_ver,grs_bitmap *col,grs_bitmap *ht);
void vmap_dot(fix x0, fix y0, fix dxdu, fix dydu, fix dxdv, fix dydv, fix dxdz, fix dydz, int near_ver,vxs_vox *vx,int dotw,int doth,bool clip);

// The four vertices of every face
//static int faces[6][4] = { {0,1,5,4},{1,2,6,5},{2,3,7,6}
//                   ,{3,0,4,7},{3,2,1,0},{4,5,6,7}};

// The dz direction of every face
//static int dzs[6][3] = { {0,0,1},{-1,0,0},{0,0,-1},{1,0,0},{0,1,0},{0,-1,0} };

// The coordinates of every vertex
// static int ver[8][3] = { {-1,-1,-1},{1,-1,-1},{1,-1,1},{-1,-1,1}
//                      ,{-1,1,-1},{1,1,-1},{1,1,1},{-1,1,1} };

// gives which visible faces for dominant vertex
// static int corners[8][3] = { {0,3,4}, {0,1,4}, {1,2,4}, {2,3,4}
//                     ,{3,0,5}, {0,1,5}, {1,2,5}, {2,3,5} };


//#define CIRCLES
//#define BMAPS
//#define BBOX
//#define STATS

void vx_render(vxs_vox *vx)
{
   fix a,b;
   //int f;
   int near_ver;   //which vertex is nearest
   g3s_vector p[4];
   g3s_phandle tmp[4];
   int i;
   int psx,psy;
   fix maxdx;
   fix maxdy;
   bool clip;
   fix tx,ty,tz;
   fix min_z;


   // static int f=0;

   #ifdef BBOX
   // for debugging
   fix x[4],y[4];
   #endif

   // spot to render on screen
   p[0].gX = p[0].gY = p[0].gZ = 0;
   // dx's and dy's and stuff
   p[1].gX = vx->pix_dist; p[1].gY = 0; p[1].gZ = 0;
   p[2].gX = 0; p[2].gY = vx->pix_dist; p[2].gZ = 0;
   p[3].gX = 0; p[3].gY = 0; p[3].gZ = vx->pix_dist;

   g3_alloc_list(4,tmp);
   g3_transform_list(4,tmp,p);

   // assuming face zero, all relative to p[0]
   // tmp[1] contains dxdu dydu
   // tmp[2] contains dxdv dydv
   // tmp[3] contains dxdz dydz

   // return if it's behind you
   if (tmp[0]->gZ < 0) {
      g3_free_list(4,tmp);
      return;
   }

   // find max and min x and y
   // note that maxdx and maxdy are in fix
   //mprintf("tmp[0]->sx,sy = %x %x\n",tmp[0]->sx,tmp[0]->sy);
  // mprintf("tmp[1]->sx,sy = %x %x\n",tmp[1]->sx,tmp[1]->sy);
   //mprintf("tmp[2]->sx,sy = %x %x\n",tmp[2]->sx,tmp[2]->sy);
   //mprintf("tmp[3]->sx,sy = %x %x\n",tmp[3]->sx,tmp[3]->sy);

   // turn vectors into deltas
   for (i=1;i<4;++i) {
      tmp[i]->sx -= tmp[0]->sx;
      tmp[i]->sy -= tmp[0]->sy;
   }

   a = tmp[0]->sx - ((vx->w*tmp[1]->sx+vx->h*tmp[2]->sx+vx->d*tmp[3]->sx)>>1);
   b = tmp[0]->sy - ((vx->w*tmp[1]->sy+vx->h*tmp[2]->sy+vx->d*tmp[3]->sy)>>1);

#ifdef CIRCLES
   // Top line from vertex 0 to 1 just for debugging
   gr_set_fcolor(255);
   gr_int_disk(fix_rint(tmp[0]->sx),fix_rint(tmp[0]->sy),5);
#endif

#ifdef BMAPS
   gr_bitmap(vx->col,fix_rint(tmp[0]->sx),fix_rint(tmp[0]->sy));
   gr_bitmap(vx->ht,fix_rint(tmp[0]->sx)-vx->w,fix_rint(tmp[0]->sy)-vx->h);
#endif

   //calculate pixel size 
   psy = fix_div(grd_bm.w * vx->pix_size,tmp[0]->gZ)>>1;
   psx = fix_mul(psy,grd_cap->aspect);

//   mprintf("psx = %x psy = %x\n",psx,psy);

   // make them at least one pixel
   if (psy<FIX_UNIT) psy = FIX_UNIT;
   if (psx<FIX_UNIT) psx = FIX_UNIT;

   maxdx = (psx<<1) + (vx->w*fix_abs(tmp[1]->sx) + vx->h*fix_abs(tmp[2]->sx) + vx->d*fix_abs(tmp[3]->sx));
   maxdy = (psy<<1) + (vx->w*fix_abs(tmp[1]->sy) + vx->h*fix_abs(tmp[2]->sy) + vx->d*fix_abs(tmp[3]->sy));
   //mprintf("maxdx = %x maxdy = %x\n",maxdx,maxdy);

   // these can overflow and become negative in extreme situations causing it not to clip
   if ((maxdy < 0) || (maxdx < 0)) {
      g3_free_list(4,tmp);
      return;
   }

   maxdx = maxdx >> 1;
   maxdy = maxdy >> 1;

   tx = fix_abs(tmp[0]->gX);
   ty = fix_abs(tmp[0]->gY);
   tz = tmp[0]->gZ;

   // clip if it SEEMS to be out of bounds, or if psx is bigger than 10.  Kind of a hack to compensate
   // for weirdo fixed point saturation.
   clip = ((tmp[0]->sx - maxdx) < 0) || ((tmp[0]->sx + maxdx) > fix_make(grd_bm.w,0)) ||
      ((tmp[0]->sy - maxdy) < 0) || ((tmp[0]->sy + maxdy) > fix_make(grd_bm.h,0)) || (psx > fix_make(10,0));

   #ifdef STATS
   mprintf("vx: tx = %g ty = %g tz = %g\n",(float)tx/65536.0,(float)ty/65536.0,(float)tz/65536.0);
   mprintf("pd = %g vx->w = %d vx->h %d\n",(float)vx->pix_dist/65536.0,vx->w,vx->h);
   mprintf("c = %d tx/tz = %g  ty/tz = %g\n",clip,(float)fix_div(tx,tz)/65536.0,(float)fix_div(ty,tz)/65536.0);
   mprintf("minus tx/tz = %g ty/tz = %g\n",(float)fix_div(tx- vx->pix_dist * vx->w,tz)/65536.0,
      (float)fix_div(ty- vx->pix_dist * vx->h,tz)/65536.0);
   #endif
                           
   if ( (tx-(vx->pix_dist * vx->w) > tz ) || (ty-(vx->pix_dist * vx->h) > tz)) {
      #ifdef BBOX
      mprintf("vox: punting due to out of view cone\n");
      #endif
      g3_free_list(4,tmp);
      return;                              
   }
    
#ifdef BBOX
   // different color when clipping
   gr_set_fcolor(0x4c+clip*0x10);
   x[0] = a;
   y[0] = b;
   x[1] = a+(vx->w)*(tmp[1]->sx);
   y[1] = b+(vx->w)*(tmp[1]->sy);
   x[2] = x[1]+(vx->h)*(tmp[2]->sx);
   y[2] = y[1]+(vx->h)*(tmp[2]->sy);
   x[3] = a+(vx->h)*(tmp[2]->sx);
   y[3] = b+(vx->h)*(tmp[2]->sy);
   
   gr_fix_line(x[0],y[0],x[1],y[1]);
   gr_fix_line(x[1],y[1],x[2],y[2]);
   gr_fix_line(x[2],y[2],x[3],y[3]);
   gr_fix_line(x[3],y[3],x[0],y[0]);
   
   for (i=0;i<4;++i) {
      x[i] += (vx->d)*(tmp[3]->sx);
      y[i] += (vx->d)*(tmp[3]->sy);
   }

   gr_fix_line(x[0],y[0],x[1],y[1]);
   gr_fix_line(x[1],y[1],x[2],y[2]);
   gr_fix_line(x[2],y[2],x[3],y[3]);
   gr_fix_line(x[3],y[3],x[0],y[0]);
#endif

   //mprintf("f = %d clip = %d\n",f++,clip);

   //   if (f==97)
   //      mprintf("uh oh\n");

   //   x[0] = tmp[0]->sx + maxdx;
   //   x[1] = tmp[0]->sx - maxdx;
   //   y[0] = tmp[0]->sy + maxdy;
   //   y[1] = tmp[0]->sy - maxdy;
   //   gr_fix_line(x[0],y[0],x[1],y[0]);
   //   gr_fix_line(x[1],y[0],x[1],y[1]);
   //   gr_fix_line(x[1],y[1],x[0],y[1]);
   //   gr_fix_line(x[0],y[1],x[0],y[0]);

   // find near vertex
   near_ver = 0;
   min_z = tmp[0]->gZ;

   // check for vertex 1
   if (tmp[1]->gZ < min_z) {
      near_ver = 1;
      min_z = tmp[1]->gZ;
   }
   if (tmp[1]->gZ + tmp[2]->gZ < min_z) {
      near_ver = 2;
      min_z = tmp[1]->gZ + tmp[2]->gZ;
   }
   if (tmp[2]->gZ < min_z) {
      near_ver = 3;
      min_z = tmp[2]->gZ;
   }

   //mprintf("near_ver = %d\n",near_ver);

   // calculate face 1
   //   for(i=0;i<1;++i) {
   //      // defines which faces visible at that vertex
   //      f = corners[near_ver][i];
   //      f = 0;
   //      for(j=0;j<4;++j) {
   //         // faces gives vertices at each face
   //         x[j] = a+v[faces[f][j]] [0];
   //         y[j] = b+v[faces[f][j]] [1];
   //      }

   // find out which one of the vertices is the near_ver
   // faces looks for all the vertices there
   //for(j=0;j<4;++j) {
   //   if (near_ver == faces[f][j]) break;
   //}
   // f= 0;

   //mprintf("clip = %d\n",clip);
   //mprintf("tmp[0]->gZ = %x\n",tmp[0]->gZ);

   vmap_dot(a,b,tmp[1]->sx,tmp[1]->sy,tmp[2]->sx,tmp[2]->sy,tmp[3]->sx,tmp[3]->sy,near_ver,vx,fix_rint(psx),fix_rint(psy),clip);

   g3_free_list(4,tmp);
}
