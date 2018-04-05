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
// Exclusive, one time only Freefall appearance!!!

// Many games require objects which travel faster than the renderer can possibly draw.  The
// stuff in this file handles these things in various ways.  For instance, a laser weapon
// can be raycast instantaneously, dependant only upon the terrain and object models.
// ==================================================================================


#include <EDMS_int.h>

#ifdef EDMS_SHIPPABLE
#include <mout.h>
#endif

#include <EDMS_vt.h>
#include <physhand.h>
#include <externs.h>

#include <lg.h>
#include <_edms.h>
#pragma warning 555 9

#pragma off (unreferenced)

// Here is some stuff that the line finder needs that is stupid to pass around...
// ==============================================================================
static Q initial_X[3] = {0,0,0},
         final_X[3]   = {0,0,0};

static Q D_old[3];                         // saved initial vector; this is a unit vector!


// For now...
// ----------
static terrain_ff TFF;                    //3 guesses, first 2 don't count...

physics_handle FF_object_check( unsigned int data_word, int hx, int hy,
                                Q size, Q range, int exclude, int steps );
physics_handle FF_object_check_new ( unsigned int data_word, int hx, int hy,
                                     Q size, Q range, int exclude, int max_step );

typedef struct
{
   int hx, hy;                         // hash location
   int bits;                           // what object bits were in that loc
} hash_info;

Q sampling_period;


