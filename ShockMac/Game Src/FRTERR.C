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
 * FrTerr.c
 *
 * $Source: r:/prj/cit/src/RCS/frterr.c $
 * $Revision: 1.27 $
 * $Author: jaemz $
 * $Date: 1994/09/06 05:03:37 $
 *
 * Citadel Renderer
 *  terrain parsing, internal and external faces, facelet
 *  physics interaction?
 */

#include <string.h>
#include <stdlib.h>

#include <stdio.h> // for FILE for now

#include "map.h"
#include "mapflags.h"
#include "objects.h"    // for ClearDealt
#include "frintern.h"
#include "frtables.h"
#include "tilename.h"
#include "frparams.h"
#include "frflags.h"
#include "frquad.h"
#include "frsubclp.h"
#include "textmaps.h"  // pain, sadness
#include "star.h"

#define FLIP_BITS

// for the game system to grab and deparse for distance and all
int _game_fr_tmap;

// these should go somewhere else...
#define MAX_HGT HGT_STEPS

#define FDT_TMPPTCNT 8

#define T_LEFT  0
#define T_RIGHT 1

// really need oct_low and oct_high mirrors for fhgt_list so we can +/- 33 as oct_parm

// globals
int  _fdt_dist;											// distance in hv squares to us
fix  _fr_fhgt_step;									// single hgt step
fix  _fr_fhgt_list[MAX_HGT+1];         		// all hgt steps fix mapping
sfix _fr_sfuv_list[(2*MAX_HGT)+1];   	// all hgt steps uv mapping
fix  slope_norm[MAX_HGT][3];             		// table for sloped floors

// these example values are for a 32wide map, they get set in frpipe when map is resized
int  wall_adds[]={DEF_MAP_SZ,1,-DEF_MAP_SZ,-1};  			// how to get from one tile to next ptr math
int  csp_trans_add[]={0,DEF_MAP_SZ,DEF_MAP_SZ+1,1}; 	// alternate mapping of same deltas

int _fdt_x, _fdt_y, _fdt_mask;					// implicit parameters to draw tile
MapElem *_fdt_mptr;                              	// more implicit parameters

// texture mapping function pointers.
void (*_fr_lit_floor_func)(int, g3s_phandle *, grs_bitmap *);	
void (*_fr_floor_func)(int, g3s_phandle *, grs_bitmap *);
void (*_fr_lit_wall_func)(int, g3s_phandle *, grs_bitmap *);
void (*_fr_wall_func)(int, g3s_phandle *, grs_bitmap *);
void (*_fr_lit_per_func)(int, g3s_phandle *, grs_bitmap *);
void (*_fr_per_func)(int, g3s_phandle *, grs_bitmap *);


#if _fr_defdbg(CURSOR)
int _fr_cursorx, _fr_cursory;
#endif

#if _fr_defdbg(ALTCAM)
int _fr_altcamx, _fr_altcamy;
#endif

// various nested locals, read static globals in C
static uchar         _fdt_tt;          				// local storage of our current tile's tiletype
static TilesToWalls  	_fdt_ttw;				// secret tiles to walls code
static TilesToFloors 	*_fdt_ttf;				// pointer to current floor spew
static uchar		 _fdt_hgts[5];				// parm, flr, ceil, fprm, cprm
static fix			 _fdt_fix_parm;    			// fix rep for parameter
static fix			 _fdt_cur_parm;			// dont pass around, as it is constant per tile
static char			 _fdt_icnt;						// internal walls count - can be made a local again, i think
static uchar		*_fdt_fo;						// current tile's fo data
static fix			_fdt_hgt_val;					// last point hgt world coordinate
static uchar		_fdt_whichpt;				//  fhgt_list_entry 0-MAX_HGT for last point looked up height w/parm adjust
static uchar		_fdt_hgt_pt;					// actual fdt_hgts value for this whichpt, since we use it 4-8 times for lit pts
g3s_phandle		_fdt_tmppts[8];				// these are used for all temporary point sets
static int			_fdt_wallid;					// current external wall id
static g3s_phandle  *_fdt_lcore;				// left core for external walls
static g3s_phandle  *_fdt_rcore;				// right core for external walls
static int   			_fdt_rbase;					// value of pbase at right of external walls, we use pbase itself for left
static int 			_fdt_me_flags;				// hold store the current map flags
static int			_fdt_wmap;					// current tmap family to use
static sfix			_fdt_slock;					// sfix value for vlock in square

int 					_fdt_pbase;					// where pbase is, for corner for idx lookup - sadly global for object lighting

static uchar		last_csp_fr=0;				// last frame annoyance
static uchar		 _fdt_flip;						// tile flippitude of cur tile
static bool			_fdt_terr;						// are we doing terrain or physics

// indirections for low level render functions
static void (*_fr_terr_int_wall)(int wall_id);
static void (*_fr_terr_ext_wall)(fix pt_list[4][2]);
static void (*_fr_terr_flr)(void);
static void (*_fr_terr_ceil)(void);
// how to render a composite inner wall
static void (*_fr_render_walls)(int which, int cnt);
// how to deal w/the object chain at a square
static void (*_fr_parse_obj)(void);

// in rendtool.c
#define TIM_WERE_AWAKE
#ifdef TIM_WERE_AWAKE
#define IsTpropNotStars() (textprops[_game_fr_tmap].force_dir==0)
#else
#define IsTpropNotStars() (_game_fr_tmap>=4)
#endif
extern bool draw_tmap_p(int ptcnt);
#define quik_draw_tmap_p(ptcnt) ((IsTpropNotStars())||(draw_tmap_p(ptcnt)))
//#define quik_draw_tmap_p(ptcnt) (TRUE)

// lookups into arrays
#define FDT_LK_FLR   0
#define FDT_LK_CEIL  1

// points in hgt structures
#define FDT_PT_PARM   0
#define FDT_PT_FLR    1
#define FDT_PT_CEIL   2
#define FDT_PT_FPRM   3
#define FDT_PT_CPRM   4

int _fr_terr_prim=1;

// Internal prototypes (for Mac version)
void _fr_figure_pt(g3s_phandle tmp, int pt_code);
fix get_light(fix dist_to);
int _fr_do_light_val(int which, fix dist_val);
int _obj_do_light(int which, fix dist);
void _fr_do_cspace(g3s_phandle wrk, int which);
void _fr_draw_wire_cpoly_4(void);
void _fr_draw_wire_cpoly_3or4(int cnt);
void flip_setup(int wall_code);
int fr_get_ext_gap(fix *face_l, uchar in_fo, uchar *hgts, uchar *mmptr);
int _fr_get_anti_gap(fix *face_l, uchar in_fo, uchar *hgts, uchar *mmptr);
void _render_3d_walls(int which, int cnt);
int merge_walls(fix *dst_wall,fix *i_wall,int i_cnt,fix *o_wall,int o_cnt);
void parse_clip_tile();
void fr_terr_cspace_pick(uchar do_wall);
int _fr_tfunc_flr_normal(fix *vec, int *lflg, int fnorm_entry);
int _fr_tfunc_ceil_normal(fix *vec, int *lflg, int fnorm_entry);
int do_fc_trans(fix *xy, fix *sc, fix *vals);
void do_floor_element(int nrm_mask);
void do_ceil_element(int nrm_mask);
void fr_tfunc_grab_start(void) ;
void fr_tfunc_grab_fast(int mask);




///// AAHRRHRHGGHGHG switch to secondary data cache!!!

// probably should have 3 of these, one for cspace, one for unlit, one for lantern, so on, swap in pointers, 
// and for asm have a table of calls to this which we switch if necessary at start of frame

// oh oh oh baby oh baby oh self modify height delta step baby now baby please
// really, this wants to become the threaded nightmare assembler point coder from hell
// and to use the 4 respts per map pt idea so we can reuse too
// 3 tests,
//   either succeed jump out and reuse
//   or fail all 3 (no jumps taken) and optimally build new one     

// deal with optimality of data structures when rewriting in asm

// SPEED

// the phone is ringing its 4 am
// if it's your friends then i dont want to hear from them
// please leave a number and a message at the tone
// or you can just go on and leave me alone
// and i...  dont want to know if you are lonely

// hmm.. have to deal with halve points and such
void _fr_figure_pt(g3s_phandle tmp, int pt_code)
{
   g3s_phandle *core;
   pt_mods *ptm;

   ptm=&pt_deref[pt_code&FRPTSPTOFF];
   switch (_fdt_pbase=ptm->base)
   {  // bad bad bad - hmmm... arrays?
   case 0: core=_fr_ptbase;   break;
   case 1: core=_fr_ptnext;   break;
   case 2: core=_fr_ptnext+1; break;
   case 3: core=_fr_ptbase+1; break;
//#ifndef SHIP
//   default: Warning(("_fr_terr_poly_wall: bad point base code %x gives %d\n",pt_code,ptm->base)); break;
//#endif
   }
   core+=_fdt_x;
   _fdt_whichpt=((pt_code&FRPTSZMASK)>>FRPTSZSHF);
   _fdt_hgt_pt =_fdt_hgts[_fdt_whichpt+1];
   _fdt_hgt_val=_fr_fhgt_list[_fdt_hgt_pt];
   g3_replace_add_delta_y(*core,tmp,-_fdt_hgt_val);
   switch (ptm->modcnt)
   {     // 1 function table
   case FRMODYAXIS: g3_add_delta_z(tmp,ptm->arg); break;
   case FRMODXAXIS: g3_add_delta_x(tmp,ptm->arg); break;
   case FRMODNONE: break;
   }
}


fix get_light(fix dist_to) // , fix dot_prod)
{
//   if (fr_normal_lights)
//	   dist_to+=fix_mul(dist_to>>fr_normal_shf,fix_make(1,0)-dot_prod);      // 1/(1<<norm_shf) * dist_to*(anti_dot_prod)
//   Spew(DSRC_TESTING_Test1,("v%s",fix_sprint(ft,dist_to)));
// wow, fix this up, make fixes in frp, so on
   if (dist_to<fix_make((int)_frp.lighting.rad[0],0))
      return fix_make((int)_frp.lighting.base[0],0);
   if (dist_to>fix_make((int)_frp.lighting.rad[1],0))
      return fix_make((int)_frp.lighting.base[1],0);
   return fix_mul(dist_to,_frp.lighting.slope)+_frp.lighting.yint;
}


#define get_light_t(targ,dist_to) \
   if (dist_to<fix_make((int)_frp.lighting.rad[0],0)) \
      targ=((int)_frp.lighting.base[0])<<8; \
   else if (dist_to>fix_make((int)_frp.lighting.rad[1],0)) \
      targ=((int)_frp.lighting.base[1])<<8; \
   else targ=(fix_mul(dist_to,_frp.lighting.slope)+_frp.lighting.yint)>>8

#ifndef NEW_WAY
#define set_terr_light(our_mp,which) \
      if (which==FRPTSZCEIL_DN) \
	      i=(int)me_light_ceil(our_mp)-(int)me_templight_ceil(our_mp); \
	   else \
	      i=(int)me_light_flr(our_mp)-(int)me_templight_flr(our_mp); \
      if (i<0) i=0
#else
#define set_terr_light(our_mp,which) \
   { \
      int j; \
      if (which==FRPTSZCEIL_DN) \
       { i=(int)me_light_ceil(our_mp); j=(int)me_templight_ceil(our_mp); } \
	   else \
	    { i=(int)me_light_flr(our_mp);  j=(int)me_templight_flr(our_mp);  } \
      i=min(i,j); \
   }
#endif

// should redo these to setup sfix with punch in not shift
// for now hack with which to get stuff running
// later we want an interpolator
// rewrite this in assembler
int _fr_do_light_val(int which, fix dist_val)
{
   MapElem *our_mp=_fdt_mptr;
   int i;

      our_mp+=csp_trans_add[_fdt_pbase];
      set_terr_light(our_mp,which);
      i<<=8;

   // speed this up, add fast dist check to trivial reject it.. ick, is this correct
   // secret sacred secret sacred secret gnosis... neat, eh?
   if ((_fr_curflags&FR_HACKCAM_MASK)==0)
   {
      if (_frp_light_bits_cam())
         if ((dist_val>>16)<=_frp.lighting.rad[1]+1)
   	   {
            fix light_res;
   	      get_light_t(light_res,dist_val);
   	      if (light_res<i) i-=light_res; else i=0;
   	   }
   }

   if (_frp.lighting.global_mod)
   {
		i-=_frp.lighting.global_mod;
	   if (i<0) i=0; else if (i>(15<<8)) i=(15<<8);
   }

   if (me_bits_rend3_x(our_mp))
   {
      i=0;
//		i-=(6<<8);     // ah-yep....
//	   if (i<0) i=0;
   }

   return i;

//   if (_frp_light_bits_norm())
//   wrk->i=i;
//   *((uchar *)&wrk->i)=i;
//   return wrk->i;
}

