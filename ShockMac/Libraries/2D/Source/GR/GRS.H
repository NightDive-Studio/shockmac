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
 * $Source: n:/project/lib/src/2d/RCS/grs.h $
 * $Revision: 1.13 $
 * $Author: baf $
 * $Date: 1993/12/16 00:30:50 $
 *
 * Public 2D system data structures.
 *
 * This file is part of the 2d library.
 *
 * $Log: grs.h $
 * Revision 1.13  1993/12/16  00:30:50  baf
 * Removed include of spntyp.h
 * 
 * Revision 1.12  1993/12/14  22:36:29  kevin
 * Took out perspective mapper context structure.  Sorry
 *  it should never have been
 * here in the first place.
 * 
 * Revision 1.11  1993/12/04  12:42:30  kevin
 * Added context structure for perspective mappers.
 * 
 * Revision 1.10  1993/10/19  10:22:49  kaboom
 * Font member of grs_context is now a grs_font *.
 * 
 * Revision 1.9  1993/10/15  18:04:45  baf
 * Moved transtab to screen
 * 
 * Revision 1.8  1993/10/15  12:28:16  baf
 * Added transluceny table to graphics context
 * 
 * Revision 1.7  1993/10/06  16:08:56  baf
 * Moved grs_span definition to spntyp.h
 * 
 * Revision 1.6  1993/10/06  13:33:57  kevin
 * Added default color lookup table to screen type.
 * 
 * Revision 1.5  1993/07/08  23:02:53  kaboom
 * Added wlog and hlog fields to grs_bitmap structure.
 * 
 * Revision 1.4  1993/06/01  13:51:46  kaboom
 * Added lighting table to screen structure.
 * 
 * Revision 1.3  1993/04/29  18:37:08  kaboom
 * Added system info structures.  Pared down driver capability structure.
 * 
 * Revision 1.1  1993/02/04  17:36:18  kaboom
 * Initial revision
 * 
 ********************************************************************
 * Log entries from old 2d.h
 * Revision 1.20  1993/01/25  11:09:57  matt
 * Removed structure definition for 'vector' (leaving in 'grs_vector'),
 * since vector conflicted with the 3d structure of the same name.
 * 
 * Revision 1.19  1993/01/22  19:49:02  kaboom
 * Added some structures for texture mapper vertices and palette.
 * 
 * Revision 1.18  1993/01/15  21:41:52  kaboom
 * Changed grs_stencil to grs_sten_elem and made new grs_stencil that
 * includes a flags word.
 * 
 * Revision 1.12  1992/12/30  14:59:53  kaboom
 * Changed from using grs_span for stencil elements to new grs_stencil
 * type.
 *
 * Revision 1.8  1992/12/10  11:11:31  kaboom
 * Moved ytab field from bitmap structure to canvas.  Changed clipping
 * regions from 2 structures, which are used as casts, to a union with
 * fixed and int fields.
 *
 * Revision 1.6  1992/11/12  13:54:09  kaboom
 * Added type, align and ytab fields to bitmap structure.  Reduced
 * bitmap flags to a short.  Changed clipping region to store fixed-
 * point values, and added different structures to allow their access
 * as fixed-point or integer values without shifting.  
 *
 * Revision 1.5  1992/10/21  15:58:48  kaboom
 * Changed naming for gr_xxx structures to grs_xxx to avoid some name
 * collisions with functions.
 * 
 * Revision 1.3  1992/10/09  16:50:55  kaboom
 * Added gr_ylrpp structure for scanline pixel-based drawing.
 */

#ifndef __GRS_H
#define __GRS_H

#include "fix.h"

/* system information structure. */
typedef struct {
   uchar id_maj;     /* major id---type of graphics system */
   uchar id_min;     /* minor id---vendor */
   short memory;     /* memory in kilobytes */
   short modes[16];  /* array of modes, ends with -1 */
} grs_sys_info;

