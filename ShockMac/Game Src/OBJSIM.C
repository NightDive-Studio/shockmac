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
 * $Source: r:/prj/cit/src/RCS/objsim.c $
 * $Revision: 1.292 $
 * $Author: xemu $
 * $Date: 1994/11/21 21:06:58 $
 */

// Object simulator code for Citadel
#define __OBJSIM_SRC

#include <string.h>
#include <stdlib.h>

#include "Shock.h"
#include "ShockDialogs.h"

#include "citmat.h"
#include "citres.h"
#include "colors.h"
#include "criterr.h"
#include "cybstrng.h"		// for word bitmaps
#include "froslew.h"		// objslew and camera stuff..
#include "gameobj.h"
#include "gamescr.h"		// for citadel font
#include "gettmaps.h"
#include "lvldata.h"
#include "mapflags.h"
#include "objart.h"
#include "objart2.h"  
#include "objart3.h"  
#include "objects.h"
#include "objapp.h"
#include "objbit.h"
#include "objload.h"
#include "objsim.h"
#include "objprop.h"
#include "objwpn.h"
#include "objwarez.h"
#include "objstuff.h"
#include "objver.h"
#include "otrip.h"
#include "objgame.h"
#include "objcrit.h"
#include "physics.h"
#include "physunit.h"
#include "refstuf.h"
#include "textmaps.h"
#include "tools.h"
#include "weapons.h"
#include "gamestrn.h"
#include "mainloop.h"
#include "shodan.h"
#include "grenades.h"
#include "player.h"
#include "tpolys.h"
#include "render.h"
#include "faketime.h"
#include "cybmem.h"
#include "damage.h"
#include "pathfind.h"
#include "cyber.h"
#include "sfxlist.h"
#include "ai.h"
#include "fauxrint.h"
#include "cybrnd.h"
#include "trigger.h"
#include "effect.h"  // for anim_list adding and removing
#include "mfdext.h"
#include "musicai.h"

// Some useful constants

#define OBSELETE_WARES_USELESS
#define NUM_CONTENTS 4
#define MAX_CONTAINER_OBJS    NUM_CONTENTS

#define CUSTOM_MATERIAL_BASE     RES_customTextureMaps

#define PLAYER_TRIP MAKETRIP(CLASS_CRITTER,0,6)

#define inv_coor(fixval) ((fix_int((fixval)) << MAP_SH) + (fix_frac((fixval)) >> MAP_MS))
#define obj_inv_coor_x(oloc) ((OBJ_LOC_BIN_X(oloc) << MAP_SH) + (OBJ_LOC_FINE_X(oloc) >> MAP_MS))
#define obj_inv_coor_y(oloc) ((OBJ_LOC_BIN_Y(oloc) << MAP_SH) + (OBJ_LOC_FINE_Y(oloc) >> MAP_MS))
#define obj_inv_coor_z(oid) (inv_coor(fix_from_obj_height(oid)))

extern void instantiate_robot(int triple, Robot* new_robot);

extern char container_extract(ObjID *pidlist, int d1, int d2);
extern void container_stuff(ObjID *pidlist, int numobjs, int* d1, int* d2);
extern bool is_container(ObjID id, int** d1, int** d2);

char clearwithFF = false;

// global symbol for the player camera...
cams player_cam;

uchar cam_mode = OBJ_PLAYER_CAMERA;
cams objmode_cam;
bool  new_cyber_orient=TRUE;
bool  ocp_settle_the_player=TRUE;

bool properties_changed = FALSE;
bool trigger_check = TRUE;
ObjID physics_handle_id[MAX_OBJ];
int physics_handle_max=-1;

// Internal Prototypes
errtype ObjClassInit(ObjID id, ObjSpecID specid, int subclass);
errtype obj_set_secondary_properties();
errtype do_ecology_triggers();
errtype obj_physics_refresh(short x, short y, bool use_floor);
grs_bitmap *get_text_bitmap_from_string(int d1, char dest_type, char *s, bool scroll, int scroll_index);
grs_bitmap *get_text_bitmap_obj(ObjID cobjid, char dest_type, char *pscale);
grs_bitmap *obj_get_model_data(ObjID id, fix *x, fix *y, fix *z, grs_bitmap *bm2, Ref *ref1, Ref *ref2);
void place_obj_at_objloc(ObjID id, ObjLoc *newloc, ushort xsize, ushort ysize);
char extract_object_special_color(ObjID id);
Ref ref_from_critter_data(ObjID oid, int triple, ubyte posture, ubyte frame, ubyte view);
void spew_contents(ObjID id, int d1, int d2);
bool obj_is_useless(ObjID oid);
bool obj_is_display(int triple);
errtype obj_settle_func(ObjID id);
bool death_check(ObjID id, bool* destr);


errtype set_door_data(ObjID id); // ��� here for now

errtype set_door_data(ObjID id)
{
   // Do we block the renderer?
   if (objs[id].info.current_frame == 0)
   {
      if (ObjProps[OPNUM(id)].flags & RENDER_BLOCK)
         objs[id].info.inst_flags |= RENDER_BLOCK_FLAG;
   }
   else
   {
      objs[id].info.inst_flags &= ~RENDER_BLOCK_FLAG;
   }

   // Are we vertical?
   if ((objs[id].loc.p == 0) && (objs[id].loc.b == 0))
      objs[id].info.inst_flags |= CLASS_INST_FLAG;
   else
      objs[id].info.inst_flags &= ~CLASS_INST_FLAG;

   return(OK);
}

#ifdef PLAYTEST
int extra_object_frames(int triple)
{
#ifdef SPEW_ON
   char temp[100];
#endif
   int retval;
   retval = FRAME_NUM_3D(ObjProps[OPTRIP(triple)].bitmap_3d);
//   Spew(DSRC_OBJSIM_Editor, ("extra_frames for %x %s",triple, get_object_long_name(triple,temp,100)));
//   Spew(DSRC_OBJSIM_Editor, ("= %hd (from %hx) OTRIP=%d\n", retval,ObjProps[OPTRIP(triple)].bitmap_3d,OPTRIP(triple)));
   return(retval);
}
#endif

Id critter_id_table[NUM_CRITTER];

grs_bitmap *get_text_bitmap(int d1, int d2, char dest_type, bool scroll);

#define NUM_TEXT_BITMAPS   2
short text_bitmaps_x[NUM_TEXT_BITMAPS] = {128, 64};
short text_bitmaps_y[NUM_TEXT_BITMAPS] = {32, 64};
ushort text_bitmap_flags[NUM_TEXT_BITMAPS] = {BMF_TRANS, 0};
Ref text_bitmap_refs[NUM_TEXT_BITMAPS] = { REF_STR_WordZero, REF_STR_ScreenZero };

grs_bitmap *text_bitmap_ptrs[NUM_TEXT_BITMAPS];
grs_canvas text_canvases[NUM_TEXT_BITMAPS];

int memcount = 0;

errtype obj_init()
{
   uchar c_class, c_subclass;
   int i,j,count,class_count=0;
   Id ids;
   
   // Create the word-buffer bitmap
   for (i=0; i < NUM_TEXT_BITMAPS; i++)
   {
      text_bitmap_ptrs[i] = gr_alloc_bitmap(BMT_FLAT8,text_bitmap_flags[i],text_bitmaps_x[i], text_bitmaps_y[i]);
      gr_make_canvas(text_bitmap_ptrs[i], &text_canvases[i]);
   }
   
   // initialize physics handle mapping to objID
   for (i=0;i<MAX_OBJ;i++)
      physics_handle_id[i]=OBJ_NULL;
   physics_handle_max=-1;

   // Load object properties from disk
   obj_load_properties();
   AdvanceProgress();

   // Create base array
   count = 0;
//   Spew(DSRC_OBJSIM_Editor, ("num_classes = %d num_objects = %d\n",NUM_CLASSES,NUM_OBJECT));
   for (c_class = 0; c_class < NUM_CLASSES; c_class++)
   {
      class_count = 0;
      for (c_subclass = 0; c_subclass < num_subclasses[c_class]; c_subclass++)
      {
          // Set Base pointer for offset of class
          ObjBaseArray[OBJBASE(MAKETRIP(c_class, c_subclass, 0))] = count;
          ClassBaseArray[c_class][c_subclass] = class_count;
          for (i=0; i < num_types(c_class, c_subclass); i++)
            bitmaps_2d[count + i] = NULL;
          class_count += i;
          count += i;
      }
   }

   // Initialize critter cache stuff
   count = 0;
   ids = 0;
   for (i=0; i < num_subclasses[CLASS_CRITTER]; i++)
   {
      for (j=0; j < num_types(CLASS_CRITTER, i); j++)
      {
          critter_id_table[count] = ids;
          ids += CritterProps[CPTRIP(MAKETRIP(CLASS_CRITTER,i,j))].views;
//         Spew(DSRC_GFX_Anim, ("views for %d,%d = %d, ids = %d corpse = %x\n", i,j,
//            CritterProps[CPTRIP(MAKETRIP(CLASS_CRITTER,i,j))].views, ids,
//            CritterProps[CPTRIP(MAKETRIP(CLASS_CRITTER,i,j))].corpse));
          count++;
      }
   }
   obj_set_secondary_properties();
   
   return(OK);
}

Id posture_bases[] = { 
   // Full direction postures
   CRITTER_STAND_BASE, CRITTER_MOVE_BASE,
   // Front-only postures
   CRITTER_ATTACK_BASE, CRITTER_ATTACK_REST_BASE, CRITTER_KNOCKBACK_BASE, CRITTER_DEATH_BASE,  CRITTER_DISRUPT_BASE,
   CRITTER_ATTACK2_BASE };

#define CRITTER_LOADING_PAGE_LIMIT 45000

Ref ref_from_critter_data(ObjID, int triple, ubyte posture, ubyte frame, ubyte view) //, bool *pmirror)
{
   Ref retval;
   ubyte v,p;
   ulong old_ticks;
   extern ulong last_real_time;
   Id our_id;
   RefTable *prt;
   char curr_frames;
   bool load_all_views = TRUE;
//���   extern ulong page_amount;

   // Set mirror pointer
//   if (pmirror != NULL)
//      *pmirror = CritterProps[CPTRIP(triple)].mirror;
   switch (triple)
   {
      case ROBOBABE_TRIPLE:
         posture = STANDING_CRITTER_POSTURE;
         view = 0;
         break;
   }

//���   if (page_amount > CRITTER_LOADING_PAGE_LIMIT)
//���      load_all_views = FALSE;
//���   else
   {
      for (p=STANDING_CRITTER_POSTURE; p <= MOVING_CRITTER_POSTURE; p++)
      {
         for (v=0; v < 8; v++)
         {
            Id id;
            id = critter_id_table[CPTRIP(triple)] + v + posture_bases[p];
            if (ResPtr(id))
               load_all_views = FALSE;
         }
      }
   }
//   mprintf("lav %d     page_amt = %d\n",load_all_views,page_amount);

   // Maybe this should be default_posture if view != FRONT_VIEW?
   if (posture >= FIRST_FRONT_POSTURE)
      retval = MKREF(posture_bases[posture] + get_nth_from_triple(triple), frame);
   else
   {
      our_id = critter_id_table[get_nth_from_triple(triple)] + view + posture_bases[posture];
      if (CritterProps[CPTRIP(triple)].frames[posture] == 0)
          posture = DEFAULT_CRITTER_POSTURE;
      if (view == FRONT_VIEW)
         curr_frames = CritterProps[CPTRIP(triple)].frames[posture];
      else
      {
//         prt = ResReadRefTable(our_id);		
         prt = (RefTable *)ResLock(our_id); 
         curr_frames = prt->numRefs;
//         ResFreeRefTable(prt);					
         ResUnlock(our_id);
      }
      if (frame >= curr_frames)
         frame = frame % curr_frames;
      retval = MKREF(our_id, frame);
   }

   // See if we're loaded from all views, if not load us!
   // This may slow down the universe a bit too much...if so then 
   // we'll need to streamline & optimize it some.
   old_ticks = *tmd_ticks;

   if (load_all_views)
   {
      for (p=STANDING_CRITTER_POSTURE; p <= MOVING_CRITTER_POSTURE; p++)
      {
         for (v=0; v < 8; v++)
         {
            Id id;
            id = critter_id_table[CPTRIP(triple)] + v + posture_bases[p];
            if ((id != our_id) && (ResPtr(id) == NULL))
            {
               ResLock(id);
               ResUnlock(id);
            }
         }
      }
   }

   // suspend game time for the duration of the loading
   last_real_time += *tmd_ticks - old_ticks;

   return(retval);
}

#include "MacTune.h"