int _fr_do_light(g3s_phandle wrk, int which)
{
//   fix dval=abs(wrk->x)+abs(wrk->y)+abs(wrk->z));
//   fix dval=fix_fast_pyth_dist(fix_fast_pyth_dist(wrk->x,wrk->y),wrk->z);
// really, we want the original, not transformed, points
// how bout this mess, eh?
   static int x_mod[]={0,0,1,1}, y_mod[]={0,1,1,0};
   int _lgt_x, _lgt_y;
   fix dval;
   _lgt_x=_fdt_x+x_mod[_fdt_pbase];
   _lgt_y=_fdt_y+y_mod[_fdt_pbase];
   dval=fix_fast_pyth_dist(fix_fast_pyth_dist((_lgt_x<<16)-fr_camera_last[0],(_lgt_y<<16)-fr_camera_last[1]),fr_camera_last[2]-_fdt_hgt_val);
   wrk->i=_fr_do_light_val(which,dval);
//mprintf("f %x d %x from %x %x %x,%x %x %x\n",wrk->i,dval,_lgt_x,_lgt_y,_fdt_hgt_val,fr_camera_last[0],fr_camera_last[1],fr_camera_last[2]);
   return wrk->i;
}

int _obj_do_light(int which, fix dist)
{
   return _fr_do_light_val(which,dist);
}

void _fr_do_cspace(g3s_phandle wrk, int which)
{
   int col, dst, ncval;
   MapElem *our_mp=_fdt_mptr+csp_trans_add[_fdt_pbase];
   if (which==FRPTSZCEIL_DN) col=me_cybcolor_ceil(our_mp); else col=me_cybcolor_flr(our_mp);
   // light table based on radius here, somehow....
   dst=_fdt_dist-6;     // dimming code. i guess
   if (dst<0) dst=0; else if (dst>10) dst=10;
   dst<<=8;
   ncval=*((_fr_clut_list[0])+dst+col);
   wrk->rgb=grd_bpal[ncval];  // col
   wrk->p3_flags|=PF_RGB;
}

#define dlC FRPTSZCEIL_DN
#define dlF FRPTSZFLR_DN
#define _fr_pt_light(phnd)  _fr_do_light(phnd,_fdt_whichpt&FRPTSZPICK_DN)
#define _fr_pt_cspace(phnd) _fr_do_cspace((phnd),_fdt_whichpt&FRPTSZPICK_DN)

void _fr_draw_wire_cpoly_4(void)
{
   g3_draw_cline(_fdt_tmppts[0],_fdt_tmppts[1]);
   g3_draw_cline(_fdt_tmppts[1],_fdt_tmppts[2]);
   g3_draw_cline(_fdt_tmppts[2],_fdt_tmppts[3]);
   g3_draw_cline(_fdt_tmppts[3],_fdt_tmppts[0]); 
}

void _fr_draw_wire_cpoly_3or4(int cnt)
{
   g3_draw_cline(_fdt_tmppts[0],_fdt_tmppts[1]);
   g3_draw_cline(_fdt_tmppts[1],_fdt_tmppts[2]);
   if (cnt==3)
	   g3_draw_cline(_fdt_tmppts[2],_fdt_tmppts[0]);
   else
   {
	   g3_draw_cline(_fdt_tmppts[2],_fdt_tmppts[3]);
	   g3_draw_cline(_fdt_tmppts[3],_fdt_tmppts[0]);
   }
}

void flip_setup(int wall_code)
{
   int lflags=_fdt_me_flags;
   if (lflags&MAP_FLIP_FNCY_MASK)
   {
      lflags&=~MAP_FLIP_FNCY_MASK;                              /* clear fancy bit */
      lflags+=(((_fdt_x+_fdt_y+wall_code)&1)<<MAP_FLIP_PRTY_SHF); /* add in current parity */
      lflags&=~MAP_FLIP_FNCY_MASK;                              /* mod 1 within the bit field */
   }
   _fdt_flip=((lflags&MAP_FLIP_MASK)>>MAP_FLIP_SHF);
#ifdef FLIP_SPEW
   if (_fdt_me_flags&MAP_FLIP_MASK)
      mprintf("Flip now set to %x from %x,%x at %x %x t %x\n",_fdt_flip,lflags,_fdt_me_flags,_fdt_x,_fdt_y,wall_code);
   else if (_fdt_flip)
      mprintf("Hey, flip set, huh? %x %x %x\n",_fdt_x,_fdt_y,wall_code);
#endif
}

#ifdef FLIP_BITS
#define WALL_L_SET(pt) pt->uv.u=(_fdt_flip)<<8
#define WALL_R_SET(pt) pt->uv.u=((1+_fdt_flip)&1)<<8
#else
#define WALL_L_SET(pt) pt->uv.u=0<<8
#define WALL_R_SET(pt) pt->uv.u=1<<8
#endif

// uv for a wall
/// ARRRGH, sfix points suck.. make this a look up through _fdt_hgts+vhold?
#define wall_uv_l(tmp) \
   tmp->uv.v=_fr_sfuv_list[_fdt_hgt_pt]+_fdt_slock; \
   tmp->p3_flags|=PF_U|PF_V; \
   WALL_L_SET(tmp)

#define wall_uv_r(tmp) \
   tmp->uv.v=_fr_sfuv_list[_fdt_hgt_pt]+_fdt_slock; \
   tmp->p3_flags|=PF_U|PF_V; \
   WALL_R_SET(tmp)

#define wall_uv_l_i(tmp) \
   tmp->uv.v=_fr_sfuv_list[_fdt_hgt_pt]+_fdt_slock; \
   tmp->p3_flags|=PF_U|PF_V|PF_I; \
   WALL_L_SET(tmp)

#define wall_uv_r_i(tmp) \
   tmp->uv.v=_fr_sfuv_list[_fdt_hgt_pt]+_fdt_slock; \
   tmp->p3_flags|=PF_U|PF_V|PF_I; \
   WALL_R_SET(tmp)

// if ((pt_num+1)&2) tmp->u=0; else tmp->u=1<<8;

#ifndef PARTIAL_TILES
#define EXT_WALL_L_SET(pt) WALL_L_SET(pt)
#define EXT_WALL_R_SET(pt) WALL_R_SET(pt)
#else
#ifdef FLIP_BITS
#define EXT_WALL_L_SET(pt) pt->uv.u=(fix_flip+(flip_sign)*hgt_data[0])>>8
#define EXT_WALL_R_SET(pt) pt->uv.u=(fix_flip+(flip_sign)*hgt_data[0])>>8
#else
#define EXT_WALL_L_SET(pt) pt->uv.u=hgt_data[0]>>8
#define EXT_WALL_R_SET(pt) pt->uv.u=hgt_data[0]>>8
#endif
#endif

// really, these could use 0 and 1<<8 for u coordinates, since why not without partial tiles
#define ext_wall_uv_l(tmp,hgt_data) \
   tmp->uv.v=((fix_make(4,0)-hgt_data[1])>>8)+_fdt_slock; \
   tmp->p3_flags|=PF_U|PF_V; \
   EXT_WALL_L_SET(tmp)

#define ext_wall_uv_r(tmp,hgt_data) \
   tmp->uv.v=((fix_make(4,0)-hgt_data[1])>>8)+_fdt_slock; \
   tmp->p3_flags|=PF_U|PF_V; \
   EXT_WALL_R_SET(tmp)

#define ext_wall_uv_l_i(tmp,hgt_data) \
   tmp->uv.v=((fix_make(4,0)-hgt_data[1])>>8)+_fdt_slock; \
   tmp->p3_flags|=PF_U|PF_V|PF_I; \
   EXT_WALL_L_SET(tmp)

#define ext_wall_uv_r_i(tmp,hgt_data) \
   tmp->uv.v=((fix_make(4,0)-hgt_data[1])>>8)+_fdt_slock; \
   tmp->p3_flags|=PF_U|PF_V|PF_I; \
   EXT_WALL_R_SET(tmp)

// i can feel it in my bones
// im gonna spend my whole life alone

// tabs_int_wall
//#pragma disable_message(202)
static void _fr_null_int_wall(int wall_id) {}
//#pragma enable_message(202)

static void _fr_flat_int_wall(int wall_id)
{
   WallsToPts *wpt=&wall_pts[wall_id];
   // need to have face_code set and be ready with flip and hold and all that jazz
   _fr_figure_pt(_fdt_tmppts[0],wpt->ul);
   _fr_figure_pt(_fdt_tmppts[1],wpt->ur);
   _fr_figure_pt(_fdt_tmppts[2],wpt->lr);         
   _fr_figure_pt(_fdt_tmppts[3],wpt->ll);
   _fr_ndbg(NO_REND,g3_draw_poly((*fr_get_idx)(),4,_fdt_tmppts));
   _fr_sdbg(STATS,_frp.stats.int_wall++);
}

#ifdef FLAT_SUPPORT
static void _fr_flat_lit_int_wall(int wall_id)
{
   WallsToPts *wpt=&wall_pts[wall_id];
   // need to have face_code set and be ready with flip and hold and all that jazz
   _fr_figure_pt(_fdt_tmppts[0],wpt->ul); _fr_pt_light(_fdt_tmppts[0]); _fdt_tmppts[0]->p3_flags|=PF_I;
   _fr_figure_pt(_fdt_tmppts[1],wpt->ur); _fr_pt_light(_fdt_tmppts[1]); _fdt_tmppts[1]->p3_flags|=PF_I;
   _fr_figure_pt(_fdt_tmppts[2],wpt->lr); _fr_pt_light(_fdt_tmppts[2]); _fdt_tmppts[2]->p3_flags|=PF_I;
   _fr_figure_pt(_fdt_tmppts[3],wpt->ll); _fr_pt_light(_fdt_tmppts[3]); _fdt_tmppts[3]->p3_flags|=PF_I;
   gr_set_fcolor((*fr_get_idx)());
   _fr_ndbg(NO_REND,g3_draw_spoly(4,_fdt_tmppts));
   _fr_sdbg(STATS,_frp.stats.int_wall++);
}
#else
#define _fr_flat_lit_int_wall _fr_tmap_lit_int_wall
#endif

static void _fr_tmap_int_wall(int wall_id)
{
   WallsToPts *wpt=&wall_pts[wall_id];
   // need to have face_code set and be ready with flip and hold and all that jazz
   _fr_figure_pt(_fdt_tmppts[0],wpt->ul); wall_uv_l(_fdt_tmppts[0]);
   _fr_figure_pt(_fdt_tmppts[1],wpt->ur); wall_uv_r(_fdt_tmppts[1]);
   _fr_figure_pt(_fdt_tmppts[2],wpt->lr); wall_uv_r(_fdt_tmppts[2]);
   _fr_figure_pt(_fdt_tmppts[3],wpt->ll); wall_uv_l(_fdt_tmppts[3]);
   if (quik_draw_tmap_p(4))
    { _fr_ndbg(NO_REND,_fr_wall_func(4,_fdt_tmppts,(*fr_get_tmap)())); }
   _fr_sdbg(STATS,_frp.stats.int_wall++);
}

