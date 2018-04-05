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
 * $Header: n:/project/lib/src/edms/RCS/soliton.cc 1.6 1994/04/20 18:44:56 roadkill Exp $
 */

//	Soliton.CPP - the core of EDMS, the Emetic Dynamics Modeling System (tm).

//	"Real time stiff equations are such a joy that hey, let's solve whole systems of them
//	at once."  -Jon Blackley, prior to choking self on mousepad.

//	Soliton and soliton_lite are heavily modified, tuned, and stoked versions of the previous
//	solvers, which are based on MASA internal memo #AF34-DE-3030 1991.  It has been modified to
//	be faster and better suited to stiff and nonlinear problems.  Nota Bene: There IS NO
//	SOLUTION ORDER.	Which means that all system coordinates are treated as quasi independent
//	degrees of freedom of the same well posed problem and are thus approximated simultaneously.
//	=====================================================================

//	Who is responsible:
//	===================
//	Jon Blackley, Oct. 25, 1991


//#include <iostream.h>
#include "EDMS_int.h"				//Object types, END conventions, etc.

#include "physhand.h"
#include "lg.h"

//#include <mout.h>

// For convenience
#define HashSpew(a) Spewpp( DSRC_EDMS_Hash, a )

Q     S[MAX_OBJ][7][4];    //State stream... Accessable to all...

extern void EDMS_kill_object( physics_handle ph );



//	==============
//	INTERNAL STUFF
//	==============

// Why does this get set to 100 and then immediately reset to .02
// in soliton_lite? - DS

//extern "C" {
Q       snooz_threshold = 100;
//}

int     EDMS_integrating = 0;

EDMS_Argblock_Pointer A;            //non-vector type arguments for perturbation...


Q	I[MAX_OBJ][DOF_MAX],          //Internal degrees of freedom...
	k[4][MAX_OBJ][7];             	//expansion coefficients...

void  ( *idof_functions[MAX_OBJ] )( int ),         //Pointers to the appropriate places...
      ( *equation_of_motion[MAX_OBJ][7] )( int );  //The integer is the object number...


Q     *utility_pointer[MAX_OBJ];       //Biped skeletons, Jello translucencies, etc...    


Q     hash_scale = 1.;        //The ratio betwixt coordinate and collision...

// Courtesy of C++ and inline fixpoint and such...
// ===============================================
const Q	one_sixth = .1666666666667,      //Overboard?
			point_five = .5,
			point_one_two_five = .125,
			two = 2.,
			point_1 = 0.1,
			min_scale_slice = .03;	// ееее╩.03

//      Sleeping...
//      -----------
int      no_no_not_me[MAX_OBJ];
int      alarm_clock[MAX_OBJ];
int      industrial_strength[MAX_OBJ];

//      *******************HACK*HACK*HACK*HACK*****************************+
//      ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// uchar   EDMS_Seamus_is_an_asshole[12000];
//      *******************HACK*HACK*HACK*HACK*****************************+
//      ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void soliton( Q /*timestep*/ ) {}


// Soliton_Lite (tm)...
// ====================

int    active_objects = 0;

// Are non-sleeping objects in the middle of doing their integrating thing,
// and thus using A[][][] instead of S[][][]?
bool   A_is_active = FALSE;