// deparses the bitmap_3d data field for a tpoly object into the bitmap to be textured onto it
grs_bitmap *bitmap_from_tpoly_data(int tpdata, ubyte *scale, int *index, uchar *type, Ref *use_ref)
{
   extern grs_bitmap *static_bitmap;
   extern char camera_map[NUM_HACK_CAMERAS];
   extern char num_customs;
   short style;
   int use_index;
   Id useme;
   grs_bitmap	*result;

   if (*use_ref != NULL)
      *use_ref = 0;

   tpdata = tpdata & 0xFFF;
   style = (tpdata & TPOLY_STYLE_MASK) >> (TPOLY_INDEX_BITS + TPOLY_TYPE_BITS + TPOLY_SCALE_BITS);
   *scale = ((tpdata & TPOLY_SCALE_MASK) >> (TPOLY_INDEX_BITS + TPOLY_TYPE_BITS)) << TPOLY_SCALE_SHIFT;
   *index = tpdata & TPOLY_INDEX_MASK;
   *type  = (tpdata & TPOLY_TYPE_MASK) >> TPOLY_INDEX_BITS;
   switch(*type)
   {
      case TPOLY_TYPE_ALT_TMAP:
         if (!ResInUse(TEXTURE_SMALL_ID + *index))
         {
            Warning(("Invalid tpoly alt texture type request!"));
            useme = 0;
         }
         else
            useme = TEXTURE_SMALL_ID + *index;
         if (use_ref != NULL)
            *use_ref = MKREF(useme,0);
         return(lock_bitmap_from_ref_anchor(MKREF(useme,0), NULL));
         break;
     case TPOLY_TYPE_CUSTOM_MAT:
        if ((*index >= FIRST_CAMERA_TMAP) && (*index <= FIRST_CAMERA_TMAP + NUM_HACK_CAMERAS))
         {
            extern uchar hack_cameras_needed;
            extern grs_bitmap *hack_cam_bitmaps[NUM_HACK_CAMERAS];
            short temp_val = (*index) - FIRST_CAMERA_TMAP;
            hack_cameras_needed |= (1 << temp_val);
            do_screen_static();
            if (camera_map[temp_val])
               return(hack_cam_bitmaps[camera_map[temp_val] - 1]);
            else
               return(static_bitmap);
         }
         else if ((*index == REGULAR_STATIC_MAGIC_COOKIE) || (*index == SHODAN_STATIC_MAGIC_COOKIE))
         {
            do_screen_static();
            return(static_bitmap);
         }
         else if ((*index >= FIRST_AUTOMAP_MAGIC_COOKIE) && (*index <= FIRST_AUTOMAP_MAGIC_COOKIE + NUM_AUTOMAP_MAGIC_COOKIES))
         {
            extern grs_bitmap *screen_automap_bitmap(char which_amap);
            return(screen_automap_bitmap(*index - FIRST_AUTOMAP_MAGIC_COOKIE));
         }

         if (!ResInUse(CUSTOM_MATERIAL_BASE + *index))
         {
//            Warning(("Invalid custom material request (%d)!\n",*index));
            do_screen_static();
            return(static_bitmap);
         }
         else
         {
            *use_ref = MKREF(CUSTOM_MATERIAL_BASE + *index, 0);
            return(lock_bitmap_from_ref_anchor(*use_ref, NULL));
         }   
         break;
      case TPOLY_TYPE_TEXT_BITMAP:
         // style=style?2:3;
         style=3-style;
         if (*index == RANDOM_TEXT_MAGIC_COOKIE)
         {
            char use_buf[10];
            int seed;

            seed=*tmd_ticks>>7;
            use_index=((seed*9277+7)%14983)%10;
		  numtostring((long) use_index, use_buf);
		  clearwithFF = true;
            result = get_text_bitmap_from_string(style, 1, use_buf, FALSE, 0);
		  clearwithFF = false;
		  return(result);
//            return(get_text_bitmap_from_string(style, 1, itoa(use_index, use_buf, 10), FALSE, 0));
         }
         else
         {
		  clearwithFF = true;
            result = get_text_bitmap(style, *index, 1,FALSE);
		  clearwithFF = false;
		  return(result);
//            return(get_text_bitmap(style, *index, 1,FALSE));
         }
         break;
      case TPOLY_TYPE_SCROLL_TEXT:
         // style=style?2:3;
		style=3-style;
		clearwithFF = true;
		result = get_text_bitmap(style, *index, 1,TRUE);
		clearwithFF = false;
		return (result);
//         return(get_text_bitmap(style, *index, 1,TRUE));
         break;
   }

   return(NULL);
}


#define BARRICADE_DEF_X 0x4
#define BARRICADE_DEF_Y 0x1
#define BARRICADE_DEF_Z 0x10

#define BRIDGE_DEF_X 0x4
#define BRIDGE_DEF_Y 0x4
#define BRIDGE_DEF_Z 0x01
#define BRIDGE_DEF_TEXTURE  0x80
#define BRIDGE_DEF_TEXTURE2 0x80

#define CATWALK_DEF_X 0x2
#define CATWALK_DEF_Y 0x4
#define CATWALK_DEF_Z 0x01
#define CATWALK_DEF_TEXTURE  0x80
#define CATWALK_DEF_TEXTURE2 0x80

#define PILLAR_DEF_X 0x2
#define PILLAR_DEF_Y 0x2
#define PILLAR_DEF_Z 0xB0
#define PILLAR_DEF_TEXTURE  0x80
#define PILLAR_DEF_TEXTURE2 0x81

#define SMALL_CRT_DEF_X 0x08
#define SMALL_CRT_DEF_Y 0x08
#define SMALL_CRT_DEF_Z 0x08
#define SMALL_CRT_DEF_TEXTURE  0x0C
#define SMALL_CRT_DEF_TEXTURE2 0x0B

#define LG_CRT_DEF_X 0x10
#define LG_CRT_DEF_Y 0x10
#define LG_CRT_DEF_Z 0x10
#define LG_CRT_DEF_TEXTURE  0x0C
#define LG_CRT_DEF_TEXTURE2 0x0B

#define SECURE_CONTR_DEF_X 0x20
#define SECURE_CONTR_DEF_Y 0x20
#define SECURE_CONTR_DEF_Z 0x20
#define SECURE_CONTR_DEF_TEXTURE  0x0A
#define SECURE_CONTR_DEF_TEXTURE2 0x0A

#define BIGSTUFF_MODEL_XY_SHF    13
#define BIGSTUFF_MODEL_Z_SHF     10
#define CONTAINER_MODEL_SHF      10

#define FULL_EXTENSION  255
// This is a TOTAL guess
#define FULL_EXTENSION_Z   8
#define EXTENDED_FRAMES 32


// hack_foo is the amount of the full object to show.  
// hack_type is the kind of hack == 
// 0 is extending out from one side, 
// 1 is expanding out from center
// 2 is rising from the floor/descending from the ceiling
errtype obj_model_hack(ObjID id, uchar *hack_x, uchar *hack_y, uchar *hack_z, uchar *hack_type)
{  
   switch (ID2TRIP(id))
   {
      case FORCE_BRIJ2_TRIPLE:
         *hack_type = 1;
         *hack_x = FULL_EXTENSION * (EXTENDED_FRAMES - objs[id].info.current_frame) / EXTENDED_FRAMES;
         *hack_y = FULL_EXTENSION * (EXTENDED_FRAMES - objs[id].info.current_frame) / EXTENDED_FRAMES;
         break;
      case FORCE_BRIJ_TRIPLE:
         *hack_type = 0;
         *hack_y = FULL_EXTENSION * (EXTENDED_FRAMES - objs[id].info.current_frame) / EXTENDED_FRAMES;
         break;
      case LABFORCE_TRIPLE:
      case RESFORCE_TRIPLE:
         *hack_type = 2;
         *hack_z = FULL_EXTENSION_Z * (EXTENDED_FRAMES - objs[id].info.current_frame) / EXTENDED_FRAMES;
         break;
     default:
         *hack_type = 0xFF;
         break;
   }
   return(OK);
}

grs_bitmap *obj_get_model_data(ObjID id, fix *x, fix *y, fix *z, grs_bitmap *bm2, Ref *ref1, Ref *ref2)
{
   int pval;
   uchar p1,p2,p3,p4,p5;
   grs_bitmap *retval = NULL;
   grs_bitmap *temp_bm = NULL;

   if (!objs[id].active)
   {
      Warning(("Attempted to get model params from invalid object!\n"));
      return(NULL);
   }
   switch (objs[id].obclass)
   {
      case CLASS_BIGSTUFF: 
         pval = objBigstuffs[objs[id].specID].data1;  
         p1 = pval & 0xF;
         p2 = (pval & 0xF0) >> 4;
         p3 = (pval & 0xFF00) >> 8;
         p4 = (pval & 0xFF0000) >> 16;
         p5 = (pval & 0xFF000000) >> 24;
         break;
      case CLASS_SMALLSTUFF:
         pval = objSmallstuffs[objs[id].specID].data1;  
         p1 = pval & 0xF;
         p2 = (pval & 0xF0) >> 4;
         p3 = (pval & 0xFF00) >> 8;
         p4 = (pval & 0xFF0000) >> 16;
         p5 = (pval & 0xFF000000) >> 24;
         break;
      case CLASS_CONTAINER:
         p1 = objContainers[objs[id].specID].dim_x;
         p2 = objContainers[objs[id].specID].dim_y;
         p3 = objContainers[objs[id].specID].dim_z;
         p4 = objContainers[objs[id].specID].data1 & 0xFF;
         p5 = (objContainers[objs[id].specID].data1 & 0xFF00) >> 8;
         break;
      default:
//         Warning(("Object %d not of correct class (class = %d) to extract model data!\n",id, objs[id].obclass));
         return(NULL);
   }

   // Now convert into appropriate units!
   // and do defaults if zero.

   // Okay, this could be done more elegantly with arrays, but hey, it's already
   // done this way.
   switch (objs[id].obclass)
   {
      case CLASS_BIGSTUFF:    // bridge, catwalk, pillar
         switch (ID2TRIP(id))
         {
            case BRIDGE_TRIPLE:
               if (p1 == 0)                  p1 = BRIDGE_DEF_X;
               if (p2 == 0)                  p2 = BRIDGE_DEF_Y;
               if (p3 == 0)                  p3 = BRIDGE_DEF_Z;
               if (p4 == 0)                  p4 = BRIDGE_DEF_TEXTURE;
               if (p5 == 0)                  p5 = BRIDGE_DEF_TEXTURE2;
               break;
            case FORCE_BRIJ_TRIPLE:
            case CATWALK_TRIPLE:
               if (p1 == 0)                  p1 = CATWALK_DEF_X;
               if (p2 == 0)                  p2 = CATWALK_DEF_Y;
               if (p3 == 0)                  p3 = CATWALK_DEF_Z;
               if (p4 == 0)                  p4 = CATWALK_DEF_TEXTURE;
               if (p5 == 0)                  p5 = CATWALK_DEF_TEXTURE2;
               break;
            case PILLAR_TRIPLE:
               if (p1 == 0)                  p1 = PILLAR_DEF_X;
               if (p2 == 0)                  p2 = PILLAR_DEF_Y;
               if (p3 == 0)                  p3 = PILLAR_DEF_Z;
               if (p4 == 0)                  p4 = PILLAR_DEF_TEXTURE;
               if (p5 == 0)                  p5 = PILLAR_DEF_TEXTURE2;
               break;
         }

         // Transform into fix
         *x = p1 << BIGSTUFF_MODEL_XY_SHF;
         *y = p2 << BIGSTUFF_MODEL_XY_SHF;
         *z = p3 << BIGSTUFF_MODEL_Z_SHF;
         break;
      case CLASS_SMALLSTUFF:    // bridge, catwalk, pillar
         switch (ID2TRIP(id))
         {
            case BARRICADE_TRIPLE:
               if (p1 == 0)                  p1 = BARRICADE_DEF_X;
               if (p2 == 0)                  p2 = BARRICADE_DEF_Y;
               if (p3 == 0)                  p3 = BARRICADE_DEF_Z;
               break;
         }

         // Transform into fix
         // okay, this is using BIGSTUFF defines but really, who cares?
         *x = p1 << BIGSTUFF_MODEL_XY_SHF;
         *y = p2 << BIGSTUFF_MODEL_XY_SHF;
         *z = p3 << BIGSTUFF_MODEL_Z_SHF;
         break;
      case CLASS_CONTAINER:
         switch (ID2TRIP(id))
         {
            case SML_CRT_TRIPLE:
               if (p1 == 0)                  p1 = SMALL_CRT_DEF_X;
               if (p2 == 0)                  p2 = SMALL_CRT_DEF_Y;
               if (p3 == 0)                  p3 = SMALL_CRT_DEF_Z;
               if (p4 == 0)                  p4 = SMALL_CRT_DEF_TEXTURE;
               if (p5 == 0)                  p5 = SMALL_CRT_DEF_TEXTURE2;
               break;
            case LG_CRT_TRIPLE:
               if (p1 == 0)                  p1 = LG_CRT_DEF_X;
               if (p2 == 0)                  p2 = LG_CRT_DEF_Y;
               if (p3 == 0)                  p3 = LG_CRT_DEF_Z;
               if (p4 == 0)                  p4 = LG_CRT_DEF_TEXTURE;
               if (p5 == 0)                  p5 = LG_CRT_DEF_TEXTURE2;
               break;
            case SECURE_CONTR_TRIPLE:
               if (p1 == 0)                  p1 = SECURE_CONTR_DEF_X;
               if (p2 == 0)                  p2 = SECURE_CONTR_DEF_Y;
               if (p3 == 0)                  p3 = SECURE_CONTR_DEF_Z;
               if (p4 == 0)                  p4 = SECURE_CONTR_DEF_TEXTURE;
               if (p5 == 0)                  p5 = SECURE_CONTR_DEF_TEXTURE2;
               break;
         }
         // Transform into fix
         *x = p1 << CONTAINER_MODEL_SHF;
         *y = p2 << CONTAINER_MODEL_SHF;
         *z = p3 << CONTAINER_MODEL_SHF;
         break;   
   }

   // if bm2 is NULL then we basically don't need to get the texture map
   if (bm2==NULL)
   {
      // a NON-NULL pointer 
      return((grs_bitmap *) 1);
   }

   // Now get the texture map

   // Okay, this is wacky that we process p5 before p4, but it is necessary that the bitmap that
   // we actually copy out (as opposed to returning) be done first -- specifically that the thing
   // we return has to have been done the most recently.
   if (p5 & 0x80)
   {
      if ((p5 & 0x7F) >= NUM_LOADED_TEXTURES)  
         { temp_bm=SAFE_TEXTURE; *bm2 = *temp_bm; Warning(("Bogus Tmap")); } // doofy you screwed up thing
      else 
      {
         temp_bm = get_texture_map(p5 & 0x7F,TEXTURE_64_INDEX);
         *bm2 = *temp_bm;
      }
   }
   else
   {
      if (ref2 != NULL)
      {
         *ref2 = MKREF(CUSTOM_MATERIAL_BASE + (p5 & 0x7F), 0);
         temp_bm = lock_bitmap_from_ref_anchor(*ref2, NULL);
         *bm2 = *temp_bm;
      }
   }

   if (p4 & 0x80)
   {
   if ((p4 & 0x7F) >= NUM_LOADED_TEXTURES)  { retval=SAFE_TEXTURE; Warning(("Bogus Tmap")); } else // doofy you screwed up thing
      retval = get_texture_map(p4 & 0x7F, TEXTURE_64_INDEX);
   }
   else
   {
      if (ref1 != NULL)
      {
         *ref1 = MKREF(CUSTOM_MATERIAL_BASE + (p4 & 0x7F), 0);
         retval = lock_bitmap_from_ref_anchor(*ref1, NULL);
      }
   }

   return(retval);
}

