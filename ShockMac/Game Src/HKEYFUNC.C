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
 * $Source: r:/prj/cit/src/RCS/hkeyfunc.c $
 * $Revision: 1.173 $
 * $Author: dc $
 * $Date: 1994/11/18 00:24:50 $
 */

#define __HKEYFUNC_SRC

#include <string.h>
#include <stdlib.h>

#include "Shock.h"
#include "ShockBitmap.h"
#include "InitMac.h"
#include "Prefs.h"

#include "frtypes.h"
#include "FrUtils.h"
#include "fullscrn.h"
#include "hkeyfunc.h"
#include "loops.h"     // this is needed for demoview update
#include "mfdint.h"
#include "mfdext.h"
#include "MacTune.h"
#include "musicai.h"
#include "screen.h"
#include "status.h"
#include "tools.h"
#include "wares.h"

//--------------
//  PROTOTYPES
//--------------
int select_object_by_class(int obclass, int num, ubyte* quantlist);


#ifdef NOT_YET //���

#include <frprotox.h>
#include <wrapper.h>
#include <saveload.h>
#include <player.h>
#include <physics.h>
#include <fauxrint.h>
#include <frcamera.h>
#include <version.h>
#include <objsim.h>
#include <gamewrap.h>
#include <damage.h>
#include <cutscene.h>
#include <cursors.h>
#include <render.h>
#include <status.h>
#include <scrntext.h>
#include <keydefs.h>
#include <gamescr.h>
#include <gamestrn.h>
#include <shodan.h>
#include <cybmem.h>
#include <gr2ss.h>

#include <cybstrng.h>
#include <memstat.h>

#define SIGNATURE "giSoink"
#define CFG_HKEY_GO "cyberia"
bool yes_3d = TRUE;
extern bool properties_changed;

#ifdef PLAYTEST
#pragma disable_message(202)
bool maim_player(short keycode, ulong context, void* data)
{
   player_struct.hit_points = 5;
   return TRUE;
}
#pragma enable_message(202)

#pragma disable_message(202)
bool salt_the_player(short keycode, ulong context, void* data)
{
   if (config_get_raw(CFG_HKEY_GO,NULL,0))
   {
      player_struct.hit_points = 255;
      player_struct.cspace_hp = 255;
      memset(player_struct.hit_points_lost,0, NUM_DAMAGE_TYPES);
      player_struct.energy = 255;
      player_struct.fatigue= 0;
      player_struct.experience = TRUE;
      chg_set_flg(VITALS_UPDATE);
   }
   else
   {
      message_info("Winners don't use hotkeys");
      damage_player(25, EXPLOSION_FLAG, 0);
   }
   return TRUE;
}

bool automap_seen(short keycode, ulong context, void* data)
{
   ushort x,y;
   MapElem* pme;

   if (config_get_raw(CFG_HKEY_GO,NULL,0))
   {
      for(x=0;x<MAP_XSIZE;x++) {
         for(y=0;y<MAP_YSIZE;y++) {
            me_bits_seen_set(MAP_GET_XY(x,y));
         }
      }
   }
   return TRUE;
}

extern errtype give_player_loot(Player *pplr);

bool give_player_hotkey(short keycode, ulong context, void *data)
{
   if (config_get_raw(CFG_HKEY_GO,NULL,0))
   {
      give_player_loot(&player_struct);
      chg_set_flg(INVENTORY_UPDATE);
      mfd_force_update();
   }
   return TRUE;
}

#pragma enable_message(202)
#endif

#ifdef PLAYTEST
bool new_cone_clip = TRUE;

#pragma disable_message(202)
bool change_clipper(short keycode, ulong context, void *data)
{
   extern errtype render_run(void);
   new_cone_clip = !new_cone_clip;
   if (new_cone_clip)
      mprintf("NEW CONE CLIP\n");
   else
      mprintf("OLD CONE CLIP\n");
   render_run();
   return TRUE;
}
#pragma enable_message(202)
#endif

#pragma disable_message(202)
bool quit_key_func(short keycode, ulong context, void* data)
{
#ifndef GAMEONLY
   extern bool possible_change;
#endif

#ifndef GAMEONLY
   if ((!possible_change) || (confirm_box("Level changed without save. Really exit?")))
#endif
   {
      _new_mode = -1;
      chg_set_flg(GL_CHG_LOOP);
   }
   return TRUE;
}

extern void loopmode_exit(short),loopmode_enter(short);

bool keyhelp_hotkey_func(short keycode, ulong context, void* data)
{
   void* keyhelp_txtscrn;
   int fake_inp=0;
   extern errtype update_state(bool time_passes);


   loopmode_exit(_current_loop);

   uiHideMouse(NULL);
   uiFlush();

#ifdef RES_keyhelp
   keyhelp_txtscrn=scrntext_init(RES_smallTechFont,0x4C,RES_keyhelp);
   while(scrntext_advance(keyhelp_txtscrn,fake_inp)) {

      fake_inp=0;

      // translate keys to escape
      if(uiCheckInput()) {
         fake_inp=KEY_PGDN;
      }
      tight_loop(FALSE);
   }

   scrntext_free(keyhelp_txtscrn);
#else
   Warning(("Ce n'est pas RES_keyhelp\n"));
#endif

   uiShowMouse(NULL);

   update_state(FALSE);
   loopmode_enter(_current_loop);

   return TRUE;
}