//////////////////////////////
//
// New version that calls Freefall terrain function
//
physics_handle EDMS_FF_cast_projectile_new
             ( Q *X,                   // source
               Q D[3],                 // unit vector of ray
               Q speed,                // for knockback
               Q mass,                 // for knockback
               Q size,                 // radius
               Q range,                // how far it can reach
               int exclude,            // what object it cannot hit, typically the firer
               int shooter,            // not used
               long &g_info,           // information about ground hit
               long &w_info,           // information about wall hit
               bool &hit )             // whether it hit something
{
   sampling_period = size*2;           // diameter of projectile
                                       // this is four times what Seamus had it as originally
                                       // what's life without a little risk?
   int stepper = 0,
       object_pointer = 0,          //For object checks...
       max_step = ( range / sampling_period ).to_int(),     //samples per meter...
       victim_on = 0,
       shooter_on = 0;

   hash_info must_check_objects[MAX_OBJ];
   unsigned int test_data;                   // bitmask result of hash query
   unsigned int last_test_data = 0;

   physics_handle victim = -1;               //It is what is says it is...
                  
   Q ground;                        //FF terrain issues...

   int hx, hy;                         // hash location of X[]

   // Looks at terrain...
   // ===================
   fix   checker = 0;

// PRINT3D( X );
// mout << "Max: " << max_step << " :range: " << range << " : size: " << size << ".\n";

   // Save the initial vectors for the line finder...
   // ===============================================
   initial_X[0] = X[0];
   initial_X[1] = X[1];
   initial_X[2] = X[2];
   hx = floor( hash_scale*X[0] );
   hy = floor( hash_scale*X[1] );

   // Ensure that D is a unit vector

   Q anus = sqrt( D[0]*D[0] + D[1]*D[1] + D[2]*D[2] );
   if (anus < .99 || range < 1 || anus > 1.01)
   {
      D[2] = 1;
      D[1] = D[0] = 0;
#ifdef EDMS_SHIPPABLE
      mout << "Hey, raycast with zero vector!\n";
#endif
      range = 10;
   }

// mout << "Initially in square (" << floor(hash_scale*X[0]) << "," << floor(hash_scale*X[1]) << ").\n";

   // Rescale direction...
   // ====================
   D_old[0] = D[0];
   D_old[1] = D[1];
   D_old[2] = D[2];

   sampling_period *= 16;              // go nuts now that Freefall is doing the terrain raycast

   // Turn D into the stepping vector

   D[0] *= sampling_period;
   D[1] *= sampling_period;
   D[2] *= sampling_period;

   // Make sure the game doesn't take this too seriously...
   // =====================================================
   TFF.caller = 0;
   TFF.my_size = size.to_fix();

   Q where_hit[3];
   bool hit_terrain;

   // The EDMS raycaster works by first calculating the endpoint of the ray disregarding
   // objects (thus it can only be terminated by hitting terrain or ranging out).  Then
   // a front-to-back pass is made looking for objects.
   //
   // In this version, we ask Freefall for the endpoint of the ray and then
   // do the object check ourselves.

   // Make sure we're on the map.
   if (hx < 0 || hy < 0 || hx > EDMS_DATA_SIZE - 1 || hy > EDMS_DATA_SIZE - 1)
   {
//    mout << "!EDMS: bad raycast start at:\n";
//    PRINT3D( X )
//    mout << "!EDMS: raycast excluded physics handle " << exclude << "\n";
//    flush( mout );
   }
   else
   {
      hit_terrain = ff_raycast (X[0], X[1], X[2], D_old, range, where_hit, &TFF);
      ground = where_hit[2];

      // For now, do stupid thing: set range to where we hit terrain, and then
      // look for objects the old way.  In reality we should do a thick Bresenham
      // line from our source to the end of the terrain raycast, checking one
      // square at a time.
      {
         Q dx = where_hit[0] - X[0];
         Q dy = where_hit[1] - X[1];
         Q dz = where_hit[2] - X[2];
         Q planar_dist;
         Q dist;

         Spew (DSRC_EDMS_Collide, ("ff_raycast says hit at %q %q %q\n", X[0], X[1], X[2]));

         planar_dist.fix_to (fix_safe_pyth_dist (dx.to_fix(), dy.to_fix()));
         dist.fix_to (fix_safe_pyth_dist (planar_dist.to_fix(), dz.to_fix()));

         max_step = (dist / sampling_period).to_int();
      }

      // Find impact point...
      // ====================
      for ( stepper = 0; (stepper < max_step) && (checker == 0); stepper++ )
      {
         // Check for object collisions...
         // ==============================
         test_data = data[ hx ][ hy ];

         if (test_data != last_test_data && test_data != 0)
         {
            // Save off what objects are here for second pass

            if (object_pointer < MAX_OBJ)
            {
               must_check_objects[object_pointer].hx   = hx;
               must_check_objects[object_pointer].hy   = hy;
               must_check_objects[object_pointer].bits = test_data;
               object_pointer++;
            }
            last_test_data = test_data;
         }
         
         // Move the check point...
         // =======================
         X[0] += D[0];
         X[1] += D[1];
         X[2] += D[2];

         hx = floor( hash_scale*X[0] );
         hy = floor( hash_scale*X[1] );

         if (hx < 0 || hy < 0 || hx > EDMS_DATA_SIZE - 1 || hy > EDMS_DATA_SIZE - 1)
         {
            break; // off map
         }
      }

//    Spew (DSRC_EDMS_Collide, ("made %d terrain checks from %f %f %f to %f %f %f\n", stepper, initial_X[0],
//       initial_X[1], initial_X[2], X[0], X[1], X[2]));


      // Save the final point of the line segment...
      // ===========================================
      final_X[0] = X[0];
      final_X[1] = X[1];
      final_X[2] = X[2];

      // Now check for objects

      int i;
      for (i = 0; i < object_pointer; i++)
      {
         victim = FF_object_check_new (must_check_objects[i].bits, must_check_objects[i].hx, must_check_objects[i].hy,
                                   size, range, exclude, max_step);

         if (victim != -1)
         {
            // We hit someone!

            hit_terrain = FALSE;

            victim_on = ph2on[victim];
            Q iota_c = 20*I[victim_on][36]*mass*speed;

            // Knock back our victim

            I[victim_on][32] = D_old[0]*iota_c;
            I[victim_on][33] = D_old[1]*iota_c;
            I[victim_on][34] = D_old[2]*iota_c;

            I[victim_on][35] = 1;      //Deweet!

            no_no_not_me[victim_on] = 1;  //Make sure we're up...

            break;                  // All done looking!
         }
      }

      // If we did, in face, hit a wall, the 3D system precision may be insufficient to sort the hit
      // art in front of the wall.  Therefore...
      // =======================================
      if ( victim > -1 )
      {
         // This should at least move back by the sum of the radiuses of this object and victim.

         X[0] = S[victim_on][0][0] - D_old[0];   //Simplest thing for now...
         X[1] = S[victim_on][1][0] - D_old[1];
         X[2] = S[victim_on][2][0] - D_old[2];
      }
      else
      {
         X[0] = where_hit[0];
         X[1] = where_hit[1];
         X[2] = where_hit[2];
      }


      // Did we hit a wall, or did we hit range out?
      // ===========================================
      hit = FALSE;
      g_info =
      w_info = 0;

      if (hit_terrain)
      {
         hit = TRUE;
         g_info = TFF.DATA1;
      }
      else
      // Did we range out?
      // -----------------
      if ( stepper == max_step )
      {           //Check for rangeout...
         hit = FALSE;
         g_info = 
         w_info = 0;
      }


      // Did we hit somebody?
      // --------------------
      if( victim != -1 )
      {
         hit = TRUE;
         g_info =
         w_info = 0;
      }
   }

   return victim;
}

