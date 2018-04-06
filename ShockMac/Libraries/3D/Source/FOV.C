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
// $Source: r:/prj/lib/src/3d/RCS/fov.asm $
// $Revision: 1.4 $
// $Author: jaemz $
// $Date: 1994/10/26 21:30:21 $
//
// Routines to get FOV and zoom
//
// $Log: fov.asm $
// Revision 1.4  1994/10/26  21:30:21  jaemz
// Added get_zoom refresh aspect rat from 2d
// 
// Revision 1.3  1994/06/02  15:07:37  junochoe
// changed matrix_scale to _matrix_scale
// 
// 
// Revision 1.2  1993/08/10  22:54:07  dc
// add _3d.inc to includes
// 
// Revision 1.1  1993/05/04  17:39:45  matt
// Initial revision
// 
// 


#include "lg.h"
#include "3d.h"
#include "GlobalV.h"

// returns current field of view. returns ax=x FOV, bx=y FOV
// trashes eax,ebx,ecx,edx,edi
//formula is fov = acos( (x-z) / (x+z) ) where x,z are the matrix scale values
void g3_get_FOV(fixang *x,fixang *y)
 {
 	fix	X2,Y2,Z2;
 	
  Z2 = fix_mul(_matrix_scale.gZ,_matrix_scale.gZ);	// get z squared

//compute y
	Y2 = fix_mul(_matrix_scale.gY,_matrix_scale.gY);	// get y squared
	*y = fix_acos(fix_div(Y2-Z2, Y2+Z2));

//compute x
  X2 = fix_mul(_matrix_scale.gX,_matrix_scale.gX);	// get z squared
	*x = fix_acos(fix_div(X2-Z2, X2+Z2));
}


//returns zoom for a desired FOV. 
//takes bx=FOV angle, al=axis ('X' or 'Y'), ecx=window width, edx=window height
//returns in eax. trashes all but ebp

fix g3_get_zoom(char axis, fixang angle, int window_width, int window_height)
 {	
 	fix 	sin_val, cos_val;
 	fix		unscalezoom,temp1;
 	long	templong;
 	
	fix_sincos(angle, &sin_val, &cos_val);		// call	fix_sincos	;angle in bx
	temp1 = fix_div(f1_0 - cos_val,cos_val + f1_0);	

	unscalezoom = fix_sqrt(temp1);	// 	call	fix_sqrt_	;eax = unscaled zoom

//now, temp1 would be zoom if not for window and pixel matrix scaling.
//correct for these

//get pixel ratio
	pixel_ratio = grd_cap->aspect;

//get matrix scale value for given window size	
	templong = fix_mul_div(window_height, pixel_ratio, window_width);		// imul	pixel_ratio	;height * pixrat
																																			// 	idiv	ebx	;eax = h * pixrat / w
// window and pixrat scaling affects y. see if y FOV requested
	if (templong <= f1_0)	// cmp	eax,f1_0	;< 1.0? jle	scale_x	;scale x
	 {
	 	if (axis!='X') return(unscalezoom);
	 	return (fix_mul(unscalezoom, templong));
	 }
	else
	 {
	 	if (axis!='Y') return(unscalezoom);
	 	return (fix_div(unscalezoom, templong));
	 }
 }
