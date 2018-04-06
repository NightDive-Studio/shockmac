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
 * $Header: r:/prj/lib/src/new_edms/rcs/edms.h 1.2 1994/08/11 18:54:10 dfan Exp $
 */


//	This is the MAIN header file for the EDMS system. It is used ONLY IN THE MAIN CALLING
//	ROUTINE! Just so that functions are handy!
//	==========================================


//	Condom...
//	---------
#ifndef __EDMS_H
#define __EDMS_H


#include "fix.h"


//	Max and Min
//	===========
#define MAX_OBJ 96



//	Physics handles handlers...
//	===========================
#include "physhand.h"



//	Stuff that used to be in physhand.h...
//	--------------------------------------
typedef int					object_number;
extern int					min_physics_handle;

extern object_number 	ph2on[MAX_OBJ];
extern physics_handle	on2ph[MAX_OBJ];

#define physics_handle_to_object_number(ph) (ph2on[ph])
#define object_number_to_physics_handle(on) (on2ph[on])

//	Just in case...
//	---------------
#ifdef __cplusplus

extern "C" {
#endif
void           EDMS_init_handles( void );
physics_handle EDMS_bind_object_number( object_number on );
void           EDMS_remap_object_number( object_number old, object_number nu );
physics_handle EDMS_get_free_ph( void );
void           EDMS_release_object( physics_handle ph );

#ifdef __cplusplus
}
#endif


//	Memory conserving stuff...
//	==========================
//typedef Q EDMS_Argument_Block[MAX_OBJ][7][4];
//typedef Q (*EDMS_Argblock_Pointer)[7][4];

//	The user must make sure that there's this much space...
//	=======================================================
//#define EDMS_ARGBLOCKSIZE (sizeof(EDMS_Argument_Block))


//	Soliton the magic Solver...
//	===========================
//void	EDMS_soliton( fix timestep );
void		EDMS_soliton_lite( fix timestep );
void		EDMS_soliton_vector( fix timestep );
void		EDMS_soliton_vector_holistic( fix timestep );




// Tools...
// ========
// Here is a routine that will attempt to settle an object to the local b/c.  It is NOT intended for
// online use.  A negative return value indicates a badly placed or unphysical model...
// ====================================================================================
int	EDMS_settle_object( physics_handle ph );

void	EDMS_mprint_state( physics_handle ph );

//	Prints out state and sleep information on ALL objects.  Show sleepers is 1 to display sleeping objects...
//	---------------------------------------------------------------------------------------------------------
void	EDMS_inventory_and_statistics( int show_sleepers );

// Returns TRUE if an object is awake, FLASE otherwise...
// ------------------------------------------------------
bool  EDMS_frere_jaques( physics_handle ph );

//	Checks integrity of EDMS.  Returns EDMS error codes, as seen below.
//	-------------------------------------------------------------------
int   EDMS_sanity_check( void );

//	Here we exclude objects from hitting specific others...
//	-------------------------------------------------------
void	EDMS_ignore_collisions( physics_handle ph1, physics_handle ph2 );

//	Here we reallow collisions...
//	-----------------------------
void	EDMS_obey_collisions( physics_handle ph1 );

//	Autodestruct objects kill themselves after the first or second collision callback.  This is model specific.
//	-----------------------------------------------------------------------------------------------------------
void	EDMS_set_autodestruct( physics_handle ph );
void	EDMS_defuse_autodestruct( physics_handle ph );

// Wake me up no matter what (i.e. terrain is changing, new level, etc.)...
// ------------------------------------------------------------------------
void  EDMS_crystal_meth( physics_handle ph );



//	Structs...
//	==========
typedef struct {

fix	playfield_size;
int	min_physics_handle;

void	( *collision_callback )( physics_handle caller, physics_handle victim, int badness, long DATA1, long DATA2, fix location[3] ),
		( *autodestruct_callback )( physics_handle caller ),
		( *awol_callback )( physics_handle caller ),
		( *snooz_callback )( physics_handle caller );

void	*argblock_pointer;


} EDMS_data;

typedef struct {

fix	X, Y, Z,
   	alpha, beta, gamma;
fix	X_dot, Y_dot, Z_dot,
   	alpha_dot, beta_dot, gamma_dot;

} State;



//	Master control...
//	=================
void	EDMS_get_state( physics_handle ph, State *s ),
   	EDMS_startup( EDMS_data* D ),
   	EDMS_kill_object( physics_handle ph ),
      EDMS_holistic_teleport( physics_handle ph, State *s );



//	Object packages...
//	==================


//	Marble:
//	-------
typedef struct {

fix	mass,
      size,
      pep;

} Marble;


physics_handle EDMS_make_marble( Marble *m, State *s );
void EDMS_get_marble_parameters( physics_handle ph, Marble *m );
void EDMS_set_marble_parameters( physics_handle ph, Marble *m );
void EDMS_control_marble( physics_handle ph, fix X, fix Y, fix Z );


//	Robot:
//	------
typedef struct {

fix	mass,
      size,
      hardness,
      pep,
      gravity;
int	cyber_space;

} Robot;