char extract_object_special_color(ObjID id)
{
   switch(objs[id].obclass)
   {
      case CLASS_BIGSTUFF:
         return(objBigstuffs[objs[id].specID].data2);
         break;
      case CLASS_SMALLSTUFF:
         return(objSmallstuffs[objs[id].specID].data2);
         break;
      case CLASS_DOOR:
         return(objDoors[objs[id].specID].cosmetic_value);
         break;
      default:
         return(0);
         break;
   }
}


// Shutdown the object system and free up memory as appropriate
errtype obj_shutdown()
{
	int	i;
	extern errtype obj_load_art(bool flush_all);
	
	// Free the word-buffer bitmap
	for (i=0; i < NUM_TEXT_BITMAPS; i++)
	{
		if (text_bitmap_ptrs[i] != NULL)
			DisposePtr((Ptr)text_bitmap_ptrs[i]);
	}
	
	obj_load_art(TRUE);
	
	return(OK);
}

void spew_contents(ObjID id, int d1, int d2)
{
	char i, num_objs;
	ObjLoc newloc;
	ObjID id_list[MAX_CONTAINER_OBJS];
	num_objs = container_extract(id_list, d1, d2);
	for (i=0; i < num_objs; i++)
	{
		if (id_list[i] != OBJ_NULL)
		{
			newloc = objs[id].loc;
			newloc.x += rand() & 0x6F;
			newloc.y += rand() & 0x6F;
			obj_move_to(id_list[i], &newloc, TRUE);
		}
	}
}

bool obj_is_useless(ObjID oid)
{
	bool useless;
	
	useless = (ObjProps[OPNUM(oid)].flags & USELESS_FLAG)!=0;
#ifdef OBSELETE_WARES_USELESS
	if(!useless && objs[oid].obclass==CLASS_HARDWARE)
		useless=player_struct.hardwarez[CPTRIP(ID2TRIP(oid))]>=objHardwares[objs[oid].specID].version;
#endif
	useless = useless && !(objs[oid].info.inst_flags & USEFUL_FLAG);
	return useless;
}


#define OBJ_DESTROY_TRIES  5
#define MIN_OBJKILL_DIST 8

// Creates the basic object, but does not place it into the world
bool obj_autodelete = TRUE;
ObjID obj_create_base(int triple)
{
   ObjID new_id;
   ObjSpecID new_specid;
   extern errtype obj_screen_animate(ObjID id);

   if (obj_autodelete)
   {
      while(!ObjAndSpecGrab(TRIP2CL(triple), &new_id, &new_specid))
      {
         ObjID kill_obj = OBJ_NULL,oid;
         ObjID kill_container = OBJ_NULL;
         int kill_index, num_objs;
         ObjID idlist[NUM_CONTENTS];
         ObjLoc ploc = objs[PLAYER_OBJ].loc;
         ObjLoc killobjloc;
         int *d1, *d2, content, dist, obclass, maxdist = 0;
         
//         Warning(("ObjAndSpecGrab could not find ObjSpec for this class: %d.\n", TRIP2CL(triple)));

         FORALLOBJS(oid)
         {
            if(oid==player_struct.panel_ref) continue;
            obclass=objs[oid].obclass;
            if (obclass == TRIP2CL(triple)) {

               if (objs[oid].ref == OBJ_REF_NULL)
                  continue;

               if(obj_is_useless(oid))
               {
                  killobjloc=objs[oid].loc;
                  dist=long_fast_pyth_dist(killobjloc.x - ploc.x, killobjloc.y - ploc.y);
                  if(dist>=maxdist)
                  {
                     kill_obj = oid;
                     kill_container = NULL;
                     maxdist = dist;
                  }
               }
            }
            else if (is_container(oid,&d1,&d2))
            {
               if (!(((ID2TRIP(oid) >= MUT_CORPSE1_TRIPLE) && (ID2TRIP(oid) <= OTH_CORPSE8_TRIPLE)) ||
                     ((ID2TRIP(oid) >= CORPSE1_TRIPLE) && (ID2TRIP(oid) <= CORPSE8_TRIPLE)))        
                  && (objs[oid].info.inst_flags & CLASS_INST_FLAG))
               {
                  num_objs=container_extract(idlist,*d1,(d2 == NULL) ? 0 : *d2);
                  for(content=0;content<num_objs;content++) {
                     if(idlist[content]==OBJ_NULL) continue;
                     if(objs[idlist[content]].obclass != TRIP2CL(triple)) continue;
                  
                     if(obj_is_useless(idlist[content])) {
                        killobjloc=objs[oid].loc;
                        dist=long_fast_pyth_dist(killobjloc.x - ploc.x, killobjloc.y - ploc.y);
                        if(dist>=maxdist)
                        {
                           kill_obj=idlist[content];
                           kill_container=oid;
                           kill_index=content;
                           maxdist=dist;
                        }
                     }
                  }
               }
            }
         }
         if(maxdist<MIN_OBJKILL_DIST) kill_obj=OBJ_NULL;
 //        Warning(("Destroying remote instance to accomodate...obj ID 0x%x\n",kill_obj));
         if (kill_obj != OBJ_NULL) {
            if(kill_container != OBJ_NULL) {
               is_container(kill_container,&d1,&d2);
               num_objs=container_extract(idlist,*d1,(d2 == NULL) ? 0 : *d2);
               idlist[kill_index]=OBJ_NULL;
               container_stuff(idlist,num_objs-1,d1,d2);
            }
            if(is_container(kill_obj,&d1,&d2) && !(objs[kill_obj].info.inst_flags & CLASS_INST_FLAG)) {
               do_random_loot(kill_obj);
               spew_contents(kill_obj,*d1,(d2 == NULL) ? 0 : *d2);
            }
            obj_destroy(kill_obj);
         }
         else
            return(OBJ_NULL);
      }
   }
   else if (!ObjAndSpecGrab(TRIP2CL(triple), &new_id, &new_specid))
   {
 //        Warning(("ObjAndSpecGrab could not find ObjSpec for this obclass: %d.\n", TRIP2CL(triple)));
         return(OBJ_NULL);
   }

   objs[new_id].info.type = TRIP2TY(triple);    // needs type info - so it knows the correct hp
   ObjClassInit(new_id, new_specid, TRIP2SC(triple));
   objs[new_id].info.ph = -1;
   objs[new_id].loc.x = -1;
   objs[new_id].info.current_frame = 0;
   objs[new_id].info.time_remainder = 0;
   switch(objs[new_id].obclass)
   {
      case CLASS_GUN:
         if ((TRIP2SC(triple) == GUN_SUBCLASS_BEAM) ||
             (TRIP2SC(triple) == GUN_SUBCLASS_BEAMPROJ))
         {
            objGuns[new_specid].ammo_type = 30;
            objGuns[new_specid].ammo_count = 0;
         }
         break;
      case CLASS_PHYSICS:
         LG_memset(&objPhysicss[new_specid].p1, 0, (sizeof (ObjLoc)));
         LG_memset(&objPhysicss[new_specid].p2, 0, (sizeof (ObjLoc)));
         LG_memset(&objPhysicss[new_specid].p3, 0, (sizeof (ObjLoc)));
         break;
      case CLASS_DOOR:
         if (ObjProps[OPTRIP(triple)].flags & RENDER_BLOCK)
            objs[new_id].info.inst_flags |= RENDER_BLOCK_FLAG;
         break;
      case CLASS_SMALLSTUFF:
         if ((triple >= CORPSE1_TRIPLE) && (triple <= CORPSE8_TRIPLE))
         {
            objs[new_id].info.inst_flags |= CLASS_INST_FLAG;
         }
         break;
      case CLASS_CRITTER:
         objCritters[new_specid].path_id = -1;
         break;
   }

   if (ANIM_3D(ObjProps[OPTRIP(triple)].bitmap_3d))
   {
      switch(TRIP2CL(triple))
      {
         case CLASS_BIGSTUFF:
         case CLASS_SMALLSTUFF:
            obj_screen_animate(new_id);
            break;
         default:
            add_obj_to_animlist(new_id, REPEAT_3D(ObjProps[OPTRIP(triple)].bitmap_3d),FALSE,FALSE,0,NULL,NULL,0);
            break;
      }
   }
   increment_shodan_value(new_id,TRUE);
   if (trigger_check)
      do_ecology_triggers();

   return(new_id);
}


// Clones an object, including copying all of it's instance data
// WARNING:  Do not try this function with objects of CLASS_DINOSAUR!! -- D. Nedry

ObjID obj_create_clone(ObjID dna)
{
   ObjID new_obj;
   ubyte *pspec, *pdef;
   ObjSpecID specid, osi;
   ObjSpecHeader *spec_hdr = &objSpecHeaders[objs[dna].obclass]; 

   if (dna == OBJ_NULL)
   {
      return(OBJ_NULL);
   }
   new_obj = obj_create_base(MAKETRIP(objs[dna].obclass,objs[dna].subclass, objs[dna].info.type));
   if (new_obj == OBJ_NULL)
   {
      return(OBJ_NULL);
   }
   specid = objs[new_obj].specID;
   osi = objs[dna].specID;

   switch(objs[dna].obclass)
   {
      case CLASS_GUN: pspec = (ubyte *)&objGuns[specid]; pdef = (ubyte *)&objGuns[osi]; break;
      case CLASS_AMMO: pspec = (ubyte *)&objAmmos[specid]; pdef = (ubyte *)&objAmmos[osi]; break;
      case CLASS_PHYSICS: pspec = (ubyte *)&objPhysicss[specid]; pdef = (ubyte *)&objPhysicss[osi]; break;
      case CLASS_GRENADE: pspec = (ubyte *)&objGrenades[specid]; pdef = (ubyte *)&objGrenades[osi]; break;
      case CLASS_DRUG: pspec = (ubyte *)&objDrugs[specid]; pdef = (ubyte *)&objDrugs[osi]; break;
      case CLASS_HARDWARE: pspec = (ubyte *)&objHardwares[specid]; pdef = (ubyte *)&objHardwares[osi]; break;
      case CLASS_SOFTWARE: pspec = (ubyte *)&objSoftwares[specid]; pdef = (ubyte *)&objSoftwares[osi]; break;
      case CLASS_BIGSTUFF: pspec = (ubyte *)&objBigstuffs[specid]; pdef = (ubyte *)&objBigstuffs[osi]; break;
      case CLASS_SMALLSTUFF: pspec = (ubyte *)&objSmallstuffs[specid]; pdef = (ubyte *)&objSmallstuffs[osi]; break;
      case CLASS_FIXTURE: pspec = (ubyte *)&objFixtures[specid]; pdef = (ubyte *)&objFixtures[osi]; break;
      case CLASS_DOOR: pspec = (ubyte *)&objDoors[specid]; pdef = (ubyte *)&objDoors[osi]; break;
      case CLASS_ANIMATING: pspec = (ubyte *)&objAnimatings[specid]; pdef = (ubyte *)&objAnimatings[osi]; break;
      case CLASS_TRAP: pspec = (ubyte *)&objTraps[specid]; pdef = (ubyte *)&objTraps[osi]; break;
      case CLASS_CONTAINER: pspec = (ubyte *)&objContainers[specid]; pdef = (ubyte *)&objContainers[osi]; break;
      case CLASS_CRITTER: pspec = (ubyte *)&objCritters[specid]; pdef = (ubyte *)&objCritters[osi]; break;
  }

   // copy instance data from default
   LG_memcpy(pspec + sizeof(ObjSpec),pdef + sizeof(ObjSpec),spec_hdr->struct_size - sizeof(ObjSpec));

   // Give back the object ID of the fresh clone
   return(new_obj);
}


// ------------------------------------------------------
// place_obj_at_objloc() puts an object down in the map, 
// adding the appropriate refs for placing an object with the 
// specified xsize and ysize in object coordinates. 
#define MAX_PLACE_SIZE  0x100

void place_obj_at_objloc(ObjID id, ObjLoc *newloc, ushort xsize, ushort ysize)
{
   ObjLocState newstate;
   ushort nxl,nxh,nyl,nyh,bx,by,ox,oy;
   short refcount = 0;
   ObjRefID refid,origref;

   if ((xsize > MAX_PLACE_SIZE) || (ysize > MAX_PLACE_SIZE))
   {
//      Warning(("place_obj_at_objloc: obj %d size too large!! (xsize = 0x%x  ysize = 0x%x)\n",id,xsize,ysize));
      xsize = min(0x200,xsize);
      ysize = min(0x200,ysize);
   }

   // Clear old homesquare data
   origref = refid = objs[id].ref;
   do {
      CitrefClearHomeSq(refid);
      refid = objRefs[refid].nextref;
   } while (refid!=origref);

   newstate.obj = id;
   newstate.loc = *newloc;
   ox = bx = OBJ_LOC_BIN_X(*newloc);
   oy = by = OBJ_LOC_BIN_Y(*newloc);
   newstate.refs[refcount++].bin.sq = MakePoint(bx,by);
   nxl = max(0,(newloc->x - xsize) >> 8);
   nyl = max(0,(newloc->y - ysize) >> 8);
   nxh = min(global_fullmap->x_size, (newloc->x + xsize) >> 8);
   nyh = min(global_fullmap->y_size, (newloc->y + ysize) >> 8);

   if ((ObjProps[OPNUM(id)].render_type == FAUBJ_TEXBITMAP) ||
       (ObjProps[OPNUM(id)].render_type == FAUBJ_TPOLY))
   {
      if (!(newloc->h & ~0xC0))
      {
         if (newloc->h & 0x40)
         {  // Don't go along x axis
            nxl = nxh = ox;
         }
         else
         {  // Don't go along y axis
            nyl = nyh = oy;
         }
      }
   }

   // Cardinal directions...
   for (bx = nxl; bx <= nxh; bx++)
      for (by = nyl; by <= nyh; by++)
         if ((bx != ox) || (by != oy))
            newstate.refs[refcount++].bin.sq = MakePoint(bx,by);
   ObjRefStateBinSetNull(newstate.refs[refcount].bin);
   ObjUpdateLocs(&newstate);

   // Place homesquare -- we can't do this earlier since we don't
   // know refids until earlier
   origref = refid = objs[id].ref;
   do {
      if ((objRefs[refid].state.bin.sq.x == ox) && (objRefs[refid].state.bin.sq.y == oy))
       { CitrefSetHomeSq(refid); break; }
      refid = objRefs[refid].nextref;
   } while (refid!=origref);
}

