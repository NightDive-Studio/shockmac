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
// $Source: r:/prj/lib/src/3d/RCS/alloc.c $
// $Revision: 1.19 $
// $Author: jaemz $
// $Date: 1994/09/28 19:01:01 $
//
// Point allocation, system init and shutdown
// 

#include "lg.h"
#include "3d.h"
#include "GlobalV.h"

//----------------------------------------------------------------------------
// Function: short g3_init(short max_points,int user_x_axis,int user_y_axis,int user_z_axis)
//
// Starts up the 3d system, allocating the requested number of points,
//  installing the divide overflow handler, and setting up the axes
//
// Input:	number of points requested
//				  axis numbers
// Output:	number actually allocated
// Side effects:	allocates point array
//----------------------------------------------------------------------------
short g3_init(short max_points,int user_x_axis,int user_y_axis,int user_z_axis)
 {
  int 	temp_user_y_axis;
 	long	temp_long;
 	char	temp_char;
 	long	allocSize;
	char 	temp_neg_flags[3]={0,0,0};
 	
#ifdef stereo_on
		extn divide_overflow_r3d, proj_div_2
#endif

// set axis neg flags
	axis_swap_flag = 0;			// mov	axis_swap_flag,0
	axis_neg_flag	= 0;			// mov	axis_neg_flag,0

	temp_user_y_axis = user_y_axis;	//	push	ecx	;save y axis
	if (user_x_axis<0)							// 	or	ebx,ebx	;check sign jns	no_neg_x
	 {
	 	user_x_axis = -user_x_axis;		// neg	ebx
	 	temp_neg_flags[0] = 1;						//	mov	temp_neg_flags,1
   }
	axis_x = user_x_axis;						// no_neg_x:	mov	axis_x,ebx
	
	if (user_y_axis<0)							//  or	ecx,ecx	;check sign		jns	no_neg_y
	 {
		user_y_axis = -user_y_axis;		// neg	ecx
		temp_neg_flags[1] = 1;				// mov	temp_neg_flags+1,1
	 }
	axis_y = user_y_axis;						// no_neg_y:	mov	axis_y,ecx

	if (user_z_axis<0)							//  or	edx,edx	;check sign  jns	no_neg_z 
	 {
		user_z_axis = -user_z_axis;		// neg	edx
		temp_neg_flags[2] = 1;				// mov	temp_neg_flags+2,1
	 }
	axis_z = user_z_axis;						// no_neg_z:	mov	axis_z,edx

// set axis swap flags
	if (user_x_axis>=user_y_axis)		// cmp	ebx,ecx	;check swap x,y			jl	no_swap_xy
	 {
	 	axis_swap_flag |= 1;					//	or	axis_swap_flag,1
	 	temp_long = user_x_axis;
	 	user_x_axis = user_y_axis;
	 	user_y_axis = temp_long;				// 	xchg	ebx,ecx
	 	
	 	temp_char = temp_neg_flags[0];
	 	temp_neg_flags[0]=temp_neg_flags[1];
	 	temp_neg_flags[1] = temp_char;	// mswap	temp_neg_flags,temp_neg_flags+1
	 }

	if (user_x_axis>=user_z_axis)		// cmp	ebx,edx	;check swap x,z			jl	no_swap_xy
	 {
	 	axis_swap_flag |= 2;					//	or	axis_swap_flag,1
	 	temp_long = user_x_axis;
	 	user_x_axis = user_z_axis;
	 	user_z_axis = temp_long;				// 	xchg	ebx,edx
	 	
	 	temp_char = temp_neg_flags[0];
	 	temp_neg_flags[0]=temp_neg_flags[2];
	 	temp_neg_flags[2] = temp_char;	// mswap	temp_neg_flags,temp_neg_flags+2
	 }

	if (user_y_axis>=user_z_axis)		// cmp	ecx,edx	;check swap y,z			jl	no_swap_xy
	 {
	 	axis_swap_flag |= 4;					//	or	axis_swap_flag,1	 	
	 	temp_char = temp_neg_flags[1];
	 	temp_neg_flags[1]=temp_neg_flags[2];
	 	temp_neg_flags[2] = temp_char;	// mswap	temp_neg_flags+1,temp_neg_flags+2
	 }
	 
// set neg flags bitmask
	axis_neg_flag = (temp_neg_flags[2]<<2) | (temp_neg_flags[1]<<1) | temp_neg_flags[0];
	
	user_y_axis = temp_user_y_axis-1;		// pop	ecx	;get back y axis
																			// 	dec	ecx	;make y axis 0,1,2
	up_axis = (user_y_axis<<1) + user_y_axis;
	
// set axis offset vars. offset is number of elements, not bytes
	axis_x_ofs = ((axis_x-1)<<1) + (axis_x-1);
	axis_y_ofs = ((axis_y-1)<<1) + (axis_y-1);
	axis_z_ofs = ((axis_z-1)<<1) + (axis_z-1);
	
// get pixel ratio
	pixel_ratio = grd_cap->aspect;

//now allocate point memory

//	_mark_	<initialize 3d system>
	allocSize = max_points*sizeof(g3s_point);
	
#ifdef stereo_on		// ; if stereo mode multiply by 2 at last moment
	if (_g3d_stereo_base)
	  allocSize<<=1;
#endif
 
  point_list = (g3s_point *) NewPtr(allocSize);
  if (!point_list) return(0);

// MLA - all divide overflow/divide by zero errors are handled around the individual divide
// instructions.
//
// Since we can't do divide overflow/zero traps on both the 68k and PPC.  

//install divide overflow callbacks
/*	mov	eax,EXM_DIVIDE_ERR      ;tell handler to do callbacks
	call	ex_startup_             ;install handler

	lea	eax,proj_div_0
	lea	edx,divide_overflow_3d
	call	ex_push_div_call_       ;callback for first pyr div

	lea	eax,proj_div_1
	lea	edx,divide_overflow_3d
	call	ex_push_div_call_       ;callback for 2nd pyr div

ifdef stereo_on
        ; save stereo_list
        mov     eax,point_list
        add     eax,_g3d_stereo_base
        mov     _g3d_stereo_list,eax

	lea	eax,proj_div_2
	lea	edx,divide_overflow_r3d
	call	ex_push_div_call_       ;callback for 2nd pyr div
endif
*/
	n_points = max_points;
	return(n_points);
 }


