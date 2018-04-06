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
 * FrTables.c
 *
 * $Source: n:/project/cit/src/RCS/frtables.c $
 * $Revision: 1.2 $
 * $Author: dc $
 * $Date: 1994/01/12 22:06:34 $
 *
 * Citadel Renderer
 *  tables for the tile definitions/quadrant codes
 *  
 * $Log: frtables.c $
 * Revision 1.2  1994/01/12  22:06:34  dc
 * facelet hacking, new terrain function setup
 * 
 * Revision 1.1  1994/01/02  17:12:13  dc
 * Initial revision
 * 
 */

#define __FRTABLES_SRC

#include "frintern.h"
#include "frtables.h"
#include "tilename.h"

//======== Points
pt_mods pt_deref[FRPTSUNIQUE]=
{
 {0,0,0},{0,1,fix_make(0,0x2000)},{0,1,fix_make(0,0x8000)},{0,1,fix_make(0,0xE000)},
 {1,0,0},{3,1,fix_make(0,0x2000)},{3,1,fix_make(0,0x8000)},{3,1,fix_make(0,0xE000)},
 {2,0,0},{0,2,fix_make(0,0x2000)},{0,2,fix_make(0,0x8000)},{0,2,fix_make(0,0xE000)},
 {3,0,0},{1,2,fix_make(0,0x2000)},{1,2,fix_make(0,0x8000)},{1,2,fix_make(0,0xE000)},
 {0,1,fix_make(0,FROCTNUM)},{0,1,fix_make(0,0x10000-FROCTNUM)},
 {3,1,fix_make(0,FROCTNUM)},{3,1,fix_make(0,0x10000-FROCTNUM)},
 {0,2,fix_make(0,FROCTNUM)},{0,2,fix_make(0,0x10000-FROCTNUM)},
 {1,2,fix_make(0,FROCTNUM)},{1,2,fix_make(0,0x10000-FROCTNUM)}
};

#define _uv1 0x100
#define _uva 0x020
#define _uvb (FROCTNUM>>8)
#define _uvc 0x080
#define _uvd ((0x10000-FROCTNUM)>>8)
#define _uve 0x0E0
// points verse rotation vs unique
ushort pt_uv[FRPTSUNIQUE][4][2]=
{
 {{0,_uv1},{_uv1,_uv1},{_uv1,0},{0,0}},
 {{0,_uve},{_uve,_uv1},{_uv1,_uva},{_uva,0}},
 {{0,_uvc},{_uvc,_uv1},{_uv1,_uvc},{_uvc,0}},
 {{0,_uva},{_uva,_uv1},{_uv1,_uve},{_uve,0}},
 {{0,0},{0,_uv1},{_uv1,_uv1},{_uv1,0}},
 {{_uv1,_uve},{_uve,0},{0,_uva},{_uva,_uv1}},
 {{_uv1,_uvc},{_uvc,0},{0,_uvc},{_uvc,_uv1}},
 {{_uv1,_uva},{_uva,0},{0,_uve},{_uve,_uv1}},
 {{_uv1,0},{0,0},{0,_uv1},{_uv1,_uv1}},
 {{_uva,_uv1},{_uv1,_uve},{_uve,0},{0,_uva}},
 {{_uvc,_uv1},{_uv1,_uvc},{_uvc,0},{0,_uvc}},
 {{_uve,_uv1},{_uv1,_uva},{_uva,0},{0,_uve}},
 {{_uv1,_uv1},{_uv1,0},{0,0},{0,_uv1}},
 {{_uva,0},{0,_uve},{_uve,_uv1},{_uv1,_uva}},
 {{_uvc,0},{0,_uvc},{_uvc,_uv1},{_uv1,_uvc}},
 {{_uve,0},{0,_uva},{_uva,_uv1},{_uv1,_uve}},
 {{0,_uvd},{_uvd,_uv1},{_uv1,_uvb},{_uvb,0}},
 {{0,_uvb},{_uvb,_uv1},{_uv1,_uvd},{_uvd,0}},
 {{_uv1,_uvd},{_uvd,0},{0,_uvb},{_uvb,_uv1}},
 {{_uv1,_uvb},{_uvb,0},{0,_uvd},{_uvd,_uv1}},
 {{_uvb,_uv1},{_uv1,_uvd},{_uvd,0},{0,_uvb}},
 {{_uvd,_uv1},{_uv1,_uvb},{_uvb,0},{0,_uvd}},
 {{_uvb,0},{0,_uvd},{_uvd,_uv1},{_uv1,_uvb}},
 {{_uvd,0},{0,_uvb},{_uvb,_uv1},{_uv1,_uvd}}
};

