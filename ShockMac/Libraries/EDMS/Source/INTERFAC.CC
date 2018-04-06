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
 * $Header: n:/project/lib/src/edms/RCS/interfac.cc 1.11 1994/04/20 18:44:15 roadkill Exp $
 */

//	State information and utilities...
//	========================


#include "fixpp.h"
#include "EDMS_int.h"
#include "idof.h"

#pragma require_prototypes off			// Added by KC for this file for Mac version.

#ifdef EDMS_SHIPPABLE
//#include "mout.h"
#endif

//	Here we need include files for each and every model that we'll be using...
//	=====================================================
#include "robot.h"

//	The physics handles definitions...
//	==================================
#include "physhand.h"

//      Sanity...
//      ---------
extern int      EDMS_integrating;

//	Callbacks...
//	============
void	( *EDMS_object_collision )( physics_handle caller, physics_handle victim, int badness,
                                  long DATA1, long DATA2, fix location[3] ) = NULL,
   	( *EDMS_autodestruct )( physics_handle caller ) = NULL,
   	( *EDMS_off_playfield )( physics_handle caller ) = NULL,
   	( *EDMS_sleepy_snoozy )( physics_handle caller ) = NULL;

//	Robots...
//	---------
typedef struct
{
   fix	mass,
      	size,
      	hardness,
      	pep,
      	gravity;
   int	cyber_space;

} Robot;


//	Conversions...
//	==============
//	physics handle to object number...
//	----------------------------------
object_number ph2on[MAX_OBJ];

//	Object number to physics handle...
//	----------------------------------
physics_handle on2ph[MAX_OBJ];



//	Constants...
//	============
Q	fix_zero = 0.;




//	Bridge routine to the terrain functions.  C++ functions are all lower case, C are Capped...
//	===========================================================================================

//	This is outside the extern "C" {} thing
//	because it is not called by the user,
//	but rather calls the user function Terrain().
//	=============================================
Q terrain( Q X, Q Y, int deriv )
{
	Q	ans;

	ans.fix_to( Terrain( X.to_fix(), Y.to_fix(), deriv ) );
	return ans;
}


//	Same with Indoors...
//	--------------------
void indoor_terrain( Q X, Q Y, Q Z, Q R, physics_handle ph )
{
	if ( (X > 1)  &&  (Y > 1)  &&   (X < 64) &&  (Y < 64 ) )
   {
      Indoor_Terrain( X.to_fix(), Y.to_fix(), Z.to_fix(), R.to_fix(), ph );
	}
   else
   {
//      EDMS_robot_global_badness_indicator = 1000;
//      mout << "!EDMS: integrator = " << EDMS_integrating << " !!\n";
//      mout << "!EDMS: Physics handle: " << ph << " is asking for bad terrain.\n";
//      mout << "!EDMS: Asked for location: (" << X << ", " << Y << ", " << Z << ").\n";

      if (ph > -1 )
      {
         int on = ph2on[ph];
//         mout << "!EDMS: Integration location (should match): (" << A[on][0][0] << ", " << A[on][1][0] << ", " << A[on][2][0] << ").\n";
//         mout << "!EDMS: Object location last frame: (" << S[on][0][0] << ", " << S[on][1][0] << ", " << S[on][2][0] << ").\n";
//         mout << "!EDMS: Sleep: " << no_no_not_me[on] << ", EDMS_sanity_check = " << sanity_check() << ".\n";
//         mout << "!EDMS: object number " << on << ", EDMS_type: " << I[on][30] << "\n";
//         mout << "!EDMS: Calling AWOL callback...\n";
//         mout << "Awol in interfac.cc\n";
         EDMS_off_playfield( ph ); 
         no_no_not_me[on] = 0;  //Safety!!!
      }
      else
      {
//         mout << "!EDMS: No further information is available for physics handle -1.  SORRY!\n";
      }
   }
}


//      And with Freefall...
//      --------------------
bool ff_terrain( Q X, Q Y, Q Z, bool fast, terrain_ff* FFT )
{
   return FF_terrain( X.to_fix(), Y.to_fix(), Z.to_fix(), fast, FFT );
}

bool ff_raycast (Q x, Q y, Q z, Q vec[3], Q range, Q where_hit[3], terrain_ff* FFT)
{
   return FF_raycast (x.to_fix(), y.to_fix(), z.to_fix(), (fix *) vec, range.to_fix(), (fix *) where_hit, FFT);
}


