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
** externs.h
**
** extern declarations in EDMS
**
** $Header: r:/prj/lib/src/new_edms/RCS/externs.h 1.8 1994/09/19 14:16:17 dfan Exp $
** $Log: externs.h $
 * Revision 1.8  1994/09/19  14:16:17  dfan
 * A_is_active, object_check_hash
 * 
 * Revision 1.7  1994/08/19  19:32:51  dfan
 * declaration of data[][]
 * 
 * Revision 1.6  1994/08/15  18:43:18  roadkill
 * *** empty log message ***
 * 
 * Revision 1.5  1994/08/12  17:30:02  dfan
 * check_for_hit now macro
 * 
 * Revision 1.4  1994/08/12  15:17:19  roadkill
 * *** empty log message ***
 * 
 * Revision 1.3  1994/08/11  15:46:34  dfan
 * are_you_there, check_for_hit,
 * callback functions
 * 
 * Revision 1.2  1994/08/10  12:08:48  dfan
 * k, min_scale_slice
 * 
 * Revision 1.1  1994/08/10  11:37:01  dfan
 * Initial revision
 * 
*/

#ifndef __EXTERNS_H
#define __EXTERNS_H

// state of each object
extern Q S[MAX_OBJ][DOF][DOF_DERIVS];

// This is a copy of S that can be mucked with
extern EDMS_Argblock_Pointer A;

// internal degrees of freedom for each object
extern Q I[MAX_OBJ][DOF_MAX];

// expansion coefficients ??
extern Q k[4][MAX_OBJ][DOF];

// 0 if this object is sleeping ??
extern int no_no_not_me[MAX_OBJ];

// minimum valid physics handle
extern int min_physics_handle;

// ??
extern const Q min_scale_slice;

// does A contain the current state of awake objects, instead of S?
extern bool A_is_active;

//// interfac.cc

// tables to go back and forth from physics handle to object number
extern object_number ph2on[MAX_OBJ];
extern physics_handle on2ph[MAX_OBJ];

// callback functions
extern void	( *EDMS_object_collision )( physics_handle caller, physics_handle victim, int badness,
                                        long DATA1, long DATA2, fix location[3] );
extern void ( *EDMS_autodestruct )( physics_handle caller ); 
extern void ( *EDMS_off_playfield )( physics_handle caller );
extern void ( *EDMS_sleepy_snoozy )( physics_handle caller );

//// collide.cc

extern unsigned int data[ EDMS_DATA_SIZE ][ EDMS_DATA_SIZE ];

////

// multiply by this to go from physics units to collision bin units
extern Q	hash_scale;

// ??
extern int     EDMS_robot_global_badness_indicator;

////////////////////////////// functions

//	Killers and snoozers...
//	=======================
void	EDMS_initialize( EDMS_data* D );
int	EDMS_kill( int object );
void	collision_wakeup( int object );


//	Solvers
//	=======
void	soliton( Q timestep );
void	soliton_lite( Q timestep );
void	soliton_lite_holistic( Q timestep );
void	soliton_vector( Q timestep );
void	soliton_vector_holistic( Q timestep );

// Tools
// =====
int	settle_object( int object );
void	mprint_state( int object );
void	inventory_and_statistics( int show_sleepers );
int	sanity_check( void );

//	Collisions
//	==========
void exclude_from_collisions( int guy_1, int guy_2 );
void reset_collisions( int object );

//	EDMS internal testbed wireframe...
//	==================================
void	draw_object( int );
void	setup_graphics( void );
void	kill_graphics( void );

//	Get the Euler angles we need from the stuff in the state...
//	===========================================================
void	EDMS_get_Euler_angles( Q &alpha, Q &beta, Q &gamma, int object );

////////////////////////////// more stuff

// Collision handling...
// ---------------------
void  write_object( int );          //Write and unwrite to the collision table
void  delete_object( int );         //based on arguments...
void  state_write_object( int );    //and state.
void  state_delete_object( int );
int   are_you_there( int object );
bool  object_check_hash (int object, int hx, int hy);

extern unsigned int test_bitmask;

// int check_for_hit( int other_object ); // are_you_there must be called first!
// has been turned into a macro in edms_int.h!

#endif // __EXTERNS_H
