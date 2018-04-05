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
#ifndef __STRWRAP_H
#define __STRWRAP_H

/*
 * $Source: n:/project/cit/src/inc/RCS/strwrap.h $
 * $Revision: 1.1 $
 * $Author: mahk $
 * $Date: 1994/06/16 21:40:14 $
 *
 */

// Includes
#define CHAR_SOFTCR     0x01            // soft carriage return (wrapped text)
#define CHAR_SOFTSP     0x02            // soft space (wrapped text)

extern int wrap_text(char *ps, short width);
extern void unwrap_text(char *s);


#endif // __STRWRAP_H

