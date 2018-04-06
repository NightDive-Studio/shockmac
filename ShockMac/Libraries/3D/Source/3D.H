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
 * $Source: r:/prj/lib/src/3d/RCS/3d.h $
 * $Revision: 1.35 $
 * $Author: jaemz $
 * $Date: 1994/09/20 13:21:45 $
 *
 * Header file for LookingGlass 3D library
 *
 * $Log: 3d.h $
 
 *------------------------------------------------------*
 * MAC VERSION NOTES
 * 
 *  All of the #pragma aux statements have been removed, because they only
 *  specify the register order parameters are passed into routines.  Since
 *  all this stuff has to work on both 68K and PowerPC machines, we have
 *  to use C calling conventions almost everwhere.
 *
 *------------------------------------------------------* 
 
 * Revision 1.35  1994/09/20  13:21:45  jaemz
 * Added lighting features, took out old call interpret_object
 * 
 * Revision 1.33  1994/08/28  14:48:05  kevin
 * Added linear mapping alternative for detail level stuff.
 * 
 * Revision 1.32  1994/08/18  03:45:08  jaemz
 * Added call for g3_object_scale
 * 
 * Revision 1.31  1994/07/21  00:28:25  jaemz
 * Added stereo declares
 * 
 * Revision 1.29  1994/06/08  21:20:12  jaemz
 * Commented transform matrix
 * 
 * Revision 1.28  1994/06/01  15:57:22  jaemz
 * Added a "*" to g3_compute_normal
 * 
 * Revision 1.27  1994/05/31  16:38:36  jaemz
 * Added documenting comment to .h file about matrices being
 * column based
 * 
 * Revision 1.26  1994/05/19  09:47:42  kevin
 * g3_light(draw)_t(l,floor_,wall_)map now use watcom register passing conventions.
 * 
 * Revision 1.25  1994/05/02  23:39:01  kevin
 * Added prototypes for wall and floor map procedures.
 * 
 * Revision 1.24  1994/02/08  20:48:06  kaboom
 * Added translucent polygon prototypes\pragmas.
 * 
 * Revision 1.23  1993/12/15  01:52:17  dc
 * alloc_list
 * 
 * Revision 1.22  1993/12/14  22:58:04  kevin
 * Added declarations for biasx,biasy, and perspective mapper context manipulation routines.
 * 
 * Revision 1.21  1993/12/11  04:03:12  kevin
 * Added declarations for g3_rotate_grad and _norm.
 * 
 * Revision 1.20  1993/12/06  15:51:25  unknown
 * c:\app\star24\vmode 132x44 uv mappers to take a phandle *, as they do
 * 
 * Revision 1.19  1993/12/04  17:00:02  kaboom
 * Added declarations for bitmap lighters.
 * 
 * Revision 1.18  1993/11/07  09:02:15  dc
 * support for replace_add_delta along axis
 * 
 * Revision 1.17  1993/10/22  09:37:11  kaboom
 * Added new prototypes and pragmas for linear map routines.
 * 
 * Revision 1.16  1993/10/02  11:01:36  kaboom
 * Changed names of clip_{line,polygon} to g3_clip_{line,polygon} to avoid
 * name collisions.
 * 
 * Revision 1.15  1993/10/02  09:11:30  kaboom
 * Added declarations for scrw,scrh.  Added prototypes&pragmas for new
 * clipping routines and g3_{draw,light}_tmap.  New inline for g3_set_vcolor.
 * 
 * Revision 1.14  1993/08/11  15:02:42  kaboom
 * Added prototypes and pragmas for lighting texture mappers.
 * 
 * Revision 1.13  1993/08/04  00:48:39  dc
 * move interpreter zaniness to separate file
 * 
 * Revision 1.12  1993/07/08  23:36:08  kaboom
 * Added prototypes for g3_bitmap(), g3_anchor_bitmap(), and
 * g3_set_bitmap_scale().
 * 
 * Revision 1.11  1993/06/30  11:20:01  spaz
 * Added prototypes for g3_draw_cline, g3_draw_sline
 * 
 * Revision 1.10  1993/06/18  15:47:24  kaboom
 * Updated prototype for g3_project_point to reflect return value.
 * 
 * Revision 1.9  1993/06/09  04:23:46  kaboom
 * Changed prototype and comment for g3_draw_tmap_tile to reflect new usage.
 * 
 * Revision 1.8  1993/06/04  16:53:29  matt
 * Added hacks to get around c++ non-support of anonymous structures in unions.
 * 
 * Revision 1.7  1993/05/24  15:51:04  matt
 * Added g3_get_slew_step(), removed g3_draw_smooth_horizon(), changed a few
 * comments.
 * 
 * Revision 1.6  1993/05/21  16:09:17  matt
 * Added new way to specifiy axis orientation, which may be more intuitive
 * 
 * Revision 1.5  1993/05/13  12:17:08  matt
 * Added new function, g3_check_codes()
 * 
 * Revision 1.4  1993/05/11  15:23:49  matt
 * Changed comment for g3_draw_tmap_tile() to reflect new functionality
 * 
 * Revision 1.3  1993/05/11  14:57:53  matt
 * Changed g3_vec_scale() to takes seperate dest & src.
 * Added g3_get_view_pyramid().
 * 
 * Revision 1.2  1993/05/10  13:25:14  matt
 * Added the ability to access the elements of a vector as an array. Fields 
 * x,y,z can be accessed as xyz[0..2].
 * 
 * Revision 1.1  1993/05/04  17:39:56  matt
 * Initial revision
 */

#ifndef __3D_H
#define __3D_H

#include "fix.h"
#include "2d.h"

// MLA defines
#define SwapFix(x,y) {fix temp = (x); (x) = (y); (y) = temp;}
#define vm1 view_matrix.m1
#define vm2 view_matrix.m2
#define vm3 view_matrix.m3
#define vm4 view_matrix.m4
#define vm5 view_matrix.m5
#define vm6 view_matrix.m6
#define vm7 view_matrix.m7
#define vm8 view_matrix.m8
#define vm9 view_matrix.m9

