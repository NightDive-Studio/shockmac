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
 * $Source: r:/prj/cit/src/RCS/faceobj.c $
 * $Revision: 1.28 $
 * $Author: dc $
 * $Date: 1994/09/08 06:30:25 $
 */

#include <string.h>

#include "frcamera.h"
#include "gameobj.h"

#include "fr3d.h"
#include "frintern.h"
#include "frtables.h"

#include "map.h"

#include "objsim.h"
#include "objprop.h"
#include "objclass.h"

#include "render.h"
#include "otrip.h"
#include "objbit.h"

#include "frflags.h"
#include "doorparm.h"

#include "tfdirect.h"
#include "ss_flet.h"

#include "textmaps.h"      // i really dont want to have this here

#define height_step fix_make(0,0x010000>>SLOPE_SHIFT)

// for raycast exclusion and wackiness
extern ObjID terrain_hit_obj;
extern ObjID terrain_hit_exclusion;

static int _n_o_rad;
static Obj *_n_fr_cobj;

void terrain_object_collide(physics_handle src, ObjID target);

#define start_facelet(which) (&facelets[which][0][0])

// Internal Prototypes
fix *localize_object(fix *ax_pt);
bool _axial_relativize(fix *src_pts, fix *targ_pts, bool vec);
bool setup_cube_face(fix ndist, fix xp, fix yp, fix xhlf, fix yhlf, int nrm_cmp);
bool _face_parm_cube(fix *cntr, int x, int y, int z);
void _face_secret_repulsor_hack(void);
void facelet_obj(ObjID cobjid);


// should these become static or pass around, or make relativize a #define
static void _1d_relativize(fixang a_v, fix *plst, fix *tpts, uchar t1, uchar t2, uchar t3)
{
   fix s,c;
   fix_fastsincos(a_v,&s,&c);
   tpts[t1]=fix_mul(plst[t1], c)+fix_mul(plst[t2], s);
   tpts[t2]=fix_mul(plst[t1],-s)+fix_mul(plst[t2], c);
   tpts[t3]=plst[t3];
}

fix *localize_object(fix *ax_pt)
{
   ax_pt[0]=tf_raw_pt[0]-((fix)(_n_fr_cobj->loc.x<<8));
   ax_pt[1]=tf_raw_pt[1]-((fix)(_n_fr_cobj->loc.y<<8));
   ax_pt[2]=tf_raw_pt[2]-((fix)(_n_fr_cobj->loc.z*(height_step>>3)));
   return &ax_pt[0];
}

// convert a point into a relative FoRef
// pass in nothing, oh well
// implied are the current object, which has a location and an x,y,z
bool _axial_relativize(fix *src_pts, fix *targ_pts, bool vec) // (fixang l_h, fixang l_p, fixang l_b)
{
//   fixang l_h, fixang l_p, fixang l_b)
   fixang l_h, l_p, l_b;
   // tweak if necessary for non-standard object centers, i guess
   if (vec)
   {
      l_h=( (_n_fr_cobj->loc.h<<8));
      l_p=(-(_n_fr_cobj->loc.p<<8));
      l_b=(-(_n_fr_cobj->loc.b<<8));
   }
   else
   {
      l_h=(-(_n_fr_cobj->loc.h<<8));
      l_p=( (_n_fr_cobj->loc.p<<8));
      l_b=( (_n_fr_cobj->loc.b<<8));
   }

   // now go relativize
   if ((l_p)|(l_b))
   {  // if any pitch of bank
      if (((l_b)|(l_h))==0)
         _1d_relativize(l_p,src_pts,targ_pts,1,2,0);
      else if ((l_b)==0)
      {  // 2d transform, just pitch and heading..
//         Warning(("Obj with pitch and heading changed\n"));
         return FALSE;
      }
      else
      {     // unsupported case for now
//         Warning(("Obj w/all 3 axis changed\n"));
         return FALSE;
      }
   }
   else if (l_h)
      _1d_relativize(l_h,src_pts,targ_pts,0,1,2);
   else

   * (g3s_vector *) targ_pts = * (g3s_vector *) src_pts; // _memcpy32l(targ_pts,src_pts,3);
      
//   mprintf("Rtv'd %x %x %x to %x %x %x from %x %x %x\n",
//      src_pts[0],src_pts[1],src_pts[2],targ_pts[0],targ_pts[1],targ_pts[2],l_h,l_p,l_b);
   return TRUE;
}

