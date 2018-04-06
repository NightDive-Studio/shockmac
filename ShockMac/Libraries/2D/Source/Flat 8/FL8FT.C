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
 * $Source: r:/prj/lib/src/2d/RCS/fl8ft.c $
 * $Revision: 1.1 $
 * $Author: kevin $
 * $Date: 1994/08/16 13:18:21 $
 *
 * Gruesome function tables.
 * flat8 canvas.
 *
 * This file is part of the 2d library.
 */

#include "bitmap.h"
#include "fill.h"
#include "gentf.h"
#include "grnull.h"
#include "ifcn.h"
#include "tmapint.h"
#include "fl8tf.h"
#include "gentf.h"

typedef void (*ptr_type)();

void (*flat8_function_table[GRD_FILL_TYPES][GRD_FUNCS*REAL_BMT_TYPES])() =
{
   {
      /* normal fill type */
      #include "fl8nft.h"
   },
   {
      /* clut fill type */
      #include "fl8cft.h"
   },
   {
      /* xor fill type */
      #include "fl8xft.h"
   },
   {
      /* blend fill type */
      #include "fl8bft.h"
   },
   {
      /* solid fill type */
      #include "fl8sft.h"
   }
};
