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
 * $Source: r:/prj/lib/src/2d/RCS/buffer.c $
 * $Revision: 1.5 $
 * $Author: kaboom $
 * $Date: 1994/08/01 21:56:43 $
 *
 * 2d temporary storage management routines.
 *
 * This file is part of the 2d library.
 */

#include "buffer.h"

#ifndef GR_TEMP_USE_MEMSTACK
#define GRD_TEMP_SIZE 16384
uchar grd_temp_buf[GRD_TEMP_SIZE];
uchar *grd_temp_p = grd_temp_buf;

void *gr_alloc_temp (int n)
{
   void *p;

   if (grd_temp_p+n+sizeof(int) >= grd_temp_buf+GRD_TEMP_SIZE)
      return NULL;
   *((int *)grd_temp_p) = n;
   p = grd_temp_p+sizeof (int);
   grd_temp_p += n+sizeof (int);

   return p;
}

void gr_free_temp (void *p)
{
   int n;

   n = *((int *)p-1);
   grd_temp_p -= n+sizeof (int);
}
#endif /* !GR_TEMP_USE_MEMSTACK */
