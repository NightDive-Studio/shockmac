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
 * $Source: r:/prj/lib/src/2d/RCS/gentm.c $
 * $Revision: 1.20 $
 * $Author: kevin $
 * $Date: 1994/09/16 03:58:06 $
 *
 * Horizontal and vertical scanning texture mapper dispatchers.
 *
 * This file is part of the 2d library.
 *
 */

#include "bitmap.h"
#include "buffer.h"
#include "clpcon.h"
#include "clpfcn.h"
#include "cnvdat.h"
#include "fill.h"
#include "grnull.h"
#include "ifcn.h"
#include "polyint.h"
#include "poly.h"
#include "scrmac.h"
#include <string.h>
#include "tmapint.h"
#include "tmaps.h"
#include "tmaptab.h"
#include "tmapfcn.h"
#include "lg.h"

typedef void (*tm_init_type)(grs_tmap_loop_info *, grs_vertex **);
typedef void (*edge_type)(grs_tmap_loop_info *, grs_vertex **, grs_vertex **, int);

void h_umap(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti)
{
   grs_vertex **p_left;          /* current left & right vertices */
   grs_vertex **p_right;
   ulong y_min, y_max;           /* min & max vertex y coords */
   fix w_min,w_max;
   int y,y_limit;                /* current screen coordinates */
   void (*tm_init)(grs_tmap_loop_info *, grs_vertex **);
   fix *old_w = NULL;
   grs_tmap_loop_info info;      /* values for inner loop routine */


   info.n=(bm->flags&BMF_TRANS) + ti->tmap_type + GRD_FUNCS*bm->type;
/*{
 Str255 s;
 NumToString(info.n,s);
 DebugStr(s);
}*/


   if (grd_gc.fill_type!=FILL_NORM)
      info.clut=(uchar *)grd_gc.fill_parm;
   else if (ti->flags&TMF_CLUT)
      if ((info.clut=ti->clut)==NULL)
         info.clut=gr_get_clut();

   tm_init = (tm_init_type) grd_tmap_init_table[info.n];

   info.left_edge_func=info.right_edge_func=info.loop_func=gr_null;

   /* start with degenerate min and max values. */
//   poly_find_yw_extrema(y_min,y_max,w_min,w_max,p_left,vpl,n);
// moved this code from the #define in PolyInt.h because the compiler couldn't handle it
do {                                      
   grs_vertex **pvp;                     
   int y;                                 
   p_left = vpl;                        
   y_min = fix_cint(vpl[0]->y);         
   y_max = fix_cint(vpl[0]->y);         
   w_min = vpl[0]->w;                   
   w_max = vpl[0]->w;                   
   for (pvp=vpl+1; pvp<vpl+n; ++pvp) {
      y=fix_cint((*pvp)->y);              
      if (y < y_min) {                    
         y_min = y;                       
         w_min = (*pvp)->w;              
         p_left = pvp;                   
      }                                   
      if (y > y_max) {                   
         y_max = y;                     
         w_max = (*pvp)->w;              
      }                                   
   }                                      
   if (y_min == y_max) return;          
} while(0);

   if (ti->flags&TMF_FLOOR) {
      grs_vertex **pvp=vpl;
      fix y0=(*p_left)->y;
      fix dw;
      /* fix w_min,w_max if they're too big.  Save old w's. */
      if (w_max>0x20000) {
         fix i;
         old_w=(fix *)gr_alloc_temp(n*sizeof(fix));
         do {
            w_max=w_max>>2;
            w_min=w_min>>2;
         } while (w_max>0x20000);
         for (i=0;i<n;i++) old_w[i]=vpl[i]->w;
      }
      /* fix w's so w=w_min+dw*(y-y0) for all vertices. */
      info.dw=dw=(w_max-w_min)/((long )(y_max-y_min));
      info.w=w_min+fix_mul(dw,fix_ceil(y0)-y0);
      for (; pvp<vpl+n; ++pvp)
         (*pvp)->w=w_min+fix_mul((*pvp)->y - y0,dw);
   }
   p_right=p_left;

   info.bm = *bm;	//  memcpy(&(info.bm),bm,sizeof(*bm));
   info.y=y_min;
   info.u_mask=(1<<bm->wlog)-1;
   info.v_mask=((1<<bm->hlog)-1)<<bm->wlog;
   info.vtab=NULL;

   /* we want to set n_left and n_right to be leftmost and rightmost vertices
      with y = y_min.  usually, both are n_min, but if there is a horizontal
      edge at y = y_min, they will be different. */

   /* draw each span, starting at y_min. */
   tm_init(&info,vpl);
   for (y=y_min; y!=y_max; ) {

      if (fix_cint((*p_left)->y)<=y) {
         fix y_left,y_prev;
         grs_vertex *prev;
         poly_do_left_edge(p_left,prev,y_left,y_prev,y,vpl,n);
         ((edge_type) info.left_edge_func) (&info,p_left,&prev,TMS_LEFT);
      }

      if (fix_cint((*p_right)->y)<=y) {
         fix y_right,y_prev;
         grs_vertex *prev;
         poly_do_right_edge(p_right,prev,y_right,y_prev,y,vpl,n);
         ((edge_type) info.right_edge_func) (&info,p_right,&prev,TMS_RIGHT);
      }
      y_limit=ulong_min(info.right.y,info.left.y);
      info.n=y_limit-y;

      if (((int (*)(grs_tmap_loop_info *))(info.loop_func))(&info)) break;
      y=y_limit;
   }
   if (info.vtab)
      gr_free_temp(info.vtab);
   if (old_w) {
      int i;
      for (i=0;i<n;i++) vpl[i]->w=old_w[i];
      gr_free_temp(old_w);
   }
}

