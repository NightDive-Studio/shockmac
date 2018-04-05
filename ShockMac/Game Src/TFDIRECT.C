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
 * $Source: r:/prj/cit/src/RCS/tfdirect.c $
 * $Revision: 1.16 $
 * $Author: dc $
 * $Date: 1994/09/08 06:30:34 $
 */

#define __TFDIRECT_SRC
#include <stdlib.h>
#include <string.h>

// pretty cool set of header files, eh?
#include "tfdirect.h"			// ditto, yep yep yep
#include "ss_flet.h"			// for everything, basically, constants mostly
#include "map.h"				// for MAP_HEIGHTS, must be'fore frintern for fdt_mptr
#include "mapflags.h"			// for light_flr and light_ceil
#include "objects.h"			// for ObjsClearDealt
#include "frintern.h"			// for _fdt_x, _y and _mptr
// gruesome annoyance to destroy projectiles
#include "damage.h"
#include "objsim.h"
#include "objbit.h"
#include "objprop.h"

extern void tile_hit(int mx, int my);

#define USE_OLD_PASSING
//#define SAFETY_RETURN
//#define DIAGONAL_CORNERS
#define STAIRS_NEAR_THE_TOP
#define STAIRS_ABOVE_DA_TOP

// i love us
uchar v_to_cur[]=
{
   (SS_BCD_CURR_E|SS_BCD_CURR_LOW)>>SS_BCD_CURR_SHF,
   (SS_BCD_CURR_W|SS_BCD_CURR_LOW)>>SS_BCD_CURR_SHF,
   (SS_BCD_CURR_N|SS_BCD_CURR_LOW)>>SS_BCD_CURR_SHF,
   (SS_BCD_CURR_S|SS_BCD_CURR_LOW)>>SS_BCD_CURR_SHF,
   (SS_BCD_CURR_E|SS_BCD_CURR_MID)>>SS_BCD_CURR_SHF,
   (SS_BCD_CURR_W|SS_BCD_CURR_MID)>>SS_BCD_CURR_SHF,
   (SS_BCD_CURR_N|SS_BCD_CURR_MID)>>SS_BCD_CURR_SHF,
   (SS_BCD_CURR_S|SS_BCD_CURR_MID)>>SS_BCD_CURR_SHF,
   (SS_BCD_CURR_E|SS_BCD_CURR_HIGH)>>SS_BCD_CURR_SHF,
   (SS_BCD_CURR_W|SS_BCD_CURR_HIGH)>>SS_BCD_CURR_SHF,
   (SS_BCD_CURR_N|SS_BCD_CURR_HIGH)>>SS_BCD_CURR_SHF,
   (SS_BCD_CURR_S|SS_BCD_CURR_HIGH)>>SS_BCD_CURR_SHF,
   (SS_BCD_REPUL_UP|SS_BCD_CURR_LOW)>>SS_BCD_CURR_SHF,
   (SS_BCD_REPUL_DOWN|SS_BCD_CURR_LOW)>>SS_BCD_CURR_SHF,
   (SS_BCD_REPUL_UP|SS_BCD_CURR_MID)>>SS_BCD_CURR_SHF,
   (SS_BCD_REPUL_DOWN|SS_BCD_CURR_MID)>>SS_BCD_CURR_SHF,
   (SS_BCD_REPUL_UP|SS_BCD_CURR_HIGH)>>SS_BCD_CURR_SHF,
   (SS_BCD_REPUL_DOWN|SS_BCD_CURR_HIGH)>>SS_BCD_CURR_SHF
};

// Local Prototypes
bool _tf_set_flet(int flags, fix att, fix dist, fix *norm);
bool _tf_internal_chk(void);
fix _tf_border_check_2d(void);
void _tf_norm_create(void);
void _tf_get_crosses(void);
fix tf_solve_2d_case(int flags);
int _stair_check(fix walls[4][2], int flags);
void terrfunc_one_map_square(int fmask);
bool tf_direct(fix fix_x, fix fix_y, fix fix_z, fix rad, int ph, int tf_type);


// old style physics...
extern TerrainData terrain_info;

// passing globals
ss_facelet_return ss_edms_facelets[SS_MAX_FACELETS];
uchar             ss_edms_facelet_cnt;
int               ss_edms_bcd_flags;
int               ss_edms_bcd_param;
uchar             ss_edms_stupid_flag=TFD_FULL;

// globals...
fix (*tf_vert_2d)[2];   // 2d vertices of the face, when reset
char  tf_norm_hnts[4];  // normal hints for strange param stuff
fix  *tf_pt;            // 3 elements: first 2 in plane, 3 is distance from plane
fix   tf_loc_pt[3];     // localized relative to current map tile
fix   tf_raw_pt[3];     // raw world location of object
int   tf_ph;            // current physics handle
fix   tf_rad;           // rad of current physics'ed object