static void _fr_tmap_lit_int_wall(int wall_id)
{
   WallsToPts *wpt=&wall_pts[wall_id];
   // need to have face_code set and be ready with flip and hold and all that jazz
   _fr_figure_pt(_fdt_tmppts[0],wpt->ul); wall_uv_l_i(_fdt_tmppts[0]); _fr_pt_light(_fdt_tmppts[0]);
   _fr_figure_pt(_fdt_tmppts[1],wpt->ur); wall_uv_r_i(_fdt_tmppts[1]); _fr_pt_light(_fdt_tmppts[1]);
   _fr_figure_pt(_fdt_tmppts[2],wpt->lr); wall_uv_r_i(_fdt_tmppts[2]); _fr_pt_light(_fdt_tmppts[2]);
   _fr_figure_pt(_fdt_tmppts[3],wpt->ll); wall_uv_l_i(_fdt_tmppts[3]); _fr_pt_light(_fdt_tmppts[3]);
   if (quik_draw_tmap_p(4))
    { _fr_ndbg(NO_REND,_fr_lit_wall_func(4,_fdt_tmppts,(*fr_get_tmap)())); }
   _fr_sdbg(STATS,_frp.stats.int_wall++);
}

static void _fr_cspace_wire_int_wall(int wall_id)
{
   WallsToPts *wpt=&wall_pts[wall_id];
   // need to have face_code set and be ready with flip and hold and all that jazz
   _fr_figure_pt(_fdt_tmppts[0],wpt->ul); _fr_pt_cspace(_fdt_tmppts[0]);
   _fr_figure_pt(_fdt_tmppts[1],wpt->ur); _fr_pt_cspace(_fdt_tmppts[1]);
   _fr_figure_pt(_fdt_tmppts[2],wpt->lr); _fr_pt_cspace(_fdt_tmppts[2]);
   _fr_figure_pt(_fdt_tmppts[3],wpt->ll); _fr_pt_cspace(_fdt_tmppts[3]);
   _fr_ndbg(NO_REND,_fr_draw_wire_cpoly_4());
   _fr_sdbg(STATS,_frp.stats.int_wall++);
}

static void _fr_cspace_full_int_wall(int wall_id)
{
   WallsToPts *wpt=&wall_pts[wall_id];
   // need to have face_code set and be ready with flip and hold and all that jazz
   _fr_figure_pt(_fdt_tmppts[0],wpt->ul); _fr_pt_cspace(_fdt_tmppts[0]);
   _fr_figure_pt(_fdt_tmppts[1],wpt->ur); _fr_pt_cspace(_fdt_tmppts[1]);
   _fr_figure_pt(_fdt_tmppts[2],wpt->lr); _fr_pt_cspace(_fdt_tmppts[2]);
   _fr_figure_pt(_fdt_tmppts[3],wpt->ll); _fr_pt_cspace(_fdt_tmppts[3]);
   _fr_ndbg(NO_REND,g3_draw_cpoly(4,_fdt_tmppts));
   _fr_sdbg(STATS,_frp.stats.int_wall++);
}

// tabs_ext_wall

// this takes a wacky hgt data set 
// uses _fdt_wallid, _fdt_lcore, _fdt_rcore
//#pragma disable_message(202)
static void _fr_null_ext_wall(fix pt_list[4][2]) {}
//#pragma enable_message(202)

static void _fr_flat_ext_wall(fix pt_list[4][2])
{  // note we do this out of order so we can have left left right right, ie. note their indicies
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[0],-pt_list[0][1]); 
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[3],-pt_list[3][1]);
   _fdt_pbase=_fdt_rbase;
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[1],-pt_list[1][1]);
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[2],-pt_list[2][1]);
   _fr_ndbg(NO_REND,g3_draw_poly((*fr_get_idx)(),4,_fdt_tmppts));
   _fr_sdbg(STATS,_frp.stats.ext_wall++);
}

#ifdef FLAT_SUPPORT
static void _fr_flat_lit_ext_wall(fix pt_list[4][2])
{
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[0],-pt_list[0][1]); _fdt_hgt_val=-pt_list[0][1]; _fr_do_light(_fdt_tmppts[0],dlC); _fdt_tmppts[0]->p3_flags|=PF_I;
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[3],-pt_list[3][1]); _fdt_hgt_val=-pt_list[3][1]; _fr_do_light(_fdt_tmppts[3],dlF); _fdt_tmppts[3]->p3_flags|=PF_I;
   _fdt_pbase=_fdt_rbase;
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[1],-pt_list[1][1]); _fdt_hgt_val=-pt_list[1][1]; _fr_do_light(_fdt_tmppts[1],dlC); _fdt_tmppts[1]->p3_flags|=PF_I;
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[2],-pt_list[2][1]); _fdt_hgt_val=-pt_list[2][1]; _fr_do_light(_fdt_tmppts[2],dlF); _fdt_tmppts[2]->p3_flags|=PF_I;
   gr_set_fcolor((*fr_get_idx)());
   _fr_ndbg(NO_REND,g3_draw_spoly(4,_fdt_tmppts));
   _fr_sdbg(STATS,_frp.stats.ext_wall++);
}
#else
#define _fr_flat_lit_ext_wall _fr_tmap_lit_ext_wall
#endif

static void _fr_tmap_ext_wall(fix pt_list[4][2])
{
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[0],-pt_list[0][1]); ext_wall_uv_l(_fdt_tmppts[0],pt_list[0]);
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[3],-pt_list[3][1]); ext_wall_uv_l(_fdt_tmppts[3],pt_list[3]);
   _fdt_pbase=_fdt_rbase;
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[1],-pt_list[1][1]); ext_wall_uv_r(_fdt_tmppts[1],pt_list[1]);
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[2],-pt_list[2][1]); ext_wall_uv_r(_fdt_tmppts[2],pt_list[2]);
   if (quik_draw_tmap_p(4))
    { _fr_ndbg(NO_REND,_fr_wall_func(4,_fdt_tmppts,(*fr_get_tmap)())); }
   _fr_sdbg(STATS,_frp.stats.ext_wall++);
}

static void _fr_tmap_lit_ext_wall(fix pt_list[4][2])
{
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[0],-pt_list[0][1]); ext_wall_uv_l_i(_fdt_tmppts[0],pt_list[0]); _fdt_hgt_val=pt_list[0][1]; _fr_do_light(_fdt_tmppts[0],dlC); 
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[3],-pt_list[3][1]); ext_wall_uv_l_i(_fdt_tmppts[3],pt_list[3]); _fdt_hgt_val=pt_list[3][1]; _fr_do_light(_fdt_tmppts[3],dlF);
   _fdt_pbase=_fdt_rbase;
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[1],-pt_list[1][1]); ext_wall_uv_r_i(_fdt_tmppts[1],pt_list[1]); _fdt_hgt_val=pt_list[1][1]; _fr_do_light(_fdt_tmppts[1],dlC);
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[2],-pt_list[2][1]); ext_wall_uv_r_i(_fdt_tmppts[2],pt_list[2]); _fdt_hgt_val=pt_list[2][1]; _fr_do_light(_fdt_tmppts[2],dlF);
   if (quik_draw_tmap_p(4))
    { _fr_ndbg(NO_REND,_fr_lit_wall_func(4,_fdt_tmppts,(*fr_get_tmap)())); }
   _fr_sdbg(STATS,_frp.stats.ext_wall++);
}

static void _fr_cspace_wire_ext_wall(fix pt_list[4][2])
{
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[0],-pt_list[0][1]); _fr_do_cspace(_fdt_tmppts[0],dlC);
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[3],-pt_list[3][1]); _fr_do_cspace(_fdt_tmppts[3],dlF);
   _fdt_pbase=_fdt_rbase;
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[1],-pt_list[1][1]); _fr_do_cspace(_fdt_tmppts[1],dlC);
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[2],-pt_list[2][1]); _fr_do_cspace(_fdt_tmppts[2],dlF);
   _fr_ndbg(NO_REND,_fr_draw_wire_cpoly_4());
   _fr_sdbg(STATS,_frp.stats.ext_wall++);
}

static void _fr_cspace_full_ext_wall(fix pt_list[4][2])
{
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[0],-pt_list[0][1]); _fr_do_cspace(_fdt_tmppts[0],dlC);
   g3_replace_add_delta_y(*_fdt_lcore,_fdt_tmppts[3],-pt_list[3][1]); _fr_do_cspace(_fdt_tmppts[3],dlF);
   _fdt_pbase=_fdt_rbase;
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[1],-pt_list[1][1]); _fr_do_cspace(_fdt_tmppts[1],dlC);
   g3_replace_add_delta_y(*_fdt_rcore,_fdt_tmppts[2],-pt_list[2][1]); _fr_do_cspace(_fdt_tmppts[2],dlF);
   _fr_ndbg(NO_REND,g3_draw_cpoly(4,_fdt_tmppts));
   _fr_sdbg(STATS,_frp.stats.ext_wall++);
}

// tabs_flr

// should store pt_code, and look up in there....
#define floor_uv(pt,pn)   {(pt)->uv.u=pt_uv[pn][pt_rot][0]; (pt)->uv.v=pt_uv[pn][pt_rot][1]; (pt)->p3_flags|=PF_U|PF_V;}
#define ceil_uv(pt,pn)    {(pt)->uv.u=pt_uv[pn][pt_rot][0]; (pt)->uv.v=pt_uv[pn][pt_rot][1]; (pt)->p3_flags|=PF_U|PF_V;}

#define floor_uv_i(pt,pn) {(pt)->uv.u=pt_uv[pn][pt_rot][0]; (pt)->uv.v=pt_uv[pn][pt_rot][1]; (pt)->p3_flags|=PF_U|PF_V|PF_I;}
#define ceil_uv_i(pt,pn)  {(pt)->uv.u=pt_uv[pn][pt_rot][0]; (pt)->uv.v=pt_uv[pn][pt_rot][1]; (pt)->p3_flags|=PF_U|PF_V|PF_I;}

static void _fr_null_flrceil(void) {}

// lets start a war, jack up the dow jones
static void _fr_flat_flr(void)
{
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;
   int i, pt_code, loopcnt;
   g3s_phandle *pb;

   loopcnt=_fdt_ttf->flags>>FRFLRSHF_2ELEM;  // 0 or 1
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_FLR];
   do {
      for (pb=&_fdt_tmppts[0],i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb++,pt_code);
      }
//�� bug??      _fr_ndbg(NO_REND,g3_draw_poly((*fr_get_idx)(),_fdt_ttf->ptsper,&_fdt_tmppts));
      _fr_ndbg(NO_REND,g3_draw_poly((*fr_get_idx)(),_fdt_ttf->ptsper,_fdt_tmppts));
   } while (loopcnt-->0);
   _fr_sdbg(STATS,_frp.stats.flr++);
}

#ifdef FLAT_SUPPORT
static void _fr_flat_lit_flr(void)
{
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;
   int i, pt_code, loopcnt;
   g3s_phandle *pb;

   loopcnt=_fdt_ttf->flags>>FRFLRSHF_2ELEM;  // 0 or 1
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_FLR];
   gr_set_fcolor((*fr_get_idx)());
   do {
      for (pb=&_fdt_tmppts[0],i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb,pt_code); _fr_pt_light(*pb); (*pb++)->p3_flags|=PF_I;
      }
      _fr_ndbg(NO_REND,g3_draw_spoly(_fdt_ttf->ptsper,&_fdt_tmppts));
   } while (loopcnt-->0);
   _fr_sdbg(STATS,_frp.stats.flr++);
}
#else
#define _fr_flat_lit_flr _fr_tmap_lit_flr
#endif

static void _fr_tmap_flr(void)
{
   uchar nrm_mask=fr_fnorm_list[_fdt_tt];
   int i, pt_code, loopcnt;
   g3s_phandle *pb;
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;
   int pt_rot=me_rotflr(_fdt_mptr);

   loopcnt=_fdt_ttf->flags>>FRFLRSHF_2ELEM;  // 0 or 1
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_FLR];
   do {
      for (pb=&_fdt_tmppts[0],i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb,pt_code); pt_code&=FRPTSPTOFF; floor_uv(*pb,pt_code); pb++;
      }
      if (quik_draw_tmap_p(_fdt_ttf->ptsper))
      {   
	      if ((nrm_mask&0xf)==FRFNORM_VFULL)
		    { _fr_ndbg(NO_REND,_fr_floor_func(_fdt_ttf->ptsper,_fdt_tmppts,(*fr_get_tmap)())); }
	      else
	       { _fr_ndbg(NO_REND,_fr_per_func(_fdt_ttf->ptsper,_fdt_tmppts,(*fr_get_tmap)())); }
      }
   } while (loopcnt-->0);
   _fr_sdbg(STATS,_frp.stats.flr++);
}

