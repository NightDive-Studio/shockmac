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
 * $Source: r:/prj/lib/src/2d/RCS/persetup.c $
 * $Revision: 1.3 $
 * $Author: kevin $
 * $Date: 1994/11/02 19:39:48 $
 * 
 * setup routines for full perspective mappers.
 * 
*/

#include "cnvdat.h"
#include "plytyp.h"
#include "scrdat.h"
#include "buffer.h"
#include "pertyp.h"
#include "fix.h"
#include "fl8p.h"

//#include "mprintf.h"

// prototypes
int gri_per_umap_setup (int n, grs_vertex **vplist, grs_per_setup *ps);

grs_per_context *grd_per_context=NULL;  /* perspective mapping context */
struct {
   fix scale,x_off,y_off;
} grd_pc_trans;
struct {
   grs_point3d p0;
   fix u0,v0;
} grd_pc_point;

int gri_per_umap_setup (int n, grs_vertex **vplist, grs_per_setup *ps)
{
   fix a,b,c0,cz,z_min,aos,bos;
   fix z_max,y_min,y_max,x_min,x_max;
   fix wux,wuy,wuz,wvx,wvy,wvz;
   grs_point3d *l3d,*pl3d,*l3d0,*l3d1,*l3d2;
   grs_vertex *vp,**pvp,*vpl0,*vpl1,*vpl2,*vpl_z_max,*vpl_z_min;
   int i;
   fix delta_min;
   int scale;

   scale = (grd_bm.w>grd_bm.h) ? grd_bm.w:grd_bm.h;
   l3d = (grs_point3d *)gr_alloc_temp(n*sizeof(grs_point3d));
   if (l3d==NULL) return GR_PER_CODE_MEMERR;

/* First get variables for plane equation: a*x+b*y+c*z=k */
   z_max=FIX_MIN;   z_min=FIX_MAX;
   y_max=FIX_MIN;   y_min=FIX_MAX;
   x_max=FIX_MIN;   x_min=FIX_MAX;
   for (pl3d=l3d,vp=*(pvp=vplist),i=0;i<n;i++,pl3d++,vp=*(++pvp)) {
      pl3d->x=(vp->x-fix_make(ACENT,0));
      pl3d->y=(vp->y-fix_make(BCENT,0));
      pl3d->z=fix_div(FIX_UNIT,vp->w);
      if (z_max<pl3d->z) {z_max=pl3d->z; vpl_z_max=vp;}
      if (vp->x>x_max) x_max=vp->x;
      if (vp->x<x_min) x_min=vp->x;
      if (vp->y>y_max) y_max=vp->y;
      if (vp->y<y_min) y_min=vp->y;
   }
   delta_min=x_max-x_min;
   if (delta_min<y_max-y_min) delta_min=y_max-y_min;
   if (fix_int(delta_min)<(10-flat8_per_ltol)<<1) {
      /*use linear mapper.*/
      gr_free_temp(l3d);
      return GR_PER_CODE_LIN;
   }
   for (pl3d=l3d,i=0;i<n;i++,pl3d++) {
      pl3d->z = fix_div_16_16_3(pl3d->z,z_max);
      if (z_min>pl3d->z) {z_min=pl3d->z; vpl_z_min=vplist[i];}
      pl3d->x = fix_mul_div_3_16_16_3(pl3d->x,pl3d->z,fix_make(scale,0));
      pl3d->y = fix_mul_div_3_16_16_3(pl3d->y,pl3d->z,fix_make(scale,0));
   }
   if (fix_sar(z_min,flat8_per_ltol) > (FIX_UNIT_3-z_min)) {
      /*use linear mapper.*/
      gr_free_temp(l3d);
      return GR_PER_CODE_LIN;
   }
/**********************************************************
Determine best vertices to use for plane calculations.
Use the three vertices with the maximum shortest distance
between pairs.
**********************************************************/
   {
      fix maxsd, minsd, sd;
      grs_vertex *v1,*v2,*v3;
      short j,k,i0=0,i1=1,i2=2;
 
      maxsd=0;
      for (v1=vplist[i=0]; i<n; v1=vplist[++i]) {
         for (v2=vplist[j=i+1]; j<n; v2=vplist[++j]) {
            for (v3=vplist[k=j+1]; k<n; v3=vplist[++k]) {
               minsd=fix_abs(v1->x - v2->x)
                  +fix_abs(v1->y - v2->y);
               sd=fix_abs(v3->x - v2->x)
                  +fix_abs(v3->y - v2->y);
               if (sd<minsd) minsd=sd;
               sd=fix_abs(v1->x - v3->x)
                  +fix_abs(v1->y - v3->y);
               if (sd<minsd) minsd=sd;
               if (minsd>maxsd) {
                  maxsd=minsd; i0=i; i1=j; i2=k;
               }
            }
         }
      }
      l3d0=l3d+i0; l3d1=l3d+i1; l3d2=l3d+i2;
      vpl0=vplist[i0]; vpl1=vplist[i1]; vpl2=vplist[i2];
   }

   a=fix_mul_3_3_3((l3d1->y-l3d0->y),(l3d2->z-l3d0->z))
      -fix_mul_3_3_3((l3d2->y-l3d0->y),(l3d1->z-l3d0->z));
   b=fix_mul_3_3_3((l3d1->z-l3d0->z),(l3d2->x-l3d0->x))
      -fix_mul_3_3_3((l3d2->z-l3d0->z),(l3d1->x-l3d0->x));
   cz=fix_mul_3_3_3((l3d1->x-l3d0->x),(l3d2->y-l3d0->y))
      -fix_mul_3_3_3((l3d2->x-l3d0->x),(l3d1->y-l3d0->y));
   c0=(fix_mul_3_3_3(a,l3d0->x)+fix_mul_3_3_3(b,l3d0->y)+fix_mul_3_3_3(cz,l3d0->z));
                                        
    if (fix_sar(fix_abs(a),7) > fix_sar(fix_abs(b),7-flat8_per_wftol)) {
//       use wall mapper
       gr_free_temp(l3d);
       return GR_PER_CODE_WALL;
    }
    if (fix_sar(fix_abs(b),7) > fix_sar(fix_abs(a),7-flat8_per_wftol)) {
//       use floor mapper
       gr_free_temp(l3d);
       return GR_PER_CODE_FLOOR;
    }

/******************************************************
so virtual scan line equations are given by
a*x_scr+b*y_scr=scale*((c0/z)-cz)
******************************************************/
                                         
/**************************************************************************
Get wu, wv: vectors s.t. u-u0=wu*(r-r0); v-v0=wv*(r-r0).
the solutions are determined from two points in the plane (besides r0) and
the fact that wu and wv are perpendicular to the normal to the plane,
f=(a,b,cz).
**************************************************************************/
   {
      fix dx1, dx2, dy1, dy2, dz1, dz2, denom;  /*3's*/
      fix du1, du2;                             /*16's*/
      fix dv1, dv2;

      dx1=l3d1->x-l3d0->x; dx2=l3d2->x-l3d0->x;
      dy1=l3d1->y-l3d0->y; dy2=l3d2->y-l3d0->y;
      dz1=l3d1->z-l3d0->z; dz2=l3d2->z-l3d0->z;
      du1=vpl1->u-vpl0->u; du2=vpl2->u-vpl0->u;
      dv1=vpl1->v-vpl0->v; dv2=vpl2->v-vpl0->v;

      denom=-(fix_mul_3_3_3(a,a)+fix_mul_3_3_3(b,b)
         +fix_mul_3_3_3(cz,cz));
      if (denom==0) {gr_free_temp(l3d); return GR_PER_CODE_BADPLANE;}

      wux=fix_div_16_3_16(fix_mul_3_16_16(b,fix_mul_3_16_16(dz2,du1)-fix_mul_3_16_16(dz1,du2))
         -fix_mul_3_16_16(cz,fix_mul_3_16_16(dy2,du1)-fix_mul_3_16_16(dy1,du2)),denom);
      wuy=fix_div_16_3_16(fix_mul_3_16_16(cz,fix_mul_3_16_16(dx2,du1)-fix_mul_3_16_16(dx1,du2))
         -fix_mul_3_16_16(a,fix_mul_3_16_16(dz2,du1)-fix_mul_3_16_16(dz1,du2)),denom);
      wuz=fix_div_16_3_16(fix_mul_3_16_16(a,fix_mul_3_16_16(dy2,du1)-fix_mul_3_16_16(dy1,du2))
         -fix_mul_3_16_16(b,fix_mul_3_16_16(dx2,du1)-fix_mul_3_16_16(dx1,du2)),denom);

      wvx=fix_div_16_3_16(fix_mul_3_16_16(b,fix_mul_3_16_16(dz2,dv1)-fix_mul_3_16_16(dz1,dv2))
         -fix_mul_3_16_16(cz,fix_mul_3_16_16(dy2,dv1)-fix_mul_3_16_16(dy1,dv2)),denom);
      wvy=fix_div_16_3_16(fix_mul_3_16_16(cz,fix_mul_3_16_16(dx2,dv1)-fix_mul_3_16_16(dx1,dv2))
         -fix_mul_3_16_16(a,fix_mul_3_16_16(dz2,dv1)-fix_mul_3_16_16(dz1,dv2)),denom);
      wvz=fix_div_16_3_16(fix_mul_3_16_16(a,fix_mul_3_16_16(dy2,dv1)-fix_mul_3_16_16(dy1,dv2))
         -fix_mul_3_16_16(b,fix_mul_3_16_16(dx2,dv1)-fix_mul_3_16_16(dx1,dv2)),denom);

   }
   aos=fix_make(ACENT,0)/scale;
   bos=fix_make(BCENT,0)/scale;
   ps->alpha_v=fix_mul_3_16_20(c0,wvx);
   ps->beta_v=fix_mul_3_16_20(c0,wvy);
   ps->gamma_v=fix_mul_16_32_20(fix_mul_3_16_16(c0,wvz-
      (fix_mul(wvx,aos)+fix_mul(wvy,bos))),scale);
   ps->alpha_u=fix_mul_3_16_20(c0,wux);
   ps->beta_u=fix_mul_3_16_20(c0,wuy);
   ps->gamma_u=fix_mul_16_32_20(fix_mul_3_16_16(c0,wuz-
      (fix_mul(wux,aos)+fix_mul(wuy,bos))),scale);
//   mprintf("gu:%x  gv:%x\n",ps->gamma_u,ps->gamma_v);
   ps->a=fix_3_16(a);
   ps->b=fix_3_16(b);
   ps->c=fix_mul_3_32_16(cz,scale)-fix_mul_3_32_16(a,ACENT)-fix_mul_3_32_16(b,BCENT);
   if (fix_abs(a)<=fix_abs(b)) {
      ps->scan_slope=-fix_div_3_3_16(a,b);
      gr_free_temp(l3d);
      return GR_PER_CODE_BIGSLOPE;
   }
   else {
      ps->scan_slope=-fix_div_3_3_16(b,a);
      gr_free_temp(l3d);
      return GR_PER_CODE_SMALLSLOPE;
   }
}