bool setup_cube_face(fix ndist, fix xp, fix yp, fix xhlf, fix yhlf, int nrm_cmp)
{
   fix unit_norm[3];
   fix l_pt[3], walls[4][2], nrm[3];
   int flg=SS_BCD_PRIM_MULTI|TF_FLG_BOX_FULL;

   walls[0][0]=-xhlf; walls[0][1]= yhlf;
   walls[1][0]= xhlf; walls[1][1]= yhlf;
   walls[2][0]= xhlf; walls[2][1]=-yhlf;
   walls[3][0]=-xhlf; walls[3][1]=-yhlf;
   l_pt[0]=xp; l_pt[1]=yp; l_pt[2]=ndist;

   LG_memset(unit_norm,0,3*4);	//  _memset32l(unit_norm,0,3);
   if (nrm_cmp<0)
      unit_norm[(-nrm_cmp)-1] = -fix_1;
   else
      unit_norm[  nrm_cmp -1] =  fix_1;
   _axial_relativize(unit_norm,nrm,TRUE);
   if (nrm[2]>fix_make(0,0xC000))   // sure, why not...
      flg|=SS_BCD_TYPE_FLOOR;
   else 
      flg|=SS_BCD_TYPE_WALL;
//   mprintf("SCF: dist %x, pos %x %x, size %x %x, norm %x %x %x f %d\n",ndist,xp,yp,xhlf,yhlf,nrm[0],nrm[1],nrm[2],nrm_cmp);
   return tf_solve_aligned_face(l_pt,walls,flg,nrm);
}

//#define MIN_SIZE 0x0500
#define MIN_SIZE 0x0050
bool _face_parm_cube(fix *cntr, int x, int y, int z)
{
   bool rv=FALSE;
//   mprintf("FDPC: o %x %x %x ph %x %x %x, at %x %x %x, size %x %x %x, rad %x\n",
//      _n_fr_p.x,_n_fr_p.y,_n_fr_p.z,tf_raw_pt[0],tf_raw_pt[1],tf_raw_pt[2],cntr[0],cntr[1],cntr[2],x,y,z,tf_rad);
   if ((y>MIN_SIZE)&&(z>MIN_SIZE))
      if ((cntr[0]>x)&&(cntr[0]<x+tf_rad))
         rv|=setup_cube_face( cntr[0]-x, cntr[1],cntr[2],y,z, 1);
      else if ((cntr[0]<-x)&&(cntr[0]>-x-tf_rad))
         rv|=setup_cube_face(-cntr[0]-x,-cntr[1],cntr[2],y,z,-1);
   if ((x>MIN_SIZE)&&(z>MIN_SIZE))
      if ((cntr[1]>y)&&(cntr[1]<y+tf_rad))
         rv|=setup_cube_face( cntr[1]-y,-cntr[0],cntr[2],x,z, 2);
      else if ((cntr[1]<-y)&&(cntr[1]>-y-tf_rad))
         rv|=setup_cube_face(-cntr[1]-y, cntr[0],cntr[2],x,z,-2);
   if ((x>MIN_SIZE)&&(y>MIN_SIZE))
      if ((cntr[2]>z)&&(cntr[2]<z+tf_rad))
         rv|=setup_cube_face( cntr[2]-z,cntr[0], cntr[1],x,y, 3);
      else if ((cntr[2]<-z)&&(cntr[2]>-z-tf_rad))
         rv|=setup_cube_face(-cntr[2]-z,cntr[0],-cntr[1],x,y,-3);
   return rv;
}