int h_map(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti)
{
   grs_vertex **cpl;          /* clipped vertices */
   int m;                     /* number of clipped vertices */

   cpl = NULL;
   m = gr_clip_poly(n,4,vpl,&cpl);
   if (m>2)
      h_umap(bm,m,cpl,ti);
   gr_free_temp(cpl);

   return (m>2) ? CLIP_NONE : CLIP_ALL;
}

typedef void (*tm_init_type2)(grs_tmap_loop_info *);

void v_umap(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti)
{
   grs_vertex **p_top;              /* current top & bot vertices */
   grs_vertex **p_bot;
   ulong x_min, x_max;              /* min & max vertex x coords */
   int x,x_limit;                   /* current screen coordinates */
   fix w_min,w_max;
   fix *old_w = NULL;               /* list of old w values from vpl */
   grs_tmap_loop_info info;         /* values for inner loop routine */
   void (*tm_init)(grs_tmap_loop_info *);

   info.n=bm->flags&BMF_TRANS;
   if (info.n+2*grd_gc.fill_type==2*FILL_SOLID) {
      h_umap(bm,n,vpl,ti);
      return;
   }
   info.n+=ti->tmap_type+GRD_FUNCS*bm->type;

   if (grd_gc.fill_type!=FILL_NORM)
      info.clut=(uchar *)grd_gc.fill_parm;
   else if (ti->flags&TMF_CLUT)
      if ((info.clut=ti->clut)==NULL)
         info.clut=gr_get_clut();

   tm_init = (tm_init_type2) grd_tmap_init_table[info.n];
   info.top_edge_func=info.bot_edge_func=info.loop_func=gr_null;

   /* start with degenerate min and max values. */
   poly_find_xw_extrema(x_min,x_max,w_min,w_max,p_top,vpl,n);

   if (ti->flags&TMF_WALL) {
      grs_vertex **pvp=vpl;
      fix x0=(*p_top)->x;
      fix dw;
      /* fix w_min,w_max if they're too big.  Save old w's. */
      if (w_max>0x20000) {
         fix i;
         old_w=(fix *)gr_alloc_temp(n*sizeof(fix));
         do {
            w_max=w_max>>2;
            w_min=w_min>>2;
         } while (w_max>0x20000);
         for (i=0;i<n;i++) old_w[i]=vpl[i]->w;
      }
      /* fix w's so w=w0+dw*(x-x0) for all vertices. */
      info.dw=dw=(w_max-w_min)/((long )(x_max-x_min));
      info.w=w_min+fix_mul(dw,fix_ceil(x0)-x0);
      for (; pvp<vpl+n; ++pvp)
         (*pvp)->w=w_min+fix_mul((*pvp)->x - x0,dw);
   }
   p_bot=p_top;

   info.bm = *bm;	//  memcpy(&(info.bm),bm,sizeof(*bm));
   info.x=x_min;
   info.u_mask=(1<<bm->wlog)-1;
   info.v_mask=((1<<bm->hlog)-1)<<bm->wlog;

   info.vtab=NULL;
   /* we want to set n_top and n_bot to be topmost and botmost vertices
      with y = y_min.  usually, both are n_min, but if there is a horizontal
      edge at y = y_min, they will be different. */

   /* draw each span, starting at x_min. */
   tm_init(&info);
   for (x=x_min; x!=x_max; ) {

      if (fix_cint((*p_top)->x)<=x) {
         fix x_top,x_prev;
         grs_vertex *prev;
         poly_do_top_edge(p_top,prev,x_top,x_prev,x,vpl,n);
         ((edge_type) info.top_edge_func) (&info,p_top,&prev,TMS_LEFT);
      }

      if (fix_cint((*p_bot)->x)<=x) {
         fix x_bot,x_prev;
         grs_vertex *prev;
         poly_do_bot_edge(p_bot,prev,x_bot,x_prev,x,vpl,n);
         ((edge_type) info.bot_edge_func) (&info,p_bot,&prev,TMS_RIGHT);
      }
      x_limit=ulong_min(info.bot.x,info.top.x);
      info.n=x_limit-x;

      if (((int (*)(grs_tmap_loop_info *))(info.loop_func))(&info)) break;
      x=x_limit;
   }
   if (info.vtab)
      gr_free_temp(info.vtab);
   if (old_w) {
      int i;
      for (i=0;i<n;i++) vpl[i]->w=old_w[i];
      gr_free_temp(old_w);
   }
}

int v_map(grs_bitmap *bm, int n, grs_vertex **vpl, grs_tmap_info *ti)
{
   grs_vertex **cpl;          /* clipped vertices */
   int m;                     /* number of clipped vertices */

   cpl = NULL;
   m = gr_clip_poly(n,4,vpl,&cpl);
   if (m>2)
      v_umap(bm,m,cpl,ti);
   gr_free_temp(cpl);

   return (m>2) ? CLIP_NONE : CLIP_ALL;
}
