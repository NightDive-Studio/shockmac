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
 * $Source: r:/prj/cit/src/inc/RCS/ss_flet.h $
 * $Revision: 1.2 $
 * $Author: dc $
 * $Date: 1994/08/24 05:43:55 $
 */

typedef struct {
   fix norm[3];   // unit normal 
   fix att;       // attentuation
   fix comp;      // compression
   int flags;     // flag field, BCD, Primary, Which type, so on
} ss_facelet_return;

// globals
#define SS_MAX_FACELETS 16
extern ss_facelet_return ss_edms_facelets[SS_MAX_FACELETS];
extern uchar             ss_edms_facelet_cnt;
extern int               ss_edms_bcd_flags;
extern int               ss_edms_bcd_param;
extern uchar             ss_edms_stupid_flag;

// bitfield values
// 0bEtttttttRRRRCCCC00FF00MM00TT0PPP

#define SS_BCD_PRIM_SHF   0
#define SS_BCD_PRIM_MASK  0x7
#define SS_BCD_AXIS_MASK  0x6
#define SS_BCD_PRIM_MULTI 0x0
#define SS_BCD_PRIM_XAXIS 0x2
#define SS_BCD_PRIM_YAXIS 0x4
#define SS_BCD_PRIM_ZAXIS 0x6
#define SS_BCD_PRIM_NEG   0x1
#define SS_BCD_PRIM_NEG_X 0x3
#define SS_BCD_PRIM_NEG_Y 0x5
#define SS_BCD_PRIM_NEG_Z 0x7

#define SS_BCD_TYPE_SHF   4
#define SS_BCD_TYPE_MASK  0x30
#define SS_BCD_TYPE_ERR   0x00
#define SS_BCD_TYPE_FLOOR 0x10
#define SS_BCD_TYPE_WALL  0x20
#define SS_BCD_TYPE_CEIL  0x30

#define SS_BCD_MISC_SHF   8
#define SS_BCD_MISC_MASK  0xF00
#define SS_BCD_MISC_CLIMB 0x100
#define SS_BCD_MISC_STAIR 0x200

#define SS_BCD_FRIC_SHF   12
#define SS_BCD_FRIC_MASK  0x3000
#define SS_BCD_FRIC_ZERO  0x0000
#define SS_BCD_FRIC_LOW   0x1000
#define SS_BCD_FRIC_NORM  0x2000
#define SS_BCD_FRIC_HIGH  0x3000

#define SS_BCD_CURR_SHF   16
#define SS_BCD_CURR_ON    0xC0000   // mask with this to see if any current
#define SS_BCD_CURR_DIR   0x30000   // this gives the direction
#define SS_BCD_CURR_N     0x00000
#define SS_BCD_CURR_E     0x10000
#define SS_BCD_CURR_S     0x20000
#define SS_BCD_CURR_W     0x30000
#define SS_BCD_CURR_SPD   0xC0000   // this gives "speed"
#define SS_BCD_CURR_NULL  0x00000   // null is no current
#define SS_BCD_CURR_LOW   0x40000   // low-high as expected
#define SS_BCD_CURR_MID   0x80000
#define SS_BCD_CURR_HIGH  0xC0000

#define SS_BCD_REPUL_SHF  20
#define SS_BCD_REPUL_ON   0x700000
#define SS_BCD_REPUL_TYPE 0x700000
#define SS_BCD_REPUL_NULL 0x000000
#define SS_BCD_REPUL_UP   0x100000
#define SS_BCD_REPUL_DOWN 0x200000
#define SS_BCD_REPUL_N    0x300000
#define SS_BCD_REPUL_S    0x400000
#define SS_BCD_REPUL_E    0x500000
#define SS_BCD_REPUL_W    0x600000
#define SS_BCD_REPUL_SPD  0x800000
#define SS_BCD_REPUL_NORM 0x000000
#define SS_BCD_REPUL_FAST 0x800000
                                 
#define SS_BCD_EOF        (1<<31)
#define TF_FLG_HPARAM     (1<<31)

#define TF_FLG_BOX_MASK   0x30000000
#define TF_FLG_BOX_NONE   0x00000000
#define TF_FLG_BOX_LR     0x10000000
#define TF_FLG_BOX_TB     0x20000000
#define TF_FLG_BOX_FULL   0x30000000

#define TF_FLG_ICHK_MASK  0x0C000000
#define TF_FLG_ICHK_NONE  0x00000000
#define TF_FLG_ICHK_INT   0x04000000
#define TF_FLG_ICHK_EDGE  0x08000000
#define TF_FLG_ICHK_OUT   0x0C000000

#define TF_FLG_3PNT_MASK  0x02000000

#define TF_FLG_NHINT_MASK 0x01000000

#define TF_FLG_ALL_TERR   0x7F000000

// rah rah stupidity
#define TFD_FULL  0
#define TFD_RCAST 1
#define TFD_BCD   2