#define uvm1 unscaled_matrix.m1
#define uvm2 unscaled_matrix.m2
#define uvm3 unscaled_matrix.m3
#define uvm4 unscaled_matrix.m4
#define uvm5 unscaled_matrix.m5
#define uvm6 unscaled_matrix.m6
#define uvm7 unscaled_matrix.m7
#define uvm8 unscaled_matrix.m8
#define uvm9 unscaled_matrix.m9


// Defines for stuff strewn all about the world
// #define stereo_on 1

#define f1_0	fix_make(1,0)
#define f0_5	fix_make(0,0x8000)
#define f0_25	fix_make(0,0x4000)


// gets the next available pnt in reg.  
#define getpnt(res) {g3s_point *scratch; if (res = first_free) {scratch = res->next; first_free = scratch;}}

// frees the point in the specified register. uses ebx as scratch
#define freepnt(src) {g3s_point *scratch = first_free; src->next = scratch; first_free=src;}

//new types:
typedef short sfix;

#define sfix_make(a,b) ((((short)(a))<<8)|(b))
#define fix_from_sfix(a) (((fix )(a))<<8)

#define g3_set_i(pnt,ii) do {pnt->i = sfix_make(ii,0); pnt->p3_flags |= PF_I;} while (0);
#define g3_set_rgb(pnt,r,g,b) do {pnt->rgb = gr_bind_rgb(r,g,b); pnt->p3_flags |= PF_RGB;} while (0);

//constants

//these are for rotation order when generating a matrix from angles
//bit 2 means y before z if 0, 1 means x before z if 0, 0 means x before y if 0

#define ORDER_XYZ  0    //000
#define ORDER_YXZ  1    //001
#define ORDER_YZX  3    //011
#define ORDER_XZY  4    //100
#define ORDER_ZXY  6    //110
#define ORDER_ZYX  7    //111

//To specify user's coordinate system: use one of these for each user_x_axis,
//user_y_axis,& user_z_axis in g3_init to tell the 3d what your x,y,& z mean.

#define AXIS_RIGHT  1
#define AXIS_UP     2
#define AXIS_IN     3
#define AXIS_LEFT  -AXIS_RIGHT
#define AXIS_DOWN  -AXIS_UP
#define AXIS_OUT   -AXIS_IN

//vectors, points, matrices

// NIGHTMARE CODE!!!!!!  ANONYMOUS UNIONS ARE BAAAAAAAD THINGS MAN
// MLA - left in the #ifdef so the C++ compiler handles it, we just have to change
// references of g3s_vector.x to g3s_vector.xyz[0] (or .gX, .gY, etc. from the #defines)
typedef struct g3s_vector {
#if 0 // MLA #ifdef __cplusplus
	fix x,y,z;
#else
	union {
		struct {fix x,y,z;};
		fix xyz[3];
	};
#endif
} g3s_vector;

#define gX xyz[0]
#define gY xyz[1]
#define gZ xyz[2]


typedef struct g3s_angvec {fixang tx,ty,tz;} g3s_angvec;

// This transformation matrix is row based, ie
//|m1 m2 m3|   |x|   |x'|
//|m4 m5 m6| * |y| = |y'|
//|m7 m8 m9|   |z|   |z'|
//
// but of course the incoming y coordinates
// are inverted, have a nice day
typedef struct g3s_matrix {
	fix m1,m2,m3,m4,m5,m6,m7,m8,m9;
} g3s_matrix;

//typedef short g3s_phandle;    //used to refer to points, equal to pntnum * 4
typedef struct g3s_point *g3s_phandle;

typedef struct g3s_point {
#if 0 // #ifdef __cplusplus
	fix x,y,z;
#else
	union {                                         //rotated 3d coords, use as vector or elements
		g3s_vector vec; 
		struct {fix x,y,z;};
		fix xyz[3];
    g3s_phandle next; //next in free list, when point is unused
	};
#endif

	fix sx,sy;                                      //screen coords
	byte codes;                                     //clip codes
	ubyte p3_flags;                 //misc flags
#if 0 // #ifdef __cplusplus
	sfix u,v;
#else
	union {
		struct {sfix u,v;} uv;     //for texturing, etc.
		grs_rgb rgb;                    //for RGB-space gouraud shading
	};
#endif
	sfix i;                                         //gouraud shading & lighting
} g3s_point;


//clip codes
#define CC_OFF_LEFT  1
#define CC_OFF_RIGHT 2 
#define CC_OFF_BOT   4
#define CC_OFF_TOP   8
#define CC_BEHIND    128
#define CC_CLIP_OVERFLOW     16

//flags for the point structure
#define PF_U                    1               //is u value used?
#define PF_V                    2               //is v value used?
#define PF_I                    4               //is i value used?
#define PF_PROJECTED    8               //has this point been projected?
#define PF_RGB                  16              //are the RBG values used?
#define PF_CLIPPNT      32              //this point created by clipper
#define PF_LIT       64    //has this point been lit by the lighter?


// lighting codes
#define LT_NONE    0
#define LT_SOLID   1
#define LT_DIFF    2
#define LT_SPEC    4
#define LT_GOUR    128

#define LT_NEAR_LIGHT 16  // TRUE if light is near and has to be evaluated
#define LT_NEAR_VIEW  8  // TRUE if viewing point is near, and has to be reevaled
#define LT_LOC_LIGHT  32  // TRUE if light is a local point, not a vector
#define LT_TABSIZE 24  // size of the shading table
#define LT_BASELIT 15  // table entry of normal intensity (before saturating)


extern fix scrw,scrh;
extern fix biasx,biasy;

extern ubyte   g3d_light_type;
extern fix     g3d_amb_light,g3d_diff_light,g3d_spec_light;
extern fix     g3d_ldotv,g3d_sdotl,g3d_sdotv,g3d_flash;
extern ubyte   *g3d_light_tab;
extern g3s_vector g3d_light_src,g3d_light_trans;
extern g3s_vector g3d_light_src,g3d_light_trans;
extern g3s_vector g3d_view_vec,g3d_light_vec;


typedef struct g3s_codes {byte or,and;} g3s_codes;

/*
 *      We're going to want a bunch of general-purpose 3d vector math
 *      routines. These are:
 *
 *      Question:  some of these take a destination, and some change the
 *              vector passed to them. Should we adopt a consistent interface?
 *
 * There's sure to be more of these
 *
 */
