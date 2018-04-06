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
 * $Source: n:/project/cit/src/RCS/objprop.c $
 * $Revision: 1.23 $
 * $Author: tjs $
 * $Date: 1994/02/26 19:38:26 $
 *
 * $Log: objprop.c $
 * Revision 1.23  1994/02/26  19:38:26  tjs
 * Got disgusted with giant case statement in num_types, replaced it
 * with a giant table.  Executable shrinks by 4K.
 * 
 * Revision 1.22  1994/02/03  17:01:11  minman
 * new ammo\gun regime'
 * 
 * Revision 1.21  1994/01/14  15:21:47  xemu
 * smallstuff plot
 * 
 * Revision 1.20  1994/01/09  03:33:16  xemu
 * load frames from art
 * 
 * Revision 1.19  1993/12/14  15:10:48  xemu
 * better frame warning
 * 
 * Revision 1.18  1993/12/08  22:01:15  xemu
 * more spew, added objart3
 * 
 * Revision 1.17  1993/11/22  19:47:31  xemu
 * sanity checker
 * 
 * Revision 1.16  1993/10/08  00:56:49  xemu
 * The Object Millienia is HERE
 * 
 * Revision 1.15  1993/10/01  23:53:38  xemu
 * new object regime
 * 
 * Revision 1.14  1993/09/02  23:02:33  xemu
 * angle!
 * 
 * Revision 1.13  1993/08/19  20:19:57  jojak
 * revamped object hierarchy
 * 
 * Revision 1.12  1993/08/17  21:52:51  minman
 * fixed some bugs with assuming subclasses always 16
 * also added get_nth_from triple
 * 
 * Revision 1.11  1993/08/10  15:52:34  xemu
 * removed trap subclass
 * 
 * Revision 1.10  1993/08/06  16:03:58  minman
 * changed software subclasse
 * 
 * Revision 1.9  1993/08/05  19:16:24  mahk
 * Added nth_after_triple
 * 
 * Revision 1.8  1993/08/05  14:02:27  minman
 * changed to new object properties order
 * 
 * Revision 1.7  1993/08/04  02:08:23  minman
 * squished the grenade/drug list to 7 types a piece
 * 
 * Revision 1.6  1993/08/03  23:15:06  minman
 * added transitory animating subclass
 * 
 * Revision 1.5  1993/08/02  20:39:25  spaz
 * Fixed get_triple_From_class_nth_item()
 * 
 * Revision 1.4  1993/08/01  23:24:03  spaz
 * get_triple_from_class_nth_item() func
 * 
 * Revision 1.3  1993/07/31  20:49:29  minman
 * changed class/subclass hierarchy - no more shotguns
 * 
 * Revision 1.2  1993/07/26  00:49:22  minman
 * added comments
 * 
 * Revision 1.1  1993/07/25  23:16:00  minman
 * Initial revision
 * 
 *
 */

#include "objclass.h"
#include "objprop.h"

// ----------------------------------------------
// num_types()
//
// ## INSERT NEW CLASS HERE
// ## INSERT NEW SUBCLASS HERE

#define MAX_SUBCLASSES 8

// Curse us for having both NUM_OBJECT_ANIMATING and NUM_OBJECTS_ANIMATING.
// Note that NUM_OBJECT_ANIMATING is the only constant in this list that
// breaks our naming convention, being as it is the number of types in the
// subclass ANIMATING_SUBCLASS_OBJECTS, not ANIMATING_SUBCLASS_OBJECT.

static uchar numtypes_array[NUM_CLASSES][MAX_SUBCLASSES] = {
    { NUM_PISTOL_GUN, NUM_AUTO_GUN, NUM_SPECIAL_GUN, NUM_HANDTOHAND_GUN,
      NUM_BEAM_GUN, NUM_BEAMPROJ_GUN, },
    { NUM_PISTOL_AMMO, NUM_NEEDLE_AMMO, NUM_MAGNUM_AMMO, NUM_RIFLE_AMMO,
      NUM_FLECHETTE_AMMO, NUM_AUTO_AMMO, NUM_PROJ_AMMO, },
    { NUM_TRACER_PHYSICS, NUM_SLOW_PHYSICS, NUM_CAMERA_PHYSICS },
    { NUM_DIRECT_GRENADE, NUM_TIMED_GRENADE, },
    { NUM_STATS_DRUG, },
    { NUM_GOGGLE_HARDWARE, NUM_HARDWARE_HARDWARE, },
    { NUM_OFFENSE_SOFTWARE, NUM_DEFENSE_SOFTWARE, NUM_ONESHOT_SOFTWARE,
      NUM_MISC_SOFTWARE, NUM_DATA_SOFTWARE, },
    { NUM_ELECTRONIC_BIGSTUFF, NUM_FURNISHING_BIGSTUFF, NUM_ONTHEWALL_BIGSTUFF,
      NUM_LIGHT_BIGSTUFF, NUM_LABGEAR_BIGSTUFF, NUM_TECHNO_BIGSTUFF,
      NUM_DECOR_BIGSTUFF, NUM_TERRAIN_BIGSTUFF, },
    { NUM_USELESS_SMALLSTUFF, NUM_BROKEN_SMALLSTUFF, NUM_CORPSELIKE_SMALLSTUFF,
      NUM_GEAR_SMALLSTUFF, NUM_CARDS_SMALLSTUFF, NUM_CYBER_SMALLSTUFF,
      NUM_ONTHEWALL_SMALLSTUFF, NUM_PLOT_SMALLSTUFF, },
    { NUM_CONTROL_FIXTURE, NUM_RECEPTACLE_FIXTURE, NUM_TERMINAL_FIXTURE,
      NUM_PANEL_FIXTURE, NUM_VENDING_FIXTURE, NUM_CYBER_FIXTURE, },
    { NUM_NORMAL_DOOR, NUM_DOORWAYS_DOOR, NUM_FORCE_DOOR, NUM_ELEVATOR_DOOR,
      NUM_SPECIAL_DOOR, },
    { NUM_OBJECT_ANIMATING, NUM_TRANSITORY_ANIMATING, NUM_EXPLOSION_ANIMATING, },
    { NUM_TRIGGER_TRAP, NUM_FEEDBACKS_TRAP, NUM_SECRET_TRAP, },
    { NUM_ACTUAL_CONTAINER, NUM_WASTE_CONTAINER, NUM_LIQUID_CONTAINER,
      NUM_MUTANT_CORPSE_CONTAINER, NUM_ROBOT_CORPSE_CONTAINER,
      NUM_CYBORG_CORPSE_CONTAINER, NUM_OTHER_CORPSE_CONTAINER, },
    { NUM_MUTANT_CRITTER, NUM_ROBOT_CRITTER, NUM_CYBORG_CRITTER,
      NUM_CYBER_CRITTER, NUM_ROBOBABE_CRITTER, }
};

