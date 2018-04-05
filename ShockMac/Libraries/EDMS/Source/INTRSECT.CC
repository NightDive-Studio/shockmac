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
//	Collision intersection code for EDMS models...
//	==============================================



//	Seamus, "in Prozac we trust," 1994
//	==================================



#include "EDMS_int.h"
#include "idof.h"

//#ifdef EDMS_SHIPPABLE
//#include <mout.h>
//#endif

//              Collision wakeups go here...
//              ----------------------------
extern int      alarm_clock[MAX_OBJ];
extern int      no_no_not_me[MAX_OBJ];

bool do_work( int object, int other_object, Q my_rad, Q your_rad,
              Q my_pos[3], Q your_pos[3], Q &result0, Q &result1, Q &result2 );

void shall_we_dance( int object, Q& result0, Q& result1, Q& result2 );


//	Call me instead of having special code everywhere...
//	====================================================
void shall_we_dance( int object, Q& result0, Q& result1, Q& result2 )
{
   int other_object;

   Q	my_radius,
   	your_radius;

   Q	my_position[3],
   	your_position[3];

   //	Collision B/C...
   //	----------------
	result0 = result1 = result2 = 0;						//B/C...

   //	Here we assume that all hits are encompassed by the projection of the
   //	default radius.  If this is not true, then special care must be taken
   //	in the design of the model...
   //	-----------------------------

   // mask contains the bits corresponding to the objects that could be
   // intersecting object.

   ulong mask = are_you_there (object);
   ulong bit  = 0;                     // which object bit we're checking

   while (mask != 0)
   {
      if (mask & 1)
      {
         // Object bit number 'bit' is on, we must check all objects which have that bit
         for (other_object = bit; other_object < MAX_OBJ && S[other_object][0][0] > END; other_object += NUM_OBJECT_BITS)
         {


            if (other_object != object && I[object][IDOF_COLLIDE].to_int() != other_object )
            {
            
               //	Okay, now we have a confirmed hash hit...
               //	-----------------------------------------

               //	Do the regular guy, workaday collision...
               //	-----------------------------------------
         		my_position[0] = A[object][0][0];
         		my_position[1] = A[object][1][0];
         		my_position[2] = A[object][2][0];

               // if you're asleep, then we have to look at STATE...
               // --------------------------------------------------
               if (no_no_not_me[other_object] == 1 )
               {
            		your_position[0] = A[other_object][0][0];
            		your_position[1] = A[other_object][1][0];
            		your_position[2] = A[other_object][2][0];
               }
               else
               {
         		   your_position[0] = S[other_object][0][0];
         		   your_position[1] = S[other_object][1][0];
         		   your_position[2] = S[other_object][2][0];
               }

               my_radius = I[object][IDOF_RADIUS];
               your_radius = I[other_object][IDOF_RADIUS];

         		do_work( object, other_object, my_radius, your_radius,
                        my_position, your_position, result0, result1, result2);

               int	you_are_special = 0,
                  	I_am_special = 0;

               //	Are YOU special???
               //	------------------
         		if ( I[other_object][IDOF_MODEL] == PELVIS )
               {
                  Q offset_x = I[other_object][0]*sin( A[other_object][4][0] ),
                  offset_y =-1.5*I[other_object][0]*sin( A[other_object][5][0] ),
                  offset_z = I[other_object][0]*cos( A[other_object][4][0] )*cos( A[other_object][5][0] );

                  Q sin_alpha = 0,
                  cos_alpha = 0;

                  sincos( -A[other_object][3][0], &sin_alpha, &cos_alpha );

                  Q final_x = cos_alpha*offset_x + sin_alpha*offset_y;
                  Q final_y =-sin_alpha*offset_x + cos_alpha*offset_y;

            		your_position[0] = A[other_object][0][0] + final_x;
            		your_position[1] = A[other_object][1][0] + final_y;
            		your_position[2] = A[other_object][2][0] + offset_z;

            		my_radius = I[object][IDOF_RADIUS];
                  your_radius = .75*I[other_object][IDOF_PELVIS_RADIUS];

            		do_work( object, other_object, my_radius, your_radius,
                           my_position, your_position, result0, result1, result2);
         		}	//You're not special.

               //		Am I special???
               //		---------------
         		if ( I[object][IDOF_MODEL] == PELVIS )
               {
                  Q offset_x = I[object][0]*sin( A[object][4][0] ),
                  offset_y =-1.5*I[object][0]*sin( A[object][5][0] ),
                  offset_z = I[object][0]*cos( A[object][4][0] )*cos( A[object][5][0] );

                  Q sin_alpha = 0,
                  cos_alpha = 0;

                  sincos( -A[object][3][0], &sin_alpha, &cos_alpha );

                  Q final_x = cos_alpha*offset_x + sin_alpha*offset_y;
                  Q final_y =-sin_alpha*offset_x + cos_alpha*offset_y;

		            my_position[0] = A[object][0][0] + final_x;
		            my_position[1] = A[object][1][0] + final_y;
		            my_position[2] = A[object][2][0] + offset_z;

   
                  // if you're asleep, then we have to look at STATE...
                  // --------------------------------------------------
                  if (no_no_not_me[other_object] == 1 )
                  {
         		      your_position[0] = A[other_object][0][0];
         		      your_position[1] = A[other_object][1][0];
         		      your_position[2] = A[other_object][2][0];
                  }
                  else
                  {
                		your_position[0] = S[other_object][0][0];
               		your_position[1] = S[other_object][1][0];
               		your_position[2] = S[other_object][2][0];
                  }
                           
                  my_radius   = .75*I[object][IDOF_PELVIS_RADIUS];
                  your_radius = I[other_object][IDOF_RADIUS];

            		do_work( object, other_object, my_radius, your_radius,
                           my_position, your_position, result0, result1, result2);
         		}	//I'm not special...
      		}	//No hash hit...
         }
      }

      // Shift over the mask so we're testing the next object bit
      mask >>= 1;
      bit++;
   }
}