//	We need to link to c...
//	=======================
//extern "C" {




//	Startup the mighty and perilous EDMS engine...
//	==============================================
void EDMS_startup( EDMS_data* D )
{
//	Stoke the internals...
//	======================
	EDMS_initialize( D );

//	Get the handles in order...
//	===========================
	EDMS_init_handles();

//	Set the callbacks...
//	====================
	EDMS_object_collision	= D -> collision_callback;
	EDMS_autodestruct			= D -> autodestruct_callback;
	EDMS_off_playfield		= D -> awol_callback;
	EDMS_sleepy_snoozy		= D -> snooz_callback;

//	Done.
//	=====
	
}


//////////////////////////////
//
// Tells EDMS what space to use for A
// 
void EDMS_set_workspace (void *place)
{
   A = (EDMS_Argblock_Pointer) place;
}


//	Although this seems a very stupid way to do this, it's actually not...
//	======================================================================


//	Autodestruct gets turned on...
//	==============================
void EDMS_set_autodestruct( physics_handle ph )
{
	if (ph > -1)
   {
      int		object = ph2on[ph];
		I[object][38] = 1;
	}
}

//	Autodestruct gets turned off...
//	===============================
void EDMS_defuse_autodestruct( physics_handle ph )
{
	if (ph > -1)
   {
      int		object = ph2on[ph];
		I[object][38] = 0;
	}
}


//	I am death incarnate.  666. So there.
//	=====================================
void EDMS_kill_object( physics_handle ph )
{
	//	Are you there, really???
	//	========================
	if ( ph > -1 )
	{
		int	on = physics_handle_to_object_number( ph ),
				i;

		//	Do it, just do it...
		//	--------------------
		EDMS_kill( on );
		EDMS_release_object( ph );

		//	Simulate the action of soliton packing...
		//	-----------------------------------------
		for ( i = (on+1); (i < MAX_OBJ); i++ )
		{
			EDMS_remap_object_number( i, (i-1) );
		}

		if ( EDMS_integrating == 1 )
		{
//                mout << "Killed " << on << " while integrating!\n";

      		//	You were...
      		//	===========
     	}
	}
}


//	How you get your damn stuff out...
//	==================================
void EDMS_get_state( physics_handle ph, State *s )
{

	int on = physics_handle_to_object_number( ph );
	if ( on > -1 && on < MAX_OBJ ) {
	s->X = S[on][0][0].to_fix();		s->X_dot = S[on][0][1].to_fix();
	s->Y = S[on][1][0].to_fix();		s->Y_dot = S[on][1][1].to_fix();
	s->Z = S[on][2][0].to_fix();		s->Z_dot = S[on][2][1].to_fix();
	s->alpha = S[on][3][0].to_fix();       	s->alpha_dot = S[on][3][1].to_fix();
	s->beta  = S[on][4][0].to_fix();       	s->beta_dot  = S[on][4][1].to_fix();
	s->gamma = S[on][5][0].to_fix();       	s->gamma_dot = S[on][5][1].to_fix();
	}

#ifdef EDMS_SHIPPABLE
	else cout << "Hey, EDMS_get_state sez: physics_handle " << ph << " is nonexistant!!\n";
#endif

}




