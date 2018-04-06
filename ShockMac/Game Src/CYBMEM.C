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
 * $Source: r:/prj/cit/src/RCS/cybmem.c $
 * $Revision: 1.40 $
 * $Author: xemu $
 * $Date: 1994/11/28 06:40:01 $
 *
 */

// Memory management and manipulation functions
// for Cyberia
#define __CYBMEM_SRC
#include "ShockDialogs.h"

#include "cybmem.h"
#include "tools.h"
#include "objsim.h"
#include "objclass.h"
#include "objprop.h"
#include "textmaps.h"
#include "objcrit.h"
#include "dynmem.h"
#include "Shock.h"
#include "sideicon.h"
#include "criterr.h"

/*
#include <memstat.h>
#include <ckpoint.h>
#include <keydefs.h>
#include <musicai.h>       // for stop_digi_fx()
*/
int loadcount = 0;

extern Id critter_id_table[NUM_CRITTER][NUM_CRITTER_POSTURES];
extern Id posture_bases[];

extern void free_textures(void);
int flush_resource_cache(void);

int hand_fnum, digi_fnum, critter_fnum, critter_fnum2,texture_fnum;

int flush_resource_cache(void)
{
   Id curr_id = ID_MIN;
   int count = 0;
   while (curr_id < resDescMax)
   {
      if (ResInUse(curr_id) && ResPtr(curr_id) && !ResLocked(curr_id))
      {
         ResDrop(curr_id);
         count++;
      }
      curr_id++;
   }
   return(count);
}

errtype free_dynamic_memory(int mask)
{
   // Release textures   
   if (loadcount & DYNMEM_TEXTURES & mask)
   {
      free_textures();
      ResCloseFile(texture_fnum);
   }
   
   if (loadcount & mask & DYNMEM_SIDEICONS)
   {
      side_icon_free_bitmaps();
   }

   if (loadcount & mask & DYNMEM_FHANDLE_1)
   {
 	 ResCloseFile(hand_fnum);
   }
   
   // digifx used to be fhandle 2
   if (loadcount & mask & DYNMEM_FHANDLE_3)
   {
      ResCloseFile(critter_fnum);
   }
   
   if (loadcount & mask & DYNMEM_FHANDLE_4)
   {
      ResCloseFile(critter_fnum2);
   }

   loadcount &= ~mask;
   return(OK);
}

errtype load_dynamic_memory(int mask)
{
   FSSpec	fSpec;
   extern short _new_mode;
	
  FSMakeFSSpec(gDataVref, gDataDirID, "\ptexture.rsrc", &fSpec);
   if (_new_mode != -1)
   {
      if ((~loadcount) & mask & DYNMEM_TEXTURES)
      {
         texture_fnum = ResOpenFile(&fSpec);
         load_textures();

         if (texture_fnum < 0)
            critical_error(CRITERR_RES|7);
      }
      if ((~loadcount) & mask & DYNMEM_SIDEICONS)
      {
         side_icon_load_bitmaps();
      }
	 AdvanceProgress();
      
      if ((~loadcount) & mask & DYNMEM_FHANDLE_1)
      {
  		FSMakeFSSpec(gDataVref, gDataDirID, "\phandart.rsrc", &fSpec);
		hand_fnum = ResOpenFile(&fSpec);
		if (hand_fnum < 0)
			critical_error(CRITERR_RES|3);
      }
	 AdvanceProgress();
      
      // digifx used to be FHANDLE_2
      if ((~loadcount) & mask & DYNMEM_FHANDLE_3)
      {
  		FSMakeFSSpec(gDataVref, gDataDirID, "\pobjart2.rsrc", &fSpec);
		critter_fnum = ResOpenFile(&fSpec);
		if (critter_fnum < 0)
			critical_error(CRITERR_RES|8);
      }
	 AdvanceProgress();
      
      if ((~loadcount) & mask & DYNMEM_FHANDLE_4)
      {
  		FSMakeFSSpec(gDataVref, gDataDirID, "\pobjart3.rsrc", &fSpec);
		critter_fnum2 = ResOpenFile(&fSpec);
		if (critter_fnum2 < 0)
			critical_error(CRITERR_RES|8);
      }
	 AdvanceProgress();

      loadcount |= mask;
   }
   return(OK);
}

#define LARGEST_GUESS 				8000000
#define DECREMENT_INTERVAL 	10000
#define MAX_PTRS           			25
#define MINIMUM_SLORK_SIZE 	100000

int slorkatron_memory_check()
{
   int retval,size;
   int ptr_count,i;
   uchar *mem_ptrs[MAX_PTRS];

   for (ptr_count = 0; ptr_count < MAX_PTRS; ptr_count++)
      mem_ptrs[ptr_count] = NULL;

   ptr_count = 0;

   size = LARGEST_GUESS + DECREMENT_INTERVAL;
   retval = 0;

   while ((size > MINIMUM_SLORK_SIZE) && (ptr_count < MAX_PTRS))
   {
      mem_ptrs[ptr_count] = (uchar *)NewPtr(size);		//  mem_ptrs[ptr_count] = Malloc(size);
      if (mem_ptrs[ptr_count] == NULL)
         size -= DECREMENT_INTERVAL;
      else
      {
         retval += size;
         ptr_count++;
      }
   }
   for (i=ptr_count - 1; i >= 0; i--)
      if (mem_ptrs[i] != NULL)
         DisposePtr((Ptr)mem_ptrs[i]);		//  Free(mem_ptrs[i]);

   return(retval);
}
