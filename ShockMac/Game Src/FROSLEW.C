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
 * FrOslew.c
 *
 * $Source: n:/project/cit/src/RCS/froslew.c $
 * $Revision: 1.8 $
 * $Author: dc $
 * $Date: 1994/04/10 05:34:29 $
 *
 * Citadel Renderer
 *  object slew system controllers/prototypes/vars
 *
 * $Log: froslew.c $
 * Revision 1.8  1994/04/10  05:34:29  dc
 * hack return codes to avoid move_to teleport and physics vector reset...
 * 
 * Revision 1.7  1994/04/10  05:15:41  dc
 * support for cyberman, vfx1, other 6d control structure, inc. HEAD_H
 * 
 * Revision 1.6  1994/01/02  17:12:02  dc
 * Initial revision
 * 
 * Revision 1.5  1993/12/05  05:43:57  mahk
 * Fixed player height thing for slew mode.
 * 
 * Revision 1.4  1993/11/04  16:09:50  dc
 * fix flat floor slewing
 * 
 * Revision 1.3  1993/09/14  05:41:49  dc
 * new fr/camera regieme
 * 
 * Revision 1.2  1993/09/05  20:54:16  dc
 * new regieme for real, or at least more so
 * 
 * Revision 1.1  1993/09/05  20:21:41  dc
 * Initial revision
 * 
 */

#define __FROSLEW_SRC
#include "fauxrint.h"
#include "froslew.h"
#ifndef __RENDTEST__
#include "objsim.h"
#endif
#include "map.h"
#include "tilename.h"
#include "mapflags.h"

long eye_mods[3]={0,0,0};

// prototype
long *fr_objslew_obj_to_list(long *flist, Obj *cobj, int count);
Obj  *fr_objslew_list_to_obj(long *flist, Obj *cobj, int count);

long *fr_objslew_obj_to_list(long *flist, Obj *cobj, int count)
{
   switch (count-1)
   {
   case 5: flist[5]=(cobj->loc.b<<8);
   case 4: flist[4]=(cobj->loc.p<<8);
   case 3: flist[3]=(cobj->loc.h<<8);
   case 2: flist[2]=(cobj->loc.z<<SLOPE_SHIFT_D);
   default:
   case 1: flist[1]=cobj->loc.y;
   case 0: flist[0]=cobj->loc.x;
      break;
   }
   return flist;
}

Obj  *fr_objslew_list_to_obj(long *flist, Obj *cobj, int count)
{
   switch (count-1)
   {
   case 5: cobj->loc.b=(flist[5]>>8);
   case 4: cobj->loc.p=(flist[4]>>8);
   case 3: cobj->loc.h=(flist[3]>>8);
   case 2: cobj->loc.z=(flist[2]>>SLOPE_SHIFT_D);
   default:
   case 1: cobj->loc.y=flist[1];
   case 0: cobj->loc.x=flist[0];
      break;
   }
   return cobj;
}

// returns whether the camera moved or not
bool fr_objslew_go_real_height(Obj *cobj, long *eye)
{
   long     leye[3];
   int      x,y,z;
   MapElem *o_t;

   if (cobj!=NULL) { eye=leye; fr_objslew_obj_to_list(eye,cobj,3); }
   x=eye[0]>>MAP_SH; y=eye[1]>>MAP_SH;
   o_t=MAP_GET_XY(x,y);
   z=me_height_flr(o_t);
   z*=MAP_SC>>SLOPE_SHIFT;
   // someday should teach this about other tile types....
   if (me_bits_mirror(o_t)!=MAP_FFLAT)
	   if ((me_tiletype(o_t)>=TILE_SLOPEUP_N)&&(me_tiletype(o_t)<=TILE_SLOPEUP_W))
	   {
	      int diff;
	      switch (me_tiletype(o_t))
	      {
	      case TILE_SLOPEUP_N: diff=eye[1]&MAP_MK; break;
	      case TILE_SLOPEUP_E: diff=eye[0]&MAP_MK; break;
	      case TILE_SLOPEUP_S: diff=MAP_MK-(eye[1]&MAP_MK); break;
	      case TILE_SLOPEUP_W: diff=MAP_MK-(eye[0]&MAP_MK); break;
	      }
	      z+=(diff*me_param(o_t))>>SLOPE_SHIFT;
	   }
   // now add height of current posture...
   eye[2]=z+PLAYER_HEIGHT/2;             /* this should probably be fixed */
   if (cobj!=NULL) fr_objslew_list_to_obj(eye,cobj,3);
   return TRUE;
}

