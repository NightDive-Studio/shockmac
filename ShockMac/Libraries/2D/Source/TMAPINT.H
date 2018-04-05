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
 * $Source: r:/prj/lib/src/2d/RCS/tmapint.h $
 * $Revision: 1.10 $
 * $Author: kevin $
 * $Date: 1994/08/24 18:45:56 $
 *
 * texture mapping internal data structures.
 *
 * This file is part of the 2d library.
 *
 * $Log: tmapint.h $
 * Revision 1.10  1994/08/24  18:45:56  kevin
 * Added scanline func field.
 * 
 * Revision 1.9  1994/07/26  00:22:23  kevin
 * imbedded entire grs_bitmap structure into grs_tmap_loop_info.
 * 
 * Revision 1.8  1994/07/18  17:09:59  kevin
 * Eliminated per_setup structure (superceded by grs_per_setup in pertyp.h).
 * Changed tmap_edge_info and tmap_loop_info structures to reduce size.
 * Added new aliases in same.
 * 
 * Revision 1.7  1994/06/17  10:39:15  kevin
 * Changed redefinitions of fix_ceil and fix_cint so that they don't break.
 * 
 * Revision 1.6  1994/06/03  20:33:27  kevin
 * Added l3d pointer to per_setup struct so it can be freed in the right order.
 * 
 * Revision 1.5  1994/02/26  22:45:41  kevin
 * made p_src_off signed to enable proper storage of negative values for v.
 * 
 * Revision 1.4  1994/02/09  23:27:16  kevin
 * Changed grs_loop_info structure for use with new wacky edges.
 * 
 * Revision 1.3  1994/01/18  13:11:26  kevin
 * Added optimized ulong_min,max pragmas.  Also sides for edge calculations.
 * 
 * Revision 1.2  1994/01/13  12:21:10  kevin
 * changed tmap_inner_loop prototype to take flags parameter.
 * 
 * Revision 1.1  1994/01/03  22:03:06  kevin
 * Initial revision
 * 
 * 
*/

#ifndef __TMAPINT_H
#define __TMAPINT_H

// I GIVE UP! I'm going to define the offsets for all the stupid grs_tmap_loop_info entries!!!!!! - MLA
#define T_X 0x04
#define T_Y 0x04
#define HLog 0x17
#define T_W 0x18
#define T_DW 0x64

#define LeftX 0x1c
#define LeftY 0x20
#define LeftU 0x24
#define LeftV 0x28
#define LeftI 0x2C
#define LeftDX 0x30
#define LeftDY 0x30
#define LeftDU 0x34
#define LeftDV 0x38
#define LeftDI 0x3C

#define RightX 0x40
#define RightY 0x44
#define RightU 0x48
#define RightV 0x4C
#define RightI 0x50
#define RightDX 0x54
#define RightDY 0x54
#define RightDU 0x58
#define RightDV 0x5C
#define RightDI 0x60

#include "fix.h"
#include "grs.h"
#include "plytyp.h"

typedef struct {
   fix x,y,u,v,i;
   union {fix dx,dy;};
   fix du,dv,di;
} grs_tmap_edge;

typedef struct {
   int n;                     /* number of lines */
   union {                    /* destination pointer/scanline coord */
      uchar *d;
      int x,y;
   };
   union {
      grs_bitmap bm;
      struct {
         uchar *s;                                        /* bitmap bits pointer */
         uchar bm_type,bm_align;
         short bm_flags,bm_w,bm_h,bm_row;                 /* bitmap width & height */
         uchar wlog;
         union {uchar hlog,loop;};
      };
   };
   fix w;
   /* edge data */
   union {grs_tmap_edge left,top;};
   union {grs_tmap_edge right,bot;};
   fix dw;
   ulong u_mask;
   union {ulong v_mask,mask;};
   uchar *clut;               /* color lookup table */
   long *vtab;                /* for non power of 2 widths */
   void (*scanline_func)();   /* function for individual scanline */
   void (*loop_func)();       /* actually, chunk function */
   union {void (*left_edge_func)(), (*top_edge_func)();};
   union {void (*right_edge_func)(),(*bot_edge_func)();};
} grs_tmap_loop_info;

#define TMS_RIGHT 0
#define TMS_LEFT 1
#define TMS_BOT 0
#define TMS_TOP 1

#define sgn(a) (((a)>0) ? 1 : ((a)<0) ? -1 : 0)

#define ulong_min(x, y)	((((ulong) x) < ((ulong) y)) ? (x) : (y))
#define ulong_max(x, y)	((((ulong) x) < ((ulong) y)) ? (y) : (x))

/*ulong ulong_min (ulong a, ulong b);
#pragma aux ulong_min =   \
   "sub     edx,eax"    \
   "sbb     ebx,ebx"    \
   "and     ebx,edx"    \
   "add     eax,ebx"    \
   parm [eax] [edx]     \
   modify [eax ebx edx];

ulong ulong_max (ulong a, ulong b);
#pragma aux ulong_max =   \
   "sub     edx,eax"    \
   "sbb     ebx,ebx"    \
   "not     ebx"        \
   "and     ebx,edx"    \
   "add     eax,ebx"    \
   parm [eax] [edx]     \
   modify [eax ebx edx];
*/

#undef fix_ceil
#define fix_ceil(a) ((fix)(((a)+0xffff)&0xffff0000))
#undef fix_cint
#define fix_cint(a) ((short)(((a)+0xffff)>>16))

#define fix_light(i) ((i>>8)&0xff00)

#endif /* !__TMAPINT_H */