fix pt_offs[FRPTSOFFS]=
 {fix_make(0,0),fix_make(0,0x2000),fix_make(0,FROCTNUM),fix_make(0,0x8000),
  fix_make(0,-FROCTNUM),fix_make(0,0xE000),fix_make(1,0),0xffffffff};

uchar pt_from_faceoff[4][FRPTSOFFS]=
{
 {0x4,0xD,0x16,0xE,0x17,0xF,0x8,0x4},
 {0x8,0x7,0x13,0x6,0x12,0x5,0xC,0x8},
 {0xC,0xB,0x15,0xA,0x14,0x9,0x0,0xC},
 {0x0,0x1,0x10,0x2,0x11,0x3,0x4,0x0}
};

// #define fxptoff(n) fix_make(0,0x2000*n)
// {fxptoff(0),fxptoff(1),fxptoff(2),fxptoff(3),fxptoff(4),fxptoff(5),fxptoff(6),fxptoff(7)};
// {fix_make(0,0),fix_make(0,0x2000),fix_make(0,0x8000),fix_make(0,0xe000)};

//======== WallstoPts
// convience macros for building the pt_wall table
#define zBo   FRPTSZBASE
#define zBU   FRPTSZBASEU
#define zTo   FRPTSZCEIL
#define zTD   FRPTSZCEILD
// actual wall list builders
#define wall_pt(z1,z2,z3,z4,h1,h2,h3,h4) {z1|h1,z2|h2,z3|h3,z4|h4}
#define norm_wall(lft,rgt)    wall_pt(zTo,zTo,zBo,zBo,lft,rgt,rgt,lft)
#define oct_wall(tl,tr,br,bl) wall_pt(zTo,zTo,zTD,zTD,tl,tr,br,bl),wall_pt(zTD,zTD,zBU,zBU,bl,br,br,bl),wall_pt(zBU,zBU,zBo,zBo,bl,br,tr,tl)
#define parm_walls(lft,rgt)   wall_pt(zBU,zBU,zBo,zBo,lft,rgt,rgt,lft),wall_pt(zTo,zTo,zTD,zTD,lft,rgt,rgt,lft)

WallsToPts wall_pts[FRWALLPTSCNT]=
{
 // main diagonals
 norm_wall(12,4), norm_wall(0,8), norm_wall(8,0), norm_wall(4,12),
 // slope 1/2 quarter diagonals
 norm_wall(6,0), norm_wall(0,6), norm_wall(8,2), norm_wall(2,8),
 norm_wall(12,2), norm_wall(2,12), norm_wall(6,4), norm_wall(4,6),
 // slope 2 quarter diagonals
 norm_wall(14,0), norm_wall(0,14), norm_wall(8,10), norm_wall(10,8),
 norm_wall(10,4), norm_wall(4,10), norm_wall(12,14), norm_wall(14,12),
 // halve tiles
 norm_wall(6,2), norm_wall(2,6), norm_wall(14,10), norm_wall(10,14),
 // one foot walls
 norm_wall(5,1), norm_wall(3,7), norm_wall(15,11), norm_wall(9,13),
 // triangle
 wall_pt(zTo,zTo,zBo,zBo,10,14,4,0),
 wall_pt(zTo,zTo,zBo,zBo,14,10,12,8),
 wall_pt(zTo,zTo,zBo,zBo,6,2,0,12),
 wall_pt(zTo,zTo,zBo,zBo,2,6,8,4),
 // oct NS  (note each macro expands to 3 walls)
 oct_wall(0x14,0x16,4,0), oct_wall(0x17,0x15,12,8),
 oct_wall(0x12,0x10,0,12), oct_wall(0x11,0x13,8,4),
 // parm diagonals, (each is 2 walls)
 parm_walls(12,4), parm_walls(0,8), parm_walls(8,0), parm_walls(4,12),
 // normal edges (n,e,s,w)
 norm_wall(4,8), norm_wall(8,12), norm_wall(12,0), norm_wall(0,4)
};