physics_handle EDMS_make_robot( Robot *m, State *s );
void EDMS_get_robot_parameters( physics_handle ph, Robot *m );
void EDMS_set_robot_parameters( physics_handle ph, Robot *m );
void EDMS_control_robot( physics_handle ph, fix thrust_lever, fix attitude_jets, fix jump_jet );
fix  EDMS_get_robot_damage( physics_handle ph );
void EDMS_make_robot_antisocial( physics_handle ph );
void EDMS_make_robot_social( physics_handle ph );

//	Nota bene:  Here the desired heading is specified is in the range 
//		    0 <= desired_heading < 2pi.	Urgency is a number in the range
//		    0 <= urgency <= 20.  A zero urgency will produce no control input.
//		    ------------------------------------------------------------------
void EDMS_ai_control_robot( physics_handle ph,
			    fix desired_heading,
			    fix desired_speed,
				 fix sidestep,
			    fix urgency,
			    fix *are_we_there_yet,
             fix distance );


				 
//	Pelvis:
//	------
typedef struct {

fix	mass,
      size,
   	hardness,
	   pep,
   	gravity,
		height;

int   cyber_space;

} Pelvis;

physics_handle EDMS_make_pelvis( Pelvis *p, State *s );
void EDMS_control_pelvis( physics_handle ph, fix forward, fix turn, fix sidestep, fix lean, fix jump, int crouch );
void EDMS_get_pelvic_viewpoint( physics_handle ph, State *s );
void EDMS_get_pelvis_parameters( physics_handle ph, Pelvis *p );
void EDMS_set_pelvis_parameters( physics_handle ph, Pelvis *p );
fix  EDMS_get_pelvis_damage( physics_handle ph, fix delta_t );


//	Death...
//	--------
typedef struct {

fix	mass,
		size,
		gravity;

} Death;

physics_handle EDMS_make_death( Death *d, State *s );
void EDMS_get_death_parameters( physics_handle ph, Death *d );
void EDMS_set_death_parameters( physics_handle ph, Death *d );



//	Aggregate objects:
//	------------------
extern int make_jello_cube( fix X, fix Y, fix Z, fix size, int points );
extern int make_octahedron( fix X, fix Y, fix Z, fix size );
extern int make_chair( fix X, fix Y, fix Z, fix size, int points );



//	Bipeds...
//	---------
typedef struct {

fix	mass,
   	max_speed,
	   skill,
   	gravity;

fix   hip_radius,
      thigh,
      shin,
      torso;

fix   shoulders,  //Arms have 1/2 length segments, i.e. elbow always at 1/2*arms
      arms;
} Biped;

physics_handle	EDMS_make_biped( Biped *b, State *s, fix *skeleton );
void		EDMS_make_biped_skeleton( physics_handle ph );
void		EDMS_control_biped( physics_handle ph, fix forward, fix side_rotate, int mode );
void		EDMS_set_biped_parameters( physics_handle ph, Biped *b );



//	"Faster than light" objects... Objects that move over their entire trajectory in a frame or two...
//	--------------------------------------------------------------------------------------------------
physics_handle	EDMS_beam_weapon( fix X[3],						//Location of gun, *returns hit location*...
											fix D[3],						//Unit vector in direction of barrel...
											fix kick,						//Art Min requested hacked kickback parameter, no physical meaning...
											fix knock,						//Art Min requested hacked knockback (see above)...
											fix size,						//Radius of bullet in meters...
											fix range,						//Range of weapon in meters...
											physics_handle exclude,		//A physics object that is immune to hits...
											physics_handle shooter );	//The physics object who fired...



//	FGREFALL ONLY - this is temporary until terrain functions are unified!!!...
//	---------------------------------------------------------------------------
physics_handle	EDMS_FF_beam_weapon( fix X[3],						//Location of gun, *returns hit location*...
												fix D[3],						//Unit vector in direction of barrel...
												fix speed,						//Speed of bullet in m/s...
												fix mass,						//Mass of projectile in kilos...
												fix size,						//Radius of bullet in meters...
												fix range,						//Range of weapon in meters...
												physics_handle exclude,		//A physics object that is immune to hits...
												physics_handle shooter,		//The physics object who fired...
												long *w_info,					//FF wall information, returned if a wall is hit...
												long *g_info,					//FF ground information, "" "" "" "" ...
												bool *hit );					//TRUE for a hit, FALSE if range is exceeded without a hit...


// Freefall terrain data structures...
// ===================================
typedef struct{

fix   g_height,                  //The ground...
      g_dx,
      g_dy,
      g_dz,

      w_x,                       //Any walls...
      w_y,
      w_z;

fix	terrain_information;			//Squishiness, friction, et cetera...

long	DATA1,							//For terrain return information...
		DATA2;

physics_handle	caller;				//Who's responsible...

} terrain_ff;


//	The indoor terrain data structures...
//	=====================================
typedef struct {						//Filled by user when
											//Indoor_Terrain is
fix	cx,cy,cz;						//called...
fix	fx,fy,fz;						
fix	wx,wy,wz;

} TerrainData;


//	Finally, EDMS error codes...
//	============================
#define		EDMS_TOO_MANY_OBJECTS 1
#define		EDMS_PHYSICS_HANDLES_CORRUPT 2
#define		EDMS_OBJECT_HANDLES_CORRUPT 3
#define		EDMS_IDOF_POINTERS_CORRUPT 4


#endif /* __EDMS_H */