// locals
static fix  tf_norm_2d[4][2];             // computed normals
static char cross_face[4][2], cross_cnt;  // which face is being crossed, count
static char tf_pcnt;                      // points for working face
static fix  tf_cur_rad;                   // obj radius for this solve, changes if remetriced, say

// sneaky sneaky, locals which arent, im such a bad person
#define tfunc_mptr   _fdt_mptr
#define tfunc_map_x  _fdt_x
#define tfunc_map_y  _fdt_y

static fix tfunc_minz, tfunc_maxz;

// no unreachables...
// dbg system stuff

#ifdef TF_TALK_SYSTEM

#define FletList (1<<0)
#define FletSet  (1<<1)
#define IntChk   (1<<2)
#define BordChk  (1<<3)
#define AlignFce (1<<4)
#define RemetFce (1<<5)
#define Grab     (1<<6)
#define Calls    (1<<7)
#define Area     (1<<8)
#define Ret      (1<<9)
#define Cylinder (1<<10)

//#define DEFAULT_TALK        Ret|FletList
#define DEFAULT_TALK        0
#define TF_TALK_STATICS     0xffff

int   tf_talk=DEFAULT_TALK, tf_tmp;

#define tf_talk_setup()     tf_talk=DEFAULT_TALK
#define tf_turn_on(flg)     tf_tmp=tf_talk, tf_talk|=(flg)
#define tf_talk_check(flg)  ((flg&TF_TALK_STATICS)&&(tf_talk&flg))
#define tf_undo_set(flg)    tf_talk=tf_tmp|(flg)

#define do_tf_Spew(flg,dat) mprintf dat
#define tf_Spew(flg,dat)    if (tf_talk_check(flg)) do_tf_Spew(flg,dat)
#else
#define tf_talk_setup() 
#define tf_turn_on(flg)
#define tf_talk_check(flg)  FALSE
#define tf_undo_set(flg)
#define do_tf_Spew(flg,dat)
#define tf_Spew(flg,dat)
#endif
#define tf_Stat(dat)
#define terrfunc_it_calls_inc()

// renderer stuff we call
extern void fr_tfunc_grab_fast(int mask);


#define _tf_list_flet()

// here we need the tmap/size parser stuff to happen....

// actually output a facelet
bool _tf_set_flet(int flags, fix att, fix dist, fix *norm)
{
   fix full_norms[2]={fix_make(1,0),-fix_make(1,0)};
   int pv;
   ss_facelet_return *cur_fc=&ss_edms_facelets[ss_edms_facelet_cnt++];

   tf_Spew(FletSet,("Set %d.. vals %x %x %x, norm %x %x %x\n",ss_edms_facelet_cnt-1,flags,att,dist,norm!=NULL?norm[0]:0xb,norm!=NULL?norm[1]:0xa,norm!=NULL?norm[2]:0xd));
   if (ss_edms_facelet_cnt>=SS_MAX_FACELETS)
      return FALSE;
   cur_fc->flags=flags;
   cur_fc->att=att;
   cur_fc->comp=tf_rad-dist;
   switch (flags&SS_BCD_AXIS_MASK)
   {
   case SS_BCD_PRIM_MULTI:
      * (g3s_vector *) cur_fc->norm = * (g3s_vector *) norm;		//    _memcpy32l(cur_fc->norm,norm,3);
#ifdef USE_OLD_PASSING
      goto i_hate_everyone;
#endif
      return TRUE;
   case SS_BCD_PRIM_XAXIS:
      pv=0;
      break;
   case SS_BCD_PRIM_YAXIS:
      pv=1;
      break;
   case SS_BCD_PRIM_ZAXIS:
      pv=2;
      break;
   }

   LG_memset(cur_fc->norm,0,3*4);	//  _memset32l(cur_fc->norm,0,3);
   cur_fc->norm[pv]=full_norms[flags&SS_BCD_PRIM_NEG];
#ifdef USE_OLD_PASSING
i_hate_everyone:
	{
      int which, prim;
      which=((flags&SS_BCD_TYPE_MASK)==SS_BCD_TYPE_WALL)?2:(((flags&SS_BCD_TYPE_MASK)==SS_BCD_TYPE_CEIL)?0:1);
      prim=((flags&SS_BCD_AXIS_MASK)>>1)-1;
      if (prim==-1) prim=FCE_NO_PRIM;
	   facelet_add(which,cur_fc->norm,cur_fc->att,cur_fc->comp,prim);
//      if ((flags&SS_BCD_MISC_STAIR)&&(cur_fc->att!=fix_1))
//         flags&=~SS_BCD_MISC_STAIR;    // no stair bit when attenuated
      ss_edms_bcd_flags|=flags;
   }
#endif
//   if (ss_edms_bcd_flags&SS_BCD_MISC_CLIMB)
//      tf_talk|=Ret|FletList;
   return TRUE;
}

