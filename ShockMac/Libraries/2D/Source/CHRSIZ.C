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
 * $Source: n:/project/lib/src/2d/RCS/chrsiz.c $
 * $Revision: 1.5 $
 * $Author: lmfeeney $
 * $Date: 1994/06/15 01:19:53 $
 *
 * Character width and height calculator.
 *
 * This file is part of the 2d library.
 *
 * $Log: chrsiz.c $
 * Revision 1.5  1994/06/15  01:19:53  lmfeeney
 * support extended ascii (c > 127) w\ uchars, don't change fn i\f
 * 
 * Revision 1.4  1994/04/09  00:10:39  lmfeeney
 * routine takes grs_font* arguement, #define in str.h for compatibility
 * 
 * Revision 1.3  1993/10/19  02:57:50  kaboom
 * Replaced #include   new headers.
 * 
 * Revision 1.2  1993/10/08  01:15:02  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.1  1993/06/02  16:17:21  kaboom
 * Initial revision
 */

#include "grs.h"
#include "ctxmac.h"

/* returns the width in pixels of c in the specified font */

void gr_font_char_size (grs_font *f, char c, short *w, short *h)
{
   short *off_tab;            /* character offset table */
   short offset;              /* offset of current character */

   if ((uchar)c < f->min || (uchar)c > f->max) return;
   off_tab = f->off_tab;
   offset = off_tab[(uchar)c - f->min];
   if ((uchar)c < f->min || (uchar)c > f->max)
      *w = 0;
   else
      *w = off_tab[(uchar)c - f->min+1]-offset;
   *h = f->h;
}

