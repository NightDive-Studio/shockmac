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
// $Source: r:/prj/lib/src/3d/RCS/points.asm $
// $Revision: 1.17 $
// $Author: jaemz $
// $Date: 1994/09/28 19:00:52 $
//
// Point definition routines
//

#include <FixMath.h>
#include "lg.h"
#include "3d.h"
#include "GlobalV.h"

// prototypes
void rotate_norm(g3s_vector *v, fix *x, fix *y, fix *z);
void do_norm_rotate(fix x, fix y, fix z, fix *rx, fix *ry, fix *rz);

#if (defined(powerc) || defined(__powerc))	
void do_rotate(fix x, fix y, fix z, fix *rx, fix *ry, fix *rz);
#else
asm void do_rotate(fix x, fix y, fix z, fix *rx, fix *ry, fix *rz);
#endif

//void xlate_rotate_point(g3s_vector *v, fix *x, fix *y, fix *z);     
#define xlate_rotate_point(v,x,y,z) do_rotate(v->gX-_view_position.gX, v->gY-_view_position.gY, v->gZ-_view_position.gZ,x,y,z)    

extern int code_point(g3s_point *pt);
extern char SubLongWithOverflow(long *result, long src, long dest);
extern char AddLongWithOverflow(long *result, long src, long dest);

//for temp use in rotate_list, etc.
g3s_codes g_codes;


//rotate a normal or gradient vector. esi=vector, returns edi=point, bl=codes.
//assumes perspective mapper scale factor will be set to _scrw.
g3s_phandle g3_rotate_norm(g3s_vector *v)
 {
 	fix					x,y,z;
 	fix					temp,temp2,temp3;
 	g3s_point 	*point;
 	
 	rotate_norm(v,&x,&y,&z);		

	temp = fix_div(z,_matrix_scale.gZ);
	temp2 = fix_div(x,_matrix_scale.gX);																	
	temp3 = fix_div(y,_matrix_scale.gY);		
	
	temp3 = -fix_mul_div(temp3,_scrw,_scrh);	// because projecting negates too, of course. Grrr.														

	getpnt(point);		
	point->gX = temp2;
	point->gY = temp3;
	point->gZ = temp;
	point->p3_flags = 0;
	
	return(point);
 }
 
g3s_phandle g3_rotate_point(g3s_vector *v)
 {	
 	g3s_point 	*point;

	getpnt(point);		
 	xlate_rotate_point(v,&point->gX,&point->gY,&point->gZ);
	point->p3_flags = 0;

	code_point(point);	
	return(point);
 }

// matrix multiply and project a point. esi=vector, returns edi=point
g3s_phandle g3_transform_point(g3s_vector *v)
 {
 	g3s_phandle tempH;
 	
 	tempH = g3_rotate_point(v);
 	g3_project_point(tempH);
 	return(tempH);
 }

// takes edi = ptr to point. projects, fills in sx,sy, sets flag.
// returns 0 if z<=0, 1 if z>0.
// trashes eax,ecx,edx.
#if (defined(powerc) || defined(__powerc))	
int g3_project_point(g3s_phandle p)
 {
 	fix		x,y,z,res;
 	
#ifdef stereo_on
        test    _g3d_stereo,1
        jz      no_stereo1
        // is this a sister point?
        cmp     edi,_g3d_stereo_list
        jl      not_sister

        // debug_brk 'yo, found projecting sister'

        // copy the point and add
	mov     esi,edi
	sub     esi,_g3d_stereo_base
	mov     ecx,(size g3s_point)/4
	rep movsd

        mov     eax,_g3d_eyesep
        sub     edi,(size g3s_point)    //restore edi
        add     [edi].x,eax

        // call clip encoder on this point
        mov     ecx,ebx
        call    code_point
        mov     ebx,ecx

        // project point like a normal point
        mov     _g3d_stereo,0
        pop     esi
        call    g3_project_point
        mov     _g3d_stereo,1

        ret

        not_sister: 
        // copy the point
	mov     esi,edi
	add     edi,_g3d_stereo_base
	mov     ecx,(size g3s_point)/4
	rep movsd
        mov     eax,_g3d_eyesep
        sub     edi,(size g3s_point)    //restore edi
        add     [edi].x,eax

        // call clip encoder
        mov     ecx,ebx
        call    code_point
        mov     ebx,ecx

        sub     edi,_g3d_stereo_base
no_stereo1:
#endif

	// check if this point is in front of the back plane.
	z = p->gZ;
	if (z<=0) return 0;
	x = p->gX;
	y = p->gY;
	
	// point is in front of back plane---do projection.
	// project y coordinate.
	res = fix_mul_div(y,_scrh,z);
	if (gOVResult) {p->codes |= CC_CLIP_OVERFLOW; return 1;}
	res = -res;
	if (AddLongWithOverflow(&res, res, _biasy)) {p->codes |= CC_CLIP_OVERFLOW; return 1;}
	p->sy = res;

  // now project x point
	res = fix_mul_div(x,_scrw,z);
	if (gOVResult) {p->codes |= CC_CLIP_OVERFLOW; return 1;}
	if (AddLongWithOverflow(&res, res, _biasx)) {p->codes |= CC_CLIP_OVERFLOW; return 1;}
	p->sx = res;
	
	// modify point flags to indicate projection.
	p->p3_flags |= PF_PROJECTED;

#ifdef stereo_on
        test    _g3d_stereo,1
        jz      no_stereo2
        mov     eax,[edi].sy            //copy over old sy
        add     edi,_g3d_stereo_base     //load twin address
        mov     [edi].sy,eax            //make new sy, could add the .5 addition here too

	mov     eax,[edi].x
	// reproject the x coord
	imul    _scrw                   //* screen width
proj_div_2:
	idiv    ecx                     /// z
	add     eax,_biasx              //+center
	mov     [edi].sx,eax            //save

        // indicate projection
	or      [edi].p3_flags,PF_PROJECTED

	// restore edi
	sub     edi,_g3d_stereo_base
no_stereo2:
#endif
	
	// point has been projected.
	return 1;
 }