physics_handle FF_object_check_new ( unsigned int data_word, int hx, int hy, Q size, Q range, int exclude, int max_step )
{
   // General purpose...
   // ==================
   int object;
   physics_handle victim = -1;

   // For the lines...
   // ================
   Q  a = final_X[0] - initial_X[0],
      b = final_X[1] - initial_X[1],
      c = final_X[2] - initial_X[2],
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
               if (!object_check_hash (object, hx, hy)) continue; // didn't mean this one

               // Distances of potentially hit object to beginning of ray
               Q dx = S[object][0][0] - initial_X[0];
               Q dy = S[object][1][0] - initial_X[1];
               Q dz = S[object][2][0] - initial_X[2];

               // Hey, first thing we should do is scale <dx dy dz> down so we don't
               // have overflow problems.  The following code ensures that after scaling,
               // dx, dy, and dz are all <= SMALL_ENOUGH.

#define SMALL_ENOUGH 16

               int scale = 1;
               int scale_shift = 0;
               while (abs(dx) > scale * SMALL_ENOUGH) {scale <<= 1; scale_shift++;}
               while (abs(dy) > scale * SMALL_ENOUGH) {scale <<= 1; scale_shift++;}
               while (abs(dz) > scale * SMALL_ENOUGH) {scale <<= 1; scale_shift++;}

//               Spew (DSRC_EDMS_Collide, ("\nShifting by %d, was %f %f %f\n", scale_shift, dx, dy, dz));

               dx >>= scale_shift;
               dy >>= scale_shift;
               dz >>= scale_shift;

               // Okay, see what the distance of the object to our ray is.
               // D_old is a unit vector in the direction of the ray.
               // par_component is the dot product D_old and <dx dy dz>, which is
               //   the component of the vector to the object is the D_old direction.

               Q par_component = D_old[0] * dx + D_old[1] * dy + D_old[2] * dz;

               if (par_component < 0) continue; // object is behind us

               // Since everything is scaled down, this shouldn't give us any overflow problems

               Q dist_squared = dx*dx + dy*dy + dz*dz;

               Q perp_component_squared = dist_squared - par_component * par_component;

//               Spew (DSRC_EDMS_Collide, ("Trying object %d ph %d\n", object, on2ph[object]));
//               Spew (DSRC_EDMS_Collide, ("Size %f range %f\n", size, range));
//               Spew (DSRC_EDMS_Collide, ("ray %f %f %f, delta %f %f %f\n", D_old[0], D_old[1], D_old[2], dx, dy, dz));

//               Spew (DSRC_EDMS_Collide, ("par %f dist_sq %f perp_sq %f\n", par_component, dist_squared,
//                                        perp_component_squared));

               // kill_zone will be (after we scale it back up) the distance of the center
               // of the beam to the center of the object.

               kill_zone = sqrt (perp_component_squared);

               // Now we can scale kill_zone back up.
               kill_zone <<= scale_shift;

//               Spew (DSRC_EDMS_Collide, ("kill_zone %f compared to %f\n", kill_zone, I[object][31]+size));

//             mout << "Checking PH #" << on2ph[object] << " with intersect distance " << kill_zone << "\n";
//             mout << "T1: " << top_1 << ", T2: " << top_2 << ", T3: " << top_3 << "\n";

               // We hit the object if the distance between our centers is less than
               // the sum of our radii.

               if ( kill_zone < ( I[object][31] + size) )
               {
                  kzdist = sqrt (dist_squared) << scale_shift;

                  // Make sure that
                  //   1. the object is in front of whatever terrain stopped the beam
                  //   2. it's closer than any previous objects we saw.

                  if ( ( kzdist < sampling_period*max_step - I[object][31] ) && ( kzdist < kzdisto ) )
                  {
                     victim = on2ph[object];
                     kzdisto = kzdist;

                  }
               }

            }                          // if (object != exclude)
         }                             // for (object = bit)
      }                                // if (data_word & 1)

      // Shift over the mask so we're testing the next object bit
      data_word >>= 1;
      bit++;
   }

   return victim;

}