//======== TilestoWalls
// shorthands for tile tables
#define wN     FRWALLNORTH
#define wE     FRWALLEAST
#define wS     FRWALLSOUTH
#define wW     FRWALLWEST
#define wI(c)  (c)

#define tTw(ext,intcnt,intbase) {ext|wI(intcnt),intbase}

TilesToWalls tile_walls[FRTILEWALLCNT]=
{
 // solid, open
 tTw(0,0,0), tTw(wN|wE|wS|wW,0,0),
 // main diagonals
 tTw(wE|wS,1,1), tTw(wS|wW,1,3), tTw(wW|wN,1,2), tTw(wN|wE,1,0),
 // basic slopes
 tTw(wN|wE|wS|wW,0,0), tTw(wN|wE|wS|wW,0,0), tTw(wN|wE|wS|wW,0,0), tTw(wN|wE|wS|wW,0,0),
 // zany slopes
 tTw(wN|wE|wS|wW,0,0), tTw(wN|wE|wS|wW,0,0), tTw(wN|wE|wS|wW,0,0), tTw(wN|wE|wS|wW,0,0),
 tTw(wN|wE|wS|wW,0,0), tTw(wN|wE|wS|wW,0,0), tTw(wN|wE|wS|wW,0,0), tTw(wN|wE|wS|wW,0,0),
 // diagonal splits
 tTw(wN|wE|wS|wW,2,46), tTw(wN|wE|wS|wW,2,50), tTw(wN|wE|wS|wW,2,48), tTw(wN|wE|wS|wW,2,44),
 // oct
 tTw(wN|wS,6,32), tTw(wE|wW,6,38),
 // tri
 tTw(wN|wS,2,28), tTw(wE|wW,2,30),
 // heinous 1/4 diagonals
 tTw(wE|wS|wW,1,11), tTw(wN|wE|wS,1,16), tTw(wN|wE|wW,1,4),  tTw(wN|wE|wS,1,13),
 tTw(wE|wS|wW,1,7),  tTw(wN|wS|wW,1,14), tTw(wN|wE|wW,1,8),  tTw(wN|wS|wW,1,19),
 // heinous 3/4 diagonals, note strange symmetry, ie. 1and4 are -1, 2and3 +1, faces rev.
 tTw(wN|wE,1,10), tTw(wS|wW,1,17), tTw(wE|wS,1,5), tTw(wN|wW,1,12),
 tTw(wN|wW,1,6),  tTw(wE|wS,1,15), tTw(wS|wW,1,9), tTw(wN|wE,1,18),
 // vertical split
 tTw(wN|wE|wS|wW,0,0), 
 // halves
 tTw(wE|wS|wW,1,21), tTw(wN|wS|wW,1,22), tTw(wN|wE|wW,1,20), tTw(wN|wE|wS,1,23),
 // thin walls
 tTw(wE|wS|wW,1,25), tTw(wN|wS|wW,1,26), tTw(wN|wE|wW,1,24), tTw(wN|wE|wS,1,27)
};

//======== TilesToFloors

#define zNo   (0)
#define zPa   (FRFLRDATA_ZMOD)

#define flg2  (FRFLRFLG_2ELEM)
#define flgP  (FRFLRFLG_USEPR)
#define flgNT (FRFLRFLG_NOTOP)

#define fF3(p1,p2,p3)          {0,3,0x##p1##,0x##p2##,0x##p3##,0,0,0}
#define fF4(p1,p2,p3,p4)       {0,4,0x##p1##,0x##p2##,0x##p3##,0x##p4##,0,0}
#define fT4(p1,p2,p3,p4)       {flgNT,4,0x##p1##,0x##p2##,0x##p3##,0x##p4##,0,0}

#define fS3cc(pl,p2,p3,p4,p5)  {flg2|flgP,3,zPa|(0x##p2##),zPa|(0x##p3##),0x##pl##,zPa|(0x##p4##),zPa|(0x##p5##),0x##pl##}
#define fS3cv(ph,p2,p3,p4,p5)  {flg2|flgP,3,0x##p2##,0x##p3##,zPa|(0x##ph##),0x##p4##,0x##p5##,zPa|(0x##ph##)}
#define fS4(p1,p2,p3,p4)       {flgP,4,zPa|(0x##p1##),zPa|(0x##p2##),0x##p3##,0x##p4##,0,0}
#define fSspl(pla,pha,pm1,pm2) {flg2|flgP,3,0x##pla##,0x##pm1##,0x##pm2##,zPa|(0x##pha##),zPa|(0x##pm2##),zPa|(0x##pm1##)}

