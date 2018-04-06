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
// $Source: r:/prj/lib/src/3d/RCS/instance.asm $
// $Revision: 1.8 $
// $Author: jaemz $
// $Date: 1994/10/24 01:05:30 $
//
// Instancing routines
//
// $Log: instance.asm $
// Revision 1.8  1994/10/24  01:05:30  jaemz
// Fixed inverted pitch problem by saving state of ecx
// 
// Revision 1.7  1994/10/12  01:09:06  jaemz
// Inverted wtoo_matrix to make it correct
// for lighting
// 
// Revision 1.6  1994/09/20  13:32:48  jaemz
// Lighting support
// 
// Revision 1.5  1994/08/18  03:46:57  jaemz
// Changed stereo glob names to have underscore for c
// 
// Revision 1.4  1994/07/15  14:13:28  jaemz
// Added _view_position with an underscore to make it c readable
// 
// Revision 1.3  1993/08/10  22:54:12  dc
// add _3d.inc to includes
// 
// Revision 1.2  1993/06/22  18:35:32  kaboom
// Changed g3_matrix_x_matrix to g3_matrix_x_matrix_ so it's callable
// from watcom C w/register passing.
// 
// Revision 1.1  1993/05/04  17:39:49  matt
// Initial revision
// 
// 

#include "lg.h"
#include "3d.h"
#include "GlobalV.h"

// externs
extern void angles_2_matrix(g3s_angvec *angles, g3s_matrix *view_matrix, int rotation_order);

// prototypes
bool instance_x(fixang tx);
bool instance_y(fixang ty);
bool instance_z(fixang tz);
void instance_matrix(g3s_matrix *src, g3s_matrix *dest);
bool save_context(void);
bool g3_start_object_angles_zy(g3s_vector *p,fixang ty,fixang tz,int rotation_order);
bool start_obj_common(g3s_vector *p,g3s_angvec *o,int rotation_order);



#define MAX_INSTANCE_DEPTH 5

#define CONTEXT_SIZE (sizeof(g3s_matrix) + sizeof(g3s_vector))

//stack for pushed context while instanced
char context_stack[MAX_INSTANCE_DEPTH * CONTEXT_SIZE];
char *cstack_ptr = context_stack;

long	cstack_depth;

//takes esi=position. No orientation, just offset
bool g3_start_object(g3s_vector *p)    //position only (no orientation)
 {
 	if (save_context()) return 0;
 	
//compute new view position
	_view_position.gX -= p->gX;
	_view_position.gY -= p->gY;
	_view_position.gZ -= p->gZ;
	
	return -1;	//success
 }
 
//takes esi=position, ecx=rotation order, angles=eax,ebx,edx
bool g3_start_object_angles_xyz(g3s_vector *p,fixang tx,fixang ty,fixang tz,int rotation_order)
 {
	g3s_angvec temp_angles;

 	if (save_context()) return 0;

	temp_angles.tx = tx;
	temp_angles.ty = ty;
	temp_angles.tz = tz;
	
	return (start_obj_common(p,&temp_angles,rotation_order));
 }


//takes esi=position, edi=orientation vector, ecx=rotation order
bool g3_start_object_angles_v(g3s_vector *p,g3s_angvec *o,int rotation_order)
 {
 	if (save_context()) return 0;
	return (start_obj_common(p,o,rotation_order));
 }
 
//takes esi=position, edi=orientation vector, ecx=rotation order
bool start_obj_common(g3s_vector *p,g3s_angvec *o,int rotation_order)
 {
	g3s_matrix temp_matrix;

//compute new context
	_view_position.gX -= p->gX;
	_view_position.gY -= p->gY;
	_view_position.gZ -= p->gZ;
	
  // copy obj offset to world to obj structure
  // used for lighting, dude
  _wtoo_position = *p;

	angles_2_matrix(o,&temp_matrix,rotation_order);

//rotate view vector through instance matrix
	g3_vec_rotate(&_view_position,&_view_position,&temp_matrix);

  // save off to the obj_to_world matrix
  // untransposed, to get inverse
	_wtoo_matrix = temp_matrix;

	g3_transpose(&temp_matrix);		//transpose esi in place
	instance_matrix(&temp_matrix,&view_matrix);

	return -1;	//ok!
 }

