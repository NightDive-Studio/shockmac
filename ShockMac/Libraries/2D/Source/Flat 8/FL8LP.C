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
 * $Source: r:/prj/lib/src/2d/RCS/fl8lp.c $
 * $Revision: 1.23 $
 * $Author: kevin $
 * $Date: 1994/11/02 19:39:45 $
 * 
 * lit full perspective texture mapper.
 * 
*/

// ************************************************************************************
// ************************************************************************************
// 
//  MLA - don't think we need to optimize this, its in C on the PC too
//
// ************************************************************************************
// ************************************************************************************

#include "cnvdat.h"
#include "pertyp.h"
#include "plytyp.h"
#define safe_fix_cint(x) ((fix_frac(x)==0) ? (fix_int(x)):(fix_int(x)+1))
#define fix_16_20(a) ((a)>>4)

/**************************************************************
Routines to scan polygon.  hscan=standard horizontal scanlines.
vscan=vertical scanlines.
**************************************************************/

// prototypes
void gri_lit_per_umap_hscan(grs_bitmap *bm, int n, grs_vertex **vpl, grs_per_setup *ps);
void gri_lit_per_umap_vscan(grs_bitmap *bm, int n, grs_vertex **vpl, grs_per_setup *ps);


void gri_lit_per_umap_hscan(grs_bitmap *bm, int n, grs_vertex **vpl, grs_per_setup *ps)
{
   grs_per_info pi;
   fix y_prime[10];
   fix yp_left,yp_right;
   fix x_left,x_right;
   fix y_left,y_right;
   fix dx_left,dx_right;
   fix i_left,i_right;
   fix di_left,di_right;
   int yp_min,yp_max,yp_next;
   int x_min,x_max,xr_min,xr_max,xl_min,xl_max;
   int n_min,n_left,n_right;
   int j;

   pi.scale=grd_bm.w;
   pi.scan_slope=ps->scan_slope;
   pi.dp=ps->dp;
   pi.clut=ps->clut;
   if (fix_abs(pi.scan_slope) > FIX_UNIT) return;

   if (bm->row != 1<<(bm->wlog)) return;
   if (bm->h != 1<<(bm->hlog)) return;
   pi.u_mask=bm->row-1;
   pi.v_mask=(bm->h-1)<<bm->wlog;
   pi.v_shift=16-bm->wlog;

   yp_min=yp_max=fix_cint(y_prime[n_min=0]=vpl[0]->y-fix_mul(vpl[0]->x,ps->scan_slope));
   for (j=1; j<n; j++) {
      pi.yp=fix_cint(y_prime[j]=vpl[j]->y-fix_mul(vpl[j]->x,ps->scan_slope));
      if (pi.yp<yp_min) {
         yp_min=pi.yp;
         n_min=j;
      }
      if (pi.yp>yp_max)
         yp_max=pi.yp;
   }
   if (yp_max==yp_min) return;
   pi.denom=fix_16_20(ps->c+fix_mul(ps->b,y_prime[0]));
   pi.u0=vpl[0]->u-fix_div(fix_mul(vpl[0]->x,ps->alpha_u)+fix_mul(vpl[0]->y,ps->beta_u)+ps->gamma_u,pi.denom);
   pi.v0=vpl[0]->v-fix_div(fix_mul(vpl[0]->x,ps->alpha_v)+fix_mul(vpl[0]->y,ps->beta_v)+ps->gamma_v,pi.denom);

   n_left=n_right=n_min;
   pi.yp=yp_min;
   while (fix_cint(y_prime[(n_left+n-1)%n])==pi.yp)
      n_left=(n_left+n-1)%n;
   while (fix_cint(y_prime[(n_right+1)%n])==pi.yp)
      n_right=(n_right+1)%n;

   pi.yp--;
   pi.denom=fix_16_20(ps->c+pi.yp*ps->b);
   pi.unum=ps->gamma_u+pi.yp*ps->beta_u;
   pi.dunum=ps->alpha_u+fix_mul(ps->scan_slope,ps->beta_u);
   pi.vnum=ps->gamma_v+pi.yp*ps->beta_v;
   pi.dvnum=ps->alpha_v+fix_mul(ps->scan_slope,ps->beta_v);

   if (n_right!=n_left) {

      pi.dxl=(vpl[n_right]->x-vpl[n_left]->x)/pi.scale;
      pi.dyl=(vpl[n_right]->y-vpl[n_left]->y)/pi.scale;
      pi.x=fix_cint(vpl[n_left]->x);
      pi.xl=fix_cint(vpl[n_right]->x);
      if (pi.scan_slope>0) {
         x_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.bot-1)-pi.yp,1),pi.scan_slope));
         x_min=fix_int(fix_div(fix_make((grd_int_clip.top-1)-pi.yp,1),pi.scan_slope))+1;
      }
      else {
         x_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.top-1)-pi.yp,1),pi.scan_slope));
         x_min=fix_int(fix_div(fix_make((grd_int_clip.bot-1)-pi.yp,1),pi.scan_slope))+1;
      }
      if (pi.x<x_min) pi.x=x_min;
      if (pi.xl>x_max) pi.xl=x_max;
      pi.xr0=pi.xr=pi.xl;
      pi.i=vpl[n_left]->i;
      if (pi.xr-pi.x) pi.di=(vpl[n_right]->i-pi.i)/(pi.xr-pi.x);
      else {pi.di = 0x7fffffff; if (vpl[n_right]->i-pi.i < 0) pi.di = -pi.di;}
      
      pi.cl=fix_mul(pi.dxl,vpl[n_left]->y)-fix_mul(pi.dyl,vpl[n_left]->x);
      if (pi.x * pi.dyl - fix_mul(fix_make(pi.yp-1,0) + pi.x * pi.scan_slope,pi.dxl) + pi.cl<0) {
         pi.dyl=-pi.dyl;
         pi.dxl=-pi.dxl;
         pi.cl=-pi.cl;
      }
      if (pi.scan_slope>0) pi.dtl=pi.dyl-pi.dxl;
      else pi.dtl=pi.dyl+pi.dxl;

			((void (*)(grs_per_info *, grs_bitmap *))(ps->scanline_func))(&pi,bm);

      pi.denom+=fix_16_20(ps->b),pi.unum+=ps->beta_u,pi.vnum+=ps->beta_v;
      yp_min--;  /* first line already done */
   }
   pi.yp++;
   while (pi.yp<yp_max) {
      /* check left edge */
      if (fix_cint(y_prime[n_left])<=pi.yp) {
         int n_prev,dyp;
         fix d;
         do {
            if (fix_cint(y_prime[n_left])==pi.yp) n_prev=n_left;
            if (--n_left < 0) n_left=n-1;
         } while (fix_cint(y_prime[n_left])<=pi.yp);
         yp_left=y_prime[n_prev];
         x_left=vpl[n_prev]->x;
         y_left=vpl[n_prev]->y;
         i_left=vpl[n_prev]->i;
         dyp=fix_cint(y_prime[n_left])-fix_cint(yp_left);
         di_left=(vpl[n_left]->i-i_left)/dyp;
         xl_min=fix_cint(x_left);
         xl_max=fix_cint(vpl[n_left]->x);
         pi.dxl=(vpl[n_left]->x-x_left)/pi.scale;
         pi.dyl=(vpl[n_left]->y-y_left)/pi.scale;
         pi.cl=fix_mul(pi.dxl,y_left)-fix_mul(pi.dyl,x_left);
         d=pi.dyl-fix_mul(pi.dxl,pi.scan_slope);
         dx_left=fix_div(pi.dxl,d);
         x_left=fix_div(fix_mul(pi.dyl,x_left)+fix_mul(pi.dxl,fix_ceil(yp_left)-y_left),d);
         if (xl_max<xl_min) {
            fix foo=xl_min;
            xl_min=xl_max;
            xl_max=foo;
         }
         if (fix_mul(vpl[n_left]->x - FIX_UNIT,pi.dyl) - fix_mul(vpl[n_left]->y - pi.scan_slope,pi.dxl)
               + pi.cl<0) {
            pi.dyl=-pi.dyl;
            pi.dxl=-pi.dxl;
            pi.cl=-pi.cl;
         }
         if (pi.scan_slope>0) pi.dtl=pi.dyl-pi.dxl;
         else pi.dtl=pi.dyl+pi.dxl;
         yp_left=y_prime[n_left];
      }

      /* check right edge */
      if (fix_cint(y_prime[n_right])<=pi.yp) {
         int n_prev,dyp;
         fix d;
         do {
            if (fix_cint(y_prime[n_right])==pi.yp) n_prev=n_right;
            if (++n_right == n) n_right=0;
         } while (fix_cint(y_prime[n_right])<=pi.yp);
         yp_right=y_prime[n_prev];
         x_right=vpl[n_prev]->x;
         y_right=vpl[n_prev]->y;
         i_right=vpl[n_prev]->i;
         dyp=fix_cint(y_prime[n_right])-fix_cint(yp_right);
         di_right=(vpl[n_right]->i-i_right)/dyp;
         xr_min=fix_cint(x_right);
         xr_max=fix_cint(vpl[n_right]->x);
         pi.dxr=(vpl[n_right]->x-x_right)/pi.scale;
         pi.dyr=(vpl[n_right]->y-y_right)/pi.scale;
         pi.cr=fix_mul(pi.dxr,y_right)-fix_mul(pi.dyr,x_right);
         d=pi.dyr-fix_mul(pi.dxr,pi.scan_slope);
         dx_right=fix_div(pi.dxr,d);
         x_right=fix_div(fix_mul(pi.dyr,x_right)+fix_mul(pi.dxr,fix_ceil(yp_right)-y_right),d);
         if (xr_max<xr_min) {
            fix foo=xr_min;
            xr_min=xr_max;
            xr_max=foo;
         }
         if (fix_mul(vpl[n_right]->x - FIX_UNIT,pi.dyr) - fix_mul(vpl[n_right]->y - pi.scan_slope,pi.dxr)
               + pi.cr<0) {
            pi.dyr=-pi.dyr;
            pi.dxr=-pi.dxr;
            pi.cr=-pi.cr;
         }
         if (pi.scan_slope>0) pi.dtr=pi.dyr-pi.dxr;
         else pi.dtr=pi.dyr+pi.dxr;
         yp_right=y_prime[n_right];
      }
      yp_next = (yp_right<yp_left) ? fix_cint(yp_right):fix_cint(yp_left);

      /* do 0th scanline if at yp_min */
      if (pi.yp==yp_min) {
         x_left-=dx_left;
         x_right-=dx_right;
         pi.yp--;
      }
      for (;pi.yp<yp_next;pi.yp++) {
         if ((pi.yp+1==yp_max) && (n_left!=n_right)) {
            pi.dxl=(vpl[n_right]->x-vpl[n_left]->x)/pi.scale;
            pi.dyl=(vpl[n_right]->y-vpl[n_left]->y)/pi.scale;
            pi.x=fix_cint(vpl[n_left]->x);
            pi.xl=fix_cint(vpl[n_right]->x);
            if (pi.scan_slope>0) {
               x_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.bot-1)-pi.yp,1),pi.scan_slope));
               x_min=fix_int(fix_div(fix_make((grd_int_clip.top-1)-pi.yp,1),pi.scan_slope))+1;
            }
            else {
               x_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.top-1)-pi.yp,1),pi.scan_slope));
               x_min=fix_int(fix_div(fix_make((grd_int_clip.bot-1)-pi.yp,1),pi.scan_slope))+1;
            }
            if (pi.x<x_min) pi.x=x_min;
            if (pi.xl>x_max) pi.xl=x_max;
            pi.xr0=pi.xr=pi.xl;
            pi.i=i_left;
            if (pi.xr-pi.x) pi.di=(i_right-pi.i)/(pi.xr-pi.x);
            else {pi.di = 0x7fffffff; if (i_right-pi.i < 0) pi.di = -pi.di;}

            pi.cl=fix_mul(pi.dxl,vpl[n_left]->y)-fix_mul(pi.dyl,vpl[n_left]->x);
            if (pi.x*pi.dyl - fix_mul(fix_make(pi.yp+1,0) + pi.x*pi.scan_slope,pi.dxl) + pi.cl<0) {
               pi.dyl=-pi.dyl;
               pi.dxl=-pi.dxl;
               pi.cl=-pi.cl;
            }
            if (pi.scan_slope>0) pi.dtl=pi.dyl-pi.dxl;
            else pi.dtl=pi.dyl+pi.dxl;

            ((void (*)(grs_per_info *, grs_bitmap *))(ps->scanline_func))(&pi,bm);

            pi.yp=yp_max;
            break;
         }

         if (dx_left>0) {
            pi.x=fix_fint(x_left);
            pi.xl=fix_cint(x_left+dx_left);
         } else {
            pi.x=fix_fint(x_left+dx_left);
            pi.xl=fix_cint(x_left);
         }
         if (dx_right>0) {
            pi.xr0=fix_fint(x_right);
            pi.xr=fix_cint(x_right+dx_right);
         } else {
            pi.xr0=fix_fint(x_right+dx_right);
            pi.xr=fix_cint(x_right);
         }
         if (pi.scan_slope>0) {
            x_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.bot-1)-pi.yp,1),pi.scan_slope));
            x_min=fix_int(fix_div(fix_make((grd_int_clip.top-1)-pi.yp,1),pi.scan_slope))+1;
         }
         else {
            x_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.top-1)-pi.yp,1),pi.scan_slope));
            x_min=fix_int(fix_div(fix_make((grd_int_clip.bot-1)-pi.yp,1),pi.scan_slope))+1;
         }
         if (xl_min>x_min) x_min=xl_min;
         if (xr_max<x_max) x_max=xr_max;
         if (pi.xr>x_max) pi.xr=x_max;
         if (pi.xr0>x_max) pi.xr0=x_max;
         if (pi.xl>pi.xr0) pi.xl=pi.xr0;
         if (pi.x<x_min) pi.x=x_min;
         pi.i=i_left;
         if (pi.xr-pi.x) pi.di=(i_right-pi.i)/(pi.xr-pi.x);
         else {pi.di = 0x7fffffff; if (i_right-pi.i < 0) pi.di = -pi.di;}

         ((void (*)(grs_per_info *, grs_bitmap *))(ps->scanline_func))(&pi,bm);

         if (pi.yp>=yp_min) {
            i_left+=di_left,i_right+=di_right;
         }
         x_left+=dx_left, x_right+=dx_right;
         pi.denom+=fix_16_20(ps->b),pi.unum+=ps->beta_u,pi.vnum+=ps->beta_v;
      }
   }
}

