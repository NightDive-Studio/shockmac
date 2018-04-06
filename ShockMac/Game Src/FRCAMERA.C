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
 * FrCamera.c
 *
 * $Source: r:/prj/cit/src/RCS/frcamera.c $
 * $Revision: 1.15 $
 * $Author: dc $
 * $Date: 1994/07/15 13:58:34 $
 *
 * Citadel Renderer
 *  camera position/modification/creation system
 *
 * bool    fr_camera_create (cams *camtype, int *arg1, int *arg2)
 * int     fr_camera_update (cams *cam, int *arg1, int *arg2)
 * void    fr_camera_slewone(cams *cam, int which, int how)
 * fix    *fr_camera_getpos (cams *cam)
 * void    fr_camera_setdef (cams *cam)
 * void    fr_camera_slewcam(cams *cam, int which, int how)
 *  
 * $Log: frcamera.c $
 * Revision 1.15  1994/07/15  13:58:34  dc
 * check for cameras off the map at getpos time
 * 
 * Revision 1.14  1994/06/28  20:07:16  dc
 * allow null cameras without default cameras without crashing the game
 * 
 * Revision 1.13  1994/05/03  15:58:57  dc
 * flatten other 360 side views....
 * 
 * Revision 1.12  1994/04/10  05:15:26  dc
 * support for cyberman, vfx1, other 6d control structure, inc. HEAD_H
 * 
 * Revision 1.11  1994/04/02  03:43:07  dc
 * clean up errors, so on
 * 
 * Revision 1.10  1994/01/31  05:31:37  dc
 * various hacks for reality, new actual use of camera system
 * 
 * Revision 1.9  1994/01/06  10:35:40  xemu
 * self/run
 * 
 * Revision 1.8  1994/01/02  17:11:25  dc
 * New renderer
 * 
 * Revision 1.7  1993/12/08  22:01:36  unknown
 * yea yea yea
 * 
 * Revision 1.6  1993/12/08  21:38:08  unknown
 * player model
 * 
 * Revision 1.5  1993/09/19  19:09:49  xemu
 * made _def_cam externally accessible
 * 
 * Revision 1.4  1993/09/17  16:57:47  mahk
 * Added 360 support
 * 
 * Revision 1.3  1993/09/16  23:54:54  dc
 * Yo, use cameras for real, allow type mods on the fly
 * 
 * Revision 1.2  1993/09/05  20:54:06  dc
 * new regieme for real
 * 
 * Revision 1.1  1993/09/05  20:21:57  dc
 * Initial revision
 * 
 */

#define __FRCAMERA_SRC
#include <string.h>
#include <stdlib.h>        // for abs, of course

#include "frcamera.h"
#include "froslew.h"       // has objects
#include "fauxrint.h"
#include "map.h"

fix    fr_camera_last[CAM_COOR_CNT]={0,0,0,0,0,0};
fix    cam_slew_scale[CAM_COOR_CNT]={fix_make(4,0),fix_make(4,0),fix_make(4,0),128,128,128};
cams  *_def_cam=NULL;

#define _cam_top(cam) cams *_cam=(cam==NULL)?_def_cam:cam; if (_cam==NULL) return 

void    fr_camera_setdef (cams *cam)
 { _def_cam=cam; }

cams   *fr_camera_getdef (void)
 { return _def_cam; }

bool    fr_camera_create (cams *cam, int camtype, void *arg1, void *arg2)
{
   _cam_top(cam) FALSE;
   _cam->type=camtype;
   if (camtype&CAMBIT_OBJ)
      _cam->obj_id=(ushort)arg1;
   else
      LG_memcpy(_cam->coor,arg1,sizeof(fix)*CAM_COOR_CNT);
   LG_memcpy(_cam->args,arg2,sizeof(fix)*CAM_ARGS_CNT);
   return TRUE;
}

uchar   fr_camera_modtype(cams *cam, uchar type_on, uchar type_off)
{
   uchar ret;
   _cam_top(cam) 0;
   ret=_cam->type;
   _cam->type&=~type_off;
   _cam->type|=type_on;
   return ret;
}