#define COMPRESSION_FACTOR    fix_make(0,0xD000)

#define FUNKY_GRAV(oloc)   (level_gamedata.hazard.zerogbio && me_hazard_bio(MAP_GET_XY(OBJ_LOC_BIN_X(oloc),OBJ_LOC_BIN_Y(oloc))))
#define FUNKY_GRAV_VAL     (fix_make(level_gamedata.hazard.bio,0))

// Moves an object to an objloc with a given velocity (in physics units, whatever
// they are).
errtype obj_move_to_vel(ObjID id, ObjLoc *newloc, bool phys_tel, fix x_dot, fix y_dot, fix z_dot)
{
   State  new_state;
   ushort xsize = 0, ysize = 0;
   extern cams *_def_cam;

	if (phys_tel && ((ObjProps[OPNUM(id)].physics_model != EDMS_NONE) || (id == PLAYER_OBJ)))
	{
		if (CHECK_OBJ_PH(id))
			EDMS_get_state(objs[id].info.ph, &new_state);
		else
			new_state = standard_state;
		new_state.X = fix_from_obj_coord(newloc->x);
		new_state.Y = fix_from_obj_coord(newloc->y);
		new_state.Z = fix_from_obj_height_val(newloc->z);
		new_state.X_dot = x_dot;
		new_state.Y_dot = y_dot;
		new_state.Z_dot = z_dot;
		new_state.alpha = phys_angle_from_obj(newloc->h);

		if (!(CHECK_OBJ_PH(id)))
		{
			assemble_physics_object(id, &new_state);
			if (!CHECK_OBJ_PH(id))
				return(ERR_NOEFFECT);
		}
		else
			EDMS_holistic_teleport(objs[id].info.ph, &new_state);
	}
   
   if (objs[id].loc.x == -1)
   {
      ObjRefState newref;

      ObjPlace(id, newloc);
      newref.bin.sq.x = OBJ_LOC_BIN_X(*newloc);
      newref.bin.sq.y = OBJ_LOC_BIN_Y(*newloc);
      ObjRefMake(id, newref);
   }

   if (!CHECK_OBJ_PH(id))
   {
      switch (ID2TRIP(id))
      {
         case WORDS_TRIPLE:
            {
               grs_bitmap *bmap;
               char scale;
               bmap = get_text_bitmap_obj(id, 0, &scale);

               // In theory, we take the size of the bitmap, scale it down, then mutliply by 3/4
               xsize = ysize = ((bmap->w << scale) * 3) >> 2;
            }
            break;
         default:
            xsize = ysize = obj_coord_from_fix(fix_make(ObjProps[OPNUM(id)].physics_xr,0) / PHYSICS_RADIUS_UNIT);
            break;
      }
   }
   else
   {
      switch(ObjProps[OPNUM(id)].physics_model)
      {
         case EDMS_ROBOT:
         {
            Robot new_robot;
            EDMS_get_robot_parameters(objs[id].info.ph, &new_robot);
            if (!global_fullmap->cyber)
            {
               if (!((objs[id].obclass == CLASS_CRITTER) && (CritterProps[CPNUM(id)].flags & AI_FLAG_FLYING)))
               {
                  if (FUNKY_GRAV(objs[id].loc))
                  {
                     if (new_robot.gravity == STANDARD_GRAVITY)
                        apply_gravity_to_one_object(id, FUNKY_GRAV_VAL);
                  }
                  else if ((new_robot.gravity != STANDARD_GRAVITY) && (objs[id].obclass != CLASS_PHYSICS))
                     apply_gravity_to_one_object(id, STANDARD_GRAVITY);
               }

               if (objs[id].obclass == CLASS_PHYSICS)
               {
                  if (PhysicsProps[CPNUM(id)].flags & PROJ_LIGHT_FLAG)
                  {
                     ObjSpecID osid = objs[id].specID;

                     if ((OBJ_LOC_TO_LIGHT_LOC(newloc->x) != OBJ_LOC_TO_LIGHT_LOC(objPhysicss[osid].p1.x)) ||
                         (OBJ_LOC_TO_LIGHT_LOC(newloc->y) != OBJ_LOC_TO_LIGHT_LOC(objPhysicss[osid].p1.y)))
                     {
                        MapElem *mmp;

                        if (objPhysicss[osid].p2.x)
                        {
                           mmp=MAP_GET_XY(OBJ_LOC_TO_LIGHT_LOC(objPhysicss[osid].p1.x), OBJ_LOC_TO_LIGHT_LOC(objPhysicss[osid].p1.y));
                           me_rend3_set(mmp, me_bits_rend3(mmp)-1);
                           objPhysicss[osid].p2.x = 0;
                        }
                        mmp=MAP_GET_XY(OBJ_LOC_TO_LIGHT_LOC(newloc->x), OBJ_LOC_TO_LIGHT_LOC(newloc->y));
                        if (!me_bits_rend3(mmp))
                        {
                           me_rend3_set(mmp, me_bits_rend3(mmp)+1);
                           objPhysicss[osid].p2.x = 1;
                        }
                     }
                     objPhysicss[osid].p1 = *newloc;
                  }
               }
            }
            xsize = ysize = obj_coord_from_fix(fix_mul(new_robot.size,COMPRESSION_FACTOR));
            break;
         }

         case EDMS_PELVIS:
         {
#ifdef DIRAC_EDMS
            if (global_fullmap->cyber)
            {
               Dirac_frame new_dirac;
               EDMS_get_Dirac_frame_parameters(objs[id].info.ph, &new_dirac);
               // Do appropiate gravity hacks here someday...

               // Bleh, got to figure out the right size.  Ack!
               // Note that this is completely not the right thing at all.
               // DIRAC_FIX
   #ifdef WE_HAD_ANY_IDEA
               xsize = ysize = obj_coord_from_fix(fix_mul(new_dirac.?????, COMPRESSION_FACTOR));
   #else
               xsize = ysize = 0;
   #endif
            }
            else
#endif
            {
               Pelvis new_pelvis;
               EDMS_get_pelvis_parameters(objs[id].info.ph, &new_pelvis);
               if (!global_fullmap->cyber)
               {
                  if (FUNKY_GRAV(objs[id].loc))
                  {
                     if (new_pelvis.gravity == STANDARD_GRAVITY)
                        apply_gravity_to_one_object(id, FUNKY_GRAV_VAL);
                  } else if (new_pelvis.gravity != STANDARD_GRAVITY)
                     apply_gravity_to_one_object(id, STANDARD_GRAVITY);
               }
               xsize = ysize = obj_coord_from_fix(fix_mul(new_pelvis.size, COMPRESSION_FACTOR));
            }
            break;
         }

         default:
            xsize = ysize = obj_coord_from_fix(fix_make(ObjProps[OPNUM(id)].physics_xr,0) / PHYSICS_RADIUS_UNIT);
            break;

      }
   }
   place_obj_at_objloc(id,newloc,xsize,ysize);

   // If the camera is on the object we are moving, then hey  
   if ((_def_cam != NULL) && (_def_cam->obj_id == id))
      chg_set_flg(_current_3d_flag);

   return(OK);
}

// Moves an object to a given objloc, with no velocity
errtype obj_move_to(ObjID id, ObjLoc *newloc, bool phys_tel)
{
   return(obj_move_to_vel(id,newloc,phys_tel, 0,0,0));
}

// Destroys an object, deals automagically with it's DOS and EDMS
// representations
bool obj_destroy(ObjID id)
{
	int retval = -1;
	extern void check_panel_ref(bool puntme);
	short x,y;
	bool  terrain_object = FALSE;
	
	decrement_shodan_value(id, TRUE);
	if (id != OBJ_NULL) 
	{
		if (player_struct.panel_ref==id)
		{
			check_panel_ref(TRUE);
		}
		if (objs[id].active)
		{
			check_deathwatch_triggers(id,TRUE);
			remove_obj_from_animlist(id);
			switch(objs[id].obclass)
			{
				case CLASS_CRITTER:
					if (objCritters[objs[id].specID].path_id != -1)
						delete_path(objCritters[objs[id].specID].path_id);
					break;
			}
			terrain_object = ((ObjProps[OPNUM(id)].flags & TERRAIN_OBJECT) != 0);
			if (terrain_object)
			{
				x = OBJ_LOC_BIN_X(objs[id].loc);
				y = OBJ_LOC_BIN_Y(objs[id].loc);
			}
			retval = ObjDel(id);
			if (CHECK_OBJ_PH(id))
			{
				EDMS_kill_object(objs[id].info.ph);
				physics_handle_id[objs[id].info.ph]=OBJ_NULL;
				
				if (objs[id].info.ph==physics_handle_max)       // are we the last valid ph?
					while (physics_handle_id[--physics_handle_max]==OBJ_NULL)
					;  // count down through physics_handles till we find an object
			}
			if ((terrain_object) && (x != -1))
				obj_physics_refresh(x,y,FALSE);
		}
		if (trigger_check)
		{
			trigger_check_destroyed(id);
			do_ecology_triggers();
		}
	}
	return(retval);
}


// Create the player's object, both in DOS and EDMS and set appropriate
// player structure state, as well as setting up the camera structures.
errtype obj_create_player(ObjLoc *plr_loc)
{
   State new_state;
   bool use_new = FALSE;
   physics_handle ph;
   Pelvis player_pelvis;
#ifdef DIRAC_EDMS
   Dirac_frame player_dirac;
#endif
   fix pos_list[3];

   player_struct.rep = obj_create_base(PLAYER_TRIP);
   if (player_struct.rep == OBJ_NULL)
   {
      Warning(("MAJOR BADNESS!! Could not create player!\n"));
      return(ERR_NOEFFECT);
   }
   player_dos_obj = &(objs[PLAYER_OBJ]);
   fr_camera_create(&player_cam,CAMTYPE_OBJ|CAMMOD_USEMOD,(fix *)player_struct.rep,0);
   if (cam_mode == OBJ_PLAYER_CAMERA)
      fr_camera_setdef(&player_cam);

   pos_list[0]=fix_from_obj_coord(plr_loc->x)>>8;
   pos_list[1]=fix_from_obj_coord(plr_loc->y)>>8;
   pos_list[2]=fix_from_obj_height_val(plr_loc->z)>>8;
   fr_objslew_go_real_height(NULL, (long *)pos_list);
   plr_loc->z=obj_height_from_fix(pos_list[2]<<8);

   if ((player_struct.edms_state[0]) && (!global_fullmap->cyber))
   {
      extern void state_to_objloc(State *s, ObjLoc *l);
      LG_memcpy(&new_state, player_struct.edms_state, sizeof(fix)*12);
      state_to_objloc(&new_state,plr_loc);
      use_new = TRUE;
   }
   else
   {
      new_state = standard_state;
      new_state.X = pos_list[0]<<8;
      new_state.Y = pos_list[1]<<8;
      new_state.Z = pos_list[2]<<8;
      new_state.alpha = phys_angle_from_obj(plr_loc->h);
      new_state.beta = 0;
      new_state.gamma = 0;
   }

#ifdef DIRAC_EDMS
   if (global_fullmap->cyber)
   {
      instantiate_dirac(PLAYER_TRIP,&player_dirac);
      objs[PLAYER_OBJ].info.ph = ph = EDMS_make_Dirac_frame(&player_dirac, &new_state);
      new_cyber_orient = TRUE;
   }
   else
#endif
   {
      instantiate_pelvis(PLAYER_TRIP,&player_pelvis);
      objs[PLAYER_OBJ].info.ph = ph = EDMS_make_pelvis(&player_pelvis, &new_state);
   }
   physics_handle_id[ph] = PLAYER_OBJ;

   if (ph>physics_handle_max)
      physics_handle_max=ph;

   obj_move_to(PLAYER_OBJ, plr_loc, !use_new);

   if ( (!global_fullmap->cyber) && (ocp_settle_the_player) )
      EDMS_settle_object(ph);

   return(OK);
}


// -------------------------------------------------
// ObjClassInit()
//
// ## INSERT NEW OBJ PROPS HERE
//