//	Here is a slower converging, albeit hopefully equally stable integrator, soliton_light, which
//	we'll subject to some speed trials.  This could be used for certain applications, such as
//	out-of scope models or complex aggregate or articulated objects.  Well, let's see...
//	====================================================================================
void soliton_lite( Q timestep )
{
   extern void     robot_idof( int ),
                   pelvis_idof( int );

register int   object = 0;
register int   coord = 0;
register	Q	*S_Object;

   Q       frequency_check;
   Q       average_frequency;
   int     ccount = 0;
   int     count = 0;

   // Copy the state vector initially into the argument vector...
   // ===========================================================
   for ( object = 0; S[object][0][0] > END; object++ )
   {
      if (no_no_not_me[object] == 1)
      {
      	S_Object = (Q *) S[object];
      	      	
         state_delete_object( object );
         for ( coord = 0; coord < 7 && S_Object[coord<<2] > END;  coord++ )
         {
            A[object][coord][0].val = S_Object[coord<<2].val;
            A[object][coord][1].val = S_Object[(coord<<2)+1].val;
         }
         state_write_object( object );                 //Here to initialize new models for sure...
      }
   }

   // Here is the leading (order zero) term...
   // ========================================
   count = 0;
   average_frequency = 0;

   for ( object = 0; S[object][0][0] > END; object++ )
   {
      if (no_no_not_me[object] == 1)
      {
      	S_Object = (Q *) S[object];

         // Are we wasting time...
         // ----------------------
         frequency_check = 0;

//       mout << "II0\n";
         ( *idof_functions[object] )( object );

         for ( coord = 0; coord < 7 && S_Object[coord<<2] > END;  coord++ )
         {
            k[0][object][coord].val = fix_mul(timestep.val,S_Object[(coord<<2)+2].val);
            if ( abs(S_Object[(coord<<2)+2]) > .001)
            {
               // This check makes the function discontinuous (it goes from 10000 down
               // to 1000 when S[object][coord][2] reaches 100)... - DS
               if ( abs(S_Object[(coord<<2)+2]) < 100 )
                  frequency_check.val += fix_mul( S_Object[(coord<<2)+2].val,S_Object[(coord<<2)+2].val );
               else
                  frequency_check.val += fix_make(1000,0);
            }

            if ( abs(S_Object[(coord<<2)+1]) > .001)
            {
               if ( abs(S_Object[(coord<<2)+1]) < 50  )
                  frequency_check.val += fix_div(fix_mul( S_Object[(coord<<2)+1].val,S_Object[(coord<<2)+1].val),I[object][31].val);
               else
                  frequency_check.val += fix_make(1000,0);
            }    

            // This is angular velocity I think - DS
            if ( (I[object][30] == ROBOT) && (coord == 3) )
               frequency_check += abs(50*S_Object[(coord<<2)+2]);
         }
               
         // Are you in stiff and in need of invariant imbedding?
         // ----------------------------------------------------
         industrial_strength[object] = 0;          //Guilty until...
         if ( frequency_check > 15 )
         { 
//          mout << "-";
            industrial_strength[object] = 1;
         }

         count += 1;
         average_frequency += frequency_check;

//       mout << "f: " << frequency_check << "\n";

         if ( frequency_check < snooz_threshold )
         {
            EDMS_sleepy_snoozy( on2ph[object] );
            no_no_not_me[object] = 0;
            if ( I[object][30] == PELVIS )  no_no_not_me[object] = 1;   //Hack for now...
            if ( I[object][30] == BIPED )   no_no_not_me[object] = 1;
            if ( I[object][30] == D_FRAME ) no_no_not_me[object] = 1;
//          if (no_no_not_me[object] == 0) { mout << "!EDMS: sleeping f = " << frequency_check << "\n";}
         }
      }       //End of object sleep test...
   }       //End of object...

   // Sleeping...
   // -----------
   snooz_threshold  = .2;

   // Here is a frobbed term...
   // -------------------------

// mout << "frob...\n";

   for ( object = 0; S[object][0][0] > END; object++ )
   {
      if (no_no_not_me[object] == 1)
      {
      	S_Object = (Q *) S[object];

         state_delete_object( object );              //Do collisions...      
         for ( coord = 0; coord < 7 && S_Object[(coord<<2)+0] > END;  coord++ )
         {
            A[object][coord][0].val += fix_mul(timestep.val,S_Object[(coord<<2)+1].val);
            A[object][coord][1].val += k[0][object][coord].val;
         }
         write_object( object );                //Do collisions...      
      }
   }

   A_is_active = TRUE;                 // A is where object's real locations are

// mout << "NextStep...\n";

   // Now for the more complicated step...
   // ====================================
   for ( object = 0; S[object][0][0] > END; object++ )
   {
      if (no_no_not_me[object] == 1)
      {
      	S_Object = (Q *) S[object];

         // If we've got a hot one, EDMS now becomes industrial strength (note collisions set above)...
         // -------------------------------------------------------------------------------------------
         if (industrial_strength[object] == 1)
         {
//          mout << "IT:";

            // First order...
            // --------------
            delete_object( object );
            for ( coord = 0; coord < 7 && S_Object[(coord<<2)+0] > END;  coord++ )
            {
               A[object][coord][0].val = S_Object[(coord<<2)+0].val
                                     + fix_mul(fix_mul(point_five.val,timestep.val),S_Object[(coord<<2)+1].val)
                                     + fix_mul(fix_mul(point_one_two_five.val,timestep.val),k[0][object][coord].val);
               A[object][coord][1].val = S_Object[(coord<<2)+1].val
                                     + fix_mul(point_five.val,k[0][object][coord].val);
            }
            write_object( object );
      
//          mout << "II1\n";

            ( *idof_functions[object] )( object );

            for ( coord = 0; coord < 7 && S_Object[(coord<<2)+0] > END;  coord++ )
            {
               k[1][object][coord].val = fix_mul(timestep.val,S_Object[(coord<<2)+2].val);
            }


            // Second order...
            // ---------------
            delete_object( object );      
            for ( coord = 0; coord < 7 && S_Object[(coord<<2)+0] > END;  coord++ )
            {
               A[object][coord][0].val = S_Object[(coord<<2)+0].val
                                     + fix_mul(fix_mul(point_five.val,timestep.val),S_Object[(coord<<2)+1].val)
                                     + fix_mul(fix_mul(point_one_two_five.val,timestep.val),k[1][object][coord].val);
               A[object][coord][1] .val= S_Object[(coord<<2)+1].val
                                     + fix_mul(point_five.val,k[1][object][coord].val);
            }
            write_object( object );

//          mout << "II2\n";
            ( *idof_functions[object] )( object );

            for ( coord = 0; coord < 7 && S_Object[(coord<<2)+0] > END;  coord++ )
            {
               k[2][object][coord].val = fix_mul(timestep.val,S_Object[(coord<<2)+2].val);
            }


            // Third order...
            // --------------
            delete_object( object );
            for ( coord = 0; coord < 7 && S_Object[(coord<<2)+0] > END;  coord++ )
            {
               // Different convergence requirement from other terms!
               A[object][coord][0].val = S_Object[(coord<<2)+0].val
                                     + fix_mul(timestep.val,S_Object[(coord<<2)+1].val)
                                     + fix_mul(fix_mul(point_five.val,timestep.val),k[2][object][coord].val);
               A[object][coord][1].val = S_Object[(coord<<2)+1].val
                                    + k[2][object][coord].val;
            }
            write_object( object );

//          mout << "II3\n";
            ( *idof_functions[object] )( object );

            for ( coord = 0; coord < 7 && S_Object[(coord<<2)+0] > END;  coord++ )
            {
               k[3][object][coord].val = fix_mul(timestep.val,S_Object[(coord<<2)+2].val);
            }
         }       //End of stoked...
         else
         {
            // Regular strength...
            // ===================
            ( *idof_functions[object] )( object );                    

            for ( coord = 0; coord < 7 && S_Object[(coord<<2)+0] > END;  coord++ )
            {
               k[1][object][coord].val = fix_mul(timestep.val,S_Object[(coord<<2)+2].val);
            }       
         }       //End of else for regular guy...
   
         // Does anybody need to wake up?
         // -----------------------------
         for (coord = 0; coord < MAX_OBJ && S[coord][0][0] > END; coord++ )
         if ( alarm_clock[coord] != 0 )
         {
//          mout << "Alarm2: " << coord << "\n";
            alarm_clock[coord] = 0;
            collision_wakeup( coord );
         }
      }       //End of object...
   }


int     total = 0;

   // Hey! We're already able to assemble the solution!  Wasn't that better than soliton?
   // ===-----======-------------------------------------------------
   for ( object = 0; S[object][0][0] > END; object++ )
   {
      if (no_no_not_me[object] == 1)
      {
                total += 1;
         Q anus[7];

      	S_Object = (Q *) S[object];

         // Calculate the multiplier...
         // ---------------------------
         if ( I[object][30] == D_FRAME )
         {
            Q lagrange_multiplier = .5/timestep;
		   Q l_m;
		   Q lagrange;
		   
            lagrange.val = (1 << 16) - (fix_mul(S_Object[(3<<2)+0].val,S_Object[(3<<2)+0].val)
			                         + fix_mul(S_Object[(4<<2)+0].val,S_Object[(4<<2)+0].val)
			                         +fix_mul(S_Object[(5<<2)+0].val,S_Object[(5<<2)+0].val)
			                         + fix_mul(S_Object[(6<<2)+0].val,S_Object[(6<<2)+0].val));
 		   l_m.val = fix_mul(lagrange_multiplier.val,lagrange.val);
            anus[0].val = anus[1].val = anus[2].val = 0;
            anus[3].val = fix_mul(S_Object[(3<<2)+0].val,l_m.val);
            anus[4].val = fix_mul(S_Object[(4<<2)+0].val,l_m.val);
            anus[5].val = fix_mul(S_Object[(5<<2)+0].val,l_m.val);
            anus[6].val =fix_mul( S_Object[(6<<2)+0].val,l_m.val);
         }       //End of calculation...

         // Lupe over coordinates...
         // ------------------------
         for ( coord = 0; coord < 7 && S_Object[(coord<<2)+0] > END;  coord++ )
         {
            if ( industrial_strength[object] == 1 )
            {
               // These guys need more multiplies...
               // ----------------------------------
//             mout << "IS:";

               // Check for Dirac...
               // ------------------                
               if ( I[object][30] == D_FRAME )
               {
//                mout << "D";

                  S_Object[(coord<<2)+0].val += fix_mul( timestep.val, (S_Object[(coord<<2)+1].val + anus[coord].val 
                                                     + fix_mul(one_sixth.val,(k[0][object][coord].val
                                                     + k[1][object][coord].val
                                                     + k[2][object][coord].val ))));
               }
               else
               {
//                mout << "N";

                  S_Object[(coord<<2)+0].val += fix_mul(timestep.val , (S_Object[(coord<<2)+1].val
                                                     + fix_mul(one_sixth.val,( k[0][object][coord].val
                                                     + k[1][object][coord].val
                                                     + k[2][object][coord].val ))));
               }                          

               S_Object[(coord<<2)+1].val += fix_mul(one_sixth.val , (k[0][object][coord].val
                                                   + fix_mul(two.val,(k[1][object][coord].val + k[2][object][coord].val)
                                                   + k[3][object][coord].val)));
            }
            else
            {
               // These guys don't...
               // ===================
//             mout << "RS:";

               // Use the multiplier...
               // ---------------------
               if ( I[object][30] == D_FRAME )
               {
                  S_Object[(coord<<2)+0].val = S_Object[(coord<<2)+0].val + fix_mul(timestep.val,( S_Object[(coord<<2)+1].val + anus[coord].val ))
                                        + fix_mul(point_five.val,(fix_mul(fix_mul(timestep.val,timestep.val),k[0][object][coord].val)));
               }
               else
               {
                  S_Object[(coord<<2)+0].val += fix_mul(timestep.val,( S_Object[(coord<<2)+1].val + fix_mul(fix_mul(point_five.val,timestep.val),k[0][object][coord].val)));
               }

               S_Object[(coord<<2)+1].val += fix_mul(point_five.val,( k[0][object][coord].val + k[1][object][coord].val ));
            }    //End of else for regular guys...
         }

         // Update the collision table...
         // -----------------------------
         delete_object(object);
         state_write_object(object);

         if ( I[object][38] < 0 )
            EDMS_kill_object( on2ph[object] );    //You deserve to die!
      }
   }
   A_is_active = FALSE;
}



