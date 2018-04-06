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
//	Here is a box of tools for making physics easier for the programmer...
//	===================================================


//	Well, aren't we snooty...
//	=========================


//	Seamus, Nov 9 1993...
//	=====================



//	Get on with it then...
//	======================
#include <iostream.h>
#include "EDMS_Int.h"				//Object types, END conventions, etc.
#include "idof.h"

#include "physhand.h"


//extern "C" {					//Debugging spew...
//}


//	==============
//	INTERNAL STUFF
//	==============

void snobby_soliton_lite( Q timestep, int object );

extern void	( *idof_functions[MAX_OBJ] )( int ),		//Pointers to the appropriate places...
		( *equation_of_motion[MAX_OBJ][7] )( int );	//The integer is the object number...


//	Courtesy of C++ and inline fixpoint and such...
//	===============================================
static Q	one_sixth = .1666666666667,			//Overboard?
		point_five = .5,
		point_one_two_five = .125,
		two = 2.,
		point_1 = 0.1;

//	Here is a routine that should help in the placement of EDMS objects.  3D+ only!
//	===============================================================================
int settle_object( int object )
{
   int	return_value = -1;					//Failure...

   Q	nrg = 1000.,
   	nrg_min = .1,
   	pass_time = .01;					//Not meta-stable!

   int	count = 0,
	max_count = 1000;
   	
   //	First, are you for real?
   //	------------------------
	if ( S[object][0][0] > END )
   {
      //	Ok, now let's actually DO the settling...
      //	=========================================
    	while ( ( nrg > nrg_min ) && ( count < max_count ) )
      {

      	snobby_soliton_lite( pass_time, object );

      	nrg = S[object][DOF_X][1]*S[object][DOF_X][1]
      	    + S[object][DOF_Y][1]*S[object][DOF_Y][1]			//Too dangerous for assumptions...
             + S[object][DOF_Z][1]*S[object][DOF_Z][1];			//Forget the others...

      	count += 1;
   	}	

      //	Did we diverge?
      //	---------------
     	if ( count < max_count ) return_value = 1;		//Success! 

//	Yes, you were for real...
//	-------------------------
	}

//	All done...
//	===========
	return return_value;
}


//	Here is a version of soliton_lite which runs on only one object.  This should be useful for
//	settling objects at level starts, placing things exactly on the ground, etc...
//	============================================================================== 
void snobby_soliton_lite( Q timestep, int object )
{
	int	coord = 0;

	//	Copy the state vector initially into the argument vector...
	//	===========================================================
	state_delete_object( object ); 					//Do collisions, let this guy go free for a while...

	for ( coord = 0; coord < DOF && S[object][coord][0] > END;  coord++ )
	{
		A[object][coord][0] = S[object][coord][0];
		A[object][coord][1] = S[object][coord][1];
	}


   //	Here is the leading (order zero) term...
   //	========================================
	( *idof_functions[object] )( object );

	for ( coord = 0; coord < DOF && S[object][coord][0] > END;  coord++ )
	{
		k[0][object][coord] = timestep*S[object][coord][2];
	}

   //	Here is a frobbed term...
   //	-------------------------
	for ( coord = 0; coord < DOF && S[object][coord][0] > END;  coord++ )
	{
		A[object][coord][0] = S[object][coord][0] + timestep*S[object][coord][1];
		A[object][coord][1] = S[object][coord][1] + k[0][object][coord];
   }

	( *idof_functions[object] )( object );

	for ( coord = 0; coord < DOF && S[object][coord][0] > END;  coord++ )
	{
		k[1][object][coord] = timestep*S[object][coord][2];
	}


   //	Hey! We're already able to assemble the solution!  Wasn't that better than soliton?
   //	===--------=======-----------------------------------------------------------------
	for ( coord = 0; coord < DOF && S[object][coord][0] > END;  coord++ )
	{
      S[object][coord][0] = S[object][coord][0] + timestep*S[object][coord][1]
					               + point_five*( timestep*timestep*k[0][object][coord] );
                              
      S[object][coord][1] = S[object][coord][1] + point_five*( k[0][object][coord]
					               + k[1][object][coord] );

	}

	state_write_object( object );   //Put it back in. Note that IT saw EVERYONE else, but they only see IT now!
}