void g3_vec_sub(g3s_vector *dest,g3s_vector *src1,g3s_vector *src2);
void g3_vec_add(g3s_vector *dest,g3s_vector *src1,g3s_vector *src2);

fix g3_vec_mag(g3s_vector *v);
void g3_vec_scale(g3s_vector *dest,g3s_vector *src,fix s);
void g3_vec_normalize(g3s_vector *v);

void  g3_compute_normal(g3s_vector *norm,g3s_vector *v0,g3s_vector *v1,g3s_vector *v2);

fix g3_vec_dotprod(g3s_vector *v0,g3s_vector *v1);

void  g3_vec_rotate(g3s_vector *dest,g3s_vector *src,g3s_matrix *m);
	//src and dest can be the same

void g3_transpose(g3s_matrix *m);       //transpose in place
void g3_copy_transpose(g3s_matrix *dest,g3s_matrix *src);       //copy and transpose
void g3_matrix_x_matrix(g3s_matrix *dest,g3s_matrix *src1,g3s_matrix *src2);

int g3_clip_line(g3s_point *src[],g3s_point *dest[]);
// MLA #pragma aux g3_clip_line parm [esi] [edi] modify [eax ebx ecx edx esi edi];
int g3_clip_polygon(int n,g3s_point *src[],g3s_point *dest[]);
// MLA #pragma aux g3_clip_polygon parm [ecx] [esi] [edi] modify [eax ebx ecx edx esi edi];

/*
 *      Graphics-specific 3d routines
 *
 */

// stereo functions
short g3_init_stereo(short max_points,int user_x_axis,int user_y_axis,int user_z_axis);
   // sets up system for stereo by allocating twice as many
   // points, setting g3d_stereo_base to the amount of memory
   // the points take up

void g3_start_stereo_frame(grs_canvas *rt);
   // mark all points as being unused, sets g3d_stereo to true
   // sets g3d_rt_canv to rt so you know where to render to

void g3_set_eyesep(fix sep);
   // sets g3d_eyesep_raw to sep

// System inialization, etc.

short g3_init(short max_points,int user_x_axis,int user_y_axis,int user_z_axis);
	//the three axis vars describe your coordintate system.  Use the constants
	//      X_AXIS,Y_AXIS,Z_AXIS, or negative of these, to describe what your 
	// coordinates mean. For each of width_,height_, and depth_axis, specify
	//      which of your axes goes in that dimension. Depth is into the screen,
	//      height is up, and width is to the right
	//returns number actually allocated


void g3_shutdown(void);                         //frees allocated points, and whatever else
int g3_count_free_points(void);

// Point definition and manipulation

g3s_phandle g3_alloc_point(void);
	//returns a free point

int g3_alloc_list(int n, g3s_phandle *p);
   // allocates n points into p, returns 0 for none, or n for ok

g3s_phandle g3_rotate_point(g3s_vector *v);
	//translate, rotate, and code point in 3-space. returns point handle

g3s_phandle g3_rotate_norm(g3s_vector *v);
	//rotate normal in 3-space. returns point handle.

//g3s_phandle g3_rotate_grad(g3s_vector *v);
// MLA - defined as same routine in POINT.C
#define g3_rotate_grad g3_rotate_norm
	//rotate gradient in 3-space. returns point handle.

g3s_phandle g3_rotate_light_norm(g3s_vector *v);
   //rotate light norm from obj space into viewer space

#if (defined(powerc) || defined(__powerc))	
int g3_project_point(g3s_phandle p);
#else
asm int g3_project_point(g3s_phandle p);
#endif

	//project already-rotated point. returns true if z>0

g3s_phandle g3_transform_point(g3s_vector *v);
	//translate, rotate, code, and project point (rotate_point + project_point);
	//returns point handle


void g3_rotate_delta_v(g3s_vector *dest,g3s_vector *src);
	//rotates a delta - takes vector input, fills in vector

void g3_rotate_delta_x(g3s_vector *dest,fix dx);
void g3_rotate_delta_y(g3s_vector *dest,fix dy);
void g3_rotate_delta_z(g3s_vector *dest,fix dz);
void g3_rotate_delta_xz(g3s_vector *dest,fix dx,fix dz);
void g3_rotate_delta_yz(g3s_vector *dest,fix dy,fix dz);
void g3_rotate_delta_xy(g3s_vector *dest,fix dx,fix dy);
void g3_rotate_delta_xyz(g3s_vector *dest,fix dx,fix dy,fix dz);
	//rotate a deltas - take just the spefified values

void g3_add_delta_v(g3s_phandle p,g3s_vector *delta);
void g3_add_delta_x(g3s_phandle p,fix dx);
void g3_add_delta_y(g3s_phandle p,fix dy);
void g3_add_delta_z(g3s_phandle p,fix dz);
void g3_add_delta_xy(g3s_phandle p,fix dx,fix dy);
void g3_add_delta_xz(g3s_phandle p,fix dx,fix dz);
void g3_add_delta_yz(g3s_phandle p,fix dy,fix dz);
void g3_add_delta_xyz(g3s_phandle p,fix dx,fix dy,fix dz);
	//adds a delta to a point

g3s_phandle g3_copy_add_delta_v(g3s_phandle src,g3s_vector *delta);
g3s_phandle g3_copy_add_delta_x(g3s_phandle src,fix dx);
g3s_phandle g3_copy_add_delta_y(g3s_phandle src,fix dy);
g3s_phandle g3_copy_add_delta_z(g3s_phandle src,fix dz);
g3s_phandle g3_copy_add_delta_xy(g3s_phandle src,fix dx,fix dy);
g3s_phandle g3_copy_add_delta_xz(g3s_phandle src,fix dx,fix dz);
g3s_phandle g3_copy_add_delta_yz(g3s_phandle src,fix dy,fix dz);
g3s_phandle g3_copy_add_delta_xyz(g3s_phandle src,fix dx,fix dy,fix dz);
	//adds a delta to a point, and stores in a new point