// dest=c1*s1+c2*s2
#define update_m(dest,c1,s1,c2,s2) \
 {AWide	result,result2; \
	AsmWideMultiply(c1, s1, &result);\
	AsmWideMultiply(c2, s2, &result2); \
	AsmWideAdd(&result, &result2);\
	dest = (result.hi<<16) | (((ulong) result.lo)>>16);}

// dest=c1*s1-c2*s2
#define update_ms(dest,c1,s1,c2,s2) \
 {AWide	result,result2; \
	AsmWideMultiply(c1, s1, &result);\
	AsmWideMultiply(c2, s2, &result2); \
	AsmWideNegate(&result2); \
	AsmWideAdd(&result, &result2);\
	dest = (result.hi<<16) | (((ulong) result.lo)>>16);}


//rotate around the specified axis. angle = ebx
//takes esi=position, 
bool g3_start_object_angles_y(g3s_vector *p,fixang ty)
 {
 	if (save_context()) return 0;

//compute new view position
	_view_position.gX -= p->gX;
	_view_position.gY -= p->gY;
	_view_position.gZ -= p->gZ;

	return(instance_y(ty));
 }
 
//get sin & cos - angles still in ebx
bool instance_y(fixang ty)
 {
 	fix		temp;
	fix		sin_y;
	fix		cos_y;
	fix		temp1;
	fix		temp2;
	fix		temp3;
	AWide	result,result2;

 	fix_sincos (ty, &sin_y, &cos_y);

//rotate viewer vars
	AsmWideMultiply(_view_position.gZ, sin_y, &result);
	AsmWideMultiply(_view_position.gX, cos_y, &result2);
	AsmWideNegate(&result);
	AsmWideAdd(&result, &result2);
	temp = (result.hi<<16) | (((ulong) result.lo)>>16);

	AsmWideMultiply(_view_position.gX, sin_y, &result);
	_view_position.gX = temp;
	AsmWideMultiply(_view_position.gZ, cos_y, &result);
	AsmWideAdd(&result, &result2);
	_view_position.gZ = (result.hi<<16) | (((ulong) result.lo)>>16);

//now modify matrix
	update_ms(temp1,cos_y,vm1,sin_y,vm7);
	update_ms(temp2,cos_y,vm2,sin_y,vm8);
	update_ms(temp3,cos_y,vm3,sin_y,vm9);
	update_m(vm7,sin_y,vm1,cos_y,vm7);
	update_m(vm8,sin_y,vm2,cos_y,vm8);
	update_m(vm9,sin_y,vm3,cos_y,vm9);
	vm1 = temp1;
	vm2 = temp2;
	vm3 = temp3;
	
//we're done
	return -1;	//ok!
 }

//rotate around the specified axis. angle = ebx
//takes esi=position, 
bool g3_start_object_angles_x(g3s_vector *p,fixang tx)
 {
 	if (save_context()) return 0;

//compute new view position
	_view_position.gX -= p->gX;
	_view_position.gY -= p->gY;
	_view_position.gZ -= p->gZ;

	return(instance_x(tx));
 }
 
