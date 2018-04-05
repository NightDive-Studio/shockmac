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
 * $Source: r:/prj/lib/src/2d/RCS/polyint.h $
 * $Revision: 1.8 $
 * $Author: kevin $
 * $Date: 1994/07/18 17:08:25 $
 *
 * Polygon scanning internal macros.
 *
*/

#ifndef __POLYINT_H
#define __POLYINT_H

#include "fix.h"
#include "grs.h"
#include "plytyp.h"

#define poly_find_y_extrema(_y_min,_y_max,_p_left,_vpl,_n) \
do {                                      \
   grs_vertex **pvp;                      \
   int __y;                               \
   _y_min = fix_cint(_vpl[0]->y);         \
   _y_max = fix_cint(_vpl[0]->y);         \
   _p_left = _vpl;                        \
   for (pvp=_vpl+1; pvp<_vpl+_n; ++pvp) { \
      __y=fix_cint((*pvp)->y);            \
      if (__y < _y_min) {                 \
         _y_min = __y;                    \
         _p_left = pvp;                   \
      }                                   \
      if (__y > _y_max)                   \
         _y_max = __y;                    \
   }                                      \
   if (_y_min == _y_max) return;          \
} while(0)
/*
#define poly_find_yw_extrema(_y_min,_y_max,_w_min,_w_max,_p_left,_vpl,_n) \
do {                                      \
   grs_vertex **pvp;                      \
   int y;                                 \
   _p_left = _vpl;                        \
   _y_min = fix_cint(_vpl[0]->y);         \
   _y_max = fix_cint(_vpl[0]->y);         \
   _w_min = _vpl[0]->w;                   \
   _w_max = _vpl[0]->w;                   \
   for (pvp=_vpl+1; pvp<_vpl+_n; ++pvp) { \
      y=fix_cint((*pvp)->y);              \
      if (y < _y_min) {                   \ 
         _y_min = y;                      \ 
         _w_min = (*pvp)->w;              \
         _p_left = pvp;                   \
      }                                   \
      if (y > _y_max) {                   \
         _y_max = y;                      \
         _w_max = (*pvp)->w;              \
      }                                   \
   }                                      \
   if (_y_min == _y_max) return;          \
} while(0)
*/
#define poly_find_x_extrema(_x_min,_x_max,_p_top,_vpl,_n) \
do {                                      \
   grs_vertex **pvp;                      \
   int __x;                               \
   _x_min = fix_cint(_vpl[0]->x);         \
   _x_max = fix_cint(_vpl[0]->x);         \
   _p_top = _vpl;                        \
   for (pvp=_vpl+1; pvp<_vpl+_n; ++pvp) { \
      __x=fix_cint((*pvp)->x);            \
      if (__x < _x_min) {                 \
         _x_min = __x;                    \
         _p_top = pvp;                   \
      }                                   \
      if (__x > _x_max)                   \
         _x_max = __x;                    \
   }                                      \
   if (_x_min == _x_max) return;          \
} while(0)

#define poly_find_xw_extrema(_x_min,_x_max,_w_min,_w_max,_p_top,_vpl,_n) \
do {                                      \
   grs_vertex **pvp;                      \
   int __x;                               \
   _x_min = fix_cint(_vpl[0]->x);         \
   _x_max = fix_cint(_vpl[0]->x);         \
   _w_min = _vpl[0]->w;                   \
   _w_max = _vpl[0]->w;                   \
   _p_top = _vpl;                         \
   for (pvp=_vpl+1; pvp<_vpl+_n; ++pvp) { \
      __x=fix_cint((*pvp)->x);            \
      if (__x < _x_min) {                 \
         _x_min = __x;                    \
         _w_min = (*pvp)->w;              \
         _p_top = pvp;                    \
      }                                   \
      if (__x > _x_max) {                 \
         _w_max = (*pvp)->w;              \
         _x_max = __x;                    \
      }                                   \
   }                                      \
   if (_x_min == _x_max) return;          \
} while(0)

#define poly_find_x_extrema_retval(_x_min,_x_max,_p_top,_vpl,_n,_retval) \
do {                                      \
   grs_vertex **pvp;                      \
   int __x;                               \
   _x_min = fix_cint(_vpl[0]->x);         \
   _x_max = fix_cint(_vpl[0]->x);         \
   _p_top = _vpl;                         \
   for (pvp=_vpl+1; pvp<_vpl+_n; ++pvp) { \
      __x=fix_cint((*pvp)->x);            \
      if (__x < _x_min) {                 \
         _x_min = __x;                    \
         _p_top = pvp;                    \
      }                                   \
      if (__x > _x_max)                   \
         _x_max = __x;                    \
   }                                      \
   if (_x_min == _x_max) return _retval;  \
} while(0)


