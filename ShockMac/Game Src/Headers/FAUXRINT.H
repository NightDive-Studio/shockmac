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
#ifndef __FAUXRINT_H
#define __FAUXRINT_H

#define CFG_REND_TEST "rend_test"

#define PLAYER_HEIGHT 174

#define FR_CUR_OBJ_BASE 65

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
#define xaxis        gX
#define yaxis        gY
#define zaxis        gZ

// conversions
#define build_fix_angle(ang) ((65536*(ang))/360)

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

#define RNG_MUL        4
#define RNG_1          (0*RNG_MUL)
#define RNG_2          (1*RNG_MUL)
#define RNG_3          (2*RNG_MUL)

#define BOT_P          0
#define TOP_P          2
#define BOT_S          0
#define TOP_S          1

#define LEFT_E         0
#define RIGHT_E        1


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
#define FACE_CEIL   1
#define FACE_WALLS  2

#define REND_BT_N   (1<<0)
#define REND_BT_E   (1<<1)
#define REND_BT_S   (1<<2)
#define REND_BT_W   (1<<3)

#define REND_BT_FLR (1<<4)
#define REND_BT_CIE (1<<5)
#define REND_BT_INT (1<<6)

#endif