#else
// 68K g3_project_point
asm int g3_project_point(g3s_phandle p)
 {
	move.l	d3,a1		// save d3

 	move.l	4(a7),a0
 	
	// check if this point is in front of the back plane.
	move.l	8(a0),d2			// mov     ecx,[edi].z             //get z
												// or      ecx,ecx                 //check neg z
	ble.s		no_proj				// jle     no_proj
	
	// point is in front of back plane---do projection.

	// project y coordinate.
	move.l	4(a0),d0			// mov     eax,[edi].y             //get y
	move.l	_scrh,d3
	dc.l		0x4C030C01   	// muls.l	d3,d1:d0		// imul    _scrh       //* screen height
proj_div_1:
	dc.l		0x4C420C01		// divs.l	d2,d1:d0			// idiv    ecx                     /// z
	bvs.s		project_overflow
@divback1:
	neg.l		d0						// neg     eax                     //convert to screen convention ARGHHH!! LAMEASS SONUFABITCH
	add.l		_biasy,d0			// add     eax,_biasy              //+center
	bvs.s		project_overflow		// jo      project_overflow
	move.l	d0,16(a0)	// mov     [edi].sy,eax            //save

        // now project x point
	move.l	(a0),d0				// mov     eax,[edi].x             //get x
	move.l	_scrw,d3
	dc.l		0x4C030C01   	// muls.l	d3,d1:d0		// imul    _scrw                   //* screen width
proj_div_0:
	dc.l		0x4C420C01		// divs.l	d2,d1:d0			// idiv    ecx                     /// z
	bvs.s		project_overflow
@divback0:
	add.l		_biasx,d0			// add     eax,_biasx              //+center
	bvs.s		project_overflow		// jo      project_overflow
	move.l	d0,12(a0)	// mov     [edi].sx,eax            //save

	// modify point flags to indicate projection.
	or.b		#PF_PROJECTED,21(a0)	// or      [edi].p3_flags,PF_PROJECTED
	moveq		#1,d0					// mov     eax,1  //return true when z>0
	move.l	a1,d3
 	rts

no_proj:
	move		#0,d0
	move.l	a1,d3
	rts
	 	
project_overflow:
	or.b		#CC_CLIP_OVERFLOW,20(a0)	
	moveq		#1,d0					// mov     eax,1  //return true when z>0
	move.l	a1,d3
	rts
 }
#endif
 
// MLA - all the divide exception handler overflow stuff was removed, and checked before
// each divide.  So all of this stuf isn't needed
/*
        public  proj_div_0,proj_div_1,divide_overflow_3d
ifdef  stereo_on
        public  proj_div_2,divide_overflow_r3d
endif

//this gets called by the system divide overflow handler when there is an
//overflow at proj_div_0, proj_div_1
divide_overflow_3d:
//       fall    project_overflow
project_overflow:       
        or      [edi].codes,CC_CLIP_OVERFLOW

ifdef stereo_on
        test    _g3d_stereo,1
        jz      no_stereo3
        add     edi,_g3d_stereo_base
divide_overflow_r3d:
        or      [edi].codes,CC_CLIP_OVERFLOW
        sub     edi,_g3d_stereo_base
no_stereo3:
endif
                                      
	cspew   "!"     //"project overflow!"
        // this did not use to restore this
        ex_set_div_action esi
        pop     esi
	ret
*/
 
