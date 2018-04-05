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
 * $Source: n:/project/lib/src/2d/RCS/sscrn.c $
 * $Revision: 1.1 $
 * $Author: kaboom $
 * $Date: 1993/11/03 12:48:41 $
 *
 * Routine to set active screen.
 *
 * This file is part of the 2d library.
 *
 * $Log: sscrn.c $
 * Revision 1.1  1993/11/03  12:48:41  kaboom
 * Initial revision
 * 
 */

#include "grs.h"
#include "canvas.h"
#include "cnvdat.h"
#include "scrdat.h"
#include "screen.h"

/* set current screen to s.  also default canvas to full screen. */
void gr_set_screen(grs_screen *s)
{
   grd_screen=s;
   grd_screen_canvas=s->c;
   grd_visible_canvas=s->c+1;
   gr_set_canvas(grd_screen_canvas);
}
