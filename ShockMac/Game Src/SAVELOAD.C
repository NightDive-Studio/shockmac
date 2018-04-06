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
 * $Source: r:/prj/cit/src/RCS/saveload.c $
 * $Revision: 1.145 $
 * $Author: xemu $
 * $Date: 1994/11/21 21:07:36 $
 */

#include <string.h>

#include "ShockDialogs.h"
#include "MacTune.h"

#include "saveload.h"
#include "criterr.h"
#include "cyber.h"
#include "cybmem.h"
#include "dynmem.h"
#include "effect.h"
#include "frflags.h"
#include "frprotox.h"
#include "gamerend.h"
#include "gamewrap.h"
#include "hkeyfunc.h"
#include "input.h"
#include "lvldata.h"
#include "objapp.h"
#include "objects.h"
#include "objload.h"
#include "objprop.h"
#include "objsim.h"
#include "objwpn.h"
#include "objwarez.h"
#include "objstuff.h"
#include "objgame.h"
#include "objcrit.h"
#include "objver.h"
#include "otrip.h"
#include "mainloop.h"
#include "map.h"
#include "mfdext.h"
#include "musicai.h"
#include "pathfind.h"
#include "physics.h"
#include "player.h"
#include "render.h"
#include "rendtool.h"
#include "schedule.h"
#include "sfxlist.h"
#include "shodan.h"
#include "statics.h"
#include "textmaps.h"
#include "tilemap.h"
#include "tools.h"
#include "trigger.h"
#include "verify.h"

/*
#include <schedule.h>
#include <dpaths.h>
#include <mapflags.h>
#include <frtables.h>
#include <frsubclp.h>
#include <flicker.h>
#include <ai.h>
#include <wsample.h>
#include <memstat.h>
#include <_gamesys.h>
// #include <amap.h>  // hey, this is included in lvldata.h
#include <ckpoint.h>
#include <mlimbs.h>

#include <mprintf.h>
*/


// INTERNAL PROTOTYPES
// -----------------
void load_level_data();
void store_objects(char** buf, ObjID *obj_array, char obj_count);
void restore_objects(char* buf, ObjID *obj_array, char obj_count);
errtype write_id(Id id_num, short index, void *ptr, long sz, int fd, short flags);


#define REF_WRITE(id_num,index,x) write_id(id_num, index, &(x), sizeof(x), fd, 0);    AdvanceProgress()
#define REF_WRITE_LZW(id_num,index,x) write_id(id_num, index, &(x), sizeof(x), fd, RDF_LZW);    AdvanceProgress()
#define REF_WRITE_RAW(id_num,index,ptr,sz) write_id(id_num,index,ptr,sz,fd,RDF_LZW);    AdvanceProgress()
#define REF_READ(id_num,index,x) ResExtract(id_num + index, &(x));  AdvanceProgress()


#define FIRST_CSPACE_LEVEL    14

#define SAVE_AUTOMAP_STRINGS

char saveload_string[30];
bool display_saveload_checkpoints = FALSE;
bool saveload_static = FALSE;
uint dynmem_mask = DYNMEM_ALL;

extern ObjID hack_cam_objs[NUM_HACK_CAMERAS];
extern ObjID hack_cam_surrogates[NUM_HACK_CAMERAS];
extern height_semaphor h_sems[NUM_HEIGHT_SEMAPHORS];
extern bool trigger_check;


//-------------------------------------------------------
void store_objects(char** buf, ObjID *obj_array, char obj_count)
{
   char*	s = (char *)NewPtr(obj_count * sizeof(Obj) * 3);
   int 		i;
   if (s == NULL)
      critical_error(CRITERR_MEM|3);
   *buf = s;
   for (i = 0; i < obj_count; i++)
   {
   check_null:
      if (obj_array[i] != OBJ_NULL)
      {
         ObjID id = obj_array[i];
         ObjSpecHeader* sh = &objSpecHeaders[objs[id].obclass];
         // dumb hack to avoid page fault for now, are not we cool
         if (!objs[id].active)
         {
            obj_array[i] = OBJ_NULL;
            goto check_null;
         }
            *(Obj*)s = objs[id];
         s+= sizeof(Obj);
         LG_memcpy(s,sh->data+sh->struct_size*objs[id].specID,sh->struct_size);
         s+= sh->struct_size;
         ObjDel(id);
      }
      else
      {

         ((Obj*)s)->active = FALSE;
         s+= sizeof(Obj);
      }
   }
}


//-------------------------------------------------------
void restore_objects(char* buf, ObjID *obj_array, char obj_count)
{
   char* s = buf;
   int i;
   for (i = 0; i < obj_count; i++)
   {
      Obj* next = (Obj*)s;
      if (!next->active)
      {
         s+=sizeof(Obj);
         obj_array[i] = OBJ_NULL;
      }
      else
      {
         ObjID id = obj_create_base(MAKETRIP(next->obclass,next->subclass,next->info.type));
         ObjSpecHeader* sh = &objSpecHeaders[next->obclass];
         char* spec;
         s+=sizeof(Obj);
         if (id != OBJ_NULL)
         {
            objs[id].info = next->info;
            spec = (sh->data+sh->struct_size*objs[id].specID);
            LG_memcpy(spec+sizeof(ObjSpec),s+sizeof(ObjSpec),sh->struct_size-sizeof(ObjSpec));
            obj_array[i] = id;
         }
         s+=sh->struct_size;
      }
   }
   DisposePtr((Ptr)buf);
}


//-------------------------------------------------------
#define SECRET_VOODOO_ENDGAME_CSPACE   10