TilesToFloors tile_floors[FRTILEFLOORCNT]=
{
 // solid, open
 {0,0,0,0,0,0,0,0},fF4(4,8,C,0),
 // main diagonals
 fF3(8,C,0),fF3(C,0,4),fF3(4,8,0),fF3(4,8,C),
 // basic slopes
 fS4(4,8,C,0), fS4(8,C,0,4), fS4(C,0,4,8), fS4(0,4,8,C),
 // zany slopes
 fS3cc(C,4,8,0,4),fS3cc(0,8,C,4,8),fS3cc(4,C,0,8,C),fS3cc(8,0,4,C,0),
 fS3cv(C,4,8,0,4),fS3cv(0,8,C,4,8),fS3cv(4,C,0,8,C),fS3cv(8,0,4,C,0),
 // diagonal splits
 fSspl(C,4,0,8), fSspl(0,8,4,C), fSspl(4,C,8,0), fSspl(8,0,C,4),
 // oct
 fF4(16,17,15,14),fF4(13,12,10,11),
 // tri
 fT4(4,8,C,0), fT4(4,8,C,0),
 // heinous 1/4 diagonals
 fF4(4,6,C,0),fF4(4,8,C,A),fF4(4,8,6,0),fF4(E,8,C,0),
 fF4(8,C,0,2),fF4(4,8,A,0),fF4(4,8,C,2),fF4(E,C,0,4),
 // heinous 3/4 diagonals
 fF3(4,8,6),fF3(4,A,0),fF3(6,C,0),fF3(4,E,0),
 fF3(4,8,2),fF3(8,C,A),fF3(2,C,0),fF3(8,C,E),
 // vertical split
 {FRFLRFLG_DBL,4,4,8,0xC,0,0,0},
 // halves
 fF4(2,6,C,0),fF4(4,E,A,0),fF4(4,8,6,2),fF4(E,8,C,A),
 // thin walls
 fF4(3,7,C,0),fF4(4,F,B,0),fF4(4,8,5,1),fF4(D,8,C,9)
};

//======== Normals

//#define _fp1   fix_make(1,0)
//#define _fpdg  fix_make(0,46340)
//#define _fphx  fix_make(0,29308)  
//#define _fphy  fix_make(0,58616)

#define ff1   0 // fix_make(1,0)
#define fhy   1 // fix_make(0,58616)
#define fdg   2 // fix_make(0,46340)
#define fhx   3 // fix_make(0,29308)  

#define snp   4
#define sng   0

#define pff1  (ff1|snp)
#define nff1  (ff1|sng)
#define pfhy  (fhy|snp)
#define nfhy  (fhy|sng)
#define pfdg  (fdg|snp)
#define nfdg  (fdg|sng)
#define pfhx  (fhx|snp)
#define nfhx  (fhx|sng)
#define npnp  (8)

#define zm 0x4000
#define ym 0x2000
#define xm 0x1000

#define hZ    (0x2<<12)
#define hY    (0x1<<12)
#define hYZ   (0x3<<12)
#define hX    (0x4<<12)
#define hXZ   (0x6<<12)
#define hXY   (0x5<<12)
#define hXYZ  (0x7<<12)

#define mk_mnorm(m,xn,yn,zn)            (m|(xn<<8)|(yn<<4)|zn)
#define mk_hnorm(h,xn,yn,zn)            (h|(xn<<8)|(yn<<4)|zn)
#define mk_norm(xn,yn,zn)            ((xn<<8)|(yn<<4)|zn)

fix fr_norm_elements[9]=
{
   -fix_make(1,0),-fix_make(0,58616),-fix_make(0,46340),-fix_make(0,29308),
    fix_make(1,0), fix_make(0,58616), fix_make(0,46340), fix_make(0,29308), fix_make(0,0)
};