bool fr_objslew_allowed(Obj *cobj, long *eye)
{
   int x,y; //z
   MapElem *o_t;

   if (cobj!=NULL) fr_objslew_obj_to_list(eye,cobj,3);
   x=eye[0]>>MAP_SH; y=eye[1]>>MAP_SH;
   if ((x<0)||(x>=MAP_XSIZE)||(y<0)||(y>=MAP_YSIZE))
      return FALSE;
   o_t=MAP_GET_XY(x,y);
   if (me_tiletype(o_t)==TILE_SOLID)
      return FALSE;
   if (cobj!=NULL) fr_objslew_list_to_obj(eye,cobj,3);
   return TRUE;
}

// to physics teleport or not
// should teach it not to slam all velocities!
bool fr_objslew_moveone(Obj *cobj, ObjID objnum, int which, int how, bool conform)
{
   long eye[4];
   bool valid_pos=TRUE;

   if (cobj==NULL) cobj=&objs[objnum];
   fr_objslew_obj_to_list(eye,cobj,4);
   switch (which) 
   {
   case EYE_HEADH: eye_mods[0]+=how*cam_slew_scale[which]; return valid_pos; // break;
   case EYE_H:     cobj->loc.h+=(how*cam_slew_scale[which])>>8; break;
   case EYE_RESET: eye_mods[0]=eye_mods[1]=eye_mods[2]=0; return valid_pos; // break;  
   case EYE_B:
   case EYE_P:     eye_mods[which-3]+=how*cam_slew_scale[which]; return valid_pos; // break;
   case EYE_Z:     eye[2]+=how<<SLOPE_SHIFT_D; break;
   case EYE_Y:
   case EYE_X:
	   {
	      fix v[2], tot;
	      tot=how*cam_slew_scale[which];
	      fix_sincos((fixang)(eye[3]+(which==EYE_X?0x4000:0)),v+0,v+1);
	      eye[0]+=fix_int(fix_mul(v[0],tot));
	      eye[1]+=fix_int(fix_mul(v[1],tot));
		   if (conform)
	         if ((valid_pos=fr_objslew_allowed(NULL,eye))==TRUE)
			      fr_objslew_go_real_height(NULL,eye);
         break;
	   }
   }
   fr_objslew_list_to_obj(eye,cobj,3);
   obj_move_to(cobj-objs, &cobj->loc, TRUE);
   return valid_pos;
}

// to physics teleport or not
//#pragma disable_message(202)
bool fr_objslew_setone(int which, int l_new)
{
   switch (which) 
   {
   case EYE_HEADH:
	      eye_mods[0]=l_new;
         return TRUE;
   case EYE_H:     
      break; 
   case EYE_RESET: eye_mods[0]=eye_mods[1]=eye_mods[2]=0; return TRUE;
   case EYE_P:     eye_mods[1]=l_new; return TRUE;
   case EYE_B:     eye_mods[2]=l_new; return TRUE;
   case EYE_Z:     
   case EYE_Y:
   case EYE_X:     break;
   }
   return TRUE;
}
//#pragma enable_message(202)

/* KLC - not used
bool fr_objslew_tele_to(Obj *, int , int )
{
   return TRUE;
}
*/