//takes esi=ptr to array of vectors, edi=ptr to list for point handles, ecx=count
g3s_codes g3_transform_list(short n, g3s_phandle *dest_list, g3s_vector *v)
 {
 	int					i;
 	g3s_phandle	temphand;
 	
 	g_codes.or = 0;
 	g_codes.and = 0xff;
 	
 	for (i = n; i--; i>0)
 	 {
 	 	temphand = g3_transform_point(v++);
 	 	g_codes.or |= temphand->codes;
 	 	g_codes.and &= temphand->codes;
 	 	
 	 	*(dest_list++) = temphand;
 	 }
 	return(g_codes);
 }

//takes esi=ptr to array of vectors, edi=ptr to list for point handles, ecx=count
//returns bh=codes and, bl=codes or
g3s_codes g3_rotate_list(short n,g3s_phandle *dest_list,g3s_vector *v)
 {
 	int					i;
 	g3s_phandle	temphand;
 	
 	g_codes.or = 0;
 	g_codes.and = 0xff;

 	for (i = n; i--; i>0)
 	 {
 	 	temphand = g3_rotate_point(v++);
 	 	g_codes.or |= temphand->codes;
 	 	g_codes.and &= temphand->codes;
 	 	
 	 	*(dest_list++) = temphand;
 	 }
 	return(g_codes);
 }
 

//takes esi=ptr to array of point handles, ecx=count
g3s_codes g3_project_list(short n,g3s_phandle *point_list)
 {
 	int					i;
 	g3s_phandle	temphand;
 	
 	g_codes.or = 0;
 	g_codes.and = 0xff;

 	for (i = n; i--; i>0)
 	 {
		temphand = *(point_list++);
		g_codes.or |= temphand->codes;
		g_codes.and &= temphand->codes;
	
		g3_project_point(temphand);
	 }
	
 	return(g_codes);
 }

//takes esi=ptr to array of vectors, edi=ptr to dest vectors
g3s_phandle g3_rotate_light_norm(g3s_vector *v)
 {
 	g3s_point 	*point;

	getpnt(point);
 	do_rotate(v->gX, v->gY, v->gZ, &point->gX, &point->gY, &point->gZ);
 	return(point);
 }

//takes esi=ptr to normal vector. returns in <ecx,esi,eax>. trashes all regs
void rotate_norm(g3s_vector *v, fix *x, fix *y, fix *z)
 {
	do_norm_rotate(v->gX,v->gY,v->gZ,x,y,z);
 }


//does the rotate with the view matrix.
//takes <x,y,z> = <esi,edi,ebp>, returns <x,y,z> = <ecx,esi,eax>
void do_norm_rotate(fix x, fix y, fix z, fix *rx, fix *ry, fix *rz)
 {
	AWide	result,result2;

//this matrix multiply here will someday be optimized for zero and one terms
//uses unscaled rotation matrix.

//first column
	AsmWideMultiply(x, uvm1, &result);
	AsmWideMultiply(y, uvm4, &result2);
	AsmWideAdd(&result, &result2);
	AsmWideMultiply(z, uvm7, &result2);
	AsmWideAdd(&result, &result2);
	*rx = (result.hi<<16) | (((ulong) result.lo)>>16);

//second column
	AsmWideMultiply(x, uvm2, &result);
	AsmWideMultiply(y, uvm5, &result2);
	AsmWideAdd(&result, &result2);
	AsmWideMultiply(z, uvm8, &result2);
	AsmWideAdd(&result, &result2);
	*ry = (result.hi<<16) | (((ulong) result.lo)>>16);

//third column
	AsmWideMultiply(x, uvm3, &result);
	AsmWideMultiply(y, uvm6, &result2);
	AsmWideAdd(&result, &result2);
	AsmWideMultiply(z, uvm9, &result2);
	AsmWideAdd(&result, &result2);
	*rz = (result.hi<<16) | (((ulong) result.lo)>>16);
 }
 
// made this a define - MLA 
/*//takes esi=ptr to vector. returns <x,y,z> in <ecx,esi,eax>. trashes all regs
void xlate_rotate_point(g3s_vector *v, fix *x, fix *y, fix *z)     
 {
 	do_rotate(v->gX-_view_position.gX, v->gY-_view_position.gY, v->gZ-_view_position.gZ,x,y,z);
 }*/
 