// Soliton_Vector...
// =================

// Here is soliton_vector, which is a scalable convergence integrator.  In concert with the
// other members of the EDMS integration team, soliton_vector can dramatically increase the
// efficiency of the integration step, while remaining very stable.  Details about its use
// will follow when it proves useful...
// ====================================
void soliton_vector( Q timestep )
{
   // Here i yam...
   // -------------
   EDMS_integrating = 1;

// timestep = .03;

	int     count = 0;

// for (int test = 0; test < 12000; test++) EDMS_Seamus_is_an_asshole[test] = 243643;

   // Stupid scaling for now, for FF prototype which is very SLOW!
   // -----------------------------------------------------------
   while ( timestep > min_scale_slice )
   {
      count++;
      soliton_lite( min_scale_slice ); 
      timestep -= min_scale_slice;
   }


   // Now do the rest...
   // ==================
   if ( timestep > .01 || count == 0) soliton_lite( timestep );  
// else mout << "!EDMS: timestep is small, dt = " << timestep << "\n";

// mout << count << "\n";      

   // Here i yam...
   // -------------
   EDMS_integrating = 0;
}



// Here is an integrator that fools the models into not colliding.  Hopefully not often used.
// It is a version of Soliton Lite(tm)...
// ======================================
void soliton_lite_holistic( Q /*timestep*/ ) {}


