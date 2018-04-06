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
 * $Header: n:/project/lib/src/edms/RCS/collide.cc 1.3 1994/04/20 18:44:14 roadkill Exp $
 */

//	Here are the routines that write object presences to the terrain database.
//	Optimally, these will be swapped with the appropriate routine on a project
//	to project basis.  They are quite simple here.
//	==============================================


#include "EDMS_int.h"
#include "idof.h"
#include "lg.h"

#pragma require_prototypes off			// Added by KC for this file for Mac version.

//	The space itself...
//	===================
//
// We divide the playfield into a EDMS_DATA_SIZE by EDMS_DATA_SIZE grid.
// Each <x,y> bin in the grid contains (possibly) object n if
// data[x][y] & (1 << n) != 0.
//
unsigned int	data[ EDMS_DATA_SIZE ][ EDMS_DATA_SIZE ];


//	Constants for the fixpoint...
//	=============================
const int	collision_max = EDMS_DATA_SIZE - 1;
extern int  EDMS_integrating;

//////////////////////////////
//
// Okay, here is the generic function that write_object and state_write_object
// will both call.  When I can check out the appropriate files, they'll turn
// into macros.
//
void generic_write_object (int object, EDMS_Argblock_Pointer state)
{
   extern void	( *EDMS_off_playfield )( physics_handle caller );

   Q q_hash_x = hash_scale*state[object][DOF_X][0];
   Q q_hash_y = hash_scale*state[object][DOF_Y][0];

	unsigned int obit = object_bit( object );

   int hash_x = (q_hash_x).to_int();	     		//The floor should be a function
	int hash_y = (q_hash_y).to_int();	     		//that returns the edges of the ref. squares...

	if ((hash_x > 1) && (hash_y > 1) &&
       (hash_x < collision_max) && (hash_y < collision_max ))
   {
      // We want to write a 2x2 block.  We adjust the upper left corner of it
      // appropriately given our location in the square.

      if (q_hash_x - hash_x < DELTA_BY_TWO) hash_x--;
      if (q_hash_y - hash_y < DELTA_BY_TWO) hash_y--;

      write_object_bit (hash_x    , hash_y,     obit);
      write_object_bit (hash_x    , hash_y + 1, obit);
      write_object_bit (hash_x + 1, hash_y,     obit);
      write_object_bit (hash_x + 1, hash_y + 1, obit);
	}
	else
   {  
      //	End of slowness, and inform the world that something stinks in Denmark...
      //	-------------------------------------------------------------------------
//      mout << "Collide...\n";
      EDMS_off_playfield( on2ph[object] );
   }						    
}

void write_object (int object)
{
//   mout << "Write A\n";
   generic_write_object (object, A);
}

void state_write_object (int object)
{
//   mout << "Write S\n";
   generic_write_object (object, S);
}



//////////////////////////////
//
// And here's a generic function for delete_object.
//

void generic_delete_object (int object, EDMS_Argblock_Pointer state)
{
   Q q_hash_x = hash_scale*state[object][DOF_X][0];
   Q q_hash_y = hash_scale*state[object][DOF_Y][0];

	unsigned int obit = object_bit( object );

   int hash_x = (q_hash_x).to_int();	     		//The floor should be a function
	int hash_y = (q_hash_y).to_int();	     		//that returns the edges of the ref. squares...

// AAAAAhhhhhhhhh...
// mout << "DA" << object << ": (" << hash_x << ", " << hash_y << ")\n";
// if ( EDMS_integrating != 1 ) mout << "BADBADBADBADBADBADBADBADBADBADBADBADBADBADBADBBADBADB!!!!\n";

   //	How slow can you go?
   //	--------------------
	if ((hash_x > 1) && (hash_y > 1) &&
       (hash_x < collision_max) && (hash_y < collision_max ))
   {
      // We want to delete a 2x2 block.  We adjust the upper left corner of it
      // appropriately given our location in the square.

      if (q_hash_x - hash_x < DELTA_BY_TWO) hash_x--;
      if (q_hash_y - hash_y < DELTA_BY_TWO) hash_y--;

      delete_object_bit (hash_x    , hash_y,     obit);
      delete_object_bit (hash_x    , hash_y + 1, obit);
      delete_object_bit (hash_x + 1, hash_y,     obit);
      delete_object_bit (hash_x + 1, hash_y + 1, obit);
	}
   else
   {
      //	End of slowness, and inform the world that something stinks in Denmark...
      //	-------------------------------------------------------------------------
//        mout << "collide2...\n";
   	EDMS_off_playfield( on2ph[object] );
//      Spew (DSRC_EDMS_Collide, ("Bounds on %d ph %d hash [%d %d]\n", object, on2ph[object], hash_x, hash_y));
//      Spew (DSRC_EDMS_Collide, ("state x = %8x y = %8x\n", state[object][DOF_X][0], state[object][DOF_Y][0]));
   }
}

