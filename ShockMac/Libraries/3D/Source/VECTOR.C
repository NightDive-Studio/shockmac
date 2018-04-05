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
// $Source: n:/project/lib/src/3d/RCS/vector.asm $
// $Revision: 1.3 $
// $Author: dc $
// $Date: 1993/08/10 22:54:29 $
//
// Vector math routines for 3d library
//
// $Log: vector.asm $
// Revision 1.3  1993/08/10  22:54:29  dc
// add _3d.inc to includes
// 
// Revision 1.2  1993/05/11  15:03:18  matt
// Changed g3_vec_scale() to take seperate dest & src
// Fixed bug in vector compute
// 
// Revision 1.1  1993/05/04  17:39:56  matt
// Initial revision
// 
// 

#include <FixMath.h>
#include "lg.h"
#include "3d.h"
#include "GlobalV.h"
#include "fix.h"


// double a 64 bit long (two 32's), basically a shift left
#define Double_64(a,b) if (((long) b) < 0) {b<<=1; a<<=1; a++;} else {b<<=1; a<<=1;}

// prototypes
void g3_compute_normal_quick(g3s_vector *v, g3s_vector *v0,g3s_vector *v1,g3s_vector *v2);

// adds two vectors:  edi = esi + ebx
void g3_vec_add(g3s_vector *dest,g3s_vector *src1,g3s_vector *src2)
 {
 	dest->gX = src1->gX + src2->gX;
 	dest->gY = src1->gY + src2->gY;
 	dest->gZ = src1->gZ + src2->gZ;
 }
 
// subtracts two vectors:  edi = esi - ebx. trashes eax
void g3_vec_sub(g3s_vector *dest,g3s_vector *src1,g3s_vector *src2)
 {
 	dest->gX = src1->gX - src2->gX;
 	dest->gY = src1->gY - src2->gY;
 	dest->gZ = src1->gZ - src2->gZ;
 }


// scale a vector in place. takes edi=dest vector, esi=src vector, ebx=scale
void g3_vec_scale(g3s_vector *dest,g3s_vector *src,fix s)
 {
 	dest->gX = fix_mul(src->gX,s);
 	dest->gY = fix_mul(src->gY,s);
 	dest->gZ = fix_mul(src->gZ,s);
 }
  
// fix mag(vector *v)
// takes esi = v. returns mag in eax. trashes all but ebp
fix g3_vec_mag(g3s_vector *v)
 {
	AWide		result,result2;
	
	AsmWideMultiply(v->gX, v->gX, &result);
	AsmWideMultiply(v->gY, v->gY, &result2);
	AsmWideAdd(&result, &result2);
	AsmWideMultiply(v->gZ, v->gZ, &result2);
	AsmWideAdd(&result, &result2);

	return (quad_sqrt(result.hi, result.lo));

//	return(quad_sqrt(result.hi, result.lo));
 }

// compute dot product of vectors at [esi] & [edi]
fix g3_vec_dotprod(g3s_vector *v0,g3s_vector *v1)
 {
	AWide		result,result2;
	
	AsmWideMultiply(v0->gX, v1->gX, &result);
	AsmWideMultiply(v0->gY, v1->gY, &result2);
	AsmWideAdd(&result, &result2);
	AsmWideMultiply(v0->gZ, v1->gZ, &result2);
	AsmWideAdd(&result, &result2);
  return((result.hi<<16) | (((ulong) result.lo)>>16));
 }


// compute normalized surface normal from three points.
// takes edi=dest, eax,edx,ebx = points. fills in [edi]. 
// trashes eax,ebx,ecx,edx,esi
void g3_compute_normal(g3s_vector *norm,g3s_vector *v0,g3s_vector *v1,g3s_vector *v2)
 {
 	g3_compute_normal_quick(norm,v0,v1,v2);
	g3_vec_normalize(norm);	// now normalize
 }


// normalizes the vector at esi. trashes all but esi,ebp
void g3_vec_normalize(g3s_vector *v)
 {
 	fix temp;
 	
 	temp = g3_vec_mag(v);
 	
 	v->gX = fix_div(v->gX,temp);
 	v->gY = fix_div(v->gY,temp);
 	v->gZ = fix_div(v->gZ,temp);
 }


// compute surface normal from three points. DOES NOT NORMALIZE!
// takes edi=dest, eax,edx,ebx = points. fills in [edi]. 
// trashes eax,ebx,ecx,edx,esi
// the quick version does not normalize
void g3_compute_normal_quick(g3s_vector *v, g3s_vector *v0,g3s_vector *v1,g3s_vector *v2)
 {
	AWide				result,result2;
	g3s_vector	temp_v0;
	g3s_vector	temp_v1;
	g3s_vector	temp_high;
	long				temp_long;
 	int					shiftcount;
 	
 	g3_vec_sub(&temp_v0,v1,v0);
 	g3_vec_sub(&temp_v1,v2,v1);

// dest->x = v1z * v0y - v1y * v0z;
	AsmWideMultiply(temp_v1.gZ, temp_v0.gY, &result);
	AsmWideMultiply(temp_v1.gY, temp_v0.gZ, &result2);
	AsmWideNegate(&result2);
	AsmWideAdd(&result, &result2);
 	v->gX = result.lo;
 	temp_high.gX = result.hi;
 	
// dest->y = v1x * v0z - v1z * v0x;
	AsmWideMultiply(temp_v1.gX, temp_v0.gZ, &result);
	AsmWideMultiply(temp_v1.gZ, temp_v0.gX, &result2);
	AsmWideNegate(&result2);
	AsmWideAdd(&result, &result2);
 	v->gY = result.lo;
 	temp_high.gY = result.hi;

// dest->z = v1y * v0x - v1x * v0y;
	AsmWideMultiply(temp_v1.gY, temp_v0.gX, &result);
	AsmWideMultiply(temp_v1.gX, temp_v0.gY, &result2);
	AsmWideNegate(&result2);
	AsmWideAdd(&result, &result2);
 	v->gZ = result.lo;
 	temp_high.gZ = result.hi;

// see if fit into a longword
	result.hi = temp_high.gX;
	result.lo = v->gX;
	if (result.hi < 0)
		AsmWideNegate(&result);
	Double_64(result.hi,result.lo);
	temp_long = result.hi;
	
	result.hi = temp_high.gY;
	result.lo = v->gY;
	if (result.hi < 0)
		AsmWideNegate(&result);
	Double_64(result.hi,result.lo);
	temp_long |= result.hi;

	result.hi = temp_high.gZ;
	result.lo = v->gZ;
	if (result.hi < 0)
		AsmWideNegate(&result);
	Double_64(result.hi,result.lo);
	temp_long |= result.hi;
	
	if (!temp_long) return; // everything fits in the low longword. hurrah. see ya.
	
// see how far to shift to fit in a longword
	shiftcount = 0;
	while (((unsigned long) temp_long) >= 0x0100)
	 {
	 	shiftcount += 8;
	 	temp_long >>= 8;
	 }
	shiftcount += shift_table[temp_long];

// now get the results
	result.hi = temp_high.gX;
	result.lo = v->gX;
	AsmWideBitShift(&result, shiftcount);
	v->gX = result.lo;

	result.hi = temp_high.gY;
	result.lo = v->gY;
	AsmWideBitShift(&result, shiftcount);
	v->gY = result.lo;

	result.hi = temp_high.gZ;
	result.lo = v->gZ;
	AsmWideBitShift(&result, shiftcount);
	v->gZ = result.lo;
 }