//does the rotate with the view matrix.
//takes <x,y,z> = <esi,edi,ebp>, returns <x,y,z> = <ecx,esi,eax>
#if (defined(powerc) || defined(__powerc))	
void do_rotate(fix x, fix y, fix z, fix *rx, fix *ry, fix *rz)
 {
 	AWide 	result,result2;
//this matrix multiply here will someday be optimized for zero and one terms

//first column
	AsmWideMultiply(x, vm1, &result);
	AsmWideMultiply(y, vm4, &result2);
	AsmWideAdd(&result, &result2);
	AsmWideMultiply(z, vm7, &result2);
	AsmWideAdd(&result, &result2);
	*rx = (result.hi<<16) | (((ulong) result.lo)>>16);

//second column
	AsmWideMultiply(x, vm2, &result);
	AsmWideMultiply(y, vm5, &result2);
	AsmWideAdd(&result, &result2);
	AsmWideMultiply(z, vm8, &result2);
	AsmWideAdd(&result, &result2);
	*ry = (result.hi<<16) | (((ulong) result.lo)>>16);

//third column
	AsmWideMultiply(x, vm3, &result);
	AsmWideMultiply(y, vm6, &result2);
	AsmWideAdd(&result, &result2);
	AsmWideMultiply(z, vm9, &result2);
	AsmWideAdd(&result, &result2);
	*rz = (result.hi<<16) | (((ulong) result.lo)>>16);
 }
#else
asm void do_rotate(fix x, fix y, fix z, fix *rx, fix *ry, fix *rz)
 { 
 	movem.l	d3-d7,-(sp)
 	
 	move.l	24(sp),d5
 	move.l	28(sp),d6
 	move.l	32(sp),d7
	
//first column
	move.l	d5,d0
	move.l	vm1,d2
	dc.l		0x4C020C01			//	muls.l	d2,d1:d0
	
	move.l	d6,d3
	move.l	vm4,d2
	dc.l		0x4C023C04			// 	muls.l	d2,d4:d3
	add.l		d3,d0
	addx.l	d4,d1

	move.l	d7,d3
	move.l	vm7,d2
	dc.l		0x4C023C04			// 	muls.l	d2,d4:d3
	add.l		d3,d0
	addx.l	d4,d1

	move.w	d1,d0
	swap		d0
	move.l	36(sp),a0
	move.l	d0,(a0)
	
//second column
	move.l	d5,d0
	move.l	vm2,d2
	dc.l		0x4C020C01			//	muls.l	d2,d1:d0
	
	move.l	d6,d3
	move.l	vm5,d2
	dc.l		0x4C023C04			// 	muls.l	d2,d4:d3
	add.l		d3,d0
	addx.l	d4,d1

	move.l	d7,d3
	move.l	vm8,d2
	dc.l		0x4C023C04			// 	muls.l	d2,d4:d3
	add.l		d3,d0
	addx.l	d4,d1

	move.w	d1,d0
	swap		d0
	move.l	40(sp),a0
	move.l	d0,(a0)

//third column
	move.l	d5,d0
	move.l	vm3,d2
	dc.l		0x4C020C01			//	muls.l	d2,d1:d0
	
	move.l	d6,d3
	move.l	vm6,d2
	dc.l		0x4C023C04			// 	muls.l	d2,d4:d3
	add.l		d3,d0
	addx.l	d4,d1

	move.l	d7,d3
	move.l	vm9,d2
	dc.l		0x4C023C04			// 	muls.l	d2,d4:d3
	add.l		d3,d0
	addx.l	d4,d1

	move.w	d1,d0
	swap		d0
	move.l	44(sp),a0
	move.l	d0,(a0)

 	movem.l	(sp)+,d3-d7
 	rts
 }
#endif 

//rotate an x delta. takes edi=dest vector, eax=dx
//trashes eax,ebx,edx
void g3_rotate_delta_x(g3s_vector *dest,fix dx)
 {
 	dest->gX = fix_mul(dx,vm1);
 	dest->gY = fix_mul(dx,vm2);
 	dest->gZ = fix_mul(dx,vm3);
 }
 
//rotate a y delta. takes edi=dest vector, eax=dy
//trashes eax,ebx,edx
void g3_rotate_delta_y(g3s_vector *dest,fix dy)
 {
 	dest->gX = fix_mul(dy,vm4);
 	dest->gY = fix_mul(dy,vm5);
 	dest->gZ = fix_mul(dy,vm6);
 }
 