Q       dx, dy, dz;
 
//	Here's the meat of the sutuation...
//	===================================
bool do_work( int object, int other_object, Q my_rad, Q your_rad,
              Q my_pos[3], Q other_pos[3]/*, Q my_dot[3], your_dot[3]*/,
              Q &result0, Q &result1, Q &result2 )
{
   Q cm_radius = ( my_rad + your_rad );

   // First do a preliminary check to avoid overflow.
   dx = my_pos[0] - other_pos[0];
   dy = my_pos[1] - other_pos[1];
   dz = my_pos[2] - other_pos[2];

   if (dx >= cm_radius || dy >= cm_radius || dz >= cm_radius)
   {
      return FALSE;                          // couldn't possibly collide
   }

//	Test for primary collision...
//	=============================
	Q test_radius = sqrt(dx * dx + dy * dy + dz * dz);

 	if ((test_radius < cm_radius) && (test_radius > 0.0005))
   {

//              Is there a problem???
//              ---------------------
                if (test_radius < .03) test_radius = .03;


      //	Callback...
      //	-----------
      physics_handle	C = on2ph[object],
		        V = on2ph[other_object];

      int badness = ( 20*(1. - test_radius/cm_radius) ).to_int();

      fix location[3];

      location[0] = my_pos[0].to_fix();
      location[1] = my_pos[1].to_fix();
      location[2] = my_pos[2].to_fix();

   	EDMS_object_collision( C, V, badness, 0, 0, location );

      Q Eta = ( cm_radius - test_radius );	  	   		//Eta...
        	
      test_radius = 1 / test_radius;
		result0 += Eta*dx*test_radius;
		result1 += Eta*dy*test_radius;
      result2 += Eta*dz*test_radius;

      // God save the Queen...
      // ---------------------                
      if (result0 > my_rad) result0 = my_rad;
      if (result0 <-my_rad) result0 =-my_rad;
      
      if (result1 > my_rad) result1 = my_rad;
      if (result1 <-my_rad) result1 =-my_rad;

      //	Wakeup...
      //	=========
		if (no_no_not_me[other_object] == 0 )
      {
//       mout << "Other guy was asleep: " << other_object << "\n";
//   		collision_wakeup( other_object );
                alarm_clock[other_object] = 1;
		}		

      return TRUE;                     // collision
	}	//End of radius check...
   else
   {
      return FALSE;
   }
}
