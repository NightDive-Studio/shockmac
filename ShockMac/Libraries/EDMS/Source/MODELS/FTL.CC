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
//	Many games require objects which travel faster than the renderer can possibly draw.  The
//	stuff in this file handles these things in various ways.  For instance, a laser weapon
//	can be raycast instantaneously, dependant only upon the terrain and object models.
//	==================================================================================


#include "EDMS_int.h"

//#ifdef EDMS_SHIPPABLE
//#include <mout.h>
//#endif
//#include <lg.h>
//#include <_edms.h>

#include "EDMS_vt.h"
#include "physhand.h"

//extern "C" {
#include "ss_flet.h"
//}

//	Here is some stuff that the line finder needs that is stupid to pass around...
//	==============================================================================
static Q		initial_X[3] = {0,0,0},
			final_X[3]   = {0,0,0};


extern int EDMS_integrating;

extern int alarm_clock[MAX_OBJ];

physics_handle	object_check( unsigned int data_word, Q size, Q range, int exclude, int steps, Q &dist);	//Checks for hits...

//	Here is the high velocity weapon primitive...
//	=============================================
physics_handle EDMS_cast_projectile( Q *X, Q D[3], Q kick, Q knock, Q size, Q range, int exclude, int shooter )
{
   extern Q	PELVIS;
   int		stepper = 0,
      		max_step = ( 2*range / size ).to_int(),		//samples per meter...
      		victim_on = 0,
      		shooter_on = 0,
            object_pointer = 0,
            i = 0;

   unsigned int   must_check_objects[MAX_OBJ];

   unsigned int	test_data;
   unsigned int   last_test_data = 0;

   physics_handle	victim = -1,				//It is what is says it is...
   		  return_victim = -1;			//The number actually returned...

   // I think now victim and return_victim can be the same thing - DS

   Q iota_c = 0;				//Some variable or other...
   Q D_old[3];                               //Hey, blow blow blow...

   Q dist;                             // where did we hit the victim?

   //	Reset the object collisions...
   //	==============================
   int	no_dont_do_it = 0;

   //	Looks at terrain...
   //	===================
   fix	checker = 0;

   // PRINT3D( X );
   // mout << "Max: " << max_step << " :range: " << range << " : size: " << size << ".\n";

   //	Save the initial vectors for the line finder...
   //	===============================================
	initial_X[0] = X[0];
	initial_X[1] = X[1];
	initial_X[2] = X[2];

   Q anus = sqrt( D[0]*D[0] + D[1]*D[1] + D[2]*D[2] );
   if (anus < .80 || range < .25 )
   {
      if (anus < .80)
      {
         D[2] = 1;
		   D[1] = D[0] = 0;
         no_dont_do_it = 1;
		}

//#ifdef EDMS_SHIPPABLE
//      mout << "!EDMS: raycast with bad vector: mag = " << anus << ", range = " << range << "\n";
//#endif
      range = .25;
   }

   //	Rescale direction to 1/3 decimeters...
   //	======================================
   D_old[0] = D[0];
   D_old[1] = D[1];
   D_old[2] = D[2];

   D[0] *= .5*size;
	D[1] *= .5*size;
	D[2] *= .5*size;

//	Make sure we're ON THE MAP before casting into random memory...
//	===============================================================
	if ((X[0] < 0) || (X[1] < 0) ||
       (X[0] > EDMS_DATA_SIZE - 1) || (X[1] > EDMS_DATA_SIZE - 1))
   {
      checker = 1000;
//      mout << "!EDMS: bad raycast start at:\n";
//      PRINT3D( X )
//      mout << "!EDMS: raycast excluded physics handle " << exclude << "\n";
//		flush( mout );
		no_dont_do_it = 1;
   }


//	Are we good to go?
//	==================
	if ( no_dont_do_it == 0 )
   {
      //	Find impact point...
      //	====================
   	for ( stepper = 0; (stepper < max_step) && (checker == 0); stepper++ )
      {
      	checker = 0;

         ss_edms_stupid_flag = TFD_RCAST;

      	indoor_terrain(X[0],					//Get the info...
                			X[1],
			               X[2],
			               size,
			               -1 );

         ss_edms_stupid_flag = TFD_FULL;

         //	Check the terrain...
         //	====================
#ifdef NOT
		   checker = ( terrain_info.cx ) | ( terrain_info.cy ) | ( terrain_info.cz )
			        | ( terrain_info.fx ) | ( terrain_info.fy ) | ( terrain_info.fz )
			        | ( terrain_info.wx ) | ( terrain_info.wy ) | ( terrain_info.wz );
#else
         checker = (ss_edms_facelet_cnt != 0);
#endif

         //	Check for object collisions...
         //	==============================
         int hx = floor( hash_scale*X[0] );
         int hy = floor( hash_scale*X[1] );
   		test_data = data[ hx ][ hy ];

         if (test_data != last_test_data && test_data != 0)
         {
            if (object_pointer < MAX_OBJ)
            {
               must_check_objects[object_pointer] = test_data;
               object_pointer++;
            }
            last_test_data = test_data;
         }

         //	Move the check point...
         //	=======================
	      X[0] += D[0];
	      X[1] += D[1];
	      X[2] += D[2];

      	if ((X[0] < 0) || (X[1] < 0) || (X[0] > EDMS_DATA_SIZE - 1) || (X[1] > EDMS_DATA_SIZE - 1))
         {
            checker = 1000;	//Get out...
//            PRINT3D( X )
//            mout << "!EDMS: Raycast has left map!\n";
         }
   	}

      //	Save the final point of the line segment...
      //	===========================================
   	final_X[0] = X[0];
   	final_X[1] = X[1];
   	final_X[2] = X[2];

      // Now we're done following the ray.

      for (i = 0; i < object_pointer; i++)
      {
         victim = object_check (must_check_objects[i], size, range, exclude, stepper, dist);

         if (victim != -1)
         {
            // We hit someone!
    			return_victim = victim;		//return the right guy!
		
      		victim_on = ph2on[victim];

            Q inv_mass = (I[victim_on][30] == ROBOT ? I[victim_on][24] : I[victim_on][36]);

            if (inv_mass > 0.05 && knock * inv_mass > 10.0 / inv_mass)
            {
//             mout << "Clamping knock from " << knock;

               knock = 10.0 / (inv_mass * inv_mass);

//             mout << " to " << knock << "\n";
            }

            if (I[victim_on][30] == ROBOT)
               iota_c = 200*inv_mass*inv_mass*knock;
            else
               iota_c = 200*inv_mass*inv_mass*knock;

			   I[victim_on][32] = D_old[0]*iota_c;	//Absolute blows off walls, remember explosions too...
			   I[victim_on][33] = D_old[1]*iota_c;
			   I[victim_on][34] = D_old[2]*iota_c;

			   I[victim_on][35] = 1;		//Deweet!

            if (no_no_not_me[victim_on]==0)
            {  // hey folks, if our poor victim is asleep, wake him in the way appropriate to us
   	         if (EDMS_integrating)
	               alarm_clock[victim_on] = 1;
	            else
	   			   no_no_not_me[victim_on] = 1;	//Make sure we're up...          a
            }

            break;                  // All done looking!
         }
      }


      //	If we did, in fact, hit a wall, the 3D system precision may be insufficient to sort the hit
      //	art in front of the wall.  Therefore...
      //	=======================================

      // The EDMS code used to set the endpoint of the beam to the center of the
      // victim if it hit someone.  But in reality, bugs in the code made it
      // always think at that point that it hadn't hit anyone, so the code was
      // never executed.  When the bugs were fixed and beams actually started
      // hitting centers of objects, people complained.  So I am just changing
      // back to the old way, which one would think puts the beam way too far
      // away since it ignores the position of the victim entirely, but apparently
      // due to the way System Shock sorts beams and hits everything works out
      // okay in the end.  I tried a little to do it correctly for real but I ran
      // out of time.  - DS

      if (victim > -1)
      {
         // ha ha, the above text lies, it now works

         X[0] = initial_X[0] + D_old[0] * dist;
         X[1] = initial_X[1] + D_old[1] * dist;
         X[2] = initial_X[2] + D_old[2] * dist;

//         Spew (DSRC_EDMS_Collide, ("vic %f %f %f hit %f %f %f\n", S[victim_on][0][0], S[victim_on][1][0],
//            S[victim_on][2][0], X[0], X[1], X[2]));
      }
      else
      {
         // Apparently things are going through walls a little so let's bring
         // it even farther back.

         X[0] -= D[0]*2;
 		   X[1] -= D[1]*2;
  		   X[2] -= D[2]*2;
      }

      //	Did we hit a wall, or did we hit range out?
      //	===========================================
   	if ( (stepper == max_step) && (victim == -1) )
      {
   		X[0] = 
   		X[1] = 
   		X[2] = END;
		}


      //	Do the kickback...
      //	==================
   	if( shooter != -1 )
      {
         shooter_on = ph2on[shooter];
			iota_c = I[shooter_on][29]*kick;
			
         if ( I[shooter_on][30] == PELVIS )
         {
            I[shooter_on][8] = D_old[0]*iota_c;
         	I[shooter_on][9] = D_old[1]*iota_c;
         }
   	}

      //	Were we good to go?
      //	===================
	}


//	Hit for now...
//	==============
	return return_victim;

}


