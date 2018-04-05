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
 * $Source: n:/project/lib/src/2d/RCS/invtab.c $
 * $Revision: 1.1 $
 * $Author: kevin $
 * $Date: 1994/05/15 17:35:27 $
 *
 * Fixed point inverse table.
 *
 * This file is part of the 2d library.
 *
 */

#include "fix.h"
#include "Invtab.h"

fix inverse_table[1024];

void init_inverse_table(void)
{
   int i;

   inverse_table[0]=FIX_MAX;
   for (i=1; i<1024; i++)
      inverse_table[i]=FIX_UNIT/i;
}