bool really_quit_key_func(short keycode, ulong context, void* data)
{
   _new_mode = -1;
   chg_set_flg(GL_CHG_LOOP);
   return TRUE;
}

bool toggle_bool_func(short keycode, ulong context, bool* tgl)
{
   *tgl = !*tgl;
   return TRUE;
}

#endif //��� NOT_YET

extern Boolean	DoubleSize;

bool change_mode_func(short , ulong , void* data)
{
   int newm = (int)data;
   
   if ((newm == AUTOMAP_LOOP) && ((!player_struct.hardwarez[HARDWARE_AUTOMAP]) || (global_fullmap->cyber)))
      return TRUE;
   _new_mode = newm;
   chg_set_flg(GL_CHG_LOOP);
   return TRUE;
}

#ifdef NOT_YET //���

#ifdef HANDART_ADJUST

short    hdx=0, hdy=0;
ubyte    hcount = 0;

bool move_handart(short keycode, ulong context, void *data)
{
   short amt=1;
   ubyte foo = (ubyte) data;
   short *dir;

   if (foo & 0x10)
   {
      hdx=hdy=0;
      return TRUE;
   }

   if (foo & 0x08)
      amt = 10;
   foo &=0x7F;   
   dir = (foo&0x02) ? &hdx : &hdy;

   if (foo & 0x01)
      (*dir)+=amt;
   else
      (*dir)-=amt;

   return TRUE;
}

bool adv_handart(short keycode, ulong context, void* data)
{
   hcount = (hcount+1)%5;
   return TRUE;
}

#endif // HANDART_ADJUST

bool toggle_view_func(short keycode, ulong context, void* data)
{
   extern bool full_game_3d;
   return(change_mode_func(keycode,context, (full_game_3d) ? (void *)GAME_LOOP : (void *)FULLSCREEN_LOOP));
}


#endif //��� NOT_YET

void start_music(void)
{
	extern errtype mlimbs_AI_init(void);

//���   if (music_card)
//���   {
	if (MacTuneInit() == 0)
	{
	    	music_on = TRUE;
		mlimbs_on = TRUE;
		mlimbs_AI_init();
		load_score_for_location(PLAYER_BIN_X, PLAYER_BIN_Y);
		MacTuneStartCurrentTheme();
	}
	else
	{
		gShockPrefs.soBackMusic = FALSE;
		SavePrefs(kPrefsResID);
	}
//���   }
}

void stop_music(void)
{
	extern bool mlimbs_on;
	
	MacTuneShutdown();
	music_on = FALSE;
	mlimbs_on = FALSE;
	mlimbs_peril = DEFAULT_PERIL_MIN;
	mlimbs_monster = NO_MONSTER;
}

bool toggle_music_func(short, ulong, void*)
{
	if (music_on)
	{
		message_info("Music off.");
		stop_music();
	}
	else
	{
		start_music();
		if (mlimbs_status == 1)
			message_info("Music on.");
		else
			message_info("Not enough memory to start music.");
	}
	
	gShockPrefs.soBackMusic = music_on;
	SavePrefs(kPrefsResID);
	
	return (FALSE);
}


bool arm_grenade_hotkey(short , ulong , void* )
{
   extern bool show_all_actives;
   extern void super_drop_func(int dispnum, int row);
   extern void mfd_force_update(void);
   extern errtype inventory_draw(void);
   extern short inv_last_page;
   extern bool activate_grenade_on_cursor(void);
   int i, row, act;

   if(!show_all_actives) {
      show_all_actives=TRUE;
      inv_last_page=-1;
      chg_set_flg(INVENTORY_UPDATE);
      mfd_force_update();
      return TRUE;
   }
   if(activate_grenade_on_cursor())
      return TRUE;
   act=player_struct.actives[ACTIVE_GRENADE];
   for(i=row=0;i<act;i++) 
      if(player_struct.grenades[i]) row++;
   super_drop_func(ACTIVE_GRENADE,row);
   return TRUE;
}

int select_object_by_class(int obclass, int num, ubyte* quantlist)
{
   extern bool show_all_actives;
   extern short inv_last_page;
   int act=player_struct.actives[obclass];
   int newobj=act;

   inv_last_page=-1;
   chg_set_flg(INVENTORY_UPDATE);
   if(!show_all_actives) {
      show_all_actives=TRUE;
      return -1;
   }
   do {
      newobj=(newobj+1)%num;
   } while(quantlist[newobj]==0 && newobj!=act);

   player_struct.actives[obclass]=newobj;
   return newobj;
}

bool select_grenade_hotkey(short , ulong , void* )
{
   int newobj;

   newobj=select_object_by_class(ACTIVE_GRENADE,NUM_GRENADES,player_struct.grenades);
   set_inventory_mfd(MFD_INV_GRENADE,newobj,TRUE);
   return TRUE;
}

bool select_drug_hotkey(short , ulong , void* )
{
   int newobj;

   newobj=select_object_by_class(ACTIVE_DRUG,NUM_DRUGS,player_struct.drugs);
   set_inventory_mfd(MFD_INV_DRUG,newobj,TRUE);
   return TRUE;
}

