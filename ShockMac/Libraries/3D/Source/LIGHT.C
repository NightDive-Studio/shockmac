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
// $Source: r:/prj/lib/src/3d/RCS/light.asm $
// $Revision: 1.2 $
// $Author: jaemz $
// $Date: 1994/10/13 20:51:49 $
//
// Light routines
// 

#include "lg.h"
#include "3d.h"
#include "GlobalV.h"

// prototypes
void check_for_near(void);
void scale_light_vec(void);
void g3_light_obj(g3s_phandle norm,g3s_phandle pos);
fix light_diff_raw(g3s_phandle src, g3s_phandle dest);
fix light_spec_raw(g3s_phandle src, g3s_phandle dest);
fix light_dands_raw(g3s_phandle src, g3s_phandle dest);

// look ma, a zero vector 
g3s_phandle tmp1;
g3s_phandle tmp2;
g3s_vector  zero_vec ={0,0,0};       


// sets a light vector in source space directly
// this light vector has to be in user space so we can dot it with
// other vector
// g3_set_light_src(g3s_vector *l)
// takes eax, trashes esi,edi
void g3_set_light_src(g3s_vector *l)
 {
 	_g3d_light_src = *l;
 }

// This should be called after a frames' angles and stuff have been
// set, does not need to be done per object
// void g3_eval_vec_light(void) 
// Means you should be not near 
void g3_eval_vec_light(void)
 {
	// needs to be rotated through view matrix 
 	g3_vec_rotate(&_g3d_light_vec,&_g3d_light_src,&_wtoo_matrix);
 	scale_light_vec();
 }

	// should be normalized already
	// multiply by _g3d_diff_light
	// to set light intensity
void scale_light_vec(void)
 {
 	_g3d_light_vec.gX = fix_mul(_g3d_light_vec.gX, _g3d_diff_light);
 	_g3d_light_vec.gY = fix_mul(_g3d_light_vec.gY, _g3d_diff_light);
 	_g3d_light_vec.gZ = fix_mul(_g3d_light_vec.gZ, _g3d_diff_light);
 }
 

// transforms local light source into viewer coords in anticipation
// of calling g3_eval_loc_light.  Saves a transformation don't you know
// ideally you'd want to transform the view vec and light vec into
// object coords.  That way you wouldn't have to transform their normals
// at all.  The new 3d should provide inverse transforms.  That way things
// could be lit more cheaply
void g3_trans_loc_light(void)
 {
 	g3s_phandle	 p;
 	
 	p = g3_rotate_point(&_g3d_light_src);
 	_g3d_light_trans = * (g3s_vector *) p;
 }

// evaluates light point relative to another point, src is in world coords
// and pos is already transformed into eye coords
// void g3_eval_loc_light(eax)//
void g3_eval_loc_light(g3s_phandle pos)
 {
 	fix			temp;
 	
  // transform light src point to eye coords

  // take difference with pos, and unscale them
	temp = -(pos->gX - _g3d_light_trans.gX);
	_g3d_light_vec.gX = fix_div(temp,_matrix_scale.gX);

	temp = -(pos->gY - _g3d_light_trans.gY);
	_g3d_light_vec.gY = fix_div(temp,_matrix_scale.gY);

	temp = -(pos->gZ - _g3d_light_trans.gZ);
	_g3d_light_vec.gZ = fix_div(temp,_matrix_scale.gZ);
	
  // normalize vector
	g3_vec_normalize(&_g3d_light_vec);

  // multiply by diff and divide by scale
  // so dot product just works
	scale_light_vec();
 }


// evaluate the view vector relative to a point
// similar to above except src is always 0,0,0
// pos in eax has to be transformed into viewer coords
// eax points to the point in 3d space
void g3_eval_view(g3s_phandle pos)
 {
 	fix temp;
 	
 	_g3d_view_vec.gX = pos->gX - _view_position.gX;
 	_g3d_view_vec.gY = pos->gY - _view_position.gY;
 	_g3d_view_vec.gZ = pos->gZ - _view_position.gZ;

  // normalize
 	g3_vec_normalize(&_g3d_view_vec);

	// multiply by spec and negate vector
  // since it currently points at the
  // point instead of the viewer
  // you might think, why not do this
  // afterwards, and you're right.  But only
  // only if this view vec only gets used once

	temp = -_g3d_spec_light;
	_g3d_view_vec.gX = fix_mul(_g3d_view_vec.gX,temp);
	_g3d_view_vec.gY = fix_mul(_g3d_view_vec.gY,temp);
	_g3d_view_vec.gZ = fix_mul(_g3d_view_vec.gZ,temp);
 }