//      Here is a tool that Marc LeBlanc has requested...
//      =================================================
void    mprint_state( int /*object*/ )
{
/*
   int coord = 0,
       deriv = 0;

   if ( S[object][0][0] > END )          //Are you for real?
   {

      for ( coord = 0; (coord < DOF) && (S[object][coord][0] > END); coord++ )
      {
         for ( deriv = 0; deriv < 3; deriv++ )
         {
         }
      }
   }
*/
}


//	Here is an inventory of everything in the system...
//	===================================================
void	inventory_and_statistics( int show_sleepers )
{
   int	object = 0;

	for ( object = 0; object < MAX_OBJ && S[object][0][0] > END;  object++ )
   {

      if ( (no_no_not_me[object] == 1) || show_sleepers == 1 )
      {

#ifdef EDMS_SHIPPABLE

//   		mout << object << ".) ";

//   		mout << "Physics handle: " << on2ph[object] << " is a " << I[object][IDOF_MODEL]
//   		     << " at   X:" << S[object][DOF_X][0] << "   Y:" << S[object][DOF_Y][0] << "   Z:" << S[object][DOF_Z][0]
//   		     << " Sleep: ";
                
//         if ( no_no_not_me[object] == 0 ) mout << "Y";
//         else mout << "N";

//         mout << "\n";

#endif

      }
	} // End of sleeper check...

//#ifdef EDMS_SHIPPABLE
//	mout << "There are " << object << " objects currently running.\n";
//#endif
}


//	This is EDMS' sanity checker.  Call it to see what's wrong.  Problems
//	will return a nonzero result...
//	===============================
int	sanity_check( void )
{
   //	The idof functions...
   //	=====================
   // extern void	biped_idof( int ),
   //		marble_idof( int ),
   //		robot_idof( int ),
   //		pelvis_idof( int ),
   //		deadly_idof( int );


   //		Have some variables...
   //		----------------------

   int object = 0,
		 coord = 0,
		 dof = 0,
		 object_number = 0;

   physics_handle	ph = 0;


//		Finally, EDMS error codes, included here only, see EDMS.h
//		=========================================================
#define		EDMS_TOO_MANY_OBJECTS 1
#define		EDMS_PHYSICS_HANDLES_CORRUPT 2
#define		EDMS_OBJECT_HANDLES_CORRUPT 3
#define		EDMS_IDOF_POINTERS_CORRUPT 4


//		Here's the return value...
//		--------------------------
int		return_value = 0;		//Innocent until...




//	First check the number of active objects...
//	===========================================
	for ( object = 0; S[object][0][0] > END; object++ );
	if ( object > MAX_OBJ )
   {
   	return_value = EDMS_TOO_MANY_OBJECTS;

//#ifdef EDMS_SHIPPABLE
//		mout << "EDMS_sanity_check: too many objects (" << object << ")!\n";
//#endif
   }


//	Now check the physics handle mappings...
//	========================================
	for ( object = 0; S[object][0][0] > END; object++ )
   {
   	if ( ph2on[ (on2ph[object]) ] != object )
	   {
			return_value = EDMS_PHYSICS_HANDLES_CORRUPT;
//#ifdef EDMS_SHIPPABLE
//			mout << "EDMS_sanity_check: physics handles corrupted (object " << object << ")!\n";
//#endif
		}
	}

//	Now the object handle mappings...
//	=================================
	for ( ph = 0; ph < MAX_OBJ; ph++ )
   {
		if ( ph2on[ph] > 0 && ph2on[ph] < MAX_OBJ )
      {
   		if ( on2ph[ (ph2on[ph]) ] != ph )
         {
				return_value = EDMS_OBJECT_HANDLES_CORRUPT;

//#ifdef EDMS_SHIPPABLE
//				mout << "EDMS_sanity_check: object handles corrupted (handle " << ph << ")!\n";
//#endif
			}
		}
	}

//	Now we see if the idof pointers are corrupt...
//	==============================================
//	for ( object = 0; S[object][0][0] > END; object++ ) {
//	    if (	idof_functions[object] != biped_idof
//		&&	idof_functions[object] != marble_idof
//		&&	idof_functions[object] != robot_idof
//		&&	idof_functions[object] != pelvis_idof
//		&&	idof_functions[object] != deadly_idof
//	       ) {
//	       return_value = EDMS_IDOF_POINTERS_CORRUPT;
//
//#ifdef EDMS_SHIPPABLE
//	       mout << "EDMS-sanity_check: idof pointers corrupt (object: " << object << " with ph: " << on2ph[object] << ")!\n";
//#endif
//
//	       }
//	}



//	Und das ist alles...
//	====================
	return return_value;
}








