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
#ifndef __RENDER_H
#define __RENDER_H

/*
 * $Source: n:/project/cit/src/inc/RCS/render.h $
 * $Revision: 1.16 $
 * $Author: xemu $
 * $Date: 1994/03/20 21:16:13 $
 *
 *
 */

// Includes

// C Library Includes

// System Library Includes

// Master Game Includes

// Game Library Includes

// Game Object Includes


// Defines
#define TM_SIZE_CNT 3

// Prototypes
errtype render_run(void);

// Globals
extern LGRect* rendrect;

#ifdef __RENDER_SRC
bool fr_texture           = TRUE;
bool fr_txt_walls         = TRUE;
bool fr_txt_floors        = TRUE;
bool fr_txt_ceilings      = TRUE;
bool fr_lights_out        = FALSE;
bool fr_lighting          = TRUE;
bool fr_play_lighting     = FALSE;
bool fr_normal_lights     = TRUE;
int  fr_detail_value      = 100;
int  fr_drop[TM_SIZE_CNT] ={1,4,10};
int  fr_view_radius       = 10;
int  fr_clear_color       = 0;
bool fr_show_tilecursor   = FALSE;
bool fr_cont_tilecursor   = FALSE;
bool fr_show_all          = TRUE;
int  fr_qscale_obj        = 2;
int  fr_qscale_crit       = 2;
bool fr_highlights        = FALSE;
int  fr_normal_shf        = 2;
int  fr_lite_rad1         = 0,
     fr_lite_base1        = 10,
     fr_lite_rad2         = 7,
     fr_lite_base2        = 0;
fix  fr_lite_slope        = (-fix_make(2,0)+fix_make(0,0x5000)),
     fr_lite_yint         = fix_make(12,0x2000);
int  fr_detail_master     = 3;         /* 0-3 master detail */
int  fr_pseudo_spheres    = 0;
#else
extern bool fr_texture;
extern bool fr_txt_walls;
extern bool fr_txt_floors;
extern bool fr_txt_ceilings;
extern bool fr_lighting, fr_play_lighting, fr_lights_out, fr_normal_lights;
extern int  fr_detail_value;
extern int  fr_drop[TM_SIZE_CNT];
extern bool fr_view_radius;
extern bool fr_clear_color;
extern bool fr_show_tilecursor;
extern bool fr_cont_tilecursor;
extern bool fr_show_all;
extern int  fr_qscale_crit, fr_qscale_obj;
extern bool fr_highlights;
extern int  fr_lite_rad1, fr_lite_base1, fr_lite_rad2, fr_lite_base2;
extern int  fr_normal_shf;
extern fix  fr_lite_slope, fr_lite_yint;
extern int  fr_detail_master;
extern int  fr_pseudo_spheres;
#endif

#define MAX_CAMERAS_VISIBLE   2
#define NUM_HACK_CAMERAS      8
// hack cameras are "custom textures" 7c through 7f 
// (so, factoring in type, the low byte is fc to ff
//#define FIRST_CAMERA_TMAP     (short)0x80 - NUM_HACK_CAMERAS 
#define FIRST_CAMERA_TMAP     0x78

errtype init_hack_cameras(void);
errtype shutdown_hack_cameras(void);
errtype do_screen_static(void);
errtype render_hack_cameras(void);
errtype hack_camera_takeover(int hack_cam);
errtype hack_camera_relinquish(void);


#endif // __RENDER_H