// takes the dot product of view and light, for specular light
// assumes both light_vec and view_vec have been evaluated already
// everything is normal and in object space
// void g3_eval_ldotv(void)
void g3_eval_ldotv(void)
 {
  // multiply and scale once to find true dotproduct.  
  // I hate this scaling stuff.  Erg.
  // transforming the light and view vectors into object 
  // space would avoid this entirely
  
  _g3d_ldotv = g3_vec_dotprod(&_g3d_light_vec,&_g3d_view_vec);
 }

// Evaluates and sets light vectors as necessary at the start of
// an object.  Does view vec if SPEC is set.  Transforms light
// vector or point depending how LOC_LIGHT is set.  Use this if
// both light and view will be modelled as far.  In fact, make
// sure both are set as far, or you will be sorry.
// Evaluates at the object center.  If necessary, evaluates the
// ldotv for light and view
// void g3_eval_light_obj_cen(void)
// this could be optimized a bit more when both spec
// both spec and diff is true
// this should do eval vec as well, basically everything
void g3_eval_light_obj_cen(void)
 {
 	DebugStr("\pCall Mark if you see this");
 	
 	// MLA - this routine is buggy as far as I can tell, it doesn't work at all
 	// edi is never set or just happens to be set right, or it always falls
 	// through the first test (non_local)
 	
    /*    ; is local?
        test    _g3d_light_type,LT_LOC_LIGHT or LT_SPEC
        jz     non_local

        ; find center of object, duh, center is at 0,0,0
        ;lea     esi,zero_vec
        ;call    g3_rotate_point; this would be point in viewer space
        ; returns point in edi
        ;mov     tmp1,edi

        ; evaluate local light
        test    _g3d_light_type,LT_LOC_LIGHT
        jz      no_loc
        mov     eax,edi
        call    g3_eval_loc_light
        jmp     test_spec
        no_loc:
        call    g3_eval_vec_light

        ; is spec set?
        test_spec:
        test    _g3d_light_type,LT_SPEC
        jz     spec_done

        ; evaluate view
        ; view vector relative to zero is in _view_position
        ; already
        ;mov     eax,tmp1
        lea     eax,zero_vec    ;in reality should write a diff routine
        call    g3_eval_view

        call    g3_eval_ldotv

        spec_done:
        ;mov     edi,tmp1
        ;freepnt edi
        ret

        non_local:
        ; assumes you've transformed
        ; it already
        jmp    g3_eval_vec_light*/
 }


// set your view to be straight ahead, use when using specular,
// and after the light has been evaluated.  This is ultra cheap hack
// to get view vector for a whole scene, just points straight in
void g3_eval_view_ahead(void)
 {
  // this is in view coords, need in
  // object coords, this won't work
 	_g3d_view_vec.gX = 0;
 	_g3d_view_vec.gY = 0;
 	_g3d_view_vec.gZ = -0x01000;
 	
  // now eval ldotv, hm.
 	_g3d_ldotv = -_g3d_light_vec.gZ;
 }

// check to see if local stuff has to get set and
// set it if necessary
// takes args in tmp1,tmp2
void check_for_near(void)
 {
 	if ((_g3d_light_type & (LT_NEAR_VIEW | LT_NEAR_LIGHT) == 0))
 	 	return;

  // if light near, evaluate
 	if ((_g3d_light_type & LT_NEAR_LIGHT)!=0)
 	 	g3_eval_loc_light(tmp2);
 	 
  //if view near, eval
 	if ((_g3d_light_type & LT_NEAR_VIEW)!=0)
 	 	g3_eval_view(tmp2);

  //evaluate ldotv if either was local
  // MLA - this is stupid, the code that tests for whether or not to call 
	// g3_eval_ldotv makes no sense, it does a JZ on an undetermined condition code
	// setup.  So I just call it all the time. Look in Light.ASM in the PC 3D code for
	// the original stuff.
	g3_eval_ldotv();
 }


