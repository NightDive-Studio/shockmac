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
 * $Source: n:/project/lib/src/2d/RCS/bit.c $
 * $Revision: 1.2 $
 * $Author: kaboom $
 * $Date: 1992/12/11 13:01:33 $
 *
 * Definition of bit masks for monochrome bitmaps.
 *
 * This file is part of the 2d library.
 *
 * $Log: bit.c $
 * Revision 1.2  1992/12/11  13:01:33  kaboom
 * Reversed order of bits in the bitmask.
 * 
 * Revision 1.1  1992/11/19  02:27:09  kaboom
 * Initial revision
 */

#include "LG.h" 

/* used by monochrome bitmap routines. */
uchar bitmask[9] = { 128, 64, 32, 16, 8, 4, 2, 1, 0 };