//	Now that EDMS is no longer using Euler angles to represent object orientation,
//	and since space is really 6D + a constraint, and all, and because SU(2) is
//	rilly rilly my best friend, and because other people rilly rilly love
//	Euler angles, and because using this routine will give them those, and
//	because even though the models won't blow up at alpha=2pi this conversion
//	becomes degenerate in roll and heading (beta and gamma) and such...
//	===================================================================


//	Seamus says:  Use my matrix whenever you can, this conversion incurrs many
//	of the same problems that using Euler angles to begin with causes.  Come
//	talk to me about how YOUR game could benefit from Dirac spinors today!
//	----------------------------------------------------------------------


//	Get the Euler angles we need from the stuff in the state...
//	===========================================================
void	EDMS_get_Euler_angles( Q &alpha, Q &beta, Q &gamma, int object )
{
   Q	e0, e1, e2, e3;

	e0 =  S[object][DOF_ALPHA][0];	e1 =  S[object][DOF_BETA][0];
	e2 =  S[object][DOF_GAMMA][0];	e3 =  S[object][DOF_DIRAC][0];


//      Get the trig information we need...
//      ===================================
   alpha = asin( 2*(e0*e2 - e1*e3) );
   Q	cos_alpha = cos( alpha );

#define EDMS_EULER_CONVERSION_TRIG_ZERO .0001

   if (cos_alpha > 0 && cos_alpha < EDMS_EULER_CONVERSION_TRIG_ZERO ) cos_alpha = EDMS_EULER_CONVERSION_TRIG_ZERO;
   if (cos_alpha < 0 && cos_alpha >-EDMS_EULER_CONVERSION_TRIG_ZERO ) cos_alpha =-EDMS_EULER_CONVERSION_TRIG_ZERO;

   gamma =  acos( (e0*e0 + e1*e1 - e2*e2 - e3*e3) / cos_alpha );
   if ( (e1*e2 + e0*e3) < 0) gamma *= -1;               //sgn...

   beta  =  acos( (e0*e0 - e1*e1 - e2*e2 + e3*e3) / cos_alpha );
   if (e2*e3 + e0*e1 < 0) beta *= -1;

   alpha *= -1;
   beta *= -1;
   gamma *= -1;
}




//extern "C"
//{

#pragma require_prototypes off

//      Call this to see if an object is asleep...
//      ==========================================
bool    EDMS_frere_jaques( physics_handle ph )
{
   bool    rval = FALSE;

// condomate...
// ------------
   if ( ph > -1 )
   {
      if ( no_no_not_me[ ph2on[ph] ] == 1 ) rval = TRUE;
   }

   return rval;
}



//      Call this to wake an object up...
//      =================================
void    EDMS_crystal_meth( physics_handle ph )
{

   if ( ph > -1 )
   {
      no_no_not_me[ ph2on[ph] ] = 1;
   }


}

#pragma require_prototypes on

//}       //End of "Extern "C""...