// Here is the holistic vector integrator...
// =========================================
void soliton_vector_holistic( Q /*timestep*/ ) {}






//	Have some utility routines...
//	=============================



//	Initialize the state stream and do whatever else deems itself to be essential to
//	get soliton running...
//	======================
void EDMS_initialize( EDMS_data *D )
{
   extern unsigned int  data[ EDMS_DATA_SIZE ][ EDMS_DATA_SIZE ];
   int         object = 0,
               coord = 0,
               deriv = 0;
   const Q        collision_size = EDMS_DATA_SIZE;


   // Set the starting physics_handle...
   // ==================================
   min_physics_handle = D -> min_physics_handle;


   // Point the argument block to the right place...
   // ==============================================
   A = (EDMS_Argblock_Pointer)D -> argblock_pointer;

//      *******************HACK*HACK*HACK*HACK*****************************+
//      ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// A = (EDMS_Argblock_Pointer)EDMS_Seamus_is_an_asshole;
//      *******************HACK*HACK*HACK*HACK*****************************+
//      ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


   // Set the scale of the playfield and zero the collision data...
   // =============================================================
   hash_scale.fix_to( D->playfield_size );
   hash_scale = collision_size / hash_scale;

   for ( coord = 0; coord < EDMS_DATA_SIZE; coord++ )
   {
      for ( deriv = 0; deriv < EDMS_DATA_SIZE; deriv++ )
      {
         data[coord][deriv] = 0;
      }
   }


   // Most important is setting the END markers, the rest is simply anal...
   // =====================================================================
   for ( object = 0; object < MAX_OBJ; object++ )
   {
      for ( coord = 0; coord < 7; coord++ )
      {
         for ( deriv = 0; deriv < 3; deriv++ )
         {
            S[object][coord][deriv] = END;                           //Set solver bounds...
         }
      }

      for ( coord = 0; coord < DOF_MAX; coord++ )
      {
         I[object][coord] = END;
      }

      I[object][30] = VACUUM;                //Look, Ma, no object!
      I[object][38] = 0;                  //Don't kill...
   }


// Finally, set all the sleepers to waking states...
// =================================================
   for ( object = 0; object < MAX_OBJ; object++ )
   {
      no_no_not_me[object] = 1;
      alarm_clock[object]  = 0;
   }
}