//	This does exactly what it looks like it does.  It is up to the caller to make sure
//	that the transport coordinate is not dangerous, for now.  Later, perhaps,
//	it should make sure of this itself, since this is not a real-time kind of function...
//	========================================================
void EDMS_holistic_teleport( physics_handle ph, State *s )
{
	if ( ph > -1 )
	{
		int on = physics_handle_to_object_number( ph );		//Who are youuuu...

      	//	First, get rid of the collision hash reference (in state since frame is over)...
      	//	=====================================================
     	state_delete_object( on );

      	//	Now move the thing...
      	//	=====================
	   	S[on][0][0].fix_to( s->X );		S[on][0][1].fix_to( s->X_dot ) ;
	   	S[on][1][0].fix_to( s->Y );		S[on][1][1].fix_to( s->Y_dot ) ;
	   	S[on][2][0].fix_to( s->Z );		S[on][2][1].fix_to( s->Z_dot ) ;

     	 if ( I[on][30] != D_FRAME )
     	 {     
			S[on][3][0].fix_to( s->alpha );       	S[on][3][1] = fix_zero;
			S[on][4][0].fix_to( s->beta );         S[on][4][1] = fix_zero;
			S[on][5][0].fix_to( s->gamma );       	S[on][5][1] = fix_zero;
	      }
      	else
     	 {
         		Q	alpha,
				beta,
				gamma,
				sin_alpha, cos_alpha,
				sin_beta,  cos_beta,
				sin_gamma, cos_gamma;

//       	alpha.fix_to( s -> alpha);
//       	beta.fix_to( s -> beta);
//      	 gamma.fix_to( s -> gamma);

//       	For shock...
//       	------------
	         alpha.fix_to( s -> beta);
	         beta.fix_to( s -> gamma);
	         gamma.fix_to( s -> alpha);
	
	         alpha = beta = 0;
	
	         sincos( .5*alpha, &sin_alpha, &cos_alpha );                
	         sincos( .5*beta, &sin_beta,  &cos_beta  );                
	         sincos( .5*gamma, &sin_gamma, &cos_gamma );                
	         
	         S[on][3][0] = cos_gamma*cos_alpha*cos_beta + sin_gamma*sin_alpha*sin_beta;
	         S[on][4][0] = cos_gamma*cos_alpha*sin_beta - sin_gamma*sin_alpha*cos_beta;
	         S[on][5][0] = cos_gamma*sin_alpha*cos_beta + sin_gamma*cos_alpha*sin_beta;
	         S[on][6][0] =-cos_gamma*sin_alpha*sin_beta + sin_gamma*cos_alpha*cos_beta;
	
	         S[on][3][1] = 0;                  //Derivatives
	         S[on][4][1] = 0;
	         S[on][5][1] = 0;
	         S[on][6][1] = 0;
		}

     	//	Restart collisions on it...
      	//	===========================
	   	state_write_object( on );
	  
	//	Gee, I hope that that is a good location, sunny, and free of solid objects...
	//	==================================================
	}
}





//	Here we exclude objects from hitting each specific others...
//	============================================================
void	EDMS_ignore_collisions( physics_handle ph1, physics_handle ph2 )
{
	int	on1,
			on2;

	//	Safety dance...
	//	---------------
	if ( (ph1 > -1) && (ph2 > -1) )
	{
		on1 = ph2on[ph1];
		on2 = ph2on[ph2];

		exclude_from_collisions( on1, on2 );
	}
}

//	Here we reallow collisions...
//	=============================
void	EDMS_obey_collisions( physics_handle ph1 )
{
   int	on1;

   //	Safety ballet...
   //	---------------
	if (ph1 > -1)
   {
		on1 = ph2on[ph1];
		reset_collisions( on1 );
	}
}


//      Turns collisions OFF for a given robot, useful in a variety of household chores...
//      ==================================================================================
void    EDMS_make_robot_antisocial( physics_handle ph ) {

int     on = 0;

//      Do you suck...
//      --------------
        if (ph > -1) {

                on = ph2on[ph];
                
                if ( I[on][30] == ROBOT ) I[on][5] = -1;

        }       //You suck...

}


//      Turns collisions ON for a given robot, useful in a variety of household chores...
//      =================================================================================
void    EDMS_make_robot_social( physics_handle ph ) {

int     on = 0;

//      Do you suck...
//      --------------
        if (ph > -1) {

                on = ph2on[ph];
                
                if ( I[on][30] == ROBOT ) I[on][5] = 0;

        }       //You suck...

}


// Here is a routine that will attempt to settle an object to the local b/c.  It is NOT intended for
// online use.  A negative return value indicates a badly placed or unphysical model...
// =================================================================
int EDMS_settle_object( physics_handle ph )
{
   int on = 0,
   return_value = -1;

// Are you really there?
// ---------------------
   if ( ph > -1 )
   {
      on = ph2on[ph];
      return_value = settle_object( on );
   }       //Happy joy...
               
// All done...
// -----------
   return return_value;
}



//      Prints out a state vector...
//      ============================
void    EDMS_mprint_state( physics_handle ph )
{
   int on = ph2on[ph];

   mprint_state( on );
}


//	Here is the beginning of an EDMS diagnostic statistics tool...
//	==============================================================
void	EDMS_inventory_and_statistics( int show_sleepers ) {

	inventory_and_statistics( show_sleepers );

}



