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
//	Here is the test of the EDMS interface...
//	=========================================

#include <stdio.h>
#include <stdlib.h>

#include "EDMS.h"

#include "mouse.h"
#include "kb.h"
//#include "mprintf.h"

#include "test_bed.h"		       // For our graphics routines in test_bed.cc

#define MOUSE_X_MAX 638                // Empirically derived
#define MOUSE_Y_MAX 200

//Biped biped;
//Death death;
State state;
//Marble marble;
Robot robot;
Pelvis	elvis;
EDMS_data emetic;
physics_handle silly, sillym, sillyr, sillyp;

//	Globals the EDMS needs.
int  ss_edms_bcd_flags = 0;
int  ss_edms_bcd_param = 0;


//	Time stuff...
//	=============
int		frames = 0;
unsigned long	ttime = 0;
float		FPS = 0;
int		delay = 0;


extern "C"
{
//	Here are some fake functions which should be dealt with!!!...
//	=============================================================

//	To be replaced with the terrain functions of your choice...
//	==========================================================
//	Regular guy terrain...
//	----------------------
fix	Terrain( fix X, fix Y, int deriv ) {

	if( deriv == 0 ) return fix_mul( fix_make(0,0x2000), (X - fix_make(20,0) ) );
	if( deriv == 1 ) return fix_mul( fix_make(0,0x2000), (X - fix_make(20,0) ) );
	if( deriv == 2 ) return 0;

	return 0;
}

//	Indoor terrain vis a vis Citadel...
//	-----------------------------------
void	Indoor_Terrain( fix X, fix Y, fix Z, fix R, physics_handle ph )
{
	X = Y*Z + R;
}
} // extern "C"

bool  FF_terrain( fix X, fix Y, fix Z, bool fast, terrain_ff* TFF ) {

	TFF->g_height = fix_mul( fix_make(0,0x2000), (X - fix_make(20,0) ) );
	TFF->g_dx     = fix_make(0,0x2000);
	TFF->g_dy     = 0;
        TFF->g_dz     = 0;

	TFF->w_x = 0;
//	if( X < fix_make(14,0) ) TFF->w_x = fix_make(14,0) - X;

	TFF->w_y = 0;
//	if( Y < fix_make(14,0) ) TFF->w_y = fix_make(14,0) - Y;

 	TFF->w_z = 0;

	TFF->terrain_information = 0;

	return (TRUE);
}



//	And here, ladies and gentlemen, is a celebration of C and C++ and their
//	untamed passion...
//	==================
TerrainData	terrain_info;


//	Now the actual stuff...
//	=======================




