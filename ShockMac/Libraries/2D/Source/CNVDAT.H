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
 * $Source: n:/project/lib/src/2d/RCS/cnvdat.h $
 * $Revision: 1.1 $
 * $Author: kaboom $
 * $Date: 1993/10/19 10:12:33 $
 *
 * Declarations for current canvas and related globals.
 *
 * This file is part of the 2d library.
 *
 * $Log: cnvdat.h $
 * Revision 1.1  1993/10/19  10:12:33  kaboom
 * Initial revision
 * 
 */

#ifndef __CNVDAT_H
#define __CNVDAT_H
#include "grs.h"

extern grs_canvas *grd_screen_canvas;
extern grs_canvas *grd_visible_canvas;
extern grs_canvas *grd_canvas;

#define grd_bm (grd_canvas->bm)
#define grd_gc (grd_canvas->gc)
#define grd_ytab (grd_canvas->ytab)
#define grd_int_clip (grd_gc.clip.i)
#define grd_fix_clip (grd_gc.clip.f)
#define grd_clip (grd_int_clip)
#endif /* !__CNVDAT_H */