//rotate a z delta. takes edi=dest vector, eax=dz
//trashes eax,ebx,edx
void g3_rotate_delta_z(g3s_vector *dest,fix dz)
 {
 	dest->gX = fix_mul(dz,vm7);
 	dest->gY = fix_mul(dz,vm8);
 	dest->gZ = fix_mul(dz,vm9);
 }
 
//rotate an xz delta. takes edi=dest vector, eax=dx, ebx=dz
//trashes eax,ebx,edx
void g3_rotate_delta_xz(g3s_vector *dest,fix dx,fix dz)
 {
	AWide	result,result2;

//first column
	AsmWideMultiply(dx, vm1, &result);
	AsmWideMultiply(dz, vm7, &result2);
	AsmWideAdd(&result, &result2);
	dest->gX = (result.hi<<16) | (((ulong) result.lo)>>16);

//second column
	AsmWideMultiply(dx, vm2, &result);
	AsmWideMultiply(dz, vm8, &result2);
	AsmWideAdd(&result, &result2);
	dest->gY = (result.hi<<16) | (((ulong) result.lo)>>16);

//third column
	AsmWideMultiply(dx, vm3, &result);
	AsmWideMultiply(dz, vm9, &result2);
	AsmWideAdd(&result, &result2);
	dest->gZ = (result.hi<<16) | (((ulong) result.lo)>>16);
 }

//rotate an xy delta. takes edi=dest vector, eax=dx, ebx=dy
//trashes eax,ebx,edx
void g3_rotate_delta_xy(g3s_vector *dest,fix dx,fix dy)
 {
	AWide	result,result2;

//first column
	AsmWideMultiply(dx, vm1, &result);
	AsmWideMultiply(dy, vm4, &result2);
	AsmWideAdd(&result, &result2);
	dest->gX = (result.hi<<16) | (((ulong) result.lo)>>16);

//second column
	AsmWideMultiply(dx, vm2, &result);
	AsmWideMultiply(dy, vm5, &result2);
	AsmWideAdd(&result, &result2);
	dest->gY = (result.hi<<16) | (((ulong) result.lo)>>16);

//third column
	AsmWideMultiply(dx, vm3, &result);
	AsmWideMultiply(dy, vm6, &result2);
	AsmWideAdd(&result, &result2);
	dest->gZ = (result.hi<<16) | (((ulong) result.lo)>>16);
 }

//rotate a yz delta. takes edi=dest vector, eax=dy, ebx=dz
//trashes eax,ebx,edx
void g3_rotate_delta_yz(g3s_vector *dest,fix dy,fix dz)
 {
	AWide	result,result2;

//first column
	AsmWideMultiply(dy, vm4, &result);
	AsmWideMultiply(dz, vm7, &result2);
	AsmWideAdd(&result, &result2);
	dest->gX = (result.hi<<16) | (((ulong) result.lo)>>16);

//second column
	AsmWideMultiply(dy, vm5, &result);
	AsmWideMultiply(dz, vm8, &result2);
	AsmWideAdd(&result, &result2);
	dest->gY = (result.hi<<16) | (((ulong) result.lo)>>16);

//third column
	AsmWideMultiply(dy, vm6, &result);
	AsmWideMultiply(dz, vm9, &result2);
	AsmWideAdd(&result, &result2);
	dest->gZ = (result.hi<<16) | (((ulong) result.lo)>>16);
 }

//rotate a delta vector. takes edi=dest, eax,ebx,ecx=dx,dy,dz
//trashes all but ebp,edi
void g3_rotate_delta_xyz(g3s_vector *dest,fix dx,fix dy,fix dz)
 {
 	do_rotate(dx, dy, dz, &dest->gX, &dest->gY, &dest->gZ);
 }

//rotate a delta vector. takes edi=dest, esi=src
//trashes all but ebp,edi
void g3_rotate_delta_v(g3s_vector *dest,g3s_vector *src)
 {
 	do_rotate(src->gX, src->gY, src->gZ, &dest->gX, &dest->gY, &dest->gZ);
 }

//like add_delta, but creates and returns a new point
//takes esi=src, ebx=delta, returns edi=new point
//trashes eax,ebx
g3s_phandle g3_copy_add_delta_v(g3s_phandle src, g3s_vector *delta)
 {
 	g3s_point 	*point;

	getpnt(point);
	point->gX = src->gX + delta->gX;
	point->gY = src->gY + delta->gY;
	point->gZ = src->gZ + delta->gZ;
	point->p3_flags = 0;
	code_point(point);
	return(point);
 }
 