/* mode information descriptor structure. */
typedef struct {
   short w;          /* screen width */
   short h;          /* screen height */
   uchar b;          /* number of bits per pixel */
} grs_mode_info;

/* amazing rgb type. */
typedef ulong grs_rgb;

/* structure for bitmaps to be drawn from and to.  if a bitmap is contained
   within a larger bitmap, the row field tells how wide the containing bitmap
   is. */
typedef struct {
   uchar *bits;      /* ptr to data */
   uchar type;       /* type of data in bitmap, 1-bit, 8-bit, etc */
   uchar align;      /* where data really starts */
   ushort flags;     /* whether compressed, transparent, etc */
   short w;          /* width in pixels */
   short h;          /* height */
   ushort row;       /* bytes in row of containing bitmap */
   uchar wlog;       /* log2 of w */
   uchar hlog;       /* log2 of h */
} grs_bitmap;

/* stencil element for non-rectangular clipping. */
typedef struct _sten {
   short l;          /* left edge of stencil */
   short r;          /* right */
   struct _sten *n;  /* pointer to next span in this scanline */
} grs_sten_elem;

/* stencil header for non-rectangular clipping. */
typedef struct {
   grs_sten_elem *elem;    /* pointer to first stencil element */
   long flags;             /* specific stencil data. */
} grs_stencil;

/* structure for clipping regions.  a clipping region can either be a simple
   rectangle (given by left,top,right,bot) or a grs_stencil, pointed to by
   sten. */
typedef union {
   struct {
      grs_stencil *sten;  /* pointer to stencil for nonrect clip region */
      fix left;            /* current clipping rectangle */
      fix top;             /* fixed-point coordinates */
      fix right;
      fix bot;
   } f;
   struct {
      grs_stencil *sten;  /* pointer to stencil for nonrect clip region */
      short left;          /* current clipping rectangle */
      short pad0;
      short top;           /* integral coordinates */
      short pad1;
      short right;
      short pad2;
      short bot;
      short pad3;
   } i;
} grs_clip;

typedef struct {
	ushort id;
	char dummy1[34];
	short min;
	short max;
	char dummy2[32];
	long cotptr;
	long buf;
	short w;
	short h;
	short off_tab[1];
} grs_font;

/* structure for drawing context.  the context contains data about which
   color, font attributes, filling attributes, and an embedded clipping
   region structure. */
typedef struct {
   long fcolor;      /* current drawing color */
   long bcolor;      /* background color */
   grs_font *font;   /* font id */
   long text_attr;   /* attributes for text */
   long fill_type;   /* how to fill primitives */
   long fill_parm;   /* parameter for fill */
   grs_clip clip;    /* clipping region */
} grs_context;

/* a canvas is a bitmap   drawing context. */
typedef struct {
   grs_bitmap  bm;   /* bitmap to draw into/read out of */
   grs_context gc;   /* graphic context */
   uchar **ytab;     /* pointer to an optional y table */
} grs_canvas;

/* a screen is a descriptor for a visible region of video memory. */
typedef struct {
   grs_bitmap bm;    /* where we actually draw */
   grs_canvas *c;    /* pointer to 2 system canvases */
   uchar *pal;
   grs_rgb *bpal;
   uchar *ipal;
   uchar *ltab;
   uchar ***transtab;/* table of colors under translucency */
   uchar *clut;      /* default color lookup table */
   short x;          /* upper left coordinates of visible */
   short y;          /* region of virtual buffer */
} grs_screen;

/* driver capability/info structure. */
typedef struct {
   fix aspect;       /* fixed point aspect ratio w/h */
   short w;          /* screen width */
   short h;          /* screen height */
   uchar *vbase;     /* base video address */
} grs_drvcap;

/* 3d point structure for perspective mapper. */
typedef struct {
   fix x,y,z;    /* 3's */
} grs_point3d;

#endif /* !__GRS_H */