short num_types(uchar obclass, uchar subclass)
{
   if (obclass>=NUM_CLASSES || subclass>=num_subclasses[obclass])
   {
//      Warning(("Class and subclass given isn't a valid pair. Class - %d Subclass - %d.\n", obclass, subclass));
      return(0);
   }

   return(numtypes_array[obclass][subclass]);
}

// ---------------------------------------------------------------------------
// get_triple_from_class_nth_item()
//
// If you specify the nth overall item of a class, this function will
// return the corresponding triple.  Returns -1 if the nth item of
// the class did not exist.

int get_triple_from_class_nth_item(uchar obclass, uchar n)
{
   ubyte i, subclass_types, total_types;
   int   triple;

   total_types = 0;

   // Cycle through all subclasses for this obclass
   for (i = 0; i < num_subclasses[obclass]; i++) {

      // Skip the entire next subclass
      subclass_types = num_types(obclass, i);
      total_types   += subclass_types;

      // Aha! We've found or gone past the desired triple
      if (total_types > n) {

         triple = MAKETRIP(obclass, i, n - (total_types - subclass_types));
         return triple;
      }
   }

   // We've gone through all subclasses, and still haven't found what we're looking
   // for.  Ah well...
//   Warning(("Invalid obclass and n given to get_triple_from_class_nth_item"));
//   Warning(("Class - %d N - %d\n", obclass, n));
   return -1;
}   


// ---------------------------------------------------------------------------
// nth_after_triple()
//
// Returns the nth valid object triple after base

int nth_after_triple(int base, uchar n)
{
   ubyte i, subclass_types, total_types;
   int   triple;
   ubyte obclass;

   total_types = 0;

   // Cycle through all 16 members of the specified class
   for (obclass = TRIP2CL(base); obclass < NUM_CLASSES; obclass++) 
      for (i = TRIP2SC(base); i < num_subclasses[obclass]; i++)
      {

         // Skip the entire next subclass
         subclass_types = num_types(obclass, i);
         total_types   += subclass_types;

         // Aha! We've found or gone past the desired triple
         if (total_types > n)
         {
            triple = MAKETRIP(obclass, i, n - (total_types - subclass_types));
            return triple;
         }
      }

   // We've gone through all 16, and still haven't found what we're looking
   // for.  Ah well...

   return -1;
}


// ------------------------------------------------------------------
// get_nth_from_triple()
//

int get_nth_from_triple(int triple)
{
   ubyte    obclass, subclass;
   int      n, j;

   obclass = TRIP2CL(triple);
   subclass = TRIP2SC(triple);

   if (obclass > NUM_CLASSES)
      return(0);
   if (subclass > num_subclasses[obclass])
      return(0);

   for (n=0,j=0;j<subclass;j++)
      n += num_types(obclass, j);

   n += TRIP2TY(triple);

   return(n);
}

/*  Don't need this for Mac version!!

// ------------------------------------------------------------------
// sanity_check_obj_props()
//
// Performs a basic sanity check on the object properties.  Useful for making 
// sure that data hasn't been corrupted, either by bad load-time data or memory
// trashes.  
//
// Initially, this will probably be pretty bonehead and straightforward, but hopefully
// it will get more stoked as time goes by and people decide to add stuff to it.

#define MAX_SIZE  253
#define MAX_PEP   253
#define MAX_HARDNESS 253

errtype sanity_check_obj_props()
{
   int i;
   extern Id posture_bases[];
   extern Id critter_id_table[];

   for (i = 0; i < NUM_OBJECT; i++)
   {
      if (ObjProps[i].physics_model != 0)
      {
         if (ObjProps[i].hardness > MAX_HARDNESS)
            Warning(("object index %d, triple 0x%x, has hardness %d!\n",i,nth_after_triple(0,i),
               ObjProps[i].hardness));
         if (ObjProps[i].mass < 0)
            Warning(("object index %d, triple 0x%x, has mass %d!\n",i,nth_after_triple(0,i),
               ObjProps[i].mass));
         if (ObjProps[i].physics_xr > MAX_SIZE)
            Warning(("object index %d, triple 0x%x, has physics_xr %d!\n",i,nth_after_triple(0,i),
               ObjProps[i].physics_xr));
         if (ObjProps[i].pep > MAX_PEP)
            Warning(("object index %d, triple 0x%x, has pep %d (model %d) !\n",i,nth_after_triple(0,i),
               ObjProps[i].pep,ObjProps[i].physics_model));
      }
   }
   return(OK);
}
*/
