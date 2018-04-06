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
 * FrCamera.h
 *
 * $Source: n:/project/cit/src/inc/RCS/frcamera.h $
 * $Revision: 1.6 $
 * $Author: dc $
 * $Date: 1994/04/10 05:16:16 $
 *
 * Citadel Renderer
 *  camera position/modification/creation system
 *  
 * $Log: frcamera.h $
 * Revision 1.6  1994/04/10  05:16:16  dc
 * support for cyberman, vfx1, other 6d control structure, inc. HEAD_H
 * 
 * Revision 1.5  1994/01/31  05:37:27  dc
 * yea yea yea
 * 
 * Revision 1.4  1994/01/02  17:16:19  dc
 * Initial revision
 * 
 * Revision 1.3  1993/09/17  17:00:31  mahk
 * Added 360 support
 * 
 * Revision 1.2  1993/09/16  23:55:06  dc
 * support for fr internal camera view changing
 * 
 * Revision 1.1  1993/09/05  20:58:57  dc
 * Initial revision
 * 
 */

#ifndef __FRCAMERA_H
#define __FRCAMERA_H

#define CAM_COOR_CNT    6
#define CAM_ARGS_CNT    3

typedef struct {
   uchar  type;                 // camera type code
   ushort obj_id;               // current obj_id, or null if abs used
   fix    coor[CAM_COOR_CNT];   // current abs pos
   fix    args[CAM_ARGS_CNT];   // args interpreted based on type
} cams;

bool    fr_camera_create (cams *cam, int camtype, void *arg1, void *arg2);
uchar   fr_camera_modtype(cams *cam, uchar type_on, uchar type_off);
int     fr_camera_update (cams *cam, void *arg1, int whicharg, void *arg2);
void    fr_camera_slewone(cams *cam, int which, int how);
void    fr_camera_setone (cams *cam, int which, int newCam);
fix    *fr_camera_getpos (cams *cam);
void    fr_camera_slewcam(cams *cam, int which, int how);
cams   *fr_camera_getdef (void);
void    fr_camera_setdef (cams *cam);
void    fr_camera_getobjloc (int oid, fix *store);


#ifndef __FRCAMERA_SRC
extern  fix  cam_slew_scale[CAM_COOR_CNT];
extern  fix  fr_camera_last[CAM_COOR_CNT];
#endif

// cameras are now 8 bit flags
//   1 mods, 1 flt, 2 ang, 2 off, 1 obj
//
//

#define CAMOBJ_S        0
#define CAMOBJ_Z        1
#define CAMOFF_S        (CAMOBJ_S+CAMOBJ_Z)
#define CAMOFF_Z        2
#define CAMANG_S        (CAMOFF_S+CAMOFF_Z)
#define CAMANG_Z        2
#define CAMFLT_S        (CAMANG_S+CAMANG_Z)
#define CAMFLT_Z        1
#define CAMMOD_S        (CAMFLT_S+CAMFLT_Z)
#define CAMMOD_Z        1

//#define MakeCambit(x)   CAMBIT_##x## (((1<<CAM##x##_Z)-1)<<CAM##x##_S)
//#define MakeCambit(OBJ)
//#define MakeCambit(OFF)
//#define MakeCambit(ANG)

#define CAMBIT_OBJ    (((1<<CAMOBJ_Z)-1)<<CAMOBJ_S)
#define CAMBIT_OFF    (((1<<CAMOFF_Z)-1)<<CAMOFF_S)
#define CAMBIT_ANG    (((1<<CAMANG_Z)-1)<<CAMANG_S)
#define CAMBIT_FLT    (((1<<CAMFLT_Z)-1)<<CAMFLT_S)
#define CAMBIT_MOD    (((1<<CAMMOD_Z)-1)<<CAMMOD_S)

#define CAMANG_STRAIGHT (0<<CAMANG_S)
#define CAMANG_LEFT     (1<<CAMANG_S)
#define CAMANG_BACK     (2<<CAMANG_S)
#define CAMANG_RIGHT    (3<<CAMANG_S)

#define CAMOFF_NORM     (0<<CAMOFF_S)
#define CAMOFF_ORBIT    (1<<CAMOFF_S)
#define CAMOFF_SHOULDER (2<<CAMOFF_S)

#define CAMTYPE_ABS     (0<<CAMOBJ_S)
#define CAMTYPE_OBJ     (1<<CAMOBJ_S)

#define CAMFLT_FULL3D   (0<<CAMFLT_S)
#define CAMFLT_FLAT     (1<<CAMFLT_S)

#define CAMMOD_NOMOD    (0<<CAMMOD_S)
#define CAMMOD_USEMOD   (1<<CAMMOD_S)

#define CAM_UPDATE_ALL  (-1)
#define CAM_UPDATE_NONE (CAM_COOR_CNT)

// camera/eyeposition controllers
#define EYE_X     0
#define EYE_Y     1
#define EYE_Z     2
#define EYE_H     3
#define EYE_P     4
#define EYE_B     5
#define EYE_RESET 6
#define EYE_HEADH 7

#endif