g3s_phandle g3_replace_add_delta_x(g3s_phandle src, g3s_phandle dst, fix dx);
g3s_phandle g3_replace_add_delta_y(g3s_phandle src, g3s_phandle dst, fix dy);
g3s_phandle g3_replace_add_delta_z(g3s_phandle src, g3s_phandle dst, fix dz);
   //adds a delta to src and stores to preallocated point dst

g3s_phandle g3_dup_point(g3s_phandle p);                //makes copy of a point

void g3_copy_point(g3s_phandle dest,g3s_phandle src);

//do a whole bunch of points. returns codes and & or
g3s_codes g3_rotate_list(short n,g3s_phandle *dest_list,g3s_vector *v);
g3s_codes g3_transform_list(short n,g3s_phandle *dest_list,g3s_vector *v);
g3s_codes g3_project_list(short n,g3s_phandle *point_list);


void g3_free_point(g3s_phandle p);      //adds to free list
void g3_free_list(int n_points,g3s_phandle *p); //adds to free list


// Frame setup commands

void g3_start_frame(void);          //mark all points as unused
void g3_set_view_matrix(g3s_vector *pos,g3s_matrix *m,fix zoom);
void g3_set_view_angles(g3s_vector *pos,g3s_angvec *angles,int rotation_order,fix zoom);        //takes ptr to angles

int g3_end_frame(void);             //returns number of points lost. thus, 0==no error

// Lighting commands
void g3_light_diff(g3s_phandle norm,g3s_phandle pos); // takes normal vector transformed, dots with the light vec, puts light val in norm
// MLA #pragma aux g3_light_diff "*" parm [eax] [edx] modify [eax edx ebx ecx esi edi]; 

void g3_light_spec(g3s_phandle norm,g3s_phandle pos); // takes norm and point position, lights point
// MLA #pragma aux g3_light_spec "*" parm [eax] [edx] modify [eax edx ebx ecx esi edi]; 

void g3_light_dands(g3s_phandle norm,g3s_phandle pos); // lights with both both
// MLA #pragma aux g3_light_dands "*" parm [eax] [edx] modify [eax edx ebx ecx esi edi]; 

// farms out a point based on flags
fix g3_light(g3s_phandle norm,g3s_phandle pos);
// MLA #pragma aux g3_light "*" parm [eax] [edx] modify [eax edx ebx ecx esi edi]; 

// generic list gronker, farms these points out
void g3_light_list(int n,g3s_phandle *norm,g3s_phandle *pos);

// sets a light vector in source space directly
// this light vector has to be in user space so we can dot it with
// other vector.  This is either a point source (LT_LOC_LIGHT == TRUE)
// or a vector.  
void g3_set_light_src(g3s_vector *l);
// MLA #pragma aux g3_set_light_src "*" parm [eax] modify [esi edi];

// note these need to be called *after* you've built a frame or object
// in which you transform points
// evaluates light vector, putting it into light_vec.  Do not call this
// after setting an object up.  Only call before object frames.  Only
// for vector evaluation.  Only call for non local lighting.
// When lighting "modes" get set up, this will be called automagically
// when necessary.
void g3_eval_vec_light(void) ;
// MLA #pragma aux g3_eval_vec_light "*" modify [eax edx ebx ecx esi edi]; 

// transforms light point into viewer coords with all scaling intact
// this prepares it to be subtracted from loc light points in eval_loc
// _light.  Call this after start_frame and before an object.
void g3_trans_loc_light(void);
// MLA #pragma aux g3_trans_loc_light "*" modify [eax edx ebx ecx esi edi]; 

// evaluates light point relative to another point, uses light point
// this is only for local lighting, inside an object.  If its not local
// lighting, you don't need to call this one.  This assumes you'll want
// to set NEAR_LIGHT to zero.
void g3_eval_loc_light(g3s_phandle pos);
// MLA #pragma aux g3_eval_loc_light "*" parm [eax] modify [eax edx ebx ecx esi edi]; 

// Evaluates and sets light vectors as necessary at the start of
// an object.  Does view vec if SPEC is set.  Transforms light
// vector or point depending how LOC_LIGHT is set.  Use this if
// both light and view will be modelled as far.  In fact, make
// sure both are set as far, or you will be sorry.
// Evaluates at the object center.  If necessary, evaluates the
// ldotv for light and view
void g3_eval_light_obj_cen(void);
// MLA #pragma aux g3_eval_light_obj_cen "*" modify [eax edx ebx ecx esi edi]; 

// evaluates the light vector straight ahead pointing in
// at you, then evaluates ldotv as well.  Use after light vec
// has been evaluated.  Only need for specular
void g3_eval_view_ahead(void);
// MLA #pragma aux g3_eval_view_ahead "*" modify [eax edx ebx ecx esi edi]; 


// takes the dot product of view and light, for specular light
void g3_eval_ldotv(void);
// MLA #pragma aux g3_eval_ldotv "*" modify [eax edx ebx ecx esi edi]; 

// evaluate the view vector relative to a point
void g3_eval_view(g3s_phandle pos);
// MLA #pragma aux g3_eval_view "*" parm [eax] modify [eax edx ebx ecx esi edi]; 

// Horizon

void g3_draw_horizon(int sky_color,int ground_color);  

// Misc commands

g3s_codes g3_check_codes(int n_verts,g3s_phandle *p);
	//returns codes_and & codes_or of points

bool g3_check_normal_facing(g3s_vector *v,g3s_vector *normal);
	//takes surface normal and unrotated point on poly. normal need not
	//be normalized

bool g3_check_poly_facing(g3s_phandle p0,g3s_phandle p1,g3s_phandle p2);
	//takes 3 rotated points on poly

void g3_get_FOV(fixang *x,fixang *y);
	//fills in field of view across width and height of screen

fix g3_get_zoom(char axis,fixang angle,int window_width,int window_height);
	//returns zoom factor to achieve the desired view angle. axis is 'y' or 'x'

void g3_get_view_pyramid(g3s_vector *corners);
	//fills in 4 vectors, which unit vectors from the eye that describe the 
	//view pyramid.  first vector is upper right, then clockwise

void g3_get_slew_step(fix step_size,g3s_vector *x_step,g3s_vector *y_step,g3s_vector *z_step);
	//fills in three vectors, each of length step_size, in the specified
	//direction in the viewer's frame of reference.  any (or all) of the 
	//vector pointers can be NULL to skip that axis.

