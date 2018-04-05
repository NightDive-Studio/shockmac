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
//
// $Source: n:/project/lib/src/2d/RCS/chnfuncs.asm $
// $Revision: 1.5 $
// $Author: kevin $
// $Date: 1994/12/02 21:52:27 $
//
// The Chain Handler, and attendant data.
//
// This file is part of the 2D library.
//
// Revision 1.5  1994/12/02  21:52:27  kevin
// Moved chaining globals accessed in gr_set_canvas to grd.c to avoid linking in unused code.
// 
// Revision 1.4  1993/11/16  23:07:04  baf
// Added the ability to chain void functions
// after the primitive.
// 
// Revision 1.3  1993/11/16  14:32:05  baf
// Made over in accordance with Kaboom's
// aesthetics. Also fixed a small bug.
// 
// Revision 1.2  1993/11/15  03:29:54  baf
// Added support for chained void functions.
// 
// Revision 1.1  1993/11/12  10:06:48  baf
// Initial revision
// 
//

#include "ICanvas.h"
#include "chain.h"
#include "chnfuncs.h"


// globals
grs_func_chain 	*gr_current_chain;
short						funcnum;
Ptr							firstfunc;

Ptr							chain_stack[256];
Ptr							chain_stack_ptr;

void 						(**chn_primitives[GRD_CANVAS_FUNCS])();
int 						gr_current_primitive = GRD_CANVAS_FUNCS;