// tf_internal_chk...
bool _tf_internal_chk(void)
{
   fix dp[2];
	dp[0]=fix_mul(tf_norm_2d[cross_face[0][0]][0],(tf_pt[0]-tf_vert_2d[cross_face[0][1]][0]))+
         fix_mul(tf_norm_2d[cross_face[0][0]][1],(tf_pt[1]-tf_vert_2d[cross_face[0][1]][1]));
	dp[1]=fix_mul(tf_norm_2d[cross_face[1][0]][0],(tf_pt[0]-tf_vert_2d[cross_face[1][1]][0]))+
         fix_mul(tf_norm_2d[cross_face[1][0]][1],(tf_pt[1]-tf_vert_2d[cross_face[1][1]][1]));
   tf_Spew(IntChk,("tic: %x and %x - cross faces %d and %d\n",dp[0],dp[1],cross_face[0][0],cross_face[1][0]));
	return ((dp[0]>0)&&(dp[1]>0));
}

// solver for border case check...

// unit-normals version



// non-unit normals version
// behind the edge? set and then check to see if valid range
#define set_n_chk_1(val) _1d_pt[1]=(val); if ((_1d_pt[1]>0)||(_1d_pt[1]<-tf_cur_rad)) continue

// f_dist is distance away in 3rd dimension, cnt is number of points
// sign is sorta unknown, as f_dist is distance, not direction, hmmm...
fix _tf_border_check_2d(void)
{
   fix _1d_pt[2],_1d_endpt[2];
   int i;
   for (i=0; i<tf_pcnt; i++)
   {
      if ((tf_norm_2d[i][0]!=0)&&(tf_norm_2d[i][1]!=0))
      {  // do the real case, have to learn about cross clock, i think
         fix nfac, c_pt[2];
         tf_Spew(BordChk,("gc2d: face %d fixing to solve real case w %x %x\n",i,tf_norm_2d[i][0],tf_norm_2d[i][1]));
         c_pt[0]=tf_pt[0]-tf_vert_2d[i][0]; c_pt[1]=tf_pt[1]-tf_vert_2d[i][1];
         _1d_pt[1]=fix_mul(tf_norm_2d[i][0],c_pt[0])+fix_mul(tf_norm_2d[i][1],c_pt[1]);
         if (_1d_pt[1]>=0) continue;                  // behind the edge, here we can only check behind, as we are unnormalized as of yet
         _1d_pt[0]=fix_mul(tf_norm_2d[i][1],c_pt[0])-fix_mul(tf_norm_2d[i][0],c_pt[1]);
         _1d_endpt[0]=fix_mul(tf_norm_2d[i][1],tf_norm_2d[i][1])+fix_mul(tf_norm_2d[i][0],tf_norm_2d[i][0]);
         nfac=fix_sqrt(_1d_endpt[0]);                 // well, its all true, sadly, need to normalize
         _1d_pt[1]=fix_div(_1d_pt[1],nfac);
			if (_1d_pt[1]<-tf_cur_rad) continue;          // too far away
         if ((_1d_pt[0]>0)&&(_1d_pt[0]<_1d_endpt[0]))
            return -_1d_pt[0];                        // distance from facelet
         else
         {                                            // finish setting up the 1d case
            _1d_pt[0]   = fix_div(_1d_pt[0],nfac);
            _1d_endpt[0]= fix_div(_1d_endpt[0],nfac);
         }
      }
      else       // is clock gnosis correct or not? who the hell knows...
      {          
         tf_Spew(BordChk,("gc2: face %d simp case nrms %x %x\n",i,tf_norm_2d[i][0],tf_norm_2d[i][1]));
         if (tf_norm_2d[i][0]==0)     // north/south normal
            if (tf_norm_2d[i][1]>0)   // this is the east facing edge, north facing interior normal
             { set_n_chk_1(tf_pt[1]-tf_vert_2d[i][1]); _1d_pt[0]=tf_vert_2d[i][0]-tf_pt[0]; _1d_endpt[0]= tf_norm_2d[i][1]; }
            else                    // south facing normal, ie. invert everything, perhaps this could be cooler, eh?
             { set_n_chk_1(tf_vert_2d[i][1]-tf_pt[1]); _1d_pt[0]=tf_pt[0]-tf_vert_2d[i][0]; _1d_endpt[0]=-tf_norm_2d[i][1]; } 
         else                       // east/west normal
            if (tf_norm_2d[i][0]>0)   // this is the south facing edge, east facing interior normal
             { set_n_chk_1(tf_pt[0]-tf_vert_2d[i][0]); _1d_pt[0]=tf_pt[1]-tf_vert_2d[i][1]; _1d_endpt[0]= tf_norm_2d[i][0]; }
            else
             { set_n_chk_1(tf_vert_2d[i][0]-tf_pt[0]); _1d_pt[0]=tf_vert_2d[i][1]-tf_pt[1]; _1d_endpt[0]=-tf_norm_2d[i][0]; }
      }
      // ok, if we are here, we have already made sure _1d_pt[1] is tween 0 and -tf_cur_rad
      // now we hack it totally for now, since we are lame... basically, just grow square
      tf_Spew(BordChk,("gc2: face %d 1d case pt0 %x pt1 %x endpt0 %x\n",i,_1d_pt[0],_1d_pt[1],_1d_endpt[0]));
#ifdef DIAGONAL_CORNERS
      if ((_1d_pt[0]>=-tf_cur_rad+tf_pt[2])&&(_1d_pt[0]<=_1d_endpt[0]+tf_cur_rad-tf_pt[2])) // over the attenuated facelet
         return -_1d_pt[1];
#else
      if ((_1d_pt[0]>=-tf_cur_rad)&&(_1d_pt[0]<=_1d_endpt[0]+tf_cur_rad)) // over the attenuated facelet
         return -_1d_pt[1];
#endif
   }
   return 0;
}