#define poly_do_left_edge(_p_left,_prev,_y_left,_y_prev,_y,_vpl,_n) \
do {                                            \
   _y_left = (*_p_left)->y;                     \
   do {                                         \
      if (fix_cint(_y_left)==_y) _prev=*_p_left;\
      if (--_p_left < _vpl)                     \
         _p_left=_vpl+_n-1;                     \
      _y_prev=_y_left;                          \
      _y_left=(*_p_left)->y;                    \
   } while (fix_cint(_y_left)<=_y);             \
} while (0)

#define poly_do_top_edge(_p_top,_prev,_x_top,_x_prev,_x,_vpl,_n) \
do {                                          \
   _x_top = (*_p_top)->x;                     \
   do {                                       \
      if (fix_cint(_x_top)==_x) _prev=*_p_top;\
      if (++_p_top >= _vpl+n)                 \
         _p_top=_vpl;                         \
      _x_prev=_x_top;                         \
      _x_top=(*_p_top)->x;                    \
   } while (fix_cint(_x_top)<=_x);            \
} while (0)

#define poly_do_right_edge(_p_right,_prev,_y_right,_y_prev,_y,_vpl,_n) \
do {                                               \
   _y_right = (*_p_right)->y;                      \
   do {                                            \
      if (fix_cint(_y_right)==_y) _prev=*_p_right; \
      if (++_p_right >= _vpl+_n)                   \
         _p_right=_vpl;                            \
      _y_prev=_y_right;                            \
      _y_right=(*_p_right)->y;                     \
   } while (fix_cint(_y_right)<=_y);               \
} while (0)

#define poly_do_bot_edge(_p_bot,_prev,_x_bot,_x_prev,_x,_vpl,_n) \
do {                                          \
   _x_bot = (*_p_bot)->x;                     \
   do {                                       \
      if (fix_cint(_x_bot)==_x) _prev=*_p_bot;\
      if (--_p_bot < _vpl)                    \
         _p_bot=_vpl+_n-1;                    \
      _x_prev=_x_bot;                         \
      _x_bot=(*_p_bot)->x;                    \
   } while (fix_cint(_x_bot)<=_x);            \
} while (0)

#define poly_do_x(p_next,_prev,y_next,_y_prev,_d,x0,dx) \
do {                                            \
   _d = y_next-_y_prev;                         \
   x0 = _prev->x;                               \
   dx = fix_div((*p_next)->x-x0, _d);           \
   x0 += fix_mul(dx,fix_ceil(_y_prev)-_y_prev); \
} while (0)

#define poly_do_y_bot(p_next,_prev,x_next,_x_prev,_d,y0,dy) \
do {                                            \
   _d = x_next-_x_prev;                         \
   y0 = _prev->y;                               \
   dy = fix_div((*p_next)->y-y0, _d);           \
   y0 += fix_mul(dy,fix_ceil(_x_prev)-_x_prev); \
   if (dy>0) y0++;                              \
} while (0)

#define poly_do_y_top(p_next,_prev,x_next,_x_prev,_d,y0,dy) \
do {                                            \
   _d = x_next-_x_prev;                         \
   y0 = _prev->y;                               \
   dy = fix_div((*p_next)->y-y0, _d);           \
   y0 += fix_mul(dy,fix_ceil(_x_prev)-_x_prev); \
   if (dy>0) y0++;                              \
} while (0)

#define poly_do_uv(p_next,_prev,_d,u0,du,v0,dv) \
do {                                            \
   u0 = (_prev->u);                             \
   du = fix_div((*p_next)->u-u0, _d);           \
   u0 += fix_mul(du,fix_ceil(_prev->y)-_prev->y);\
   v0 = (_prev->v);                             \
   dv = fix_div((*p_next)->v-v0, _d);           \
   v0 += fix_mul(dv,fix_ceil(_prev->y)-_prev->y);\
} while (0)

#define poly_do_rgb(p_next,_prev,_d,r0,dr,g0,dg,b0,db) \
do {                                            \
   r0 = (_prev->u);                             \
   dr = fix_div((*p_next)->u-r0, _d);           \
   r0 += fix_mul(dr,fix_ceil(_prev->y)-_prev->y);\
   g0 = (_prev->v);                             \
   dg = fix_div((*p_next)->v-g0, _d);           \
   g0 += fix_mul(dg,fix_ceil(_prev->y)-_prev->y);\
   b0 = (_prev->w);                             \
   db = fix_div((*p_next)->w-b0, _d);           \
   b0 += fix_mul(db,fix_ceil(_prev->y)-_prev->y);\
} while (0)