//	Here is the sanity checker, but you already can read that, can't you...
//	=======================================================================
int	EDMS_sanity_check( void ) {


	return sanity_check();


}





//	Here are the bridge routines to the "default" EDMS models, others are segregates(d)...
//	======================================================================================




//	Robot routines...
//	==================


//      Hardness scale for robots...
//      ----------------------------
#define ROBOT_HARD_FAC 10


//	This guy is BROKEN FOR NOW (until we get the params finalized)...
//	------------------------------------------------
void EDMS_get_robot_parameters( physics_handle ph, Robot *m )
{
	int on = physics_handle_to_object_number( ph );

#ifdef EDMS_SHIPPABLE
   if (I[on][IDOF_MODEL] != ROBOT)
      mout << "You are trying to get ROBOT parameters for an " << I[on][IDOF_MODEL] << "!\n";
#endif

// mout << "RD: " << I[on][IDOF_ROBOT_ROLL_DRAG] << " : M:" << I[on][IDOF_ROBOT_MASS] << "\n";
	m -> pep =		(I[on][IDOF_ROBOT_ROLL_DRAG] / (1.5*I[on][IDOF_ROBOT_MASS])).to_fix();
	m -> size =		 I[on][IDOF_ROBOT_RADIUS].to_fix();
	m -> hardness = 	(I[on][IDOF_ROBOT_K]*I[on][IDOF_ROBOT_RADIUS]/(I[on][IDOF_ROBOT_MASS]*ROBOT_HARD_FAC) ).to_fix();
	m -> mass = 		 I[on][IDOF_ROBOT_MASS].to_fix();
	m -> gravity = 		 I[on][IDOF_ROBOT_GRAVITY].to_fix();
	m -> cyber_space = 	 I[on][IDOF_CYBERSPACE].to_int();
}




//	And the compression test for terrain "traps..."
//	===============================================
fix EDMS_get_robot_damage( physics_handle ph )
{
   int	object;

	object = ph2on[ph];			//As stupid as it gets...
	return (I[object][14]).to_fix();
}



//	In flux (Thrust, attitude and JumpJets)...
//	==========================================
void EDMS_control_robot( physics_handle ph, fix T, fix A, fix J )
{
   Q	TT,                              // thrust
		AA,                              // attitude jets
		JJ;                              // jump jets

#ifdef EDMS_SHIPPABLE
	if ( ph < 0 ) mout << "Hey, you are an idiot...";
#endif

	TT.fix_to( T );
	AA.fix_to( A );
	JJ.fix_to( J );

	int on = physics_handle_to_object_number( ph );
	robot_set_control( on, TT, AA, JJ );
}


//	AI control routines...
//	======================
void EDMS_ai_control_robot( physics_handle ph, fix D_H, fix D_S, fix S_S, fix U, fix *T_Y, fix D )
{
   Q	DH,                              // desired heading
		DS,                              // desired speed
		 SS,                              // sidestep
		UU,                              // urgency
		TU,                              // there yet?
		DD;                              // distance

#ifdef EDMS_SHIPPABLE
 	if ( ph < 0 ) mout << "Hey, you are and idiot...";
#endif


	DH.fix_to( D_H );
	DS.fix_to( D_S );
	SS.fix_to( S_S );
	UU.fix_to( U );
	DD.fix_to( D );

	int on = physics_handle_to_object_number( ph );
	robot_set_ai_control( on, DH, DS, SS, UU, TU, DD );

	*T_Y = TU.to_fix();
}