static void _fr_tmap_lit_flr(void)
{
   uchar nrm_mask=fr_fnorm_list[_fdt_tt];
   int i, pt_code, loopcnt;
   g3s_phandle *pb;
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;
   int pt_rot=me_rotflr(_fdt_mptr);

   loopcnt=_fdt_ttf->flags>>FRFLRSHF_2ELEM;  // 0 or 1
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_FLR];
   do {
      for (pb=&_fdt_tmppts[0],i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb,pt_code); pt_code&=FRPTSPTOFF; floor_uv_i(*pb,pt_code); _fr_pt_light(*pb++);
      }
      if (quik_draw_tmap_p(_fdt_ttf->ptsper))
      {
	      if ((nrm_mask&0xf)==FRFNORM_VFULL)
          { _fr_ndbg(NO_REND,_fr_lit_floor_func(_fdt_ttf->ptsper,_fdt_tmppts,(*fr_get_tmap)())); }
	      else
	       { _fr_ndbg(NO_REND,_fr_lit_per_func(_fdt_ttf->ptsper,_fdt_tmppts,(*fr_get_tmap)())); }
      }
   } while (loopcnt-->0);
   _fr_sdbg(STATS,_frp.stats.flr++);
}

static void _fr_cspace_wire_flr(void)
{
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;
   int i, pt_code, loopcnt;
   g3s_phandle *pb;

   loopcnt=_fdt_ttf->flags>>FRFLRSHF_2ELEM;  // 0 or 1
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_FLR];
   gr_set_fcolor((*fr_get_idx)());
   do {
      for (pb=&_fdt_tmppts[0],i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb,pt_code); _fr_pt_cspace(*pb); pb++;
      }
      _fr_ndbg(NO_REND,_fr_draw_wire_cpoly_3or4(_fdt_ttf->ptsper));
   } while (loopcnt-->0);
   _fr_sdbg(STATS,_frp.stats.flr++);
}

static void _fr_cspace_full_flr(void)
{
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;
   int i, pt_code, loopcnt;
   g3s_phandle *pb;

   loopcnt=_fdt_ttf->flags>>FRFLRSHF_2ELEM;  // 0 or 1
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_FLR];
   gr_set_fcolor((*fr_get_idx)());
   do {
      for (pb=&_fdt_tmppts[0],i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb,pt_code); _fr_pt_cspace(*pb); pb++;
      }
      _fr_ndbg(NO_REND,g3_draw_cpoly(_fdt_ttf->ptsper,_fdt_tmppts));
   } while (loopcnt-->0);
   _fr_sdbg(STATS,_frp.stats.flr++);
}

// tabs_ceil
static void _fr_flat_ceil(void)
{
   int i, pt_code, loopcnt=1;
   g3s_phandle *pb;
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;

   if (_fdt_ttf->flags&FRFLRFLG_NOTOP) return;
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_CEIL];
   do {
      for (pb=(&_fdt_tmppts[0])+_fdt_ttf->ptsper-1,i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb--,pt_code);
      }
      _fr_ndbg(NO_REND,g3_draw_poly((*fr_get_idx)(),_fdt_ttf->ptsper,_fdt_tmppts));
   } while ((_fdt_ttf->flags&FRFLRFLG_2ELEM)&&(loopcnt-->0));  
   _fr_sdbg(STATS,_frp.stats.ceil++);
}

#ifdef FLAT_SUPPORT
static void _fr_flat_lit_ceil(void)
{
   int i, pt_code, loopcnt=1;
   g3s_phandle *pb;
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;

   if (_fdt_ttf->flags&FRFLRFLG_NOTOP) return;
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_CEIL];
   do {
      for (pb=(&_fdt_tmppts[0])+_fdt_ttf->ptsper-1,i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb,pt_code); _fr_pt_light(*pb); (*pb--)->p3_flags|=PF_I;
      }
      gr_set_fcolor((*fr_get_idx)());
      _fr_ndbg(NO_REND,g3_draw_spoly(_fdt_ttf->ptsper,_fdt_tmppts));
   } while ((_fdt_ttf->flags&FRFLRFLG_2ELEM)&&(loopcnt-->0));  
   _fr_sdbg(STATS,_frp.stats.ceil++);
}
#else
#define _fr_flat_lit_ceil _fr_tmap_lit_ceil
#endif

static void _fr_tmap_ceil(void)
{
   uchar nrm_mask=fr_fnorm_list[_fdt_tt];
   int i, pt_code, loopcnt=1;
   g3s_phandle *pb;
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;
   int pt_rot=me_rotceil(_fdt_mptr);

   if (_fdt_ttf->flags&FRFLRFLG_NOTOP) return;
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_CEIL];
   do {
      for (pb=(&_fdt_tmppts[0])+_fdt_ttf->ptsper-1,i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb,pt_code); pt_code&=FRPTSPTOFF; ceil_uv(*pb,pt_code); pb--;
      }
      if (quik_draw_tmap_p(_fdt_ttf->ptsper))
      {
	      if ((nrm_mask&0xf)==FRFNORM_VFULL)
          { _fr_ndbg(NO_REND,_fr_floor_func(_fdt_ttf->ptsper,_fdt_tmppts,(*fr_get_tmap)())); }
	      else
	       { _fr_ndbg(NO_REND,_fr_per_func(_fdt_ttf->ptsper,_fdt_tmppts,(*fr_get_tmap)())); }
      }
   } while ((_fdt_ttf->flags&FRFLRFLG_2ELEM)&&(loopcnt-->0));  
   _fr_sdbg(STATS,_frp.stats.ceil++);
}

static void _fr_tmap_lit_ceil(void)
{
   uchar nrm_mask=fr_fnorm_list[_fdt_tt];
   int i, pt_code, loopcnt=1;
   g3s_phandle *pb;
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;
   int pt_rot=me_rotceil(_fdt_mptr);

   if (_fdt_ttf->flags&FRFLRFLG_NOTOP) return;
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_CEIL];
   do {
      for (pb=(&_fdt_tmppts[0])+_fdt_ttf->ptsper-1,i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb,pt_code); pt_code&=FRPTSPTOFF; ceil_uv_i(*pb,pt_code); _fr_pt_light(*pb--);
      }
      if (quik_draw_tmap_p(_fdt_ttf->ptsper))
      {
         if ((nrm_mask&0xf)==FRFNORM_VFULL)
	       { _fr_ndbg(NO_REND,_fr_lit_floor_func(_fdt_ttf->ptsper,_fdt_tmppts,(*fr_get_tmap)())); }
	      else
		    { _fr_ndbg(NO_REND,_fr_lit_per_func(_fdt_ttf->ptsper,_fdt_tmppts,(*fr_get_tmap)())); }
      }
    } while ((_fdt_ttf->flags&FRFLRFLG_2ELEM)&&(loopcnt-->0));  
   _fr_sdbg(STATS,_frp.stats.ceil++);
}

//#pragma disable_message(202)
static void _fr_cspace_wire_ceil(void)
{
   int i, pt_code, loopcnt=1;
   g3s_phandle *pb;
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;

   if (_fdt_ttf->flags&FRFLRFLG_NOTOP) return;
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_CEIL];
   do {
      for (pb=(&_fdt_tmppts[0])+_fdt_ttf->ptsper-1,i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb,pt_code); _fr_pt_cspace(*pb); pb--;
      }
      _fr_ndbg(NO_REND,_fr_draw_wire_cpoly_3or4(_fdt_ttf->ptsper));
   } while ((_fdt_ttf->flags&FRFLRFLG_2ELEM)&&(loopcnt-->0));  
   _fr_sdbg(STATS,_frp.stats.ceil++);
}

static void _fr_cspace_full_ceil(void)
{
   int i, pt_code, loopcnt=1;
   g3s_phandle *pb;
   uchar *ptdat=_fdt_ttf->data, *pt_merge_mask;

   if (_fdt_ttf->flags&FRFLRFLG_NOTOP) return;
   pt_merge_mask=merge_masks[me_bits_mirror(_fdt_mptr)][FDT_LK_CEIL];
   do {
      for (pb=(&_fdt_tmppts[0])+_fdt_ttf->ptsper-1,i=0; i<_fdt_ttf->ptsper; i++)
      {
         pt_code=*ptdat++; pt_code=(pt_code^pt_merge_mask[0])&pt_merge_mask[1];
         _fr_figure_pt(*pb,pt_code); _fr_pt_cspace(*pb); pb--;
      }
      _fr_ndbg(NO_REND,g3_draw_cpoly(_fdt_ttf->ptsper,_fdt_tmppts));
   } while ((_fdt_ttf->flags&FRFLRFLG_2ELEM)&&(loopcnt-->0));  
   _fr_sdbg(STATS,_frp.stats.ceil++);
}
//#pragma enable_message(202)


// reoptimize these to _NOT_ do all the boneheaded stuff

// i ask for nothing, for myself
// for i am dead, for i am dead
int fr_get_ext_gap(fix *face_l, uchar in_fo, uchar *hgts, uchar *mmptr)
{
   int fo_t, fo_b;
   if (in_fo==0xff) return 0;
//   if (in_fo&FO_HT_MSK)
   fo_t=(in_fo^mmptr[2])&mmptr[3]; fo_b=(in_fo^mmptr[0])&mmptr[1];
//   if (_fdt_ttf->flags&FRFLRFLG_USEPR)
   if ((fo_t|fo_b)&FO_HT_MSK)
   {
//	   uchar cur_fo;
//    cur_fo=(in_fo^mmptr[2])&mmptr[3];
	   *(face_l+0)=fo_unpack[fo_t&FO_LR_MSK][T_LEFT];     // set x for left ceil
	   *(face_l+2)=fo_unpack[fo_t&FO_LR_MSK][T_RIGHT];    // set x for right ceil
	   *(face_l+1)=*(face_l+3)=_fr_fhgt_list[hgts[FDT_PT_CEIL]];  // set y's
      if (fo_t&FO_L_PARM) *(face_l+1)-=_fr_fhgt_list[hgts[FDT_PT_PARM]];
      if (fo_t&FO_R_PARM) *(face_l+3)-=_fr_fhgt_list[hgts[FDT_PT_PARM]];
//      cur_fo=(in_fo^mmptr[0])&mmptr[1];
	   *(face_l+4)=fo_unpack[fo_b&FO_LR_MSK][T_RIGHT];     // set x for right flr
	   *(face_l+6)=fo_unpack[fo_b&FO_LR_MSK][T_LEFT];     // set x for left flr
	   *(face_l+5)=*(face_l+7)=_fr_fhgt_list[hgts[FDT_PT_FLR]];   // set y's
      if (fo_b&FO_R_PARM) *(face_l+5)+=_fr_fhgt_list[hgts[FDT_PT_PARM]];
      if (fo_b&FO_L_PARM) *(face_l+7)+=_fr_fhgt_list[hgts[FDT_PT_PARM]];
      if ((*(face_l+3)==*(face_l+5))&&(*(face_l+1)==*(face_l+7))) return 0;
   }
   else
   {
	   *(face_l+0)=fo_unpack[in_fo&FO_LR_MSK][T_LEFT];     // set x for left ceil
	   *(face_l+2)=fo_unpack[in_fo&FO_LR_MSK][T_RIGHT];    // set x for right ceil
	   *(face_l+1)=*(face_l+3)=_fr_fhgt_list[hgts[FDT_PT_CEIL]];  // set y's
	   *(face_l+4)=fo_unpack[in_fo&FO_LR_MSK][T_RIGHT];     // set x for right flr
	   *(face_l+6)=fo_unpack[in_fo&FO_LR_MSK][T_LEFT];     // set x for left flr
	   *(face_l+5)=*(face_l+7)=_fr_fhgt_list[hgts[FDT_PT_FLR]];   // set y's
      if (*(face_l+1)==*(face_l+5)) return 0;
   }
   return 1;
}

#define ANTI_MASK_COUNT    0x0fff
#define ANTI_MASK_PUNTTOP  0x1000
#define ANTI_MASK_PUNTBOT  0x2000
// or in 1, since there is still the top face, though it is to be punted
#define ANTI_RET_PUNTALL  (ANTI_MASK_PUNTTOP|ANTI_MASK_PUNTBOT|1)

