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
 * $Source: r:/prj/lib/src/2d/RCS/svgainit.c $
 * $Revision: 1.1 $
 * $Author: kevin $
 * $Date: 1994/12/05 21:07:57 $
 *
 * Routine to initialize the 2d system for svga use.
 *
 * This file is part of the 2d library.
 *
 * $Log: svgainit.c $
 * Revision 1.1  1994/12/05  21:07:57  kevin
 * Initial revision
 * 
 */

#include "grs.h"
#include "detect.h"
#include "init.h"
#include "initint.h"

int gr_init(void)
{
// MLA - ditched this, because there is only one mode to detect on the Mac
//   grd_detect_func=svga_detect;
   return gri_init();
}