void _face_secret_repulsor_hack(void)
{
   extern bool comparator_check(int comparator, ObjID obj, uchar *special_code);
   int comparator = objTraps[_n_fr_cobj->specID].comparator, r_prm, flg, r_top, r_bot;
   uchar special;

   if (tf_ph==-1) return;                                            // dont fuck with bullets or L-O-Sight
   if ((tf_loc_pt[0]<0)||(tf_loc_pt[1]<0)||                          // if we are not in the main square for 
       (tf_loc_pt[0]>=fix_1)||(tf_loc_pt[1]>=fix_1)) return;         //   the object, dont repulse it..

   if (!comparator_check(comparator, OBJ_NULL, &special)) return;    // make sure the repulsor is active

   if ((r_bot=objTraps[_n_fr_cobj->specID].p2)!=0)                   // if lower bound
      if (objTraps[_n_fr_cobj->specID].p2 > tf_raw_pt[2]) return;    // too low
   if ((r_top=objTraps[_n_fr_cobj->specID].p3)!=0)                   // if upper bound
      if (r_top+(tf_rad>>1)+(tf_rad>>2) < tf_raw_pt[2]) return;      // too high
   flg=(objTraps[_n_fr_cobj->specID].p4+1)<<SS_BCD_REPUL_SHF;
   if ((flg&SS_BCD_REPUL_TYPE)==SS_BCD_REPUL_UP)
   {
	   if (r_top==0) r_prm=fix_make(7453,0);                          // that should be pretty darn high, folks
      else r_prm=r_top+(tf_rad>>1)+(tf_rad>>2);
   }
   else
      r_prm=r_bot;

   tf_global_bcd_add(flg|TF_FLG_HPARAM,r_prm);
}

