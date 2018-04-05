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
 * $Source: n:/project/lib/src/2d/RCS/strwid.c $
 * $Revision: 1.6 $
 * $Author: lmfeeney $
 * $Date: 1994/06/15 01:17:20 $
 *
 * String width calculator.
 *
 * This file is part of the 2d library.
 *
 * $Log: strwid.c $
 * Revision 1.6  1994/06/15  01:17:20  lmfeeney
 * support extended ascii (c > 127) w\ uchars, don't change fn i\f
 * 
 * Revision 1.5  1994/04/09  07:42:32  lmfeeney
 * added grs_font * as first argument, #define for compatibility
 * 
 * Revision 1.4  1993/10/19  09:58:01  kaboom
 * Replaced #include   new headers.
 * 
 * Revision 1.3  1993/10/08  01:16:29  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.2  1993/06/02  16:25:25  kaboom
 * Nows handles strings with hard and soft carriage returns as well
 * as soft spaces.
 * 
 * Revision 1.1  1993/04/08  16:26:47  kaboom
 * Initial revision
 */

#include "chr.h"
#include "ctxmac.h"
#include "str.h"

/* returns the width of string s in pixels for the specified font */

short gr_font_string_width (grs_font *f, char *s)
{
   short *offset_tab;         /* table of character offsets */
   short offset;              /* offset of current character */
   short w_lin=0;             /* current line's width so far */
   short w=0;                 /* width of widest line */
   uchar c;                    /* current character */

   offset_tab = f->off_tab;
   while ((c= (uchar) (*s++)) != '\0') {
      if (c == CHAR_SOFTSP)
         continue;
      if (c=='\n' || c==CHAR_SOFTCR) {
         if (w_lin>w) w=w_lin;
         w_lin = 0;
         continue;
      }
      offset = offset_tab[c-f->min];
      w_lin += offset_tab[c-f->min+1]-offset;
   }
   return (w_lin>w) ? w_lin : w;

   }
