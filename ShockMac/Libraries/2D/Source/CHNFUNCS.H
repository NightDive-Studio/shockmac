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
 * $Source: n:/project/lib/src/2d/RCS/chnfuncs.h $
 * $Revision: 1.3 $
 * $Author: baf $
 * $Date: 1993/11/16 23:07:05 $
 *
 * Prototypes for stuff internal to the chaining
 * routines.
 *
 * This file is part of the 2D library.
 *
 * $Log: chnfuncs.h $
 * Revision 1.3  1993/11/16  23:07:05  baf
 * Added the ability to chain void functions
 * after the primitive.
 * 
 * Revision 1.2  1993/11/15  03:30:45  baf
 * One measly extra macro for void chaining.
 * 
 * Revision 1.1  1993/11/12  10:07:13  baf
 * Initial revision
 * 
 */
#ifndef __CHNFUNCS
#define __CHNFUNCS

/* The following is for the flags field in the
   grs_func_chain struct. */
#define CHNF_VOID 1
#define CHNF_AFTER 3

extern grs_func_chain *gr_current_chain;
extern void (*chn_canvas_table[])();
extern void (**chn_primitives[])();
extern int gr_current_primitive;

#endif