// connect the god damn dots
// connect the god damn dots
// connect the god damn dots
// who am i trying to impress
// who could care less
int _fr_get_anti_gap(fix *face_l, uchar in_fo, uchar *hgts, uchar *mmptr)
{
   int fo_t, fo_b;
   int facecnt=1;
// more importantly, we think face_l is starting 4 in, we fill normally which works
// also note that if no height changes, it could mean we have half walls
// so we check in that case, and if so, add it in place
   _fr_sdbg(ANAL_CHK,if (in_fo==0xff) mprintf("_fr_get_anti_gap: hey in_fo is 0xff\n"));
   fo_t=(in_fo^mmptr[2])&mmptr[3]; fo_b=(in_fo^mmptr[0])&mmptr[1];
   if ((fo_t|fo_b)&FO_HT_MSK)
   {                                                             // these cant be at top, since they have parameter
//	   uchar cur_fo;                                              // unless of course it masks out.... hmmm
//      cur_fo=(in_fo^mmptr[2])&mmptr[3];
      if (((fo_t&FO_HT_MSK)==0)&&(hgts[FDT_PT_CEIL]==MAX_HGT)) // we are fully at top
         facecnt|=ANTI_MASK_PUNTTOP;
      else
      {
		   *(face_l+5)=*(face_l+7)=_fr_fhgt_list[hgts[FDT_PT_CEIL]]; // set y's
	      if (fo_t&FO_L_PARM) *(face_l+5)-=_fr_fhgt_list[hgts[FDT_PT_PARM]];
	      if (fo_t&FO_R_PARM) *(face_l+7)-=_fr_fhgt_list[hgts[FDT_PT_PARM]];
      }
      face_l+=8;
//      cur_fo=(in_fo^mmptr[0])&mmptr[1];
      if (((fo_b&FO_HT_MSK)==0)&&(hgts[FDT_PT_FLR]==0)) // we are fully at bottom
         facecnt|=ANTI_MASK_PUNTBOT;
      else
      {
		   *(face_l+1)=*(face_l+3)=_fr_fhgt_list[hgts[FDT_PT_FLR]];  // set y's
	      if (fo_b&FO_R_PARM) *(face_l+1)+=_fr_fhgt_list[hgts[FDT_PT_PARM]];
	      if (fo_b&FO_L_PARM) *(face_l+3)+=_fr_fhgt_list[hgts[FDT_PT_PARM]];
      }
   }
   else
   {
      if (hgts[FDT_PT_CEIL]==MAX_HGT)                           // already at top
         facecnt|=ANTI_MASK_PUNTTOP;
      else        // correct x's for this have been set already in _fr_init_slopes in frpipe
		   *(face_l+5)=*(face_l+7)=_fr_fhgt_list[hgts[FDT_PT_CEIL]];    // set y's  
      face_l+=8;  // skip to 2nd entry, as we have finshed or punted the first
      if ((in_fo&FO_LR_MSK)!=0x06)   // are we not fully free, tm? (ie. foClr)
      {  // add the anti of the current here...  
         facecnt++;
// im sure this is wrong
		   *(face_l+0)=fo_unpack[in_fo&FO_LR_MSK][T_LEFT];    // flip since this is real order
		   *(face_l+2)=fo_unpack[in_fo&FO_LR_MSK][T_RIGHT];    
		   *(face_l+1)=*(face_l+3)=_fr_fhgt_list[hgts[FDT_PT_CEIL]];  // set y's
		   *(face_l+4)=fo_unpack[in_fo&FO_LR_MSK][T_RIGHT];     // har har, same flip
		   *(face_l+6)=fo_unpack[in_fo&FO_LR_MSK][T_LEFT];   
		   *(face_l+7)=*(face_l+5)=_fr_fhgt_list[hgts[FDT_PT_FLR]];   // set y's
         face_l+=8;        // skip to 3rd, as we have just done middle
      }
      if (hgts[FDT_PT_FLR]==0)
         facecnt|=ANTI_MASK_PUNTBOT;
      else
		   *(face_l+1)=*(face_l+3)=_fr_fhgt_list[hgts[FDT_PT_FLR]];   // set y's
   }
   return facecnt;
}

// amazes me the will of instinct

// keep these in place so the compiler can deal
// these are of wall_id vs. point_id vs. x and y (as in u,v)
static fix inner_wall[2][4][2], final_wall[4][4][2];
       fix outer_wall[3][4][2];      // base is set up in frpipe.c
static fix *use_outer_wall;          // pointer to the outer wall area to really use

fix tf_diag_walls[4][2]=
  {{0,fix_make(4,0)},{fix_make(1,0),fix_make(4,0)},{fix_make(1,0),0},{0,0}};
static fix diag_norms[3]=
  {0,0,0};


#ifdef HACK_SHOW
void hack_show(fix edward[4][2])
{
   mprintf(" %8.8x %8.8x   %8.8x %8.8x\n",edward[0][0],edward[0][1],edward[1][0],edward[1][1]);
   mprintf(" %8.8x %8.8x   %8.8x %8.8x\n",edward[3][0],edward[3][1],edward[2][0],edward[2][1]);
}
#endif

// renders icnt of final_walls
void _render_3d_walls(int which, int cnt)
{
   WallsToPts *wpt=&wall_pts[FROUTERWALLS+which];
   _fdt_wallid=which;
   flip_setup(which);

   switch (_fdt_pbase=pt_deref[(wpt->ul&FRPTSPTOFF)].base)
   {
   case 0: _fdt_lcore=_fr_ptbase; break;
   case 1: _fdt_lcore=_fr_ptnext; break;
   case 2: _fdt_lcore=_fr_ptnext+1; break;
   case 3: _fdt_lcore=_fr_ptbase+1; break;
   }
   switch (_fdt_rbase=pt_deref[(wpt->ur&FRPTSPTOFF)].base)
   {
   case 0: _fdt_rcore=_fr_ptbase; break;
   case 1: _fdt_rcore=_fr_ptnext; break;
   case 2: _fdt_rcore=_fr_ptnext+1; break;
   case 3: _fdt_rcore=_fr_ptbase+1; break;
   }
   _fdt_lcore+=_fdt_x; _fdt_rcore+=_fdt_x;
   switch (cnt)
   {
   case 4: _fr_terr_ext_wall(final_wall[3]);
   case 3: _fr_terr_ext_wall(final_wall[2]);
   case 2: _fr_terr_ext_wall(final_wall[1]);
   case 1: _fr_terr_ext_wall(final_wall[0]);
   case 0: break;
   }
}

// currently always assumes full width... ie. is really broken

#define sgn(x,y)                 (x>y?1:(x==y?0:-1))

#define io_wall_cmp(iwp,cmp,owp) ( (*(i_wall+(iwp<<1)+1)) cmp (*(o_wall+(owp<<1)+1)) )
#define io_wall_sgn(iwp,owp)     ( sgn((*(i_wall+(iwp<<1)+1)),(*(o_wall+(owp<<1)+1))) )
#define dst_from_i(dwp,iwp)      { (*(dst_wall+(dwp<<1)+1))=(*(i_wall+(iwp<<1)+1)); (*(dst_wall+(dwp<<1)))=(*(i_wall+(iwp<<1))); }
#define dst_from_o(dwp,owp)      { (*(dst_wall+(dwp<<1)+1))=(*(o_wall+(owp<<1)+1)); (*(dst_wall+(dwp<<1)))=(*(o_wall+(owp<<1))); }

int f_is_i;

/*
 * currently broken when the following is true:             40000 40000
 *     /| as the outside gap, and |  |  inside         ie.  10000 20000          20000 20000
 *    / |                         |  |              o_wall  10000 10000  i_wall  10000 10000
 *                                                          00000 00000
 * the problem is 40 40 > 10 20, and 10 <= 10 but 20 ! <= 10 so it thinks bottom crossing failure
 */
// a hack is now in to fix this, should be rewritten in assembler as well
int merge_walls(fix *dst_wall,fix *i_wall,int i_cnt,fix *o_wall,int o_cnt)
{
   int cur_i=0, cur_o=0, f_cnt=0, tmp1, tmp2;
   int sgn0,sgn1,sgn2,sgn3;

   f_is_i=TRUE;
   while (cur_i<i_cnt)
   {
      while (io_wall_cmp(0,<=,3)&&io_wall_cmp(1,<=,2))   // skip to next outer wall, this one is above our gap
       { o_wall+=8; if (++cur_o>=o_cnt) return f_cnt; }  
      if (io_wall_cmp(3,>=,0)&&io_wall_cmp(2,>=,1))      // if gap bottom above current outer wall, skip to next gap
       { i_wall+=8; f_is_i=FALSE; if (++cur_i>=i_cnt) return f_cnt; else continue; }
      f_cnt++;
      if ((sgn0=io_wall_sgn(0,0))!=(sgn1=io_wall_sgn(1,1)))   // tmp is gap top below outer wall top
      {
         if      (sgn0==0) tmp1=(sgn1<=0);
         else if (sgn1==0) tmp1=(sgn0<=0);
         else
         {
//            Warning(("top crossing case\n")); // crossing case sucks rocks
            tmp1=0;
         }
      }
      else tmp1=(sgn0<=0);
      if (tmp1)
       { dst_from_i(0,0); dst_from_i(1,1); }
      else
       { dst_from_o(0,0); dst_from_o(1,1); }
      if ((sgn3=io_wall_sgn(3,3))!=(sgn2=io_wall_sgn(2,2)))   // tmp is gap bottom above outer wall bottom
      {
         if      (sgn2==0) tmp2=(sgn3>=0);
         else if (sgn3==0) tmp2=(sgn2>=0);
         else
         {
//		      Warning(("bottom crossing case\n"));
            tmp2=0;
         }
      }
      else tmp2=(sgn2>=0);
      f_is_i=(f_is_i&&tmp1&&tmp2);
      if (tmp2)
       { dst_from_i(2,2); dst_from_i(3,3); i_wall+=8; if (++cur_i>=i_cnt) break; }
      else
       { dst_from_o(2,2); dst_from_o(3,3); o_wall+=8; if (++cur_o>=o_cnt) break; }
      dst_wall+=8;
   }
   return f_cnt;
}

uchar solid_chk[4]={FMK_INT_NW,FMK_INT_EW,FMK_INT_SW,FMK_INT_WW};
uchar clear_chk[4]={FMK_NW,FMK_EW,FMK_SW,FMK_WW};

// local statics dont actually work, since i define static null in debug, so this is here
static int last_ocnt=-1;