ushort fr_wnorm_list[FRWALLPTSCNT]=
{
 // main diagonals
 mk_hnorm(hXY,pfdg,pfdg,npnp), mk_hnorm(hXY,pfdg,nfdg,npnp), mk_hnorm(hXY,nfdg,pfdg,npnp), mk_hnorm(hXY,nfdg,nfdg,npnp),
 // slope 1/2 quarter diagonals
 mk_hnorm(hXY,nfhx,pfhy,npnp), mk_hnorm(hXY,pfhx,nfhy,npnp), mk_hnorm(hXY,nfhx,pfhy,npnp), mk_hnorm(hXY,pfhx,nfhy,npnp),
 mk_hnorm(hXY,pfhx,pfhy,npnp), mk_hnorm(hXY,nfhx,nfhy,npnp), mk_hnorm(hXY,pfhx,pfhy,npnp), mk_hnorm(hXY,nfhx,nfhy,npnp),
 // slope 2 quarter diagonals
 mk_hnorm(hXY,nfhy,pfhx,npnp), mk_hnorm(hXY,pfhy,nfhx,npnp), mk_hnorm(hXY,nfhy,pfhx,npnp), mk_hnorm(hXY,pfhy,nfhx,npnp),
 mk_hnorm(hXY,pfhy,pfhx,npnp), mk_hnorm(hXY,nfhy,nfhx,npnp), mk_hnorm(hXY,pfhy,pfhx,npnp), mk_hnorm(hXY,nfhy,nfhx,npnp),
 // halve tiles
 mk_hnorm(hY,npnp,pff1,npnp), mk_hnorm(hY,npnp,nff1,npnp), mk_hnorm(hX,nff1,npnp,npnp), mk_hnorm(hX,pff1,npnp,npnp),
 // one foot walls
 mk_hnorm(hY,npnp,pff1,npnp), mk_hnorm(hY,npnp,nff1,npnp), mk_hnorm(hX,nff1,npnp,npnp), mk_hnorm(hX,pff1,npnp,npnp),
 // triangle
 mk_hnorm(hXZ,pfhy,npnp,pfhx), mk_hnorm(hXZ,nfhy,npnp,pfhx), mk_hnorm(hYZ,npnp,pfhy,pfhx), mk_hnorm(hYZ,npnp,nfhy,pfhx),
 // oct NS
 mk_hnorm(hXZ,pfdg,npnp,pfdg), mk_hnorm(hX,pff1,npnp,npnp), mk_hnorm(hXZ,pfdg,npnp,nfdg),
 mk_hnorm(hXZ,nfdg,npnp,pfdg), mk_hnorm(hX,nff1,npnp,npnp), mk_hnorm(hXZ,nfdg,npnp,nfdg),
 mk_hnorm(hYZ,npnp,pfdg,pfdg), mk_hnorm(hY,npnp,pff1,npnp), mk_hnorm(hYZ,npnp,pfdg,nfdg),
 mk_hnorm(hYZ,npnp,nfdg,pfdg), mk_hnorm(hY,npnp,nff1,npnp), mk_hnorm(hYZ,npnp,nfdg,nfdg),
 // parm diagonals, (each is 2 walls)
 mk_hnorm(hXY,pfdg,pfdg,npnp), mk_hnorm(hXY,pfdg,nfdg,npnp), mk_hnorm(hXY,nfdg,pfdg,npnp), mk_hnorm(hXY,nfdg,nfdg,npnp),
 mk_hnorm(hXY,pfdg,pfdg,npnp), mk_hnorm(hXY,pfdg,nfdg,npnp), mk_hnorm(hXY,nfdg,pfdg,npnp), mk_hnorm(hXY,nfdg,nfdg,npnp),
 // normal edges (n,s,e,w)
 mk_hnorm(hY,npnp,nff1,npnp), mk_hnorm(hX,nff1,npnp,npnp), mk_hnorm(hY,npnp,pff1,npnp), mk_hnorm(hX,pff1,npnp,npnp)
};

// direction the vector heads, ie the low side
#define slpN      FRFNORM_SLPN 
#define slpE      FRFNORM_SLPE 
#define slpS      FRFNORM_SLPS 
#define slpW      FRFNORM_SLPW 
#define vzero     FRFNORM_VZERO
#define vfull     FRFNORM_VFULL
#define sl2N      (0<<4)
#define sl2E      (1<<4)
#define sl2S      (2<<4)
#define sl2W      (3<<4)
#define vrealfull ((vfull<<4)|vfull)