#define poly_do_uvw(p_next,_prev,_d,u0,du,v0,dv,w0,dw) \
do {                                                          \
   w0 = _prev->w;                                             \
   dw = fix_div((*p_next)->w-w0, _d);                         \
   u0 = fix_mul(_prev->u,w0);                                 \
   du = fix_div(fix_mul((*p_next)->u,(*p_next)->w) - u0, _d); \
   u0 += fix_mul(du,fix_ceil(_prev->y)-_prev->y);              \
   v0 = fix_mul(_prev->v,w0);                                 \
   dv = fix_div(fix_mul((*p_next)->v,(*p_next)->w) - v0, _d); \
   v0 += fix_mul(dv,fix_ceil(_prev->y)-_prev->y);              \
} while (0)

#define poly_do_i(p_next,_prev,_d,i0,di) \
do {                                  \
   i0 = _prev->i;                     \
   di = fix_div((*p_next)->i-i0, _d); \
   i0 += fix_mul(di,fix_ceil(_prev->y)-_prev->y);\
} while (0)

#define poly_do_iw(p_next,_prev,_d,i0,di,w0) \
do {                                                         \
   i0 = fix_mul(_prev->i,w0);                                \
   di = fix_div(fix_mul((*p_next)->i,(*p_next)->w) - i0, _d);\
   i0 += fix_mul(di,fix_ceil(_prev->y)-_prev->y);\
} while (0)

#define poly_do_uv_vscan(p_next,_prev,_d,u0,du,v0,dv) \
do {                                            \
   u0 = (_prev->u);                             \
   du = fix_div((*p_next)->u-u0, _d);           \
   u0 += fix_mul(du,fix_ceil(_prev->x)-_prev->x);\
   v0 = (_prev->v);                             \
   dv = fix_div((*p_next)->v-v0, _d);           \
   v0 += fix_mul(dv,fix_ceil(_prev->x)-_prev->x);\
} while (0)

#define poly_do_rgb_vscan(p_next,_prev,_d,r0,dr,g0,dg,b0,db) \
do {                                            \
   r0 = (_prev->u);                             \
   dr = fix_div((*p_next)->u-r0, _d);           \
   r0 += fix_mul(dr,fix_ceil(_prev->x)-_prev->x);\
   g0 = (_prev->v);                             \
   dg = fix_div((*p_next)->v-g0, _d);           \
   g0 += fix_mul(dg,fix_ceil(_prev->x)-_prev->x);\
   b0 = (_prev->w);                             \
   db = fix_div((*p_next)->w-b0, _d);           \
   b0 += fix_mul(db,fix_ceil(_prev->x)-_prev->x);\
} while (0)

#define poly_do_uvw_vscan(p_next,_prev,_d,u0,du,v0,dv,w0,dw) \
do {                                                          \
   w0 = _prev->w;                                             \
   dw = fix_div((*p_next)->w-w0, _d);                         \
   u0 = fix_mul(_prev->u,w0);                                 \
   du = fix_div(fix_mul((*p_next)->u,(*p_next)->w) - u0, _d); \
   u0 += fix_mul(du,fix_ceil(_prev->x)-_prev->x);              \
   v0 = fix_mul(_prev->v,w0);                                 \
   dv = fix_div(fix_mul((*p_next)->v,(*p_next)->w) - v0, _d); \
   v0 += fix_mul(dv,fix_ceil(_prev->x)-_prev->x);              \
} while (0)

#define poly_do_i_vscan(p_next,_prev,_d,i0,di) \
do {                                  \
   i0 = _prev->i;                     \
   di = fix_div((*p_next)->i-i0, _d); \
   i0 += fix_mul(di,fix_ceil(_prev->x)-_prev->x);\
} while (0)

#define poly_do_iw_vscan(p_next,_prev,_d,i0,di,w0) \
do {                                                         \
   i0 = fix_mul(_prev->i,w0);                                \
   di = fix_div(fix_mul((*p_next)->i,(*p_next)->w) - i0, _d);\
   i0 += fix_mul(di,fix_ceil(_prev->x)-_prev->x);\
} while (0)
#endif /* !__POLYINT_H */