// Instancing. These all return true if everything ok

bool g3_start_object(g3s_vector *p);    //position only (no orientation).

bool g3_start_object_matrix(g3s_vector *p,g3s_matrix *m);       //position and orientation. these can nest

bool g3_start_object_angles_v(g3s_vector *p,g3s_angvec *o,int rotation_order);  //position and orientation vector. these can nest
bool g3_start_object_angles_xyz(g3s_vector *p,fixang tx,fixang ty,fixang tz,int rotation_order);

bool g3_start_object_angles_x(g3s_vector *p,fixang tx);
bool g3_start_object_angles_y(g3s_vector *p,fixang ty);
bool g3_start_object_angles_z(g3s_vector *p,fixang tz);
bool g3_start_object_angles_xy(g3s_vector *p,fixang tx,fixang ty,int rotation_order);
bool g3_start_object_angles_xz(g3s_vector *p,fixang tx,fixang tz,int rotation_order);
bool g3_start_object_angles_yz(g3s_vector *p,fixang ty,fixang tz,int rotation_order);

// you can use this to scale things like make small boxes and the like.  The effect is to 
// shrink or expand the points in their SOURCE coordinate system.  Only call this after
// calling one of the start_object routines.  You can do it within a frame as well, the
// effect will be to make surrounding space smaller or bigger.  This will make you shoot
// towards or away from the origin, probably not the effect you're looking for
void g3_scale_object(fix s);

void g3_end_object(void);

// Drawing commands.
// all return 2d clip codes. See 2d header for values

// note that for the 2 gouraud line drawers, the 3d assumes that the calling function
// has not only set the rgb or i fields of the passed points, but has also set the
// p3_flags byte to indicate which is being used.  'tis a terrible hack indeed
// (Spaz, 6/29)

int g3_draw_point(g3s_phandle p);
int g3_draw_line(g3s_phandle p0,g3s_phandle p1);
int g3_draw_cline(g3s_phandle p0,g3s_phandle p1); // rgb-space gouraud line
int g3_draw_sline(g3s_phandle p0,g3s_phandle p1); // 2d-intensity gouraud line

int g3_draw_poly(long c,int n_verts,g3s_phandle *p);
// MLA #pragma aux g3_draw_poly "*" parm [eax] [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];
int g3_draw_tluc_poly(long c,int n_verts,g3s_phandle *p);
// MLA #pragma aux g3_draw_tluc_poly "*" parm [eax] [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];
int g3_draw_spoly(int n_verts,g3s_phandle *p);             //smooth poly
// MLA #pragma aux g3_draw_spoly "*" parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];
int g3_draw_tluc_spoly(int n_verts,g3s_phandle *p);   //smooth poly
// MLA #pragma aux g3_draw_tluc_spoly "*" parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];
int g3_draw_cpoly(int n_verts,g3s_phandle *p);        //RBG-space smooth poly
// MLA #pragma aux g3_draw_cpoly "*" parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];

int g3_check_and_draw_poly(long c,int n_verts,g3s_phandle *p);
// MLA #pragma aux g3_check_and_draw_poly "*" parm [eax] [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];
int g3_check_and_draw_tluc_poly(long c,int n_verts,g3s_phandle *p);
// MLA #pragma aux g3_check_and_draw_tluc_poly "*" parm [eax] [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];
int g3_check_and_draw_spoly(int n_verts,g3s_phandle *p);
// MLA #pragma aux g3_check_and_draw_spoly "*" parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];
int g3_check_and_draw_tluc_spoly(int n_verts,g3s_phandle *p);
// MLA #pragma aux g3_check_and_draw_tluc_spoly "*" parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];
int g3_check_and_draw_cpoly(int n_verts,g3s_phandle *p);
// MLA #pragma aux g3_check_and_draw_cpoly "*" parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];

//versions of the poly routines which take the args on the stack
int g3_draw_poly_st(int n_verts,...);
int g3_draw_cpoly_st(int n_verts,...);          //RBG-space smooth poly
int g3_draw_spoly_st(int n_verts,...);          //smooth poly
int g3_check_and_draw_poly_st(int n_verts,...);
int g3_check_and_draw_cpoly_st(int n_verts,...);
int g3_check_and_draw_spoly_st(int n_verts,...);

grs_vertex **g3_bitmap (grs_bitmap *bm, g3s_phandle p);
// MLA #pragma aux g3_bitmap "*" parm [esi] [edi] modify [eax ebx ecx edx];
grs_vertex **g3_anchor_bitmap(grs_bitmap *bm, g3s_phandle p, short u_anchor, short v_anchor);
// MLA #pragma aux g3_anchor_bitmap "*" parm [esi] [edi] [eax] [edx] modify [eax ebx ecx edx];
grs_vertex **g3_light_bitmap(grs_bitmap *bm, g3s_phandle p);
// MLA #pragma aux g3_light_bitmap "*" parm [esi] [edi] modify [eax ebx ecx edx];
grs_vertex **g3_light_anchor_bitmap(grs_bitmap *bm, g3s_phandle p, short u_anchor, short v_anchor);
// MLA #pragma aux g3_light_anchor_bitmap "*" parm [esi] [edi] [eax] [edx] modify [eax ebx ecx edx];
grs_vertex **g3_full_light_bitmap(grs_bitmap *bm, grs_vertex **p);
// MLA #pragma aux g3_full_light_bitmap "*" parm [esi] [edi] modify [eax ebx ecx edx];
grs_vertex **g3_full_light_anchor_bitmap(grs_bitmap *bm, grs_vertex **p, short u_anchor, short v_anchor);
// MLA #pragma aux g3_full_light_anchor_bitmap "*" parm [esi] [edi] [eax] [edx] modify [eax ebx ecx edx];

void g3_set_bitmap_scale (fix u_scale, fix v_scale);
// MLA #pragma aux g3_set_bitmap_scale "*" parm [ebx] [ecx] modify [eax ebx ecx edx];