// create normal set from vectors
#define set_norm(from,to) \
   tf_norm_2d[from][0]=tf_vert_2d[to][1]-tf_vert_2d[from][1]; \
   tf_norm_2d[from][1]=tf_vert_2d[from][0]-tf_vert_2d[to][0]

void _tf_norm_create(void)
{
   int i;
   for (i=0; i<tf_pcnt-1; i++)
    { set_norm(i,i+1); }
   set_norm(i,0);
}

#define cross_check(from, to) \
   if ((tf_vert_2d[to][0]<tf_pt[0])!=cross_lr) \
   { \
      cross_lr=!cross_lr;  \
      cross_face[cross_cnt][0]=from; \
      cross_face[cross_cnt][1]=to; \
      cross_cnt++; \
   }

void _tf_get_crosses(void)
{
   int i, cross_lr;

   cross_cnt=0;
   cross_lr=tf_vert_2d[0][0]<tf_pt[0];

   for (i=0; i<tf_pcnt-1; i++)
    { cross_check(i,i+1); }
   cross_check(i,0);
}


/* tf_solve_2d_case
 *  ok, the solver for facelet sets
 *  note... several things
 *   1st: 5 vertices must be in the facelet list
 *   2nd: box hints are used in the first pass and then split up
 *   3rd: prebuilt normal/params set somehow? xtra params?
 *   4th: pointlist is clockwise (from upper left if a box, else anywhere)
 *
 *  first check for trivial cases (note easy box test and such)
 *  next build normals if necessary
 *  then do real internal check if triv case failed
 *  then the real border case if we have to
 *
 *  returns the attenuation, so 0 means not involved
 */

// todo: finish/optimize box cases
//       get min/max during norm or cross create, then triv check prior to border mess