bool go_to_different_level(int targlevel)
{
   State player_state;
   char* buf;
   extern void update_level_gametime(void);
   extern errtype do_level_entry_triggers();
   bool in_cyber = global_fullmap->cyber;
   bool retval = FALSE;
   errtype rv;

   dynmem_mask = DYNMEM_ALL;
   if ((targlevel >= FIRST_CSPACE_LEVEL) || (targlevel == SECRET_VOODOO_ENDGAME_CSPACE))
   {
      if (!in_cyber)
      {
         // indicate static
         saveload_static = TRUE;

         // force to fullscreen

         enter_cyberspace_stuff(targlevel);
         retval = TRUE;
      }
   }
   else if (in_cyber)
   {
      saveload_static = TRUE;
   }
   if (saveload_static)
   {
      extern void physics_zero_all_controls();
      extern void start_asynch_digi_fx();
      bool old_music;
      physics_zero_all_controls();
      fr_global_mod_flag(FR_SOLIDFR_STATIC, FR_SOLIDFR_MASK);
      render_run();
      play_digi_fx(SFX_ENTER_CSPACE,-1);
      old_music = music_on;
      if (music_on)
         stop_music();
      music_on = old_music;
//KLC      start_asynch_digi_fx();
      dynmem_mask = DYNMEM_PARTIAL;
   }
//   else if (music_on)									// Don't play music while
//      MacTuneShutdown();								// elevator switches levels

	// KLC - if changing levels via the elevator, we need to make sure the elevator music
	//			  continues playing.  We'll do this by queueing up 3 additional chunks of music.
	else if (music_on)
	{
		for (int t = 0; t < 3; t++)
		{
			MacTuneQueueTune(mlimbs_boredom);
			mlimbs_boredom++;
			if (mlimbs_boredom >= 8)
				mlimbs_boredom = 0;
		}
	}

   // make a note of when we left;
   level_gamedata.exit_time = player_struct.game_time;

   // Zap the player to there...
   store_objects(&buf, player_struct.inventory, NUM_GENERAL_SLOTS);
   if (!in_cyber)
   {
      EDMS_get_state(objs[PLAYER_OBJ].info.ph, &player_state);
      LG_memcpy(player_struct.edms_state, &player_state, sizeof (fix) * 12);
   } 
   else
   {
      extern errtype early_exit_cyberspace_stuff();
      early_exit_cyberspace_stuff();
   }
      
   rv = write_level_to_disk(ResIdFromLevel(player_struct.level), TRUE);
   if (rv)
      critical_error(CRITERR_FILE|4);
   
   rv = load_level_from_file(targlevel);
   // Hmm, should we criterr out on this case?
   restore_objects(buf,player_struct.inventory, NUM_GENERAL_SLOTS);
   obj_load_art(FALSE);
   
   // reset renderer data
   game_fr_reparam(-1,-1,-1);

   // move the level to the present
   if (!in_cyber && !global_fullmap->cyber)
      update_level_gametime();

   // exit cyberspace
   if (in_cyber)
   {
      exit_cyberspace_stuff();
   }

   // Now do any level-entry triggers
   do_level_entry_triggers();

   // Set the current target to null.
   player_struct.curr_target = OBJ_NULL;

   // Undo static if we did it before
   if (saveload_static)
   {
      extern void clear_digi_fx();
      extern void stop_asynch_digi_fx();
      saveload_static = FALSE;
      fr_global_mod_flag(0,FR_SOLIDFR_MASK);
      if (music_on)
      	start_music();
//KLC      stop_asynch_digi_fx();
      clear_digi_fx();
   }
/*  if no music while elevator moves
   else if (music_on)
   {
		if (MacTuneInit() == 0)
		{
			mlimbs_on = TRUE;
			mlimbs_AI_init();
			mlimbs_boredom = TickCount() % 8;
			load_score_guts(7);
			MacTuneStartCurrentTheme();
		}
   }
*/
/*KLC
   else if (music_on)
      mlimbs_return_to_synch();
*/
   dynmem_mask = DYNMEM_ALL;
   mfd_force_update();
   return(retval);
}


#define ANOTHER_DEFINE_FOR_NUM_LEVELS  16

errtype write_id(Id id_num, short index, void *ptr, long sz, int fd, short flags)
{
   ResMake(id_num + index, ptr, sz, RTYPE_APP, fd, flags);
   if (ResWrite(id_num + index) == -1)
      critical_error(CRITERR_FILE|6);
   ResUnmake(id_num + index);
   return(OK);
}

errtype save_current_map(FSSpec* fSpec, Id id_num, bool /*flush_mem*/, bool )
{
   int i,goof;
   int idx = 0;
   int fd;
   int vnum = MAP_VERSION_NUMBER;
   int ovnum = OBJECT_VERSION_NUMBER;
   int mvnum = MISC_SAVELOAD_VERSION_NUMBER;
   ObjLoc plr_loc;
   bool make_player = FALSE;
   State player_edms;
   int verify_cookie = 0;

//KLC - Mac cursor showing at this time   begin_wait();

   // make pathfinding state stable by fulfilling PF requests
   check_requests(FALSE);

/* KLC - not needed for game
   if (id_num - LEVEL_ID_NUM < ANOTHER_DEFINE_FOR_NUM_LEVELS)
   {  // were in the editor, so clear out game state hack stupid i suck kill me 
      fr_compile_rect(global_fullmap,0,0,MAP_XSIZE,MAP_YSIZE,TRUE);
   }
*/

   // do not ecology while player is being destroyed and created.
   trigger_check=FALSE;

   // save off physics stuff
   EDMS_get_state(objs[PLAYER_OBJ].info.ph, &player_edms);
   if (PLAYER_OBJ != OBJ_NULL)
   {
      plr_loc = objs[PLAYER_OBJ].loc;
      obj_destroy(PLAYER_OBJ);
      make_player = TRUE;
   }

   // Read appropriate state modifiers
//���   if (flush_mem)
//���      free_dynamic_memory(DYNMEM_PARTIAL);
   AdvanceProgress();

   // Open the file we're going to save into.
   fd = ResEditFile(fSpec,TRUE);
   if (fd < 0)
   {
//KLC      end_wait();
      return ERR_FOPEN;
   }
   AdvanceProgress();

//KLC - just write the last one   REF_WRITE(SAVELOAD_VERIFICATION_ID, 0, verify_cookie);

   idx++;	//KLC - not used   REF_WRITE(id_num,idx++,vnum);
   idx++;	//KLC - not used   REF_WRITE(id_num,idx++,ovnum);   
   REF_WRITE(id_num,idx++,*global_fullmap);

   REF_WRITE_RAW(id_num,idx++,MAP_MAP,sizeof(MapElem) << (MAP_XSHF + MAP_YSHF));

   // Here we are writing out the schedules.  It's only a teeny tiny rep exposure.  
   for (i = 0; i < NUM_MAP_SCHEDULES; i++)
   {
      int sz = min(global_fullmap->sched[i].queue.fullness+1,global_fullmap->sched[i].queue.size);
      REF_WRITE_RAW(id_num,idx++,global_fullmap->sched[i].queue.vec, sizeof(SchedEvent)*sz);
   }
   REF_WRITE(id_num,idx++,loved_textures);

   obj_zero_unused();
   REF_WRITE_LZW(id_num,idx++,objs);
   REF_WRITE_LZW(id_num,idx++,objRefs);
   REF_WRITE(id_num,idx++,objGuns);
   REF_WRITE(id_num,idx++,objAmmos);
   REF_WRITE_LZW(id_num,idx++,objPhysicss);
   REF_WRITE(id_num,idx++,objGrenades);
   REF_WRITE(id_num,idx++,objDrugs);
   REF_WRITE(id_num,idx++,objHardwares);
   REF_WRITE(id_num,idx++,objSoftwares);
   REF_WRITE_LZW(id_num,idx++,objBigstuffs);
   REF_WRITE_LZW(id_num,idx++,objSmallstuffs);
   REF_WRITE_LZW(id_num,idx++,objFixtures);
   REF_WRITE_LZW(id_num,idx++,objDoors);
   REF_WRITE(id_num,idx++,objAnimatings);
   REF_WRITE_LZW(id_num,idx++,objTraps);
   REF_WRITE_LZW(id_num,idx++,objContainers);
   REF_WRITE_LZW(id_num,idx++,objCritters);

   // Default objects
   REF_WRITE(id_num,idx++,default_gun);
   REF_WRITE(id_num,idx++,default_ammo);
   REF_WRITE(id_num,idx++,default_physics);
   REF_WRITE(id_num,idx++,default_grenade);
   REF_WRITE(id_num,idx++,default_drug);
   REF_WRITE(id_num,idx++,default_hardware);
   REF_WRITE(id_num,idx++,default_software);
   REF_WRITE(id_num,idx++,default_bigstuff);
   REF_WRITE(id_num,idx++,default_smallstuff);
   REF_WRITE(id_num,idx++,default_fixture);
   REF_WRITE(id_num,idx++,default_door);
   REF_WRITE(id_num,idx++,default_animating);
   REF_WRITE(id_num,idx++,default_trap);
   REF_WRITE(id_num,idx++,default_container);
   REF_WRITE(id_num,idx++,default_critter);

	idx++;  //KLC - not used   REF_WRITE(id_num,idx++,mvnum);

//   idx++; // where flickers once lived
	idx++;	//KLC - not used   REF_WRITE(id_num,idx++,filler);
	REF_WRITE(id_num,idx++,animtextures);

	REF_WRITE(id_num,idx++,hack_cam_objs);
	REF_WRITE(id_num,idx++,hack_cam_surrogates);

   // Other level data -- at resource id right after maps
   level_gamedata.size = sizeof(level_gamedata);
   REF_WRITE(id_num, idx++, level_gamedata);
#ifdef SAVE_AUTOMAP_STRINGS
//   REF_WRITE(id_num, idx++, amap_str_reref(0));
//��� LZW later   ResMake(id_num + (idx++), &(amap_str_reref(0)), AMAP_STRING_SIZE, RTYPE_APP, fd,  RDF_LZW);
   ResMake(id_num + (idx++), (amap_str_reref(0)), AMAP_STRING_SIZE, RTYPE_APP, fd,  0);
   ResWrite(id_num + (idx - 1));
   ResUnmake(id_num + (idx - 1));
   goof = amap_str_deref(amap_str_next());
   REF_WRITE(id_num, idx++, goof);
#endif
   idx++;	//KLC - no need to be saved.   REF_WRITE(id_num, idx++, player_edms); 
   REF_WRITE(id_num, idx++, paths);
   REF_WRITE(id_num, idx++, used_paths);
   REF_WRITE(id_num, idx++, animlist);
   REF_WRITE(id_num, idx++, anim_counter);
   REF_WRITE(id_num, idx++, h_sems);

/* KLC - not used
   if (pack)
   {
      int reclaim;
      reclaim = ResPack(fd);
      if (reclaim == 0)
         Warning(("%d bytes reclaimed from ResPack!\n",reclaim));
   }
*/
   verify_cookie = VERIFY_COOKIE_VALID;
   REF_WRITE(SAVELOAD_VERIFICATION_ID,0,verify_cookie);
   ResCloseFile(fd);
   
   FlushVol(nil, fSpec->vRefNum);			// Make sure everything is saved.
   
   if (make_player)
      obj_create_player(&plr_loc);
   trigger_check=TRUE;
//���   if (flush_mem)
//���      load_dynamic_memory(DYNMEM_PARTIAL);
   EDMS_holistic_teleport(objs[PLAYER_OBJ].info.ph, &player_edms);

//KLC   end_wait();
   {
      extern void spoof_mouse_event();
//���what does this do???      spoof_mouse_event();
   }
   return OK;
}