//void g3_light_diff(g3s_phandle norm,g3s_phandle pos)// 
//takes normal vector transformed, dots with the light vec, 
//puts light val in norm,
//takes args in [eax,edx]
void g3_light_diff(g3s_phandle norm, g3s_phandle pos)
 {
  // push eax if not gouraud, or edx if, so we know
  // whether to light the normal or the point
  // maybe we could make this self modifying based
  // on a light type setter, if this is slow

 	// MLA - whatever, I made it normal C code

	tmp1 = norm;
	tmp2 = pos;
	check_for_near();
	
	if ((_g3d_light_type & LT_GOUR) == 0)
		light_diff_raw(tmp1,norm);
	else
		light_diff_raw(tmp1,pos);
 }
 
// raw version
//dot product with normal
//esi and edi
//ret eax
fix light_diff_raw(g3s_phandle src, g3s_phandle dest)
{ 
	fix		temp;
	
	temp = g3_vec_dotprod(&_g3d_light_vec, (g3s_vector *) src);
	
  //set lighting value in norm
  //test eax for negativity, zero if negative
	if (temp<0) temp = 0;
	temp += _g3d_amb_light;	//add ambient light
	temp >>= 4;		//convert to sfix, consider row 16 normal
	dest->i = temp;
	return(temp);
 }


//void g3_light_spec(g3s_phandle norm,g3s_phandle pos)// 
//takes norm and point position, lights point
//could both be the same, of course [eax,edx]
void g3_light_spec(g3s_phandle norm,g3s_phandle pos)
 {
  // push eax if not gouraud, or edx if, so we know
  // whether to light the normal or the point
  // maybe we could make this self modifying based
  // on a light type setter, if this is slow

 	// MLA - whatever, I made it normal C code

  // save norm and pos off so we don't push and
  // pop them forever 
	tmp1 = norm;
	tmp2 = pos;
	check_for_near();
	
	if ((_g3d_light_type & LT_GOUR) == 0)
		light_spec_raw(tmp1,norm);
	else
		light_spec_raw(tmp1,pos);
 }
 
 
//pure specular lighting is equal to 
//2(s.l)(s.v) - (l.v)
//take (s.l)
fix light_spec_raw(g3s_phandle src, g3s_phandle dest)
 {
 	fix		temp;
 	
 	temp = g3_vec_dotprod(&_g3d_light_vec, (g3s_vector *) src);
	if (temp<0)
	 {
	 	dest->i = _g3d_amb_light>>4;
	 	return(dest->i);
	 }
	
	_g3d_sdotl = temp;
	
  //take (s.v), note that this is jnorm, if its been done
  //we can eliminate this step intelligently somehow
  _g3d_sdotv = temp = g3_vec_dotprod(&_g3d_view_vec, (g3s_vector *) tmp1);
	temp <<= 1;	 // multiply (s.v) by 2
	temp = fix_mul(temp,_g3d_sdotl);	// mult by (s.l)
	temp -= _g3d_ldotv;			 // subtract ldotv, done!

  //test eax for flash point zero if under
  //or better test eax for spec threshhold
  if (temp < _g3d_flash)
  	temp = 0;
  	
  // add ambient light
  temp += _g3d_amb_light;

  // check to see if its greater than the max row
  // and truncate if it is
  if (temp >= (LT_TABSIZE << 12));
  	temp = (LT_TABSIZE << 12) - 1;	// if its over the max, set it to just under max

	dest->i = temp>>4;	 //convert to sfix, consider row 16 normal
	return(temp);
 }


//void g3_light_dands(g3s_phandle norm,g3s_phandle pos)// 
//lights with both diff and spec 
//[eax,edx]
void g3_light_dands(g3s_phandle norm,g3s_phandle pos)
 {
	// MLA - same stuff as before, changed to C....
	tmp1 = norm;
	tmp2 = pos;
	check_for_near();
	
	if ((_g3d_light_type & LT_GOUR) == 0)
		light_dands_raw(tmp1,norm);
	else
		light_dands_raw(tmp1,pos);
 }
 