//	Kill object number deadguy and perform garbage collection.
//	==========================================================
int EDMS_kill( int deadguy )
{
   int   error_code = -1;
   int   deriv = 0;
   int	 object;

   // First see if there is, in fact, an object there...
   // ==================================================
   if ( S[deadguy][0][0] > END )
   {
//    if (EDMS_integrating == 1) mout << "Deleting " << deadguy << "\n";

      // Release from any contractual involvements with other objects...
      // ===============================================================
      reset_collisions( deadguy );

      // Faster, pussycat, kill kill.  First perform the garbage collection...
      // =====================================================================
      for ( object = deadguy + 1; object < MAX_OBJ && S[object][0][0] > END; object++ )
      {
         if (A_is_active && no_no_not_me[(object-1)] == 1)
            delete_object( object - 1 );
         else
            state_delete_object( object - 1 );        //For collisions...

         idof_functions[object-1] = idof_functions[object];

         for ( int coord = 0; coord < 7; coord++ )
         {
            equation_of_motion[object-1][coord] = equation_of_motion[object][coord];
            for ( int deriv = 0; deriv < 3; deriv++ )
            {
               S[( object - 1 )][coord][deriv] = S[object][coord][deriv];   
               if (A_is_active)
                  A[( object - 1 )][coord][deriv] = A[object][coord][deriv];
            }

            k[0][( object - 1)][coord] = k[0][object][coord];
            k[1][( object - 1)][coord] = k[1][object][coord];
            k[2][( object - 1)][coord] = k[2][object][coord];
            k[3][( object - 1)][coord] = k[3][object][coord];
         }
         
         for ( int number = 0; number < DOF_MAX; number++ )
         {                                                   //Copy the parameters... 
            I[( object - 1 )][number] = I[object][number];
         }

         // Fix the excluded collision information...
         // =========================================
         if( I[ (object-1) ][37] > -1 )
            I[ I[ (object-1) ][37].to_int() ][37] = object - 1;

         // Utility pointers also need fixing...
         // ====================================
         utility_pointer[( object - 1 )] = utility_pointer[object];

         // Frequency checks also...
         // ========================
         no_no_not_me[(object-1)] = no_no_not_me[object];
         alarm_clock[(object-1)]  = alarm_clock[object];
         industrial_strength[(object-1)] = industrial_strength[object];
   
         if (A_is_active && no_no_not_me[(object-1)] == 1)
            write_object( object - 1 );
      	else
            state_write_object( object - 1 );			//For collisions...
      }

      // Now kill the old last object...
      // ===============================
      if (A_is_active && no_no_not_me[(object-1)] == 1)
      {
         delete_object( object - 1 );
      }
   	else
         state_delete_object( object - 1 );			//For collisions...

      for ( int coord = 0; coord < 7; coord++ )
      {
         for ( int deriv = 0; deriv < 3; deriv++ )
         {
            S[( object - 1 )][coord][deriv] = END;
         }
      }

      for ( int number = 0; number < DOF_MAX; number++ )
      {                                         //Kill the parameters... 
         I[( object - 1 )][number] = END;
      }

      I[( object - 1 )][30] = VACUUM;                                         //Nothing there...
      I[( object - 1 )][38] = 0;                 //Don't kill...

      // Points nowhere...
      // =================
      idof_functions[ object - 1 ] = NULL;

      // Okay, everything has gone okay...
      // =================================
      error_code = 0;

      // Frequency checks also...
      // ========================
      no_no_not_me[(object-1)] = 1;
      industrial_strength[(object-1)] = 0;
      alarm_clock[(object-1)] = 0;
   }       //Validity...

   return error_code;
}