//	These are different parameters than for the marble now...
//	---------------------------------------------------------
physics_handle EDMS_make_robot( Robot *m, State *s )
{
	Q	params[10],
    		init_state[6][3];
	
	Q	mass,
		pep,
		hardness,
		size,
		gravity;

	int cyber_space;

	int on = 0;
	physics_handle	ph = 0;

	init_state[DOF_X][0].fix_to(s->X);		    init_state[DOF_X][1].fix_to( s->X_dot );
	init_state[DOF_Y][0].fix_to(s->Y);		    init_state[DOF_Y][1].fix_to( s->Y_dot );    
	init_state[DOF_Z][0].fix_to(s->Z);		    init_state[DOF_Z][1].fix_to( s->Z_dot );    
	init_state[DOF_ALPHA][0].fix_to(s->alpha); init_state[DOF_ALPHA][1].fix_to( s->alpha_dot );
	init_state[DOF_BETA][0] =			          init_state[DOF_BETA][1] = 
	init_state[DOF_GAMMA][0] = 				    init_state[DOF_GAMMA][1] = END;

	mass.fix_to( m -> mass );
	size.fix_to( m -> size );
//	if ( size > .45/hash_scale ) size = .45/hash_scale;
	hardness.fix_to( m -> hardness );
	pep.fix_to( m -> pep );
	gravity.fix_to( m -> gravity );
	cyber_space = m -> cyber_space;

//        if (hardness > 15) { mout << "Hardness too too too: " << hardness << "\n"; hardness = 15; }

        if ( mass < 1 ) mass = 1;
        if ( mass > 30 ) mass = 30;

	hardness = hardness*(mass*ROBOT_HARD_FAC/size);
        if (hardness > 4000) { hardness = 4000;
//                               mout << "Hard cap!\n";
                             }

	params[OFFSET(IDOF_ROBOT_K)]          = hardness;
	params[OFFSET(IDOF_ROBOT_D)]          = 1.5*sqrt( params[OFFSET(IDOF_ROBOT_K)] ) * sqrt( mass );
	params[OFFSET(IDOF_ROBOT_RADIUS)]     = size;

//        mout << params[OFFSET(IDOF_ROBOT_D)] << "\n";
        
        params[OFFSET(IDOF_ROBOT_ROLL_DRAG)]  = 1.5 * pep * mass;
	params[OFFSET(IDOF_ROBOT_MASS_RECIP)] = 1. / mass;
	params[OFFSET(IDOF_ROBOT_GRAVITY)]    = gravity;
	params[OFFSET(IDOF_ROBOT_MASS)]       = mass;
//	params[7] = 1. / ( .4*mass*size*size );
//	params[8] = 5.*(1. / params[7]);
//	params[9] = .4*mass*size*size;
        params[OFFSET(IDOF_ROBOT_MOI)]         = .4*mass*size*size;
        params[OFFSET(IDOF_ROBOT_ROT_DRAG)]    = 5*params[OFFSET(IDOF_ROBOT_MOI)];

        if (params[OFFSET(IDOF_ROBOT_MOI)]!=0)

      params[OFFSET(IDOF_ROBOT_MOI_RECIP)]= 1.0/params[OFFSET(IDOF_ROBOT_MOI)];
   else
      params[OFFSET(IDOF_ROBOT_MOI_RECIP)]= 0.0;


	on = make_robot( init_state, params );

// Here is where cyberspace gets turned on...
//	------------------------------------------
   I[on][IDOF_CYBERSPACE] = (cyber_space>0);		
		
	ph = EDMS_bind_object_number( on );


#ifdef EDMS_SHIPPABLE
   if (params[OFFSET(IDOF_ROBOT_MOI)]==0) mout << "object " << on << " got 0 size or mass\n";
#endif

	return ph;
}


void EDMS_set_robot_parameters( physics_handle ph, Robot *m )
{
   Q	mass,
	   hardness,
	   size,
	   pep,
	   gravity;

   int cyber_space;

	mass.fix_to( m -> mass );
	size.fix_to( m -> size );
//	if ( size > .45/hash_scale ) size = .45/hash_scale;
	hardness.fix_to( m -> hardness );
	pep.fix_to( m -> pep );
	gravity.fix_to( m -> gravity );
	cyber_space = m -> cyber_space;

	int on = physics_handle_to_object_number( ph );

#ifdef EDMS_SHIPPABLE
	if (I[on][IDOF_MODEL] != ROBOT) mout << "You are trying to set ROBOT parameters for an " << I[on][30] << "!\n";
#endif

//	mout << "Set Robot " << on << "\n";
//	mout << "	mass: " << mass << "\n";
//	mout << "	size: " << size << "\n";
//	mout << "	hard: " << hardness << "\n";
//	mout << "	pepp: " << pep << "\n";
//	mout << "	grav: " << gravity << "\n";

        if ( mass < 1 ) mass = 1;
        if ( mass > 30 ) mass = 30;

	hardness = hardness*(mass*ROBOT_HARD_FAC/size);

//	hardness = hardness*(mass*ROBOT_HARD_FAC/size);
        if (hardness > 4000) { hardness = 4000;
//                               mout << "Hard cap!\n";
                             }
        
        I[on][IDOF_ROBOT_K] = hardness;
	I[on][IDOF_ROBOT_D] = 1.5*sqrt( I[on][IDOF_ROBOT_K] ) * sqrt( mass );
	I[on][IDOF_ROBOT_RADIUS] = size;
	I[on][IDOF_ROBOT_ROLL_DRAG] = 1.5 * pep * mass;
	I[on][IDOF_ROBOT_MASS_RECIP] = 1. / mass;
	I[on][IDOF_ROBOT_GRAVITY] = gravity;
	I[on][IDOF_ROBOT_MASS] = mass;
	I[on][IDOF_ROBOT_MOI_RECIP] = 1. / ( .4*mass*size*size );
	I[on][IDOF_ROBOT_ROT_DRAG] = 5.*(1. / I[on][IDOF_ROBOT_MOI_RECIP]);
	I[on][IDOF_ROBOT_MOI] = .4*mass*size*size;
        I[on][IDOF_CYBERSPACE] = (cyber_space>0);

//        mout << "Roll: " << I[on][IDOF_ROBOT_ROLL_DRAG] << "\n";
}