//
//
//
// Here is the old code for reference
//
//
//

#ifdef OBSOLETE // this has been replaced by the code using the FF raycaster

// Here is the high velocity weapon primitive...
// =============================================
physics_handle EDMS_FF_cast_projectile( Q *X, // source
               Q D[3],                 // unit vector of ray
               Q speed,                // for knockback
               Q mass,                 // for knockback
               Q size,                 // radius
               Q range,                // how far it can reach
               int exclude,            // what object it cannot hit, typically the firer
               int shooter,            // not used
               long &g_info,           // information about ground hit
               long &w_info,           // information about wall hit
               bool &hit )             // whether it hit something
{
   sampling_period = size*2;           // diameter of projectile
                                       // this is four times what Seamus had it as originally
                                       // what's life without a little risk?
   int stepper = 0,
       object_pointer = 0,          //For object checks...
       max_step = ( range / sampling_period ).to_int(),     //samples per meter...
       victim_on = 0,
       shooter_on = 0;

   hash_info must_check_objects[MAX_OBJ];
   unsigned int test_data;                   // bitmask result of hash query
   unsigned int last_test_data = 0;

   physics_handle victim = -1;               //It is what is says it is...
                  
   Q ground;                        //FF terrain issues...

   int hx, hy;                         // hash location of X[]

   // Looks at terrain...
   // ===================
   fix   checker = 0;

// PRINT3D( X );
// mout << "Max: " << max_step << " :range: " << range << " : size: " << size << ".\n";

   // Save the initial vectors for the line finder...
   // ===============================================
   initial_X[0] = X[0];
   initial_X[1] = X[1];
   initial_X[2] = X[2];
   hx = floor( hash_scale*X[0] );
   hy = floor( hash_scale*X[1] );

   // Ensure that D is a unit vector

   Q anus = sqrt( D[0]*D[0] + D[1]*D[1] + D[2]*D[2] );
   if (anus < .99 || range < 1 || anus > 1.01)
   {
      D[2] = 1;
      D[1] = D[0] = 0;
#ifdef EDMS_SHIPPABLE
      mout << "Hey, raycast with zero vector!\n";
#endif
      range = 10;
   }

// mout << "Initially in square (" << floor(hash_scale*X[0]) << "," << floor(hash_scale*X[1]) << ").\n";

   // Rescale direction...
   // ====================
   D_old[0] = D[0];
   D_old[1] = D[1];
   D_old[2] = D[2];

   // Turn D into the stepping vector

   D[0] *= sampling_period;
   D[1] *= sampling_period;
   D[2] *= sampling_period;

   // Make sure the game doesn't take this too seriously...
   // =====================================================
   TFF.caller = 0;
   TFF.my_size = size.to_fix();

   // The EDMS raycaster works by first calculating the endpoint of the ray disregarding
   // objects (thus it can only be terminated by hitting terrain or ranging out).  Then
   // a front-to-back pass is made looking for objects.
   //
   // It can probably be sped up significantly by letting the game take care of the terrain
   // pass.  Using the current method, EDMS makes many terrain queries very close together.
   // If the game had control over the terrain pass, it could throw away whole areas at once.

   // Make sure we're on the map.
   if (hx < 0 || hy < 0 || hx > EDMS_DATA_SIZE - 1 || hy > EDMS_DATA_SIZE - 1)
   {
//    mout << "!EDMS: bad raycast start at:\n";
//    PRINT3D( X )
//    mout << "!EDMS: raycast excluded physics handle " << exclude << "\n";
//    flush( mout );
   }
   else
   {
      // Find impact point...
      // ====================
      for ( stepper = 0; (stepper < max_step) && (checker == 0); stepper++ )
      {
         checker = 0;

         bool maybe_hit = ff_terrain (X[0],                //Get the info...
                          X[1],
                          X[2],
                          TRUE,
                          &TFF);

         if (maybe_hit)
         {
            ground.fix_to( TFF.g_height );

            // Chexk the terrain...
            // ====================
            checker = TFF.w_x | TFF.w_y | TFF.w_z | (ground > X[2] - size);
         }

         // Check for object collisions...
         // ==============================
         test_data = data[ hx ][ hy ];

         if (test_data != last_test_data && test_data != 0)
         {
            // Save off what objects are here for second pass

            if (object_pointer < MAX_OBJ)
            {
               must_check_objects[object_pointer].hx   = hx;
               must_check_objects[object_pointer].hy   = hy;
               must_check_objects[object_pointer].bits = test_data;
               object_pointer++;
            }
            last_test_data = test_data;
         }
         
         // Move the check point...
         // =======================
         X[0] += D[0];
         X[1] += D[1];
         X[2] += D[2];

         hx = floor( hash_scale*X[0] );
         hy = floor( hash_scale*X[1] );

         if (hx < 0 || hy < 0 || hx > EDMS_DATA_SIZE - 1 || hy > EDMS_DATA_SIZE - 1)
         {
            break; // off map
         }
      }

//    Spew (DSRC_EDMS_Collide, ("made %d terrain checks from %f %f %f to %f %f %f\n", stepper, initial_X[0],
//       initial_X[1], initial_X[2], X[0], X[1], X[2]));


      // Save the final point of the line segment...
      // ===========================================
      final_X[0] = X[0];
      final_X[1] = X[1];
      final_X[2] = X[2];

      // Now check for objects

      int i;
      for (i = 0; i < object_pointer; i++)
      {
         victim = FF_object_check (must_check_objects[i].bits, must_check_objects[i].hx, must_check_objects[i].hy,
                                   size, range, exclude, stepper);

         if (victim != -1)
         {
            // We hit someone!

            victim_on = ph2on[victim];
            Q iota_c = 20*I[victim_on][36]*mass*speed;

            // Knock back our victim

            I[victim_on][32] = D_old[0]*iota_c;
            I[victim_on][33] = D_old[1]*iota_c;
            I[victim_on][34] = D_old[2]*iota_c;

            I[victim_on][35] = 1;      //Deweet!

            no_no_not_me[victim_on] = 1;  //Make sure we're up...

            break;                  // All done looking!
         }
      }

      // If we did, in face, hit a wall, the 3D system precision may be insufficient to sort the hit
      // art in front of the wall.  Therefore...
      // =======================================
      if ( victim > -1 )
      {
         X[0] = S[victim_on][0][0] - D[0];   //Simplest thing for now...
         X[1] = S[victim_on][1][0] - D[1];
         X[2] = S[victim_on][2][0] - D[2];
      }
      else
      {
         X[0] -= D[0];
         X[1] -= D[1];
         X[2] -= D[2];
      }


      // Did we hit a wall, or did we hit range out?
      // ===========================================
      hit = FALSE;
      g_info =
      w_info = 0;

      // Hit walls...
      // ------------
      if (TFF.w_x | TFF.w_y | TFF.w_z)
      {
         g_info = 0;
         w_info = TFF.DATA2;
         hit = TRUE;
      }

      // Hit ground...
      // -------------
      if ( ground > X[2] - size )
      {
         g_info = TFF.DATA1;  //Check for ground...
         w_info = 0;
         hit = TRUE;
      }  

      // Did we range out?
      // -----------------
      if ( stepper == max_step )
      {           //Check for rangeout...
         hit = FALSE;
         g_info = 
         w_info = 0;
      }


      // Did we hit somebody?
      // --------------------
      if( victim != -1 )
      {
         hit = TRUE;
         g_info =
         w_info = 0;
      }
   }

   return victim;
}





