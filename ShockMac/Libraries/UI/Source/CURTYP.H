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
 * $Source: n:/project/lib/src/ui/RCS/curtyp.h $
 * $Revision: 1.1 $
 * $Author: kaboom $
 * $Date: 1993/12/16 07:46:45 $
 *
 * Declarations for cursor types.
 *
 * $Log: curtyp.h $
 * Revision 1.1  1993/12/16  07:46:45  kaboom
 * Initial revision
 * 
 */

/* the saveunder for bitmap cursors */
struct _cursor_saveunder {
   grs_bitmap bm;
   int mapsize;
};