//adds a delta vector (created by rotate delta) to a point
//takes edi=point, esi=delta. clears projected bit, computes codes 
//trashes eax,esi,bl
void g3_add_delta_v(g3s_phandle p, g3s_vector *delta)
 {
 	p->gX += delta->gX;
 	p->gY += delta->gY;
 	p->gZ += delta->gZ;
 	
	p->p3_flags &= ~PF_PROJECTED;
	code_point(p);
 }
 
//add an x delta to a point. takes edi=point, eax=dx
//trashes eax,ebx,edx
void g3_add_delta_x(g3s_phandle p, fix dx)
 {
 	p->gX += fix_mul(vm1,dx);
 	p->gY += fix_mul(vm2,dx);
 	p->gZ += fix_mul(vm3,dx);
 	p->p3_flags &= ~PF_PROJECTED;
 	
 	code_point(p);
 }

//add a y delta to a point. takes edi=point, eax=dy
//trashes eax,ebx,edx
void g3_add_delta_y(g3s_phandle p, fix dy)
 {
 	p->gX += fix_mul(vm4,dy);
 	p->gY += fix_mul(vm5,dy);
 	p->gZ += fix_mul(vm6,dy);
 	p->p3_flags &= ~PF_PROJECTED;
 	
 	code_point(p);
 }

//add a z delta to a point. takes edi=point, eax=dz
//trashes eax,ebx,edx
void g3_add_delta_z(g3s_phandle p, fix dz)
 {
 	p->gX += fix_mul(vm7,dz);
 	p->gY += fix_mul(vm8,dz);
 	p->gZ += fix_mul(vm9,dz);
 	p->p3_flags &= ~PF_PROJECTED;
 	
 	code_point(p);
 }


//add an xy delta to a point. takes edi=point, eax=dx, ebx=dy
//trashes eax,ebx,ecx,edx,esi
void g3_add_delta_xy(g3s_phandle p,fix dx,fix dy)
 {
 	AWide 	result,result2;
 	
//first column
	AsmWideMultiply(dx, vm1, &result);
	AsmWideMultiply(dy, vm4, &result2);
	AsmWideAdd(&result, &result2);
	p->gX += (result.hi<<16) | (((ulong) result.lo)>>16);

//second column
	AsmWideMultiply(dx, vm2, &result);
	AsmWideMultiply(dy, vm5, &result2);
	AsmWideAdd(&result, &result2);
	p->gY += (result.hi<<16) | (((ulong) result.lo)>>16);

//third column
	AsmWideMultiply(dx, vm3, &result);
	AsmWideMultiply(dy, vm6, &result2);
	AsmWideAdd(&result, &result2);
	p->gZ += (result.hi<<16) | (((ulong) result.lo)>>16);

 	p->p3_flags &= ~PF_PROJECTED;
 	code_point(p);
 }

//add an xz delta to a point. takes edi=point, eax=dx, ebx=dz
//trashes eax,ebx,ecx,edx,esi
void g3_add_delta_xz(g3s_phandle p,fix dx,fix dz)
 { 
 	AWide 	result,result2;
 	
//first column
	AsmWideMultiply(dx, vm1, &result);
	AsmWideMultiply(dz, vm7, &result2);
	AsmWideAdd(&result, &result2);
	p->gX += (result.hi<<16) | (((ulong) result.lo)>>16);

//second column
	AsmWideMultiply(dx, vm2, &result);
	AsmWideMultiply(dz, vm8, &result2);
	AsmWideAdd(&result, &result2);
	p->gY += (result.hi<<16) | (((ulong) result.lo)>>16);

//third column
	AsmWideMultiply(dx, vm3, &result);
	AsmWideMultiply(dz, vm9, &result2);
	AsmWideAdd(&result, &result2);
	p->gZ += (result.hi<<16) | (((ulong) result.lo)>>16);

 	p->p3_flags &= ~PF_PROJECTED;
 	code_point(p);
 }