int g3_draw_tmap(int n,g3s_phandle *vp,grs_bitmap *bm);
int g3_light_tmap(int n,g3s_phandle *vp,grs_bitmap *bm);
int g3_draw_floor_map(int n,g3s_phandle *vp,grs_bitmap *bm);
int g3_light_floor_map(int n,g3s_phandle *vp,grs_bitmap *bm);
int g3_draw_wall_map(int n,g3s_phandle *vp,grs_bitmap *bm);
int g3_light_wall_map(int n,g3s_phandle *vp,grs_bitmap *bm);
int g3_draw_lmap(int n,g3s_phandle *vp,grs_bitmap *bm);
int g3_light_lmap(int n,g3s_phandle *vp,grs_bitmap *bm);

#define g3_draw_tmap_quad(vp,bm) g3_draw_tmap_quad_tile(vp,bm,1,1)
#define g3_check_and_draw_tmap_quad(vp,bm) g3_check_and_draw_tmap_quad_tile(vp,bm,1,1)
	//these take four points, which match the four points of the texture map
	//corners.  The points are clockwise, and the first point is the upper left.

int g3_draw_tmap_quad_tile(g3s_phandle *vp,grs_bitmap *bm,int width_count,int height_count);
// MLA #pragma aux g3_draw_tmap_quad_tile "*" parm [esi] [edi] [eax] [ebx] value [eax] modify [eax ebx ecx edx esi edi];
int g3_check_and_draw_tmap_quad_tile(g3s_phandle *vp,grs_bitmap *bm,int width_count,int height_count);
// MLA #pragma aux g3_check_and_draw_tmap_quad_tile "*" parm [esi] [edi] [eax] [ebx] value [eax] modify [eax ebx ecx edx esi edi];
	//these are like draw_tmap_quad(), but tile the specified number of times
	//across and down.
int g3_light_tmap_quad_tile(g3s_phandle *vp,grs_bitmap *bm,int width_count,int height_count);
// MLA #pragma aux g3_light_tmap_quad_tile "*" parm [esi] [edi] [eax] [ebx] value [eax] modify [eax ebx ecx edx esi edi];
int g3_check_and_light_tmap_quad_tile(g3s_phandle *vp,grs_bitmap *bm,int width_count,int height_count);
// MLA #pragma aux g3_check_and_light_tmap_quad_tile "*" parm [esi] [edi] [eax] [ebx] value [eax] modify [eax ebx ecx edx esi edi];

int g3_draw_lmap_quad_tile(g3s_phandle *vp,grs_bitmap *bm,int width_count,int height_count);
// MLA #pragma aux g3_draw_lmap_quad_tile "*" parm [esi] [edi] [eax] [ebx] value [eax] modify [eax ebx ecx edx esi edi];
int g3_check_and_draw_lmap_quad_tile(g3s_phandle *vp,grs_bitmap *bm,int width_count,int height_count);
// MLA #pragma aux g3_check_and_draw_lmap_quad_tile "*" parm [esi] [edi] [eax] [ebx] value [eax] modify [eax ebx ecx edx esi edi];
int g3_light_lmap_quad_tile(g3s_phandle *vp,grs_bitmap *bm,int width_count,int height_count);
// MLA #pragma aux g3_light_lmap_quad_tile "*" parm [esi] [edi] [eax] [ebx] value [eax] modify [eax ebx ecx edx esi edi];
int g3_check_and_light_lmap_quad_tile(g3s_phandle *vp,grs_bitmap *bm,int width_count,int height_count);
// MLA #pragma aux g3_check_and_light_lmap_quad_tile "*" parm [esi] [edi] [eax] [ebx] value [eax] modify [eax ebx ecx edx esi edi];

int g3_draw_tmap_tile(g3s_phandle upperleft,g3s_vector *u_vec,g3s_vector *v_vec,int nverts,g3s_phandle *vp,grs_bitmap *bm);
// MLA #pragma aux g3_draw_tmap_tile "*" parm [eax] [ebx] [ecx] [edx] [esi] [edi] value [eax] modify [eax ebx ecx edx esi edi];
	//this will tile a texture map over an arbitrary polygon. upperleft is the
	//point in 3-space the matches the upper left corner of the texture map. 
	//u_vec and v_vec are the u and v basis vectors respectively.
	//upperleft need not be in the polygon.  If upperleft is 0, the warp matrix
	//from the last texture map that drew (i.e. was at least partly on screen)
	//will be used.
int g3_light_tmap_tile(g3s_phandle upperleft,g3s_vector *u_vec,g3s_vector *v_vec,int nverts,g3s_phandle *vp,grs_bitmap *bm);
// MLA #pragma aux g3_light_tmap_tile "*" parm [eax] [ebx] [ecx] [edx] [esi] [edi] value [eax] modify [eax ebx ecx edx esi edi];
int g3_draw_lmap_tile(g3s_phandle upperleft,g3s_vector *u_vec,g3s_vector *v_vec,int nverts,g3s_phandle *vp,grs_bitmap *bm);
// MLA #pragma aux g3_draw_lmap_tile "*" parm [eax] [ebx] [ecx] [edx] [esi] [edi] value [eax] modify [eax ebx ecx edx esi edi];
int g3_light_lmap_tile(g3s_phandle upperleft,g3s_vector *u_vec,g3s_vector *v_vec,int nverts,g3s_phandle *vp,grs_bitmap *bm);
// MLA #pragma aux g3_light_lmap_tile "*" parm [eax] [ebx] [ecx] [edx] [esi] [edi] value [eax] modify [eax ebx ecx edx esi edi];

void g3_interpret_object(ubyte *object_ptr,...);
extern void g3_set_tmaps_linear(void);
extern void g3_reset_tmaps(void);

//Pragmas for all these functions