// look around at all my playthings
// eighteen percent a year, for nothing
static void _fr_parse_wall(int which)
{
   int icnt, fcnt, ocnt, useocnt, are_solid;
   MapElem *oth_mptr;
   uchar oth_hgts[3], oth_fo;

   oth_mptr=_fdt_mptr+wall_adds[which];

   if (_fdt_me_flags&MAP_FRIEND_FULL_MASK)
      _game_fr_tmap = me_tmap_wall(oth_mptr);
   else
	   _game_fr_tmap = _fdt_wmap;

   if (me_clearsolid(_fdt_mptr)&solid_chk[which])
   {
      _fr_sdbg(STATS,_frp.stats.hitsolid++);
      are_solid=2;
   }
   else
   {
	   oth_fo = face_obstruct[me_tiletype(oth_mptr)][(which+2)&3];
      are_solid=(oth_fo==0xff);
   }

   if (are_solid)  // totally solid on other side, just do us, baby baby
   {
      _fr_sdbg(STATS,if ((me_clearsolid(_fdt_mptr)&solid_chk[which])==0) _frp.stats.setsolid++);
      _me_clearsolid(_fdt_mptr)|=solid_chk[which];
// int walls doesnt know about parameters, so we lost
//      if (_fdt_ttf->flags&FRFLRFLG_USEPR)
//      {
         fcnt=fr_get_ext_gap(&final_wall[0][0][0], _fdt_fo[which], _fdt_hgts, (uchar *)mmask_facelet[me_bits_mirror(_fdt_mptr)]);
         if (fcnt==0)
          { _me_clearsolid(_fdt_mptr)|=clear_chk[which]; _fr_sdbg(STATS, _frp.stats.setclear++); }
         else
	         (*_fr_render_walls)(which,fcnt);
//      }
//      else
//	      _fr_terr_int_wall(FROUTERWALLS+which);
      return;
   }
   _fr_sdbg(NO_RTF,return);
   // otherwise, grind away
   oth_hgts[FDT_PT_FLR]  = me_height_flr(oth_mptr);
   oth_hgts[FDT_PT_CEIL] = MAX_HGT-me_height_ceil(oth_mptr);
   oth_hgts[FDT_PT_PARM] = me_param(oth_mptr);
   ocnt=_fr_get_anti_gap(&outer_wall[0][0][0], oth_fo, oth_hgts, (uchar *)mmask_facelet[me_bits_mirror(oth_mptr)]);
   if (ocnt!=ANTI_RET_PUNTALL)
   {   
	   useocnt=(ocnt&ANTI_MASK_COUNT);
	   if ((ocnt&ANTI_MASK_PUNTBOT)==0)
	   {  // touch up the bottom edge
	      if (useocnt!=last_ocnt)
	      {  // we want to store the ocnt we actually filled, not ended with
	         last_ocnt=useocnt; outer_wall[useocnt][1][0]=outer_wall[useocnt][2][0]=fix_make(1,0);
			   outer_wall[useocnt][0][0]=outer_wall[useocnt][2][1]=outer_wall[useocnt][3][0]=outer_wall[useocnt][3][1]=0;
	      }
	      useocnt++;
	   }
	   if ((ocnt&ANTI_MASK_PUNTTOP)==0) use_outer_wall=&outer_wall[0][0][0];
	   else { use_outer_wall=&outer_wall[1][0][0]; useocnt--; }
	   _fr_sdbg(SANITY,if (useocnt==0) mprintf("_fr_parse_wall: have 0 usecnt, but didnt punt yet\n"));
	   _fr_sdbg(SANITY,if (_fdt_fo[which]==0xff) mprint("_fr_parse_wall: local fo==0xff, wallbits should have been 0"));
	   icnt=fr_get_ext_gap(&inner_wall[0][0][0], _fdt_fo[which], _fdt_hgts, (uchar *)mmask_facelet[me_bits_mirror(_fdt_mptr)]);

      fcnt=merge_walls(&final_wall[0][0][0],&inner_wall[0][0][0],icnt,use_outer_wall,useocnt);
      if ((f_is_i)&&(fcnt==icnt))
       { _fr_sdbg(STATS,_frp.stats.fisisolid++); _me_clearsolid(_fdt_mptr)|=solid_chk[which]; }
		(*_fr_render_walls)(which,fcnt);

#ifdef HACK_SHOW
		{
	      int i;
	      mprintf("%d outer (%d) are:\n",useocnt,me_tiletype(oth_mptr)); for (i=0; i<useocnt; i++) hack_show(use_outer_wall+(i*8));
	      mprintf("%d inner (%d) are:\n",icnt,me_tiletype(_fdt_mptr)); for (i=0; i<icnt; i++) hack_show(inner_wall[i]);
	      mprintf("%d final walls be:\n",fcnt); for (i=0; i<fcnt; i++) hack_show(final_wall[i]);
		}
#endif

   }
   else fcnt=0;
   if (fcnt==0)
   {
      _me_clearsolid(_fdt_mptr)|=clear_chk[which];  // or just shift here? who can tell
      _fr_sdbg(STATS,_frp.stats.setclear++);
   }
}

// take the mptr, decode it's clip vectors, and set initial wall masks for the tile
void parse_clip_tile()
{
   
}

// so, ahh, should have wall bits for floor and ceiling, so we can start clipping

#if _fr_defdbg(STATS)                         
#define wall_check(wmask1,wmask2,w_id) \
	if ((_fdt_mask&wmask1)&&(_fdt_ttw.wallbits&wmask2)) \
      if ((me_clearsolid(_fdt_mptr)&wmask1)!=0) \
        { _fr_sdbg(STATS,_frp.stats.hitclear++); } \
      else if ((me_subclip(_fdt_mptr)&wmask1)!=0) \
        { _fr_sdbg(STATS,_frp.stats.hitsc++); } \
      else \
         _fr_parse_wall(w_id)
#else       // do wacky shit to make this work
#define wall_check(wmask1,wmask2,w_id) \
   if ((_fdt_mask&wmask1)&&(_fdt_ttw.wallbits&wmask2)&& \
      ((me_clearsolid(_fdt_mptr)&wmask1)==0)&&((me_subclip(_fdt_mptr)&wmask1)==0)) _fr_parse_wall(w_id)
#endif

#include "tilename.h"

// if you're so special, why aren't you dead
// implicit parameters are _fdt_x,_fdt_y,_fdt_mptr,_fdt_mask
void fr_draw_tile(void)
{
   if ((_fdt_tt=me_tiletype(_fdt_mptr))==TILE_SOLID)    // really, clip should deal
   {
      me_subclip_set(_fdt_mptr,SUBCLIP_OUT_OF_CONE);    // sure, deal with it...
      return;                                           // so make this a warn later
   }
   if (me_subclip(_fdt_mptr)==SUBCLIP_OUT_OF_CONE)
   {
      return;                                           
   }

   _fr_ndbg(NO_SUB_CLIP,parse_clip_tile());
   _fr_sdbg(SHOW_BASE, if (_fr_terr_prim) _fr_terr_base(0); else _fr_terr_dumb_base(0));
   _fdt_ttw = tile_walls[_fdt_tt];
   _fdt_ttf =&tile_floors[_fdt_tt];
   _fdt_hgts[FDT_PT_FLR]  = me_height_flr(_fdt_mptr);
   _fdt_hgts[FDT_PT_CEIL] = MAX_HGT-me_height_ceil(_fdt_mptr);
   _fdt_hgts[FDT_PT_PARM] = me_param(_fdt_mptr);
   _fdt_hgts[FDT_PT_FPRM] = me_height_flr(_fdt_mptr)+me_param(_fdt_mptr);
   _fdt_hgts[FDT_PT_CPRM] = MAX_HGT-me_height_ceil(_fdt_mptr)-me_param(_fdt_mptr);
   _fdt_cur_parm=_fdt_fix_parm = _fr_fhgt_list[_fdt_hgts[FDT_PT_PARM]];
   _fdt_fo                = face_obstruct[_fdt_tt];
   _fdt_wmap              = me_tmap_wall(_fdt_mptr);
   _fdt_me_flags          = me_flags(_fdt_mptr);
   _fdt_slock             = _fr_fhgt_list[_fdt_me_flags&MAP_VLOCK_MASK]>>8;

   if (_frp.faces.cyber&&_fdt_terr)
	{
      int tmp=me_bits_flip(_fdt_mptr);
	   void fr_terr_cspace_pick(uchar do_w);

      if (tmp!=last_csp_fr)
         fr_terr_cspace_pick(last_csp_fr=tmp);
   }

// how about external walls, eh?   
   wall_check(FMK_NW,FRWALLNORTH,0);
   wall_check(FMK_EW,FRWALLEAST ,1);
   wall_check(FMK_SW,FRWALLSOUTH,2);
   wall_check(FMK_WW,FRWALLWEST ,3);

   _game_fr_tmap = _fdt_wmap;
   _fdt_icnt=(_fdt_ttw.wallbits&FRWALLINT);
// set real flip for internal faces now
   if (_fdt_icnt)
   {
	   flip_setup(4); // 4 is secret internal face code
	   switch (_fdt_icnt)
	   {
#ifdef USE_OCT
	   case 6: _fdt_cur_parm=FROCTNUM;  // recompute wacky prm stuff
				   _fdt_hgts[FDT_PT_FPRM]=me_height_flr(_fdt_mptr)+me_param(_fdt_mptr);
	            // this cant be done yet
				   // _fdt_hgts[FDT_PT_CPRM]=MAX_HGT-me_height_ceil(_fdt_mptr)-me_param(_fdt_mptr);
			   	// _fdt_hgts[FDT_PT_PARM]=me_param(_fdt_mptr);
	           _fr_terr_int_wall(_fdt_ttw.wallbase+5);        // or hmm.. a temp
	   case 5: _fr_terr_int_wall(_fdt_ttw.wallbase+4);        //  variable and then
	   case 4: _fr_terr_int_wall(_fdt_ttw.wallbase+3);        //  increment it in
	   case 3: _fr_terr_int_wall(_fdt_ttw.wallbase+2);        //  each case??
#endif
	   case 2: _fr_terr_int_wall(_fdt_ttw.wallbase+1);
	   case 1: _fr_terr_int_wall(_fdt_ttw.wallbase+0);
	   case 0: break;
	   }
   }
#ifdef USE_OCT  // in case it was overloaded by FROCTNUM, not needed if no oct tiles
   _fdt_cur_parm=_fdt_fix_parm;
#endif
   _game_fr_tmap=me_tmap_flr(_fdt_mptr);
   _fr_terr_flr();
   _game_fr_tmap=me_tmap_ceil(_fdt_mptr);
   _fr_terr_ceil();
   // now check object cache

   _fr_parse_obj();

#ifdef CLEAR_AS_WE_GO
   me_subclip_set(_fdt_mptr,SUBCLIP_OUT_OF_CONE);  // sure, deal with it...
#endif
//   mprintf("-");
}

// when all the leaves have fallen and turned to dust
// will we remain entrenched within our ways
// indifference, the plague that moves throughout this land

#define FRT_PTS_POLY 1
#define FRT_PTS_TMAP 2
#define FRT_PTS_LITE 4
#define FRT_PTS_CYB  8

#define FRT_FLAT     0
#define FRT_LIT      1
#define FRT_TMAP     2
#define FRT_CSPACE   4
#define FRT_NULL     6
#define FRT_MAX_F    7

static void (*_fr_tabs_int_wall[FRT_MAX_F])(int wall_id)=
{_fr_flat_int_wall,_fr_flat_lit_int_wall,_fr_tmap_int_wall,_fr_tmap_lit_int_wall,_fr_cspace_wire_int_wall,_fr_cspace_full_int_wall,_fr_null_int_wall};
static void (*_fr_tabs_ext_wall[FRT_MAX_F])(fix pt_list[4][2])=
{_fr_flat_ext_wall,_fr_flat_lit_ext_wall,_fr_tmap_ext_wall,_fr_tmap_lit_ext_wall,_fr_cspace_wire_ext_wall,_fr_cspace_full_ext_wall,_fr_null_ext_wall};
static void (*_fr_tabs_flr[FRT_MAX_F])(void)=
{_fr_flat_flr,_fr_flat_lit_flr,_fr_tmap_flr,_fr_tmap_lit_flr,_fr_cspace_wire_flr,_fr_cspace_full_flr,_fr_null_flrceil};
static void (*_fr_tabs_ceil[FRT_MAX_F])(void)=
{_fr_flat_ceil,_fr_flat_lit_ceil,_fr_tmap_ceil,_fr_tmap_lit_ceil,_fr_cspace_wire_ceil,_fr_cspace_full_ceil,_fr_null_flrceil};

#define CSPACE_WALLS

void fr_terr_cspace_pick(uchar do_wall)
{
   int to_do, wall_do;

   if (do_wall)
   {
      wall_do=to_do=FRT_CSPACE+1;  // FRT_FLAT
      if (do_wall>=2)
         if (_fr_curflags&FR_PICKUPM_MASK)
            wall_do=FRT_NULL;
         else
            wall_do--;
   }
   else
   {
      if (_fr_curflags&FR_PICKUPM_MASK)
         wall_do=to_do=FRT_NULL;
      else
         wall_do=to_do=FRT_CSPACE;  
   }

   _fr_terr_int_wall = _fr_tabs_int_wall[wall_do];
   _fr_terr_ext_wall = _fr_tabs_ext_wall[wall_do];
   _fr_terr_ceil     = _fr_tabs_ceil[to_do];
   _fr_terr_flr      = _fr_tabs_flr[to_do];
}