bool use_drug_hotkey(short , ulong , void* )
{
   extern bool show_all_actives;
   extern void super_use_func(int dispnum, int row);
   extern void mfd_force_update(void);
   extern errtype inventory_draw(void);
   extern short inv_last_page;
   int i, row, act;

   if(!show_all_actives) {
      show_all_actives=TRUE;
      inv_last_page=-1; // to force redraw
      chg_set_flg(INVENTORY_UPDATE);
      return TRUE;
   }
   act=player_struct.actives[ACTIVE_DRUG];
   for(i=row=0;i<act;i++) 
      if(player_struct.drugs[i]) row++;
   super_use_func(ACTIVE_DRUG,row);
   return TRUE;
}

bool clear_fullscreen_func(short , ulong , void* )
{
   extern char last_message[128];
   extern MFD mfd[2];
   
   full_lower_region(&mfd[MFD_RIGHT].reg2);
   full_lower_region(&mfd[MFD_LEFT].reg2);
   full_lower_region(inventory_region_full); 
   full_visible = 0;
   strcpy(last_message, "");
   chg_unset_sta(FULLSCREEN_UPDATE);
   return(FALSE);
}

#ifdef NOT_YET  //KLC

#ifndef GAMEONLY
bool zoom_func(short keycode, ulong context, void* data)
{
   ushort zoom;
   
   TileMapGetZoom(NULL,&zoom);
   if ((int)data == ZOOM_IN)
   {
     zoom++; 
   }
   else zoom = (zoom == 1) ? 1 : zoom-1;
   TileMapSetZoom(NULL,zoom);
   return TRUE;
}

bool do_popup_textmenu(short keycode, ulong context, void* g)
{
   extern errtype textmenu_popup(Gadget* parent);

   textmenu_popup((Gadget*)g);
   return TRUE;
}
#endif

#define MAP_FNAME "map.dat"

#ifdef PLAYTEST
void edit_load_func(char* fn, uchar source, short level_num)
{
   char buf[256], *buf2;
   errtype retval;
   extern Datapath savegame_dpath;
   extern char real_archive_fn[20];
   extern void store_objects(char** buf, ObjID *obj_array, char obj_count);
   extern void restore_objects(char* buf, ObjID *obj_array, char obj_count);
   extern char* get_proj_datadir(char*);

//   if (level_num != 1)
//      player_struct.level = 1;
   if (!strnicmp(fn, "level", 5))
      player_struct.level = atoi(strncpy(buf,fn + 5,2));
   store_objects(&buf2,player_struct.inventory, NUM_GENERAL_SLOTS);
   switch(source)
   {
      case 0:        // local
         strcpy(buf,DATADIR);
         strcat(buf,fn);
         retval = load_current_map(buf,LEVEL_ID_NUM,NULL);
         break;
      case 1:        // currsave
         retval = load_level_from_file(level_num);
         break;
      case 2:        // archive
         retval = load_current_map(real_archive_fn,ResIdFromLevel(level_num),&savegame_dpath);
         break;
      case 3:        // network
         if (get_proj_datadir(buf) != NULL)
         {
            strcat(buf, fn);
            retval = load_current_map(buf,LEVEL_ID_NUM,NULL);
         }
         else retval = ERR_NULL;
         break;
      case 4:        // Old Res
         retval = load_current_map(fn,OLD_LEVEL_ID_NUM,&savegame_dpath);
         break;
   }
   restore_objects(buf2,player_struct.inventory, NUM_GENERAL_SLOTS);
   switch(retval)
   {
      case ERR_FOPEN:
         lg_sprintf(buf,"Error opening %s",fn);
         message_box(buf);
         break;
      case ERR_NOEFFECT:
         message_box("bad map version.");
         break;
      case OK:
         compute_shodometer_value(FALSE);
         config_set_single_value(CFG_LEVEL_VAR,CONFIG_STRING_TYPE,fn);
#ifndef GAMEONLY
         TileMapRedrawPixels(NULL,NULL);
         chg_set_flg(EDITVIEW_UPDATE);
#endif
         message_info("Load complete");
         break;
   }
}
#endif

#ifdef GADGET
bool load_level_func(short keycode, ulong context, void* data)
{
   char fn[256];
#ifndef GAMEONLY
   extern bool possible_change;
#endif
   extern Gadget* edit_root_gadget;

   fn[0] = '\0';
#ifndef GAMEONLY
   if ((!possible_change) || (confirm_box("Level changed without save!  Load anyways?")))
#endif
      level_saveload_box("Load Map",_current_root,1,fn,edit_load_func, TRUE);
   return(TRUE);
}
#endif

#define BACKUP_FNAME "shockbak.dat"

#ifdef PLAYTEST
void edit_save_func(char* fn, uchar source, short level_num)
{
   char buf[64],b2[64];
   extern void reset_schedules(void);
#ifndef GAMEONLY
   extern bool possible_change;
#endif
   extern char savegame_dir[50];
   extern Datapath savegame_dpath;

   Spew(DSRC_EDITOR_Restore, ("edit_save_func:  fn = %s\n",fn));

   // Make a backup of previous file, if it exists
   if (DatapathFind(&savegame_dpath, fn, buf))
   {
      strcpy(b2, savegame_dir);
      strcat(b2, "\\");
      strcat(b2, BACKUP_FNAME);
      copy_file(buf,b2);
   }
   strcpy(buf,fn);
   reset_schedules();
   switch(save_current_map(buf,LEVEL_ID_NUM, TRUE, TRUE))
   {
      case ERR_FOPEN:
         lg_sprintf(buf,"Error opening %s",fn);
         message_box(buf);
         break;
      case OK:
         message_info("Save complete.");
         config_set_single_value(CFG_LEVEL_VAR,CONFIG_STRING_TYPE,fn);
#ifndef GAMEONLY
         possible_change = FALSE;
#endif
         break;
   }
}
#endif