//get sin & cos - angles still in ebx
bool instance_x(fixang tx)
 {
	fix		temp;
	fix		temp1;
	fix		temp2;
	fix		temp3;
	fix		sin_x;
	fix		cos_x;
	AWide	result,result2;

 	fix_sincos (tx, &sin_x, &cos_x);

//rotate viewer vars
	AsmWideMultiply(_view_position.gZ, sin_x, &result);
	AsmWideMultiply(_view_position.gY, cos_x, &result2);
	AsmWideAdd(&result, &result2);
	temp = (result.hi<<16) | (((ulong) result.lo)>>16);

	AsmWideMultiply(_view_position.gY, sin_x, &result);
	_view_position.gY = temp;
	AsmWideMultiply(_view_position.gZ, cos_x, &result);
	AsmWideNegate(&result);
	AsmWideAdd(&result, &result2);
	_view_position.gZ = (result.hi<<16) | (((ulong) result.lo)>>16);

//now modify matrix

	update_m(temp1,cos_x,vm4,sin_x,vm7);
	update_m(temp2,cos_x,vm5,sin_x,vm8);
	update_m(temp3,cos_x,vm6,sin_x,vm9);
	update_ms(vm7,cos_x,vm7,sin_x,vm4);
	update_ms(vm8,cos_x,vm8,sin_x,vm5);
	update_ms(vm9,cos_x,vm9,sin_x,vm6);
	vm4 = temp1;
	vm5 = temp2;
	vm6 = temp3;
	
//we're done
	return -1;	//ok!
 }


//rotate around the specified axis. angle = ebx
//takes esi=position, 
bool g3_start_object_angles_z(g3s_vector *p,fixang tz)
 {
 	if (save_context()) return 0;

//compute new view position
	_view_position.gX -= p->gX;
	_view_position.gY -= p->gY;
	_view_position.gZ -= p->gZ;

	return(instance_z(tz));
 }

//get sin & cos - angles still in ebx
bool instance_z(fixang tz)
 {
	fix		temp;
	fix		temp1;
	fix		temp2;
	fix		temp3;
	fix		sin_z;
	fix		cos_z;
	AWide	result,result2;

 	fix_sincos (tz, &sin_z, &cos_z);

//rotate viewer vars
	AsmWideMultiply(_view_position.gY, sin_z, &result);
	AsmWideMultiply(_view_position.gX, cos_z, &result2);
	AsmWideAdd(&result, &result2);
	temp = (result.hi<<16) | (((ulong) result.lo)>>16);

	AsmWideMultiply(_view_position.gX, sin_z, &result);
	_view_position.gX = temp;
	AsmWideMultiply(_view_position.gY, cos_z, &result);
	AsmWideNegate(&result);
	AsmWideAdd(&result, &result2);
	_view_position.gY = (result.hi<<16) | (((ulong) result.lo)>>16);
	
//now modify matrix
	update_m(temp1,cos_z,vm1,sin_z,vm4);
	update_m(temp2,cos_z,vm2,sin_z,vm5);
	update_m(temp3,cos_z,vm3,sin_z,vm6);
	update_ms(vm4,cos_z,vm4,sin_z,vm1);
	update_ms(vm5,cos_z,vm5,sin_z,vm2);
	update_ms(vm6,cos_z,vm6,sin_z,vm3);
	vm1 = temp1;
	vm2 = temp2;
	vm3 = temp3;
	
//we're done
	return -1;  //ok!
 }

//rotate around the specified axes. angles = ebx edx. esi=position
bool g3_start_object_angles_xy(g3s_vector *p,fixang tx,fixang ty,int rotation_order)
 {
 	if (save_context()) return 0;

//compute new view position
	_view_position.gX -= p->gX;
	_view_position.gY -= p->gY;
	_view_position.gZ -= p->gZ;

	if ((rotation_order & 1)==0)	//check xy order
   {
   	instance_x(tx);
   	return(instance_y(ty));
   }
  else
   {
   	instance_y(ty);
   	return(instance_x(tx));
   }
 }


//rotate around the specified axes. angles = ebx edx. esi=position
bool g3_start_object_angles_xz(g3s_vector *p,fixang tx,fixang tz,int rotation_order)
 {
 	if (save_context()) return 0;

//compute new view position
	_view_position.gX -= p->gX;
	_view_position.gY -= p->gY;
	_view_position.gZ -= p->gZ;

	if ((rotation_order & 2)==0)	//check xz order
   {
   	instance_x(tx);
   	return(instance_z(tz));
   }
  else
   {
   	instance_z(tz);
   	return(instance_x(tx));
   }
 }