errtype ObjClassInit(ObjID id, ObjSpecID specid, int subclass)
{
   ubyte *pspec, *pdef;
   ObjSpecHeader *spec_hdr = &objSpecHeaders[objs[id].obclass]; 

   objs[id].subclass = subclass;
   objs[id].active = TRUE;
   if (id != PLAYER_OBJ)
   {
      objs[id].info.current_hp = ObjProps[OPNUM(id)].hit_points;
      objs[id].info.make_info = 0;
   }
   switch(objs[id].obclass)
   {
      case CLASS_GUN: pspec = (ubyte *)&objGuns[specid]; pdef = (ubyte *)&default_gun; break;
      case CLASS_AMMO: pspec = (ubyte *)&objAmmos[specid]; pdef = (ubyte *)&default_ammo; break;
      case CLASS_PHYSICS: pspec = (ubyte *)&objPhysicss[specid]; pdef = (ubyte *)&default_physics; break;
      case CLASS_GRENADE: pspec = (ubyte *)&objGrenades[specid]; pdef = (ubyte *)&default_grenade; break;
      case CLASS_DRUG: pspec = (ubyte *)&objDrugs[specid]; pdef = (ubyte *)&default_drug; break;
      case CLASS_HARDWARE: pspec = (ubyte *)&objHardwares[specid]; pdef = (ubyte *)&default_hardware; break;
      case CLASS_SOFTWARE: pspec = (ubyte *)&objSoftwares[specid]; pdef = (ubyte *)&default_software; break;
      case CLASS_BIGSTUFF: pspec = (ubyte *)&objBigstuffs[specid]; pdef = (ubyte *)&default_bigstuff; break;
      case CLASS_SMALLSTUFF: pspec = (ubyte *)&objSmallstuffs[specid]; pdef = (ubyte *)&default_smallstuff; break;
      case CLASS_FIXTURE: pspec = (ubyte *)&objFixtures[specid]; pdef = (ubyte *)&default_fixture; break;
      case CLASS_DOOR: pspec = (ubyte *)&objDoors[specid]; pdef = (ubyte *)&default_door; break;
      case CLASS_ANIMATING: pspec = (ubyte *)&objAnimatings[specid]; pdef = (ubyte *)&default_animating; break;
      case CLASS_TRAP: pspec = (ubyte *)&objTraps[specid]; pdef = (ubyte *)&default_trap; break;
      case CLASS_CONTAINER: pspec = (ubyte *)&objContainers[specid]; pdef = (ubyte *)&default_container; break;
      case CLASS_CRITTER: pspec = (ubyte *)&objCritters[specid]; pdef = (ubyte *)&default_critter; break;
   }

   // copy instance data from default
   LG_memcpy(pspec + sizeof(ObjSpec),pdef + sizeof(ObjSpec),spec_hdr->struct_size - sizeof(ObjSpec));
   return(OK);
}

// ------------------------------------------------
// obj_load_properties()
//
// ## INSERT NEW CLASS HERE
// ## INSERT NEW SUBCLASS HERE

errtype obj_load_properties()
{
	 Handle	res;
	 int		version, i, j;
	 char		*cp;

	extern void SwapLongBytes(void *pval4);
	extern void SwapShortBytes(void *pval2);

/*  For Mac version, replaced with GetResource

   strcpy(levname, OBJPROP_FILENAME);
   if (DatapathFind(&DataDirPath, levname, path))
   {
      Spew(DSRC_GFX_Anim, ("objprop path = %s\n",path));
      fd = open(path, O_RDONLY|O_BINARY);
      if (fd != -1)
      {
    sz += read(fd, &version, sizeof(version));
    if (version != OBJPROP_VERSION_NUMBER)
    {
#ifndef PLAYTEST
       critical_error(CRITERR_MISC|0);
#else // !PLAYTEST
       Warning(("Bad Object Properties version number.\n"));
#endif 
    }
*/
	
	// Read in the Object Properties resource.
	
	res = GetResource('oprp',1000);
	if (!res) return(ERR_FOPEN);
	HLock(res);
	cp = *res;
	
	// Check to make sure we have the right version.
	version = *(int *)cp;
	cp += 4;
	SwapLongBytes(&version);
	if (version != OBJPROP_VERSION_NUMBER)
		critical_error(CRITERR_MISC|0);
	
	// Copy the data to the various global arrays, converting as needed.
	
	//-------------
	// GUNS
	//-------------
	BlockMoveData(cp, GunProps, sizeof(GunProps));
	cp += sizeof(GunProps);
	
	//BlockMoveData(cp, PistolGunProps, NUM_PISTOL_GUN);		Dummies
	cp += NUM_PISTOL_GUN;
	
	//BlockMoveData(cp, AutoGunProps, NUM_AUTO_GUN);				Dummies
	cp += NUM_AUTO_GUN;
	
	for (i = 0; i < NUM_SPECIAL_GUN; i++)
	{
		SpecialGunProp *sgp = &SpecialGunProps[i];
		
		sgp->damage_modifier = *(short *)cp;
		cp += 2;
		SwapShortBytes(&sgp->damage_modifier);
		sgp->offense_value = *cp++;
		sgp->damage_type = *(int *)cp;
		cp += 4;
		SwapLongBytes(&sgp->damage_type);
		sgp->penetration = *cp++;
		
		sgp->speed = *cp++;
		sgp->proj_triple = *(int *)cp;
		cp += 4;
		SwapLongBytes(&sgp->proj_triple);
		sgp->attack_mass = *cp++;
		sgp->attack_speed = *(short *)cp;
		cp += 2;
		SwapShortBytes(&sgp->attack_speed);
	}
	
	for (i = 0; i < NUM_HANDTOHAND_GUN; i++)
	{
		HandtohandGunProp *hhgp = &HandtohandGunProps[i];
		
		hhgp->damage_modifier = *(short *)cp;
		cp += 2;
		SwapShortBytes(&hhgp->damage_modifier);
		hhgp->offense_value = *cp++;
		hhgp->damage_type = *(int *)cp;
		cp += 4;
		SwapLongBytes(&hhgp->damage_type);
		hhgp->penetration = *cp++;
		
		hhgp->energy_use = *cp++;
		hhgp->attack_mass = *cp++;
		hhgp->attack_range = *cp++;
		hhgp->attack_speed = *(short *)cp;
		cp += 2;
		SwapShortBytes(&hhgp->attack_speed);
	}
	
	for (i = 0; i < NUM_BEAM_GUN; i++)
	{
		BeamGunProp *bgp = &BeamGunProps[i];
		
		bgp->damage_modifier = *(short *)cp;
		cp += 2;
		SwapShortBytes(&bgp->damage_modifier);
		bgp->offense_value = *cp++;
		bgp->damage_type = *(int *)cp;
		cp += 4;
		SwapLongBytes(&bgp->damage_type);
		bgp->penetration = *cp++;
		
		bgp->max_charge = *cp++;
		bgp->attack_mass = *cp++;
		bgp->attack_range = *cp++;
		bgp->attack_speed = *(short *)cp;
		cp += 2;
		SwapShortBytes(&bgp->attack_speed);
	}
	
	for (i = 0; i < NUM_BEAMPROJ_GUN; i++)
	{
		BeamprojGunProp *bgp = &BeamprojGunProps[i];
		
		bgp->damage_modifier = *(short *)cp;
		cp += 2;
		SwapShortBytes(&bgp->damage_modifier);
		bgp->offense_value = *cp++;
		bgp->damage_type = *(int *)cp;
		cp += 4;
		SwapLongBytes(&bgp->damage_type);
		bgp->penetration = *cp++;
		
		bgp->max_charge = *cp++;
		bgp->attack_mass = *cp++;
		bgp->attack_speed = *(short *)cp;
		cp += 2;
		SwapShortBytes(&bgp->attack_speed);
		bgp->speed = *cp++;
		bgp->proj_triple = *(int *)cp;
		cp += 4;
		SwapLongBytes(&bgp->proj_triple);
		bgp->flags = *cp++;
	}

	//-------------
	//  AMMO
	//-------------
	for (i = 0; i < NUM_AMMO; i++)
	{
		AmmoProp *ap = &AmmoProps[i];
		
		ap->damage_modifier = *(short *)cp;
		cp += 2;
		SwapShortBytes(&ap->damage_modifier);
		ap->offense_value = *cp++;
		ap->damage_type = *(int *)cp;
		cp += 4;
		SwapLongBytes(&ap->damage_type);
		ap->penetration = *cp++;
		
		ap->cartridge_size = *cp++;
		ap->bullet_mass = *cp++;
		ap->bullet_speed = *(short *)cp;
		cp += 2;
		SwapShortBytes(&ap->bullet_speed);
		ap->range = *cp++;
		ap->recoil_force = *cp++;
	}

	//BlockMoveData(cp, PistolAmmoProps, NUM_PISTOL_AMMO);				Dummies
	cp += NUM_PISTOL_AMMO;

	//BlockMoveData(cp, PistolAmmoProps, NUM_NEEDLE_AMMO);				Dummies
	cp += NUM_NEEDLE_AMMO;

	//BlockMoveData(cp, PistolAmmoProps, NUM_MAGNUM_AMMO);			Dummies
	cp += NUM_MAGNUM_AMMO;

	//BlockMoveData(cp, PistolAmmoProps, NUM_RIFLE_AMMO);				Dummies
	cp += NUM_RIFLE_AMMO;

	//BlockMoveData(cp, PistolAmmoProps, NUM_FLECHETTE_AMMO);			Dummies
	cp += NUM_FLECHETTE_AMMO;

	//BlockMoveData(cp, PistolAmmoProps, NUM_AUTO_AMMO);					Dummies
	cp += NUM_AUTO_AMMO;

	//BlockMoveData(cp, PistolAmmoProps, NUM_PROJ_AMMO);					Dummies
	cp += NUM_PROJ_AMMO;

	//-------------
	//  PHYSICS
	//-------------
	
	for (i = 0; i < NUM_PHYSICS; i++)
		PhysicsProps[i].flags = *cp++;

	for (i = 0; i < NUM_TRACER_PHYSICS; i++)
	{
		TracerPhysicsProp *tpp = &TracerPhysicsProps[i];
		
		BlockMoveData(cp, tpp->xcoords, 4*2);
		cp += 4*2;
		BlockMoveData(cp, tpp->ycoords, 4*2);
		cp += 4*2;
		BlockMoveData(cp, tpp->zcoords, 4);
		cp += 4;
		
		for (j = 0; j < 4; j++)
		{
			SwapShortBytes(&tpp->xcoords[j]);
			SwapShortBytes(&tpp->ycoords[j]);
		}
	}

	BlockMoveData(cp, SlowPhysicsProps, sizeof(SlowPhysicsProps));
	cp += sizeof(SlowPhysicsProps);

	//BlockMoveData(cp, CameraPhysicsProps, NUM_CAMERA_PHYSICS);			Dummies
	cp += NUM_CAMERA_PHYSICS;

	//-------------
	//  GRENADES
	//-------------

	for (i = 0; i < NUM_GRENADE; i++)
	{
		GrenadeProp *gp = &GrenadeProps[i];
		
		gp->damage_modifier = *(short *)cp;
		cp += 2;
		SwapShortBytes(&gp->damage_modifier);
		gp->offense_value = *cp++;
		gp->damage_type = *(int *)cp;
		cp += 4;
		SwapLongBytes(&gp->damage_type);
		gp->penetration = *cp++;
		
		gp->touchiness = *cp++;
		gp->radius = *cp++;
		gp->radius_change = *cp++;
		gp->damage_change = *cp++;
		gp->attack_mass = *cp++;
		gp->flags = *(short *)cp;
		cp += 2;
		SwapShortBytes(&gp->flags);
	}

	//BlockMoveData(cp, DirectGrenadeProps, NUM_DIRECT_GRENADE);			Dummies
	cp += NUM_DIRECT_GRENADE;

	for (i = 0; i < NUM_TIMED_GRENADE; i++)
	{
		TimedGrenadeProp *tgp = &TimedGrenadeProps[i];
		
		tgp->min_time_set = *cp++;
		tgp->max_time_set = *cp++;
		tgp->timing_deviation = *cp++;
	}

	//-------------
	//  DRUGS
	//-------------
	// For Mac version:  The data for these are all zero (I don't think they're used at all), so don't worry about
	// converting, just zero the arrays.
	
	LG_memset(DrugProps, 0, sizeof(DrugProps));
	cp += NUM_DRUG * 17;
	LG_memset(StatsDrugProps, 0, sizeof(StatsDrugProps));
	cp += NUM_STATS_DRUG * 7;

	//-------------
	//  HARDWARE
	//-------------
	// For Mac version:  The data for these are all zero, so don't worry about
	// converting, just zero the arrays.
	
	LG_memset(HardwareProps, 0, sizeof(HardwareProps));
	cp += NUM_HARDWARE * 2;
	LG_memset(GoggleHardwareProps, 0, sizeof(GoggleHardwareProps));
	cp += NUM_GOGGLE_HARDWARE;
	LG_memset(HardwareHardwareProps, 0, sizeof(HardwareHardwareProps));
	cp += NUM_HARDWARE_HARDWARE * 2;

	//-------------
	//  SOFTWARE
	//-------------
	// For Mac version:  The data for these are all zero, so don't worry about
	// converting, just zero the arrays.
	
	LG_memset(SoftwareProps, 0, sizeof(SoftwareProps));
	cp += NUM_SOFTWARE * 2;
	cp += NUM_OFFENSE_SOFTWARE;
	cp += NUM_DEFENSE_SOFTWARE;
	cp += NUM_ONESHOT_SOFTWARE;
	cp += NUM_MISC_SOFTWARE;
	cp += NUM_DATA_SOFTWARE;

	//-------------
	//  BIGSTUFF
	//-------------
	// For Mac version:  The data for these are all zero, so don't worry about
	// converting, just zero the arrays.
	
	LG_memset(BigstuffProps, 0, sizeof(BigstuffProps));
	cp += NUM_BIGSTUFF * 4;
	cp += NUM_ELECTRONIC_BIGSTUFF;
	cp += NUM_FURNISHING_BIGSTUFF;
	cp += NUM_ONTHEWALL_BIGSTUFF;
	cp += NUM_LIGHT_BIGSTUFF;
	cp += NUM_LABGEAR_BIGSTUFF;
	cp += NUM_TECHNO_BIGSTUFF;
	cp += NUM_DECOR_BIGSTUFF;
	cp += NUM_TERRAIN_BIGSTUFF;

	//-------------
	//  SMALLSTUFF
	//-------------
	// For Mac version:  Most of this data is all zeros, so in those cases don't worry about
	// converting, just zero the arrays.
	
	LG_memset(SmallstuffProps, 0, sizeof(SmallstuffProps));
	cp += NUM_SMALLSTUFF * 2;
	cp += NUM_USELESS_SMALLSTUFF;
	cp += NUM_BROKEN_SMALLSTUFF;
	cp += NUM_CORPSELIKE_SMALLSTUFF;
	cp += NUM_GEAR_SMALLSTUFF;
	cp += NUM_CARDS_SMALLSTUFF;
	
	BlockMoveData(cp, CyberSmallstuffProps, sizeof(CyberSmallstuffProps));
	cp += sizeof(CyberSmallstuffProps);

	cp += NUM_ONTHEWALL_SMALLSTUFF;

	LG_memset(PlotSmallstuffProps, 0, sizeof(PlotSmallstuffProps));
	cp += NUM_PLOT_SMALLSTUFF * 2;

	//-------------
	//  FIXTURES
	//-------------
	// For Mac version:  The data for these are all zero, so don't worry about
	// converting, just zero the arrays.
	
	LG_memset(FixtureProps, 0, sizeof(FixtureProps));
	cp += NUM_FIXTURE;
	cp += NUM_CONTROL_FIXTURE;
	cp += NUM_RECEPTACLE_FIXTURE;
	cp += NUM_TERMINAL_FIXTURE;
	cp += NUM_PANEL_FIXTURE;
	cp += NUM_VENDING_FIXTURE;
	cp += NUM_CYBER_FIXTURE;

	//-------------
	//  DOORS
	//-------------
	// For Mac version:  The data for these are all zero, so don't worry about
	// converting, just zero the arrays.
	
	LG_memset(DoorProps, 0, sizeof(DoorProps));
	cp += NUM_DOOR;
	cp += NUM_NORMAL_DOOR;
	cp += NUM_DOORWAYS_DOOR;
	cp += NUM_FORCE_DOOR;
	cp += NUM_ELEVATOR_DOOR;
	cp += NUM_SPECIAL_DOOR;
	
	cp -= 2;											//��� We got off here somehow.  Check into it!!!

	//----------------
	//  ANIMATING OBJECTS
	//----------------
	
	BlockMoveData(cp, AnimatingProps, sizeof(AnimatingProps));
	cp += sizeof(AnimatingProps);

	cp += NUM_OBJECT_ANIMATING;
	cp += NUM_TRANSITORY_ANIMATING;

	for (i = 0; i < NUM_EXPLOSION_ANIMATING; i++)
		ExplosionAnimatingProps[i].frame_explode = *cp++;

	//----------------
	//  TRAPS
	//----------------
	// For Mac version:  These are all dummy arrays, so just skip over the data here.
	
	cp += NUM_TRAP;
	cp += NUM_TRIGGER_TRAP;
	cp += NUM_FEEDBACKS_TRAP;
	cp += NUM_SECRET_TRAP;

	//----------------
	//  CONTAINERS
	//----------------
	// For Mac version:  The data for these are all zero, so don't worry about
	// converting, just zero the arrays.
	
	LG_memset(ContainerProps, 0, sizeof(ContainerProps));
	cp += NUM_CONTAINER * 3;
	cp += NUM_ACTUAL_CONTAINER;
	cp += NUM_WASTE_CONTAINER;
	cp += NUM_LIQUID_CONTAINER;
	cp += NUM_MUTANT_CORPSE_CONTAINER;
	cp += NUM_ROBOT_CORPSE_CONTAINER;
	cp += NUM_CYBORG_CORPSE_CONTAINER;
	cp += NUM_OTHER_CORPSE_CONTAINER;

	//----------------
	//  CRITTERS
	//----------------
	
	for (i = 0; i < NUM_CRITTER; i++)
	{
		CritterProp *crp = &CritterProps[i];
		
		crp->intelligence = *cp++;
		for (j = 0; j < NUM_ALTERNATE_ATTACKS; j++)
		{
			crp->attacks[j].damage_type = *(int *)cp; cp += 4;
			SwapLongBytes(&crp->attacks[j].damage_type);
			crp->attacks[j].damage_modifier = *(short *)cp; cp += 2;
			SwapShortBytes(&crp->attacks[j].damage_modifier);
			crp->attacks[j].offense_value = *cp++;
			crp->attacks[j].penetration = *cp++;
			crp->attacks[j].attack_mass = *cp++;
			crp->attacks[j].attack_velocity = *(short *)cp; cp += 2;
			SwapShortBytes(&crp->attacks[j].attack_velocity);
			crp->attacks[j].accuracy = *cp++;
			crp->attacks[j].att_range = *cp++;
			crp->attacks[j].speed = *(int *)cp; cp += 4;
			SwapLongBytes(&crp->attacks[j].speed);
			crp->attacks[j].slow_proj = *(int *)cp; cp += 4;
			SwapLongBytes(&crp->attacks[j].slow_proj);
		}
		crp->perception = *cp++;
		crp->defense = *cp++;
		crp->proj_offset = *cp++;
		crp->flags = *(int *)cp; cp += 4;
		SwapLongBytes(&crp->flags);
		crp->mirror = *cp++;
		for (j = 0; j < NUM_CRITTER_POSTURES; j++)
			crp->frames[j] = *cp++;
		crp->anim_speed = *cp++;
		crp->attack_sound = *cp++;
		crp->near_sound = *cp++;
		crp->hurt_sound = *cp++;
		crp->death_sound = *cp++;
		crp->notice_sound = *cp++;
		crp->corpse = *(int *)cp; cp += 4;
		SwapLongBytes(&crp->corpse);
		crp->views = *cp++;
		crp->alt_perc = *cp++;
		crp->disrupt_perc = *cp++;
		crp->treasure_type = *cp++;
		crp->hit_effect = *cp++;
		crp->fire_frame = *cp++;
	}

	cp += NUM_MUTANT_CRITTER;

	BlockMoveData(cp, RobotCritterProps, sizeof(RobotCritterProps));
	cp += sizeof(RobotCritterProps);

	BlockMoveData(cp, CyborgCritterProps, sizeof(CyborgCritterProps));
	cp += sizeof(CyborgCritterProps);
	for (i = 0; i < NUM_CYBORG_CRITTER; i++)
		SwapShortBytes(&CyborgCritterProps[i].shield_energy);

	for (i = 0; i < NUM_CYBER_CRITTER; i++)
	{
		CyberCritterProp *ccp = &CyberCritterProps[i];
		
		for (j = 0; j < NUM_VCOLORS; j++)
			ccp->vcolors[j] = *cp++;
		for (j = 0; j < NUM_VCOLORS; j++)
			ccp->alt_vcolors[j] = *cp++;
	}

	cp += NUM_ROBOBABE_CRITTER;

	//-----------------------
	//  GENERAL OBJECT PROPERTIES
	//-----------------------

	for (i = 0; i < NUM_OBJECT; i++)
	{
		ObjProp *op = &ObjProps[i];
		
		BlockMoveData(cp, op, 27);
		cp += 27;

		SwapLongBytes(&op->mass);
		SwapShortBytes(&op->hit_points);
		SwapLongBytes(&op->resistances);
		SwapShortBytes(&op->flags);
		SwapShortBytes(&op->mfd_id);
		SwapShortBytes(&op->bitmap_3d);
	}

	HUnlock(res);
	ReleaseResource(res);

	return(OK);
}