#ifdef GADGET
bool save_level_func(short keycode, ulong context, void* data)
{
   if (!saves_allowed)
   {
      message_box("Saves not allowed -- use control panel to change");
      return(FALSE);
   }
   if ((default_fname == NULL) && !config_get_raw(CFG_LEVEL_VAR,default_fname,256))
      strcpy(default_fname,MAP_FNAME);
   Spew(DSRC_EDITOR_Restore, ("default_fname = %s\n",default_fname));
   level_saveload_box("Save Map",_current_root,1,default_fname,edit_save_func,FALSE);
   return(TRUE);
}
#endif

#ifndef GAMEONLY
bool toggle_3d_func(short keycode, ulong context, void* data)
{
   TileEditor* te = (TileEditor*)data;
//   bool yes3d = !chg_get_sta(EDITVIEW_UPDATE);
   Point newsize;
   Point newloc;
   int z;

   if (yes_3d)
   {
      newsize.x = TILEMAP_REGION_WIDTH;
      newsize.y = TILEMAP_REGION_HEIGHT;
      newloc.x = TILEMAP_REGION_X;
      newloc.y = TILEMAP_REGION_Y;
      z = 0;
      chg_set_flg(EDITVIEW_UPDATE);
   }
   else
   {
      newsize.x = VIEW_REGION_WIDTH+TILEMAP_REGION_WIDTH;
      newsize.y = TILEMAP_REGION_HEIGHT;
      z = 1;
      newloc.x = VIEW_REGION_X;
      newloc.y = VIEW_REGION_Y;
      chg_unset_flg(EDITVIEW_UPDATE);
   }
   yes_3d = !yes_3d;
   region_begin_sequence();
   TileEditorResize(te,newsize);
   TileEditorMove(te,newloc,z);
   region_end_sequence(TRUE);
   return TRUE;
}

bool tilemap_mode_func(short keycode, ulong context, void* data)
{
   int mode;
   extern errtype terrain_palette_popup(void);
   extern void bitsmode_palette_popup(void);
   extern void cutpaste_palette_popup(void);
   extern void cybpal_popup(void);
   mode = (int)data;
   if (mode != current_palette_mode)
   {
      current_palette_mode = mode;
      switch (mode)
      {
         case OBJECT_MODE:
            object_palette_popup();
            break;
         case TERRAIN_MODE:
            terrain_palette_popup();
            break;
         case EYEBALL_MODE:
            eyeball_palette_popup();
            break;
         case CUTPASTE_MODE:
            cutpaste_palette_popup();
            break;
         case TEXTURING_MODE:
            if (global_fullmap->cyber) cybpal_popup();
            else                       texture_palette_popup();
            break;
         case BITS_MODE:
            bitsmode_palette_popup();
            break;

      }
   }   
   return(TRUE);
}

bool draw_mode_func(short keycode, ulong context, void* data)
{
   TileEditorSetMode(NULL,(int)data);
   return(TRUE);
}

bool clear_highlight_func(short keycode, ulong context, void* data)
{
   TileMapClearHighlights(NULL);
   TileMapRedrawPixels(NULL,NULL);
   return TRUE;
}
#endif

#ifndef GAMEONLY
bool texture_selection_func(short keycode, ulong context, void* data)
{
#ifdef TEXTURE_SELECTION
   textpal_create_selector();   
#endif
   return(TRUE);
}
#endif

#ifdef GADGET
bool lighting_func(short keycode, ulong context, void* data)
{
   panel_create_lighting();
   return(TRUE);
}                    

bool inp6d_panel_func(short keycode, ulong context, void* data)
{
   extern void panel_create_inp6d(void);
   panel_create_inp6d();
   return(TRUE);
}

bool render_panel_func(short keycode, ulong context, void* data)
{
   panel_create_renderer();
   return(TRUE);
}

bool popup_tilemap_func(short keycode, ulong context, void* data)
{
   return(TRUE);
}

#endif

#ifdef PLAYTEST
bool bkpt_me(short keycode, ulong context, void* data)
{  // put a break point here, goof
   return TRUE;
}
#endif

#ifdef GADGET
bool editor_options_func(short keycode, ulong context, void* data)
{
   editor_options->parent = _current_root;
   gad_menu_popup_at_mouse(editor_options);
   return(TRUE);
}

bool editor_modes_func(short keycode, ulong context, void* data)
{
   editor_modes->parent = _current_root;
   gad_menu_popup_at_mouse(editor_modes);
   return(TRUE);
}

bool misc_menu_func(short keycode, ulong context, void* data)
{
   main_misc_menu->parent = _current_root;
   main_misc_menu->parent = _current_root;
   renderer_misc_menu->parent = _current_root;
   gamesys_misc_menu->parent = _current_root;
   editor_misc_menu->parent = _current_root;
   misc_misc_menu->parent = _current_root;
   report_sys_menu->parent = _current_root;
   gad_menu_popup_at_mouse(main_misc_menu);
   return(TRUE);
}

