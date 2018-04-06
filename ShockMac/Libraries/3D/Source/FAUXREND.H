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
#ifndef __FAUXREND_H
#define __FAUXREND_H

#include "3d.h"

#define POPUPS_ALLOWED

// structures
typedef struct {
   grs_canvas *draw_canvas;
   bool       double_buffer;
   int        xtop, ytop;
   fix        viewer_zoom;
} fauxrend_context;

// fauxrend.c
void fauxrend(fauxrend_context *fr);
void fauxrend_set_context(fauxrend_context *frc);
void fauxrend_free_context(fauxrend_context *frc);
fauxrend_context *fauxrend_place_3d(fauxrend_context *fr, bool db_buf, char axis, int fov, int xc, int yc, int wid, int hgt);
void fauxrend_zoom(fauxrend_context *fr, fix mod_fac);
void fauxrend_setup_3d(void);
void fauxrend_close_3d(void);
void fauxrend_clear_3d(fauxrend_context *fr, int clear_color);

void frame_check(void);

//void eyepos_set(long x, long y, long z, long h, long p, long b);
void eyepos_tele_to(int x, int y);
void eyepos_setone(int which, int val);
void eyepos_moveone(int which, int how);
void eyepos_init(void);

#define MAP_SC 256
#define MAP_SH 8
#define MAP_MK 0xff
#define MAP_MS 8

#define EYE_X 0
#define EYE_Y 1
#define EYE_Z 2
#define EYE_H 3
#define EYE_P 4
#define EYE_B 5

#ifdef __FAUXREND_SRC
long eye[6] = {0,0,0,0,0,0};
long eye_scale[6]={1,1,1,128,128,128};
#else
extern long eye[6];
extern long eye_scale[6];
extern char eye_slew;
#endif

// axis setup for the zany extra math-o-tron 3d
//#define AXIS_ORDER   X_AXIS,Z_AXIS,Y_AXIS
//#define ANGLE_ORDER  ORDER_ZXY

//#define AXIS_ORDER   X_AXIS,-Y_AXIS,Z_AXIS
//#define AXIS_ORDER   X_AXIS,Y_AXIS,Z_AXIS

#define AXIS_ORDER   AXIS_RIGHT,AXIS_DOWN,AXIS_IN

#define ANGLE_ORDER  ORDER_YXZ
#define pitch        tx
#define bank         tz
#define head         ty
#define xaxis        x
#define yaxis        y
#define zaxis        z

// conversions
#define build_fix_angle(ang) ((65536*(ang))/360)

// defaults
#define DEFAULT_FOV     80
#define DEFAULT_AXIS    'X'
#define DEFAULT_PT_CNT  80     // sure, why not


// masks for quadrant/octant free facing check
#define FMK_NW         (1<<0)
#define FMK_EW         (1<<1)
#define FMK_SW         (1<<2)
#define FMK_WW         (1<<3)
#define FMK_D1         (1<<4)
#define FMK_D3         (1<<5)
#define FMK_D5         (1<<6)
#define FMK_D7         (1<<7)

#define MK_O_N         (0)
#define MK_O_E         (1)
#define MK_O_S         (2)
#define MK_O_W         (3)

#define HG_NW          (1<<1)
#define HG_NE          (1<<2)
#define HG_SE          (1<<3)
#define HG_SW          (1<<0)

/* note: 9 is unused
 *
 *     1 B 3
 *   0       2
 *   8   D   A    not detected yet
 *   4       6
 *     5 C 7
 */

#define QUAD_N_BASE 0
#define QUAD_S_BASE 4
#define QUAD_A_BASE 8
#define QUAD_X_OFF  2
#define QUAD_D_OFF  1
#define QUAD_CENTER 0xB

#define FACE_FLOOR  0
#define FACE_CIEL   1
#define FACE_WALLS  2

#define FLOOR_COL   32
#define CIEL_COL    44
#define WALL_COL    56

#endif