errtype obj_set_secondary_properties()
{
	FSSpec		fSpec;
	char 			i, j;
	RefTable 	*prt;
	int 			fn, fn2;
	
	FSMakeFSSpec(gDataVref, gDataDirID, "\pobjart2.rsrc", &fSpec);
	fn = ResOpenFile(&fSpec);
	FSMakeFSSpec(gDataVref, gDataDirID, "\pobjart3.rsrc", &fSpec);
	fn2 = ResOpenFile(&fSpec);
	if ((fn < 0) || (fn2 < 0))
	{
//		Warning(("Problem opening object art cache!\n"));
		return(ERR_NOEFFECT);
	}
	
	// Now go and set specific computed data, like frame counts
	for (i=0; i < NUM_CRITTER; i++)
	{
		// KLC size = 0;
		if (CritterProps[i].views > 1)
		{
			for (j=0; j < NUM_CRITTER_POSTURES; j++)
			{
				Id id;
				if (j >= FIRST_FRONT_POSTURE)
				{
					id = posture_bases[j] + i;
					prt = ResReadRefTable(id);							//	prt = (RefTable *)ResLock(id);
					// KLC size += ResSize(posture_bases[j] + i);
				}
				else
				{
					id = critter_id_table[i] + posture_bases[j] + 6;  // assumes view = front = 6
					prt = ResReadRefTable(id);									//	prt = (RefTable *)ResLock(id);
					// KLC size += ResSize(posture_bases[j] + critter_id_table[i] + 6);
				}
				if (prt == NULL) ;
//				Warning (("Could not read RefTable for creature type %d (j = %d) (id = %d + %d = %d (0x%x))!\n",i,j,
//				critter_id_table[i],posture_bases[j],critter_id_table[i] + posture_bases[j], critter_id_table[i] + posture_bases[j]));
				else
				{                
					CritterProps[i].frames[j] = prt->numRefs;
					ResFreeRefTable(prt);
//					ResUnlock(id);
//					ResDrop(id);
				}
				AdvanceProgress();
			}
		}
	}
	ResCloseFile(fn);
	ResCloseFile(fn2);
	return(OK);
}

// this function isnt a rep exposure at all, except knowing internal structure order
errtype obj_zero_unused(void)
{
   ObjID id;
   int cl;
   ObjSpecID specid;
   int counters[2][2];

   LG_memset(counters,0,4*sizeof(int));
   for (cl= CLASS_GUN; cl<NUM_CLASSES; cl++)
   {
      ObjSpecHeader *curHead=&objSpecHeaders[cl]; /* get our special class header data */
      for (specid=1; specid<curHead->size; specid++)
      {  /* find the base of our obj and cast it to a ObjSpec common header struct */
    ObjSpec *curSpec=(ObjSpec*)(curHead->data+(curHead->struct_size*specid));
    if (!objs[id=curSpec->bits.id].active)         /* toast all but the Spec part */
       LG_memset(((char *) curSpec)+sizeof(ObjSpec),0,curHead->struct_size-sizeof(ObjSpec));
    counters[0][objs[id].active?1:0]++;
      }
   }
   for (id = 0; id < NUM_OBJECTS; id++) /* go through all objects, though i bet they are all seen above */
      if (!objs[id].active)             /* and thus we should be able to skip this, i bet */
      {
    LG_memset(&objs[id].active,0,((uchar *)&objs[id].ref)-((uchar *)&objs[id].active));
    LG_memset(&objs[id].loc,0,sizeof(ObjLoc)+sizeof(ObjInfo));
    counters[1][0]++;
      }
      else counters[1][1]++; 
//   mprintf("Counters were %d %d and %d %d\n",counters[0][0],counters[0][1],counters[1][0],counters[1][1]);
   return OK;
}

ObjID physics_handle_to_id(physics_handle p)
{
   if (p > MAX_OBJ)
      return(OBJ_NULL);
   else
      return(physics_handle_id[p]);
}

/* KLC - not used
bool get_obj_radii(Obj *objp, fix *rad)
{
   Robot temp_robot;
   
   if (objp->info.ph != -1)
   {
      EDMS_get_robot_parameters(objp->info.ph, &temp_robot);
      *rad = temp_robot.size;
   }
   else
      *rad = 0;

//   *rad1=fix_make(ObjProps[objtrip].physics_xr,0)/96;
//   *rad2=fix_make(0,0x4000); // probably should be something else here, eh?
//   return TRUE;
   return FALSE;
}
*/

#define ICON_ID_BASE RES_bmIconArt_0
#define GRAF_ID_BASE RES_bmGraffitiArt_0
#define REPL_ID_BASE RES_bmRepulsArt_0

Ref obj_cache_ref(ObjID id)
{
   Ref retval = OBJ_REF_NULL;
   switch (objs[id].obclass)
   {
      case CLASS_DOOR:
    retval = MKREF(door_id(id), objs[id].info.current_frame);
    break;
      case CLASS_BIGSTUFF:
    if (objs[id].subclass == BIGSTUFF_SUBCLASS_ONTHEWALL)
    {
       switch (ID2TRIP(id))
       {
          case ICON_TRIPLE:
        retval = MKREF(ICON_ID_BASE, objs[id].info.current_frame);
        break;
          case GRAF_TRIPLE:
        retval = MKREF(GRAF_ID_BASE, objs[id].info.current_frame);
        break;
          case REPULSWALL_TRIPLE:
        retval = MKREF(REPL_ID_BASE, objs[id].info.current_frame);
        break;
          default:
        break;
       }
    }
    break;
   }
   return(retval);
}

#define  MEDIAN_WORD_SCALE 4

grs_bitmap *get_text_bitmap_from_string(int d1, char dest_type, char *s, bool scroll, int scroll_index)
{
   Id currfont;
   short w,h,c,x;
   char sc_count = 0;
   char size_remaining = text_bitmaps_y[dest_type];
   char curr_y = 1;

   gr_push_canvas(&text_canvases[dest_type]);

   // hack MLA 
   if (clearwithFF) 
   		gr_clear(0xff); 
   else
   		gr_clear(0);

   c = d1 >> 16;
   if (c == 0)
      c = RED_BASE + 6;
   gr_set_fcolor(c);
   switch (d1 & 0xF)
   {
      case 1: currfont = RES_graffitiFont; break;
      case 2: currfont = RES_smallTechFont; break;
      case 3: currfont = RES_largeTechFont; break;
      default: currfont = RES_citadelFont; break;
   }
   gr_set_font((grs_font *)ResLock(currfont));
   gr_string_size(s, &w, &h);
   while (size_remaining > h)
   {
      if (scroll)
         gr_string(s, 1, curr_y);
      else {
         x=(text_bitmaps_x[dest_type]-w)>>1;
         if(x<1) x=1;
         gr_string(s, x, (text_bitmaps_y[dest_type] - h) >> 1);
      }
      if (!scroll)
         size_remaining = 0;
      else
      {
         sc_count++;
         size_remaining -= h;
         curr_y += h + 1;
         s = get_temp_string(text_bitmap_refs[dest_type] + scroll_index + sc_count);
         gr_string_size(s, &w, &h);
      }
   }
   ResUnlock(currfont);
   gr_pop_canvas();
   return(text_bitmap_ptrs[dest_type]);
}