void delete_object (int object)
{
//   mout << "Delete A\n";
   generic_delete_object (object, A);
}

void state_delete_object (int object)
{
//   mout << "Delete S\n";
   generic_delete_object (object, S);
}


//////////////////////////////
//
// Find out whether a given object could actually be found in hash location
// <hx, hy>.  This is a handy way to find out which of the three objects with
// a given object bit is actually meant.

bool object_check_hash (int object, int hx, int hy)
{
   // We use A if we are in the middle of integrating, and the object
   // is not asleep.

   EDMS_Argblock_Pointer state = (A_is_active && no_no_not_me[object]) ? A : S;

   int my_hx = (hash_scale * state[object][DOF_X][0]).to_int();
   int my_hy = (hash_scale * state[object][DOF_Y][0]).to_int();

   return (abs (my_hx - hx) <= 1 && abs (my_hy - hy) <= 1);
}










//	Collision exclusion...
//	======================



//	Turn it off...
//	==============
void reset_collisions( int object )
{
   //	Are we really inactivated?
   //	--------------------------
	if ( I[object][IDOF_COLLIDE] > -1 )
   {
		I[ I[object][IDOF_COLLIDE].to_int() ][IDOF_COLLIDE] = -1;
		I[object][IDOF_COLLIDE] = -1;
	}
}


//	Turn it on...
//	=============
void exclude_from_collisions( int guy_1, int guy_2 )
{
	//	Are we ready?
	//	-------------
	if (  (I[guy_1][IDOF_COLLIDE] > -1) || (I[guy_2][IDOF_COLLIDE] > -1)  )
	{
		reset_collisions( guy_1 );
		reset_collisions( guy_2 );
	}
	I[guy_1][IDOF_COLLIDE] = guy_2;
	I[guy_2][IDOF_COLLIDE] = guy_1;
	
	// Viola!
}




//	Read'n'...
//	==========

unsigned int    test_bitmask; // used to be clean_test_bit


//	Basically subtract out the bit representing the calling object...
//	=================================================================
int are_you_there( int object )
{
	return (test_bitmask = data[ (hash_scale*A[object][DOF_X][0]).to_int() ][ (hash_scale*A[object][DOF_Y][0]).to_int() ]);
}


//////////////////////////////
//
// This is now a macro in edms_int.h called check_for_hit_mac

#ifdef NOPE
//	Subtract out the bit representing the calling object, then compare notes...
//	===========================================================================
int check_for_hit( int other_object )
{
// unsigned int	test_bit = data[ (hash_scale*A[object][DOF_X][0]).to_int() ][ (hash_scale*A[object][DOF_Y][0]).to_int() ];

	return 	clean_test_bit & object_bit( other_object );

//	return 	( test_bit & ~( object_bit( object ) ) )
//		& check_object( object, other_object );
}
#endif


// Won't get compiled in unless you specifically turn it on here
#ifdef DEBUGGING

extern "C" {

void spew_collision_table ()
{
   int i, j;

   for (i = 0; i < collision_max; i++)
   {
      for (j = 0; j < collision_max; j++)
      {
         if (data[i][j])
         {
            int bit, mask;

            for (bit = 0, mask = 1; bit < 32; bit++, mask <<= 1)
            {
               if (data[i][j] & mask)
               {
//                  Spew (DSRC_EDMS_Collide, ("[%d %d]: on %d ph %d\n", i, j, bit, on2ph[bit]));
               }
            }
         }
      }
   }
}

}

#endif