//#pragma off (unreferenced)
//	Here, since we know the line segment we're interested in, we check to make sure that we
//	didn't hit any objects, and return the one we did...
//	====================================================
physics_handle	object_check( unsigned int data_word, Q size, Q range, int exclude, int stepper, Q &dist)
{
   //		General purpose...
   //		==================
   int		object;
   physics_handle	victim = -1;

   //	For the lines...
   //	================
   Q	a = initial_X[0] - final_X[0],
	   b = initial_X[1] - final_X[1],
	   c = initial_X[2] - final_X[2],
	   top_1 = 0,
	   top_2 = 0,
	   top_3 = 0,
	   bottom = 0,
	   kill_zone = 0,
      kzdist = 0,
      kzdisto = 10000;


   ulong bit = 0;                      // which object bit we're checking

   while (data_word != 0)
   {
      if (data_word & 1)
      {
         // Object bit number 'bit' is on, we must check all objects which have that bit
         for (object = bit; object < MAX_OBJ && S[object][0][0] > END; object += NUM_OBJECT_BITS)
         {
            if (object != exclude)
            {
   	         top_1 = c*(S[object][1][0] - initial_X[1]) - b*(S[object][2][0] - initial_X[2]);
		         top_1 *= top_1;

		         top_2 = a*(S[object][2][0] - initial_X[2]) - c*(S[object][0][0] - initial_X[0]);
		         top_2 *= top_2;

		         top_3 = b*(S[object][0][0] - initial_X[0]) - a*(S[object][1][0] - initial_X[1]);
		         top_3 *= top_3;

		         bottom = a*a + b*b + c*c;

		         kill_zone = sqrt( (top_1 + top_2 + top_3) / bottom );		


		         if ( kill_zone < (I[object][31] + size) )
               {
                  kzdist = sqrt( (initial_X[0] - S[object][0][0])*(initial_X[0] - S[object][0][0])
                                 + (initial_X[1] - S[object][1][0])*(initial_X[1] - S[object][1][0])
                                 + (initial_X[2] - S[object][2][0])*(initial_X[2] - S[object][2][0]) );

			         if ( ( kzdist < .5*size*stepper  ) && ( kzdist < kzdisto ) )
                  {
   			         victim = on2ph[object];                       
   			         kzdisto = kzdist;
                     dist = kzdist - I[object][31];

//	   			      X[0] = S[object][0][0];	//Provide hit location, naive for now...
//	   			      X[1] = S[object][1][0];
//	   			      X[2] = S[object][2][0];
   		         }
               }


               // Is it a pelvis, and, if so, do I check for your head?  (sooooo clean..)
               // =======================================================================
               if ( I[object][30] == PELVIS )
               {

                  Q position[3];

                  Q offset_x = I[object][0]*sin( S[object][4][0] ),
                    offset_y =-1.5*I[object][0]*sin( S[object][5][0] ),
                    offset_z = I[object][0]*cos( S[object][4][0] )*cos( S[object][5][0] );

                  Q sin_alpha = 0,
                    cos_alpha = 0;

                  Q final_x = 0,
                    final_y = 0;        
      
                  sincos( -S[object][3][0], &sin_alpha, &cos_alpha );
                  final_x = cos_alpha*offset_x + sin_alpha*offset_y;
                  final_y =-sin_alpha*offset_x + cos_alpha*offset_y;

                  position[0] = S[object][0][0] + final_x;
                  position[1] = S[object][1][0] + final_y;
                  position[2] = S[object][2][0] + offset_z;

                  top_1 = c*(position[1] - initial_X[1]) - b*(position[2] - initial_X[2]);
        	         top_1 *= top_1;
      
        	         top_2 = a*(position[2] - initial_X[2]) - c*(position[0] - initial_X[0]);
                  top_2 *= top_2;

        	         top_3 = b*(position[0] - initial_X[0]) - a*(position[1] - initial_X[1]);
	               top_3 *= top_3;

        	         bottom = a*a + b*b + c*c;

	               kill_zone = sqrt( (top_1 + top_2 + top_3) / bottom );		

                  if ( kill_zone < ( .75*I[object][22] + size) )
                  {
                     kzdist = sqrt( (initial_X[0] - position[0])*(initial_X[0] - position[0])
                                    + (initial_X[1] - position[1])*(initial_X[1] - position[1])
                                    + (initial_X[2] - position[2])*(initial_X[2] - position[2]) );

                     // It's a bouncing baby head hit!...
                     // ---------------------------------
                     if ( ( kzdist < .5*size*stepper  ) && ( kzdist < kzdisto ) )
                     {
	                     victim = on2ph[object];
			               kzdisto = kzdist;
                        dist = kzdist - I[object][31];
  	        	         }
                  }

               }                       // if pelvis
            }                          // if (object != exclude)
         }                             // for (object = bit)
      }                                // if (data_word & 1)

      // Shift over the mask so we're testing the next object bit
      data_word >>= 1;
      bit++;
   }                                   // while (data_word != 0)

	return victim;
}
//#pragma on (unreferenced)


