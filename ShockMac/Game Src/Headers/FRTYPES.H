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
#ifndef __FRTYPES_H
#define __FRTYPES_H
/*
 * $Source: n:/project/cit/src/inc/RCS/frtypes.h $
 * $Revision: 1.8 $
 * $Author: kevin $
 * $Date: 1994/04/14 15:01:10 $
 *
 * Citadel renderer
 *  actual type definitions and such
 */

#include "frcamera.h"

// structures
typedef struct {
   grs_canvas draw_canvas, main_canvas, hack_canvas;
   bool       double_buffer;
   int        xtop, ytop;
   int        xwid, ywid;              /* ywid is, of course, height */
   fix        viewer_zoom;
   int        fov;
   char       axis;
   int        flags, detail, last_detail;
   int      (*draw_call)(void *dest_canvas, void *dest_bm, int x, int y, int flags);
   void     (*horizon_call)(void *dest_bm, int flags);
   void     (*render_call)(void *dest_bm, int flags);
   cams      *camptr, *xtracam;
   Ptr		  realCanvasPtr;
} fauxrend_context;

#endif // __FRTYPES_H
