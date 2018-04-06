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
 * $Source: r:/prj/cit/src/RCS/combat.c $
 * $Revision: 1.77 $
 * $Author: minman $
 * $Date: 1994/09/06 17:32:11 $
 *
 */

#define __COMBAT_C

#include <stdio.h>
#include <string.h>

#include "combat.h"
#include "objsim.h"
#include "objprop.h"
#include "damage.h"
#include "screen.h"
#include "tools.h"
#include "effect.h"
#include "otrip.h"
#include "fullscrn.h"

#include "gameloop.h"
#include "mainloop.h"
#include "player.h"

#include "frtypes.h"
#include "frprotox.h"
#include "cybrnd.h"
#include "physunit.h"


bool bullet_debug = FALSE;
ObjID terrain_hit_obj = OBJ_NULL;
ObjID terrain_hit_exclusion = OBJ_NULL;

ObjID simple_ray_caster(Combat_Ray *ray);

extern void test_object_link(void);

extern physics_handle ray_cast_wrapper(fix X[3], fix D[3], fix speed, fix mass, fix size, fix range, physics_handle exclusion);

physics_handle ray_cast_wrapper(fix X[3], fix D[3], fix speed, fix mass, fix size, fix range, physics_handle exclusion)
{
   physics_handle    ph;
   physics_handle    source;

   // give it a little more distance
   range += fix_make(0,0x4000);

   if (exclusion == objs[PLAYER_OBJ].info.ph)
      source = objs[PLAYER_OBJ].info.ph;
   else
      source = -1;

   ph = EDMS_beam_weapon(X, D, speed, mass, size, range, exclusion, source);

   return(ph);
}


// --------------------------------------------------------------------------------------------------
// simple_ray_caster()
//
// NOTE: assumes that the ray to be casted is normalized!
// 

ObjID simple_ray_caster(Combat_Ray *ray)
{
   physics_handle ph;
   fix            src[3];
   fix            dest[3];

   // copy over the source of the raycast
   src[0] = ray->origin.x; src[1] = ray->origin.y; src[2] = ray->origin.z;
   dest[0] = ray->dx; dest[1] = ray->dy; dest[2] = ray->dz;

   terrain_hit_obj = OBJ_NULL;
//   terrain_hit_exclusion = physics_handle_to_id(ray->exclusion);

   ph = ray_cast_wrapper(src, dest, ray->speed, ray->mass, ray->size, ray->range, ray->exclusion);
   terrain_hit_exclusion = OBJ_NULL;

   ray->origin.x = src[0]; ray->origin.y = src[1]; ray->origin.z = src[2];

   if (ph == -1)     // physics handle NULL is valid......
   {
      return(terrain_hit_obj);      // if we didn't hit terrain object - it'll be OBJ_NULL;
   }
   else
   {
      ray->origin.x = src[0]; ray->origin.y = src[1]; ray->origin.z = src[2];

      return(physics_handle_to_id(ph));
   }
}


// -------------------------------------------------
// ray_cast_attack()
//

ObjID ray_cast_attack(ObjID src, ObjLoc dest, fix bullet_mass, fix bullet_size, fix bullet_speed, fix bullet_range)
{
   Combat_Ray     ray;
   Combat_Pt      vector;
   fix            dist;

   // compute the location of the source object
   ray.origin.x = fix_from_obj_coord(objs[src].loc.x);
   ray.origin.y = fix_from_obj_coord(objs[src].loc.y);
   ray.origin.z = fix_from_obj_height(src);

   // compute the vector to the destination object
   vector.x = fix_from_obj_coord(dest.x) - ray.origin.x;
   vector.y = fix_from_obj_coord(dest.y) - ray.origin.y;
   vector.z = fix_from_obj_height_val(dest.z) - ray.origin.z;

   // normalize it baby!
   dist = fix_sqrt(fix_mul(vector.x, vector.x) + fix_mul(vector.y, vector.y) + fix_mul(vector.z, vector.z));
   ray.dx = fix_div(vector.x, dist);
   ray.dy = fix_div(vector.y, dist);
   ray.dz = fix_div(vector.z, dist);

   ray.mass = bullet_mass;
   ray.size = bullet_size;
   ray.speed = bullet_speed;
   ray.range = bullet_range;
   ray.exclusion = (src == OBJ_NULL) ? -1 : objs[src].info.ph;
   
   return(simple_ray_caster(&ray));
}

// -------------------------------------------
// ray_cast_points()
//

ObjID ray_cast_points(ObjID exclusion, Combat_Pt src, Combat_Pt dest, fix bullet_mass, fix bullet_size, fix bullet_speed, fix bullet_range)
{
   Combat_Ray     ray;
   fix            dist;
   ObjID          target;

   ray.origin = src;
   ray.dx = dest.x - src.x;
   ray.dy = dest.y - src.y;
   ray.dz = dest.z - src.z;

   // normalize vector and convert it
   dist = fix_sqrt(fix_mul(ray.dx, ray.dx) + fix_mul(ray.dy, ray.dy) + fix_mul(ray.dz, ray.dz));
   ray.dx = fix_div(ray.dx, dist);
   ray.dy = fix_div(ray.dy, dist);
   ray.dz = fix_div(ray.dz, dist);

   ray.mass = bullet_mass;
   ray.size = bullet_size;
   ray.speed = bullet_speed;
   ray.range = bullet_range;
   ray.exclusion = (exclusion == OBJ_NULL) ? -1 : objs[exclusion].info.ph;

   target = simple_ray_caster(&ray);
   return(target);
}

// -------------------------------------------
// ray_cast_vector()
//

