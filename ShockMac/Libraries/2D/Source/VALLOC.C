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
 * $Source: n:/project/lib/src/2d/RCS/valloc.c $
 * $Revision: 1.7 $
 * $Author: kaboom $
 * $Date: 1993/10/08 01:16:32 $
 *
 * Video memory management routines.
 *
 * This file is part of the 2d library.
 *
 * $Log: valloc.c $
 * Revision 1.7  1993/10/08  01:16:32  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.6  1993/05/03  15:08:52  kaboom
 * Removed extraneous #include lines.
 * 
 * Revision 1.5  1993/04/30  17:52:13  kaboom
 * Added grd_valloc_mode.  Pared down vblock structure.
 * 
 * Revision 1.4  1993/02/04  17:47:14  kaboom
 * Changed includes.
 * 
 * Revision 1.3  1993/01/07  21:12:50  kaboom
 * Put in SVGA hack which should be fixed.
 */

#include "grs.h"
#include "grd.h"
#include "valloc.h"

// MLA- took the v_table out, it doesn't appear to be referenced anymore
#if 0
#define VTAB_SIZE 100

typedef struct {
   uchar *p;        /* pointer to block. */
   long  size;      /* size of block (address). */
} v_block;

v_block v_table[VTAB_SIZE];
#endif

// globals
uchar grd_valloc_mode = 0;

uchar *valloc (short w, short h)
{
   if (grd_valloc_mode)
      return (uchar *)0;
   else
      return grd_cap->vbase;
}

void vfree (uchar *p)
{
}
