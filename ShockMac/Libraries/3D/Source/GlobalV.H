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
// $Source: r:/prj/lib/src/3d/RCS/globv.h $
// $Revision: 1.5 $
// $Author: jaemz $
// $Date: 1994/11/06 19:10:28 $
//
// Global vars for the 3d system
//
// $Log: globv.asm $
// Revision 1.5  1994/11/06  19:10:28  jaemz
// Allow stereo to be externed even in non stereo version to test
// 
// Revision 1.4  1994/09/28  19:01:00  jaemz
// Fixed stereo bug 
// 
// Revision 1.3  1994/09/20  13:29:08  jaemz
// Added globals for lighting
// 
// Revision 1.2  1994/08/18  03:46:56  jaemz
// Changed stereo glob names to have underscore for c
// 
// Revision 1.1  1994/08/04  17:47:21  jaemz
// Initial revision
// 
// 


#include "lg.h"
#include "3d.h"

// point allocation vars

extern g3s_point	*point_list;   //   dd      0       ;ptr to point buffer
extern short			n_points;      //   dw      0       ;num points allocated
extern g3s_point 	*first_free;   //   dd      0       ;ptr to first free pnt

extern g3s_matrix	unscaled_matrix; // g3s_matrix <>   ;unscaled & unadjusted

// note: view_matrix and view_position must remain in this order!
extern g3s_matrix 	view_matrix;     // g3s_matrix <>
extern g3s_vector	_view_position;  // g3s_vector <>
extern fix					_view_zoom;      // fix     ?
extern fix					view_heading;    // fix     ?
extern fix					view_pitch;      // fix     ?
extern fix					view_bank;       // fix     ?

//are to save inverse object to world matrix and position
//to go from world to object, take Ax + a (like in real 3d)
extern g3s_matrix	_wtoo_matrix;  		//  g3s_matrix <>
extern g3s_vector	_wtoo_position;		//  g3s_vector <>


extern fix 				pixel_ratio;    		//  fix     ?       ;copy from 2d drv_cap

extern long				window_width;  		//  dd      ?
extern long				window_height; 		//  dd      ?

extern long				ww2;  							//  dd      ?       ;one-half widht,height 
extern long				wh2; 							//  dd      ?       ;..for texture mapper

extern long				_scrw; 						//  dd      ?       ;need to do double-word mul
extern long				_scrh;  						//	dd      ?

extern fix					_biasx;  					//	fix     ?
extern fix					_biasy; 						// fix     ?

extern g3s_vector	_matrix_scale;  		//  <>   ;how the columns are scaled
extern g3s_vector	horizon_vector; 		//  <>   ;info for drawing the horizon

//this tables tells you many bits to shift to get zero
extern uchar shift_table[];
extern long			up_axis;			// dd      ?

//which axis is our x,y,z?
extern long			axis_x;			//  dd      ?
extern long			axis_z;			//  dd      ?
extern long			axis_y;			//  dd      ?

//offset into matrix of axis which is x,y,z
extern long			axis_x_ofs;			//      dd      ?
extern long			axis_z_ofs;			//      dd      ?
extern long			axis_y_ofs;			//      dd      ?

extern char			axis_swap_flag; 	// 			db      ?
extern char			axis_neg_flag;   //			db      ?

// Lighting globals
extern char 			_g3d_light_type;		// db      0       ; The lighting type, see above

extern fix				_g3d_amb_light;  				// fix    0       ; amount of ambient light 
extern fix				_g3d_diff_light; 	// fix    10000h  ; intensity of light source
extern fix				_g3d_spec_light; 				// fix    0       ; amount of spec light
extern fix				_g3d_flash;       			// fix    0       ; specular flash point below which none is applied
// note that specular light is used to provide a "flash" mostly
// so you can artificially inflate it a bit, or we could try
// funny functions to make it "flash" only within a certain
// range.  

extern g3s_vector	_g3d_light_src;			//  g3s_vector      <>      ; light source, either local or vector
extern g3s_vector	_g3d_light_trans;		//  g3s_vector   		<>      ; point source in view coords
extern g3s_vector	_g3d_light_vec;			//  g3s_vector      <>      ; current light vector, computed from src and flag

extern g3s_vector	_g3d_view_vec;				//  g3s_vector      <>      ; current viewing vector, may have to be computed periodically

extern fix					_g3d_ldotv;  				//  fix     ?       ; light vector dotted with view vector (for specular only)
extern fix					_g3d_sdotl;    			//  fix     ?       ; surface vector dotted with light vector (for diffuse and spec)
extern fix					_g3d_sdotv;    			//  fix     ?       ; surface vector dotted with view vector (ostensibly jnorm)

extern long				_g3d_light_tab;		//  dd      0       ; lighting table with 32 or 24 entries.  Should go from black to white, 


// stereo globals, read em and weep
extern fix				_g3d_eyesep_raw;       //  fix     0     ;raw 3d sep between eyes
extern fix				_g3d_eyesep;           //  fix     0     ;scaled eye sep between eyes
extern long				_g3d_stereo_base;      //   dd     0     ;stereo point offset, default zero, means non
extern long				_g3d_stereo_list;      //   dd     0     ;start of stereo point list, makes it easy to detect
extern char				_g3d_stereo;           //   db     0     ;stereo this frame
extern long				_g3d_rt_canv;          //   dd     0     ;pointer to right eye canvas
extern long				_g3d_rt_canv_bits;     //   dd     0     ;pointer to bits of rt canvas
extern long				_g3d_lt_canv_bits;     //   dd     0     ;pointer to bits of lt canvas
extern long				_g3d_stereo_tmp[14];     //   dd     14 dup (?) ;temporary point list


//palette base for gouraud-shaded polys

extern sfix				gouraud_base;  					 //  sfix    ?

//for statistics
/*	ifdef   dbg_on

n_polys dw      ?
n_polys_drawn   dw      ?
n_polys_triv_acc        dw      ?
n_polys_triv_rej        dw      ?
n_polys_clip_2d dw      ?
n_polys_clip_3d dw      ?

	endif*/