bool control_panel_func(short keycode, ulong context, void* data)
{
   panel_create_control();
   return(TRUE);
}
#endif

#ifndef GAMEONLY
bool do_find_func(short keycode, ulong context, void* data)
{
   int hilite_num;
   extern errtype generic_tile_eyedropper(TileEditor* te);
   extern errtype TerrainPalUpdate(struct _terrainpal* tp);
   extern void texture_palette_update(void);

//   hilite_num = 0;
   switch (current_palette_mode)
   {
      case OBJECT_MODE:
         TileMapFindHighlightNum(NULL, &hilite_num);
         object_find_func(hilite_num);
         break;
      case TERRAIN_MODE:
         generic_tile_eyedropper(NULL);
         TerrainPalUpdate(NULL);
         break;
      case TEXTURING_MODE:
         generic_tile_eyedropper(NULL);
         texture_palette_update();
         break;
         
      default:
         generic_tile_eyedropper(NULL);
         break;
   }
   TileMapRedrawSquares(NULL,NULL);
   return(TRUE);
}
#endif

#ifdef PLAYTEST
#ifndef GAMEONLY
bool inp6d_kbd=TRUE;
#else
bool inp6d_kbd=FALSE;
#endif

bool stupid_slew_func(short keycode, ulong context, void* data)
{
   int dir = (int)data;
   int v1,v2;
   static int slew_scale=16;
   extern bool inp6d_kbd;

   if (inp6d_kbd==FALSE) return TRUE;

   switch(dir)
   {
      case 1:  v1 = EYE_Y; v2 = slew_scale; break;
      case 2:  v1 = EYE_H; v2 = -slew_scale; break;
      case 3:  v1 = EYE_Y; v2 = -slew_scale; break;
      case 4:  v1 = EYE_H; v2 = slew_scale; break;
      case 5:  v1 = EYE_Z; v2 = slew_scale; break;
      case 6:  v1 = EYE_Z; v2 = -slew_scale; break;
      case 7:  v1 = EYE_P; v2 = -slew_scale; break;
      case 8:  v1 = EYE_P; v2 = slew_scale; break;
      case 9:  v1 = EYE_B; v2 = -slew_scale; break;
      case 10:  v1 = EYE_B; v2 = slew_scale; break;
      case 11:  v1 = EYE_X; v2 = slew_scale; break;
      case 12:  v1 = EYE_X; v2 = -slew_scale; break;
      case 13:  v1 = EYE_RESET; v2 = -slew_scale; break;
      case 14:  if (slew_scale<256) slew_scale<<=1; return TRUE;
      case 15:  if (slew_scale>1) slew_scale>>=1; return TRUE;
   }
   fr_camera_slewcam(NULL,v1,v2);
   if (_current_loop <= FULLSCREEN_LOOP)
      chg_set_flg(DEMOVIEW_UPDATE);
#ifndef GAMEONLY
   if (_current_loop == EDIT_LOOP)
   {
      TileMapUpdateCameras(NULL);
      chg_set_flg(EDITVIEW_UPDATE);
   }
#endif
   return(TRUE);
}

bool zoom_3d_func(short keycode, ulong context, void* data)
{
   bool zoomin = (bool)data;

   // cant this be current based?
   if (zoomin)
      fr_mod_cams(_current_fr_context,FR_NOCAM,fix_make(0,62500));
   else
      fr_mod_cams(_current_fr_context,FR_NOCAM,fix_make(1,3000));
   return(TRUE);
}
#endif

#ifdef GADGET
bool menu_close_func(short keycode, ulong context, void* data)
{
   return(menu_all_popdown());
}
#endif

#ifdef PLAYTEST
bool mono_clear_func(short keycode, ulong context, void* data)
{
   mono_clear();
   return(FALSE);
}

bool mono_toggle_func(short keycode, ulong context, void* data)
{
   mono_setmode(MONO_TOG);
   message_info("Monochrome Toggled.");
   return(FALSE);
}
#endif

#ifdef GADGET
Gadget *edit_flags_gadget = NULL;
bool f0, f1, f2;

bool edit_flags_close(void *vg, void *ud)
{
   // Postprocess results into change flags
   if (f0)
   {
      chg_set_sta(ML_CHG_BASE<<0);
      chg_set_flg(ML_CHG_BASE<<0);
   }
   else
   {
      chg_unset_sta(ML_CHG_BASE<<0);
      chg_unset_flg(ML_CHG_BASE<<0);
   }
   if (f1)
   {
      chg_set_sta(ML_CHG_BASE<<1);
      chg_set_flg(ML_CHG_BASE<<1);
   }
   else
   {
      chg_unset_sta(ML_CHG_BASE<<1);
      chg_unset_flg(ML_CHG_BASE<<1);
   }
   if (f2)
   {
      chg_set_sta(ML_CHG_BASE<<2);
      chg_set_flg(ML_CHG_BASE<<2);
   }
   else
   {
      chg_unset_sta(ML_CHG_BASE<<2);
      chg_unset_flg(ML_CHG_BASE<<2);
   }
   gadget_destroy(&edit_flags_gadget);
   return(FALSE);
}