// setup the point revector
void fr_terr_frame_start(void)
{
   int wall_do, ceil_do, flr_do;

// cspace
   if (_fr_curflags&FR_PICKUPM_MASK)
      if (_frp.faces.cyber)
         wall_do=ceil_do=flr_do=FRT_NULL;
      else
         wall_do=ceil_do=flr_do=FRT_FLAT;
   else if (_frp.faces.cyber)
      wall_do=ceil_do=flr_do=_frp.faces.cyber_full+FRT_CSPACE;
   else
	{  // realspace
      int lmod=_frp_light_bits_any()?1:0;
      wall_do=lmod+(((_frp.faces.main)&&(_frp.faces.wall   ))?FRT_TMAP:0);
      ceil_do=lmod+(((_frp.faces.main)&&(_frp.faces.ceiling))?FRT_TMAP:0);
	   flr_do =lmod+(((_frp.faces.main)&&(_frp.faces.floor  ))?FRT_TMAP:0);
   }

   _fr_terr_int_wall = _fr_tabs_int_wall[wall_do];
   _fr_terr_ext_wall = _fr_tabs_ext_wall[wall_do];
   _fr_terr_ceil     = _fr_tabs_ceil[ceil_do];
   _fr_terr_flr      = _fr_tabs_flr[flr_do];

   _fr_parse_obj     = render_parse_obj;

   _fr_render_walls  = _render_3d_walls;

   // setup the tmp points for this frame
   g3_alloc_list(FDT_TMPPTCNT,_fdt_tmppts);

   last_csp_fr=0; // initially no filled mode
   _fdt_terr=TRUE;
}

void fr_terr_frame_end(void)
{
   void fr_tfunc_grab_start(void); 
   fr_tfunc_grab_start();     // set up the physics facelet indirections...
   _fdt_terr=FALSE;
   g3_free_list(FDT_TMPPTCNT,_fdt_tmppts);
}

#if _fr_defdbg(CURSOR)
// wrapped within the walkman with a halo of distortion
// aural contraceptive aborting pregnant conversation
void fr_set_cursor(int x, int y)
 { _fr_cursorx=x; _fr_cursory=y; }
#endif

#define TF_DIRECT

#ifdef TF_DIRECT
#include "tfdirect.h"
#include "ss_flet.h"
// this is a direct render <-> tfunc connection
// ie. we dont accumulate and then send facelets, we just send as we go
//  by calling the tfutil stuff, namely facelet_solve and so on
//  thus we never build 3d surfaces, just distances and normals
// hopefully, this will cause things to, gasp, speed up

//#define AddTmapFlags(fl) if (_game_fr_tmap<10) fl|=SS_BCD_MISC_CLIMB
#define AddTmapFlags(fl) if (textprops[_game_fr_tmap].friction_climb) fl|=SS_BCD_MISC_CLIMB

uchar wall_pc[]=
{
   SS_BCD_PRIM_NEG_Y|SS_BCD_TYPE_WALL, SS_BCD_PRIM_NEG_X|SS_BCD_TYPE_WALL,
   SS_BCD_PRIM_YAXIS|SS_BCD_TYPE_WALL, SS_BCD_PRIM_XAXIS|SS_BCD_TYPE_WALL,
   SS_BCD_PRIM_MULTI|SS_BCD_TYPE_WALL
};

// secret gnosis becomes the order of the day
// we set up primary and transform the center point into our space
// then we just pass it on to facelet_solve, none the worse for wear...
static void _render_tfunc_walls(int which, int cnt)
{  // fuck it, ext_walls are always ext, so punt generality, lets go
   fix pt[3];
   int pc=wall_pc[which];
   switch (which)    // set up localized point set
   {  // NESW
   case 0: pt[2]=fix_1-tf_loc_pt[1]; pt[0]=tf_loc_pt[0]; break;
   case 1: pt[2]=fix_1-tf_loc_pt[0]; pt[0]=fix_1-tf_loc_pt[1]; break;
   case 2: pt[2]=tf_loc_pt[1]; pt[0]=fix_1-tf_loc_pt[0]; break;
   case 3: pt[2]=tf_loc_pt[0]; pt[0]=tf_loc_pt[1]; break;
   }
   pt[1]=tf_loc_pt[2];
   AddTmapFlags(pc);
//   if (pc&SS_BCD_MISC_CLIMB) mprintf("Set climbable... %x\n",pc);
   switch (cnt)
   {
   case 4: tf_solve_aligned_face(pt,final_wall[3],pc,NULL);
   case 3: tf_solve_aligned_face(pt,final_wall[2],pc,NULL);
   case 2: tf_solve_aligned_face(pt,final_wall[1],pc,NULL);
   case 1: tf_solve_aligned_face(pt,final_wall[0],pc,NULL);
   case 0: break;
   }
}

static void _fr_tfunc_diag_wall(int wall_id)
{
   fix pt[3], C;
   int pfl;
   switch (wall_id)
   {
   case 0: // nw - se, normal to upper ne
      diag_norms[0]= fix_inv_root2; diag_norms[1]= fix_inv_root2;
      C=tf_loc_pt[0]-tf_loc_pt[1];
      pt[0]=(fix_1-C)/2; pt[2]=tf_loc_pt[1]-pt[0];
      break;
   case 1: // sw - ne, normal to lower se
      diag_norms[0]= fix_inv_root2; diag_norms[1]=-fix_inv_root2;
      C=tf_loc_pt[0]+tf_loc_pt[1];
      pt[0]=C/2; pt[2]=pt[0]-tf_loc_pt[1];
      break;
   case 2: // ne - sw, normal to upper nw
      diag_norms[0]=-fix_inv_root2; diag_norms[1]= fix_inv_root2;
      C=tf_loc_pt[0]+tf_loc_pt[1];      
      pt[0]=fix_1-(C/2); pt[2]=tf_loc_pt[1]-(C/2);
      break;
   case 3: // nw - se, normal to lower sw
      diag_norms[0]=-fix_inv_root2; diag_norms[1]=-fix_inv_root2;
      C=tf_loc_pt[0]-tf_loc_pt[1];
      pt[0]=(fix_1-C)/2; pt[2]=pt[0]-tf_loc_pt[1];
      break;
   }
   pt[1]=tf_loc_pt[2];
   pfl=wall_pc[4]|TF_FLG_BOX_FULL;
   AddTmapFlags(pfl);
   tf_solve_remetriced_face(pt,tf_diag_walls,pfl,diag_norms,fix_inv_root2);
}

// hack this for now for diagonals
static void _fr_tfunc_int_wall(int wall_id)
{
   if ((_fdt_mask&FACELET_MASK_I)==0) return;
   _fr_tfunc_diag_wall(wall_id);
}

int _fr_tfunc_flr_normal(fix *vec, int *lflg, int fnorm_entry)
{
   int rv;
   if (me_bits_mirror_x(_fdt_mptr)==(MAP_FFLAT<<MAP_MIRROR_SHF))
      fnorm_entry=FRFNORM_VFULL;
   else
	   fnorm_entry&=0xf;
   switch (fnorm_entry)
   {
   case FRFNORM_VZERO:
   case FRFNORM_VFULL: *lflg|=SS_BCD_PRIM_ZAXIS|SS_BCD_TYPE_FLOOR; return 0;

   case FRFNORM_SLPS:  vec[0]=0; vec[1]=slope_norm[_fdt_hgts[FDT_PT_PARM]][2]; rv=1; break;
   case FRFNORM_SLPW:  vec[0]=slope_norm[_fdt_hgts[FDT_PT_PARM]][2]; vec[1]=0; rv=2; break;
   case FRFNORM_SLPN:  vec[0]=0; vec[1]=slope_norm[_fdt_hgts[FDT_PT_PARM]][0]; rv=3; break;
   case FRFNORM_SLPE:  vec[0]=slope_norm[_fdt_hgts[FDT_PT_PARM]][0]; vec[1]=0; rv=4; break;
	}
	vec[2]=slope_norm[_fdt_hgts[FDT_PT_PARM]][1];
   *lflg|=SS_BCD_PRIM_MULTI|SS_BCD_TYPE_FLOOR; 
   return rv;
}

int _fr_tfunc_ceil_normal(fix *vec, int *lflg, int fnorm_entry)
{
   int tmp=me_bits_mirror(_fdt_mptr), rv;
   switch (tmp)
   {
   case MAP_CFLAT:  fnorm_entry=FRFNORM_VFULL; break;
   case MAP_MIRROR: fnorm_entry=fnorm_entry^0x2;      // fall through to the mask
   case MAP_FFLAT:
   case MAP_MATCH:  fnorm_entry&=0xf;
	}
   switch (fnorm_entry)
   {
   case FRFNORM_VZERO:
   case FRFNORM_VZ_MIR:
   case FRFNORM_VF_MIR:
   case FRFNORM_VFULL: *lflg|=SS_BCD_PRIM_NEG_Z|SS_BCD_TYPE_CEIL; return 0;

   case FRFNORM_SLPS:  vec[0]=0; vec[1]=slope_norm[_fdt_hgts[FDT_PT_PARM]][0]; rv=1; break;
   case FRFNORM_SLPW:  vec[0]=slope_norm[_fdt_hgts[FDT_PT_PARM]][0]; vec[1]=0; rv=2; break;
   case FRFNORM_SLPN:  vec[0]=0; vec[1]=slope_norm[_fdt_hgts[FDT_PT_PARM]][2]; rv=3; break;
   case FRFNORM_SLPE:  vec[0]=slope_norm[_fdt_hgts[FDT_PT_PARM]][2]; vec[1]=0; rv=4; break;
	}
	vec[2]=-slope_norm[_fdt_hgts[FDT_PT_PARM]][1];
   *lflg|=SS_BCD_PRIM_MULTI|SS_BCD_TYPE_CEIL; 
   return rv;
}

static fix _tfunc_real_floor[4][2]={{0,fix_1},{fix_1,fix_1},{fix_1,0},{0,0}};

static fix _tfunc_nrm[3], _tfunc_rpts[3];
static int _tfunc_flg;

int do_fc_trans(fix *xy, fix *sc, fix *vals)
{
   int rm=fix_div(fix_1,sc[0]);
   vals[0]=fix_mul(xy[0], sc[0])+fix_mul(xy[1],-sc[1]);
   vals[1]=fix_mul(xy[0], sc[1])+fix_mul(xy[1], sc[0]);
//   mprintf("dft: from %x %x and %x %x stores %x %x, rm %x\n",xy[0],xy[1],sc[0],sc[1],vals[0],vals[1],rm);
   return rm;
}

void do_floor_element(int nrm_mask)
{
   int code;
   fix sc[2], xy[2];

   _tfunc_flg=0;
   code=_fr_tfunc_flr_normal(_tfunc_nrm,&_tfunc_flg,nrm_mask&0xf);
   xy[1]=tf_loc_pt[2]-_fr_fhgt_list[_fdt_hgts[FDT_PT_FLR]];
   switch (code)
   {
   case 0:

	* (g3s_vector *) _tfunc_rpts = * (g3s_vector *) tf_loc_pt;		// sub faster version?      _memcpy32l(_tfunc_rpts,tf_loc_pt,3);
      _tfunc_rpts[2]=xy[1];
      _tfunc_real_floor[0][1]=_tfunc_real_floor[1][1]=fix_1;
      tf_solve_aligned_face(_tfunc_rpts,_tfunc_real_floor,_tfunc_flg|TF_FLG_BOX_FULL,NULL); return;  // flat floor
   case 1: xy[0]=tf_loc_pt[1]; _tfunc_rpts[0]=tf_loc_pt[0]; sc[1]=_tfunc_nrm[1]; break;              // N up (slpS) slope
   case 2: xy[0]=tf_loc_pt[0]; _tfunc_rpts[0]=fix_1-tf_loc_pt[1]; sc[1]=_tfunc_nrm[0]; break;        // E up slope
   case 3: xy[0]=fix_1-tf_loc_pt[1]; _tfunc_rpts[0]=fix_1-tf_loc_pt[0]; sc[1]=-_tfunc_nrm[1]; break; // S up slope
   case 4: xy[0]=fix_1-tf_loc_pt[0]; _tfunc_rpts[0]=tf_loc_pt[1]; sc[1]=-_tfunc_nrm[0]; break;       // W up slope
   }
   sc[0]=_tfunc_nrm[2]; 
   _tfunc_real_floor[0][1]=_tfunc_real_floor[1][1]=do_fc_trans(xy,sc,&_tfunc_rpts[1]);
   if (nrm_mask<=0xf)
   	tf_solve_aligned_face(_tfunc_rpts,_tfunc_real_floor,_tfunc_flg|TF_FLG_BOX_FULL,_tfunc_nrm);
}