void main(int argc, char *argv[])
{

void	off_playfield( physics_handle ph );
void	testback( physics_handle C, physics_handle V, int bad, long DATA1, long DATA2, fix location[3] );

fix		test_parameter = 0;

fix		dx, dy, rot, timestep = fix_make(0,0x03ae);
physics_handle	m1, m2, m3, m4, m5, m6, m7, m8;
//physics_handle  death_comes_calling;

bool		killed_m2 = FALSE;
int		i = 0;

short		mouse_x,
		mouse_y,
		old_mouse_x;
bool		mbut1,
		mbut2;

/*
//	BIPED SKELETONS!!!
//	------------------
fix	skeleton0[50],
	skeleton1[50],
	skeleton2[50],
	skeleton3[50],
	skeleton4[50],
	skeleton5[50],
	skeleton6[50],
	skeleton7[50],
	skeleton8[50];


//	Make sure the skeletons are initialized...
//	------------------------------------------
	for ( i = 0; i < 50; i++ ) skeleton0[i] = skeleton1[i] = 
				   skeleton2[i] = skeleton3[i] =
				   skeleton4[i] = skeleton5[i] =
				   skeleton6[i] = skeleton7[i] =
				   skeleton8[i] = 0;
*/



//	Set the EDMS startup parameters and startup...
//	==============================================
	emetic.playfield_size = fix_make(100,0);
	emetic.min_physics_handle = 1;
	emetic.collision_callback = testback;
	emetic.awol_callback = off_playfield;
//	emetic.wall_callback = NULL;
    emetic.snooz_callback = off_playfield;
	emetic.argblock_pointer = NewPtr(0x7FFF);

	EDMS_startup( &emetic );
/*
//	Make a marble
//	==============================================

	marble.mass = fix_make(2,0);		// 1
	marble.size = fix_make(0,0x8000);	// 0.5
	marble.pep  = fix_make(10,0);		// 1

	state.X = fix_make(0,0);	state.X_dot = fix_make(1,0x1000);
	state.Y = fix_make(0,0);	state.Y_dot = fix_make(-1,0x1000);
	state.Z = fix_make(0,0);	state.Z_dot = 0;

	sillym = EDMS_make_marble( &marble, &state );
	EDMS_settle_object( sillym );
*/
//	Make a robot
//	==============================================

	robot.mass 		= fix_make(1,0);		// 1
	robot.size 		= fix_make(0,0x8000);	// 0.5
//	robot.size 		= fix_make(3,0);
	robot.hardness	= fix_make(15,0);		// 15
	robot.pep  		= fix_make(5,0);
	robot.gravity	= fix_make(4,0);
	robot.cyber_space = FALSE;

	state.X = fix_make(0,0);	state.X_dot = fix_make(1,0x8000);
	state.Y = fix_make(0,0);	state.Y_dot = fix_make(0,0x8000);
	state.Z = fix_make(0,0);	state.Z_dot = 0;

	sillyr = EDMS_make_robot( &robot, &state );
	EDMS_settle_object( sillyr );

//	Make a pelvis
//	==============================================

	elvis.mass 		= fix_make(1,0);		// 1
	elvis.size 		= fix_make(0,0x8000);	// 0.5
	elvis.hardness	= fix_make(15,0);		// 15
	elvis.pep  		= fix_make(5,0);
	elvis.gravity	= fix_make(4,0);
	elvis.height 	= fix_make(0, 0xbd00);
	elvis.cyber_space = FALSE;

	state.X = fix_make(4,0);	state.X_dot = fix_make(1,0x8000);
	state.Y = fix_make(1,0);	state.Y_dot = fix_make(0,0x8000);
	state.Z = fix_make(0,0);	state.Z_dot = 0;

	sillyp = EDMS_make_pelvis( &elvis, &state );
	EDMS_settle_object( sillyp );


//	Make some bipeds
//	==============================================
/*
	biped.mass = fix_make(10,0);	
//	biped.height = fix_make(2,0x2000);
	biped.max_speed = fix_make(5,0);
	biped.skill = fix_make(2,0x2000);
	biped.gravity = fix_make(10,0);	

	death.mass = fix_make(5,0);
	death.size = fix_make(1,0x0000);
	death.gravity = fix_make(10,0);

	state.X = fix_make(15,0);		state.X_dot = fix_make(0,0x0000);
	state.Y = fix_make(15,0);		state.Y_dot = fix_make(0,0x0000);
	state.Z = fix_make(0,0x8000);	state.Z_dot = 0;
	state.alpha = fix_make(0,0);	state.alpha_dot = fix_make(0,0);
	state.beta = fix_make(0,0);		state.beta_dot = fix_make(0,0);
	state.gamma = fix_make(0,0);	state.gamma_dot = fix_make(0,0);
	silly = EDMS_make_biped( &biped, &state, skeleton0 );
//	EDMS_settle_object( silly );

	state.X = fix_make(20,0);			state.X_dot = fix_make(10,0x0000);
	state.Y = fix_make(20,0);			state.Y_dot = fix_make(6,0x0000);
	state.Z = fix_make(1,0);			state.Z_dot = 0;
	state.gamma = fix_make(4,0x1000);	state.gamma_dot = fix_make(0,0);
//	m1    = EDMS_make_biped( &biped, &state, skeleton1 );
//	EDMS_settle_object( m1 );

	state.X = fix_make(15,0x8000);		state.X_dot = fix_make(0,0x0000);
	state.Y = fix_make(15,0);			state.Y_dot = fix_make(6,0x0000);
	state.Z = fix_make(1,0x00);			state.Z_dot = 0;
	state.gamma = fix_make(4,0x2000);	state.gamma_dot = fix_make(5,0);
//	m2 = EDMS_make_biped( &biped, &state, skeleton2 );
//	EDMS_settle_object( m2 );
 
//	biped.height = fix_make(1,0x0000);
	state.X = fix_make(20,0);	state.X_dot = fix_make(0,0x0000);
	state.Y = fix_make(10,0);	state.Y_dot = fix_make(0,0x0000);
	state.Z = fix_make(1,0x8000);	state.Z_dot = 0;
 	state.gamma = fix_make(4,0);	state.gamma_dot = fix_make(0,0);
//	m3 = EDMS_make_biped( &biped, &state, skeleton3 );

//	state.X = fix_make(25,0);	state.X_dot = fix_make(0,0x0000);
//	state.Y = fix_make(20,0);	state.Y_dot = fix_make(0,0x0000);
//	state.Z = fix_make(1,0x00);	state.Z_dot = 0;
//	state.gamma = fix_make(4,0);	state.gamma_dot = fix_make(0,0);
//	m4 = EDMS_make_biped( &biped, &state, skeleton4 );
			   
//	biped.height = fix_make(1,0x0f00);
//	biped.mass = fix_make( 20, 0 );
	state.X = fix_make(25,0);	state.X_dot = fix_make(0,0x0000);
	state.Y = fix_make(10,0);	state.Y_dot = fix_make(0,0x0000);
	state.gamma = fix_make(1,0x0000);	state.gamma_dot = fix_make(0,0);
//	m5 = EDMS_make_biped( &biped, &state, skeleton5 );

	state.X = fix_make(25,0);	state.X_dot = fix_make(0,0x0000);
	state.Y = fix_make(15,0);	state.Y_dot = fix_make(0,0x0000);
	state.gamma = fix_make(2,0x0000);	state.gamma_dot = fix_make(0,0);
//	m6 = EDMS_make_biped( &biped, &state, skeleton6 );

	state.X = fix_make(25,0);	state.X_dot = fix_make(0,0x0000);
	state.Y = fix_make(30,0);	state.Y_dot = fix_make(0,0x0000);
	state.gamma = fix_make(2,0x0000);	state.gamma_dot = fix_make(0,0);
//	m7 = EDMS_make_biped( &biped, &state, skeleton7 );

	state.X = fix_make(25,0);	state.X_dot = fix_make(0,0x0000);
	state.Y = fix_make(0,0);	state.Y_dot = fix_make(0,0x0000);
	state.gamma = fix_make(2,0x0000);	state.gamma_dot = fix_make(0,0);
//	m8 = EDMS_make_biped( &biped, &state, skeleton8 );



	state.X = fix_make(18,0);
	state.Y = fix_make(18,0);
	state.Z = fix_make(1,0);
	state.X_dot = fix_make(-40,0);
	state.Y_dot = fix_make(0,0);
	state.Z_dot = fix_make(10,0);
//	death_comes_calling = EDMS_make_death( &death, &state );
*/


//	Get goin'
//	=========
	EDMS_setup_graphics();
//	mouse_init( 500, 500 );
	kb_startup(NULL);


//	EDMS_control_biped( silly,	fix_make(0,0), fix_make(0,0), 0 );


//	Clear monochrome...
//	===================
//	for ( i=0; i < 50; i++) mprintf( "\n" );

//	Kill problem fixed?
//	===================
//	EDMS_kill_object( m4 );
//	EDMS_kill_object( m8 );

//	Get the time(ms timer)...
//	=========================
//	ttime = clock();

	HideCursor();
	
//	Loop...
//	=======
	while (kb_state(0x31) == 0)			// Go until spacebar hit
	{

//	for( delay = 0; delay < 30000; delay++ ) fix_sqrt( fix_mul( fix_sin( delay ), fix_sin( delay ) ) );

	EDMS_soliton_vector( timestep );
	test_parameter += fix_mul( fix_make(0,0x3000), timestep );

//	EDMS_soliton_lite( timestep );

//	EDMS_make_biped_skeleton( silly );	
//	EDMS_make_biped_skeleton( m1 );	
//	EDMS_make_biped_skeleton( m2 );	
//	EDMS_make_biped_skeleton( m3 );	
//	EDMS_make_biped_skeleton( m4 );	
//	EDMS_make_biped_skeleton( m5 );	
//	EDMS_make_biped_skeleton( m6 );	
//	EDMS_make_biped_skeleton( m7 );	
//      EDMS_make_biped_skeleton( m8 );	


	EDMS_draw_object( 0 );
//	getch();

//	EDMS_control_marble( sillym, fix_make(1,0), fix_make(1,0), 0 );
	EDMS_control_robot( sillyr, fix_make(1,0), fix_make(20,0), 0 );
   	EDMS_control_pelvis(sillyp,fix_make(0,0x8000), fix_make(1,0), 0, 0, 0,0);

//	EDMS_control_robot( sillyr,	fix_mul( fix_make(30,0), fix_sin( test_parameter ) ), 
//								fix_mul( fix_make(0,0x9000), fix_sin( test_parameter ) ),
//								0 );

//	EDMS_control_biped( silly,	fix_make(15,0), fix_make(1,0), 0 );
//	EDMS_control_biped( m1,	fix_mul( fix_make(30,0), fix_sin( test_parameter ) ), fix_mul( fix_make(0,0x9000), fix_sin( test_parameter ) ), 0 );
//	EDMS_control_biped( m1,		fix_make(0,0), fix_make(0,0), 0 );
//	EDMS_control_biped( m2,		fix_make(5,0), fix_make(1,0), 0 );
//	EDMS_control_biped( m3,		fix_make(5,0), fix_make(1,0), 0 );
//	EDMS_control_biped( m4,		fix_make(10,0), fix_make(1,0), 0 );
//	EDMS_control_biped( m5,		fix_make(20,0), fix_make(1,0), 0 );
//	EDMS_control_biped( m6,		fix_make(40,0), fix_make(2,0), 0 );
//	EDMS_control_biped( m7,		fix_make(60,0), fix_make(2,0), 0 );
//	EDMS_control_biped( m8,		fix_make(70,0), fix_make(3,0), 0 );

//	if (frames>100) EDMS_control_biped( silly, 0, 0, 0 );

	frames += 1;
	
	}

	ShowCursor();

//	Hmmmm...
//	========
	EDMS_kill_graphics();
	kb_flush();
	kb_shutdown();
//	mouse_shutdown();

/*

//	Timing information...
//	=====================
	ttime = clock() - ttime;
	printf( "frames: %i\n", frames );
	printf( "time in ticks(tm): %i\n", (int)ttime );
	FPS = (float)ttime / (float)CLOCKS_PER_SEC;
	printf( "time in s: %f\n", FPS );
	FPS = (float)frames/FPS;
	printf( "FPS: %f\n", FPS );

	getch();
*/		
}





//	And here is the off playfield callback...
//	=========================================
void	off_playfield( physics_handle ph ) {

//	mprintf( "Hey, physics_handle %i is off the playfield!", ph );

}


//	Here is a test collision callback...
//	====================================
void	testback( physics_handle C, physics_handle V, int bad, long DATA1, long DATA2, fix location[3] ) {

	
//	mprintf( "Callback: %i hits %i with badness %i.\n", C, V, bad );

}