//add an yz delta to a point. takes edi=point, eax=dy, ebx=dz
//trashes eax,ebx,ecx,edx,esi
void g3_add_delta_yz(g3s_phandle p,fix dy,fix dz) 
 {
 	AWide 	result,result2;
 	
//first column
	AsmWideMultiply(dy, vm4, &result);
	AsmWideMultiply(dz, vm7, &result2);
	AsmWideAdd(&result, &result2);
	p->gX += (result.hi<<16) | (((ulong) result.lo)>>16);

//second column
	AsmWideMultiply(dy, vm5, &result);
	AsmWideMultiply(dz, vm8, &result2);
	AsmWideAdd(&result, &result2);
	p->gY += (result.hi<<16) | (((ulong) result.lo)>>16);

//third column
	AsmWideMultiply(dy, vm6, &result);
	AsmWideMultiply(dz, vm9, &result2);
	AsmWideAdd(&result, &result2);
	p->gZ += (result.hi<<16) | (((ulong) result.lo)>>16);

 	p->p3_flags &= ~PF_PROJECTED;
 	code_point(p);
 }

//add an xyz delta to a point. takes edi=point, eax=dx, ebx=dy, ecx=dz
//trashes eax,ebx,ecx,edx,esi
void g3_add_delta_xyz(g3s_phandle p,fix dx,fix dy,fix dz)
 {
 	fix 	rx,ry,rz;
 	
 	do_rotate(dx, dy, dz, &rx, &ry, &rz);
 	
 	p->gX += rx;
 	p->gY += ry;
 	p->gZ += rz;

 	p->p3_flags &= ~PF_PROJECTED;
 	code_point(p);
 }
  	

//like add_delta, but creates and returns a new point in edi
//add an x delta to a point. takes esi=point, eax=dx
//trashes eax,ebx,edx
g3s_phandle g3_copy_add_delta_x(g3s_phandle src, fix dx)
 {
 	g3s_point 	*point;

	getpnt(point);
	point->gX = src->gX + fix_mul(dx,vm1); 
	point->gY = src->gY + fix_mul(dx,vm2); 
	point->gZ = src->gZ + fix_mul(dx,vm3); 
	point->p3_flags = 0;
	code_point(point);
	return(point);
 }


//like add_delta, but creates and returns a new point in edi
//add a y delta to a point. takes esi=point, eax=dy
//trashes eax,ebx,edx
g3s_phandle g3_copy_add_delta_y(g3s_phandle src, fix dy)
 {
 	g3s_point 	*point;

	getpnt(point);
	point->gX = src->gX + fix_mul(dy,vm4); 
	point->gY = src->gY + fix_mul(dy,vm5); 
	point->gZ = src->gZ + fix_mul(dy,vm6); 
	point->p3_flags = 0;
	code_point(point);
	return(point);
 }

//like add_delta, but creates and returns a new point in edi
//add a z delta to a point. takes esi=point, eax=dz
//trashes eax,ebx,edx
g3s_phandle g3_copy_add_delta_z(g3s_phandle src,fix dz)
 {
 	g3s_point 	*point;

	getpnt(point);
	point->gX = src->gX + fix_mul(dz,vm7); 
	point->gY = src->gY + fix_mul(dz,vm8); 
	point->gZ = src->gZ + fix_mul(dz,vm9); 
	point->p3_flags = 0;
	code_point(point);
	return(point);
 }

//like add_delta, but modifies an existing point in edi
//add an x delta to a point. takes esi=src point, edi=replace point, ax=dx
//trashes eax,ebx,edx
g3s_phandle g3_replace_add_delta_x(g3s_phandle src, g3s_phandle dst, fix dx)
 {
	dst->gX = src->gX + fix_mul(dx,vm1); 
	dst->gY = src->gY + fix_mul(dx,vm2); 
	dst->gZ = src->gZ + fix_mul(dx,vm3); 
	dst->p3_flags = 0;
	code_point(dst);
	return(dst);
 }


//like add_delta, but modifies an existing point in edi
//add a y delta to a point. takes esi=src point, edi=replace point, ax=dy
//trashes eax,ebx,edx
g3s_phandle g3_replace_add_delta_y(g3s_phandle src, g3s_phandle dst, fix dy)
 {
	dst->gX = src->gX + fix_mul(dy,vm4); 
	dst->gY = src->gY + fix_mul(dy,vm5); 
	dst->gZ = src->gZ + fix_mul(dy,vm6); 
	dst->p3_flags = 0;
	code_point(dst);
	return(dst);
 }

//like add_delta, but modifies an existing point in edi
//add a z delta to a point. takes esi=src point, edi=replace point, ax=dz
//trashes eax,ebx,edx
g3s_phandle g3_replace_add_delta_z(g3s_phandle src, g3s_phandle dst, fix dz)
 {
	dst->gX = src->gX + fix_mul(dz,vm7); 
	dst->gY = src->gY + fix_mul(dz,vm8); 
	dst->gZ = src->gZ + fix_mul(dz,vm9); 
	dst->p3_flags = 0;
	code_point(dst);
	return(dst);
 }