// raw version of dands without local checking
fix light_dands_raw(g3s_phandle src, g3s_phandle dest)
 {
 	fix		temp;

  //pure specular lighting is equal to 
  //2(s.l)(s.v) - (l.v)
  //take (s.l) if neg, you know you're done, surface HAS to face the light
  
 	temp = g3_vec_dotprod(&_g3d_light_vec, (g3s_vector *) src);
  if (temp<0)
   {
   	dest->i = _g3d_amb_light>>4;
   	return(dest->i);
   }
  _g3d_sdotl = temp;

  //take (s.v), note that this is jnorm, if its been done
  //we can eliminate this step intelligently somehow
	_g3d_sdotv = temp = g3_vec_dotprod(&_g3d_view_vec, (g3s_vector *) tmp1);
	temp = fix_mul(temp,_g3d_sdotl);      // mult by (s.l)
	temp<<=1;	// multiply (s.v)(s.l) by 2
	temp -= _g3d_ldotv;	// subtract ldotv, done!

  //test eax for flash point zero if under
  //or better test eax for spec threshhold
  if (temp < _g3d_flash)
  	temp = 0;
	
	// add diffuse component & ambient light
	temp += _g3d_sdotl + _g3d_amb_light;

  // check to see if its greater than the max row
  // and truncate if it is
  if (temp >= (LT_TABSIZE << 12))
  	temp = (LT_TABSIZE << 12) - 1;	// if its over the max, set it to just under max

	dest->i = temp>>4;	 //convert to sfix, consider row 16 normal
	return(temp);
 }

// farms out a point based on flags
//void g3_light(g3s_phandle norm,g3s_phandle pos)//
//[eax,edx]
fix g3_light(g3s_phandle norm,g3s_phandle pos)
 {
 	g3s_phandle	temp;

	if ((_g3d_light_type & LT_GOUR) == 0)
		temp = norm;
	else
		temp = pos; 	
 	
	tmp1 = norm;
	tmp2 = pos;
	
	if ((_g3d_light_type & (LT_NEAR_VIEW | LT_NEAR_LIGHT)) != 0)
		check_for_near();

  // determine which routine to jump to based on flags
  switch (_g3d_light_type)
   {
   	case LT_DIFF: return(light_diff_raw(tmp1, temp)); 
   	case LT_SPEC: return(light_spec_raw(tmp1, temp)); 
   	default: return(light_dands_raw(tmp1, temp)); 
   }
 }

// farms out a point based on flags
//void g3_light_obj(g3s_vector *norm,g3s_vector *pos)//
//[eax,edx]
//norm is set with only 15 bits of fraction, pos is normal
//all vectors are in object space
//though we put these in points, they are in object space,
//not world space
void g3_light_obj(g3s_phandle norm, g3s_phandle pos)
 {
	g3s_point 	*norm_point;
	g3s_point 	*pos_point;
	fix					shade;
	
	tmp1 = norm;
	tmp2 = pos;
	getpnt(norm_point);
 	
 	norm_point->gX = norm->gX<<1;
 	norm_point->gY = norm->gY<<1;
 	norm_point->gZ = norm->gZ<<1;
 	
  // Copy position over to its own point
	getpnt(pos_point);
	* (g3s_vector *) pos_point = * (g3s_vector *) pos;
	
	shade = g3_light(norm_point, pos_point);

  //set the lighting when non gouraud
  //set fill type to address of shading table
	shade &= 0xffffff00;
	shade += _g3d_light_tab;
	
	gr_set_fill_parm(shade);

	freepnt(norm_point);
	freepnt(pos_point);
 }


// generic list gronker, farms these points out
// call this inside an object or inside a frame
// at any rate, the points need to have been
// transformed
// g3_light_list(int n,g3s_phandle *norm,g3s_phandle *pos)
//  [eax,edx,ebx]
void g3_light_list(int n,g3s_phandle *norm,g3s_phandle *pos)
 {
 }
 