void facelet_obj(ObjID cobjid)
{
   short objtrip;
   int obj_type=FAUBJ_UNKNOWN;
   char scale = 0;
   bool tfimp = FALSE;

   // This should do something to distinguish between wall-like terrain and complex terrain
   // values, but I'm not sure what.  Right now they just both keep cranking, although in 
   // reality we probably want to filter out everything but the complex terrain type, but hey
   // that's easy.  -- Rob
   objtrip=OPNUM(cobjid);
   if ((ObjProps[objtrip].flags & TERRAIN_OBJECT)==0)
      return;
   if (cobjid==terrain_hit_exclusion)
      return;

   _n_fr_cobj=&objs[cobjid];
   obj_type=ObjProps[objtrip].render_type;

   _n_o_rad=fix_make(ObjProps[objtrip].physics_xr,0)/96;      // for reanchoring

   // I just ripped out all the types for which it is meaningless to generate terrain data 
   // from/about, that you had already just had them break immediately. -- Rob
   switch (obj_type)
   {
   case FAUBJ_FLATPOLY:
   case FAUBJ_ANIMPOLY:
   case FAUBJ_TEXTPOLY:

   case FAUBJ_BITMAP:
   case FAUBJ_NOOBJ:
	   {
         fix loc_pts[3], h, r=_n_o_rad>>1;
         localize_object(loc_pts);   // fill in local frame
         if (ObjProps[objtrip].physics_z)
 	         h=fix_make(ObjProps[objtrip].physics_z,0)/96;      // for reanchoring
         else
            h=r<<1;
	      switch (ID2TRIP(cobjid))
	      {
	      case REPULSOR_TRIPLE:
	         _face_secret_repulsor_hack();
	         break;
	      case ENERGY_MINE_TRIPLE:
//            mprintf("Mine at %x %x %x, %x %x %x...%x %x %x, loc %x %x %x\n",
//               _n_fr_cobj->loc.x,_n_fr_cobj->loc.y,_n_fr_cobj->loc.z,
//               objs[physics_handle_to_id(0)].loc.x,objs[physics_handle_to_id(0)].loc.y,objs[physics_handle_to_id(0)].loc.z,
//               tf_raw_pt[0],tf_raw_pt[1],tf_raw_pt[2],loc_pts[0],loc_pts[1],loc_pts[2]);
            // gruesome hack due to other gruesome hack....
//           r=0x7800; h=0x8000;
            loc_pts[2]+=(h>>1);
            r=-r;
	      default:
//            mprintf("at %x %x %x, vs %x %x\n",loc_pts[0],loc_pts[1],loc_pts[2],r,h);
//            mprintf("ot Oid %x\n",cobjid);
            tfimp=tf_solve_cylinder(loc_pts,r,h);
	         break;
         }
      }
      break;
   case FAUBJ_SPECIAL:
   // Hey Doug!  This is where I figured I'd put the special-case renderer stuff.
   // I dunno whether or not you think this is a little too hardwired...this should
   // probably be a gamerend call, I think, but for now here's a hook you can use.
   // If you have suggestions, etc. for how to do it better, I bet you know how to
   // get me.... 
      switch(ID2TRIP(cobjid))
      {
      case TRIPBEAM_TRIPLE:
         break;
      // Hmm, these really want to be dealt with as tpolys I guess for terms of
      // facelets...  -- Rob
      case LABFORCE_TRIPLE:
      case RESFORCE_TRIPLE:
      case GENFORCE_TRIPLE:
         break;
      case SML_CRT_TRIPLE:
      case LG_CRT_TRIPLE:
      case SECURE_CONTR_TRIPLE:
         break;
      case FORCE_BRIJ_TRIPLE:
      case FORCE_BRIJ2_TRIPLE:
      case BRIDGE_TRIPLE:
      case CATWALK_TRIPLE:
      case PILLAR_TRIPLE:
      case BARRICADE_TRIPLE:
         {
            Ref r1 = 0, r2 = 0;
            grs_bitmap *b1;
            fix fx,fy,fz,loc_pts[3],cube_pts[3];
            extern grs_bitmap *obj_get_model_data(ObjID id, fix *x, fix *y, fix *z, grs_bitmap *bm2, Ref *ref1, Ref *ref2);

            // r1 and r2 are cleared so that we don't lock the resources!
            // NULL so we don't load a bitmap
            b1 = obj_get_model_data(cobjid,&fx,&fy,&fz,NULL,&r1,&r2);
            if (b1 != NULL)
            {
               localize_object(loc_pts);   // fill in local frame
               if (_axial_relativize(loc_pts,cube_pts,FALSE))
               {
                  cube_pts[2]-=fz;
                  tfimp=_face_parm_cube(cube_pts,fx,fy,fz);
               }
            }
         }
         break;
      }
      break;

// this is all outrageously horrible, as we dont know what we really need to deal with here

   case FAUBJ_TL_POLY:
   case FAUBJ_TEXBITMAP:
   case FAUBJ_TPOLY:
      {
         fix fix_xoff, fix_yoff, loc_pts[3], door_pts[3];

         scale = 0;
         if (objs[cobjid].obclass==CLASS_DOOR)
            if (objs[cobjid].info.current_frame >= DOOR_OPEN_FRAME)   // the door is open
               return;                                  // which means no facelets for now
         fix_xoff=fix_make(0,0x0200)<<6; fix_yoff=fix_make(0,0x0200)<<6;
         if (scale > 0)  { fix_xoff<<=scale; fix_yoff<<=scale; } 
         else            { fix_xoff>>=-scale; fix_yoff>>=-scale; }
         localize_object(loc_pts);   // fill in local frame
         if (_axial_relativize(loc_pts,door_pts,FALSE))
            tfimp=_face_parm_cube(door_pts, fix_xoff, 0, fix_yoff);
         break;
      }
   }
   if (tfimp)
   {
      if (tf_ph!=-1)
         terrain_object_collide(tf_ph,cobjid);
      else if (terrain_hit_obj==OBJ_NULL)
         terrain_hit_obj=cobjid;
   }
}
