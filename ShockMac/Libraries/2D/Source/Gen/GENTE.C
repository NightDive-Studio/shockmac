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
 * $Source: r:/prj/lib/src/2d/RCS/gente.c $
 * $Revision: 1.2 $
 * $Author: kevin $
 * $Date: 1994/08/16 13:03:24 $
 * 
 * texture mapping edge parameter calculation procedures.
 *
 * This file is part of the 2d library.
 *
 */

#include "fix.h"
#include "tmapint.h"
#include "gente.h"

//#include "polyint.h"
// MLA- had to put poly_do-x all on one line so the compiler wouldn't bitch at me
#define poly_do_x(_next,_prev,_d,_frac,_x0,_dx,_y_next) \
	do {	\
		_d = _next->y-_prev->y; \
		_x0 = _prev->x; \
		_y_next = fix_cint(_next->y); \
		_dx = fix_div(_next->x-_x0,_d); \
		_frac = fix_ceil(_prev->y)-_prev->y; \
		_x0 += fix_mul(_frac,_dx);} \
		while (0)

#define poly_do_y(_next,_prev,_d,_frac,_y0,_dy,_x_next) \
do {                                    \
   _d = _next->x-_prev->x;              \
   _y0 = _prev->y;                      \
   _x_next = fix_cint(_next->x);        \
   _dy = fix_div(_next->y-_y0,_d);      \
   _frac = fix_ceil(_prev->x)-_prev->x; \
   _y0 += fix_mul(_frac,_dy);           \
} while (0)

#define poly_do_t(_tf,_ti,_d,_frac,_t0,_dt) \
do {                           \
   _t0 = _ti;                  \
   _dt = fix_div(_tf-_t0, _d); \
   _t0 += fix_mul(_frac,_dt);  \
} while (0)

#define poly_do_tw(_tf,_ti,_d,_frac,_t0,_dt,_wf,_wi,_w0,_dw) \
do {                                                  \
   if (fix_abs(_tf -_ti)<FIX_UNIT/16) {               \
      _t0 = fix_mul(_ti,_w0);                         \
      _dt = fix_mul(_ti,_dw);                         \
      if (_ti-fix_floor(_ti)<FIX_UNIT/2) _t0++,_dt++; \
      else _t0--,_dt--;                               \
   } else {                                           \
      _t0 = fix_mul(_ti,_wi);                         \
      _dt = fix_div(fix_mul(_tf,_wf)-_t0,_d);         \
      _t0 += fix_mul(_dt,_frac);                      \
   }                                                  \
} while (0)

#include "tmapint.h"

void gri_x_edge
   (grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side)
{
   grs_tmap_edge *edge;
   fix d,dy;
   grs_vertex *prev,*next;

   if (side==1) edge=&(info->left);
   else edge=&(info->right);
   prev=*p_prev,next=*p;
   poly_do_x(next, prev, d, dy, edge->x, edge->dx, edge->y);
}

void gri_ix_edge
   (grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side)
{
   grs_tmap_edge *edge;
   fix d,frac;
   grs_vertex *prev,*next;

   if (side==1) edge=&(info->left);
   else edge=&(info->right);
   prev=*p_prev,next=*p;
   poly_do_x(next, prev, d, frac, edge->x, edge->dx, edge->y);
   poly_do_t(next->i, prev->i, d, frac, edge->i, edge->di);
}

void gri_rgbx_edge
   (grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side)
{
   grs_tmap_edge *edge;
   fix d,frac;
   grs_vertex *prev,*next;

   if (side==1) edge=&(info->left);
   else edge=&(info->right);
   prev=*p_prev,next=*p;
   poly_do_x(next, prev, d, frac, edge->x, edge->dx, edge->y);
   poly_do_t(next->u, prev->u, d, frac, edge->u, edge->du);
   poly_do_t(next->v, prev->v, d, frac, edge->v, edge->dv);
   poly_do_t(next->w, prev->w, d, frac, edge->i, edge->di);
}

// MLA #pragma off (unreferenced)
void gri_scale_edge /* for scaler; does both edges at once.*/
   (grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side)
{
   fix d,frac;
   grs_vertex *prev,*next;

   prev=*p_prev,next=*p;
   info->left.u=prev->u;
   info->left.x=prev->x;
   info->right.u=next->u;
   info->right.x=next->x;
   info->left.dx=info->right.dx=0;
   info->left.v=prev->v;
   info->left.y=info->right.y=fix_cint(next->y);
   frac=fix_ceil(prev->y)-prev->y;
   d=next->y-prev->y;
   info->left.dv=info->right.dv=fix_div(next->v-prev->v,d);
   info->left.v+=fix_mul(info->left.dv,frac);
}
// MLA#pragma on (unreferenced)