grs_bitmap *get_text_bitmap(int d1, int d2, char dest_type, bool scroll)
{
   return(get_text_bitmap_from_string(d1, dest_type, get_temp_string(text_bitmap_refs[dest_type] + d2), scroll, d2));
}

grs_bitmap *get_text_bitmap_obj(ObjID cobjid, char dest_type, char *pscale)
{
   char sval = (objBigstuffs[objs[cobjid].specID].data1 & 0xF0) >> 4;
   if (sval == 0)
      *pscale = 0;
   else
      *pscale = sval - MEDIAN_WORD_SCALE;
   return(get_text_bitmap(objBigstuffs[objs[cobjid].specID].data1, objBigstuffs[objs[cobjid].specID].cosmetic_value, dest_type,FALSE));
}

errtype obj_settle_func(ObjID id)
{
   int retval;
   if (!CHECK_OBJ_PH(id))
      return(OK);
//   if ((!global_fullmap->cyber) && (id == PLAYER_OBJ))
   if (!global_fullmap->cyber)
      retval = EDMS_settle_object(objs[id].info.ph);
   else
      retval = TRUE;
//   if (retval < 0)
//      Warning(("EDMS_settle on id %d is unhappy!\n",id));
   return(OK);
}


void destroy_screen_callback_func(ObjID id, void *user_data);
void diego_teleport_callback(ObjID id, void *user_data);

#define DESTROYED_SCREEN_ANIM_BASE 0x1B

void destroy_screen_callback_func(ObjID id, void *)
{
   ObjSpecID osid = objs[id].specID;
   objBigstuffs[osid].cosmetic_value = 1;
   objBigstuffs[osid].data2 = DESTROYED_SCREEN_ANIM_BASE + 3;
   objs[id].info.current_frame = 0;
}

void diego_teleport_callback(ObjID id, void *)
{
	obj_destroy(id);
}


// A critter has been killed -- do we let it die like usual or
// do we do something wacky?

// in gameobj.c also
#define DIEGO_DEATH_BATTLE_LEVEL 8

bool death_check(ObjID id, bool*)
{
   extern char damage_sound_fx;
   if(ID2TRIP(id)==DIEGO_TRIPLE && player_struct.level!=DIEGO_DEATH_BATTLE_LEVEL) 
   {
      damage_sound_fx = -1;
      play_digi_fx_obj(SFX_TELEPORT, 1, id);
      remove_obj_from_animlist(id);
      add_obj_to_animlist(id,FALSE,FALSE,FALSE,32,1,NULL,ANIMCB_REMOVE);
   }
   return FALSE;
}


// An object has been destroyed -- now we must consider doing some 
// special stuff.  Returns TRUE if the regular destruction process
// should continue.
bool obj_combat_destroy(ObjID id)
{
   bool retval = TRUE;
   ObjSpecID osid = objs[id].specID;
   int i,*d1,*d2;
   extern ObjID hack_cam_objs[NUM_HACK_CAMERAS];
   extern ObjID hack_cam_surrogates[NUM_HACK_CAMERAS];
   extern bool is_container(ObjID id, int** d1, int** d2);
   extern char container_extract(ObjID *pidlist, int d1, int d2);
   extern ObjID damage_sound_id;
   extern char damage_sound_fx;

   // Check to see if we are a camera-surrogate
   // or a hack camera itself
   for (i=0; i < NUM_HACK_CAMERAS; i++)
   {
      if ((hack_cam_surrogates[i] == id) || (hack_cam_objs[i] == id))
      {
         hack_cam_objs[i] = OBJ_NULL;
         hack_cam_surrogates[i] = OBJ_NULL;
      }
   }

   // Actually do any stuff relevant to us in specific
   if (is_container(id, &d1, &d2)) {
      do_random_loot(id);
      spew_contents(id,*d1,(d2 == NULL) ? 0 : *d2);
   }
   switch(objs[id].obclass)
   {
      case CLASS_CRITTER:
         check_deathwatch_triggers(id,FALSE);
         mai_monster_defeated();
         if(!global_fullmap->cyber) {
            if ((id == player_struct.curr_target) || (player_struct.curr_target == OBJ_NULL))
            {
               player_struct.curr_target = OBJ_NULL;
               mfd_notify_func(MFD_TARGET_FUNC, MFD_TARGET_SLOT, FALSE, MFD_ACTIVE, TRUE);
            }
            player_struct.num_victories++;
         }
         ai_critter_die(osid);
         if(death_check(id,&retval))
            return retval;
         retval = FALSE;
         break;
      case CLASS_GRENADE:
         ADD_DESTROYED_OBJECT(id);
//         do_grenade_explosion(id,TRUE);
//         objGrenades[osid].unique_id = 0;
         break;
      case CLASS_SMALLSTUFF:
         switch (ID2TRIP(id))
         {
            case TARGET_TRIPLE:
               if (objSmallstuffs[objs[id].specID].data1)
                  obj_destroy(objSmallstuffs[objs[id].specID].data1);
               break;
            case MULTIPLEXR_TRIPLE:
               player_struct.cspace_time_base = min(CSPACE_MAX_TIME, player_struct.cspace_time_base + CSPACE_MUX_BONUS);
               break;
         }
         break;
      case CLASS_BIGSTUFF:
         switch (ID2TRIP(id))
         {
            case TV_TRIPLE:
            case MONITOR2_TRIPLE:
            case SCREEN_TRIPLE:
            case BIGSCREEN_TRIPLE:
            case SUPERSCREEN_TRIPLE:
               remove_obj_from_animlist(id);
               objs[id].info.current_frame = 0;
               objBigstuffs[osid].cosmetic_value = 4;
               objBigstuffs[osid].data1 = 0;
               objBigstuffs[osid].data2 = DESTROYED_SCREEN_ANIM_BASE;
               add_obj_to_animlist(id, 0, 0, 0, 0,2, NULL, ANIMCB_REMOVE);
               damage_sound_fx = SFX_MONITOR_EXPLODE;
               damage_sound_id = id;
               retval = FALSE;
               break;
            case CAMERA_TRIPLE:
               damage_sound_fx = SFX_CAMERA_EXPLODE;
               damage_sound_id = id;
               break;
            case LARGCPU_TRIPLE:
               damage_sound_fx = SFX_CPU_EXPLODE;
               damage_sound_id = id;
               break;
            default:
               damage_sound_fx = SFX_DESTROY_BARREL;
               damage_sound_id = id;
               break;
         }
         break;
      case CLASS_CONTAINER:
         {
            MapElem *pme = MAP_GET_XY(OBJ_LOC_BIN_X(objs[id].loc), OBJ_LOC_BIN_Y(objs[id].loc));
            switch (ID2TRIP(id))
            {
               case RAD_BARREL_TRIPLE:
                  damage_sound_fx = SFX_DESTROY_BARREL;
                  damage_sound_id = id;
                  me_hazard_rad_set(pme,TRUE);
                  break;
               case TOXIC_BARREL_TRIPLE:
               case CHEM_TANK_TRIPLE:
                  damage_sound_fx = SFX_DESTROY_BARREL;
                  damage_sound_id = id;
                  if (!level_gamedata.hazard.zerogbio)
                     me_hazard_bio_set(pme,TRUE);
                  break;
               case SML_CRT_TRIPLE:
               case LG_CRT_TRIPLE:
               case SECURE_CONTR_TRIPLE:
                  damage_sound_fx = SFX_DESTROY_CRATE;
                  damage_sound_id = id;
                  break;
            }
         }
         break;
   }
   return(retval);
}

errtype obj_floor_func(ObjID id);

#define DEFAULT_AI_WAIT 15
ObjID object_place(int triple, LGPoint square)
{
   ObjID new_id;
   ObjLoc loc;
   short flrh;
   errtype retval;
   extern void cit_sleeper_callback(physics_handle caller);
   int newsize;

   new_id = obj_create_base(triple);
   if (new_id == OBJ_NULL)
   {
      return(OBJ_NULL);
   }
   flrh = me_height_flr(MAP_GET_XY(square.x,square.y));
   newsize = ObjProps[OPTRIP(triple)].physics_xr;
   if (newsize == 0)
      newsize = standard_robot.size;
   else  // convert from newstuff.otx format to appropriate physics units
      newsize = fix_make(newsize,0) / PHYSICS_RADIUS_UNIT;

   loc.z = obj_height_from_fix(fix_from_map_height(flrh) + newsize);

   loc.p = 0;   loc.h = 0;   loc.b = 0;
   loc.x = (square.x << 8) + 0x80;   loc.y = (square.y << 8) + 0x80;
   if (ObjProps[OPNUM(new_id)].flags & EDMS_PRESERVE)
      retval = obj_move_to(new_id, &loc,TRUE);
   else
      retval = obj_move_to(new_id, &loc,FALSE);    // so that things that don't care about physics don't get physics models
   if (retval != OK)
      ObjDel(new_id);
   if (CHECK_OBJ_PH(new_id))
   {
      extern void edms_delete_go();
      obj_settle_func(new_id);
      cit_sleeper_callback(objs[new_id].info.ph);     // rock-a-bye, object.
      edms_delete_go();
   }
   else
      obj_floor_func(new_id);
   if (objs[new_id].obclass == CLASS_CRITTER)
   {
      // Randomize some initial data
      objCritters[objs[new_id].specID].wait_frames = rand()%DEFAULT_AI_WAIT;
      objCritters[objs[new_id].specID].path_id = -1;
      objs[new_id].info.current_frame = rand()%(max(1,CritterProps[CPNUM(new_id)].frames[0] - 2));
   }
   return(new_id);
}

ushort obj_floor_compute(ObjID id, uchar flrh)
{
   fix newsize;

   if ((ObjProps[OPNUM(id)].render_type == FAUBJ_TEXTPOLY)
#ifndef NO_ANTIGRAV_CRATES
        ||  (ObjProps[OPNUM(id)].render_type == FAUBJ_SPECIAL)
#endif
      )
      newsize = 0;
   else
   {
      newsize = ObjProps[OPNUM(id)].physics_z;
      if(newsize==0)
         newsize = ObjProps[OPNUM(id)].physics_xr;
      if (newsize == 0)
         newsize = standard_robot.size;
      else  // convert from newstuff.otx format to appropriate physics units
         newsize = fix_make(newsize,0) / PHYSICS_RADIUS_UNIT;
   }
   return(obj_height_from_fix(fix_from_map_height(flrh) + newsize));
}

ushort obj_floor_height(ObjID id)
{
   return(obj_floor_compute(id, me_height_flr(MAP_GET_XY(OBJ_LOC_BIN_X(objs[id].loc),OBJ_LOC_BIN_Y(objs[id].loc)))));
}

errtype obj_floor_func(ObjID id)
{
   void edms_delete_go(void);

   ObjLoc newloc = objs[id].loc;
   newloc.z = obj_floor_height(id);
   obj_move_to(id, &newloc, TRUE);
   obj_settle_func(id);
   edms_delete_go();
   return(OK);
}

#ifdef NOT_YET //��� later

#ifdef PLAYTEST
#pragma disable_message(202)
bool global_settle_func(short keycode, ulong context, void* data)
{
   ObjID oid;
   message_info("settling all objects.");
   FORALLOBJS(oid)
   {
      obj_settle_func(oid);
   }
   return(FALSE);
}

bool global_floor_func(short keycode, ulong context, void* data)
{
   ObjID oid;
   message_info("flooring all objects.");
   FORALLOBJS(oid)
   {
      obj_floor_func(oid);
   }
   return(FALSE);
}

bool check_objsys_func(short keycode, ulong context, void* data)
{
   int i;
   char buf[64];
   extern char *get_object_lookname(ObjID id,char use_string[], int sz);
   Warning(("Checking objsys, looking for bad geninv\n"));
   for (i=0; i < NUM_GENERAL_SLOTS; i++)
   {
      if (player_struct.inventory[i] != OBJ_NULL)
      {
         if (!objs[player_struct.inventory[i]].active)
            Warning(("HEY, geninv %d, id 0x%x, is not active!  Ack!!!\n",i,player_struct.inventory[i]));
         else
            Warning(("%d: %s\n",i, get_object_lookname(player_struct.inventory[i],buf,64)));
      }
   }   
   if (ObjSysOkay())
      message_info("ObjSys OKAY");
   else
      message_info("ObjSys BAD!");
   return(FALSE);
}

// Just compile in whichever hack it is you want to 
// use to munge all the objects on the level

//#define TEXTURE_CRUNCH_HACK
//#define DELTA_FILENAME  "changepx.lst"
//#define SEVERED_HEAD_MUNGE
//#define NO_REFS_MUNGE
//#define CLEAR_CREATURE_PATHFIND
//#define CRITTER_HP_CONVERT
//#define CRITTER_FLAG_CLEAR
//#define CRITTER_HP_SETNORM
//#define NULL_OBJ_OBJREF_HACK
//#define REFLOOR_CRATES_HACK
//#define DOOR_HEIGHT_SQUARE
//#define ELDER_DEMON_EXORCISM
//#define TEETH
//#define ELEVATOR_CHECKERBOARD
//#define PARAMETER_DESTRUCTION

#ifdef PARAMETER_DESTRUCTION
#include <tilename.h>
#include <mprintf.h>
#endif

#ifdef ELDER_DEMON_EXORCISM
#define MAX_EXOR  10
#endif

#ifdef CRITTER_HP_CONVERT
static short old_critter_hp[] = {
   25, 325, 400, 160, 200, 65, 60, 300, 150,
   0, 50, 20, 160, 0, 125, 225, 450, 15, 110, 60, 0,
   65, 180, 45, 275, 400, 550, 450,
   30, 60, 250, 250, 150, 400, 60,
   750, 400
   };
#endif

