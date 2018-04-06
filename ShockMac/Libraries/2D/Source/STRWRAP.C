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


/* $Source: r:/prj/lib/src/2d/RCS/strwrap.c $
 * $Revision: 1.3 $
 * $Author: lmfeeney $
 * $Date: 1994/08/19 02:39:22 $
 */

/* wrapping and unwrapping routines moved into the 2d library, other 
   routines already present in 2d
*/

//		Font.C	Font-handling routines
//		Rex E. Bradford (REX)
//
//		This module provides routines for accessing fonts
//		and for calculating the area needed to display
//		text in a font, including automatic wrapping.


/* log from /project/ff/code/gfx/font.c 

 * Revision 1.6  1993/11/18  11:15:50  rex
 * Changed Font* to grs_font*
 * 
 * Revision 1.5  1993/11/04  21:37:40  kaboom
 * Changed quotes to angle brackets in includes for watcom
 * 
 * Revision 1.4  1993/05/04  13:47:45  rex
 * Fixed return value in FontWrapText()
 * 
 * Revision 1.3  1993/04/22  19:37:14  rex
 * Removed FontSetFont, added pfont param to other funcs
 * 
 * Revision 1.2  1993/02/04  12:24:09  rex
 * Converted to new debug system
 * 
 * Revision 1.1  1992/08/31  16:16:53  unknown
 * Initial revision
 * 
*/

#include "lg_types.h"
#include "chr.h"
#include "ctxmac.h"

static short *pCharPixOff;		// ptr to char offset table, with pfont->minch
					// already subtracted out!

#define CHARALIGN(pfont,c) (pCharPixOff[(uchar)c] & 7)
#define CHARPTR(pfont,c) (&pfont->bits[pCharPixOff[(uchar)c] >> 3])
#define CHARWIDTH(pfont,c) (pCharPixOff[(uchar)c+1] - pCharPixOff[(uchar)c])

#define FONT_SETFONT(pfont) (pCharPixOff = &(pfont)->off_tab[0] - (pfont)->min)

//	----------------------------------------------------
//
//	FontWrapText() inserts wrapping codes into text.
//	It inserts soft carriage returns into the text, and
//	returns the number of lines needed for display.
//
//		pfont = ptr to font
//		ps    = ptr to string (soft cr's and soft spaces inserted into it)
//		width = width of area to wrap into, in pixels
//
//	Returns: # lines string wraps into

/* renamed to gr_font_string_wrap in library */

int gr_font_string_wrap (grs_font *pfont, char *ps, short width)
{
	uchar *p;
	char *pmark;
	short numLines;
	short currWidth;

//	Set up to do wrapping

	FONT_SETFONT(pfont);
	numLines = 0;					// ps = base of current line

//	Do wrapping for each line till hit end

	while (*ps)
		{
		pmark = NULL;				// no SOFTCR insert point yet
		currWidth = 0;				// and zero width so far
		p = (uchar *) ps;

//	Loop thru each word

		while (*p)
			{

//	Skip through to next CR or space or '\0', keeping track of width

			while ((*p != 0) && (*p != '\n') && (*p != ' '))
				{
				currWidth += CHARWIDTH(pfont, *p);
				p++;
				}

//	If bypassed width, break out of word loop

			if (currWidth > width)
				{
				if ((pmark == NULL) && (*p != 0) && (*p != '\n'))
					pmark = (char *) p;
				break;
				}

//	Else set new mark point (unless eol or eos, then bust out)

			else
				{
				if ((*p == 0) || (*p == '\n'))	// hit end of line, wipe marker
					{
					pmark = NULL;
					break;
					}
				pmark = (char *) p;									// else advance marker
				currWidth += CHARWIDTH(pfont, ' ');	// and account for space
				p++;
				}
			}

//	Now insert soft cr if marked one

		if (pmark)
			{
			*pmark = CHAR_SOFTCR;
			ps = pmark + 1;
			if (*ps == ' ')			// if wrapped and following space,
				*ps++ = CHAR_SOFTSP;	// turn into (ignored) soft space
			}

//	Otherwise, bump past cr
		else
			{
			if (*p)
				++p;
			ps = (char *) p;
			}

//	Bump line counter in any case

		++numLines;
		}

//	When hit end of string, return # lines encountered

	return(numLines);
}

//	--------------------------------------------------------
//
//	FontUnwrapText() turns soft carriage returns back into
//	spaces.  Usually this is done prior to re-wrapping
//	text with a new width.
//
//		s = ptr to string (soft cr's and spaces turned back to spaces)

/* renamed to gr_font_string_unwrap in 2d library */

void gr_font_string_unwrap (char *s)
{
	int c;

	while ((c = *s) != 0)
		{
		if ((c == CHAR_SOFTCR) || (c == CHAR_SOFTSP))
			*s = ' ';
		s++;
		}
}