//	Wow.  The following is amazing.  An amazing following.
//	======================================================



//	Collision wakeup...
//	===================
void  collision_wakeup( int object )
{
   Q     idof_state[DOF_MAX],
         state[7][4],
         arg[7][4];

   int      coord = 0,
            deriv = 0,
            new_object = 0;

   physics_handle ph;

   Q     *utility_save;
   void     (*idof_function_save)( int );

   extern void     inventory_and_statistics();

   // Save me, save me...
   // ===================
   for ( coord = 0; coord < 7; coord++ )
   {
      for ( deriv = 0; deriv < 3; deriv++ )
      {
         state[coord][deriv] = S[object][coord][deriv];
         if (no_no_not_me[object])
            arg[coord][deriv]   = A[object][coord][deriv];
         else
            arg[coord][deriv]   = S[object][coord][deriv];
      }
   }

   for ( coord = 0; coord < DOF_MAX; coord++ )
   {
      idof_state[coord] = I[object][coord];
   }
   utility_save = utility_pointer[object];
   idof_function_save = idof_functions[object];

   // Now kill the offender...
   // ========================
   EDMS_kill( object );

   // Simulate the action of soliton packing...
   // -----------------------------------------
   ph = on2ph[object];
   EDMS_release_object( ph );
   for ( coord = (object+1); coord < MAX_OBJ; coord++ )
      EDMS_remap_object_number( coord, (coord-1) );

   // Where do I store the copy...
   // ============================
   while ( S[new_object++][0][0] > END );
   new_object -= 1;


   // Create the duplicate...
   // =======================
   for ( coord = 0; coord < 7; coord++ )
   {
      for ( deriv = 0; deriv < 3; deriv++ )
      {
         A[new_object][coord][deriv] = arg[coord][deriv];
         S[new_object][coord][deriv] = state[coord][deriv];
      }
   }

   for ( coord = 0; coord < DOF_MAX; coord++ )
   {
      I[new_object][coord] = idof_state[coord];
   }

   // Utilities...
   // ============
   utility_pointer[new_object] = utility_save;

   // Internals...
   // ============
   idof_functions[new_object] = idof_function_save;

   // Set the physics handles right...
   // ================================
   on2ph[new_object] = ph;
   ph2on[ph] = new_object;

   // Collisions...
   // =============
   state_write_object(new_object);               //This is ALWAYS DURING integration!!
   no_no_not_me[new_object] = 1;           //Wake up!!!
   industrial_strength[new_object] = 1;    //Wake up HOT!

   // Fix the excluded collision information...
   // =========================================
   if( I[new_object][37] > -1 )
      I[ I[new_object][37].to_int() ][37] = new_object;
}






#pragma require_prototypes off
//	This guy is the thing that models without the full six degrees of freedom get as
//	their ignorable coordinates.
//	============================
void null_function( int /*dummy*/ ) { }
#pragma require_prototypes on
