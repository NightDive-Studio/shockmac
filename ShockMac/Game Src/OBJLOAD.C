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
#include <string.h>

#include "Shock.h"

#include "criterr.h"
#include "objects.h"
#include "objload.h"
#include "player.h"
#include "objart.h"
#include "objbit.h"
#include "citres.h"
#include "treasure.h"
#include "otrip.h"
#include "gameobj.h"
#include "statics.h"
#include "cybmem.h"

#define OBJECT_ART_BASE    RES_bmObjectIcons

errtype voxel_convert(grs_bitmap *bmp);
void load_treasure_table(uchar *loadme, char cp);
void compute_complex_loadage(uchar *loadme);
grs_bitmap *get_objbitmap_from_pool(int i, bool t);


// Transform the bitmap from a greyscale drawing to an actual 0-16 depth map
// For now we simply do this by subtracting 208
errtype voxel_convert(grs_bitmap *bmp)
{
   int x,y;
   for (x=0; x < bmp->w; x++)
      for (y=0; y < bmp->h; y++)
         if (bmp->bits[(y * bmp->w) + x] != 0)
            bmp->bits[(y * bmp->w) + x] -= 208;
   return(OK);
}

void load_treasure_table(uchar *loadme, char cp)
{
   int j,t0,t1;
   for (j=0; j < NUM_TREASURE_SLOTS; j++)
   {
      t0 = treasure_table[cp][j][0];
      t1 = treasure_table[cp][j][1];
      if (t0 == 0)
         break;
      if (t1 == NOTHING_TRIPLE)
         continue;
      ObjLoadMeSet(OPTRIP(t1));
   }
}

#define FIRST_CORPSE_TTYPE 11

void compute_complex_loadage(uchar *loadme)
{
   int i,tr,objtrip;
   ObjID id;
   weapon_slot wpnslot;

   // Figure out what to load
   ObjLoadMeClearAll();

   // If it's on the level, load it, with some specialized hacks
   FORALLOBJS(id)
   {
      objtrip = ID2TRIP(id);
      switch(objtrip)
      {
         case ANTENNA_PAN_TRIPLE:
            ObjLoadMeSet(OPTRIP(PLAS_ANTENNA_TRIPLE));
            ObjLoadMeSet(OPTRIP(DEST_ANTENNA_TRIPLE));
            break;
         case FAUX_X_TRIPLE:
            ObjLoadMeSet(OPTRIP(ISOTOPE_X_TRIPLE));
            break;
         case ISOTOPE_X_TRIPLE:
            ObjLoadMeSet(OPTRIP(FAUX_X_TRIPLE));
            break;
      }
      ObjLoadMeSet(OPNUM(id));
   }

   // Also load art for things player can carry that aren't
   // in the object system but are in the player_struct
   for (i=0; i < NUM_WEAPON_SLOTS; i++)
   {
      wpnslot = player_struct.weapons[i];
      if (wpnslot.type != EMPTY_WEAPON_SLOT)
         ObjLoadMeSet(OPTRIP(MAKETRIP(CLASS_GUN, wpnslot.type, wpnslot.subtype)));
   }
   tr =OPTRIP(MAKETRIP(CLASS_HARDWARE,0,0));
   for (i=0; i < NUM_HARDWAREZ; i++)
      if (player_struct.hardwarez[i])
         ObjLoadMeSet(tr + i);
   tr = OPTRIP(MAKETRIP(CLASS_AMMO, 0,0));
   for (i=0; i < NUM_AMMO_TYPES; i++)
      if (player_struct.cartridges[i] || player_struct.partial_clip[i])
         ObjLoadMeSet(tr + i);
   tr = OPTRIP(MAKETRIP(CLASS_DRUG, 0,0));
   for (i=0; i < NUM_DRUGZ; i++)
      if (player_struct.drugs[i])
         ObjLoadMeSet(tr + i);
   tr = OPTRIP(MAKETRIP(CLASS_GRENADE, 0,0));
   for (i=0; i < NUM_GRENADEZ; i++)
      if (player_struct.grenades[i])
         ObjLoadMeSet(tr + i);

   // Load art for things that can get generated over the course of events
   // such as explosions and slow projectiles and loot
   tr = OPTRIP(MAKETRIP(CLASS_ANIMATING, ANIMATING_SUBCLASS_TRANSITORY, 0));
   for (i=0; i < NUM_TRANSITORY_ANIMATING + NUM_EXPLOSION_ANIMATING; i++)
      ObjLoadMeSet(tr + i);
   tr = OPTRIP(MAKETRIP(CLASS_PHYSICS, PHYSICS_SUBCLASS_SLOW, 0));
   for (i=0; i < NUM_SLOW_PHYSICS; i++)
      ObjLoadMeSet(tr + i);
   for (i=0; i < NUM_TREASURE_TYPES; i++)
      load_treasure_table(loadme,i);

   // Load corpses for any creatures on the level as well as their loot
   tr = OPTRIP(MAKETRIP(CLASS_CRITTER, 0, 0));
   for (i = tr; i < tr+NUM_CRITTER; i++)
   {
      // We have loaded that creature's "art", so load in it's corpse too
      if ((ObjLoadMeCheck(i)) && (CritterProps[i-tr].corpse))
         ObjLoadMeSet(OPTRIP(CritterProps[i-tr].corpse));
   }
}