fix tf_solve_2d_case(int flags)
{
   fix atv;
   tf_pcnt=(flags&TF_FLG_3PNT_MASK)?3:4;
   if (flags&TF_FLG_BOX_MASK)
   {
	   if (flags&TF_FLG_BOX_FULL)    // really should do a standard point clip
      {  // do box internal/external check, set flag
         fix xd,yd,aval;
         xd=tf_vert_2d[0][0]-tf_pt[0];                // off left side?
         if (xd<0) xd=tf_pt[0]-tf_vert_2d[2][0];      // if not, right?
         if (xd<0) xd=0;                              // in middle
         yd=tf_vert_2d[2][1]-tf_pt[1];                // now top bottom 
         if (yd<0) yd=tf_pt[1]-tf_vert_2d[0][1];      // note reverse of 2 and 0 since
         if (yd<0) yd=0;                              //  cartesian in lower left
#ifndef SET_FLAGS_ON_BOX
         if ((xd|yd)==0) return fix_make(1,0);        // flags|=TF_FLG_ICHK_INT;
         aval=(xd>yd)?xd:yd;
         if (aval>tf_cur_rad) return 0;               // flags|=TF_FLG_ICHK_OUT;
         return fix_div(tf_cur_rad-aval,tf_cur_rad);  // flags|=TF_FLG_ICHK_EDGE;
#else
         if ((xd|yd)==0)
            flags|=TF_FLG_ICHK_INT;
         else
         {
	         aval=(xd>yd)?xd:yd;
	         if (aval>tf_cur_rad)
               flags|=TF_FLG_ICHK_OUT;
            else
            {
               flags|=TF_FLG_ICHK_EDGE;
		         return fix_div(tf_cur_rad-aval,tf_cur_rad);
            }
         }
         goto parse_ichk;
#endif
      }
   }

   // now set normals, really should check nhint and then do the right thing(tm)
   //  for now, create non-unitized normals from vertices, someday should do norm hints...
   _tf_norm_create();

   // if we havent gotten a real ichk value...
   if ((flags&TF_FLG_ICHK_MASK)==0)
   {
      _tf_get_crosses();
      if ((cross_cnt==2)&&_tf_internal_chk())
         flags|=TF_FLG_ICHK_INT;
      else
         flags|=TF_FLG_ICHK_EDGE;
   }

#ifdef SET_FLAGS_ON_BOX
parse_ichk:
#endif
   switch (flags&TF_FLG_ICHK_MASK)
   {
   case TF_FLG_ICHK_INT:   // return distance, set struct and all
      return fix_make(1,0);
   case TF_FLG_ICHK_EDGE:  // are we close enough?
      break;   // fall through to attentuation case
   case TF_FLG_ICHK_NONE:
      Warning(("tfd: no ichk data\n"));
   case TF_FLG_ICHK_OUT:
      return fix_0;
   }
   // if we are here, we are in the attentuation case...
   atv=_tf_border_check_2d();
   if (atv>0)
      atv=fix_1-fix_div(atv,tf_cur_rad);
   return atv;
}

// 3 feet, or so
#define STAIR_TOLERANCE   fix_make(0,0x6187)
#define STAIR_MIN         fix_make(0,0x0508)
int _stair_check(fix walls[4][2], int flags)
{
   if (flags&TF_FLG_BOX_FULL)
   {
      int ad;
      ad=walls[0][1]-walls[2][1];
      if (ad<STAIR_MIN)
         return flags;
      else if (ad<STAIR_TOLERANCE)
         return flags|SS_BCD_MISC_STAIR;
#ifdef STAIRS_NEAR_THE_TOP
      ad=walls[0][1]-tf_pt[1];
#ifdef STAIRS_ABOVE_DA_TOP
      if (ad<tf_cur_rad)
#else
      if ((ad>0)&&(ad<tf_cur_rad))
#endif
      {
//         mprintf("Pseudo-stair %x from %x and %x\n",ad,tf_pt[1],walls[0][1]);
         return flags|SS_BCD_MISC_STAIR;
      }
//      else mprintf("no-pseudo-stair %x from %x and %x\n",ad,tf_pt[1],walls[0][1]);
#endif
   }
   else
   {
      if (tf_pcnt==4)
      {
         fix lv, rv, ad;
#ifdef STAIRS_NEAR_THE_TOP
         fix xd, yd, slp, y;
#endif
	      lv=walls[0][1]-walls[3][1];
	      rv=walls[1][1]-walls[2][1];
         ad=(lv+rv)>>1;
	      if (ad<STAIR_MIN)
	         return flags;
	      if (ad<STAIR_TOLERANCE)
	         return flags|SS_BCD_MISC_STAIR;
#ifdef STAIRS_NEAR_THE_TOP
		   xd=walls[1][0]-walls[0][0];
		   yd=walls[1][1]-walls[0][1];
         if (yd!=0)
         {
	         if (xd!=0)
            {
               slp=fix_div(yd,xd);
		         y=fix_mul(slp,tf_pt[0]-walls[0][0])+walls[0][1]; // y=mx+b
            }
            else return flags;
         }
         else y=walls[0][1];
         ad=y-tf_pt[1];
#ifdef STAIRS_ABOVE_DA_TOP
	      if (ad<tf_cur_rad)
#else
	      if ((ad>0)&&(ad<tf_cur_rad))
#endif
         {
//            mprintf("Pseudo-stair %x from %x and %x\n",ad,tf_pt[1],y);
	         return flags|SS_BCD_MISC_STAIR;
         }
//         else mprintf("no-pseudo-stair %x from %x and %x\n",ad,tf_pt[1],y);
#endif
      }
   }
   return flags;
}  

