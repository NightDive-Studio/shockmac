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
** idof.h
**
** #defines for internal degrees of freedom in EDMS
**
** $Header: r:/prj/lib/src/new_edms/RCS/idof.h 1.4 1994/09/07 15:00:54 dfan Exp $
** $Log: idof.h $
 * Revision 1.4  1994/09/07  15:00:54  dfan
 * Beginning to try to figure out robot
 * 
 * Revision 1.3  1994/08/15  18:43:35  roadkill
 * *** empty log message ***
 * 
 * Revision 1.2  1994/08/10  16:04:13  dfan
 * Blackley/approved names for indices 20/29
 * 
 * Revision 1.1  1994/08/10  11:37:25  dfan
 * Initial revision
 * 
*/

//////////////////////////////
//
// Wherein we discover how I[] is indexed
//
//////////////////////////////

// marble I[]
// 17 Z control
// 18 X control
// 19 Y control
//
// marble objectX
//
// 0  terrain height
// 1  terrain dz/dx
// 2  terrain dz/dy
// 3  cos thetaZ
// 4  sin thetaX
// 5  sin thetaY
// 6  zeta = height above ground
// 7  zeta < size?
// 8  z moment
// 9  ??
// 10 proportional to dx (actual compression?)
// 11 proportional to dy
// 12 distance between us and other object
// 13 our radius plus other object's radius
// 14 dx
// 15 dy
// 16 eta: C * mass * penetration
// 17 X drag?
// 18 Y drag?
// 19 pep


// biped I[]
//
// 0   = -N_r[2], whatever that means
// 1-2 ??
// 3-8 save_r_w_r, save_r_w_l ??
// 9   += delta_chi ??
// 10  ??
// 11  for computing a radius ??
// 12  used in computing biped x and y coordinate
// 13  ??
// 14  ??
// 15  ??
// 16  used in knees, right foot gamma?
// 17  left foot gamma?
// 18  ??
// 19  ??
//
// biped objectX
//
// 0  x component of wall vector
// 1  y component of wall vector
// 2  ??
// 3  ??
// 4  delta x
// 5  delta y
// 6
// 7  delta magnitude
// 8  omega magnitude


// pelvis I[]
//
// 0  ??
// 1-5 ??
// 6  ??
// 7  crouch control
// 8  kickback ?? control?
// 9  ?? control?
// 10 cyberspace
// 11-13 ??
// 14 damage ?
// 15 lean control
// 16 turn control
// 17 jump control
// 18 x control
// 19 y control
//
// pelvis objectX
//
// 0  V_raw x
// 1  V_raw y
// 2  V_raw z
// 3  1/checker?
// 4  normalized 0?
// 5  normalized 1?
// 6  normalized 2?
// 7  delta magnitude
// 8  hardness (I[20])
// 9  cyberspace?
// 10 result of shall_we_dance ?
// 11 result of shall_we_dance ?
// 12 distance to other object
// 13 sum of our radius plus other object's
// 14 dx
// 15 dy
// 16 dz
// 17 ??
// 18 jump jets
// 19 ??
// 20 F_m x?
// 21 F_m y?
// 22 F_m z?

// robot I[]
//
//
//
// robot objectX
//
//  0 x terrain
//  1 y terrain
//  2 z terrain
//  3 1/terrain distance?
//  4 normalized 0
//  5 normalized 1
//  6 normalized 2
//  7 delta magnitude ??
//  8 omega magnitude ??
//  9


// 2 means cyberspace ??
#define IDOF_CYBERSPACE          10

// 14 robot damage?

// 16-19 control?

// idofs 20-29 are model-specific
#define IDOF_MODEL_OFFSET        20
#define OFFSET(x)                ((x)-IDOF_MODEL_OFFSET)

#define IDOF_MARBLE_K            20
#define IDOF_MARBLE_D            21
#define IDOF_MARBLE_RADIUS       22
#define IDOF_MARBLE_ROLL_DRAG    23
#define IDOF_MARBLE_MASS_RECIP   24
#define IDOF_MARBLE_GRAVITY      25
#define IDOF_MARBLE_MASS         26
#define IDOF_MARBLE_27           27
#define IDOF_MARBLE_28           28
#define IDOF_MARBLE_29           29

#define IDOF_PELVIS_K            20
#define IDOF_PELVIS_D            21
#define IDOF_PELVIS_RADIUS       22
#define IDOF_PELVIS_ROLL_DRAG    23
#define IDOF_PELVIS_MASS_RECIP   24
#define IDOF_PELVIS_GRAVITY      25
#define IDOF_PELVIS_MASS         26
#define IDOF_PELVIS_MOI_RECIP    27
#define IDOF_PELVIS_ROT_DRAG     28
#define IDOF_PELVIS_MOI          29

#define IDOF_BIPED_MASS          20
#define IDOF_BIPED_KAPPA_LEG     21
#define IDOF_BIPED_DELTA_LEG     22
#define IDOF_BIPED_L_HIP         23
#define IDOF_BIPED_L_THIGH       24
#define IDOF_BIPED_L_SHIN        25
#define IDOF_BIPED_L_TORSO       26
#define IDOF_BIPED_M_BAL         27
#define IDOF_BIPED_SKILL         28
#define IDOF_BIPED_GRAVITY       29

#define IDOF_DEATH_MASS          20
#define IDOF_DEATH_MASS_RECIP    21
#define IDOF_DEATH_IALPHA_RECIP  22
#define IDOF_DEATH_IBETA_RECIP   23
#define IDOF_DEATH_IGAMMA_RECIP  24
#define IDOF_DEATH_FLUID_DRAG    25
#define IDOF_DEATH_GRAVITY       26
#define IDOF_DEATH_SIZE          27
#define IDOF_DEATH_28            28
#define IDOF_DEATH_29            29

#define IDOF_ROBOT_K             20
#define IDOF_ROBOT_D             21
#define IDOF_ROBOT_RADIUS        22
#define IDOF_ROBOT_ROLL_DRAG     23
#define IDOF_ROBOT_MASS_RECIP    24
#define IDOF_ROBOT_GRAVITY       25
#define IDOF_ROBOT_MASS          26
#define IDOF_ROBOT_MOI_RECIP     27
#define IDOF_ROBOT_ROT_DRAG      28
#define IDOF_ROBOT_MOI           29


// What kind of model?
// See globals.cc
#define IDOF_MODEL         30

#define IDOF_RADIUS        31

// what are these?
// 32    flag
// 33-35 external force
#define IDOF_32            32
#define IDOF_33            33
#define IDOF_34            34
#define IDOF_35            35

// see robot.cc, what is this?
#define IDOF_36            36

// -1: can collide with anything
//  x: cannot collide with object # x
#define IDOF_COLLIDE       37


// -1: about to be autodestructed
//  0: cannot be autodestructed
//  1: can be autodestructed
#define IDOF_AUTODESTRUCT  38