/* MLA 
#pragma aux g3_init_stereo "*" parm [eax] [ebx] [ecx] [edx] value [ax];
#pragma aux g3_start_stereo_frame "*" parm [eax] modify [eax ebx ecx];
#pragma aux g3_set_eyesep "*" parm [eax];

#pragma aux g3_vec_dotprod "*" parm [esi] [edi] modify exact [eax ebx ecx edx esi]
#pragma aux g3_vec_mag "*" parm [esi] modify [eax ebx ecx edx esi edi]
#pragma aux g3_vec_normalize "*" parm [esi] modify [eax ebx ecx edx edi];
#pragma aux g3_vec_add "*" parm [edi] [esi] [ebx] modify exact [eax];
#pragma aux g3_vec_sub "*" parm [edi] [esi] [ebx] modify exact [eax];
#pragma aux g3_vec_scale "*" parm [edi] [esi] [ebx] modify exact [eax edx];
#pragma aug g3_vec_rotate "*" parm [ebx] [esi] [edi] modify [eax ecx edx];

#pragma aux g3_init "*" parm [eax] [ebx] [ecx] [edx] value [ax];
#pragma aux g3_shutdown "*";
#pragma aux g3_count_free_points "*" value [eax];

#pragma aux g3_alloc_point "*" value [eax] modify exact [eax esi ebx];
#pragma aux g3_alloc_list  "*" value [eax] parm [ecx] [esi] modify exact [eax ebx esi];

#pragma aux g3_rotate_point "*" parm [esi] value [edi] modify [eax ebx ecx edx esi edi];
#pragma aux g3_rotate_grad "*" parm [esi] value [edi] modify [eax ebx ecx edx esi edi];
#pragma aux g3_rotate_norm "*" parm [esi] value [edi] modify [eax ebx ecx edx esi edi];
#pragma aux g3_project_point "*" parm [edi] modify exact [eax ecx edx];
#pragma aux g3_transform_point "*" parm [esi] value [edi] modify [eax ebx ecx edx esi edi];
#pragma aux g3_rotate_light_norm "*" parm [esi] value [edi] modify [eax ebx ecx edx esi edi];


#pragma aux g3_rotate_list "*" parm [ecx] [edi] [esi] value [ax] modify [eax ebx ecx edx esi edi];
#pragma aux g3_project_list "*" parm [ecx] [esi] value [ax] modify [eax ebx ecx edx esi edi];
#pragma aux g3_transform_list "*" parm [ecx] [edi] [esi] value [ax] modify [eax ebx ecx edx esi edi];

#pragma aux g3_rotate_delta_v "*" parm [edi] [esi] modify [eax ebx ecx edx esi];

#pragma aux g3_add_delta_v "*" parm [edi] [esi] modify [eax ebx edx esi];
#pragma aux g3_add_delta_x "*" parm [edi] [eax] modify [eax ebx edx];
#pragma aux g3_add_delta_y "*" parm [edi] [eax] modify [eax ebx edx];
#pragma aux g3_add_delta_z "*" parm [edi] [eax] modify [eax ebx edx];
#pragma aux g3_add_delta_xy "*" parm [edi] [eax] [ebx] modify exact [eax ebx ecx edx esi];
#pragma aux g3_add_delta_xz "*" parm [edi] [eax] [ebx] modify exact [eax ebx ecx edx esi];
#pragma aux g3_add_delta_yz "*" parm [edi] [eax] [ebx] modify exact [eax ebx ecx edx esi];
#pragma aux g3_add_delta_xyz "*" parm [edi] [eax] [ebx] [ecx] modify exact [eax ebx ecx edx esi];


#pragma aux g3_copy_add_delta_v "*" parm [esi] [ebx] value [edi] modify [eax ebx];
#pragma aux g3_copy_add_delta_x "*" parm [esi] [eax] value [edi] modify exact [eax ebx edx esi];
#pragma aux g3_copy_add_delta_y "*" parm [esi] [eax] value [edi] modify exact [eax ebx edx esi];
#pragma aux g3_copy_add_delta_z "*" parm [esi] [eax] value [edi] modify exact [eax ebx edx esi];
#pragma aux g3_copy_add_delta_xy "*" parm [esi] [eax] [ebx] value [edi] modify exact [eax ebx ecx edx];
#pragma aux g3_copy_add_delta_xz "*" parm [esi] [eax] [ebx] value [edi] modify exact [eax ebx ecx edx];
#pragma aux g3_copy_add_delta_yz "*" parm [esi] [eax] [ebx] value [edi] modify exact [eax ebx ecx edx];
#pragma aux g3_copy_add_delta_xyz "*" parm [esi] [eax] [ebx] [ecx] value [edi] modify exact [eax ebx ecx edx esi];

#pragma aux g3_replace_add_delta_x "*" parm [esi] [edi] [eax] value [edi] modify exact [eax ebx edx esi];
#pragma aux g3_replace_add_delta_y "*" parm [esi] [edi] [eax] value [edi] modify exact [eax ebx edx esi];
#pragma aux g3_replace_add_delta_z "*" parm [esi] [edi] [eax] value [edi] modify exact [eax ebx edx esi];

#pragma aux g3_rotate_delta_x  "*" parm [edi] [eax] modify exact [eax ebx edx];
#pragma aux g3_rotate_delta_y  "*" parm [edi] [eax] modify exact [eax ebx edx];
#pragma aux g3_rotate_delta_z  "*" parm [edi] [eax] modify exact [eax ebx edx];
#pragma aux g3_rotate_delta_xz "*" parm [edi] [eax] [ebx] modify exact [eax ebx ecx edx esi];
#pragma aux g3_rotate_delta_xy "*" parm [edi] [eax] [ebx] modify exact [eax ebx ecx edx esi];
#pragma aux g3_rotate_delta_yz "*" parm [edi] [eax] [ebx] modify exact [eax ebx ecx edx esi];
#pragma aux g3_rotate_delta_xyz "*" parm [edi] [eax] [ebx] [ecx] modify exact [eax ebx ecx edx esi];

#pragma aux g3_dup_point "*" parm [esi] value [edi] modify [ebx ecx esi];
#pragma aux g3_copy_point "*" parm [edi] [esi] modify exact [ecx esi];

#pragma aux g3_free_point "*" parm [eax] modify exact [ebx esi];
#pragma aux g3_free_list "*" parm [ecx] [esi] modify exact [eax ebx ecx esi];

#pragma aux g3_set_view_matrix "*" parm [esi] [ebx] [eax] modify [eax ebx ecx edx esi edi];
#pragma aux g3_set_view_angles "*" parm [esi] [ebx] [ecx] [eax] modify [eax ebx ecx edx esi edi];

#pragma aux g3_start_frame "*" modify [eax ebx ecx];
#pragma aux g3_end_frame "*" value [eax];

#pragma aux g3_draw_horizon "*" parm [eax] [edx] modify [eax ebx ecx edx esi edi];

#pragma aux g3_check_codes "*" parm [ecx] [esi] value [bx] modify exact [ebx ecx edx esi];

#pragma aux g3_check_normal_facing "*" parm [esi] [edi] value [al] modify exact [eax ebx ecx edx];
#pragma aux g3_check_poly_facing "*" parm [eax] [edx] [ebx] value [al] modify [eax ebx ecx edx esi edi];

#pragma aux g3_start_object "*" parm [esi] value [al] modify exact [eax];
#pragma aux g3_start_object_matrix "*" parm [esi] [edi] value [al] modify [eax ebx ecx edx esi edi];
#pragma aux g3_start_object_angles_v "*" parm [esi] [edi] [ecx] value [al] modify [eax ebx ecx edx esi edi];
#pragma aux g3_start_object_angles_xyz "*" parm [esi] [eax] [ebx] [edx] [ecx] value [al] modify [eax ebx ecx edx esi edi];
#pragma aux g3_start_object_angles_x "*" parm [esi] [ebx] value [al] modify [eax ebx ecx edx esi edi];
#pragma aux g3_start_object_angles_y "*" parm [esi] [ebx] value [al] modify [eax ebx ecx edx esi edi];
#pragma aux g3_start_object_angles_z "*" parm [esi] [ebx] value [al] modify [eax ebx ecx edx esi edi];
#pragma aux g3_start_object_angles_xy "*" parm [esi] [ebx] [edx] [ecx] value [al] modify [eax ebx ecx edx esi edi];
#pragma aux g3_start_object_angles_xz "*" parm [esi] [ebx] [edx] [ecx] value [al] modify [eax ebx ecx edx esi edi];
#pragma aux g3_start_object_angles_yz "*" parm [esi] [ebx] [edx] [ecx] value [al] modify [eax ebx ecx edx esi edi];
#pragma aux g3_end_object "*" modify [ecx esi edi];
#pragma aux g3_scale_object "*" parm [eax] modify [ecx eax edx];

#pragma aux g3_draw_line "*" parm [esi] [edi] value [eax] modify [eax ebx ecx edx esi edi];
#pragma aux g3_draw_cline "*" parm [esi] [edi] value [eax] modify [eax ebx ecx edx esi edi]; 
#pragma aux g3_draw_sline "*" parm [esi] [edi] value [eax] modify [eax ebx ecx edx esi edi];
#pragma aux g3_draw_point "*" parm [esi] value [eax] modify [eax ecx edx esi];

#pragma aux g3_get_FOV parm [esi] [edi] modify exact [eax ebx ecx edx];
#pragma aux g3_get_zoom "*" parm [eax] [ebx] [ecx] [edx] value [eax] modify exact [eax ebx ecx edx esi edi];

#pragma aux g3_get_view_pyramid "*" parm [edi] modify exact [eax ebx ecx edx esi edi];

#pragma aux g3_get_slew_step "*" parm [eax] [ebx] [ecx] [edi] modify exact [eax edx esi];

#pragma aux g3_compute_normal "*" parm [edi] [eax] [edx] [ebx] modify exact [eax ebx ecx edx esi edi];

#pragma aux g3_transpose parm [esi] modify exact [eax];
#pragma aux g3_copy_transpose parm [edi] [esi] modify exact [eax];

#pragma aux g3_matrix_x_matrix parm [ebx] [esi] [edi] modify exact [eax ebx ecx edx];

#pragma aux g3_interpret_object "*" parm caller modify [eax ebx ecx edx esi edi];
*/