// note: if it turns out aligned > 2*multi, we should do set and reset in multi for rad, and no rad set here
bool tf_solve_aligned_face(fix pt[3], fix walls[4][2], int flags, fix *norm)
{
   fix att;
   bool rv=FALSE;
//   if (norm!=NULL)
//      tf_turn_on(0xffff);
   tf_Spew(AlignFce,("tfd:aligned walls %x %x %x %x %x %x %x %x, pt %x %x %x, flg %x, nrm %x %x %x\n",
      walls[0][0],walls[0][1],walls[1][0],walls[1][1],walls[2][0],walls[2][1],walls[3][0],walls[3][1],pt[0],pt[1],pt[2],flags,
      norm!=NULL?norm[0]:0xb,norm!=NULL?norm[1]:0xa,norm!=NULL?norm[2]:0xd));
   tf_cur_rad=tf_rad;   // or reset in remetric
   if ((pt[2]>0)&&(tf_cur_rad>pt[2]))   // in range
   {
	   tf_pt=pt;
	   tf_vert_2d=walls;
	   att=tf_solve_2d_case(flags);
	   if (att>0)
      {
         if (flags&SS_BCD_TYPE_WALL)
	         flags=_stair_check(walls,flags);
		   _tf_set_flet(flags,att,pt[2],norm);
         rv=TRUE;
      }
   }
//   if (norm!=NULL)
//      tf_undo_set(Ret|FletList);
   return rv;
}

bool tf_solve_remetriced_face(fix pt[3], fix walls[4][2], int flags, fix norm[3], fix metric)
{
   fix att;
   bool rv=FALSE;
//   tf_turn_on(0xffff);
   tf_Spew(RemetFce,("tfd:remetric walls %x %x %x %x %x %x %x %x, pt %x %x %x, nrm %x %x %x, flg %x, metric %x\n",
      walls[0][0],walls[0][1],walls[1][0],walls[1][1],walls[2][0],walls[2][1],walls[3][0],walls[3][1],pt[0],pt[1],pt[2],norm[0],norm[1],norm[2],flags,metric));
   tf_pt=pt;
   tf_vert_2d=walls;
   // set current rad correctly
   tf_cur_rad=fix_mul(metric,tf_rad);
   if ((pt[2]>0)&&(tf_cur_rad>pt[2]))   // in range
   {
	   att=tf_solve_2d_case(flags);
	   if (att>0)
	   {
	      fix cdist=fix_div(pt[2],metric);
         if (flags&SS_BCD_TYPE_WALL)
            flags=_stair_check(walls,flags);
		   _tf_set_flet(flags,att,cdist,norm);
         rv=TRUE;
	   }
   }
//   tf_undo_set(Ret|FletList);
   return rv;
}

// THIS IS BROKEN
// THE INITIAL CHECK SHOULD ADD TF_RAD^2 to rad
// BUT WE HAVE TO CUT FINAL IN 20 MINUTES, SO WE ARENT GOING TO CHANGE IT
bool tf_solve_cylinder(fix pt[3], fix irad, fix height)
{
   bool rv=FALSE, slv=FALSE;
   int flags;
   fix dist_sqrd, r_dist, rad=abs(irad), urad;
   // first check height
   if ((pt[2]<-tf_rad)||(pt[2]>height+tf_rad)) return rv; // nope, high or low
   dist_sqrd=fix_mul(pt[0],pt[0])+fix_mul(pt[1],pt[1]);
   urad=rad; if (irad<0) urad+=tf_cur_rad;
   tf_Spew(Cylinder,("ph %d pts %x %x scyl is in, dsqr is %x, rsq %x, rad %x\n",tf_ph,pt[0],pt[1],dist_sqrd,fix_mul(rad,rad),rad));
   if ((irad<0)||(fix_mul(rad,rad)>dist_sqrd))
   {
      tf_Spew(Cylinder,("double rad hit.."));
      if ((r_dist=fix_sqrt(dist_sqrd))<urad)
      {
         fix nrm[3], att=fix_1, cdist;
         tf_Spew(Cylinder,("scyl in.."));
		   if ((pt[2]<0)||(pt[2]>height))   // flat top+bottom
         {
            LG_memset(nrm,0,3*4);	//            _memset32l(nrm,0,3);

            if (r_dist>(rad>>1)) att=fix_div(r_dist-(urad>>1),(urad>>1));
            else                 att=fix_1;
            if (irad>0)
            {
               if (pt[2]<0) { nrm[2]=-fix_1; cdist=-pt[2]; }
               else         { nrm[2]= fix_1; cdist= pt[2]-height; }
	            if (nrm[2]>0)  flags=SS_BCD_PRIM_ZAXIS|SS_BCD_TYPE_FLOOR;
	  	         else           flags=SS_BCD_PRIM_NEG_Z|SS_BCD_TYPE_CEIL;
	            _tf_set_flet(flags,att,cdist,nrm);
	            tf_Spew(Cylinder,("OverCyl"));
	            slv=TRUE;
            }
         }
         if (!slv)        // unitize normal, call us done
         {
            nrm[0]=fix_div(pt[0],r_dist);
            nrm[1]=fix_div(pt[1],r_dist);
            nrm[2]=0;
            if (r_dist>urad-tf_cur_rad)
            {   
  		         _tf_set_flet(SS_BCD_PRIM_MULTI|SS_BCD_TYPE_WALL,att,urad-r_dist,nrm);
               tf_Spew(Cylinder,("AroundCyl"));
            }
            else
            {
  		         _tf_set_flet(SS_BCD_PRIM_MULTI|SS_BCD_TYPE_WALL,att,0,nrm);
               tf_Spew(Cylinder,("FullyInCyl"));
            }
         }
         rv=TRUE;
//         tf_turn_on(Ret|FletList);
      }
   }
#ifdef TF_TALK_SYSTEM
   if (rv) tf_Spew(Cylinder,("\n"));
#endif
   return rv;
}