bool edit_flags_func(short keycode, ulong context, void* data)
{
   Point pt, ss;

   pt.x = 20; pt.y = 25;
   ss.x = 110;  ss.y = 8;
   if (edit_flags_gadget == NULL)
   {
      f0 = ((_change_flag&(ML_CHG_BASE<<0)) != 0);
      f1 = ((_change_flag&(ML_CHG_BASE<<1)) != 0);
      f2 = ((_change_flag&(ML_CHG_BASE<<2)) != 0);
      edit_flags_gadget = gad_qbox_start(_current_root, pt, 10, &EditorStyle, QB_ALIGNMENT, "edit_flags_gadget", ss);
      gad_qbox_add("Main Loop Flags", QB_TEXT_SLOT, NULL, QB_RD_ONLY);
      gad_qbox_add("Flag 0", QB_BOOL_SLOT, &f0, QB_ARROWS);
      gad_qbox_add("Flag 1", QB_BOOL_SLOT, &f1, QB_ARROWS);
      gad_qbox_add("Frame Rate", QB_BOOL_SLOT, &f2, QB_ARROWS);
      gad_qbox_add("Close", QB_PUSHBUTTON_SLOT, edit_flags_close, QB_NO_OPTION);
      gad_qbox_end();
   }
   return(FALSE);
}

bool music_ai_params_func(short keycode, ulong context, void* data)
{
   panel_ai_param_create();
   return(FALSE);
}
#endif

bool version_spew_func(short keycode, ulong context, void* data)
{
   char tmpstr[]=SIGNATURE;          /* for tracking versions */
   char temp[40];
   strcpy(temp,".... ");
   if (start_mem >= BIG_CACHE_THRESHOLD)
      temp[0] = 'C';
   if (start_mem > EXTRA_TMAP_THRESHOLD)
      temp[1] = 'T';
   if (start_mem > BLEND_THRESHOLD)
      temp[2] = 'B';
   if (start_mem > BIG_HACKCAM_THRESHOLD)
      temp[3] = 'M';
   strcat(temp,SYSTEM_SHOCK_VERSION);
   message_info(temp);
   return(FALSE);
}

#endif //��� NOT_YET

char conv_hex(char val);
bool location_spew_func(short , ulong , void* );


char conv_hex(char val)
{
   char retval = '?';
   if ((val >= 0) && (val <= 9))
      retval = '0' + val;
   else if ((val >= 10) && (val <= 15))
      retval = 'a' + (val - 10);
   return(retval);
}
/*KLC   moved to TOOLS.C
int str_to_hex(char val)
{
   int retval = 0;
   if ((val >= '0') && (val <= '9'))
      retval = val - '0';
   else if ((val >= 'A') && (val <= 'F'))
      retval = 10 + val - 'A';
   else if ((val >= 'a') && (val <= 'f'))
      retval = 10 + val - 'a';
   return(retval);
}

bool location_spew_func(short , ulong , void* )
{
   char goofy_string[32];

//#ifdef SVGA_SUPPORT
//   sprintf(goofy_string,"00:00.00:%s",get_temp_string(REF_STR_ScreenModeText + convert_use_mode));
//#else
   strcpy(goofy_string,"00:00.00 ");
//#endif
   goofy_string[0] = conv_hex( player_struct.level / 16 );
   goofy_string[1] = conv_hex( player_struct.level % 16 );
   if (!time_passes)
      goofy_string[2] = '!';
   goofy_string[3] = conv_hex( PLAYER_BIN_X / 16 );
   goofy_string[4] = conv_hex( PLAYER_BIN_X % 16 );
   if (!physics_running)
      goofy_string[5] = '*';
   goofy_string[6] = conv_hex( PLAYER_BIN_Y / 16 );
   goofy_string[7] = conv_hex( PLAYER_BIN_Y % 16 );

   message_info(goofy_string);
   return(FALSE);
}
*/

#ifdef NOT_YET	//���

#ifdef PLAYTEST
bool toggle_physics_func(short keycode, ulong context, void* data)
{
   physics_running = !physics_running;
   if (physics_running)
      message_info("Physics turned on");
   else
      message_info("Physics turned off");
   return(FALSE);
}

#define camera_info message_info
bool reset_camera_func(short keycode, ulong context, void* data)
{
   extern uchar cam_mode;
   extern cams objmode_cam, *motion_cam, player_cam;

   if ((uchar *)data)
   {
      if (cam_mode!=OBJ_STATIC_CAMERA) { camera_info("cant toggle"); return FALSE; }
      if (motion_cam!=NULL)
      {
         motion_cam=NULL;
         camera_info("back to cam control");
      }
      else
      {
         motion_cam=fr_camera_getdef();
         camera_info("back to obj control");
      }
   }
   else
   {
      camera_info("camera reset");
      cam_mode = OBJ_PLAYER_CAMERA;
      fr_camera_setdef(&player_cam);
   }
   chg_set_flg(_current_3d_flag);
   return(FALSE);
}

