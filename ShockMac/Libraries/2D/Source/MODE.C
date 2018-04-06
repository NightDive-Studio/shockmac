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
 * $Source: n:/project/lib/src/2d/RCS/mode.c $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/10/08 01:16:09 $
 *
 * Mode information table.
 *
 * This file is part of the 2d library.
 *
 * $Log: mode.c $
 * Revision 1.2  1993/10/08  01:16:09  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.1  1993/04/29  18:54:40  kaboom
 * Initial revision
 * 
 */

#include "grs.h"
#include "grd.h"
#include "grdev.h"
#include "mode.h"
#include "cnvtab.h"
#include "bitmap.h"

grs_mode_info grd_mode_info[GRD_MODES] = {
   {  320,  200,  8 },
   {  320,  200,  8 },
   {  320,  400,  8 },
   {  320,  240,  8 },
   {  320,  480,  8 },
   {  640,  400,  8 },
   {  640,  480,  8 },
   {  800,  600,  8 },
   { 1024,  768,  8 },
   { 1280, 1024,  8 },
   {  320,  200, 24 },
   {  640,  480, 24 },
   {  800,  600, 24 },
   { 1024,  768, 24 },
   { 1280, 1024, 24 }
};

// code from SMODE.ASM
int gr_set_mode (int mode, int clear)
 {
	gr_set_screen_mode(mode, clear);		// try to set graphics mode.
	gr_init_device(&grd_info);					// try to initialize device if pointer isn't NULL
	
	// copy width & height values from info table to capability list
	grd_mode = mode;
	grd_mode_cap.w = grd_mode_info[mode].w;
	grd_mode_cap.h = grd_mode_info[mode].h;
	
	// store aspect into capability struct.
	grd_mode_cap.aspect = 0x010000; // fixed 1:1 aspect ratio on Mac
	
	grd_canvas_table = grd_canvas_table_list[BMT_DEVICE];
	grd_pixel_table = grd_canvas_table_list[BMT_DEVICE];
	
	return(0);
 }
 
 
 