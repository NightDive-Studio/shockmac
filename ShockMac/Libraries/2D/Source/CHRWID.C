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
 * $Source: n:/project/lib/src/2d/RCS/chrwid.c $
 * $Revision: 1.6 $
 * $Author: lmfeeney $
 * $Date: 1994/06/15 01:20:01 $
 *
 * Character width calculator.
 *
 * This file is part of the 2d library.
 *
 * $Log: chrwid.c $
 * Revision 1.6  1994/06/15  01:20:01  lmfeeney
 * support extended ascii (c > 127) w\ uchars, don't change fn i\f
 * 
 * Revision 1.5  1994/04/09  00:11:41  lmfeeney
 * takes grs_font* as first arg, #define in str.h for compatibility
 * 
 * Revision 1.4  1993/10/19  02:57:51  kaboom
 * Replaced #include   new headers.
 * 
 * Revision 1.3  1993/10/08  01:15:04  kaboom
 * Changed quotes in #include liness to angle brackets for Watcom problem.
 * 
 * Revision 1.2  1993/06/02  16:17:01  kaboom
 * Added range check for character.
 * 
 * Revision 1.1  1993/04/08  18:54:28  kaboom
 * Initial revision
 */

#include "grs.h"
#include "ctxmac.h"
#include "str.h"

/* returns the width in pixels of c in the specified font. */
short gr_font_char_width (grs_font *f, char c)
{
   short *off_tab;            /* character offset table */
   short offset;              /* offset of current character */

   if ((uchar)c < f->min || (uchar)c > f->max) return 0;
   off_tab = f->off_tab;
   offset = off_tab[(uchar)c - f->min];
   return off_tab[(uchar)c - f->min+1]-offset;
}