void terrfunc_one_map_square(int fmask)
{  // add appropriately set facelet_mask appropriately
   if (fix_from_map_height(MAP_HEIGHTS-1-me_height_ceil(tfunc_mptr) - me_param(tfunc_mptr)) < tfunc_maxz)
      fmask |= FACELET_MASK_C;
   if (fix_from_map_height(me_height_flr(tfunc_mptr) + me_param(tfunc_mptr)) > tfunc_minz)
      fmask |= FACELET_MASK_F;
   tf_Spew(Grab,("grabbing at %x %x, mask %x\n",tfunc_map_x,tfunc_map_y,fmask));
	fr_tfunc_grab_fast(fmask);    // grab the facelets.. are these correct
}

#define FACELET_MASK_Z   0
#define fcs(v1,v2)      (FACELET_MASK_##v1##|FACELET_MASK_##v2##)

// probably could be just 5,5 by having it algorithimically flip the list if needed
uchar tf_wall_check[5][2][5]=
{
 {{fcs(Z,Z)},                                    {fcs(Z,Z)}                                     },
 {{fcs(Z,Z),fcs(S,Z)},                           {fcs(Z,N),fcs(Z,Z)}                            },
 {{fcs(Z,Z),fcs(S,N),fcs(Z,Z)},                  {fcs(Z,Z),fcs(S,N),fcs(Z,Z)}                   },
 {{fcs(Z,Z),fcs(S,Z),fcs(S,N),fcs(Z,Z)},         {fcs(Z,Z),fcs(S,N),fcs(Z,N),fcs(Z,Z)}          },
 {{fcs(Z,Z),fcs(S,Z),fcs(S,N),fcs(Z,N),fcs(Z,Z)},{fcs(Z,Z),fcs(S,Z),fcs(S,N),fcs(Z,N),fcs(Z,Z)} }
};

