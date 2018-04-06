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
 * $Header: n:/project/lib/src/edms/RCS/mtest.c 1.3 1994/04/05 13:49:37 roadkill Exp $
 */

//	Here is the test of the EDMS interface...
//	=========================================

//#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#include "EDMS.h"

#include "mouse.h"
#include "kb.h"
//#include "mprintf.h"

#include "test_bed.h"		       // For our graphics routines in test_bed.cc

#define MOUSE_X_MAX 638            // Empirically derived
#define MOUSE_Y_MAX 400

Marble marble;
State state;
EDMS_data emetic;
physics_handle silly;

//----------------------
// PROTOTYPES
//----------------------
void	main(void);
void	testback( physics_handle C, physics_handle V, int bad, long DATA1, long DATA2, fix location[3] );
void	off_playfield( physics_handle ph );


extern "C" {
//	Here are some fake functions which should be dealt with!!!...
//	=============================================================

//	To be replaced with the terrain functions of your choice...
//	==========================================================
//	Regular guy terrain...
//	----------------------
fix	Terrain( fix X, fix Y, int deriv )
{	

	if( deriv == 0 ) return fix_mul( fix_make(0,0x2000), (X - fix_make(20,0) ) );
	if( deriv == 1 ) return fix_mul( fix_make(0,0x2000), (X - fix_make(20,0) ) );
	if( deriv == 2 ) return 0;

}

// FF terrain...
// -------------
void  FF_terrain( fix X, fix Y, fix Z, terrain_ff* TFF )
{
   X += Y*Z;
}

//	Indoor terrain vis a vis Citadel...
//	-----------------------------------
void	Indoor_Terrain( fix X, fix Y, fix Z, fix R )
{
	X = Y*Z + R;
}

//	And here, ladies and gentlemen, is a celebration of C and C++ and their
//	untamed passion...
//	==================
TerrainData	terrain_info;
}

//	Now the actual stuff...
//	=======================




void main(void)
{

//	Collision callback testing...
//	=============================

	fix dx, dy;
	physics_handle m1, m2, m3;
	bool killed_m2 = FALSE;

	int i;

	short mousex, mousey;
	bool mbut1, mbut2;

	dx = dy = 5;


//	Set the EDMS startup parameters and startup...
//	==============================================
	emetic.playfield_size = fix_make(100,0);
	emetic.min_physics_handle = 0;
	emetic.collision_callback = testback;
	emetic.wall_callback = NULL;
	emetic.awol_callback = off_playfield;
        emetic.snooz_callback = off_playfield;

	EDMS_startup( &emetic );

	marble.mass = fix_make(1,0);		// 1
	marble.size = fix_make(0,0x8000);	// 0.5
	marble.pep  = fix_make(100,0);		// 1

	state.X = fix_make(15,0);	state.X_dot = fix_make(1,0x1000);
	state.Y = fix_make(12,0);	state.Y_dot = fix_make(5,0x1000);
	state.Z = fix_make(10,0);	state.Z_dot = 0;

	silly = EDMS_make_marble( &marble, &state );
	EDMS_settle_object( silly );


	state.X = fix_make(16,0);
	m1 = EDMS_make_marble( &marble, &state );
	EDMS_settle_object( m1 );
	state.X = fix_make(16,0);
 	m2 = EDMS_make_marble( &marble, &state );
	EDMS_settle_object( m2 );
	state.X = fix_make(16,0);
	m3 = EDMS_make_marble( &marble, &state );
	EDMS_settle_object( m3 );


	EDMS_setup_graphics();
	mouse_init( 500, 500 );
	kb_startup(NULL);


//        for ( i = 0; i < 100; i++ ) mprintf( "\n" );


	while (kb_state(0x31) == 0)			// Go until spacebar hit
	{

		mouse_get_xy( &mousex, &mousey );
		mouse_check_btn( 0, &mbut1 );
//		mouse_check_btn( 1, &mbut2 );

		EDMS_draw_object( 0 );
 
		EDMS_soliton_lite ( fix_make(0,0x0200) );
		
		if (mbut1) 
		{
			EDMS_control_marble( silly, 5*fix_div(fix_make((long)(mousex-(MOUSE_X_MAX/2)),0),fix_make(MOUSE_X_MAX,0)),
			                            5*fix_div(fix_make((long)((MOUSE_Y_MAX/2)-mousey),0),fix_make(MOUSE_Y_MAX,0)),
									    fix_make( 1, 0 ) );
//			EDMS_inventory_and_statistics();
//                        }

//		else if (mbut2) {
//			if (!killed_m2) {
//				mprintf( "Hello from inside Mtest.c" );
//				EDMS_kill_object( m2 );
//				killed_m2 = TRUE;
//			}
		}
		else
//			EDMS_control_marble( silly, 0, 0, 0 );
			EDMS_control_marble( silly,	fix_make(15,0), fix_make(1,0x0000), 0 );

	}

	EDMS_kill_graphics();

	kb_shutdown();
	mouse_shutdown();
}






//	Here is a test collision callback...
//	====================================
void	testback( physics_handle C, physics_handle V, int bad, long DATA1, long DATA2, fix location[3] ) {
									  	
//	mprintf( "Callback: %i hits %i with badness %i.\n", C, V, bad );

}

//	And here is the off playfield callback...
//	=========================================
void	off_playfield( physics_handle ph ) {

	//mprintf( "Hey, physics_handle %i is off the playfield!", ph );

}