/*KLC - no map conversion needed in Mac version.

extern bool init_done;
extern int loadcount;

#ifdef SUPPORT_9_TO_10
#pragma disable_message(202)
void convert_map_element_9_10(oMapElem *ome, MapElem *me, int x, int y)
{
   me->tiletype=ome->tiletype;
   if (ome->param && (tile_floors[ome->tiletype].flags&FRFLRFLG_USEPR))
   {  // cool, erik has params and flags which mean NOTHING... neat... now need to detect
      int tmp=(ome->flags&MAP_MIRROR_MASK)>>MAP_MIRROR_SHF;

      if ((tmp==MAP_MATCH)||(tmp==MAP_FFLAT))
      {
	      if (ome->ceil_height<ome->param)
	      {
	         me->ceil_height=0;
	         Warning(("Bad input Map Format at %d,%d, param %d and ceil %d, mirror %d\n",x,y,ome->param,ome->ceil_height,tmp));
	      }
	      else
			   me->ceil_height=ome->ceil_height-ome->param; // since this is negative
      }
      else me->ceil_height=ome->ceil_height;

      me->flr_height=ome->flr_height;
	   me->param=ome->param;
   }
   else
   {
	   me->flr_height=ome->flr_height;
	   me->ceil_height=ome->ceil_height;
      me->param=0;
   }
   me->templight=ome->templight;
   me->space=ome->space;
   me->flags=ome->flags;
   me->objRef=ome->objRef;
   // new stuff
   // wow... wild internal secret gnosis
   me->rinfo.sub_clip=SUBCLIP_OUT_OF_CONE;
   me->rinfo.clear=0;
   me->rinfo.rotflr=0;
   me->rinfo.rotceil=0;
   me->rinfo.flicker=0;
}
#pragma enable_message(202)
#endif

#pragma disable_message(202)
void convert_map_element_10_11(oMapElem *ome, MapElem *me, int x, int y)
{
   me_tiletype_set(me,ome_tiletype(ome));
   me_height_flr_set(me,ome_height_flr(ome));
   me_height_ceil_set(me,ome_height_ceil(ome));
   me_param_set(me,ome_param(ome));
   me_templight_flr_set(me,ome_templight_flr(ome));
   me_templight_ceil_set(me,ome_templight_ceil(ome));
   me_tmap_flr_set(me,ome_tmap_flr(ome));
   me_tmap_ceil_set(me,ome_tmap_ceil(ome));
   me_tmap_wall_set(me,ome_tmap_wall(ome));
   me_objref_set(me,ome_objref(ome));

   // flags nightmare
   me->flag1=ome->flags&0xff;    // low word is easy
   me->flag2=(ome_bits_friend(ome)|(ome_bits_deconst(ome)<<1)|
        (ome_bits_mirror(ome)<<2)|(ome_bits_peril(ome)<<4)|(ome_bits_music(ome)<<5));
   me->flag3=(ome_light_flr(ome)|(ome_bits_rend4(ome)<<4));
   me->flag4=(ome_light_ceil(ome)|(ome_bits_rend3(ome)<<4));   // seen is 0 at start

   // new stuff
   // wow... wild internal secret gnosis
   me->sub_clip=SUBCLIP_OUT_OF_CONE;
   me->clearsolid=0;
   me_rotflr_set(me,ome->rinfo.rotflr);
   me_rotceil_set(me,ome->rinfo.rotceil);
   me_hazard_bio_set(me,0);
   me_hazard_rad_set(me,0);
   me->flick_qclip=0;
}
#pragma enable_message(202)

void convert_cit_map(oFullMap *omp, FullMap **mp)
{
   int i, j, ibase;
   if ((*mp)!=NULL)
    { Free((*mp)->map); Free(*mp); }
   *mp=Malloc(sizeof(FullMap));
   LG_memcpy(*mp,omp,sizeof(oFullMap));
   (*mp)->map=Malloc(sizeof(MapElem)<<(omp->x_shft+omp->y_shft));
   for (ibase=0,i=0; i<(1<<omp->y_shft); i++, ibase+=(1<<(omp->x_shft)))
      for (j=0; j<(1<<omp->x_shft); j++)
         convert_map_element_10_11(omp->map+ibase+j,(*mp)->map+ibase+j,j,i);
}

#ifdef COMPRESS_OBJSPECS
extern bool HeaderObjSpecFree (ObjClass obclass, ObjSpecID id, ObjSpecHeader *head);
extern ObjSpecID HeaderObjSpecGrab (ObjClass obclass, ObjSpecHeader *head);
extern bool HeaderObjSpecCopy (ObjClass cls, ObjSpecID old, ObjSpecID new, ObjSpecHeader *head);
extern const ObjSpecHeader old_objSpecHeaders[NUM_CLASSES];

errtype fix_free_chain(char cl, short limit)
{
   ObjSpec *osp, *next_item, *next_next_item;
   short ss;
   char *data;
   bool cont = TRUE;

   ss = old_objSpecHeaders[cl].struct_size;
   data = old_objSpecHeaders[cl].data;
   osp = (ObjSpec *)data;  // Get the first elem

   // iterate through free chain, leapfrogging anything which is too high.
   while (cont)
   {
      if (osp->next >= limit)
      {
         next_item = (ObjSpec *)(data + (osp->next * ss));
         osp->next = next_item->next;
         if (osp->next == OBJ_SPEC_NULL)
            cont = FALSE;
         else
         {
            next_next_item = (ObjSpec *)(data + (osp->next * ss));
            next_next_item->prev = next_item->prev;
         }
      }
      else if (osp->next == OBJ_SPEC_NULL)
         cont = FALSE;
      else
         osp = (ObjSpec *)(data + (osp->next * ss));
   }
   return(OK);
}

bool one_compression_pass(char cl,short start)
{
   ObjSpec *osp;
   short ss;
   char *data;
   ObjID fugitive = OBJ_NULL;
   ObjSpec *p1;
   ObjSpecID new_objspec, old_specid;
   bool cont = TRUE;

   ss = old_objSpecHeaders[cl].struct_size;
   data = old_objSpecHeaders[cl].data;
   osp = (ObjSpec *)data;  // Get the first elem
   osp = (ObjSpec *)(data + (osp->id * ss)); // Extract secret gnosis
   while(cont)
   {
      if (objs[osp->id].specID >= start)
      {
         fugitive = osp->id;
         cont = FALSE;
      }
      else if (osp->next == OBJ_SPEC_NULL)
      {
         cont = FALSE;
      }
      else
         osp = (ObjSpec *)(data + (osp->next * ss));
   }
   if (fugitive == OBJ_NULL)
      return(FALSE);

   // Now that we have a guy who ought to be moved, lets move him
   // Really, I guess we have no particular reason to believe that our new location
   // is towards the low end of stuff, but hey, it usually is....
   new_objspec = HeaderObjSpecGrab(cl,&old_objSpecHeaders[cl]);
   p1 = (ObjSpec *)(data + new_objspec * ss);
   if (new_objspec == OBJ_SPEC_NULL)
   {
      Warning(("we have a problem here folks.  Can't complete HeaderObjSpecGrab!\n"));
      critical_error(22);
   }
   // copy in the data...
   HeaderObjSpecCopy(cl,objs[osp->id].specID, new_objspec, &old_objSpecHeaders[cl]);

   p1->id = osp->id;
   old_specid = objs[osp->id].specID;
   objs[osp->id].specID = new_objspec;
   HeaderObjSpecFree(cl,old_specid, &old_objSpecHeaders[cl]);
   fix_free_chain(cl,start);
   return(TRUE);
}

errtype compress_old_class(char cl)
{
   bool cont = TRUE;
   fix_free_chain(cl, objSpecHeaders[cl].size);
   while (cont)
      cont = one_compression_pass(cl,objSpecHeaders[cl].size);
   return(OK);
}
#endif

errtype expand_old_class(char cl, short new_start)
{
   ObjSpec *osp, *next_item;
   ObjSpecID osid;
   short ss;
   char *data;
   bool cont = TRUE;

   ss = objSpecHeaders[cl].struct_size;
   data = objSpecHeaders[cl].data;
   osp = (ObjSpec *)data;  // Get the first elem
   osid = 0;

   // iterate through free chain until we find the old end, then extend it a bit further
   while (cont)
   {
      if (osp->next == OBJ_SPEC_NULL)
      {
         osp->next = new_start;
         Warning(("setting osid %d to %d\n",osid,new_start));
         next_item = (ObjSpec *)(data + osp->next * ss);
         next_item->prev = osid;
         cont = FALSE;
      }
      else
      {
         osid = osp->next;
         osp = (ObjSpec *)(data + (osp->next * ss));
      }
   }
   return(OK);
}
*/