// Here, since we know the line segment we're interested in, we check to make sure that we
// didn't hit any objects, and return the one we did...
// ====================================================
physics_handle FF_object_check( unsigned int data_word, int hx, int hy, Q size, Q range, int exclude, int stepper )
{
   // General purpose...
   // ==================
   int object;
   physics_handle victim = -1;

   // For the lines...
   // ================
   Q  a = final_X[0] - initial_X[0],
      b = final_X[1] - initial_X[1],
      c = final_X[2] - initial_X[2],
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
               if (!object_check_hash (object, hx, hy)) continue; // didn't mean this one

               // Distances of potentially hit object to beginning of ray
               Q dx = S[object][0][0] - initial_X[0];
               Q dy = S[object][1][0] - initial_X[1];
               Q dz = S[object][2][0] - initial_X[2];

               // Hey, first thing we should do is scale <dx dy dz> down so we don't
               // have overflow problems.  The following code ensures that after scaling,
               // dx, dy, and dz are all <= SMALL_ENOUGH.

#define SMALL_ENOUGH 16

               int scale = 1;
               int scale_shift = 0;
               while (abs(dx) > scale * SMALL_ENOUGH) {scale <<= 1; scale_shift++;}
               while (abs(dy) > scale * SMALL_ENOUGH) {scale <<= 1; scale_shift++;}
               while (abs(dz) > scale * SMALL_ENOUGH) {scale <<= 1; scale_shift++;}

//               Spew (DSRC_EDMS_Collide, ("\nShifting by %d, was %f %f %f\n", scale_shift, dx, dy, dz));

               dx >>= scale_shift;
               dy >>= scale_shift;
               dz >>= scale_shift;

               // Okay, see what the distance of the object to our ray is.
               // D_old is a unit vector in the direction of the ray.
               // par_component is the dot product D_old and <dx dy dz>, which is
               //   the component of the vector to the object is the D_old direction.

               Q par_component = D_old[0] * dx + D_old[1] * dy + D_old[2] * dz;

               if (par_component < 0) continue; // object is behind us

               // Since everything is scaled down, this shouldn't give us any overflow problems

               Q dist_squared = dx*dx + dy*dy + dz*dz;

               Q perp_component_squared = dist_squared - par_component * par_component;

//               Spew (DSRC_EDMS_Collide, ("Trying object %d ph %d\n", object, on2ph[object]));
//               Spew (DSRC_EDMS_Collide, ("Size %f range %f\n", size, range));
//               Spew (DSRC_EDMS_Collide, ("ray %f %f %f, delta %f %f %f\n", D_old[0], D_old[1], D_old[2], dx, dy, dz));

//               Spew (DSRC_EDMS_Collide, ("par %f dist_sq %f perp_sq %f\n", par_component, dist_squared,
//                                        perp_component_squared));

               // kill_zone will be (after we scale it back up) the distance of the center
               // of the beam to the center of the object.

               kill_zone = sqrt (perp_component_squared);

               // Now we can scale kill_zone back up.
               kill_zone <<= scale_shift;

//               Spew (DSRC_EDMS_Collide, ("kill_zone %f compared to %f\n", kill_zone, I[object][31]+size));

//             mout << "Checking PH #" << on2ph[object] << " with intersect distance " << kill_zone << "\n";
//             mout << "T1: " << top_1 << ", T2: " << top_2 << ", T3: " << top_3 << "\n";

               // We hit the object if the distance between our centers is less than
               // the sum of our radii.

               if ( kill_zone < ( I[object][31] + size) )
               {
                  kzdist = sqrt (dist_squared) << scale_shift;

                  // Make sure that
                  //   1. the object is in front of whatever terrain stopped the beam
                  //   2. it's closer than any previous objects we saw.

                  if ( ( kzdist < sampling_period*stepper - I[object][31] ) && ( kzdist < kzdisto ) )
                  {
                     victim = on2ph[object];
                     kzdisto = kzdist;

                  }
               }

            }                          // if (object != exclude)
         }                             // for (object = bit)
      }                                // if (data_word & 1)

      // Shift over the mask so we're testing the next object bit
      data_word >>= 1;
      bit++;
   }

   return victim;

}

#endif // OBSOLETE