//	Bridge routines to the solvers!!
//	================================

//	4th order and very stable...
//	----------------------------
void EDMS_soliton( fix timestep )
{
	Q temp;
	temp.fix_to( timestep );
	soliton( temp );
}

//	2nd order and needs some attention...
//	-------------------------------------
void EDMS_soliton_lite( fix timestep )
{
	Q temp;
	temp.fix_to( timestep );
	soliton_lite( temp );
}

//	Efficient and unstoppable...
//	----------------------------
void EDMS_soliton_vector( fix timestep )
{
	Q temp;
	temp.fix_to( timestep );
	soliton_vector( temp );
}

//	Won't allow objects to collide w/one another...
//	-----------------------------------------------
void EDMS_soliton_vector_holistic( fix timestep )
{
	Q temp;
	temp.fix_to( timestep );
	soliton_vector_holistic( temp );
}






//	This code here handles the mapping between the user's physics handles and the
//	dynamically changing intername object numbers.
//	==============================================
// 	To make a physics handle, edms calls EDMS_bind_object_number to "bind" an internal
// 	object number to a physics handle.  When this object number changes, the function
//	EDMS_remap_object_number() needs to be called to map the nu object number to
//	the physics number.
//	===================
// 	The physics handles are valid throughout the life of a physics object; in contrast,
//	the object numbers change as the array of objects is compacted.
//	===============================================================	
// 	Make sure the include stuff is included before this.
//	====================================================



//	Initialization... must call this, man...
//	========================================
void EDMS_init_handles( void )
{
   int i;

//	Fill with the 'end' code...
//	---------------------------
	for (i=0; i<MAX_OBJ; ++i) ph2on[i] = on2ph[i] = -1;
}


//	To bind a physics handle to an object number, use this function.
//	================================================================
physics_handle EDMS_bind_object_number( object_number on )
{
   physics_handle	ph = EDMS_get_free_ph();

	if (ph == -1) return -1;

	ph2on[ph] = on;
	on2ph[on] = ph;

	return ph;
}







//	EDMS_remap_object_number() remaps object numbers so that the physics_handle that
//	used to refer to object number #old will now refer to object number #nu.
//	If object #old is not mapped, nothing happens...
//	================================================

void EDMS_remap_object_number( object_number old, object_number nu )
{
   physics_handle	ph = on2ph[old];

	if (ph == -1) return;

	on2ph[nu]  =  ph;
	on2ph[old] = -1;
	ph2on[ph]  =  nu;
}







//	To release the mapping between physics handle and object number
//	(say, when deleting an object)...
//	=================================

void EDMS_release_object( physics_handle ph )
{
   object_number	on = ph2on[ph];


//	First, clear out the object number...
//	-------------------------------------
	on2ph[on] = -1;

//	Then clear out the physics_handle...
//	------------------------------------
	ph2on[ph] = -1;

}





//	Internal routine to find an unused physics_handle.
//	==================================================

physics_handle EDMS_get_free_ph( void )
{
   int	i;


	for ( i = min_physics_handle; i<MAX_OBJ; ++i ) 
   {
//		Ho, ho...
//		---------
		if (ph2on[i] == -1) return i;
   }

//	Failed to find one...
//	---------------------
	return -1;

//	Fun...
//	======
}





//}	//End of extern "C" for the &^%$@% compiler...

