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
// $Source: n:/project/lib/src/3d/RCS/slew.asm $
// $Revision: 1.2 $
// $Author: dc $
// $Date: 1993/08/10 22:54:25 $
//
// Support function(s) for slew system
//
// $Log: slew.asm $
// Revision 1.2  1993/08/10  22:54:25  dc
// add _3d.inc to includes
// 
// Revision 1.1  1993/05/24  16:27:24  matt
// Initial revision
// 
// 

#include "lg.h"
#include "3d.h"
#include "GlobalV.h"


//fills in three vectors, each of length step_size, in the x,y, & z directions
//in the viewer's frame of reference.  Any (or all) of the vector ptrs can
//be NULL and will be skipped.
//takes eax=size, ebx,ecx,edi=x,y, & z vectors
//trashes eax,edx,esi
void g3_get_slew_step(fix step_size, g3s_vector *x_step, g3s_vector *y_step, g3s_vector *z_step)
 {
// x vector
	if (x_step)
	 {
	 	x_step->gX = fix_mul(step_size, unscaled_matrix.m1);
	 	x_step->gY = fix_mul(step_size, unscaled_matrix.m4);
	 	x_step->gZ = fix_mul(step_size, unscaled_matrix.m7);
	 }

// y vector
	if (y_step)
	 {
	 	y_step->gX = fix_mul(step_size, unscaled_matrix.m2);
	 	y_step->gY = fix_mul(step_size, unscaled_matrix.m5);
	 	y_step->gZ = fix_mul(step_size, unscaled_matrix.m8);
	 }

// z vector
	if (z_step)
	 {
	 	z_step->gX = fix_mul(step_size, unscaled_matrix.m3);
	 	z_step->gY = fix_mul(step_size, unscaled_matrix.m6);
	 	z_step->gZ = fix_mul(step_size, unscaled_matrix.m9);
	 }
 }