bool current_camera_func(short keycode, ulong context, void* data)
{
   extern cams objmode_cam, *motion_cam;
   extern uchar cam_mode;
   fix cam_locs[6], *cam_ptr_hack;

   motion_cam=NULL;
   // Not sure what to pass for last two params....
   switch ((uchar)data)
   {
   case OBJ_STATIC_CAMERA:
      if (cam_mode==OBJ_DYNAMIC_CAMERA) { camera_info("cant go static"); return FALSE; }
      motion_cam=fr_camera_getdef();         // note super sneaky fall through hack
      camera_info("camera static");
   case OBJ_DYNAMIC_CAMERA:
      fr_camera_modtype(&objmode_cam,CAMTYPE_ABS,CAMBIT_OBJ);
      cam_ptr_hack=fr_camera_getpos(NULL);
      memcpy(cam_locs,cam_ptr_hack,6*sizeof(fix));
      fr_camera_update(&objmode_cam,cam_locs,CAM_UPDATE_NONE,NULL);
      if (motion_cam==NULL) camera_info("camera dynamic");
      break;
   case OBJ_CURRENT_CAMERA:
      camera_info("current obj");
      fr_camera_modtype(&objmode_cam,CAMTYPE_OBJ,CAMBIT_OBJ);
      fr_camera_update(&objmode_cam, (void *)current_object,CAM_UPDATE_NONE,NULL);
      break;
   }
   cam_mode  = (uchar)data;
   fr_camera_setdef(&objmode_cam);
   chg_set_flg(_current_3d_flag);
   return(FALSE);
}

bool mono_log_on = FALSE;

bool log_mono_func(short keycode, ulong context, void* data)
{
   if (mono_log_on)
   {
      mono_logoff();
      message_info("Mono logging off.");
      mono_log_on = FALSE;
   }
   else
   {
      mono_logon("monolog.txt", MONO_LOG_NEW, MONO_LOG_ALLWIN);
      message_info("Mono logging on.");
      mono_log_on = TRUE;
   }
   return(FALSE);
}

bool clear_transient_lighting_func(short keycode, ulong context, void* data)
{
   int x,y;
   MapElem *pme;
   for (x=0; x < MAP_XSIZE; x++)
   {
      for (y=0; y < MAP_YSIZE; y++)
      {
         pme = MAP_GET_XY(x,y);
         me_templight_flr_set(pme,0);
         me_templight_ceil_set(pme,0);
      }
   }
   message_info("Trans. light cleared");
   return(FALSE);
}

bool level_entry_trigger_func(short keycode, ulong context, void* data)
{
   extern errtype do_level_entry_triggers();
   do_level_entry_triggers();
   message_info("Level entry triggered.");
   return(FALSE);
}

bool convert_one_level_func(short keycode, ulong context, void* data)
{
   extern errtype obj_level_munge();
#ifdef TEXTURE_CRUNCH_HACK
   extern errtype texture_crunch_init();

   texture_crunch_init();
#endif
   obj_level_munge();
   return(TRUE);
}

//#define CONVERT_FROM_OLD_RESID
//#define TEXTURE_CRUNCH_HACK

#define NUM_CONVERT_LEVELS 16

bool convert_all_levels_func(short keycode, ulong context, void* data)
{
   int i;
   char atoi_buf[10], fn[10], curr_fname[40], new_fname[40];
   errtype retval;

   extern Datapath savegame_dpath;
   extern void edit_load_func(char* fn, uchar source, short level_num);
   extern void edit_save_func(char* fn, uchar source, short level_num);
   extern errtype obj_level_munge();
#ifdef TEXTURE_CRUNCH_HACK
   extern errtype texture_crunch_init();
 
   texture_crunch_init();
#endif

   // save off old level
   edit_save_func("templevl.dat", 0,0);

   // loop through the real levels
   for (i=0; i<NUM_CONVERT_LEVELS; i++)
   {
      retval = OK;
      // load level i
      strcpy(fn, "level");
      strcat(fn, itoa(i,atoi_buf,10));
      strcat(fn, ".dat");
      Spew(DSRC_EDITOR_Modify, ("fn = %s\n",fn));
      if (DatapathFind(&savegame_dpath, fn, curr_fname))
      {
#ifdef CONVERT_FROM_OLD_RESID
         retval = load_current_map(curr_fname, OLD_LEVEL_ID_NUM, &savegame_dpath);
#else
         retval = load_current_map(curr_fname, LEVEL_ID_NUM, &savegame_dpath);
#endif
         Spew(DSRC_EDITOR_Modify, ("convert_all trying to load %s\n",curr_fname));
      }
      else
         retval = ERR_FOPEN;

      Spew(DSRC_EDITOR_Modify, ("curr_fname = %s\n",curr_fname));
      if (retval != OK)
      {
         strcpy(new_fname, "R:\\prj\\cit\\src\\data\\");
         strcat(new_fname, fn);
         retval = load_current_map(new_fname, LEVEL_ID_NUM, NULL);
         Spew(DSRC_EDITOR_Modify, ("new_fname = %s\n",new_fname));
      }

      // Generate the report
      obj_level_munge();
      Spew(DSRC_EDITOR_Modify, ("convert_all trying to save %s\n",fn));
      save_current_map(fn, LEVEL_ID_NUM,TRUE,TRUE);
   }

   // reload original level
   edit_load_func("templevl.dat",0,0);

   return(FALSE);
}

bool invulnerable_func(short keycode, ulong context, void* data)
{
   if (config_get_raw(CFG_HKEY_GO,NULL,0))
   {
      player_invulnerable = !player_invulnerable;
      if (player_invulnerable)
         message_info ("invulnerability on");
      else
         message_info ("invulnerability off");
   }
   else
   {
      message_info("Winners don't use hotkeys");
      damage_player(50, EXPLOSION_FLAG, 0);
   }
   return(FALSE);
}

