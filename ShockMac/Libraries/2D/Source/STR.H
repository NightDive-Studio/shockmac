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
 * $Source: n:/project/lib/src/2d/RCS/str.h $
 * $Revision: 1.5 $
 * $Author: lmfeeney $
 * $Date: 1994/04/09 00:31:21 $
 *
 * Prototypes for non-table string functions.
 *
 * This file is part of the 2d library.
 *
 * $Log: str.h $
 * Revision 1.5  1994/04/09  00:31:21  lmfeeney
 * added new height and wrapping routines, added #defines
 * for compatibility with string and char routines taking
 * grs_font * as first arg
 * 
 * Revision 1.4  1993/06/02  21:29:21  kaboom
 * Moved n argument for gr_string_nwidth from last to second.
 * 
 * Revision 1.3  1993/06/02  16:33:42  kaboom
 * Added prototypes for various new size & n-char routines.
 * 
 * Revision 1.2  1993/04/08  18:57:12  kaboom
 * Added prototypes for character functions.
 * 
 * Revision 1.1  1993/04/08  16:28:06  kaboom
 * Initial revision
 */

/* prototypes for non-table driven string handling routines. */
extern void gr_font_string_size (grs_font *f, char *s, short *w, short *h);
extern void gr_font_string_nsize (grs_font *f, char *s, int n, short *w, short *h);
extern short gr_font_string_width (grs_font *f, char *s);
extern short gr_font_string_nwidth (grs_font *f, char *s, int n);
extern short gr_font_string_height (grs_font *f, char *s);
extern short gr_font_string_nheight (grs_font *f, char *s, int n);
extern void gr_font_string_uclear (grs_font *f, char *s, short x, short y);
extern int gr_font_string_clear (grs_font *f, char *s, short x, short y);
extern short gr_font_char_width (grs_font *f, char c);
extern void gr_font_char_size (grs_font *f, char c, short *w, short *h);
extern void gr_font_char_uclear (grs_font *f, char c, short x, short y);
extern int gr_font_char_clear (grs_font *f, char c, short x, short y);
extern int gr_font_string_wrap (grs_font *pfont, char *ps, short width);
extern void gr_font_string_unwrap (char *s);

/* use the current font */


#define gr_string_size(s, w, h) \
   gr_font_string_size ((grs_font *) gr_get_font(), s, w, h)

#define gr_string_nsize(s, n, w, h) \
   gr_font_string_nsize ((grs_font *) gr_get_font(), s, n, w, h)

#define gr_string_width(s) \
   gr_font_string_width ((grs_font *)gr_get_font(), s)

#define gr_string_nwidth(s, n) \
   gr_font_string_nwidth ((grs_font *) gr_get_font(), s, n)

#define gr_string_height(s) \
   gr_font_string_height ((grs_font *)gr_get_font(), s)

#define gr_string_nheight(s, n) \
   gr_font_string_nheight ((grs_font *) gr_get_font(), s, n)

#define gr_string_uclear(s, x, y) \
   gr_font_string_uclear ((grs_font *) gr_get_font, s, x, y)

#define gr_string_clear(s, x, y) \
    gr_font_string_clear ((grs_font *) gr_get_font(), s, x, y)

#define gr_char_width(c) \
    gr_font_char_width ((grs_font *) gr_get_font(), c)

#define gr_char_size(c, w, h) \
    gr_font_char_size ((grs_font *) gr_get_font(), c, w, h)

#define gr_char_uclear(c, x, y) \
    gr_font_char_uclear ((grs_font *) gr_get_font(), c, x, y)

#define gr_char_clear(c, x, y) \
    gr_font_char_clear ((grs_font *) gr_get_font(), c, x, y)