//like add_delta, but creates and returns a new point in edi
//add an xy delta to a point. takes edi=point, eax=dx, ebx=dy
//trashes eax,ebx,ecx,edx,esi
g3s_phandle g3_copy_add_delta_xy(g3s_phandle src,fix dx,fix dy)
 {
 	g3s_point 	*point;
 	AWide 			result,result2;
	
	getpnt(point);
 	
//first column
	AsmWideMultiply(dx, vm1, &result);
	AsmWideMultiply(dy, vm4, &result2);
	AsmWideAdd(&result, &result2);
	point->gX = src->gX + ((result.hi<<16) | (((ulong) result.lo)>>16));

//second column
	AsmWideMultiply(dx, vm2, &result);
	AsmWideMultiply(dy, vm5, &result2);
	AsmWideAdd(&result, &result2);
	point->gY = src->gY + ((result.hi<<16) | (((ulong) result.lo)>>16));

//third column
	AsmWideMultiply(dx, vm3, &result);
	AsmWideMultiply(dy, vm6, &result2);
	AsmWideAdd(&result, &result2);
	point->gZ = src->gZ + ((result.hi<<16) | (((ulong) result.lo)>>16));

 	point->p3_flags = 0;
 	code_point(point);
	return(point);
 }

//like add_delta, but creates and returns a new point in edi
//add an xz delta to a point. takes edi=point, eax=dx, ebx=dz
//trashes eax,ebx,ecx,edx,esi
g3s_phandle g3_copy_add_delta_xz(g3s_phandle src,fix dx,fix dz)
 {
 	g3s_point 	*point;
 	AWide 			result,result2;
	
	getpnt(point);
 	
//first column
	AsmWideMultiply(dx, vm1, &result);
	AsmWideMultiply(dz, vm7, &result2);
	AsmWideAdd(&result, &result2);
	point->gX = src->gX + ((result.hi<<16) | (((ulong) result.lo)>>16));

//second column
	AsmWideMultiply(dx, vm2, &result);
	AsmWideMultiply(dz, vm8, &result2);
	AsmWideAdd(&result, &result2);
	point->gY = src->gY + ((result.hi<<16) | (((ulong) result.lo)>>16));

//third column
	AsmWideMultiply(dx, vm3, &result);
	AsmWideMultiply(dz, vm9, &result2);
	AsmWideAdd(&result, &result2);
	point->gZ = src->gZ + ((result.hi<<16) | (((ulong) result.lo)>>16));

 	point->p3_flags = 0;
 	code_point(point);
	return(point);
 }


//like add_delta, but creates and returns a new point in edi
//add an yz delta to a point. takes edi=point, eax=dy, ebx=dz
//trashes eax,ebx,ecx,edx,esi
g3s_phandle g3_copy_add_delta_yz(g3s_phandle src,fix dy,fix dz)
 {
 	g3s_point 	*point;
 	AWide 			result,result2;
	
	getpnt(point);
 	
//first column
	AsmWideMultiply(dy, vm4, &result);
	AsmWideMultiply(dz, vm7, &result2);
	AsmWideAdd(&result, &result2);
	point->gX = src->gX + ((result.hi<<16) | (((ulong) result.lo)>>16));

//second column
	AsmWideMultiply(dy, vm5, &result);
	AsmWideMultiply(dz, vm8, &result2);
	AsmWideAdd(&result, &result2);
	point->gY = src->gY + ((result.hi<<16) | (((ulong) result.lo)>>16));

//third column
	AsmWideMultiply(dy, vm6, &result);
	AsmWideMultiply(dz, vm9, &result2);
	AsmWideAdd(&result, &result2);
	point->gZ = src->gZ + ((result.hi<<16) | (((ulong) result.lo)>>16));

 	point->p3_flags = 0;
 	code_point(point);
	return(point);
 }

//like add_delta, but creates and returns a new point in edi
//add an xyz delta to a point. takes edi=point, eax=dx, ebx=dy, ecx=dz
//trashes eax,ebx,ecx,edx,esi
g3s_phandle g3_copy_add_delta_xyz(g3s_phandle src,fix dx,fix dy,fix dz)
 {
 	fix 				rx,ry,rz;
 	g3s_point 	*point;
 	
	getpnt(point);
 	do_rotate(dx, dy, dz, &point->gX, &point->gY, &point->gZ);
 	
 	point->gX += src->gX;
 	point->gY += src->gY;
 	point->gZ += src->gZ;

 	point->p3_flags = 0;
 	code_point(point);
	return(point);
 }