uchar fr_fnorm_list[FRTILEFLOORCNT]=
{
 // solid, open
 vfull, vfull,
 // main diagonals
 vfull, vfull, vfull, vfull,
 // basic slopes
 slpS, slpW, slpN, slpE,
 // zany slopes
 sl2E|slpS, sl2S|slpW, sl2W|slpN, sl2N|slpE,
 sl2W|slpN, sl2N|slpE, sl2E|slpS, sl2S|slpW,
 // diagonal splits
 vrealfull, vrealfull, vrealfull, vrealfull,
 // oct
 vfull, vfull,
 // tri
 vfull, vfull,
 // heinous 1/4 diagonals
 vfull, vfull, vfull, vfull, vfull, vfull, vfull, vfull,
 // heinous 3/4 diagonals
 vfull, vfull, vfull, vfull, vfull, vfull, vfull, vfull,
 // vertical split
 vfull,
 // halves
 vfull, vfull, vfull, vfull, 
 // thin walls
 vfull, vfull, vfull, vfull
}; 

//======== Obstruct

#define ZP    (1)
#define lS    (7)
#define rS    (6)
#define foNul (0xff)  // secret no freespace, ie. cant get through
#define foClr (0x06)  // pt0 to 6, ie. fully empty
#define foLft (0x03) 
#define foRgt (0x1E)

#define fo(l,r)                      ((l<<FO_L_SHFT)|r)
#define fo4(l1,r1,l2,r2,l3,r3,l4,r4) {fo(l1,r1),fo(l2,r2),fo(l3,r3),fo(l4,r4)}
#define zclr(p1,p2)                  (p1<<lS|p2<<rS|foClr)
#define foF(p1,p2,p3,p4,p5,p6,p7,p8) {zclr(p1,p2),zclr(p3,p4),zclr(p5,p6),zclr(p7,p8)}
#define fofo(f1,f2,f3,f4)            {f1,f2,f3,f4}
#define foC()                        {foClr,foClr,foClr,foClr}
#define foN()                        {foNul,foNul,foNul,foNul}

uchar face_obstruct[FRFACEOBSTRUCTCNT][FACE_CNT]=
{
 // solid, open
 foN(), foC(),
 // main diagonals
 fofo(foNul,foClr,foClr,foNul),
 fofo(foNul,foNul,foClr,foClr),
 fofo(foClr,foNul,foNul,foClr),
 fofo(foClr,foClr,foNul,foNul),
 // basic slopes
 foF(ZP,ZP,ZP,0,0,0,0,ZP),
 foF(0,ZP,ZP,ZP,ZP,0,0,0),
 foF(0,0,0,ZP,ZP,ZP,ZP,0),
 foF(ZP,0,0,0,0,ZP,ZP,ZP),
 // zany slopes
 foF(ZP,ZP,ZP,0,0,ZP,ZP,ZP),
 foF(ZP,ZP,ZP,ZP,ZP,0,0,ZP),
 foF(0,ZP,ZP,ZP,ZP,ZP,ZP,0),
 foF(ZP,0,0,ZP,ZP,ZP,ZP,ZP),
 foF(0,0,0,ZP,ZP,0,0,0),
 foF(0,0,0,0,0,ZP,ZP,0),
 foF(ZP,0,0,0,0,0,0,ZP),
 foF(0,ZP,ZP,0,0,0,0,0),
 // diagonal splits
 foF(ZP,ZP,0,0,0,0,ZP,ZP),
 foF(ZP,ZP,ZP,ZP,0,0,0,0),
 foF(0,0,ZP,ZP,ZP,ZP,0,0),
 foF(0,0,0,0,ZP,ZP,ZP,ZP),
 // oct
 fofo(foClr,foNul,foClr,foNul),
 fofo(foNul,foClr,foNul,foClr),
 // tri
 fofo(foClr,foNul,foClr,foNul),
 fofo(foNul,foClr,foNul,foClr),
 // heinous 1/4 diagonals
 fofo(foNul,foRgt,foClr,foClr),
 fofo(foClr,foClr,foLft,foNul),
 fofo(foClr,foLft,foNul,foClr),
 fofo(foRgt,foClr,foClr,foNul),
 fofo(foNul,foClr,foClr,foLft),
 fofo(foClr,foNul,foRgt,foClr),
 fofo(foClr,foClr,foNul,foRgt),
 fofo(foLft,foNul,foClr,foClr),
 // heinous 3/4 diagonals
 fofo(foClr,foLft,foNul,foNul),
 fofo(foNul,foNul,foRgt,foClr),
 fofo(foNul,foRgt,foClr,foNul),
 fofo(foLft,foNul,foNul,foClr),
 fofo(foClr,foNul,foNul,foRgt),
 fofo(foNul,foClr,foLft,foNul),
 fofo(foNul,foNul,foClr,foLft),
 fofo(foRgt,foClr,foNul,foNul),
 // vertical split
 foC(),
 // halves
 fofo(foNul,foRgt,foClr,foLft),
 fofo(foLft,foNul,foRgt,foClr),
 fofo(foClr,foLft,foNul,foRgt),
 fofo(foRgt,foClr,foLft,foNul),
 // thin walls
 fo4(0x1f,0xf,1,6,0,6,0,5),
 fo4(0,5,0x1f,0xf,1,6,0,6),
 fo4(0,6,0,5,0x1f,0xf,1,6),
 fo4(1,6,0,6,0,5,0x1f,0xf)
};