// i'll give you fish, i'll give you candy, i'll give you, everything I have in my hand
int     fr_camera_update (cams *cam, void *arg1, int whicharg, void *arg2)
{
   _cam_top(cam) FALSE;
   if (arg1!=NULL)
	   if (_cam->type&CAMBIT_OBJ) _cam->obj_id=(ushort)arg1; else LG_memcpy(_cam->coor,arg1,sizeof(fix)*CAM_COOR_CNT);
   if (whicharg==CAM_UPDATE_ALL)
	   LG_memcpy(_cam->args,arg2,sizeof(fix)*CAM_ARGS_CNT);
   else if (whicharg<CAM_ARGS_CNT)
      _cam->args[whicharg]=(fix)arg2;
   return TRUE;
}

void    fr_camera_setone(cams *cam, int which, int newone)
{
   _cam_top(cam); 
   if (_cam->type&CAMBIT_OBJ)
      fr_objslew_setone(which,newone);
   else
	   _cam->coor[which]=newone;
}

void    fr_camera_slewone(cams *cam, int which, int how)
{
   uchar cv[3]={0,2,1};
   _cam_top(cam);
   if (which>=3)                       /* angles */
   {
      if (which==EYE_RESET)
         _cam->coor[4]=_cam->coor[5]=0;
      else
	      _cam->coor[which]+=how*cam_slew_scale[which];
   }
   else                                /* actual move */
   {
      g3s_vector v[3];
      fix tot, _cammul;

// this just doesnt work, really

      tot=how*cam_slew_scale[which];
      g3_get_slew_step(tot,v+0,v+1,v+2);
//      if (which==2) how*=-1;
      if (_cam->type&CAMFLT_FLAT)
      {
         _cammul=fix_sqrt(fix_mul(tot,tot)-(fix_mul(v[cv[which]].gY,v[cv[which]].gY)));
         _cammul=fix_div(abs(tot),_cammul);
//       Warning(("mul %x from %x and %x\n",_cammul,tot,v[cv[which]]));
      }
      else
	    { _cam->coor[2]-=fix_int((v[cv[which]].gY))<<8; _cammul=fix_make(1,0); }
      _cam->coor[0]+=fix_mul(_cammul,((v[cv[which]].gX)>>8));
      _cam->coor[1]+=fix_mul(_cammul,((v[cv[which]].gZ)>>8));
   }
}

// also in init.c
#define MAGIC_SELFRUN_OBJID      0xC3

void    fr_camera_getobjloc (int oid, fix *store)
{
   Obj *cobj=&objs[oid];
   if (cobj->info.ph!=-1)
   {
#ifndef __RENDTEST__
      extern bool get_phys_info(int ph, fix *targ_array, int cnt);
#endif
     get_phys_info(cobj->info.ph,store,6);
   }
   else
   {
      store[0]=cobj->loc.x<<8;                        store[1]=cobj->loc.y<<8;
	   store[2]=cobj->loc.z<<(8+SLOPE_SHIFT_D);        store[3]=(cobj->loc.h<<8);
      store[4]=(cobj->loc.p<<8);                      store[5]=(cobj->loc.b<<8);
   }

}

fix    *fr_camera_getpos (cams *cam)
{
   _cam_top(cam) NULL;
   if (_cam->type&CAMBIT_OBJ)   /* set fix x,y,z etc from the object positions */
      fr_camera_getobjloc(_cam->obj_id,_cam->coor);

   LG_memcpy(fr_camera_last,_cam->coor,sizeof(fix)*CAM_COOR_CNT);
   if (_cam->type&CAMBIT_MOD)
	   fr_camera_last[3]=(fr_camera_last[3]+eye_mods[0])&0xffff;
   if ((_cam->type&(CAMBIT_OFF|CAMBIT_ANG))!=0)
   {
	   fr_camera_last[3]+=(0x10000)-((1<<(14-CAMANG_S))*(_cam->type&CAMBIT_ANG));
   }  // for now 360 view will stay body flat....
   else if (_cam->type&CAMBIT_MOD)
   {
//	   fr_camera_last[3]=(fr_camera_last[3]+eye_mods[0])&0xffff;
      fr_camera_last[4]=(fr_camera_last[4]+eye_mods[1])&0xffff;
	   fr_camera_last[5]=(fr_camera_last[5]+eye_mods[2])&0xffff;
   }
   return &fr_camera_last[0];
}

void    fr_camera_slewcam(cams *cam, int which, int how)
{
   _cam_top(cam);
   if (_cam->type&CAMBIT_OBJ)
      fr_objslew_moveone(NULL,_cam->obj_id,which,how,TRUE);
   else
      fr_camera_slewone(_cam,which,how);
}