//inline code to handle stack args for polygon routines

//Note that these are a lot uglier than they need to be, because C is so
//annoying.  For starters, I shouldn't even have to specifiy the number of
//parms, since C obviously knows this number.  Secondly,  if I have varargs,
//C forces me to put all the args on the stack, when I really want the
//count in register and the rest on the stack.  Lastly, since these are 
//inline functions, C won't do the stack fixup, and since I don't have
//access, once again, to the parameter count, I have to use the variable
//to fixup the stack.  This is really ugly since a wrong count supplied to 
//the function will mess up the stack.  Too bad C doesn't have a constant
//defined to be the parameter count for the current function.

// awwwww, poor matt

// MLA- took these out because we can't do inline asm in PowerPC (or 68K for that matter)
// and all routines need to be C callable
/*
#pragma aux g3_draw_poly_st = \
	"mov ecx,[esp]"                 \
	"lea esi,4[esp]"                        \
	"call	g3_draw_poly"     \
	"pop ecx"                               \
	"lea esp,[esp+ecx*4]"   \
	parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];

#pragma aux g3_draw_spoly_st = \
	"mov ecx,[esp]"                 \
	"lea esi,4[esp]"                        \
	"call	g3_draw_spoly"    \
	"pop ecx"                               \
	"lea esp,[esp+ecx*4]"   \
	parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];

#pragma aux g3_draw_cpoly_st = \
	"mov ecx,[esp]"                 \
	"lea esi,4[esp]"                        \
	"call	g3_draw_cpoly"    \
	"pop ecx"                               \
	"lea esp,[esp+ecx*4]"   \
	parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];

#pragma aux g3_check_and_draw_poly_st = \
	"mov ecx,[esp]"                 \
	"lea esi,4[esp]"                        \
	"call	g3_check_and_draw_poly"   \
	"pop ecx"                               \
	"lea esp,[esp+ecx*4]"   \
	parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];

#pragma aux g3_check_and_draw_cpoly_st = \
	"mov ecx,[esp]"                 \
	"lea esi,4[esp]"                        \
	"call	g3_check_and_draw_cpoly"  \
	"pop ecx"                               \
	"lea esp,[esp+ecx*4]"   \
	parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];

#pragma aux g3_check_and_draw_spoly_st = \
	"mov ecx,[esp]"                 \
	"lea esi,4[esp]"                        \
	"call	g3_check_and_draw_spoly"  \
	"pop ecx"                               \
	"lea esp,[esp+ecx*4]"   \
	parm [ecx] [esi] value [eax] modify [eax ebx ecx edx esi edi];
*/

#endif /* __3D_H */
