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
 * FrTables.h
 *
 * $Source: n:/project/cit/src/inc/RCS/frtables.h $
 * $Revision: 1.2 $
 * $Author: dc $
 * $Date: 1994/01/12 22:04:14 $
 *
 * Citadel Renderer
 * externs of the tables for the tile definitions/quadrant codes
 *
 * $Log: frtables.h $
 * Revision 1.2  1994/01/12  22:04:14  dc
 * facelet code hacking
 * 
 * Revision 1.1  1994/01/02  17:16:49  dc
 * Initial revision
 * 
 */

// data structures

#define FRTILETYPES 51

//======== Points

// layout of points
//   
//     4 D     E     F 8
//     3   g       h   7
//       b           d
//
//     2               6
//
//       a           c
//     1   e       f   5
//     0 9     A     B C
//
// a-h are really 16-23 for octagonal setups
// each point has 2bits of zmod and then the code as above
// a table is used to take the pretransformed base points into the above
// in uv table note _uv1 is full, a is 2000, b is FROCT, c is 8000, d is -FROCT, e is -2000

// pack to uchar, uchar, int
typedef struct {
   short base, modcnt;
   fix   arg;
} pt_mods;

//-- constants
#define FRPTSUNIQUE 24
#define FRPTSOFFS   8
#define FRPTSZSHF   6
#define FRPTSZMOD   (2<<FRPTSZSHF)
#define FRPTSZPICK  (1<<FRPTSZSHF)
#define FRPTSZMASK  (3<<FRPTSZSHF)
#define FRPTSPTOFF  ((1<<FRPTSZSHF)-1)
// zcoor 
#define FRPTSZBASE  (0<<FRPTSZSHF)
#define FRPTSZCEIL  (1<<FRPTSZSHF)
#define FRPTSZBASEU (2<<FRPTSZSHF)
#define FRPTSZCEILD (3<<FRPTSZSHF)

// once shifted down, use these
#define FRPTSZPICK_DN (1)
#define FRPTSZCEIL_DN (1)
#define FRPTSZFLR_DN  (0)

#define FRMODNONE   0
#define FRMODYAXIS  1
#define FRMODXAXIS  2

// 0x2000*8/(2+sqrt(2))   
//  =side of a regular octagon whose bounding box is a square of size fix1.0
#define FROCTNUM    19195      
#define FROCTFIX    fix_make(0,19195)

//-- externs
#ifndef __FRTABLES_SRC
extern fix      pt_offs[FRPTSOFFS];
extern pt_mods  pt_deref[FRPTSUNIQUE];
extern ushort   pt_uv[FRPTSUNIQUE][4][2];
extern uchar    pt_from_faceoff[4][FRPTSOFFS];
#endif

//======== WallstoPts

// this corresponds to the list of point codes that make up an internal 
// wall of a given wall code, note that the external walls are tacked on
// at the end for the compiler or dynamic parsing

// the wall codes are, in order 
//  0-3  : main diagonals, SW to NE, NW norm then SE norm, then opposite NE norm SW norm
//  4-11 : quarter diagonals, slope 1/2 low hi, then -1/2 low hi
// 12-19 : quarter diagonals, slope 2 left right, then -2 left right
// 20-23 : halve tiles, EW N norm S norm, then NS W norm E norm
// 24-27 : one foots, S wall, N wall, E wall, W wall
// 28-31 : triangle, NS left right, EW north south facing
// 32-37 : octagonal, NS W three then E three, top to bottom
// 38-43 : octagonal, EW N three then S three, top to bottom
// 44-51 : parameterized main diagonals, 2 of each order, bot then top
// bonus extra
// 52-55 : N,E,S,W

typedef
 struct {
//   union {
	   uchar ul, ur, lr, ll;
//      uchar ptlst[4];
//   };
} WallsToPts;

#define FRWALLPTSCNT 56
#define FROUTERWALLS (FRWALLPTSCNT-4)

#ifndef __FRTABLES_SRC
extern WallsToPts wall_pts[FRWALLPTSCNT];
#endif

// one has to wonder, doesnt one?
#define ED  (32+4)
#define JOE (40-4)

//======== TilestoWalls

// for each tile type in tilename, we need a list of what walls it contains
// this is done as a bitfield of N|E|S|W|Internal, where internal is a 4bit count
// the second byte is the base into the wall_pts array for this tile, where from
//   that value up to that value + internal count are the internal walls

typedef struct {
   uchar wallbits;
   uchar wallbase;
} TilesToWalls;

#define FRWALLNORTH (1<<7)
#define FRWALLEAST  (1<<6)
#define FRWALLSOUTH (1<<5)
#define FRWALLWEST  (1<<4)
#define FRWALLINT   (0xf)

#define FRTILEWALLCNT FRTILETYPES

#ifndef __FRTABLES_SRC
extern TilesToWalls tile_walls[FRTILEWALLCNT];
#endif


//======== TilesToFloors
 