errtype obj_level_munge()
{
   short count = 0;
#ifdef ELDER_DEMON_EXORCISM
   ObjID oid;
   ObjRefID oref;
   short x,y;
   MapElem *pme;
   bool found;
   char buf[128];
   ObjID exorcism[MAX_EXOR];
   char exorcise_count = 0;
#endif
#ifdef REFLOOR_CRATES_HACK
   ObjID oid;
#endif
#ifdef NO_REFS_MUNGE
   ObjID oid, next;
#endif
#ifdef CLEAR_CREATURE_PATHFIND
   ObjSpecID osid;
   ObjID id;
#endif
#ifdef NULL_OBJ_OBJREF_HACK
   ObjRefID orefid,nextref,oref2;
   short x,y;
#endif
#ifdef ELEVATOR_CHECKERBOARD
   short x,y;
   MapElem *pme;

   for (x=0; x < MAP_XSIZE; x++)
   {
      for (y=0; y < MAP_YSIZE; y++)
      {
         pme = MAP_GET_XY(x,y);
         if ((count % 2) == 0)
            me_bits_music_set(pme,7);
         count++;
      }
   }

#endif

#ifdef PARAMETER_DESTRUCTION
{
   short x,y;
   MapElem* pme;
   uchar par, chgt, mir, tt;

   for(x=0;x<MAP_XSIZE;x++) {
      for(y=0;y<MAP_YSIZE;y++) {
         pme=MAP_GET_XY(x,y);
         tt=me_tiletype(pme);

         if(tt<TILE_SLOPEUP_N && (par=me_param(pme))!=0) {
            mir=me_bits_mirror(pme);
            if(mir==MAP_MATCH || mir==MAP_FFLAT) {
               chgt=me_height_ceil(pme)+par;
               if(chgt>=MAP_HEIGHTS)
                  chgt=MAP_HEIGHTS-1;
               me_height_ceil_set(pme,chgt);
            }
            me_param_set(pme,0);
         }
      }
   }
}
#endif
#ifdef ELDER_DEMON_EXORCISM
   FORALLOBJS(oid)
   {
#ifdef TEETH
      Warning(("checking id %x\n",oid));
#endif
      found = FALSE;
      for (x=0; x < MAP_XSIZE; x++)
      {
         for (y=0; y < MAP_YSIZE; y++)
         {
            pme = MAP_GET_XY(x,y);
            oref = me_objref(pme);
            while (oref != OBJ_REF_NULL)
            {
               if (objRefs[oref].obj == oid)
               {
                  found = TRUE;
                  x = MAP_XSIZE;
                  y = MAP_YSIZE;
                  break;
               }
               oref = objRefs[oref].next;
            }
         }
      }
      if (!found)
      {
         extern char *get_object_lookname(ObjID id,char use_string[], int sz);
         Warning(("HEY, id %x, a %s, may have the taint of Shadow!\n",oid, get_object_lookname(oid,buf,128)));
         Warning(("id %x, ref = %x\n",oid,objs[oid].ref));
         exorcism[exorcise_count++] = oid;
      }
   }
   Warning(("done scanning...\n"));
#ifdef TEETH
   for (x = 0; x < exorcise_count; x++)
   {
      extern ObjID ObjRefFree (ObjRefID this, bool cleanup);
      oref = objs[exorcism[x]].ref;
      objRefs[oref].next = OBJ_REF_NULL;
      ObjRefFree(oref,TRUE);
      Warning(("Hey, deleted the ref (%x) for %x!\n",oref,exorcism[x]));
   }
   for (x = 0; x < exorcise_count; x++)
   {
      objs[exorcism[x]].ref = OBJ_REF_NULL;
      obj_destroy(exorcism[x]);
//      ObjDel(exorcism[x]);
      Warning(("deleted object %x!\n",exorcism[x]));
   }
#endif
   Warning(("done with ritual (ok = %d)!\n",ObjSysOkay()));
#endif

#ifdef REFLOOR_CRATES_HACK
   FORALLOBJS(oid)
   {
      switch(ID2TRIP(oid))
      {
         case SML_CRT_TRIPLE:
         case LG_CRT_TRIPLE:
         case SECURE_CONTR_TRIPLE:
         case RAD_BARREL_TRIPLE:
         case TOXIC_BARREL_TRIPLE:
         case CHEM_TANK_TRIPLE:
            obj_floor_func(oid);
            break;
      }
   }
#endif

#ifdef NULL_OBJ_OBJREF_HACK
   for (x=0; x < MAP_XSIZE; x++)
   {
      for (y=0; y < MAP_YSIZE; y++)
      {
         orefid = me_objref(MAP_GET_XY(x,y));
         while (orefid != OBJ_REF_NULL)
         {
            nextref = objRefs[orefid].next;
            if (objRefs[orefid].obj == OBJ_NULL)
            {
               Warning(("****** Deleting objref %d!\n",orefid));
               me_objref_set(MAP_GET_XY(x,y),OBJ_REF_NULL);
               oref2 = objRefs[0].next;
               while (objRefs[oref2].next != OBJ_REF_NULL)
               {
                  mprintf(".");
                  oref2 = objRefs[oref2].next;
               }
               Warning(("objRefs[%d].next = %d\n",oref2,objRefs[oref2].next));
               objRefs[oref2].next = orefid;
               Warning(("after: objRefs[%d].next = %d\n",oref2,objRefs[oref2].next));
               objRefs[orefid].next = OBJ_REF_NULL;
            }
            orefid = nextref;
         }
      }
   }
#endif

#ifdef CRITTER_FLAG_CLEAR
   ObjSpecID osid;
   ObjID id;
   osid = objCritters[0].id;
   while (osid != OBJ_SPEC_NULL) 
   {
      id = objCritters[osid].id;
      objCritters[osid].flags = 0;
      osid = objCritters[osid].next;
   }
#endif

#ifdef DOOR_HEIGHT_SQUARE
{
   ObjSpecID osid;
   ObjID id;
   int z;

   osid=objDoors[0].id;
   while(osid != OBJ_SPEC_NULL)
   {
      id=objDoors[osid].id;

      if(objs[id].loc.p==0) {
         z=objs[id].loc.z;
         z=(z+4)&(~7);
         objs[id].loc.z=z;
      }
      osid=objDoors[osid].next;
   }
}
#endif

#ifdef CLEAR_CREATURE_PATHFIND
   osid = objCritters[0].id;
   while (osid != OBJ_SPEC_NULL) 
   {
      id = objCritters[osid].id;
      objCritters[osid].path_id = -1;
      objCritters[osid].des_speed = 0;
      objCritters[osid].urgency = 0;
      osid = objCritters[osid].next;
   }
   used_paths = 0;
#endif

#ifdef TEXTURE_CRUNCH_HACK
   extern errtype texture_crunch_go();
   texture_crunch_go();
#endif

#ifdef SEVERED_HEAD_MUNGE
   ObjSpecID osid;
   ObjID id;

   osid = objSmallstuffs[0].id;
   while (osid != OBJ_SPEC_NULL) {
      id = objSmallstuffs[osid].id;
      if(ID2TRIP(id)==HEAD_TRIPLE || ID2TRIP(id)==HEAD2_TRIPLE) {
         objs[id].info.make_info = 0;
      }
      osid = objSmallstuffs[osid].next;
   }
#endif

#ifdef CRITTER_HP_CONVERT
   {
      ObjSpecID osid;
      ObjID id;
      int hp;

      osid = objCritters[0].id;
      while (osid != OBJ_SPEC_NULL) {
         id = objCritters[osid].id;
         hp = objs[id].info.current_hp;
         {
            hp *= ObjProps[OPNUM(id)].hit_points;
            hp /= old_critter_hp[get_nth_from_triple(ID2TRIP(id))];
            objs[id].info.current_hp = hp;
         }

         osid = objCritters[osid].next;
      }
   }
#endif

#ifdef CRITTER_HP_SETNORM
   {
      ObjSpecID osid;
      ObjID id;
      int hp;

      osid = objCritters[0].id;
      while (osid != OBJ_SPEC_NULL) {
         id = objCritters[osid].id;
         objs[id].info.current_hp = ObjProps[OPNUM(id)].hit_points;

         osid = objCritters[osid].next;
      }
   }
#endif

#ifdef NO_REFS_MUNGE
   for(oid=objs[OBJ_NULL].headused;oid!=OBJ_NULL;oid=next) {
      next = objs[oid].next;
      if(objs[oid].ref == OBJ_REF_NULL) {
         ObjDel(oid);
      }
   }
#endif

#ifdef LEVEL_MUNGE_HP_HACK
   ObjID id;
   ObjSpecID osid;

   osid = objBigstuffs[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      id = objBigstuffs[osid].id;
      switch(ID2TRIP(id))
      {
         case GENE_SPLICER_TRIPLE:
         case LARGCPU_TRIPLE:
         case SCREEN_TRIPLE:
         case BIGSCREEN_TRIPLE:
         case SUPERSCREEN_TRIPLE:
            if (objs[id].info.current_hp != 0)
            {
               objs[id].info.current_hp = ObjProps[OPNUM(id)].hit_points;
               count++;
            }
            break;
      }
      osid = objBigstuffs[osid].next;
   }
#endif

#ifdef APPLY_SIZE_DELTA_HACK
   short size_conv[NUM_OBJECT];
   FILE *f;
   short i,sc;
   char temp1[50];
   fix old_ht;   
   ObjID oid;

   // Fill it up with vomitous spew
   f = fopen(DELTA_FILENAME, "r");
   i=0;
   while (!feof(f) && (i < NUM_OBJECT))
   {
      fgets(temp1,50,f);
      size_conv[i] = atoi(temp1);
      Spew(DSRC_TESTING_Test4, ("size_conv[%d] = %d  temp1=%s",i,size_conv[i],temp1));
      i++;
   }
   fclose(f);

   FORALLOBJS(oid)
   {
      if ((ObjProps[OPNUM(oid)].render_type == FAUBJ_BITMAP) ||
          (ObjProps[OPNUM(oid)].render_type == FAUBJ_MULTIVIEW))
      {
         sc = size_conv[OPNUM(oid)];
         if (sc != 0)
         {
            count++;
      
            old_ht = fix_from_obj_height(oid);
            objs[oid].loc.z = obj_height_from_fix(old_ht + (fix_make(sc,0) / PHYSICS_RADIUS_UNIT));
         }
      }
   }
#endif

#ifdef FRESHEN_CORPSES_HACK
   ObjID oid;
   FORALLOBJS(oid)
   {
      if ((ID2TRIP(oid) >= CORPSE1_TRIPLE) && (ID2TRIP(oid) <= CORPSE8_TRIPLE))
      {
         objs[oid].info.inst_flags |= CLASS_INST_FLAG;
         count++;
      }
   }
#endif

   Spew(DSRC_EDITOR_Modify, ("munged %d objects!\n",count));
   Spew(DSRC_TESTING_Test4, ("munged %d objects!\n",count));

   return(OK);
}
#pragma enable_message(202)
#endif

#ifdef LOUD_REFRESH
void spew_about_stuff(char *txt, ObjID id)
{
   State new_state;
   EDMS_get_state(objs[id].info.ph, &new_state);
   mprintf("id %x %s: %x %x %x %x %x %x \n     dots %x %x %x %x %x %x\n",id,txt,
     new_state.X,new_state.Y,new_state.Z,new_state.alpha,new_state.beta,new_state.gamma,
     new_state.X_dot,new_state.Y_dot,new_state.Z_dot,new_state.alpha_dot,new_state.beta_dot,new_state.gamma_dot);
}
#else
#define spew_about_stuff(txt, id)
#endif

#endif //NOT_YET ���

extern bool robot_antisocial;

#define MAX_MOVE_OBJS   32
// uchar of height above the ground to refresh within
#define REFRESH_HEIGHT  0x10
errtype obj_physics_refresh(short x, short y, bool use_floor)
{
   ObjRefID oref;
   ObjID id;
   State goof;
   int count = 0,i;
   ObjID move_list[MAX_MOVE_OBJS];

   oref = me_objref(MAP_GET_XY(x,y));
   while (oref != OBJ_REF_NULL)
   {
      ObjID oid=objRefs[oref].obj;
//      if (!ObjCheckDealt(oid))
      if (1)
      {
         ObjSetDealt(oid);
	      move_list[count++] = oid;
	      if (count == MAX_MOVE_OBJS)
	      {
	         oref = OBJ_REF_NULL;
	      }
	      else
	         oref = objRefs[oref].next;
      }
      else
         oref = objRefs[oref].next;
   }
   for (i=0; i < count; i++)
   {
      id = move_list[i];
      if (id == OBJ_NULL)
         ;
      else if (id != PLAYER_OBJ)
      {
         // If we are on the floor, then refresh us!
         if ((ObjProps[OPNUM(id)].physics_model) && (use_floor || (objs[id].loc.z > obj_floor_height(id) + REFRESH_HEIGHT)))
         {
            if (CHECK_OBJ_PH(id))
            {
               EDMS_get_state(objs[id].info.ph, &goof);
               obj_move_to_vel(id, &objs[id].loc, TRUE, goof.X_dot, goof.Y_dot, goof.Z_dot);
               EDMS_crystal_meth(objs[id].info.ph);
            }
            else
            {
               // if we're going to wake it up - make it antisocial
               // cause everybody is antisocial when they wake up!
               robot_antisocial = TRUE;
               obj_move_to(id, &objs[id].loc, TRUE);
               robot_antisocial = FALSE;
            }
         }
      }
   }
   return(OK);
}

errtype obj_physics_refresh_area(short x, short y, bool use_floor)
{
   ObjsClearDealt();
   obj_physics_refresh(x-1,y,use_floor);
   obj_physics_refresh(x+1,y,use_floor);
   obj_physics_refresh(x,y-1,use_floor);
   obj_physics_refresh(x,y+1,use_floor);
   return(obj_physics_refresh(x,y,use_floor));
}

bool obj_is_display(int triple)
{
   if (ObjProps[OPTRIP(triple)].render_type == FAUBJ_TEXTPOLY)
      return(TRUE);
   switch (triple)
   {
      case SCREEN_TRIPLE:
      case SUPERSCREEN_TRIPLE:
      case BIGSCREEN_TRIPLE:
         return(TRUE);
   }
   return(FALSE);
}

