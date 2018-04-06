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
 * $Source: n:/project/cit/src/inc/RCS/fr3d.h $
 * $Revision: 1.2 $
 * $Author: dc $
 * $Date: 1994/06/01 22:50:23 $
 *
 * Citadel Renderer
 *  setup and stuff for point counts, initialization, and so on
 */

//#define FR_PT_CNT    1024
#define FR_PT_CNT    256
#define FR_DEF_FOV   80
#define FR_DEF_AXIS  'X'

#define AXIS_ORDER   AXIS_RIGHT,AXIS_DOWN,AXIS_IN

#define ANGLE_ORDER  ORDER_YXZ
#define pitch        tx
#define bank         tz
#define head         ty
#define xaxis        gX
#define yaxis        gY
#define zaxis        gZ

// conversions
//  does anyone ever call this... oh yea, for matts stuff...
//  ick
#define build_fix_angle(ang) ((65536*(ang))/360)

// coordinate merge - have to inc frcamera.h to use these
#define coor(val) (fr_camera_last[val])
#define ang(val)  (fr_camera_last[val])
#define last_coor(val) (fr_camera_last[val])
#define last_ang(val)  (fr_camera_last[val])