//======== Merger
// for floor point mods, masking to floor code
// based on actual instance

#define gtF  (0x00)
#define gtC  (0x40)
#define nFlp (0x00)
#define dFlp (0x80)
#define nFlt (0x80)
#define iFlt (0x00)

#define mm(x,y) {x|0,(uchar)(y|(~nFlt))}

// note we flip match, because in new data structure, we go down from ceiling always

// mirror by flr/ciel by xor/and
uchar merge_masks[5][2][2]=
{
 {mm(gtF|nFlp,nFlt),mm(gtC|dFlp,nFlt)},  // MAP_MATCH
 {mm(gtF|nFlp,nFlt),mm(gtC|nFlp,nFlt)},  // MAP_MIRROR
 {mm(gtF|nFlp,nFlt),mm(gtC|nFlp,iFlt)},  // MAP_CFLAT
 {mm(gtF|nFlp,iFlt),mm(gtC|dFlp,nFlt)},  // MAP_FFLAT
 {mm(gtF|nFlp,nFlt),mm(gtC|nFlp,nFlt)}   // secret no parameter
};

#define nfFlp (0x00)
#define dfFlp (0xC0)
#define nfFlt (0xC0)
#define ifFlt (0x00)

#define mf(x,y) {x|0,(uchar)(y|(~nfFlt))}

uchar mmask_facelet[5][2][2]=
{
 {mf(nfFlp,nfFlt),mf(dfFlp,nfFlt)},  // MAP_MATCH
 {mf(nfFlp,nfFlt),mf(nfFlp,nfFlt)},  // MAP_MIRROR
 {mf(nfFlp,nfFlt),mf(nfFlp,ifFlt)},  // MAP_CFLAT
 {mf(nfFlp,ifFlt),mf(dfFlp,nfFlt)},  // MAP_FFLAT
 {mf(nfFlp,nfFlt),mf(nfFlp,nfFlt)}   // secret no parameter
};

// filled in functionally
fix fo_unpack[FOBASECODES][2];
fix fo_anti_unpack[FOBASECODES][2];
uchar face_baseobstruct[FRFACEOBSTRUCTCNT][FACE_CNT];

int  fr_tables_build(void)
{
   int l,r,i,j;
   for (l=0; l<7; l++)
      for (r=0; r<7; r++)
      {
         fo_unpack[(l<<FO_L_SHFT)|r][0]=pt_offs[l];
         fo_unpack[(l<<FO_L_SHFT)|r][1]=pt_offs[r];
         fo_anti_unpack[(l<<FO_L_SHFT)|r][0]=fix_make(1,0)-pt_offs[l];
         fo_anti_unpack[(l<<FO_L_SHFT)|r][1]=fix_make(1,0)-pt_offs[r];
      }
   for (i=0; i<FRFACEOBSTRUCTCNT; i++)
      for (j=0; j<FACE_CNT; j++)
      {
         if ((j==1)||(j==2))
	         face_baseobstruct[i][j]=(face_obstruct[i][j]^0x3F)-0x9;  /* xors to get 7complement, then sub 0b01001 to get 6comp */
         else
	         face_baseobstruct[i][j]=face_obstruct[i][j];
      }
   _fr_ret;
}