void gri_lit_per_umap_vscan(grs_bitmap *bm, int n, grs_vertex **vpl, grs_per_setup *ps)
{
   grs_per_info pi;
   fix x_prime[10];
   fix xp_top,xp_bot;
   fix x_top,x_bot;
   fix y_top,y_bot;
   fix dy_top,dy_bot;
   fix i_top,i_bot;
   fix di_top,di_bot;
   int xp_min,xp_max,xp_next;
   int y_min,y_max,yr_min,yr_max,yl_min,yl_max;
   int n_min,n_top,n_bot;
   int j;

   pi.scale=grd_bm.w;
   pi.scan_slope=ps->scan_slope;
   pi.dp=ps->dp;
   pi.clut=ps->clut;
   if (fix_abs(pi.scan_slope) >= FIX_UNIT) return;

   if (bm->row != 1<<(bm->wlog)) return;
   if (bm->h != 1<<(bm->hlog)) return;
   pi.u_mask=bm->row-1;
   pi.v_mask=(bm->h-1)<<bm->wlog;
   pi.v_shift=16-bm->wlog;

   xp_min=xp_max=fix_cint(x_prime[n_min=0]=vpl[0]->x-fix_mul(vpl[0]->y,ps->scan_slope));
   for (j=1; j<n; j++) {
      pi.xp=fix_cint(x_prime[j]=vpl[j]->x-fix_mul(vpl[j]->y,ps->scan_slope));
      if (pi.xp<xp_min) {
         xp_min=pi.xp;
         n_min=j;
      }
      if (pi.xp>xp_max)
         xp_max=pi.xp;
   }
   if (xp_max==xp_min) return;
   pi.denom=fix_16_20(ps->c+fix_mul(ps->a,x_prime[0]));
   pi.u0=vpl[0]->u-fix_div(fix_mul(vpl[0]->x,ps->alpha_u)+fix_mul(vpl[0]->y,ps->beta_u)+ps->gamma_u,pi.denom);
   pi.v0=vpl[0]->v-fix_div(fix_mul(vpl[0]->x,ps->alpha_v)+fix_mul(vpl[0]->y,ps->beta_v)+ps->gamma_v,pi.denom);

   n_top=n_bot=n_min;
   pi.xp=xp_min;
   while (fix_cint(x_prime[(n_top+1)%n])==pi.xp)
      n_top=(n_top+1)%n;
   while (fix_cint(x_prime[(n_bot+n-1)%n])==pi.xp)
      n_bot=(n_bot+n-1)%n;

   pi.xp--;
   pi.denom=fix_16_20(ps->c+pi.xp*ps->a);
   pi.unum=ps->gamma_u+pi.xp*ps->alpha_u;
   pi.dunum=ps->beta_u+fix_mul(ps->scan_slope,ps->alpha_u);
   pi.vnum=ps->gamma_v+pi.xp*ps->alpha_v;
   pi.dvnum=ps->beta_v+fix_mul(ps->scan_slope,ps->alpha_v);

   if (n_bot!=n_top) {

      pi.dxl=(vpl[n_bot]->x-vpl[n_top]->x)/pi.scale;
      pi.dyl=(vpl[n_bot]->y-vpl[n_top]->y)/pi.scale;
      pi.y=fix_cint(vpl[n_top]->y);
      pi.yl=fix_cint(vpl[n_bot]->y);
      if (pi.scan_slope>0) {
         y_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.right-1)-pi.xp,1),pi.scan_slope));
         y_min=fix_int(fix_div(fix_make((grd_int_clip.left-1)-pi.xp,1),pi.scan_slope))+1;
      }
      else {
         y_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.left-1)-pi.xp,1),pi.scan_slope));
         y_min=fix_int(fix_div(fix_make((grd_int_clip.right-1)-pi.xp,1),pi.scan_slope))+1;
      }
      if (pi.y<y_min) pi.y=y_min;
      if (pi.yl>y_max) pi.yl=y_max;
      pi.yr0=pi.yr=pi.yl;
      pi.i=vpl[n_top]->i;
      if (pi.yr-pi.y) pi.di=(vpl[n_bot]->i-pi.i)/(pi.yr-pi.y);
      else {pi.di = 0x7fffffff; if (vpl[n_bot]->i-pi.i < 0) pi.di = -pi.di;}

      pi.cl=-fix_mul(pi.dxl,vpl[n_top]->y)+fix_mul(pi.dyl,vpl[n_top]->x);
      if (pi.y * pi.dxl - fix_mul(fix_make(pi.xp-1,0) + pi.y * pi.scan_slope,pi.dyl) + pi.cl<0) {
         pi.dyl=-pi.dyl;
         pi.dxl=-pi.dxl;
         pi.cl=-pi.cl;
      }
      if (pi.scan_slope>0) pi.dtl=pi.dxl-pi.dyl;
      else pi.dtl=pi.dyl+pi.dxl;

      ((void (*)(grs_per_info *, grs_bitmap *))(ps->scanline_func))(&pi,bm);

      pi.denom+=fix_16_20(ps->a),pi.unum+=ps->alpha_u,pi.vnum+=ps->alpha_v;
      xp_min--;  /* first line already done */
   }
   pi.xp++;
   while (pi.xp<xp_max) {
      /* check top edge */
      if (fix_cint(x_prime[n_top])<=pi.xp) {
         int n_prev,dxp;
         fix d;
         do {
            if (fix_cint(x_prime[n_top])==pi.xp) n_prev=n_top;
            if (++n_top == n) n_top=0;
         } while (fix_cint(x_prime[n_top])<=pi.xp);
         xp_top=x_prime[n_prev];
         x_top=vpl[n_prev]->x;
         y_top=vpl[n_prev]->y;
         i_top=vpl[n_prev]->i;
         dxp=fix_cint(x_prime[n_top])-fix_cint(xp_top);
         di_top=(vpl[n_top]->i-i_top)/dxp;
         yl_min=fix_cint(y_top);
         yl_max=fix_cint(vpl[n_top]->y);
         pi.dxl=(vpl[n_top]->x-x_top)/pi.scale;
         pi.dyl=(vpl[n_top]->y-y_top)/pi.scale;
         pi.cl=-fix_mul(pi.dxl,y_top)+fix_mul(pi.dyl,x_top);
         d=pi.dxl-fix_mul(pi.dyl,pi.scan_slope);
         dy_top=fix_div(pi.dyl,d);
         y_top=fix_div(fix_mul(pi.dxl,y_top)+fix_mul(pi.dyl,fix_ceil(xp_top)-x_top),d);
         if (yl_max<yl_min) {
            fix foo=yl_min;
            yl_min=yl_max;
            yl_max=foo;
         }
         if (fix_mul(vpl[n_top]->y - FIX_UNIT,pi.dxl) - fix_mul(vpl[n_top]->x - pi.scan_slope,pi.dyl)
               + pi.cl<0) {
            pi.dyl=-pi.dyl;
            pi.dxl=-pi.dxl;
            pi.cl=-pi.cl;
         }
         if (pi.scan_slope>0) pi.dtl=pi.dxl-pi.dyl;
         else pi.dtl=pi.dyl+pi.dxl;
         xp_top=x_prime[n_top];
      }

      /* check bot edge */
      if (fix_cint(x_prime[n_bot])<=pi.xp) {
         int n_prev,dxp;
         fix d;
         do {
            if (fix_cint(x_prime[n_bot])==pi.xp) n_prev=n_bot;
            if (--n_bot < 0) n_bot=n-1;
         } while (fix_cint(x_prime[n_bot])<=pi.xp);
         xp_bot=x_prime[n_prev];
         x_bot=vpl[n_prev]->x;
         y_bot=vpl[n_prev]->y;
         i_bot=vpl[n_prev]->i;
         dxp=fix_cint(x_prime[n_bot])-fix_cint(xp_bot);
         di_bot=(vpl[n_bot]->i-i_bot)/dxp;
         yr_min=fix_cint(y_bot);
         yr_max=fix_cint(vpl[n_bot]->y);
         pi.dxr=(vpl[n_bot]->x-x_bot)/pi.scale;
         pi.dyr=(vpl[n_bot]->y-y_bot)/pi.scale;
         pi.cr=-fix_mul(pi.dxr,y_bot)+fix_mul(pi.dyr,x_bot);
         d=pi.dxr-fix_mul(pi.dyr,pi.scan_slope);
         dy_bot=fix_div(pi.dyr,d);
         y_bot=fix_div(fix_mul(pi.dxr,y_bot)+fix_mul(pi.dyr,fix_ceil(xp_bot)-x_bot),d);
         if (yr_max<yr_min) {
            fix foo=yr_min;
            yr_min=yr_max;
            yr_max=foo;
         }
         if (fix_mul(vpl[n_bot]->y - FIX_UNIT,pi.dxr) - fix_mul(vpl[n_bot]->x - pi.scan_slope,pi.dyr)
               + pi.cr<0) {
            pi.dyr=-pi.dyr;
            pi.dxr=-pi.dxr;
            pi.cr=-pi.cr;
         }
         if (pi.scan_slope>0) pi.dtr=pi.dxr-pi.dyr;
         else pi.dtr=pi.dyr+pi.dxr;
         xp_bot=x_prime[n_bot];
      }
      xp_next = (xp_bot<xp_top) ? fix_cint(xp_bot):fix_cint(xp_top);

      /* do 0th scanline if at xp_min */
      if (pi.xp==xp_min) {
         y_top-=dy_top;
         y_bot-=dy_bot;
         pi.xp--;
      }
      for (;pi.xp<xp_next;pi.xp++) {
         if ((pi.xp+1==xp_max) && (n_top!=n_bot)) {
            pi.dxl=(vpl[n_bot]->x-vpl[n_top]->x)/pi.scale;
            pi.dyl=(vpl[n_bot]->y-vpl[n_top]->y)/pi.scale;
            pi.y=fix_cint(vpl[n_top]->y);
            pi.yl=fix_cint(vpl[n_bot]->y);
            if (pi.scan_slope>0) {
               y_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.right-1)-pi.xp,1),pi.scan_slope));
               y_min=fix_int(fix_div(fix_make((grd_int_clip.left-1)-pi.xp,1),pi.scan_slope))+1;
            }
            else {
               y_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.left-1)-pi.xp,1),pi.scan_slope));
               y_min=fix_int(fix_div(fix_make((grd_int_clip.right-1)-pi.xp,1),pi.scan_slope))+1;
            }
            if (pi.y<y_min) pi.y=y_min;
            if (pi.yl>y_max) pi.yl=y_max;
            pi.yr0=pi.yr=pi.yl;
            pi.i=i_top;
            if (pi.yr-pi.y) pi.di=(i_bot-pi.i)/(pi.yr-pi.y);
					  else {pi.di = 0x7fffffff; if (i_bot-pi.i < 0) pi.di = -pi.di;}

            pi.cl=-fix_mul(pi.dxl,vpl[n_top]->y)+fix_mul(pi.dyl,vpl[n_top]->x);
            if (pi.y*pi.dxl - fix_mul(fix_make(pi.xp+1,0) + pi.y*pi.scan_slope,pi.dyl) + pi.cl<0) {
               pi.dyl=-pi.dyl;
               pi.dxl=-pi.dxl;
               pi.cl=-pi.cl;
            }
            if (pi.scan_slope>0) pi.dtl=pi.dxl-pi.dyl;
            else pi.dtl=pi.dyl+pi.dxl;

            ((void (*)(grs_per_info *, grs_bitmap *))(ps->scanline_func))(&pi,bm);

            pi.xp=xp_max;
            break;
         }

         if (dy_top>0) {
            pi.y=fix_fint(y_top);
            pi.yl=fix_cint(y_top+dy_top);
         } else {
            pi.y=fix_fint(y_top+dy_top);
            pi.yl=fix_cint(y_top);
         }
         if (dy_bot>0) {
            pi.yr0=fix_fint(y_bot);
            pi.yr=fix_cint(y_bot+dy_bot);
         } else {
            pi.yr0=fix_fint(y_bot+dy_bot);
            pi.yr=fix_cint(y_bot);
         }
         if (pi.scan_slope>0) {
            y_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.right-1)-pi.xp,1),pi.scan_slope));
            y_min=fix_int(fix_div(fix_make((grd_int_clip.left-1)-pi.xp,1),pi.scan_slope))+1;
         }
         else {
            y_max=safe_fix_cint(fix_div(fix_make((grd_int_clip.left-1)-pi.xp,1),pi.scan_slope));
            y_min=fix_int(fix_div(fix_make((grd_int_clip.right-1)-pi.xp,1),pi.scan_slope))+1;
         }
         if (yl_min>y_min) y_min=yl_min;
         if (yr_max<y_max) y_max=yr_max;
         if (pi.yr>y_max) pi.yr=y_max;
         if (pi.yr0>y_max) pi.yr0=y_max;
         if (pi.yl>pi.yr0) pi.yl=pi.yr0;
         if (pi.y<y_min) pi.y=y_min;
         pi.i=i_top;
         if (pi.yr-pi.y) pi.di=(i_bot-pi.i)/(pi.yr-pi.y);
				 else {pi.di = 0x7fffffff; if (i_bot-pi.i < 0) pi.di = -pi.di;}

         ((void (*)(grs_per_info *, grs_bitmap *))(ps->scanline_func))(&pi,bm);

         if (pi.xp>=xp_min) {
            i_top+=di_top,i_bot+=di_bot;
         }

         y_top+=dy_top, y_bot+=dy_bot;
         pi.denom+=fix_16_20(ps->a),pi.unum+=ps->alpha_u,pi.vnum+=ps->alpha_v;
      }
   }
}