void load_level_data()
{
   extern errtype load_small_texturemaps();
	
//���KLC-removed from here	obj_load_art(FALSE);
	AdvanceProgress();
	load_small_texturemaps();
	AdvanceProgress();
}

void SwapLongBytes(void *pval4);
void SwapShortBytes(void *pval2);
#define MAKE4(c0,c1,c2,c3) ((((ulong)c0)<<24)|(((ulong)c1)<<16)|(((ulong)c2)<<8)|((ulong)c3))

//	---------------------------------------------------------
// ���� Put this in some more appropriate, global place.
void SwapLongBytes(void *pval4)
{
	long	*temp = (long *)pval4;
	*temp = MAKE4(*temp & 0xFF,
							  (*temp >> 8) & 0xFF,
							  (*temp >> 16) & 0xFF,
							  *temp >> 24);
}

void SwapShortBytes(void *pval2)
{
	short		*temp = (short *)pval2;
	*temp = ((*temp & 0xFF) << 8) | ((*temp >> 8) & 0xFF);
}


//---------------------------------------------------------------------------------
//  Loads in the map for a level, and all the other related resources (2+ MB worth).
//---------------------------------------------------------------------------------
//errtype load_current_map(char* fn, Id id_num, Datapath* dpath)
errtype load_current_map(Id id_num, FSSpec* spec)
{
	void rendedit_process_tilemap(FullMap* fmap, LGRect* r, bool newMap);
	extern errtype set_door_data(ObjID id);
	extern int physics_handle_max;
	extern ObjID physics_handle_id[MAX_OBJ];
	void cit_sleeper_callback(physics_handle caller);
	extern void edms_delete_go();
	extern void reload_motion_cursors(bool cyber);
	extern char old_bits;
	extern int compare_events(void* e1, void* e2);
   
	int 			i, idx = 0, fd, version;
	LGRect 		bounds;
	errtype 		retval = OK;
	bool 			make_player = FALSE;
	ObjLoc 		plr_loc;
	ObjID 		oid;
	char			*schedvec;			//KLC - don't need an array.  Only one in map.
//	State 			player_edms;
	curAMap 	saveAMaps[NUM_O_AMAP];
	uchar 		savedMaps;
	bool 			do_anims = FALSE;
   
//   _MARK_("load_current_map:Start");

//KLC - Mac cursor showing at this time	begin_wait();
	free_dynamic_memory(dynmem_mask);
	trigger_check = FALSE;
	if (PLAYER_OBJ != OBJ_NULL)
	{
		plr_loc = objs[PLAYER_OBJ].loc;
		obj_destroy(PLAYER_OBJ);
		make_player = TRUE;
	}
	AdvanceProgress();

	if (input_cursor_mode == INPUT_OBJECT_CURSOR)
	{
		pop_cursor_object();
	}

	// Open the saved-game (or archive) file.
	fd = ResOpenFile(spec);
	if (fd < 0)
	{
		//Warning(("Could not load map file %s (%s) , rv = %d!\n",dpath_fn,fn,retval));
		if (make_player)
			obj_create_player(&plr_loc);
		trigger_check=TRUE;
		load_dynamic_memory(dynmem_mask);
//KLC		end_wait();

		return ERR_FOPEN;
	}
	AdvanceProgress();

	if (ResInUse(SAVELOAD_VERIFICATION_ID))
	{
		int verify_cookie;
		ResExtract(SAVELOAD_VERIFICATION_ID, &verify_cookie);
		if ((verify_cookie != VERIFY_COOKIE_VALID) && (verify_cookie != OLD_VERIFY_COOKIE_VALID))
			critical_error(CRITERR_FILE|5);
	}
	AdvanceProgress();

	idx++;
/* KLC - don't need to worry with older versions any more

	// Check the version number of the map for this level.
	REF_READ(id_num, idx++, version);
	SwapLongBytes(&version);								// Mac
	if (version != MAP_VERSION_NUMBER)
	{
		//Warning(("Old Map Version Number (%d)!!  Current V. Num = %d\n",version,MAP_VERSION_NUMBER));
	
		if (version != OLD_MAP_VERSION_NUMBER)
		{
			retval = ERR_NOEFFECT;
			if (make_player)
				obj_create_player(&plr_loc);
			goto out;
		}
		else
		{
			//Warning(("Auto-converting map to v.%d from %d!\n",MAP_VERSION_NUMBER,version));
			// do auto_conversion
			convert_from = version;
		}
	}
*/
	
	idx++;
/* KLC - don't need to worry with older versions

	//  object version number!
	REF_READ(id_num, idx++, version);
	SwapLongBytes(&version);								// Mac
*/
	// Clear out old physics data and object data
	ObjsInit();
	physics_init();	

	// Read in the global fullmap (without disrupting schedule vec ptr)
	schedvec = global_fullmap->sched[0].queue.vec;		// KLC - Only one schedule, so just save it.
	
	// convert_from is the version we are coming from.
	// for now, this is only defined for coming from version 9
	{
		REF_READ(id_num, idx++, *global_fullmap);
				
		MAP_MAP = (MapElem *)static_map;
		ResExtract(id_num + (idx++), MAP_MAP);
		AdvanceProgress();
	}

	// Load schedules, performing some voodoo.  
	global_fullmap->sched[0].queue.vec = schedvec;			// KLC - Only one schedule, so restore it.
	global_fullmap->sched[0].queue.comp = compare_events;
	if (global_fullmap->sched[0].queue.fullness > 0)		// KLC - no need to read in vec if none there.
	{
		REF_READ(id_num, idx++, *global_fullmap->sched[0].queue.vec);
	}
	else
		idx++;

//KLC��� Big hack!  Force the schedule to growable.
global_fullmap->sched[0].queue.grow = TRUE;

	REF_READ(id_num, idx++, loved_textures);
/*
	for (i = 0; i < NUM_LOADED_TEXTURES; i++)
	{
		SwapShortBytes(&loved_textures[i]);
	}
*/
	map_set_default(global_fullmap);

/*���  Leave conversion from old objects out for now

   // Now set up for object conversion if necessary
   convert_from = -1;

   if (version != OBJECT_VERSION_NUMBER)
   {
      retval = ERR_NOEFFECT;
      Warning(("Old Object Version Number (%d)!!  Current V. Num = %d\n",version,OBJECT_VERSION_NUMBER));
      if (version >= 17)
      {
         Warning(("Auto-converting objects to v. %d from %d\n", OBJECT_VERSION_NUMBER,version));
         convert_from = version;
      }
      else
      {
         for (x=0; x<global_fullmap->x_size; x++)
         {
            for (y=0; y<global_fullmap->y_size; y++)
            {
               MAP_GET_XY(x,y)->objRef = 0;
            }
         }
         goto obj_out;
      }
   }
#ifdef SUPPORT_VERSION_26_OBJS
   if ((convert_from < 27) && (convert_from != -1))
   {
      extern old_Obj old_objs[NUM_OBJECTS];
      REF_READ(id_num,idx++,old_objs);
      for (x=0; x < NUM_OBJECTS; x++)
      {
         objs[x].active = old_objs[x].active;
         objs[x].obclass = old_objs[x].obclass;
         objs[x].subclass = old_objs[x].subclass;
         objs[x].specID = old_objs[x].specID;
         objs[x].ref = old_objs[x].ref;
         objs[x].next = old_objs[x].next;
         objs[x].prev = old_objs[x].prev;
         objs[x].loc = old_objs[x].loc;
         objs[x].info.ph = (char)(old_objs[x].info.ph);
         objs[x].info.type = old_objs[x].info.type;
         objs[x].info.current_hp = old_objs[x].info.current_hp;
         objs[x].info.make_info = old_objs[x].info.make_info;
         objs[x].info.current_frame = old_objs[x].info.current_frame;
         objs[x].info.time_remainder = old_objs[x].info.time_remainder;
         objs[x].info.inst_flags = old_objs[x].info.inst_flags;
      }
   }
   else
#endif
*/
	
	// Read in object information.  For the Mac version, copy from the resource's 27-byte structs, then
	// place it into an Obj struct (which is 28 bytes, due to alignment).  Swap bytes as needed.
/*	{
		uchar	*op = (uchar *)ResLock(id_num + idx);
		for(i = 0; i < NUM_OBJECTS; i++)
		{
			BlockMoveData(op, &objs[i], 3);
			BlockMoveData(op+3, &objs[i].specID, 24);
			op += 27;
			
			SwapShortBytes(&objs[i].specID);
			SwapShortBytes(&objs[i].ref);
			SwapShortBytes(&objs[i].next);
			SwapShortBytes(&objs[i].prev);
			SwapShortBytes(&objs[i].loc.x);
			SwapShortBytes(&objs[i].loc.y);
			SwapShortBytes(&objs[i].info.current_hp);
		}
		ResUnlock(id_num + idx);
		idx++;
	}*/
	REF_READ(id_num, idx++, objs);

	// Read in and convert the object refs.
	REF_READ(id_num, idx++, objRefs);
/*	for (i=0; i < NUM_REF_OBJECTS; i++)
	{
		SwapShortBytes(&objRefs[i].state.bin.sq.x);
		SwapShortBytes(&objRefs[i].state.bin.sq.y);
		SwapShortBytes(&objRefs[i].obj);
		SwapShortBytes(&objRefs[i].next);
		SwapShortBytes(&objRefs[i].nextref);
	} */
	
	// Read in and convert the gun objects.
	REF_READ(id_num, idx++, objGuns);
/*	for (i=0; i < NUM_OBJECTS_GUN; i++)
	{
		SwapShortBytes(&objGuns[i].id);
		SwapShortBytes(&objGuns[i].next);
		SwapShortBytes(&objGuns[i].prev);
	}*/
	
	// Read in and convert the ammo objects.
	REF_READ(id_num, idx++, objAmmos);
/*	for (i=0; i < NUM_OBJECTS_AMMO; i++)
	{
		SwapShortBytes(&objAmmos[i].id);
		SwapShortBytes(&objAmmos[i].next);
		SwapShortBytes(&objAmmos[i].prev);
	}*/
	
	// Read in and convert the physics objects.
	REF_READ(id_num, idx++, objPhysicss);
/*	for (i=0; i < NUM_OBJECTS_PHYSICS; i++)
	{
		SwapShortBytes(&objPhysicss[i].id);
		SwapShortBytes(&objPhysicss[i].next);
		SwapShortBytes(&objPhysicss[i].prev);
		SwapShortBytes(&objPhysicss[i].owner);
		SwapLongBytes(&objPhysicss[i].bullet_triple);
		SwapLongBytes(&objPhysicss[i].duration);
		SwapShortBytes(&objPhysicss[i].p1.x);
		SwapShortBytes(&objPhysicss[i].p1.y);
		SwapShortBytes(&objPhysicss[i].p2.x);
		SwapShortBytes(&objPhysicss[i].p2.y);
		SwapShortBytes(&objPhysicss[i].p3.x);
		SwapShortBytes(&objPhysicss[i].p3.y);
	}*/
	
	// Read in and convert the grenades.
	REF_READ(id_num, idx++, objGrenades);
/*	for (i=0; i < NUM_OBJECTS_GRENADE; i++)
	{
		SwapShortBytes(&objGrenades[i].id);
		SwapShortBytes(&objGrenades[i].next);
		SwapShortBytes(&objGrenades[i].prev);
		SwapShortBytes(&objGrenades[i].flags);
		SwapShortBytes(&objGrenades[i].timestamp);
	}*/
	
	// Read in and convert the drugs.
	REF_READ(id_num, idx++, objDrugs);
/*	for (i=0; i < NUM_OBJECTS_DRUG; i++)
	{
		SwapShortBytes(&objDrugs[i].id);
		SwapShortBytes(&objDrugs[i].next);
		SwapShortBytes(&objDrugs[i].prev);
	}*/
	
	// Read in and convert the hardwares.  Resource is array of 7-byte structs.  Ours are 8.
/*	{
		uchar	*hp = (uchar *)ResLock(id_num + idx);
		for (i=0; i < NUM_OBJECTS_HARDWARE; i++)
		{
			BlockMoveData(hp, &objHardwares[i], 7);
			hp += 7;
			SwapShortBytes(&objHardwares[i].id);
			SwapShortBytes(&objHardwares[i].next);
			SwapShortBytes(&objHardwares[i].prev);
		}
		ResUnlock(id_num + idx);
		idx++;
	}*/
	REF_READ(id_num, idx++, objHardwares);
	
	// Read in and convert the softwares.  Resource is array of 9-byte structs.  Ours are 10.
/*	{
		uchar	*sp = (uchar *)ResLock(id_num + idx);
		for (i=0; i < NUM_OBJECTS_SOFTWARE; i++)
		{
			BlockMoveData(sp, &objSoftwares[i], 7);
			BlockMoveData(sp+7, &objSoftwares[i].data_munge, 2);
			sp += 9;
			SwapShortBytes(&objSoftwares[i].id);
			SwapShortBytes(&objSoftwares[i].next);
			SwapShortBytes(&objSoftwares[i].prev);
			SwapShortBytes(&objSoftwares[i].data_munge);
		}
		ResUnlock(id_num + idx);
		idx++;
	}*/
	REF_READ(id_num,idx++,objSoftwares);	

	// Read in and convert the big stuff.
	REF_READ(id_num, idx++, objBigstuffs);
/*	for (i=0; i < NUM_OBJECTS_BIGSTUFF; i++)
	{
		SwapShortBytes(&objBigstuffs[i].id);
		SwapShortBytes(&objBigstuffs[i].next);
		SwapShortBytes(&objBigstuffs[i].prev);
		SwapShortBytes(&objBigstuffs[i].cosmetic_value);
		SwapLongBytes(&objBigstuffs[i].data1);
		SwapLongBytes(&objBigstuffs[i].data2);
	}*/

	// Read in and convert the small stuff.
	REF_READ(id_num, idx++, objSmallstuffs);
/*	for (i=0; i < NUM_OBJECTS_SMALLSTUFF; i++)
	{
		SwapShortBytes(&objSmallstuffs[i].id);
		SwapShortBytes(&objSmallstuffs[i].next);
		SwapShortBytes(&objSmallstuffs[i].prev);
		SwapShortBytes(&objSmallstuffs[i].cosmetic_value);
		SwapLongBytes(&objSmallstuffs[i].data1);
		SwapLongBytes(&objSmallstuffs[i].data2);
	}*/

	// Read in and convert the fixtures.
	REF_READ(id_num, idx++, objFixtures);
/*	for (i=0; i < NUM_OBJECTS_FIXTURE; i++)
	{
		SwapShortBytes(&objFixtures[i].id);
		SwapShortBytes(&objFixtures[i].next);
		SwapShortBytes(&objFixtures[i].prev);
		SwapLongBytes(&objFixtures[i].comparator);
		SwapLongBytes(&objFixtures[i].p1);
		SwapLongBytes(&objFixtures[i].p2);
		SwapLongBytes(&objFixtures[i].p3);
		SwapLongBytes(&objFixtures[i].p4);
		SwapShortBytes(&objFixtures[i].access_level);
	}*/

	// Read in and convert the doors.
	REF_READ(id_num, idx++, objDoors);
/*	for (i=0; i < NUM_OBJECTS_DOOR; i++)
	{
		SwapShortBytes(&objDoors[i].id);
		SwapShortBytes(&objDoors[i].next);
		SwapShortBytes(&objDoors[i].prev);
		SwapShortBytes(&objDoors[i].locked);
		SwapShortBytes(&objDoors[i].other_half);
	}*/

	// Read in and convert the animating objects.
	REF_READ(id_num, idx++, objAnimatings);
/*	for (i=0; i < NUM_OBJECTS_ANIMATING; i++)
	{
		SwapShortBytes(&objAnimatings[i].id);
		SwapShortBytes(&objAnimatings[i].next);
		SwapShortBytes(&objAnimatings[i].prev);
		SwapShortBytes(&objAnimatings[i].owner);
	}*/

	// Read in and convert the traps.
	REF_READ(id_num, idx++, objTraps);
/*	for (i=0; i < NUM_OBJECTS_TRAP; i++)
	{
		SwapShortBytes(&objTraps[i].id);
		SwapShortBytes(&objTraps[i].next);
		SwapShortBytes(&objTraps[i].prev);
		SwapLongBytes(&objTraps[i].comparator);
		SwapLongBytes(&objTraps[i].p1);
		SwapLongBytes(&objTraps[i].p2);
		SwapLongBytes(&objTraps[i].p3);
		SwapLongBytes(&objTraps[i].p4);
	}	*/
	
	// Read in and convert the containers.  Resource is array of 21-byte structs.  Ours are 22.
/*	{
		uchar	*sp = (uchar *)ResLock(id_num + idx);
		for (i=0; i < NUM_OBJECTS_CONTAINER; i++)
		{
			BlockMoveData(sp, &objContainers[i], 17);
			BlockMoveData(sp+17, &objContainers[i].data1, 4);
			sp += 21;
			SwapShortBytes(&objContainers[i].id);
			SwapShortBytes(&objContainers[i].next);
			SwapShortBytes(&objContainers[i].prev);
			SwapLongBytes(&objContainers[i].contents1);
			SwapLongBytes(&objContainers[i].contents2);
			SwapLongBytes(&objContainers[i].data1);
		}
		ResUnlock(id_num + idx);
		idx++;
	}*/
	REF_READ(id_num,idx++,objContainers);

	// Read in and convert the critters.
	REF_READ(id_num, idx++, objCritters);
/*	for (i=0; i < NUM_OBJECTS_CRITTER; i++)
	{
		SwapShortBytes(&objCritters[i].id);
		SwapShortBytes(&objCritters[i].next);
		SwapShortBytes(&objCritters[i].prev);
		SwapLongBytes(&objCritters[i].des_heading);
		SwapLongBytes(&objCritters[i].des_speed);
		SwapLongBytes(&objCritters[i].urgency);
		SwapShortBytes(&objCritters[i].wait_frames);
		SwapShortBytes(&objCritters[i].flags);
		SwapLongBytes(&objCritters[i].attack_count);
		SwapShortBytes(&objCritters[i].loot1);
		SwapShortBytes(&objCritters[i].loot2);
		SwapLongBytes(&objCritters[i].sidestep);
	}	*/

	//-------------------------------
	//  Read in the default objects.
	//-------------------------------
	
	// Convert the default gun.
	REF_READ(id_num, idx++, default_gun);
/*	SwapShortBytes(&default_gun.id);
	SwapShortBytes(&default_gun.next);
	SwapShortBytes(&default_gun.prev);*/

	// Convert the default ammo.
	REF_READ(id_num, idx++, default_ammo);
/*	SwapShortBytes(&default_ammo.id);
	SwapShortBytes(&default_ammo.next);
	SwapShortBytes(&default_ammo.prev);*/
	
	// Read in and convert the physics objects.
	REF_READ(id_num, idx++, default_physics);
/*	SwapShortBytes(&default_physics.id);
	SwapShortBytes(&default_physics.next);
	SwapShortBytes(&default_physics.prev);
	SwapShortBytes(&default_physics.owner);
	SwapLongBytes(&default_physics.bullet_triple);
	SwapLongBytes(&default_physics.duration);
	SwapShortBytes(&default_physics.p1.x);
	SwapShortBytes(&default_physics.p1.y);
	SwapShortBytes(&default_physics.p2.x);
	SwapShortBytes(&default_physics.p2.y);
	SwapShortBytes(&default_physics.p3.x);
	SwapShortBytes(&default_physics.p3.y);*/
	
	// Convert the default grenade.
	REF_READ(id_num, idx++, default_grenade);
/*	SwapShortBytes(&default_grenade.id);
	SwapShortBytes(&default_grenade.next);
	SwapShortBytes(&default_grenade.prev);
	SwapShortBytes(&default_grenade.flags);
	SwapShortBytes(&default_grenade.timestamp);*/
	
	// Convert the default drug.
	REF_READ(id_num, idx++, default_drug);
/*	SwapShortBytes(&default_drug.id);
	SwapShortBytes(&default_drug.next);
	SwapShortBytes(&default_drug.prev);*/
	
	// Convert the default hardware.  Resource is array of 7-byte structs.  Ours is 8.
/*	{
		uchar	*hp = (uchar *)ResLock(id_num + idx);
		BlockMoveData(hp, &default_hardware, 7);
		SwapShortBytes(&default_hardware.id);
		SwapShortBytes(&default_hardware.next);
		SwapShortBytes(&default_hardware.prev);
		ResUnlock(id_num + idx);
		idx++;
	}*/
	REF_READ(id_num,idx++,default_hardware);

	// Convert the default software.  Resource is array of 9-byte structs.  Ours is 10.
/*	{
		uchar	*sp = (uchar *)ResLock(id_num + idx);
		BlockMoveData(sp, &default_software, 7);
		BlockMoveData(sp+7, &default_software.data_munge, 2);
		SwapShortBytes(&default_software.id);
		SwapShortBytes(&default_software.next);
		SwapShortBytes(&default_software.prev);
		SwapShortBytes(&default_software.data_munge);
		ResUnlock(id_num + idx);
		idx++;
	}*/
	REF_READ(id_num, idx++, default_software);

	// Convert the default big stuff.
	REF_READ(id_num, idx++, default_bigstuff);
/*	SwapShortBytes(&default_bigstuff.id);
	SwapShortBytes(&default_bigstuff.next);
	SwapShortBytes(&default_bigstuff.prev);
	SwapShortBytes(&default_bigstuff.cosmetic_value);
	SwapLongBytes(&default_bigstuff.data1);
	SwapLongBytes(&default_bigstuff.data2);*/

	// Convert the default small stuff.
	REF_READ(id_num, idx++, default_smallstuff);
/*	SwapShortBytes(&default_smallstuff.id);
	SwapShortBytes(&default_smallstuff.next);
	SwapShortBytes(&default_smallstuff.prev);
	SwapShortBytes(&default_smallstuff.cosmetic_value);
	SwapLongBytes(&default_smallstuff.data1);
	SwapLongBytes(&default_smallstuff.data2);*/

	// Convert the fixture.
	REF_READ(id_num, idx++, default_fixture);
/*	SwapShortBytes(&default_fixture.id);
	SwapShortBytes(&default_fixture.next);
	SwapShortBytes(&default_fixture.prev);
	SwapLongBytes(&default_fixture.comparator);
	SwapLongBytes(&default_fixture.p1);
	SwapLongBytes(&default_fixture.p2);
	SwapLongBytes(&default_fixture.p3);
	SwapLongBytes(&default_fixture.p4);
	SwapShortBytes(&default_fixture.access_level);*/

	// Convert the default door.
	REF_READ(id_num, idx++, default_door);
/*	SwapShortBytes(&default_door.id);
	SwapShortBytes(&default_door.next);
	SwapShortBytes(&default_door.prev);
	SwapShortBytes(&default_door.locked);
	SwapShortBytes(&default_door.other_half);*/

	// Convert the default animating object.
	REF_READ(id_num, idx++, default_animating);
/*	SwapShortBytes(&default_animating.id);
	SwapShortBytes(&default_animating.next);
	SwapShortBytes(&default_animating.prev);
	SwapShortBytes(&default_animating.owner);*/

	// Read in and convert the traps.
	REF_READ(id_num, idx++, default_trap);
/*	SwapShortBytes(&default_trap.id);
	SwapShortBytes(&default_trap.next);
	SwapShortBytes(&default_trap.prev);
	SwapLongBytes(&default_trap.comparator);
	SwapLongBytes(&default_trap.p1);
	SwapLongBytes(&default_trap.p2);
	SwapLongBytes(&default_trap.p3);
	SwapLongBytes(&default_trap.p4);*/

/*	// Convert the default container.  Resource is a 21-byte struct.  Ours is 22.
	{
		uchar	*sp = (uchar *)ResLock(id_num + idx);
		BlockMoveData(sp, &default_container, 17);
		BlockMoveData(sp+17, &default_container.data1, 4);
		SwapShortBytes(&default_container.id);
		SwapShortBytes(&default_container.next);
		SwapShortBytes(&default_container.prev);
		SwapLongBytes(&default_container.contents1);
		SwapLongBytes(&default_container.contents2);
		SwapLongBytes(&default_container.data1);
		ResUnlock(id_num + idx);
		idx++;
	}*/
	REF_READ(id_num,idx++,default_container);

	// Convert the default critter.
	REF_READ(id_num, idx++, default_critter);
/*	SwapShortBytes(&default_critter.id);
	SwapShortBytes(&default_critter.next);
	SwapShortBytes(&default_critter.prev);
	SwapLongBytes(&default_critter.des_heading);
	SwapLongBytes(&default_critter.des_speed);
	SwapLongBytes(&default_critter.urgency);
	SwapShortBytes(&default_critter.wait_frames);
	SwapShortBytes(&default_critter.flags);
	SwapLongBytes(&default_critter.attack_count);
	SwapShortBytes(&default_critter.loot1);
	SwapShortBytes(&default_critter.loot2);
	SwapLongBytes(&default_critter.sidestep);*/

	idx++;
/* KLC - don't need this any more.

	REF_READ(id_num, idx++, version);
	SwapLongBytes(&version);								// Mac
	if (version != MISC_SAVELOAD_VERSION_NUMBER && version < 5)
	{
		retval = ERR_NOEFFECT;
 		anim_counter = 0;
		goto obj_out;
	}
*/
	idx++; 	// skip over resource where flickers once lived

	// Convert the anim textures.  Resource is a 7-byte struct.  Ours is 8.
/*	{
		uchar	*ap = (uchar *)ResLock(id_num + idx);
		for (i=0; i < NUM_ANIM_TEXTURE_GROUPS; i++)
		{
			BlockMoveData(ap, &animtextures[i], 7);
			ap += 7;
			SwapShortBytes(&animtextures[i].anim_speed);
			SwapShortBytes(&animtextures[i].time_remainder);
		}
		ResUnlock(id_num + idx);
		idx++;
	}*/
	REF_READ(id_num, idx++, animtextures);

	// Read in and convert the hack camera objects.
	REF_READ( id_num, idx++, hack_cam_objs);
	REF_READ( id_num, idx++, hack_cam_surrogates);
/*	for (i = 0; i < NUM_HACK_CAMERAS; i++)
	{
		SwapShortBytes(&hack_cam_objs[i]);
		SwapShortBytes(&hack_cam_surrogates[i]);
	}*/

	savedMaps = 0;
	for(i=0; i < NUM_O_AMAP; i++)
	{
		if(oAMap(i)->init)
		{
			savedMaps |= (1<<i);
			amap_settings_copy(oAMap(i), &saveAMaps[i]);
			amap_invalidate(i);
		}
	}
	
	// Get other level data at next id
	REF_READ( id_num, idx++, level_gamedata);
/*	{
		uchar *ldp = (uchar *)ResLock(id_num + idx);
		LG_memset(&level_gamedata, 0, sizeof(LevelData));
		BlockMoveData(ldp, &level_gamedata, 9);
		BlockMoveData(ldp+9, &level_gamedata.exit_time, 4);
		SwapShortBytes(&level_gamedata.size);
		SwapLongBytes(&level_gamedata.exit_time);
		for (i = 0, ldp += 13; i < NUM_O_AMAP; i++, ldp += 27)
		{
			BlockMoveData(ldp, &level_gamedata.auto_maps[i], 25);
			BlockMoveData(ldp+25, &level_gamedata.auto_maps[i].sensor_rad, 2);
			SwapLongBytes(&level_gamedata.auto_maps[i].xf);
			SwapLongBytes(&level_gamedata.auto_maps[i].yf);
			SwapShortBytes(&level_gamedata.auto_maps[i].lw);
			SwapShortBytes(&level_gamedata.auto_maps[i].lh);
			SwapShortBytes(&level_gamedata.auto_maps[i].obj_to_follow);
			SwapShortBytes(&level_gamedata.auto_maps[i].sensor_obj);
			SwapShortBytes(&level_gamedata.auto_maps[i].note_obj);
			SwapShortBytes(&level_gamedata.auto_maps[i].flags);
			SwapShortBytes(&level_gamedata.auto_maps[i].avail_flags);
			SwapShortBytes(&level_gamedata.auto_maps[i].sensor_rad);
		}
		ResUnlock(id_num + idx);
		idx++;
	}*/

#ifdef SAVE_AUTOMAP_STRINGS
	{
		int	amap_magic_num;
		char	*cp = amap_str_reref(0);
		ResExtract(id_num + idx++, cp);
//		REF_READ(id_num, idx++, amap_str_reref(0));		old way
		REF_READ(id_num, idx++, amap_magic_num);
//		SwapLongBytes(&amap_magic_num);
		amap_str_startup(amap_magic_num);
		AdvanceProgress();
	}
#endif

	idx++;		// Doesn't appear that this does anything
/*
	REF_READ(id_num, idx++, player_edms);
	SwapLongBytes(&player_edms.X);
	SwapLongBytes(&player_edms.Y);
	SwapLongBytes(&player_edms.Z);
	SwapLongBytes(&player_edms.alpha);
	SwapLongBytes(&player_edms.beta);
	SwapLongBytes(&player_edms.gamma);
	SwapLongBytes(&player_edms.X_dot);
	SwapLongBytes(&player_edms.Y_dot);
	SwapLongBytes(&player_edms.Z_dot);
	SwapLongBytes(&player_edms.alpha_dot);
	SwapLongBytes(&player_edms.beta_dot);
	SwapLongBytes(&player_edms.gamma_dot);
*/

	REF_READ(id_num,idx++,paths);
/*	for(i=0; i < MAX_PATHS; i++)
	{
		SwapShortBytes(&paths[i].source.x);
		SwapShortBytes(&paths[i].source.y);
		SwapShortBytes(&paths[i].dest.x);
		SwapShortBytes(&paths[i].dest.y);
	}*/
	REF_READ(id_num,idx++,used_paths);
//	SwapShortBytes(&used_paths);

/*	uchar	*ap = (uchar *)ResLock(id_num + idx);
	for (i=0; i < MAX_ANIMLIST_SIZE; i++)
	{
		BlockMoveData(ap, &animlist[i].id, 2);
		BlockMoveData(ap+2, &animlist[i].flags, 1);
		BlockMoveData(ap+3, &animlist[i].cbtype, 12);
		ap += 15;
		SwapShortBytes(&animlist[i].id);
		SwapShortBytes(&animlist[i].cbtype);
		SwapLongBytes(&animlist[i].callback);
		SwapShortBytes(&animlist[i].speed);
	}
	ResUnlock(id_num + idx);
	idx++;*/
	REF_READ(id_num, idx++, animlist);    

	REF_READ(id_num, idx++, anim_counter);
//	SwapShortBytes(&anim_counter);

	REF_READ(id_num, idx++, h_sems);		// Unbelievably, no conversion needed.

obj_out:
	bounds.ul.x = bounds.ul.y = 0;
	bounds.lr.x = global_fullmap->x_size;
	bounds.lr.y = global_fullmap->y_size;

   rendedit_process_tilemap(global_fullmap, &bounds, TRUE);

   for (i=0;i<MAX_OBJ;i++)
      physics_handle_id[i]=OBJ_NULL;
   physics_handle_max=-1;

   if (anim_counter == 0)
      do_anims = TRUE;

	FORALLOBJS(oid)
	{
		switch (objs[oid].obclass)
		{
			case CLASS_DOOR:
				set_door_data(oid);
				break;
		}
		
		if (do_anims && ANIM_3D(ObjProps[OPNUM(oid)].bitmap_3d))
		{
			extern errtype obj_screen_animate(ObjID id);
			switch(TRIP2CL(ID2TRIP(oid)))
			{
				case CLASS_BIGSTUFF:
				case CLASS_SMALLSTUFF:
					obj_screen_animate(oid);
					break;
				default:
					add_obj_to_animlist(oid, REPEAT_3D(ObjProps[OPNUM(oid)].bitmap_3d),FALSE,FALSE,0,NULL,NULL,0);
					break;
			}
		}

		objs[oid].info.ph = -1;
		if (objs[oid].loc.x != 0xFFFF)
			obj_move_to(oid, &objs[oid].loc,TRUE);
		
		// sleep the object (this may become "settle" the object)
		if (objs[oid].info.ph != -1)
		{
			cit_sleeper_callback(objs[oid].info.ph);
			edms_delete_go();
		}
	}
	AdvanceProgress();

   // DO NOT call this from here.  We haven't necessarily yet set
   // player_struct.level, which means the wrong shodometer quest
   // variable gets set!!!
   //
   // compute_shodometer_value(FALSE);

   if (make_player)
   {
      extern int score_playing;
      obj_create_player(&plr_loc);
      if (version > 9)
      {
         // Regenerate physics state from player_state here
      }
//���      if (music_on && (score_playing != ELEVATOR_ZONE))
//���        load_score_for_location(PLAYER_BIN_X,PLAYER_BIN_Y);
   }

out:
	ResCloseFile(fd);

	reset_pathfinding();
	old_bits = -1;

	trigger_check = TRUE;

	load_dynamic_memory(dynmem_mask);
	load_level_data();
	
	for(i=0;i<NUM_O_AMAP;i++)
	{
		if(!oAMap(i)->init && (savedMaps & (1<< i)))
		{
			automap_init(player_struct.hardwarez[CPTRIP(NAV_HARD_TRIPLE)],i);
			amap_settings_copy(&saveAMaps[i],oAMap(i));
		}
	}
   reload_motion_cursors(global_fullmap->cyber);
   
//KLC   physics_warmup();

//KLC   end_wait();
/*���   {
      extern void spoof_mouse_event();
      spoof_mouse_event();
   }
   _MARK_("load_current_map:End");
*/
	return retval;
}