void do_ceil_element(int nrm_mask)
{
   int code;
   fix sc[2], xy[2];

   _tfunc_flg=0;
   code=_fr_tfunc_ceil_normal(_tfunc_nrm,&_tfunc_flg,nrm_mask&0xf);
   xy[1]=_fr_fhgt_list[_fdt_hgts[FDT_PT_CEIL]]-tf_loc_pt[2];
   switch (code)
   {
   case 0:
      _tfunc_rpts[0]=tf_loc_pt[0]; _tfunc_rpts[1]=fix_1-tf_loc_pt[1]; _tfunc_rpts[2]=xy[1]; 
      _tfunc_real_floor[0][1]=_tfunc_real_floor[1][1]=fix_1;
      tf_solve_aligned_face(_tfunc_rpts,_tfunc_real_floor,_tfunc_flg|TF_FLG_BOX_FULL,NULL); return;  // flat ceil
   case 1: xy[0]=fix_1-tf_loc_pt[1]; _tfunc_rpts[0]=tf_loc_pt[0]; sc[1]=-_tfunc_nrm[1]; break;       // N up (slpS) slope
   case 2: xy[0]=fix_1-tf_loc_pt[0]; _tfunc_rpts[0]=fix_1-tf_loc_pt[1]; sc[1]=-_tfunc_nrm[0]; break; // E up slope
   case 3: xy[0]=tf_loc_pt[1]; _tfunc_rpts[0]=fix_1-tf_loc_pt[0]; sc[1]=_tfunc_nrm[1]; break;        // S up slope
   case 4: xy[0]=tf_loc_pt[0]; _tfunc_rpts[0]=tf_loc_pt[1]; sc[1]=_tfunc_nrm[0]; break;              // W up slope
   }
   sc[0]=-_tfunc_nrm[2]; 
   _tfunc_real_floor[0][1]=_tfunc_real_floor[1][1]=do_fc_trans(xy,sc,&_tfunc_rpts[1]);
   if (nrm_mask<=0xf)
		tf_solve_aligned_face(_tfunc_rpts,_tfunc_real_floor,_tfunc_flg|TF_FLG_BOX_FULL,_tfunc_nrm);
}

static void _fr_tfunc_flr(void)
{
   uchar nrm_mask=fr_fnorm_list[_fdt_tt];
   int fcecnt;

   if ((_fdt_mask&FACELET_MASK_F)==0) return;
   fcecnt=_fdt_ttf->flags>>FRFLRSHF_2ELEM;  // 0 or 1
//   if ((_fdt_tt!=TILE_OPEN)&&(_fdt_tt!=TILE_SOLID))
//      mprintf("BonusF %d (%d)..",nrm_mask,_fdt_tt);
   if (fcecnt==0)
      do_floor_element(nrm_mask&0xf);
   else
   {
      int tmp=me_bits_mirror(_fdt_mptr);
      uchar icky=(_fdt_tt>=TILE_SLOPECV_NW);
      if (tmp==MAP_FFLAT)
         do_floor_element(FRFNORM_VFULL);
      else if (icky)
      {
         do_floor_element(0xf0|(nrm_mask&0xf)); // now make it 3pt...
         _tfunc_real_floor[1][0]=0; 
         tf_solve_aligned_face(_tfunc_rpts,&_tfunc_real_floor[1],_tfunc_flg|TF_FLG_3PNT_MASK,_tfunc_nrm);
         _tfunc_real_floor[1][0]=fix_1; 
         do_floor_element(0xf0|(nrm_mask>>4));  // and so on...
   		tf_solve_aligned_face(_tfunc_rpts,&_tfunc_real_floor[1],_tfunc_flg|TF_FLG_3PNT_MASK,_tfunc_nrm);
      }
      else
      {
         do_floor_element(nrm_mask&0xf);
         do_floor_element(nrm_mask>>4);
      }
   }
}

static void _fr_tfunc_ceil(void)
{
   uchar nrm_mask=fr_fnorm_list[_fdt_tt];
   int fcecnt;

   if ((_fdt_mask&FACELET_MASK_C)==0) return;
   if (_fdt_ttf->flags&FRFLRFLG_NOTOP) return;
   fcecnt=_fdt_ttf->flags>>FRFLRSHF_2ELEM;  // 0 or 1
//   if ((_fdt_tt!=TILE_OPEN)&&(_fdt_tt!=TILE_SOLID))
//      mprintf("BonusC %d (%d)..",nrm_mask,_fdt_tt);
   if (fcecnt==0)
      do_ceil_element(nrm_mask);
   else
   {
      int tmp=me_bits_mirror(_fdt_mptr);
      uchar icky=(_fdt_tt<TILE_SLOPECV_NW), hard;
      if (tmp==MAP_CFLAT)       hard=0;
      else if (tmp==MAP_MIRROR) hard=!icky;
      else hard=icky;
      if (hard)
      {     // broken, depends on icky
         do_ceil_element(0xf0|(nrm_mask&0xf));
   		tf_solve_aligned_face(_tfunc_rpts,_tfunc_real_floor,_tfunc_flg|TF_FLG_3PNT_MASK,_tfunc_nrm);
         do_ceil_element(0xf0|(nrm_mask>>4));
         _tfunc_real_floor[2][0]=0; 
   		tf_solve_aligned_face(_tfunc_rpts,_tfunc_real_floor,_tfunc_flg|TF_FLG_3PNT_MASK,_tfunc_nrm);
         _tfunc_real_floor[2][0]=fix_1; 
      }
      else
      {
         do_ceil_element(nrm_mask&0xf);
         do_ceil_element(nrm_mask>>4);
      }
   }
}

// check for terrain objects
static void _fr_tfunc_obj(void)
{
   facelet_parse_obj();
}

void fr_tfunc_grab_start(void) 
{
   _fr_terr_int_wall = _fr_tfunc_int_wall;
   _fr_terr_ceil     = _fr_tfunc_ceil;
   _fr_terr_flr      = _fr_tfunc_flr;

   _fr_parse_obj     = _fr_tfunc_obj;

   _fr_render_walls  = _render_tfunc_walls;
}

void fr_tfunc_grab_fast(int mask)
{
   me_subclip_set(_fdt_mptr,SUBCLIP_FULL_TILE);
   _fdt_mask=mask;
   fr_draw_tile();
//   me_subclip_set(_fdt_mptr,SUBCLIP_OUT_OF_CONE);
}
#endif  // TFUNC_SUPPORT

#ifdef WHOSE_MUMP

// add to everything.... arrrrgghgh
#ifndef SHIP
#define CheckRendWallBt(btid) ((me_bits_rend4(c_t)&btid)==0)
#define CheckRendOthBt(btid)  ((me_bits_rend3(c_t)&btid)==0)
#else
#define CheckRendWallBt(btid) (TRUE)
#define CheckRendOthBt(btid)  (TRUE)
#endif

#endif

//==============================================================================
//  Edge-finding routines.
//==============================================================================
bool edge_get_fandc(MapElem *mp, int c_edge, char *e_list);

#define EDGE_GET

#include "fredge.h"
#ifdef EDGE_GET

bool edge_get_fandc(MapElem *mp, int c_edge, char *e_list)
{
   uchar *mmptr, fo, p=me_param(mp), in_fo;
   in_fo=face_obstruct[me_tiletype(mp)][c_edge];
   if ((me_tiletype(mp)==TILE_SOLID)||(in_fo==0xff))
   {
  	   e_list[0]=e_list[1]=MAX_HGT;
  	   e_list[2]=e_list[3]=MAX_HGT;
      return FALSE;
   }
   else if (p==0)
   {
  	   e_list[0]=e_list[1]=me_height_flr(mp);
  	   e_list[2]=e_list[3]=MAX_HGT-me_height_ceil(mp);
   }
   else
   {
	   mmptr=(uchar *)mmask_facelet[me_bits_mirror(mp)];
  	   e_list[0]=e_list[1]=me_height_flr(mp);
	   fo=(in_fo^mmptr[0])&mmptr[1];
  	   if (fo&FO_L_PARM) e_list[0]+=p;
      if (fo&FO_R_PARM) e_list[1]+=p;
	   e_list[2]=e_list[3]=MAX_HGT-me_height_ceil(mp);
      fo=(in_fo^mmptr[2])&mmptr[3];
  	   if (fo&FO_L_PARM) e_list[2]-=p;
      if (fo&FO_R_PARM) e_list[3]-=p;
   }
   return TRUE;
}

char edge_vals[3];
// returns left and right heights for a map edge
// ceil_p is 1 if it is the ceiling you care about
char *map_get_edge(void *omp, int edge, int ceil_p)
{
   uchar *mmptr, fo, p, in_fo;
   MapElem *mp=(MapElem *)omp;

   p=me_param(mp);
   in_fo=face_obstruct[me_tiletype(mp)][edge];

   if (p==0)
   {
	   if (ceil_p)
   	   edge_vals[0]=edge_vals[1]=MAX_HGT-me_height_ceil(mp);
      else
     	   edge_vals[0]=edge_vals[1]=me_height_flr(mp);
   }
   else
   {
	   mmptr=(uchar *)mmask_facelet[me_bits_mirror(mp)];
	   if (ceil_p)
	   {
	  	   edge_vals[0]=edge_vals[1]=MAX_HGT-me_height_ceil(mp);
	      fo=(in_fo^mmptr[2])&mmptr[3];
	      p=-p;
	   }
      else
	   {
		   edge_vals[0]=edge_vals[1]=me_height_flr(mp);
		   fo=(in_fo^mmptr[0])&mmptr[1];
	   }
  	   if (fo&FO_L_PARM) edge_vals[0]+=p;
      if (fo&FO_R_PARM) edge_vals[1]+=p;
   }
   return edge_vals;
}

// 0 and 1 will be ceiling, 2 and 3 will be floor for internal, +4 for other

// this is a really dumb way to a do this, really
// returns 0 if no edge space, 1 for flat, 2 for mini-step, 3 for step
//   4 for ledge, 5 for cliff
int get_edge_code(void *omp, int edge)
{
   char     edge_list[8], d_list[4], x_diff[4];
   char     el, el2, max_df[2], min_dc[2], gap[2];
   MapElem *mp=(MapElem *)omp, *oth_mp=mp+wall_adds[edge];

   // get the real data, if no internal edge we go home and punt...
   if (!edge_get_fandc(mp,edge,&edge_list[0]))
    { edge_vals[0]=edge_vals[1]=edge_vals[2]=0; return MEDGE_NO_TILE; }
   edge_get_fandc(oth_mp,(edge+2)&3,&edge_list[4]);

   for (el=el2=0; el<4; el+=2,el2+=4)
   {
	   x_diff[el+0]=edge_list[el +5]-edge_list[el +0];  // diff from us to it
	   x_diff[el+1]=edge_list[el +4]-edge_list[el +1];  //  fl,fr,cl,cr
	   d_list[el+0]=edge_list[el2+2]-edge_list[el2+0];  // height diffs in square
	  	d_list[el+1]=edge_list[el2+3]-edge_list[el2+1];  //  ourl,ourr,othl,othr
   }
   max_df[0]=max(edge_list[5],edge_list[0]);
   max_df[1]=max(edge_list[4],edge_list[1]);
   min_dc[0]=min(edge_list[7],edge_list[2]);
   min_dc[1]=min(edge_list[6],edge_list[3]);
   edge_vals[0]=gap[0]=min_dc[0]-max_df[0];
   edge_vals[1]=gap[1]=min_dc[1]-max_df[1];
   edge_vals[2]=(gap[0]+gap[1])>>1;    // sure, average, not min or max

   if ((gap[0]<=0)&&(gap[1]<=0))
      return MEDGE_NO_EGRESS;
   else if ((x_diff[0]|x_diff[1])==0)
      return MEDGE_FLAT_CASE;    // should scale these based on zshf
   else if ((abs(x_diff[0])>8)||(abs(x_diff[1])>8))
      return MEDGE_CLIFF_THING;
   else if ((abs(x_diff[0])>4)||(abs(x_diff[1])>4))
      return MEDGE_LARGE_STEP;
   else if ((abs(x_diff[0])>1)||(abs(x_diff[1])>1))
      return MEDGE_SMALL_STEP;
   return MEDGE_NO_TILE;
}
#endif