// a tile has one or two floors, depending on whether it is a split slope/diagonal
//  or not.  it also has a cieling or not.  it also has potential for points on it
//  to be modified by the parameter.  there is also a double bit for vsplits.  
// Each floor has 3 or 4 points, depending on diagonal nature or not...

// this is expressed as an 8 element reference, containing a header flags list, a 
//  byte containing the number of points per floor element, and a 6 byte data area
//  holding either a 4 element floor or 1 or 2 3 element floors

typedef struct {
   uchar flags;
   uchar ptsper;
   uchar data[6];
} TilesToFloors;

#define FRFLRFLG_2ELEM 0x40
#define FRFLRSHF_2ELEM 6
#define FRFLRFLG_USEPR 0x20
#define FRFLRFLG_DBL   0x10
#define FRFLRFLG_NOTOP 0x08
#define FRFLRFLG_PMSK  0x07  // not currently used, but could pack ptsper into struct

// note set up for zany xor hacking for floor decode
// ie we can xor zmod with 1 if flipped, or and with ~
//  if we want a flat floor or cieling
#define FRFLRDATA_ZMOD FRPTSZMOD
#define FRFLRDATA_PMSK FRPTSPTOFF

#define FRTILEFLOORCNT FRTILETYPES

#ifndef __FRTABLES_SRC
extern TilesToFloors tile_floors[FRTILEFLOORCNT];
#endif

//======== Normals
#define FRWNORM_MAX    8
#define FRWNORM_MASK   0xf
#define FRWNORM_SHFT   4

#define FRFNORM_SLPN   0
#define FRFNORM_SLPE   1
#define FRFNORM_SLPS   2
#define FRFNORM_SLPW   3
#define FRFNORM_VZERO  4
#define FRFNORM_VFULL  5
#define FRFNORM_VZ_MIR 6
#define FRFNORM_VF_MIR 7

#ifndef __FRTABLES_SRC
extern ushort fr_wnorm_list[FRWALLPTSCNT];
extern uchar  fr_fnorm_list[FRTILEFLOORCNT];
extern fix fr_norm_elements[FRWNORM_MAX+1];
#endif

//======== Obstruct

// face obstruct data contains the width data for each face of each
//  tile.  The format is l parm r parm packed into a byte, which can be looked 
//  up eventually in the big table, as opposed to actively decoded.

#define FO_L_PARM 0x80
#define FO_L_PMSK 0x38
#define FO_L_SHFT 3
#define FO_R_PARM 0x40
#define FO_R_PMSK 0x07
#define FO_R_SHFT 0
#define FO_LR_MSK 0x3F
#define FO_HT_MSK 0xC0

#define FACE_CNT 4

#define FRFACEOBSTRUCTCNT FRTILETYPES
#define FOBASECODES       64

#ifndef __FRTABLES_SRC
extern uchar face_obstruct[FRFACEOBSTRUCTCNT][FACE_CNT];
extern fix   fo_unpack[FOBASECODES][2];
extern fix   fo_anti_unpack[FOBASECODES][2];
#endif

//======== Merger
// what to or pt_codes with prior to derefing for mirror compatibility
// mirror by flr/ciel by xor/and
#ifndef __FRTABLES_SRC
extern uchar merge_masks[5][2][2];
extern uchar mmask_facelet[5][2][2];
#endif

//======== Tilelets

// a tilelet is a list of facelet entries

// there is a 16 bit pointer in each map square to a tile
//  if it is NULL, there are no external walls
//  otherwise, the low 4 bits are length, the top 12 an offset into the
//   tilelet list array, where # 16bit facelet id's live 

typedef ushort TileLet;

#define TL_LEN_MASK (0x000F)
#define TL_PTR_MASK (0xFFF0)
#define TL_PTR_SHFT 4

#define TileLet_Len(x) (x&TL_LEN_MASK)
#define TileLet_Ptr(x) (x&TL_PTR_MASK) // at 16 val res?, or shift down?
#define TileLet_Idx(x) (x>>TL_PTR_SHFT)

//======== Facelets

// a facelet id contains a 4bit header, 2 bits of type and 2 bits of which face
// followed by a 12 bit value, either data or ptr based on type

// types are:
//   simple - 5 bit top 5 bit bottom
//   middle - ptr to 4 5 bit fields + 4 bit width codes (3 bytes)
//   full   - ptr to 4 8 bit heights + 2 4 bit width codes (5 bytes)

typedef ushort FaceLet;

#define FL_LEN_MASK  0x0003
#define FL_FACE_MASK 0x000C
#define FL_DATA_MASK 0xFFF0
#define FL_DATA_SHFT 4

#define FaceLet_Len(x)  (x&FL_LEN_MASK)
#define FaceLet_Face(x) (x&FL_FACE_MASK)
#define FaceLet_Ptr(x)  (x&FL_DATA_MASK)
#define FaceLet_Data(x) (x>>FL_DATA_SHFT)