bool tf_direct(fix fix_x, fix fix_y, fix fix_z, fix rad, int ph, int tf_type)
{
   int fce_minc,xd,yd,xo,yo,centered; // fce_minc is map increment between lines, ?d LGRect size, xo clip offset
   fix minx,miny,maxx,maxy,cenx,ceny; // for full radius of us, center for us, all really ints in the end

   tf_Spew(Calls,("indoor %d at %x %x %x r %x\n",ph,fix_x,fix_y,fix_z,rad));
   cenx=fix_int(fix_x); ceny=fix_int(fix_y);
   ss_edms_facelet_cnt=ss_edms_bcd_flags=ss_edms_bcd_param=0;

   // wacky bcd stuff....
   if (global_fullmap->cyber)
   {
      MapElem *mp;
      int mb=-1,mt;
      mp=MAP_GET_XY(cenx,ceny);
      if ((mt=me_light_flr(mp))!=0)
         mb=mt-1;
      else if ((mt=me_light_ceil(mp))!=0)
         mb=14+mt;
      if (mb!=-1)
         ss_edms_bcd_flags|=((uint)v_to_cur[mb])<<SS_BCD_CURR_SHF;
   }
   if (tf_type==TFD_BCD) return FALSE;

   ObjsClearDealt();
   tf_talk_setup();
#ifdef USE_OLD_PASSING
   facelet_clear();
#endif

   // find bounding map box
   tf_rad=rad; tf_ph=ph;
   minx=fix_x-rad; if (minx<0) { xo=fix_int(-minx)+1; minx=0; } else { minx=fix_int(minx); xo=0; }
   miny=fix_y-rad; if (miny<0) { yo=fix_int(-miny)+1; miny=0; } else { miny=fix_int(miny); yo=0; }

   maxx=fix_int(fix_x+rad); xd=maxx-minx+1+xo;  // get unclipped x width/distance stuff
   if (maxx>=MAP_XSIZE) maxx=MAP_XSIZE-1;
   maxy=fix_int(fix_y+rad); yd=maxy-miny+1+yo;  // get unclipped y width/distance stuff
   if (maxy>=MAP_YSIZE) maxy=MAP_YSIZE-1;
   tfunc_maxz=fix_z+rad; tfunc_minz=fix_z-rad;  // set top and bottom height for square
   tfunc_mptr=MAP_GET_XY(minx,miny);

   tf_raw_pt[0]=fix_x;
   tf_loc_pt[0]=fix_x-fix_make(minx,0);         // what is the coordinate system, eh?
   tf_raw_pt[1]=fix_y;
   tf_loc_pt[1]=fix_y-fix_make(miny,0);
   tf_raw_pt[2]=tf_loc_pt[2]=fix_z;

   tf_Spew(Area,("looking from %x %x to %x %x, cen %x %x d's %d %d o's %d %d\n",minx,miny,maxx,maxy,cenx,ceny,xd,yd,xo,yo));

   if ((xd|yd)==1)                              // only one square to do...
   {
      tfunc_map_x=minx; tfunc_map_y=miny;       // do we really need this?
      terrfunc_one_map_square(FACELET_MASK_I);
      tf_Stat(single);
   }
   else
   {
      uchar *xmsk_base, *ymsk_base, *xmsk_now, *ymsk_now;
      int xb;

      centered=(((xd&1)==0)&&((minx-xo+(xd>>1))==cenx))?0:1;
      xmsk_base=&tf_wall_check[xd-1][centered][xo];
      centered=(((yd&1)==0)&&((miny-yo+(yd>>1))==ceny))?0:1;
      ymsk_base=&tf_wall_check[yd-1][centered][yo];
      xb=tf_loc_pt[0]; fce_minc=MAP_XSIZE-(maxx-minx)-1;
      for (tfunc_map_y=miny, ymsk_now=ymsk_base; tfunc_map_y<=maxy;
           tfunc_mptr+=fce_minc, tfunc_map_y++, ymsk_now++, tf_loc_pt[1]-=fix_1)
	      for (tfunc_map_x=minx, xmsk_now=xmsk_base, tf_loc_pt[0]=xb;  tfunc_map_x<=maxx;
              tfunc_mptr++, tfunc_map_x++, xmsk_now++, tf_loc_pt[0]-=fix_1)
         {
            terrfunc_one_map_square(((*xmsk_now)<<1)|(*ymsk_now)|FACELET_MASK_I);
            tf_Stat(multi);
            if ((tf_type==TFD_RCAST)&&ss_edms_facelet_cnt) return TRUE;
         }
   }
   if (tf_type==TFD_FULL)
   {  // actually figure out what is up with the facelets, send and all
#ifdef USE_OLD_PASSING
      facelet_send();
#endif
   }
   tf_Spew(Ret,("at %x %x %x r %x ret flg %x nrm %x %x %x, %x %x %x, %x %x %x\n",
      fix_x, fix_y, fix_z, rad, ss_edms_bcd_flags,            terrain_info.cx,terrain_info.cy,terrain_info.cz,
      terrain_info.fx,terrain_info.fy,terrain_info.fz,        terrain_info.wx,terrain_info.wy,terrain_info.wz));
   tf_Spew(Calls,("indoor done for %d, tt %d, saw %d\n",tf_ph,tf_type,ss_edms_facelet_cnt));
   if (tf_talk_check(FletList))
	   _tf_list_flet();

   // should do something a little more real here, eh?

   if ((tf_ph!=-1)&&(ss_edms_facelet_cnt))
   {
      ObjID cobjid=physics_handle_to_id(tf_ph);
      if (ObjProps[OPNUM(cobjid)].flags & SPCL_TERR_DMG)
         special_terrain_hit(cobjid);
      if (global_fullmap->cyber)
         tile_hit(cenx,ceny);
   }

   return ss_edms_facelet_cnt!=0;
}

void tf_global_bcd_add(int flg, int param)
{
   if (flg&TF_FLG_HPARAM)
   {
      flg&=~TF_FLG_HPARAM;
      ss_edms_bcd_param=param;
   }
   ss_edms_bcd_flags|=flg;
}

/* actual call
 * note us passing lots of annoying useless things on the stack and annoying everyone
 */
//extern "C"
//{
void Indoor_Terrain(fix fix_x, fix fix_y, fix fix_z, fix rad, int ph);
//}

void Indoor_Terrain(fix fix_x, fix fix_y, fix fix_z, fix rad, int ph)
{
   tf_direct(fix_x, fix_y, fix_z, rad, ph, ss_edms_stupid_flag);
}