// Wow, this is stupid, but is really wanted for ease
// of integration
// t    is 0 for 2d bitmaps
//         1 for 3d bitmaps
grs_bitmap *get_objbitmap_from_pool(int i, bool t)
{
//   Warning(("objbitmap_from_pool (%d, %d, %d vs %d)\n",i,t,(t*NUM_OBJECT) + i,OBJ_BITMAP_POOL_SIZE));
   if (((t * NUM_OBJECT) + i) > OBJ_BITMAP_POOL_SIZE)
   {
      return(NULL);
   }
   return(&obj_bitmap_pool[(t * NUM_OBJECT) + i]);
}

// Scan through all objects on the level, load all pertinent art
// and punt all irrelevant art
// NEEDED: empty bitmap support & defaulting?
//         bitmap zero support
ulong objart_loadsize = 0;

#define APPROX_REF_TAB_SIZE 7000

static bool bitmap_zero_loaded = FALSE;

errtype obj_load_art(bool flush_all)
{
   uchar loadme[NUM_OBJECT_BIT_LEN];
   LGRect dummy_anchor;
   short objart_count = 0, count_3d = 0;
   int objfnum;
   RefTable *prt;
   short i,f;
   extern bool empty_bitmap(grs_bitmap *bmp);
   bool ref_buffer_used = TRUE;

   if (flush_all)
      ObjLoadMeClearAll();
   else
      compute_complex_loadage((uchar *)loadme);

   // If low memory, see if what we are currently trying to do is any
   // different at all than current loadage.  If yes, flush all first.  
   // If no, punt out now, duh.
   if ((!flush_all) && (start_mem < BIG_CACHE_THRESHOLD))
   {
      bool different = FALSE;
      for (i=0; (i < NUM_OBJECT) && !different; i++)
      {
         if ((ObjLoadMeCheck(i) && (bitmaps_2d[i] == NULL)) ||
             (!ObjLoadMeCheck(i) && (bitmaps_2d[i] != NULL)))
         {
            different = TRUE;
         }
      }
      if (different)
         obj_load_art(TRUE);
      else
         return(OK);
   }

	// Open the damn file
	if (!flush_all)
	{
		FSSpec	fSpec;
		
		FSMakeFSSpec(gCDDataVref, gCDDataDirID, "\pobjart.rsrc", &fSpec);
		objfnum = ResOpenFile(&fSpec);
		if (objfnum < 0)
			critical_error(CRITERR_RES|5);
		
		prt = (RefTable *)(frameBuffer+sizeof(frameBuffer)-APPROX_REF_TAB_SIZE);
		
/* KLC  Changed to the following.
		if(ResExtractRefTable(OBJECT_ART_BASE, prt, APPROX_REF_TAB_SIZE))
		{
			prt = ResReadRefTable(OBJECT_ART_BASE);
			ref_buffer_used=FALSE;
		}
*/
		prt = ResReadRefTable(OBJECT_ART_BASE);
		if (prt)
			ref_buffer_used=FALSE;

		// Read out the infamous bitmap zero
		if (!bitmap_zero_loaded)
		{
			bitmaps_3d[count_3d] = get_objbitmap_from_pool(count_3d,1);	// KLC - added here.
			load_bitmap_from_res(bitmaps_3d[count_3d], OBJECT_ART_BASE, objart_count++, prt, TRUE, &dummy_anchor, NULL);
			bitmap_zero_loaded = TRUE;
			objart_loadsize += bitmaps_3d[count_3d]->w * bitmaps_3d[count_3d]->h;
			anchors_3d[count_3d] = dummy_anchor.ul;
		}
		else
			objart_count++;
	}
	count_3d++;


   // Iterate through all the art, picking out what we like
   // and skipping over what we dont (also freeing any memory that
   // we grabbed for earlier levels that is now irrelevant).
   // Make sure to skip over things already loaded, etc.
   for (i=0; i < NUM_OBJECT; i++)
   {
      if (ObjLoadMeCheck(i))
      {
//         mprintf("load bitmaps_2d[%d] = %x count_3d = %x ObjProps[%d].bitmap_3d = %x\n",i,bitmaps_2d[i],count_3d,i,
//            ObjProps[i].bitmap_3d);
         // If we're not in memory, load us
         if (bitmaps_2d[i] == NULL)
         {
            bitmaps_2d[i] = get_objbitmap_from_pool(i,0);
            load_bitmap_from_res(bitmaps_2d[i], OBJECT_ART_BASE, objart_count++, prt, TRUE, &dummy_anchor, NULL);
            objart_loadsize += bitmaps_2d[i]->w * bitmaps_2d[i]->h;
			   ObjProps[i].bitmap_3d = ObjProps[i].bitmap_3d | count_3d;
            for (f=0; f < (FRAME_NUM_3D(ObjProps[i].bitmap_3d) + 1); f++)
            {
               bool  not_using_2d = FALSE;

               if ((f == 0) && (empty_bitmap(bitmaps_2d[i])))
               {
                  objart_loadsize -= bitmaps_2d[i]->w * bitmaps_2d[i]->h;
                  DisposePtr((Ptr)bitmaps_2d[i]->bits);
                  not_using_2d = TRUE;
               }

               bitmaps_3d[count_3d] = get_objbitmap_from_pool(count_3d,1);
               load_bitmap_from_res(bitmaps_3d[count_3d], OBJECT_ART_BASE, objart_count++, prt, TRUE, &dummy_anchor, NULL);
               objart_loadsize += bitmaps_3d[count_3d]->w * bitmaps_3d[count_3d]->h;
               anchors_3d[count_3d] = dummy_anchor.ul;
               if (not_using_2d)
                  bitmaps_2d[i] = bitmaps_3d[count_3d];

               if ((f > 0) && (ObjProps[i].render_type == FAUBJ_VOX))
                  voxel_convert(bitmaps_3d[count_3d]);
               count_3d++;
            }
            objart_count++;   
         }
         else // skip over us since we are already loaded
         {
            objart_count += 3 + FRAME_NUM_3D(ObjProps[i].bitmap_3d); // 2d + 3d + editor icon + frames
            count_3d += FRAME_NUM_3D(ObjProps[i].bitmap_3d) + 1;
         }
      }
      else
      {
         // We are not wanted, flush us if loaded
         if (bitmaps_2d[i] != NULL)
         {
            if (bitmaps_2d[i] != bitmaps_3d[count_3d])
            {
               objart_loadsize -= bitmaps_2d[i]->w * bitmaps_2d[i]->h;
               DisposePtr((Ptr)bitmaps_2d[i]->bits);
            }
            bitmaps_2d[i] = NULL;
            objart_count++;
            for (f=0; f < (FRAME_NUM_3D(ObjProps[i].bitmap_3d) + 1); f++)
            {
               objart_loadsize -= bitmaps_3d[count_3d]->w * bitmaps_3d[count_3d]->h;
               DisposePtr((Ptr)bitmaps_3d[count_3d]->bits);
               count_3d++;
               objart_count++;
            }
            objart_count++;
         }
         else
         {
            objart_count += FRAME_NUM_3D(ObjProps[i].bitmap_3d) + 3; // 2d + editicon + 3d + number of frames
            count_3d += FRAME_NUM_3D(ObjProps[i].bitmap_3d) + 1;
         }
      }
   }

   // Closedown
   if (!flush_all)
   {
      if (!ref_buffer_used)
         ResFreeRefTable(prt);
      ResCloseFile(objfnum);
   }

//   mprintf("count_3d = %x(%d) NUM_OBJECT=%x(%d), EXTRA_FRAMES=%x(%d)\n",count_3d,count_3d,
//      NUM_OBJECT,NUM_OBJECT,EXTRA_FRAMES,EXTRA_FRAMES);
//   mprintf("OBJART_LOADSIZE = %d\n",objart_loadsize);
   return(OK);
}