//rotate around the specified axes. angles = ebx edx. esi=position
bool g3_start_object_angles_yz(g3s_vector *p,fixang ty,fixang tz,int rotation_order)
 {
 	if (save_context()) return 0;

//compute new view position
	_view_position.gX -= p->gX;
	_view_position.gY -= p->gY;
	_view_position.gZ -= p->gZ;

	if ((rotation_order & 4)==0)	//check yz order
   {
   	instance_y(ty);
   	return(instance_z(tz));
   }
  else
   {
   	instance_z(tz);
   	return(instance_y(ty));
   }
 }


//rotate around the specified axes. angles = ebx edx. esi=position
bool g3_start_object_angles_zy(g3s_vector *p,fixang ty,fixang tz,int rotation_order)
 {
 	if (save_context()) return 0;

//compute new view position
	_view_position.gX -= p->gX;
	_view_position.gY -= p->gY;
	_view_position.gZ -= p->gZ;

 	instance_z(tz);
 	return(instance_y(ty));
 }


//takes esi=position, edi=object matrix
bool g3_start_object_matrix(g3s_vector *p,g3s_matrix *m)
 {
	g3s_matrix temp_matrix;

 	if (save_context()) return 0;

//compute new view position
	_view_position.gX -= p->gX;
	_view_position.gY -= p->gY;
	_view_position.gZ -= p->gZ;

//rotate view vector through instance matrix
	g3_vec_rotate(&_view_position,&_view_position,m);

//copy to temp matrix, since instance routine transposes in place
	g3_copy_transpose(&temp_matrix,m);
	instance_matrix(&temp_matrix,&view_matrix);

	return -1;	//ok!
 }


//save the current view matrix + view position.
//returns carry set if cannot save. saves all regs
bool save_context(void)
 {
 	if (cstack_depth==MAX_INSTANCE_DEPTH)	
 		return 1;
 	
 	cstack_depth++;	
 	
//save current context
 	* (g3s_matrix *) cstack_ptr = view_matrix;
 	cstack_ptr += sizeof(g3s_matrix);
 	* (g3s_vector *) cstack_ptr = _view_position;
 	cstack_ptr += sizeof(g3s_vector);
 	
 	return 0;
 }

// scales an object within an object context
// argument in eax per c convention
// trashes ecx edx and eax
void g3_scale_object(fix s)
 {
  // scale vm by scale, and divide view_position
  // down by scale

	_view_position.gX = fix_div(_view_position.gX,s);
	_view_position.gY = fix_div(_view_position.gY,s);
	_view_position.gZ = fix_div(_view_position.gZ,s);

  // scale vm up by scale
	vm1 = fix_mul(vm1,s);
	vm2 = fix_mul(vm2,s);
	vm3 = fix_mul(vm3,s);
	vm4 = fix_mul(vm4,s);
	vm5 = fix_mul(vm5,s);
	vm6 = fix_mul(vm6,s);
	vm7 = fix_mul(vm7,s);
	vm8 = fix_mul(vm8,s);
	vm9 = fix_mul(vm9,s);
 }

void g3_end_object(void)
 {
 	if (cstack_depth==0) return;
 	
 	cstack_depth--;

 	cstack_ptr -= sizeof(g3s_vector);
 	_view_position = * (g3s_vector *) cstack_ptr;
 	cstack_ptr -= sizeof(g3s_matrix);
 	view_matrix = * (g3s_matrix *) cstack_ptr;
 }


//edi = esi * edi.  esi should be transposed before calling
void instance_matrix(g3s_matrix *src, g3s_matrix *dest)
 {
	g3s_matrix temp_matrix2;

 	//do multiply
 	g3_matrix_x_matrix(&temp_matrix2, src, dest);	

	//copy to real dest
	*dest = temp_matrix2;
 }