bool pacifist_func(short keycode, ulong context, void* data)
{
   extern bool pacifism_on;
   pacifism_on = !pacifism_on;
   if (pacifism_on)
      message_info ("pacifism on");
   else
      message_info ("pacifism off");
   return(FALSE);
}

#endif

int pause_id;
bool remove_pause_handler = FALSE;

bool pause_callback(uiEvent *, LGRegion *, void *)
{
   return(TRUE);
}

bool unpause_callback(uiEvent *, LGRegion *, void *)
{
   return(TRUE);
}

#endif //��� NOT_YET

bool pause_game_func(short, ulong, void*)
{
	extern bool game_paused, redraw_paused;
	extern LGRegion *inventory_region;
	
	game_paused = TRUE;
	redraw_paused=TRUE;
/* KLC - not needed for Mac version
	game_paused = !game_paused;
	if (game_paused)
	{
		uiPushGlobalCursor(&globcursor);
		uiInstallRegionHandler(inventory_region, UI_EVENT_MOUSE_MOVE, pause_callback, NULL, &pause_id);
		uiGrabFocus(inventory_region, UI_EVENT_MOUSE_MOVE);
		stop_digi_fx();
		redraw_paused=TRUE;
	}
	else
	{
		uiRemoveRegionHandler(inventory_region, pause_id);
		uiReleaseFocus(inventory_region, UI_EVENT_MOUSE_MOVE);
		uiPopGlobalCursor();
	}
*/
	return(FALSE);
}

/*KLC - not needed for Mac version
bool unpause_game_func(short, ulong, void*)
{
	extern bool game_paused;
	extern LGRegion *inventory_region;
	
	if (game_paused)
	{
		game_paused = !game_paused;
		uiRemoveRegionHandler(inventory_region, pause_id);
		uiReleaseFocus(inventory_region, UI_EVENT_MOUSE_MOVE|UI_EVENT_JOY);
		uiPopGlobalCursor();
	}
	return(FALSE);
}
*/

//--------------------------------------------------------------------
//  For Mac version.  Save the current game.
//--------------------------------------------------------------------
bool save_hotkey_func(short, ulong, void *)
{
	if (global_fullmap->cyber)					// Can't save in cyberspace.
	{
		message_info("Can't save game in cyberspace.");
		return TRUE;
	}

	if (music_on)									// Setup the environment for doing Mac stuff.
		MacTuneKillCurrentTheme();
	uiHideMouse(NULL);
	ShowCursor();
	CopyBits(&gMainWindow->portBits, &gMainOffScreen.bits->portBits, &gActiveArea, &gOffActiveArea, srcCopy, 0L);
	
	if (gIsNewGame)									// Do the save thang.
	{
		status_bio_end();
		ShowMenuBar();
	 	DoSaveGameAs();
		HideMenuBar();
		status_bio_start();
	}
	else
	 	DoSaveGame();
	
	HideCursor();									// Go back to Shock.
	uiShowMouse(NULL);
	if (music_on)
		MacTuneStartCurrentTheme();
	
	return TRUE;
}


#ifdef NOT_YET  //���


//#define CHECK_STATE_N_HOTKEY
#ifdef PLAYTEST
bool check_state_func(short keycode, ulong context, void* data)
{
   int avail_memory(int debug_src);
   avail_memory(DSRC_TESTING_Test3);
#ifdef CHECK_STATE_N_HOTKEY
   extern void check_state_every_n_seconds();
   check_state_every_n_seconds();
#endif
#ifdef CORVIN_ZILM_HKEY
   extern bool CorvinZilm;
   extern int watchcount;
   MemStat pms;
   watchcount = 0;
   CorvinZilm = TRUE;
   MemStats(&pms);
#endif
   return(TRUE);
}

bool diffdump_game_func(short keycode, ulong context, void* data)
{
   char goof[45];
   sprintf(goof, "diff=%d,%d,%d,%d\n",player_struct.difficulty[0],player_struct.difficulty[1],player_struct.difficulty[2],
      player_struct.difficulty[3]);
   message_info(goof);
   return(TRUE);
}

bool toggle_difficulty_func(short keycode, ulong context, void *data)
{
   ubyte which = (ubyte) data-1;

   player_struct.difficulty[which]++;
   player_struct.difficulty[which] %= 4;
   return (TRUE);
}

bool toggle_ai_func(short keycode, ulong context, void* data)
{
   extern bool ai_on;
   ai_on = !ai_on;
   if (ai_on)
      message_info("AI state on\n");
   else
      message_info("AI state off\n");
   return(TRUE);
}

bool toggle_safety_net_func(short keycode, ulong context, void* data)
{
   extern bool safety_net_on;
   safety_net_on = !safety_net_on;
   if (safety_net_on)
      message_info("Safety Net on\n");
   else
      message_info("Safety Net off\n");
   return(TRUE);
}
#endif

#ifdef NEW_RES_LIB_INSTALLED
bool res_cache_usage_func(short keycode, ulong context, void* data)
{
   extern long ResViewCache(bool only_locks);
   ResViewCache((bool)data);
   return(TRUE);
}
#endif

#pragma enable_message(202)

#endif //��� NOT_YET