void gri_uvx_edge
   (grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side)
{
   grs_tmap_edge *edge;
   fix d,frac;
   grs_vertex *prev,*next;

   if (side==1) edge=&(info->left);
   else edge=&(info->right);
   prev=*p_prev,next=*p;
   poly_do_x(next, prev, d, frac, edge->x, edge->dx, edge->y);
   poly_do_t(next->u, prev->u, d, frac, edge->u, edge->du);
   poly_do_t(next->v, prev->v, d, frac, edge->v, edge->dv);
}

void gri_uvix_edge
   (grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side)
{
   grs_tmap_edge *edge;
   fix d,frac;
   grs_vertex *prev,*next;

   if (side==1) edge=&(info->left);
   else edge=&(info->right);
   prev=*p_prev,next=*p;
   poly_do_x(next, prev, d, frac, edge->x, edge->dx, edge->y);
   poly_do_t(next->u, prev->u, d, frac, edge->u, edge->du);
   poly_do_t(next->v, prev->v, d, frac, edge->v, edge->dv);
   poly_do_t(next->i, prev->i, d, frac, edge->i, edge->di);
}

void gri_uvwx_edge
   (grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side)
{
   grs_tmap_edge *edge;
   fix d,frac;
   grs_vertex *prev,*next;

   if (side==1) edge=&(info->left);
   else edge=&(info->right);
   prev=*p_prev,next=*p;
   poly_do_x(next, prev, d, frac, edge->x, edge->dx, edge->y);
   poly_do_tw(next->u, prev->u, d, frac, edge->u, edge->du,
      next->w, prev->w, info->w, info->dw);
   poly_do_tw(next->v, prev->v, d, frac, edge->v, edge->dv,
      next->w, prev->w, info->w, info->dw);
}

void gri_uviwx_edge
   (grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side)
{
   grs_tmap_edge *edge;
   fix d,frac;
   grs_vertex *prev,*next;

   if (side==1) edge=&(info->left);
   else edge=&(info->right);
   prev=*p_prev,next=*p;
   poly_do_x(next, prev, d, frac, edge->x, edge->dx, edge->y);
   poly_do_tw(next->u, prev->u, d, frac, edge->u, edge->du,
      next->w, prev->w, info->w, info->dw);
   poly_do_tw(next->v, prev->v, d, frac, edge->v, edge->dv,
      next->w, prev->w, info->w, info->dw);
   poly_do_tw(next->i, prev->i, d, frac, edge->i, edge->di,
      next->w, prev->w, info->w, info->dw);
}

void gri_uvwy_edge
   (grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side)
{
   grs_tmap_edge *edge;
   fix d,frac;
   grs_vertex *prev,*next;

   if (side==1) edge=&(info->top);
   else edge=&(info->bot);
   prev=*p_prev,next=*p;
   poly_do_y(next, prev, d, frac, edge->y, edge->dy, edge->x);
   poly_do_tw(next->u, prev->u, d, frac, edge->u, edge->du,
      next->w, prev->w, info->w, info->dw);
   poly_do_tw(next->v, prev->v, d, frac, edge->v, edge->dv,
      next->w, prev->w, info->w, info->dw);
}

void gri_uviwy_edge
   (grs_tmap_loop_info *info, grs_vertex **p, grs_vertex **p_prev, int side)
{
   grs_tmap_edge *edge;
   fix d,frac;
   grs_vertex *prev,*next;

   if (side==1) edge=&(info->top);
   else edge=&(info->bot);
   prev=*p_prev,next=*p;
   poly_do_y(next, prev, d, frac, edge->y, edge->dy, edge->x);
   poly_do_tw(next->u, prev->u, d, frac, edge->u, edge->du,
      next->w, prev->w, info->w, info->dw);
   poly_do_tw(next->v, prev->v, d, frac, edge->v, edge->dv,
      next->w, prev->w, info->w, info->dw);
   poly_do_tw(next->i, prev->i, d, frac, edge->i, edge->di,
      next->w, prev->w, info->w, info->dw);
}