ObjID ray_cast_vector(ObjID exclusion, Combat_Pt *src, Combat_Pt vector, fix bullet_mass, fix bullet_size, fix bullet_speed, fix bullet_range)
{
   Combat_Ray  ray;
   ObjID       target;

   ray.origin = *src;
   ray.dx = vector.x;
   ray.dy = vector.y;
   ray.dz = vector.z;

   ray.mass = bullet_mass;
   ray.size = bullet_size;
   ray.speed = bullet_speed;
   ray.range = bullet_range;
   ray.exclusion = (exclusion == OBJ_NULL) ? -1 : objs[exclusion].info.ph;
 
   target = simple_ray_caster(&ray);

   // save the location of the hit
   src->x = ray.origin.x;
   src->y = ray.origin.y;
   src->z = ray.origin.z;

   return(target);
}

// -------------------------------------------
// ray_cast_objects()
//
ObjID ray_cast_objects(ObjID src, ObjID dest, fix bullet_mass, fix bullet_size, fix bullet_speed, fix bullet_range)
{
   Combat_Ray     ray;
   Combat_Pt      vector;
   fix            dist;
   Combat_Pt      target_loc;

   // compute the location of the source object
   ray.origin.x = fix_from_obj_coord(objs[src].loc.x);
   ray.origin.y = fix_from_obj_coord(objs[src].loc.y);

   // shift up by half it's radius
   ray.origin.z = fix_from_obj_height(src) + (fix_make(ObjProps[OPNUM(src)].physics_xr,0) / (PHYSICS_RADIUS_UNIT<<1));

   if (objs[dest].info.ph != -1)
   {
      State             new_state;
      void get_phys_state(int ph, State *new_state, ObjID id);

      get_phys_state(objs[dest].info.ph, &new_state,dest);
      target_loc.x = new_state.X;   target_loc.y = new_state.Y;   target_loc.z = new_state.Z;
   }
   else
   {
      // ray cast - even though since the object has no physics handle, we won't hit it.
      // but we might hit something in the way.

      target_loc.x = fix_from_obj_coord(objs[dest].loc.x);
      target_loc.y = fix_from_obj_coord(objs[dest].loc.y);
      target_loc.z = fix_from_obj_height(dest) + (fix_make(ObjProps[OPNUM(src)].physics_xr,0) / (PHYSICS_RADIUS_UNIT<<1));
   }

   // compute the vector to the destination object
   vector.x = target_loc.x - ray.origin.x;
   vector.y = target_loc.y - ray.origin.y;
   vector.z = target_loc.z - ray.origin.z;

   // normalize vector and convert it
   dist = fix_sqrt(fix_mul(vector.x, vector.x) + fix_mul(vector.y, vector.y) + fix_mul(vector.z, vector.z));
   ray.dx = fix_div(vector.x, dist);
   ray.dy = fix_div(vector.y, dist);
   ray.dz = fix_div(vector.z, dist);

   ray.mass = bullet_mass;
   ray.size = bullet_size;
   ray.speed = bullet_speed;
   ray.range = bullet_range;
   ray.exclusion = (src == OBJ_NULL) ? -1 : objs[src].info.ph;
 
   return(simple_ray_caster(&ray));
}


extern g3s_vector main_view_vectors[];

// ----------------------------------------------
// find_fire_vector()
//

void find_fire_vector(LGPoint *pt, Combat_Pt *vector)
{
   fix         x1, x2, y1, y2, z1, z2;
   fix         dist;
   int         x, y;

   x = pt->x - ((fauxrend_context *)_current_fr_context)->xtop;
   y = pt->y - ((fauxrend_context *)_current_fr_context)->ytop;

   // view vectors go something like this
   // vector[0] - upper right
   // vector[1] - lower right
   // vector[2] - lower left
   // vector[3] - upper left

   x1 = main_view_vectors[2].gX + (fix_mul((main_view_vectors[1].gX - main_view_vectors[2].gX), fix_make(x,0)))/((fauxrend_context *)_current_fr_context)->xwid;
   x2 = main_view_vectors[3].gX + (fix_mul((main_view_vectors[0].gX - main_view_vectors[3].gX), fix_make(x,0)))/((fauxrend_context *)_current_fr_context)->xwid;

   // negative because we're changing coordinate frames
   y1 = -main_view_vectors[2].gY + (fix_mul((main_view_vectors[2].gY - main_view_vectors[1].gY), fix_make(x,0)))/((fauxrend_context *)_current_fr_context)->xwid;
   y2 = -main_view_vectors[3].gY + (fix_mul((main_view_vectors[3].gY - main_view_vectors[0].gY), fix_make(x,0)))/((fauxrend_context *)_current_fr_context)->xwid;

   z1 = main_view_vectors[2].gZ + (fix_mul((main_view_vectors[1].gZ - main_view_vectors[2].gZ), fix_make(x,0)))/((fauxrend_context *)_current_fr_context)->xwid;
   z2 = main_view_vectors[3].gZ + (fix_mul((main_view_vectors[0].gZ - main_view_vectors[3].gZ), fix_make(x,0)))/((fauxrend_context *)_current_fr_context)->xwid;

   vector->x = x2 + (fix_mul((x1-x2 ),fix_make(y,0)))/((fauxrend_context *)_current_fr_context)->ywid;
   vector->y = z2 + (fix_mul((z1-z2),fix_make(y,0)))/((fauxrend_context *)_current_fr_context)->ywid;
   vector->z = y2 + (fix_mul((y1-y2),fix_make(y,0)))/((fauxrend_context *)_current_fr_context)->ywid;

   dist = fix_sqrt(fix_mul(vector->x, vector->x) + fix_mul(vector->y, vector->y) + fix_mul(vector->z, vector->z));
   vector->x = fix_div(vector->x, dist);
   vector->y = fix_div(vector->y, dist);
   vector->z = fix_div(vector->z, dist); 
}

