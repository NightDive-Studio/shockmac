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
 * $Source: n:/project/lib/src/2d/RCS/context.c $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1993/10/08 01:15:06 $
 *
 * Macros for access to table-driver 2d functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: context.c $
 * Revision 1.2  1993/10/08  01:15:06  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.1  1993/02/04  17:09:34  kaboom
 * Initial revision
 * 
 */

#include "grs.h"

/* Default graphic context; gets copied by init_gc. */
grs_context grd_defgc = {
   15,               /* current drawing color */
   0,	               /* background color */
   0,                /* font id */
   0,                /* attributes for text */
   0,                /* how to fill primitives */
   0,                /* parameter for fill */
   /* clipping region. */
   { NULL, 0, 0, 0, 0 }
};