void g3_start_frame(void)
 { 
 	int				i;
 	g3s_point *pt3;
 	
//get pixel ratio again in case it's changed
	pixel_ratio = grd_cap->aspect;
	
//>>>>>>> 1.15
//set up window vars
	window_width = grd_canvas->bm.w;
	_biasx = _scrw = window_width << 15;
	ww2 = _scrw >> 16;

	window_height = grd_canvas->bm.h;
	_biasy = _scrh = window_height << 15;
	wh2 = _scrh >> 16;
	
//mark all points as free
	if (n_points) 
	 {
	 	first_free = point_list;
	 	pt3 = (g3s_point *) point_list;
	 	for (i=0; i<n_points-1; i++, pt3++)
	 	 	pt3->next = (g3s_phandle) (pt3+1);
	 	
	 	pt3->next = 0L;
	 }
 }

// shut down the 3d system
void g3_shutdown(void)
 {
 	if (point_list)
	 	DisposPtr((Ptr) point_list);

	n_points = 0;
	first_free = 0;

#ifdef stereo_on		
	_g3d_stereo_base = 0;
#endif	
 }

//;does extactly what you would think
int g3_count_free_points(void)
 {
 	int					i;
 	g3s_point 	*free_p = first_free;
 	
 	i=0;
 	while (free_p) 
 	 {
 	 	i++; 
 	 	free_p = free_p->next;
 	 }
 	return(i);
 } 
 
// check if all points free. returns number of points lost
int g3_end_frame(void)
 {
#ifdef stereo_on
        mov     _g3d_stereo,0    ; kill stereo for now
#endif
	return(g3_count_free_points()-n_points);
 }

// allocate a list of points
int g3_alloc_list(int n, g3s_phandle *p)
 {
 	int					i;
 	g3s_point 	*cur_ptr;
 	
 	if (!first_free) return(0);
 	cur_ptr = first_free;
 	
 	for (i=0; i<n; i++)
 	 {
 	 	p[i] = cur_ptr;
 	 	cur_ptr = cur_ptr->next;
 	 }
 	
 	first_free = cur_ptr;
 	return(n);
 }

// allocate one point, returning handle in ax
g3s_phandle g3_alloc_point(void)
 {
 	g3s_point 	*tempPtr;
 	
 	if (!first_free) return(0L);
 	tempPtr = first_free;
 	first_free = tempPtr->next;
 	return(tempPtr);
 }
 
// free the point in eax. trashes ebx
void g3_free_point(g3s_phandle p)      //adds to free list
 {
 	p->next = first_free;
 	first_free = p;
 }


// free the list of points pointed at by esi, count in ecx
void g3_free_list(int n_points, g3s_phandle *p) 	//adds to free list
 {
 	int					i;
 	g3s_phandle *gptr;
 	g3s_point 	*tempPtr;
 	
 	gptr = p;
 	for (i=0; i<n_points; i++)
 	 {
 	 	tempPtr = p[i];
	 	tempPtr->next = first_free;
	 	first_free = tempPtr;
 	 }
 }

// make a duplicate of a point. takes esi, returns edi. trashes ebx,ecx
g3s_phandle g3_dup_point(g3s_phandle p)                //makes copy of a point
 {
 	g3s_point 	*tempPtr,*destPtr;
 	
 	destPtr = first_free;
 	first_free = destPtr->next;

 	g3_copy_point(destPtr,p);
 	
 	return(destPtr);
 }

// copy point at esi to one at edi. trashes esi,ecx
void g3_copy_point(g3s_phandle dest,g3s_phandle src)
 {
 	*dest = *src;
 }
