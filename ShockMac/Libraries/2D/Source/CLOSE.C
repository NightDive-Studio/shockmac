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
 * $Source: r:/prj/lib/src/2d/RCS/close.c $
 * $Revision: 1.6 $
 * $Author: kevin $
 * $Date: 1994/10/18 13:22:36 $
 *
 * Routine to shut down 2d system.
 *
 * This file is part of the 2d library.
 *
 * $Log: close.c $
 * Revision 1.6  1994/10/18  13:22:36  kevin
 * renamed gr_{push,pop}_state to gr_{push,pop}_video_state.
 * 
 * Revision 1.5  1993/10/19  10:12:16  kaboom
 * Removed null function pointer check---macro now does it
 * 
 * Revision 1.4  1993/10/08  01:15:05  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.3  1993/07/12  23:29:15  kaboom
 * Now gr_close() can be called before gr_init() without ill effect.
 * 
 * Revision 1.2  1993/05/16  00:31:42  kaboom
 * Now calls gr_pop_state() to restore old video state.
 * 
 * Revision 1.1  1993/04/29  16:30:38  kaboom
 * Initial revision
 */

#include "grd.h"
#include "grdev.h"
#include "state.h"
#include "status.h"
#include "close.h"

/* shut down 2d system.  call device-dependent shutdown routine and
   restore video state. */
int gr_close(void)
{
   if (grd_active == 0)
      return 0;
   gr_pop_video_state (TRUE);
   gr_close_device (&grd_info);
   grd_active = 0;
   return 0;
}
