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
 * $Source: r:/prj/cit/src/RCS/mfdfunc.c $
 * $Revision: 1.237 $
 * $Author: mahk $
 * $Date: 1994/11/23 20:34:20 $
 *
 */
#define __MFDFUNC_SRC

// Source code for all MFD Expose/Handler function pairs
// This file is for callbacks only, actual infrastructure belongs
// in newmfd.c

#include <string.h>

#include "objprop.h" // temp
#include "tools.h"
#include "colors.h"
#include "mainloop.h"
#include "gameloop.h"
#include "mfdart.h"
#include "gamescr.h"
//��#include "anim.h"
//��#include "animreg.h"
#include "objwarez.h"
#include "objsim.h"
#include "gamestrn.h"
#include "cybstrng.h"
#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "player.h"
#include "wares.h"
#include "drugs.h"
#include "weapons.h"
#include "automap.h"
#include "target.h"
#include "criterr.h"
#include "mfdgadg.h"
#include "objclass.h"
#include "otrip.h"
#include "citres.h"
#include "objuse.h"
#include "sfxlist.h"
#include "musicai.h"
#include "sideicon.h"
#include "hud.h"
#include "textmaps.h"
#include "fullscrn.h"
#include "objbit.h"
#include "limits.h"
#include "mapflags.h"
#include "input.h"
#include "cit2d.h"
#include "gr2ss.h"
#include "mfdgames.h"
#include "shodan.h"

#define MFD_SHIELD_FUNC 19

// ------------
// Useful Defines
// ------------

#define OVERLOAD_BUTTON_Y (MFD_VIEW_HGT - 24)
#define TEMPR_X      47
#define TEMPR_Y      27
#define TEMPR_WIDTH  13
#define TEMPR_HEIGHT 16
#define TEMPR_DIV    8
#define SETTING_TEXT 46
#define ENERGY_TEXT_LEN 40

static bool in_or_out = FALSE;

extern void mouse_unconstrain(void);
extern void mfd_ammo_expose(ubyte control);
extern bool mfd_ammo_handler(MFD* m, uiEvent* ev);

#define LNAME_BUFSIZE   128

#define GOOD_RED  (RED_BASE+5)
#define ITEM_COLOR  (0x5A)
#define SELECTED_ITEM_COLOR (0x4C)
#define UNAVAILABLE_ITEM_COLOR (0x60)
#define X_MARGIN 1
#define Y_STEP 5

extern void check_panel_ref(bool punt);

#define PUSH_CANVAS(x) gr_push_canvas(x)
#define  POP_CANVAS()  gr_pop_canvas()

#define MFD_REGION(m) ((full_game_3d) ? &(m)->reg2 : &(m)->reg)

// -------
// Globals
// -------

// Forward declaration of array at bottom of file

extern void lamp_set_vals(void);
extern bool full_game_3d;

LGRegion* mfd_regions[NUM_MFDS];

// ----------------
// Local Prototypes
// ----------------

void mfd_clear_view(void);
int mfd_bmap_id(int triple);
void draw_blank_mfd(void);
void draw_mfd_item_spew(Ref id, int n);

errtype mfd_item_init(MFD_Func* mfd);
void mfd_expose_blank(MFD *m, ubyte control);
void mfd_item_expose(MFD *m, ubyte control);
bool mfd_item_handler(MFD *m, uiEvent *e);
void mfd_item_micro_expose(bool full, int triple);
void mfd_item_micro_hires_expose(bool full, int triple);

void mfd_general_inv_expose(MFD* m, ubyte control, ObjID id, bool full);
bool mfd_general_inv_handler(MFD* m, uiEvent* ev, int row);

bool mfd_lantern_button_handler(MFD* m, LGPoint bttn, uiEvent* ev, void* data);
void mfd_lantern_setting(int setting);
errtype mfd_lanternware_init(MFD_Func* f);
void mfd_lanternware_expose(MFD* mfd, ubyte control);

void draw_ammo_button(int triple, short x, short y);

errtype mfd_anim_init();
void mfd_anim_expose(MFD *m, ubyte control);

errtype mfd_weapon_init(MFD_Func* mfd);
void weapon_mfd_for_reload(void);
void mfd_weapon_expose(MFD *m, ubyte control);
bool mfd_weapon_handler(MFD *m, uiEvent *e);
bool mfd_weapon_beam_handler(MFD *m, uiEvent *e);
bool mfd_weapon_projectile_handler(MFD *m, uiEvent *e, weapon_slot *ws);
bool mfd_weapon_expose_projectile(MFD *m, weapon_slot *ws, ubyte control);
void mfd_weapon_expose_beam(weapon_slot *ws, ubyte id, bool Redraw);
void mfd_weapon_draw_temp(ubyte temp);
void mfd_weapon_draw_ammo_buttons(int num_ammo_buttons, int ammo_subclass,
    ubyte *ammo_types, ubyte curr_ammo_type, int ammo_count);
void mfd_weapon_draw_beam_status_bar(int charge, int setting, bool does_overload);

void mfd_setup_keypad(char special);

bool weapon_mfd_temp;

void mfd_bioware_expose(MFD *m, ubyte control);


// ------------
// USEFUL STUFF 
// ------------

void mfd_clear_view(void)
{
   if (full_game_3d) return;
   ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
   gr_bitmap(&mfd_background, 0, 0);
   mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
}

int mfd_bmap_id(int triple)
{
   int obclass = TRIP2CL(triple);
   int t = CPTRIP(triple);
   return MKREF(RES_mfdClass_1+obclass,t);
}

void draw_blank_mfd(void)
{
   ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
//KLC - chg for new art   draw_res_bm(REF_IMG_bmBlankMFD, 0,0);
   draw_hires_resource_bm(REF_IMG_bmBlankMFD, 0, 0);
   draw_res_bm(MKREF(RES_mfdArtOverlays,MFD_ART_TRIOP),0,0);
}

// --------------------
// FUNCTION INITIALIZER
// --------------------

// ---------------------------------------------------------------------------
// mfd_init_funcs()
// 
// Here is where you set the global MFD_Func structures to point at
// expose/handler pairs, and also where you set their flags.  This is also
// where MFD virtual slots are set to point at their functions.

void mfd_init_funcs()
{
   int i;
   // Define a couple of MFD functions.
   for (i = 0; i < MFD_NUM_FUNCS; i++)
      if (mfd_funcs[i].init != NULL)
      {
         errtype err = mfd_funcs[i].init(&mfd_funcs[i]);
         if (err != OK)
            critical_error(CRITERR_MISC|1);
      }
                                
   // Set slots to point at functions
   set_slot_to_func(MFD_WEAPON_SLOT,  MFD_WEAPON_FUNC, MFD_ACTIVE);
   set_slot_to_func(MFD_ITEM_SLOT,    MFD_ITEM_FUNC,   MFD_ACTIVE);
   set_slot_to_func(MFD_MAP_SLOT,     MFD_MAP_FUNC,    MFD_ACTIVE);
   set_slot_to_func(MFD_INFO_SLOT,    MFD_EMPTY_FUNC,  MFD_ACTIVE);
   set_slot_to_func(MFD_TARGET_SLOT,  MFD_TARGET_FUNC, MFD_ACTIVE);

//   set_slot_to_func(MFD_SPECIAL_SLOT, MFD_ANIM_FUNC,   MFD_FLASH);
//   set_slot_to_func(MFD_SPECIAL_SLOT, MFD_EMPTY_FUNC, MFD_EMPTY);

//   debug_mfd_func_table();
//   debug_mfd_slots_table();
   return;
}

// ===========================================================================
//
// ===========================================================================

//                             --------------------
//                             ACTUAL MFD FUNCTIONS
//                             --------------------

// ===========================================================================
//                             * THE WEAPON MFD *
// ===========================================================================


// Hey, wow, floats and doubles are UNcool.

#define mfd_pixels_per_charge_unit (FIX_UNIT*69/100) // How many hor. pixels equals a % of charge?
#define mfd_charge_units_per_pixel (FIX_UNIT*100/69)

// note _LEFT is 0, _RIGHT is 1
#define MFDLeftOffs     MFD_LEFT
#define MFDRightOffs    MFD_RIGHT
#define MFDLastWeapon   0
#define MFDAmmo         2
#define MFDLastBeamHeat 4

#define MFD_Access(which,lorr)      mfd_fdata[MFD_WEAPON_FUNC][(which)+(lorr)]

#define MFDGetLastLeftWeapon        mfd_fdata[MFD_WEAPON_FUNC][0]
#define MFDGetLastRightWeapon       mfd_fdata[MFD_WEAPON_FUNC][1]
#define MFDGetLeftAmmo              mfd_fdata[MFD_WEAPON_FUNC][2]
#define MFDGetRightAmmo             mfd_fdata[MFD_WEAPON_FUNC][3]
#define MFDGetLastLeftBeamHeat      mfd_fdata[MFD_WEAPON_FUNC][4]
#define MFDGetLastRightBeamHeat     mfd_fdata[MFD_WEAPON_FUNC][5]
#define MFDSetLastLeftWeapon(n)    (mfd_fdata[MFD_WEAPON_FUNC][0] = (n))
#define MFDSetLastRightWeapon(n)   (mfd_fdata[MFD_WEAPON_FUNC][1] = (n))
#define MFDSetLeftAmmo(n)          (mfd_fdata[MFD_WEAPON_FUNC][2] = (n))
#define MFDSetRightAmmo(n)         (mfd_fdata[MFD_WEAPON_FUNC][3] = (n))
#define MFDSetLastLeftBeamHeat(n)  (mfd_fdata[MFD_WEAPON_FUNC][4] = (n))
#define MFDSetLastRightBeamHeat(n) (mfd_fdata[MFD_WEAPON_FUNC][5] = (n))

#define MFD_BEAMWPN_STAT_BORDER     GREEN_YELLOW_BASE
#define MFD_BEAMWPN_STAT_CHARGE     WHITE
#define MFD_BEAMWPN_STAT_MAXCHARGE  PURPLE_BASE
#define MFD_BEAMWPN_STAT_DEADSPACE  BLACK

#define WEAPON_ART_Y                7

#define AMMO_BUTTON_H               24
#define AMMO_BUTTON_W               23
#define AMMO_BUTTON_Y              (MFD_VIEW_HGT - AMMO_BUTTON_H) 
#define AMMO_STRING_Y              (AMMO_BUTTON_Y - 4)
#define AMMO_NAME_Y                (MFD_VIEW_HGT - 6)


#define AMMO_BUTTON_X1              29
#define AMMO_BUTTON_DX1             0
#define AMMO_BUTTON_X2              11
#define AMMO_BUTTON_DX2             31
#define AMMO_BUTTON_X3              1
#define AMMO_BUTTON_DX3             24

#define MFD_BEAM_RECT_X1            1
#define MFD_BEAM_RECT_X2            71
#define MFD_BEAM_RECT_Y1            52
#define MFD_BEAM_RECT_Y2            56

LGRect MfdAmmoRectZone = { { 0, AMMO_BUTTON_Y}, { MFD_VIEW_WID, AMMO_BUTTON_Y + AMMO_BUTTON_H}};
LGRect MfdBeamStatusRect;

#define NO_CONSTRAIN NUM_MFDS
static ubyte beam_constrain = NO_CONSTRAIN;

LGCursor slider_cursor;
grs_bitmap slider_cursor_bmap;


// ---------- WEAPON MFD FUNC ---------------

// --------------------------------------------------------------------------
// mfd_weapon_init()
//
// Initializes the MFD weapons function.

errtype mfd_weapon_init(MFD_Func* mfd)
{
#ifndef NO_DUMMIES
   void* yum; yum = mfd;
#endif //NO_DUMMIES

   MFDSetLastLeftWeapon(0);
   MFDSetLastRightWeapon(0);
   MFDSetLeftAmmo(0xFF);
   MFDSetRightAmmo(0xFF);

   MfdBeamStatusRect.ul.x = MFD_BEAM_RECT_X1;
   MfdBeamStatusRect.ul.y = MFD_BEAM_RECT_Y1;
   MfdBeamStatusRect.lr.x = MFD_BEAM_RECT_X2;
   MfdBeamStatusRect.lr.y = MFD_BEAM_RECT_Y2;

   return OK;
}

// --------------------------------------------------------------------------
// mfd_weapon_expose()
//
// Draws an overlay of a weapon in the current mfd slot.

void mfd_weapon_expose(MFD *m, ubyte control)
{
   weapon_slot *ws;
   char        buf[50];
   int         triple;
   bool        punt = player_struct.actives[ACTIVE_WEAPON] == EMPTY_WEAPON_SLOT;
   bool        Redraw = FALSE;
   bool        RedrawAmmoArea = TRUE;
   extern bool full_game_3d;

   if (control == 0)
   {
      uiCursorStack* cs;
      weapon_mfd_temp = FALSE;
      
      uiGetRegionCursorStack(MFD_REGION(m),&cs);
      uiPopCursorEvery(cs,&slider_cursor);


      if (beam_constrain == m->id)
      {
         beam_constrain = NO_CONSTRAIN;
//KLC         mouse_unconstrain();
      }
      return;
   }


   // Get the triple for the current weapon

   if (!punt) ws = &player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]];
   if (ws->type == EMPTY_WEAPON_SLOT) punt = TRUE;
   if (punt)
   {
      mfd_expose_blank(m,control);
      return;
   }

   triple = MAKETRIP(CLASS_GUN, ws->type, ws->subtype);

   if (control & MFD_EXPOSE) {

      PUSH_CANVAS(pmfd_canvas);
      mfd_clear_rects();

      if (control & MFD_EXPOSE_FULL) Redraw = TRUE;
      if (MFD_Access(m->id,MFDLastWeapon)!=player_struct.actives[ACTIVE_WEAPON])
      {
         Redraw = TRUE;
         MFD_Access(m->id,MFDLastWeapon)=player_struct.actives[ACTIVE_WEAPON];
         MFD_Access(m->id,MFDAmmo)=0xFF;
         weapon_mfd_temp = FALSE;
      }

      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
      if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);

      // Draw the appropriate weapon art.
      // We draw it here because it is effectively "background" 
      // Hopefully update-rects will take care of us..
      {
         int id = mfd_bmap_id(triple);
//KLC - chg for new art         draw_res_bm(id,(MFD_VIEW_WID-res_bm_width(id))/2, WEAPON_ART_Y);
         draw_hires_resource_bm(id,
         		(SCONV_X(MFD_VIEW_WID)-res_bm_width(id))/2, SCONV_Y(WEAPON_ART_Y));  //��� is this right?
      }

      // This is all stuff that should be drawn for a full expose of
      // a new weapon 
      if (Redraw) {
         short y = 2;
         short w,h;


         // Print name of gun in top line of mfd
         get_weapon_long_name(ws->type, ws->subtype, buf);
         mfd_draw_string(buf, X_MARGIN, y, GOOD_RED, TRUE);
         gr_string_size(buf,&w,&h);
         y += h;

         mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
 
      }
      
      // Here is where the dynamic info for a given weapon is drawn; ie; data that
      // requires a redraw even when the current weapon has not changed

      if ((ws->type == GUN_SUBCLASS_BEAM) || (ws->type == GUN_SUBCLASS_BEAMPROJ))
         mfd_weapon_expose_beam(ws,m->id, Redraw);
      else if (ws->type != GUN_SUBCLASS_HANDTOHAND)
         RedrawAmmoArea = mfd_weapon_expose_projectile(m,ws,control);

      // Redraw everything
      POP_CANVAS();
      mfd_update_rects(m);
   }

   return;
}



// --------------------------------------------------------------------------
// mfd_weapon_expose_projectile()
//
// Exposes relevant weapons mfd info for a projectile weapon.
// returns whether or not it drew the ammo buttons

bool mfd_weapon_expose_projectile(MFD *m, weapon_slot *ws, ubyte control)
{
   int         num_ammo_buttons;
   int         ammo_subclass;
   ubyte       ammo_types[3];
   bool        RedrawAmmoFlag = FALSE;
    
   // Get the ammo data for the current weapon
   get_available_ammo_type(ws->type, ws->subtype, &num_ammo_buttons,
      ammo_types, &ammo_subclass);

   if ((control & MFD_EXPOSE_FULL) || (ws->ammo == 0))
      RedrawAmmoFlag = TRUE;
   else if (MFD_Access(m->id,MFDAmmo)!=ws->ammo)
    { RedrawAmmoFlag = TRUE; MFD_Access(m->id,MFDAmmo)=ws->ammo; }

   // Redraw ammo buttons if neccessary
   if (RedrawAmmoFlag)
      mfd_weapon_draw_ammo_buttons(num_ammo_buttons,
         ammo_subclass, ammo_types, ws->ammo_type, ws->ammo);
 
   return RedrawAmmoFlag;
}

// ---------------------------------------------------------------------------
// mfd_weapon_draw_ammo_buttons()
// 
// Draws the labelled buttons of ammo on a projectile gun's mfd

#define MAX_CART_COLORS 3
#define CARTRIDGE_BRACKET 8              // cartridges per color
static uchar cart_colors[MAX_CART_COLORS] = { GOOD_RED, 0x4b, GREEN_BASE + 2} ; 

void draw_ammo_button(int triple, short x, short y)
{
   short h;
   int id;
   int carts = player_struct.cartridges[CPTRIP(triple)];
   ubyte cnum = min(MAX_CART_COLORS-1,(carts-1)/CARTRIDGE_BRACKET);

   // Draw the outline of an ammo box
   draw_res_bm(REF_IMG_BullFrame+min(2,max(3-carts,0)),x,y);
   id = mfd_bmap_id(triple);
   h = res_bm_height(id);
//KLC - chg for new art   draw_res_bm(id,x+4,y+AMMO_BUTTON_H-4-h);
   draw_hires_resource_bm(id, SCONV_X(x+4), SCONV_Y(y+AMMO_BUTTON_H-3)-h);
   if (carts > 0)
   {
      int cnt = carts%CARTRIDGE_BRACKET;
      if (cnt == 0) cnt = CARTRIDGE_BRACKET;
      gr_set_fcolor(cart_colors[cnum]);
      while(cnt-- > 0)
      {
         short cy = y+AMMO_BUTTON_H-5-2*cnt;
         ss_hline(x+AMMO_BUTTON_W-6,cy,x+AMMO_BUTTON_W-5);
      }
   }
   if (carts > 0 || player_struct.partial_clip[CPTRIP(triple)] > 0)
      mfd_add_rect(x,y,x+AMMO_BUTTON_W,y+AMMO_BUTTON_H);
}

void mfd_weapon_draw_ammo_buttons(int num_ammo_buttons, int ammo_subclass,
    ubyte *ammo_types, ubyte curr_ammo_type, int ammo_count)
{
   int  i, triple;

   // Draw ammo boxes
   if (ammo_count == 0)
   {
      for (i = 0; i < num_ammo_buttons; i++)
      {
         short x = (ammo_types[i]) * AMMO_BUTTON_W;
         triple = MAKETRIP(CLASS_AMMO, ammo_subclass, ammo_types[i]);
         // Get useful ammo information for box label
         draw_ammo_button(triple,x,AMMO_BUTTON_Y);
      }
      if (num_ammo_buttons > 0) 
         mfd_draw_string(get_temp_string(REF_STR_ClickToLoad),1,AMMO_STRING_Y,GREEN_YELLOW_BASE,TRUE);
   }
   else
   {
      char buf[4],buf2[50];
      triple = MAKETRIP(CLASS_AMMO, ammo_subclass, curr_ammo_type);

      numtostring(ammo_count, buf);  //KLC  itoa(ammo_count,buf,10);
      gr_set_font((grs_font*)ResGet(RES_mediumLEDFont));
      mfd_string_shadow = MFD_SHADOW_NEVER;
      mfd_draw_font_string(buf,MFD_VIEW_WID-gr_string_width(buf)-2, AMMO_BUTTON_Y+2, GOOD_RED,RES_mediumLEDFont,TRUE);
      mfd_string_shadow = MFD_SHADOW_FULLSCREEN; // default

      get_object_short_name(triple,buf2,50);
      gr_set_font((grs_font*)ResGet(MFD_FONT));
      mfd_draw_string(buf2, MFD_VIEW_WID -gr_string_width(buf2)-2, AMMO_NAME_Y, GREEN_YELLOW_BASE, TRUE);

      draw_ammo_button(triple,0,AMMO_BUTTON_Y);
      draw_res_bm(REF_IMG_BullRightArrow,AMMO_BUTTON_W,AMMO_BUTTON_Y);
   }

   if ((num_ammo_buttons == 0) && (ammo_count == 0))
   {
      draw_res_bm(REF_IMG_NoAmmo,(MFD_VIEW_WID-res_bm_width(REF_IMG_NoAmmo))/2,AMMO_BUTTON_Y);   
   }
   mfd_add_rect(0,AMMO_STRING_Y,MFD_VIEW_WID,MFD_VIEW_HGT);
   return;
}

#define MFD_BEAM_WARM        0x40  
#define MFD_BEAM_HOT         0x35
#define MFD_BEAM_READY       0x58

ubyte temp_levels[TEMPR_WIDTH] =
   {1,2,2,3,3,4,5,6,7,9,11,13,16};

// --------------------------------------------------------------------------
// mfd_weapon_draw_temp()
//

void mfd_weapon_draw_temp(ubyte temp)
{
   int i;

   gr_set_fcolor(MFD_BEAM_READY);

   for (i=0; i<TEMPR_WIDTH; i++)
   {
      if (i == 5)
         gr_set_fcolor(MFD_BEAM_WARM);
      else if (i==10)
         gr_set_fcolor(MFD_BEAM_HOT);

      if (temp >= i*TEMPR_DIV)
         ss_vline(TEMPR_X+i*2, TEMPR_Y+TEMPR_HEIGHT-temp_levels[i], TEMPR_Y+TEMPR_HEIGHT);
   }
}

// --------------------------------------------------------------------------
// mfd_weapon_draw_beam_status_bar()
//
// Takes the current charge and the maximum charge, and draws the dynamic
// portion of the beam weapon status bar

void mfd_weapon_draw_beam_status_bar(int, int setting, bool does_overload)
{
   ubyte setting_x;

   setting = (BEAM_SETTING_VAL(setting) < MIN_ENERGY_USE) ? 1 : (BEAM_SETTING_VAL(setting)-MIN_ENERGY_USE);
   if (does_overload)
      setting <<= 1;

   setting++;

   ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

   gr_set_fcolor(MFD_BEAMWPN_STAT_BORDER); // OUTLINE
   ss_box(MfdBeamStatusRect.ul.x - 1,
          MfdBeamStatusRect.ul.y - 1,
          MfdBeamStatusRect.lr.x + 1,
          MfdBeamStatusRect.lr.y + 1);   
 
   setting_x = (ubyte) fix_int(setting*mfd_pixels_per_charge_unit);
   setting_x = min(MfdBeamStatusRect.lr.x-MfdBeamStatusRect.ul.x,setting_x);

   // draw the settings bar only if we're not in overload mode

   if (!in_or_out)
      draw_raw_resource_bm(REF_IMG_BeamSetting, MfdBeamStatusRect.ul.x + setting_x-3, MfdBeamStatusRect.ul.y - 1);

   mfd_add_rect(MfdBeamStatusRect.ul.x,
                MfdBeamStatusRect.ul.y,
                MfdBeamStatusRect.lr.x,
                MfdBeamStatusRect.lr.y);
   return;
}

// --------------------------------------------------------------------------
// mfd_weapon_expose_beam()
//
// Exposes relevant weapons mfd info for a beam weapon.

void mfd_weapon_expose_beam(weapon_slot *ws, ubyte id, bool Redraw)
{
   char buf[ENERGY_TEXT_LEN];
   bool does_overload = FALSE;
   extern bool does_weapon_overload(int type, int subtype);

   ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
   if (id == MFD_LEFT)
   {
      if (MFDGetLastLeftBeamHeat > ws->ammo)
         Redraw = TRUE;
      MFDSetLastLeftBeamHeat(ws->ammo);
   }
   if (id == MFD_RIGHT)
   {
      if (MFDGetLastRightBeamHeat > ws->ammo)
         Redraw = TRUE;
      MFDSetLastRightBeamHeat(ws->ammo);
   }

   does_overload = does_weapon_overload(ws->type, ws->subtype);
   if (does_overload)
   {
      // if beam weapon is not set to overload then draw overload button, otherwise say "overload enabled"
      if (Redraw && OVERLOAD_VALUE(ws->setting))
      {
         short w = res_bm_width(REF_IMG_BeamOverloadOn);

         draw_raw_resource_bm(REF_IMG_BeamOverloadOn,1,OVERLOAD_BUTTON_Y);
         mfd_add_rect(1,OVERLOAD_BUTTON_Y,1+w,MFD_VIEW_HGT);

         get_string(REF_STR_Overload,buf,ENERGY_TEXT_LEN);
         mfd_draw_string(buf, 1, SETTING_TEXT, MFD_BEAM_HOT, TRUE);
      }
      else
      {
         short w = res_bm_width(REF_IMG_BeamOverload);

         if (Redraw)
         {
            (ws->ammo < MINIMUM_OVERLOAD) ? 
               draw_raw_resource_bm(REF_IMG_BeamOverload,1,OVERLOAD_BUTTON_Y) :
               draw_raw_resource_bm(REF_IMG_BeamOverloadOff,1,OVERLOAD_BUTTON_Y);
            mfd_add_rect(1,OVERLOAD_BUTTON_Y,1+w,MFD_VIEW_HGT);
            get_string(REF_STR_EnergySetting,buf,ENERGY_TEXT_LEN);
            mfd_draw_string(buf, 1, SETTING_TEXT, MFD_BEAM_READY, TRUE);
         }
      }
   }

   get_string(REF_STR_LowSetting,buf,ENERGY_TEXT_LEN);
   mfd_draw_string(buf, 2, MFD_BEAM_RECT_Y1 - 1, MFD_BEAM_READY, TRUE);
   get_string(REF_STR_HighSetting,buf,ENERGY_TEXT_LEN);
   mfd_draw_string(buf, 57, MFD_BEAM_RECT_Y1 - 1, MFD_BEAM_READY, TRUE);

   draw_raw_resource_bm(REF_IMG_BeamTemperature, TEMPR_X, TEMPR_Y);

   mfd_weapon_draw_temp(ws->ammo);

   // Redraw the beam energy status bar
   mfd_weapon_draw_beam_status_bar(ws->ammo, ws->setting,does_overload);

   return;
}

// ---------------------------------------------------------------------------
// mfd_weapon_handler()
//
// Mostly responsible for figuring out which ammo boxes were clicked on
// to select new ammo, and for manipulating the charge on beam weapons 

bool mfd_weapon_handler(MFD *m, uiEvent *e)
{
   weapon_slot *ws;
   uiMouseEvent *mouse;
   int triple;

   // Get the triple for the current weapon
   ws = &player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]];
   triple = MAKETRIP(CLASS_GUN, ws->type, ws->subtype);

   mouse = (uiMouseEvent *) e;

   switch(ws->type) {
      case GUN_SUBCLASS_BEAM:        // Is beam charge being set?
      case GUN_SUBCLASS_BEAMPROJ:        // Is beam charge being set?
//         if (!(mouse->action & ~MOUSE_MOTION) && !(mouse->buttons & (1 << MOUSE_LBUTTON)))
//            return FALSE;
         return mfd_weapon_beam_handler(m,e);
         break;
      case GUN_SUBCLASS_PISTOL:
      case GUN_SUBCLASS_AUTO:
      case GUN_SUBCLASS_SPECIAL:
         if (mouse->action == MOUSE_MOTION) return FALSE;
         return mfd_weapon_projectile_handler(m,e,ws);
         break;
      default:                 
         break;
   }
   return FALSE;
}

ubyte old_energy_setting = 0xFF;

// ---------------------------------------------------------------------------
// mfd_weapon_beam_handler()
//
// This is the handler for beam type weapons in the weapons mfd.

extern bool does_weapon_overload(int type, int subtype);

bool mfd_weapon_beam_handler(MFD *m, uiEvent *e)
{
   bool retval = TRUE;
   LGRect r;
   ubyte setting, setting_x;
   uiMouseEvent *mouse;
   weapon_slot *ws = &player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]];
   bool overld = does_weapon_overload(ws->type, ws->subtype);

#ifdef CURSOR_BACKUPS
   extern grs_bitmap backup_mfd_cursor;
   extern uchar *backup[NUM_BACKUP_BITS];
#endif

   mouse = (uiMouseEvent *) e;

   // We're interested in this event iff its a mouse up,down,action event
   // in the beam status bar.
   if (!(mouse->action & (MOUSE_LUP|MOUSE_LDOWN|MOUSE_MOTION))) { return FALSE; }

   if (overld)
   {
      // okay - here's the overload button code
      if (mouse->action & MOUSE_LDOWN)
      {

         if (ws->ammo < MINIMUM_OVERLOAD)
         {
            // set up the rect for the overload button
            r.ul.x = (MFD_VIEW_WID - res_bm_width(REF_IMG_NoAmmo))/2;
            r.ul.y = OVERLOAD_BUTTON_Y;
            r.lr.x = r.ul.x + res_bm_width(REF_IMG_BeamOverload);
            r.lr.y = OVERLOAD_BUTTON_Y + res_bm_height(REF_IMG_BeamOverload);
            RECT_OFFSETTED_RECT(&r, m->rect.ul, &r);

            // check if we clicked in the button
            if(RECT_TEST_PT(&r, e->pos))
            {
               // toggle between the two values
               chg_set_flg(INVENTORY_UPDATE);
               OVERLOAD_VALUE(ws->setting) ? OVERLOAD_RESET(ws->setting) : OVERLOAD_SET(ws->setting);
               mfd_force_update();              // make sure it redraws the mfd
               return TRUE;      
            }
         }
      }
   }

   RECT_OFFSETTED_RECT(&MfdBeamStatusRect, m->rect.ul, &r);

   if (!RECT_TEST_PT(&r,e->pos))
   {
      uiCursorStack* cs;

      uiGetRegionCursorStack(MFD_REGION(m),&cs);
      uiPopCursorEvery(cs,&slider_cursor);
      mfd_notify_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT,FALSE,MFD_ACTIVE,FALSE);
      if (!in_or_out) return retval;
   }
   if (!in_or_out && (mouse->buttons == 0)) return retval;

   // If the left button was pushed, we constrain the mouse to
   // the beam status bar, and change the cursor appropriately
   if (mouse->action & MOUSE_LDOWN) {

      in_or_out = TRUE;

      // Constrain the mouse to a 1-pixel y
      slider_cursor.hotspot.x=slider_cursor_bmap.w/2;
      slider_cursor.hotspot.y=(slider_cursor_bmap.h/2)+mouse->pos.y-(r.ul.y+r.lr.y)/2;
      ui_mouse_constrain_xy(r.ul.x+1,mouse->pos.y,r.lr.x-2,mouse->pos.y);
      beam_constrain = m->id;
      // Get our funky mfd-beam-phaser-setting cursor
      uiPushRegionCursor(MFD_REGION(m),&slider_cursor);
#ifdef CURSOR_BACKUPS
   backup[20] = (uchar *)Malloc(f->bm.w * f->bm.h);
   LG_memcpy(backup[20],f->bm.bits,f->bm.w * f->bm.h);
      gr_init_bm(&backup_mfd_cursor,backup[14],BMT_FLAT8, 0, mfd_cursor.w,mfd_cursor.h);
#endif
      retval = TRUE;
   }

   // If the left button was released, unconstrain the mouse and reset
   // the cursor bitmap.  There's no else here for the weird case that
   // an up and down event might happen "simultaneously"
   if (mouse->action & MOUSE_LUP) {

      in_or_out = FALSE;

      // Let the mouse run free
      // note that we are NOT using the UI here since we're already looking at grd_cap
      mouse_constrain_xy(0,0,grd_cap->w-1,grd_cap->h-1);
      beam_constrain = NO_CONSTRAIN;
      
      uiPopRegionCursor(MFD_REGION(m));
      mfd_notify_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT,FALSE,MFD_ACTIVE,TRUE);


      retval = TRUE;
   }

   // Calculate the max charge setting based on mouse position (making goofy
   // exceptions for the endpoints for aesthetics) and set the weapon
   // accordingly.
   setting_x = e->pos.x - r.ul.x;
   if ((mouse->action & MOUSE_MOTION) && (setting_x == old_energy_setting))
      return retval;
   old_energy_setting = setting_x;

   setting = (ubyte) fix_int(setting_x * mfd_charge_units_per_pixel);

   if (overld)
      setting >>= 1;

   setting += MIN_ENERGY_USE;
   uiSetCursor();

   set_beam_weapon_max_charge(player_struct.actives[ACTIVE_WEAPON],setting);
//      min(MAX_HEAT,setting));

   return TRUE;
}

// ---------------------------------------------------------------------------
// mfd_weapon_projectile_handler()
//
// This is the handler for projectile weapons in the weapons mfd.

bool mfd_weapon_projectile_handler(MFD *m, uiEvent *e, weapon_slot *ws)
{
   int          ammo_subclass, num_ammo_buttons;
   ubyte        ammo_types[3];
   uiMouseEvent *mouse;
   LGPoint pos = e->pos;

   pos.x -= m->rect.ul.x;
   pos.y -= m->rect.ul.y;
   mouse = (uiMouseEvent *) e;

   if (pos.y < AMMO_BUTTON_Y)
      return FALSE;
   // If we're already loaded, check for double click.  
   if (ws->ammo > 0)
   {
      bool retval = FALSE;
      if (mouse->action & UI_MOUSE_LDOUBLE)
      {
         extern void unload_current_weapon(void);

         unload_current_weapon();
         MFDSetLeftAmmo(0xFF);
         MFDSetRightAmmo(0xFF);
         mfd_notify_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT,FALSE,MFD_ACTIVE,FALSE);
         mfd_notify_func(MFD_ITEM_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,FALSE);
         retval = TRUE;
      }
      else if (mouse->action & MOUSE_LDOWN)
      {
         string_message_info(REF_STR_DClickToUnload);
         retval = TRUE;
      }
      return retval;
   }

   // If it wasn't a left-mouse-button down event, throw it away.         
   if (!(mouse->action & (MOUSE_LDOWN|UI_MOUSE_LDOUBLE))) return FALSE;

   // Get the ammo data for the current weapon
   get_available_ammo_type(ws->type, ws->subtype, &num_ammo_buttons,
      ammo_types, &ammo_subclass);
   {
      int b = pos.x/AMMO_BUTTON_W;
      int atype;
      for(atype=0;atype<num_ammo_buttons;atype++) {
         // If we have a hit, we let each mfd know independently that
         // it needs to redraw its ammo buttons, to save redraw time
         if(b==ammo_types[atype] && change_ammo_type(b)) {
            if(weapon_mfd_temp) {
               int mfd;
               for(mfd=0;mfd<NUM_MFDS;mfd++) {
                  if(player_struct.mfd_current_slots[mfd]==MFD_WEAPON_SLOT)
                     restore_mfd_slot(mfd);
               }
               weapon_mfd_temp=FALSE;
            }
            MFDSetLeftAmmo(0xFF);
            MFDSetRightAmmo(0xFF);
            mfd_notify_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT,FALSE,MFD_ACTIVE,FALSE);
            mfd_notify_func(MFD_ITEM_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,FALSE);
            break;
         }
      }
   }

   return TRUE;
}

void weapon_mfd_for_reload(void)
{
   uchar target_pri;
   uchar take_mfd;

   extern int mfd_choose_func(int func, int slot);

   // Do not take down target mfd in favor of weapon in this case!
   target_pri = mfd_funcs[MFD_TARGET_FUNC].priority;
   mfd_funcs[MFD_TARGET_FUNC].priority = 1;
   take_mfd=mfd_choose_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT);
   if(player_struct.mfd_current_slots[take_mfd]!=MFD_WEAPON_SLOT) {
      save_mfd_slot(take_mfd);
      weapon_mfd_temp=TRUE;
   }
   mfd_change_slot(take_mfd,MFD_WEAPON_SLOT);
   mfd_funcs[MFD_TARGET_FUNC].priority = target_pri;
}

// ===========================================================================
//                             * THE ITEM MFD *
// ===========================================================================

// OK, the item MFD is pretty heinous, and, in fact, they all 
// really want to be their own MFDfuncs, so, let's grant them their wish!


#define MFDGetLastItemClass(m) mfd_fdata[MFD_ITEM_FUNC][(m)]
#define MFDGetLastItemType(m)  mfd_fdata[MFD_ITEM_FUNC][(m)+2]
#define MFDGetCurrItemClass(m) mfd_fdata[MFD_ITEM_FUNC][(m)+4]
#define MFDGetCurrItemType(m)  mfd_fdata[MFD_ITEM_FUNC][(m)+6]

#define MFDSetLastItemClass(m,n) mfd_fdata[MFD_ITEM_FUNC][(m)]   = (n)
#define MFDSetLastItemType(m,n)  mfd_fdata[MFD_ITEM_FUNC][(m)+2] = (n)
#define MFDSetCurrItemClass(m,n) mfd_fdata[MFD_ITEM_FUNC][(m)+4] = (n)
#define MFDSetCurrItemType(m,n)  mfd_fdata[MFD_ITEM_FUNC][(m)+6] = (n)

LGRect MfdGrenadeBox[2];

#define MFD_GRENADE_BOX_X1     27
#define MFD_GRENADE_BOX_X2     47
#define MFD_GRENADE_BOX_Y      5
#define MFD_GRENADE_BOX_W      5
#define MFD_GRENADE_BOX_H      5

#define HARDWARE_BUTTON_H 13
#define HARDWARE_BUTTON_W 44
#define HARDWARE_BUTTON_X ((MFD_VIEW_WID - HARDWARE_BUTTON_W)/2)
#define HARDWARE_BUTTON_Y (MFD_VIEW_HGT - HARDWARE_BUTTON_H -1)

#define DRUG_BUTTON_H 13
#define DRUG_BUTTON_W 32
#define DRUG_BUTTON_X ((MFD_VIEW_WID - DRUG_BUTTON_W)/2)
#define DRUG_BUTTON_Y (MFD_VIEW_HGT- 1 - DRUG_BUTTON_H)


// --------------------------------------------------------------------------
// mfd_item_init()
//
// Sets some info.

errtype mfd_item_init(MFD_Func *)
{
   MFDSetCurrItemClass(0,MFD_INV_NULL);
   MFDSetCurrItemClass(1,MFD_INV_NULL);
   MFDSetLastItemClass(0,MFD_INV_NULL);
   MFDSetLastItemClass(1,MFD_INV_NULL);

   MfdGrenadeBox[0].ul.x = MFD_GRENADE_BOX_X1;
   MfdGrenadeBox[0].ul.y = MFD_GRENADE_BOX_Y;
   MfdGrenadeBox[0].lr.x = MFD_GRENADE_BOX_X1 + MFD_GRENADE_BOX_W;
   MfdGrenadeBox[0].lr.y = MFD_GRENADE_BOX_Y + MFD_GRENADE_BOX_H;

   MfdGrenadeBox[1].ul.x = MFD_GRENADE_BOX_X2;
   MfdGrenadeBox[1].ul.y = MFD_GRENADE_BOX_Y;
   MfdGrenadeBox[1].lr.x = MFD_GRENADE_BOX_X2 + MFD_GRENADE_BOX_W;
   MfdGrenadeBox[1].lr.y = MFD_GRENADE_BOX_Y + MFD_GRENADE_BOX_H;

   return OK;
}
 
//--------------------------------------------------------------
// Like mini-expose, but gets the name for you, and 
// conforms to our rect-o-tronic update facility

void mfd_item_micro_expose(bool full, int triple)
{
   if (!full_game_3d)
   {
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
      gr_bitmap(&mfd_background, 0, 0);
   }
   if (full)
   {
      LGPoint siz;
      int id;
      short y = 2; 
      char buf[LNAME_BUFSIZE];
      mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      get_object_long_name(triple,buf,LNAME_BUFSIZE);
      siz = mfd_draw_string(buf, X_MARGIN, y, GREEN_YELLOW_BASE, TRUE);
      y+= siz.y +2;

      // Draw the appropriate weapon art
      id = mfd_bmap_id(triple);
      if (RefIndexValid((RefTable*)ResGet(REFID(id)),REFINDEX(id)))
         draw_raw_resource_bm(id,(MFD_VIEW_WID-res_bm_width(id))/2,y);
      else
         ResUnlock(REFID(id));
   }
   return;
}

//--------------------------------------------------------------
// Called by things that know they have hi-res art to display. 

void mfd_item_micro_hires_expose(bool full, int triple)
{
	if (!full_game_3d)
	{
		ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
		gr_bitmap(&mfd_background, 0, 0);
	}
	if (full)
	{
		LGPoint	siz;
		int 		id;
		short 	y = 2; 
		char 		buf[LNAME_BUFSIZE];
		
		mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
		ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
		
		get_object_long_name(triple,buf,LNAME_BUFSIZE);
		siz = mfd_draw_string(buf, X_MARGIN, y, GREEN_YELLOW_BASE, TRUE);
		y+= siz.y +2;
		
		id = mfd_bmap_id(triple);
		if (RefIndexValid((RefTable*)ResGet(REFID(id)),REFINDEX(id)))
			draw_hires_resource_bm(id, (SCONV_X(MFD_VIEW_WID)-res_bm_width(id))/2, SCONV_Y(y));
		else
			ResUnlock(REFID(id));
	}
	return;
}

// --------------------------------------------------------------------------
// mfd_item_draw_grenade_setting_boxes()
//
// Draws the little increment/decrement boxes on either side of the
// "time left before grenade blows up" setting on the MFD.

#ifdef OLD_GRENADE_BUTTONS
void mfd_item_draw_grenade_setting_boxes(MFD *m)
{
   char buf[2] ;
   int i;

   for (i = 0; i < 2; i++) {
      gr_set_fcolor(WHITE);
      ss_rect(MfdGrenadeBox[i].ul.x,MfdGrenadeBox[i].ul.y,
         MfdGrenadeBox[i].lr.x,MfdGrenadeBox[i].lr.y);
      gr_set_fcolor(MFD_BTTN_FLASH);
      ss_box(MfdGrenadeBox[i].ul.x,MfdGrenadeBox[i].ul.y,
         MfdGrenadeBox[i].lr.x,MfdGrenadeBox[i].lr.y);
   }

   gr_set_fcolor(BLACK);
   sprintf(buf,"-");
   ss_string("-",MfdGrenadeBox[0].ul.x+1,MfdGrenadeBox[0].ul.y+1);
   sprintf(buf,"+");
   ss_string(buf,MfdGrenadeBox[1].ul.x+1,MfdGrenadeBox[1].ul.y+1);

   return;
}
#endif // OLD_GRENADE_BUTTONS

// --------------------------------------------------------------------------
// mfd_item_expose()
//
// Draws an overlay of a drug molecule or whatever in the current slot.

#define NULL_ACTIVE 0xFF
static ubyte cat2active[MFD_INV_CATEGORIES] =
   {
      NULL_ACTIVE,
      ACTIVE_DRUG,
      ACTIVE_HARDWARE,
      ACTIVE_GRENADE,
      ACTIVE_CART,
      ACTIVE_WEAPON,
      ACTIVE_GENERAL,
      ACTIVE_COMBAT_SOFT,
      ACTIVE_DEFENSE_SOFT,
      ACTIVE_MISC_SOFT,
   };


#define TRASH_BUTTON REF_IMG_DiscardButton

#define NULL_REF MKREF(ID_NULL,0)

Ref smallstuffSpews[] = {
   NULL_REF,
   NULL_REF,
   NULL_REF,
   REF_STR_gearSpew0,
   NULL_REF,
   NULL_REF,
   NULL_REF,
   REF_STR_plotSpew0
};

void mfd_general_inv_expose(MFD*, ubyte, ObjID id, bool full)
{
   int triple;
   Ref spew=NULL_REF;

   if (id == OBJ_NULL)
   {
      draw_blank_mfd();
      return;
   }
   triple = ID2TRIP(id);
   mfd_item_micro_expose(full,triple);
   if (full && !(ObjProps[OPNUM(id)].flags & INVENTORY_GENERAL))
   {
      short x = (MFD_VIEW_WID-res_bm_width(TRASH_BUTTON))/2;
      short y = (MFD_VIEW_HGT-res_bm_height(TRASH_BUTTON))/2;
      draw_raw_resource_bm(TRASH_BUTTON,x,y);
      mfd_add_rect(x,y,x+res_bm_width(TRASH_BUTTON),y+res_bm_height(TRASH_BUTTON));
   }
   if(objs[id].obclass==CLASS_SMALLSTUFF)
      spew=smallstuffSpews[objs[id].subclass];
   if(spew!=NULL_REF)
      draw_mfd_item_spew(spew+objs[id].info.type,1);
}

bool mfd_general_inv_handler(MFD* m, uiEvent* ev, int row)
{
   ObjID id = player_struct.inventory[row];
   if (ev->type != UI_EVENT_MOUSE || !(ev->subtype & MOUSE_LDOWN)) return FALSE;
   if (!(ObjProps[OPNUM(id)].flags & INVENTORY_GENERAL))
   {
      LGPoint pos = MakePoint(ev->pos.x - m->rect.ul.x,ev->pos.y - m->rect.ul.y);
      short x = (MFD_VIEW_WID-res_bm_width(TRASH_BUTTON))/2;
      short y = (MFD_VIEW_HGT-res_bm_height(TRASH_BUTTON))/2;
      LGRect r;
      r.ul = r.lr = MakePoint(x,y);
      r.lr.x += res_bm_width(TRASH_BUTTON);
      r.lr.y += res_bm_height(TRASH_BUTTON);
      if (RECT_TEST_PT(&r,pos))
      {
         int i;
         obj_destroy(id);
         for (i = row+1; i < NUM_GENERAL_SLOTS; i++)
            player_struct.inventory[i-1] = player_struct.inventory[i];
         player_struct.inventory[NUM_GENERAL_SLOTS-1] = OBJ_NULL;
         drain_energy(1);
         mfd_notify_func(MFD_EMPTY_FUNC,MFD_ITEM_SLOT,TRUE,MFD_EMPTY,FALSE);
         chg_set_flg(INVENTORY_UPDATE);
      }

   }
   return TRUE;

}

#define STRINGS_PER_WARE (REF_STR_wareSpew1 - REF_STR_wareSpew0)
//#define SPEW_VERT_MARGIN 10
#define SPEW_VERT_MARGIN 2

void draw_mfd_item_spew(Ref id, int n)
{
   bool oldwrap = mfd_string_wrap;
   short w,h;
   short x,y;
   char buf[256];
   int i;

   gr_set_font((grs_font*)ResLock(MFD_FONT));
   buf[0] = '\0';
#ifdef CONCATENATE_ITEMSPEW
   for (i = 0; i < n; i ++,id++)
#else
   i=n-1; id+=i;
#endif
      get_string(id,buf+strlen(buf),sizeof(buf)-strlen(buf));
   wrap_text(buf,MFD_VIEW_WID-2);
   gr_string_size(buf,&w,&h);
   x = (MFD_VIEW_WID-w)/2;
   y = (MFD_VIEW_HGT-h-SPEW_VERT_MARGIN)/2 + SPEW_VERT_MARGIN;
   mfd_string_wrap = FALSE;
   mfd_full_draw_string(buf,x,y,GREEN_YELLOW_BASE,MFD_FONT,TRUE,TRUE);
   mfd_string_wrap = oldwrap;
   ResUnlock(MFD_FONT); 
}


void mfd_item_expose(MFD *m, ubyte control)
{
   ubyte lastclass, currclass, lasttype, currtype = MFD_INV_NOTYPE;
   bool FullRedraw = FALSE;
   int triple;

   currclass = MFDGetCurrItemClass(m->id);
   lastclass = MFDGetLastItemClass(m->id);

   if (cat2active[currclass] != NULL_ACTIVE)
      currtype = player_struct.actives[cat2active[currclass]];
   if (currtype == MFD_INV_NOTYPE) currclass = MFD_INV_NULL;
   lasttype  = MFDGetLastItemType(m->id);
   MFDSetCurrItemType(m->id,currtype);

   MFDSetLastItemClass(m->id,currclass);
   MFDSetLastItemType(m->id,MFDGetCurrItemType(m->id));

   if (control & MFD_EXPOSE) {

      if ((control & MFD_EXPOSE_FULL) ||
          (currclass != lastclass) || (currtype != lasttype))
         FullRedraw = TRUE;

      // If there is no currently selected member of the current class,
      // draw the blank screen thingie
      if (MFDGetCurrItemType(m->id) == EMPTY_WEAPON_SLOT) {
         mfd_expose_blank(m,control);
         return;
      }

      PUSH_CANVAS(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
      mfd_clear_rects();
 
      switch(currclass) {

         case MFD_INV_NULL:

            if (FullRedraw) { draw_blank_mfd();}
            break;

         case MFD_INV_DRUG:

            triple = get_triple_from_class_nth_item(CLASS_DRUG,currtype);

//KLC - chg for new art            mfd_item_micro_expose(FullRedraw, triple);
            mfd_item_micro_hires_expose(FullRedraw, triple);
            if (FullRedraw)
               draw_mfd_item_spew(REF_STR_drugSpew0 + currtype,1);
            draw_res_bm(REF_IMG_Apply,DRUG_BUTTON_X,DRUG_BUTTON_Y);
            mfd_add_rect(DRUG_BUTTON_X,DRUG_BUTTON_Y,DRUG_BUTTON_X+DRUG_BUTTON_W,MFD_VIEW_HGT); 
            break;

         case MFD_INV_GRENADE:

            triple = get_triple_from_class_nth_item(CLASS_GRENADE,currtype);
//KLC - chg for new art            mfd_item_micro_expose(FullRedraw, triple);
            mfd_item_micro_hires_expose(FullRedraw, triple);
            break;

         case MFD_INV_HARDWARE:

//            if (currtype == HARDWARE_DATA_READER) {
//               ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
//               ss_bitmap(&mfd_background,0,0);
//               mfd_draw_string("Video reader",5,10,WHITE,TRUE);
//               mfd_draw_string("Text reader",5,15,WHITE,TRUE);
//               mfd_draw_string("Email reader",5,20,WHITE,TRUE);
//            }
//            else 
            {
               extern bool is_passive_hardware(int n);
               int id;
               short x = HARDWARE_BUTTON_X,y = HARDWARE_BUTTON_Y;
               triple = get_triple_from_class_nth_item(CLASS_HARDWARE,currtype);

//KLC - chg for new art               mfd_item_micro_expose(FullRedraw, triple);
               mfd_item_micro_hires_expose(FullRedraw, triple);
               if (!is_passive_hardware(currtype))
               {
                  id = (player_struct.hardwarez_status[currtype] & WARE_ON) ? REF_IMG_Active : REF_IMG_Inactive;
                  draw_res_bm(id,x,y);
                  mfd_add_rect(x,y,x+HARDWARE_BUTTON_W,MFD_VIEW_HGT);
               }
               if (FullRedraw)
               {
                  uchar version = player_struct.hardwarez[currtype];
                  draw_mfd_item_spew(REF_STR_wareSpew0 + STRINGS_PER_WARE*currtype,version);
               }
            }

            break;

         case MFD_INV_AMMO:
		   mfd_ammo_expose(control);
            break;

         case MFD_INV_SOFT_COMBAT:
         case MFD_INV_SOFT_DEFENSE:
            triple = get_ware_triple(currclass-MFD_INV_SOFT_COMBAT+WARE_SOFT_COMBAT,currtype);
            mfd_item_micro_expose(FullRedraw,triple);
            break; 
         case MFD_INV_SOFT_MISC:
            // Monkey see, monkey do, monkey will destroy you.
            {
               extern bool is_oneshot_misc_software(int n);
               short x = HARDWARE_BUTTON_X,y = HARDWARE_BUTTON_Y;
               triple = get_ware_triple(currclass-MFD_INV_SOFT_COMBAT+WARE_SOFT_COMBAT,currtype);

               mfd_item_micro_expose(FullRedraw,triple);
               if (is_oneshot_misc_software(currtype))
               {
                  draw_res_bm(REF_IMG_Activate,x,y);
                  mfd_add_rect(x,y,x+HARDWARE_BUTTON_W,MFD_VIEW_HGT);
               }
            }
            break;
         case MFD_INV_GENINV:
            mfd_general_inv_expose(m,control,player_struct.inventory[currtype],FullRedraw);
            break;

         default:
            break;
      }

      POP_CANVAS();

      mfd_update_rects(m);
   }
   
   return;
}

// ---------------------------------------------------------------------------
// mfd_item_handler()
//
// Handle clicks to, for now, the +- grenade set-time boxes.

bool mfd_item_handler(MFD *m, uiEvent *e)
{
   bool retval = FALSE;
   uiMouseEvent *mickey;

   mickey = (uiMouseEvent *) e;
   if (!(mickey->action & MOUSE_LDOWN)) return retval;

   switch(MFDGetCurrItemClass(m->id)) {

      case MFD_INV_GRENADE:

#ifdef OLD_GRENADE_BUTTONS
      {
         int i;
         LGRect r[2];
         int triple;
         ubyte min, max;
         triple = get_triple_from_class_nth_item(CLASS_GRENADE,MFDGetCurrItemType(m->id));
         if (!(TRIP2SC(triple) == GRENADE_SUBCLASS_TIMED)) return retval;

         min = TimedGrenadeProps[SCTRIP(triple)].min_time_set;
         max = TimedGrenadeProps[SCTRIP(triple)].max_time_set;

         RECT_OFFSETTED_RECT(&MfdGrenadeBox[0], m->rect.ul, &(r[0]));
         RECT_OFFSETTED_RECT(&MfdGrenadeBox[1], m->rect.ul, &(r[1]));

         for (i = 0; i < 2; i++) {

            if (RECT_TEST_PT(&r[i], e->pos)) {

               if (i == 0) {                  
                  if (player_struct.grenades_time_setting[MFDGetCurrItemType(m->id)] > min)
                     player_struct.grenades_time_setting[MFDGetCurrItemType(m->id)]--;
               }
               else if (i == 1) {
                  if (player_struct.grenades_time_setting[MFDGetCurrItemType(m->id)] < max)
                     player_struct.grenades_time_setting[MFDGetCurrItemType(m->id)]++;
               }

               mfd_notify_func(MFD_ITEM_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
            }
         }
      }
#endif // OLD_GRENADE_BUTTONS
         break;
      case MFD_INV_HARDWARE:
         {
            LGRect r = { {HARDWARE_BUTTON_X, HARDWARE_BUTTON_Y},{HARDWARE_BUTTON_X + HARDWARE_BUTTON_W, HARDWARE_BUTTON_Y+HARDWARE_BUTTON_H}};
            RECT_OFFSETTED_RECT(&r,m->rect.ul,&r);
            if (RECT_TEST_PT(&r,e->pos))
            {
               use_ware(WARE_HARD,player_struct.actives[ACTIVE_HARDWARE]);
               mfd_notify_func(MFD_ITEM_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,FALSE);
               retval = TRUE;
            }
         }
         break;
      case MFD_INV_SOFT_MISC:
         {
            LGRect r = { {HARDWARE_BUTTON_X, HARDWARE_BUTTON_Y},{HARDWARE_BUTTON_X + HARDWARE_BUTTON_W, HARDWARE_BUTTON_Y+HARDWARE_BUTTON_H}};
            RECT_OFFSETTED_RECT(&r,m->rect.ul,&r);
            if (RECT_TEST_PT(&r,e->pos))
            {
               use_ware(WARE_SOFT_MISC,player_struct.actives[ACTIVE_MISC_SOFT]);
               mfd_notify_func(MFD_ITEM_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,FALSE);
               retval = TRUE;
            }
         }
         break;
      case MFD_INV_DRUG:
         {
            LGRect r = { {DRUG_BUTTON_X, DRUG_BUTTON_Y},{DRUG_BUTTON_X + DRUG_BUTTON_W, DRUG_BUTTON_Y+DRUG_BUTTON_H}};
            RECT_OFFSETTED_RECT(&r,m->rect.ul,&r);
            // Why is it that for wares, it's use_drug whereas for drugs it's drug_use?  Perhaps we'll never know. 
            if (RECT_TEST_PT(&r,e->pos))
            {
               drug_use(player_struct.actives[ACTIVE_DRUG]);
               mfd_notify_func(MFD_ITEM_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,FALSE);
               retval = TRUE;
            }
         }
         break;
      case MFD_INV_GENINV:
         mfd_general_inv_handler(m,e,player_struct.actives[ACTIVE_GENERAL]);
         break;

      case MFD_INV_AMMO:
         mfd_ammo_handler(m,e);
         break;      
   }

   return retval;
}

// ----------------------
// * ITEM MFD FOR LANTERN 
// ----------------------

#define LANTERN_BARRAY_X   2
#define LANTERN_BARRAY_WD  (MFD_VIEW_WID - 6)
#define LANTERN_BARRAY_Y  45

#define LANTERN_LAST_SETTING(mfd) (player_struct.mfd_func_data[MFD_LANTERN_FUNC][mfd])
#define LANTERN_LAST_VERSION(mfd) (player_struct.mfd_func_data[MFD_LANTERN_FUNC][NUM_MFDS+mfd])
#define LANTERN_LAST_STATE(mfd)   (player_struct.mfd_func_data[MFD_LANTERN_FUNC][2*NUM_MFDS+mfd])
#define LANTERN_BARRAY_IDX 0

extern bool muzzle_fire_light;

int energy_cost(int warenum);

bool mfd_lantern_button_handler(MFD*, LGPoint bttn, uiEvent* ev, void*)
{
   int n = CPTRIP(LANTERN_HARD_TRIPLE);
   int s = player_struct.hardwarez_status[n];
   void mfd_lantern_setting(int setting);

   if (bttn.x >= player_struct.hardwarez[n] || !(ev->subtype & MOUSE_LDOWN))
      return FALSE; // Version too high
   if (bttn.x == LAMP_SETTING(s))
   {
      use_ware(WARE_HARD,n);
      return TRUE;
   }
   mfd_lantern_setting(bttn.x);
   return TRUE;
}

void mfd_lantern_setting(int setting)
{
   int n = CPTRIP(LANTERN_HARD_TRIPLE);
   int s = player_struct.hardwarez_status[n];

   if (s & WARE_ON)
      set_player_energy_spend(player_struct.energy_spend - energy_cost(n));
   LAMP_SETTING_SET(s,setting);
   player_struct.hardwarez_status[n] = s;
   if (!muzzle_fire_light)
   {
      player_struct.light_value = s;
      lamp_set_vals();
   }
   if (player_struct.hardwarez_status[n] & WARE_ON)
      set_player_energy_spend(min(MAX_ENERGY,(int)player_struct.energy_spend + energy_cost(n)));
   mfd_notify_func(MFD_LANTERN_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
}

errtype mfd_lanternware_init(MFD_Func* f)
{
   int cnt = 0;
   LGPoint bsize;
   LGPoint bdims;                                     
   LGRect r;
   errtype err;
   bsize.x = res_bm_width(REF_IMG_LitLamp0);
   bsize.y = res_bm_height(REF_IMG_LitLamp0);
   bdims.x = LAMP_VERSIONS;
   bdims.y = 1;
   r.ul.x = LANTERN_BARRAY_X;
   r.ul.y = LANTERN_BARRAY_Y;
   r.lr.x = r.ul.x + LANTERN_BARRAY_WD;
   r.lr.y = r.ul.y + bsize.y;
   err = MFDBttnArrayInit(&f->handlers[cnt++],&r,bdims,bsize,mfd_lantern_button_handler,NULL);
   if (err != OK) return err;
   f->handler_count = cnt;
   return OK;
}

void mfd_lanternware_expose(MFD* mfd, ubyte control)
{
   int n,s,v;
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0) return;
   n = CPTRIP(LANTERN_HARD_TRIPLE);
   s = player_struct.hardwarez_status[n];
   v = player_struct.hardwarez[n];
   PUSH_CANVAS(pmfd_canvas);
   mfd_clear_rects();
   ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
   if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
        gr_bitmap(&mfd_background, 0, 0);
//KLC - chg for new art   mfd_item_micro_expose(full,LANTERN_HARD_TRIPLE);
   mfd_item_micro_hires_expose(full,LANTERN_HARD_TRIPLE);
   if (full)
      draw_mfd_item_spew(REF_STR_wareSpew0 + STRINGS_PER_WARE*n,v);
   if (full || LAMP_SETTING(s) != LANTERN_LAST_SETTING(mfd->id)
      || LANTERN_LAST_VERSION(mfd->id) != v
      || LANTERN_LAST_STATE(mfd->id) != s & WARE_ON)
   {
      int xjump = LANTERN_BARRAY_WD - res_bm_width(REF_IMG_LitLamp0);
      int i;
      for (i = 0; i < v; i++)
      {
         int lit = i == LAMP_SETTING(s) && (s & WARE_ON);
         int id = (lit) ?  REF_IMG_LitLamp0 : REF_IMG_UnlitLamp0;
         short x = LANTERN_BARRAY_X+i*xjump/(LAMP_VERSIONS-1);
         short y = LANTERN_BARRAY_Y;
         draw_raw_resource_bm(id+i,x,LANTERN_BARRAY_Y);
         if (i == LAMP_SETTING(s))
         {
            gr_set_fcolor(GREEN_BASE+2);
            ss_box(x-1,y-1,x+res_bm_width(id+i)+1,y+res_bm_height(id+i)+1);
         }
      }
      mfd_add_rect(LANTERN_BARRAY_X-2,LANTERN_BARRAY_Y-2,LANTERN_BARRAY_X+LANTERN_BARRAY_WD+2,MFD_VIEW_HGT);
      LANTERN_LAST_SETTING(mfd->id) = LAMP_SETTING(s);
      LANTERN_LAST_VERSION(mfd->id) = v;
      LANTERN_LAST_STATE(mfd->id)   = s & WARE_ON;
   }
   POP_CANVAS();
   mfd_update_rects(mfd);
}

// ----------------------
// * ITEM MFD FOR SHIELD
// ----------------------

#define SHIELD_BARRAY_X   2
#define SHIELD_BARRAY_WD  (MFD_VIEW_WID - 4)
#define SHIELD_BARRAY_Y  45

#define SHIELD_LAST_STATUS(mfd)  (player_struct.mfd_func_data[MFD_SHIELD_FUNC][mfd])
#define SHIELD_LAST_VERSION(mfd) (player_struct.mfd_func_data[MFD_SHIELD_FUNC][NUM_MFDS+mfd])
#define SHIELD_BARRAY_IDX 0

#define SHIELD_SETTINGS 3

void mfd_shield_setting(int setting);
bool mfd_shield_handler(MFD* m, uiEvent* e);
bool mfd_shield_button_handler(MFD* m, LGPoint bttn, uiEvent* ev, void* data);
errtype mfd_shield_init(MFD_Func* f);
void mfd_shieldware_expose(MFD* mfd, ubyte control);

bool mfd_shield_handler(MFD* m, uiEvent* e)
{
   bool retval = FALSE;
   LGPoint pos = e->pos;
   ubyte n = CPTRIP(SHIELD_HARD_TRIPLE);
   ubyte v = player_struct.hardwarez[n];
   if (v != SHIELD_VERSIONS) // are we at max version
      return FALSE;
   if (!(e->subtype & MOUSE_LDOWN))
      return FALSE;
   pos.x -= m->rect.ul.x - SHIELD_BARRAY_X;
   pos.y -= m->rect.ul.y - SHIELD_BARRAY_Y;
   if (pos.x > 0 && pos.x < SHIELD_BARRAY_WD && pos.y > 0)
   {
      use_ware(WARE_HARD,n);
      mfd_shield_setting(v-1);
      retval = TRUE;
   }
   return retval;
}

bool mfd_shield_button_handler(MFD*, LGPoint bttn, uiEvent* ev, void*)
{
   int n = CPTRIP(SHIELD_HARD_TRIPLE);
   int s = player_struct.hardwarez_status[n];

   if (bttn.x >= player_struct.hardwarez[n] || !(ev->subtype & MOUSE_LDOWN))
      return FALSE; // Version too high
   if (SHIELD_SETTING(s) == bttn.x)
   {
      use_ware(WARE_HARD,n);
      return TRUE;
   }
   mfd_shield_setting(bttn.x);
   return TRUE;
}

void mfd_shield_setting(int setting)
{
   extern void shield_set_absorb(void);
   int n = CPTRIP(SHIELD_HARD_TRIPLE);
   int s = player_struct.hardwarez_status[n];

   if (s & WARE_ON)
      set_player_energy_spend(player_struct.energy_spend - energy_cost(n));
   SHIELD_SETTING_SET(s,setting);
   player_struct.hardwarez_status[n] = s;
   if(s & WARE_ON)
   {
      shield_set_absorb();
      set_player_energy_spend(min(MAX_ENERGY,(int)player_struct.energy_spend + energy_cost(n)));
   }
   mfd_notify_func(MFD_SHIELD_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
}

errtype mfd_shield_init(MFD_Func* f)
{
   int cnt = 0;
   LGPoint bsize;
   LGPoint bdims;                                     
   LGRect r;
   errtype err;
   bsize.x = res_bm_width(REF_IMG_LitShield0);
   bsize.y = res_bm_height(REF_IMG_LitShield0);
   bdims.x = SHIELD_SETTINGS;
   bdims.y = 1;
   r.ul.x = SHIELD_BARRAY_X;
   r.ul.y = SHIELD_BARRAY_Y;
   r.lr.x = r.ul.x + SHIELD_BARRAY_WD;
   r.lr.y = r.ul.y + bsize.y;
   err = MFDBttnArrayInit(&f->handlers[cnt++],&r,bdims,bsize,mfd_shield_button_handler,NULL);
   if (err != OK) return err;
   f->handler_count = cnt;
   return OK;
}

void mfd_shieldware_expose(MFD* mfd, ubyte control)
{
   int n,s,v;
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0) return;
   n = CPTRIP(SHIELD_HARD_TRIPLE);
   s = player_struct.hardwarez_status[n];
   v = player_struct.hardwarez[n];
   PUSH_CANVAS(pmfd_canvas);
   ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
   mfd_clear_rects();
   if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);
//KLC - chg for new art   mfd_item_micro_expose(full,SHIELD_HARD_TRIPLE);
   mfd_item_micro_hires_expose(full,SHIELD_HARD_TRIPLE);
   if (full)
      draw_mfd_item_spew(REF_STR_wareSpew0 + STRINGS_PER_WARE*n,v);
   if (full || s != SHIELD_LAST_STATUS(mfd->id)
      || SHIELD_LAST_VERSION(mfd->id) != v)
   {
      int xjump = SHIELD_BARRAY_WD/SHIELD_SETTINGS;
      int i;
      if (v >= SHIELD_VERSIONS)
      {
         int id = (s & WARE_ON) ? REF_IMG_LitShieldSuper : REF_IMG_UnlitShieldSuper;
         draw_raw_resource_bm(id,SHIELD_BARRAY_X+(SHIELD_BARRAY_WD-res_bm_width(id))/2,SHIELD_BARRAY_Y);
         mfd_add_rect(SHIELD_BARRAY_X,SHIELD_BARRAY_Y,SHIELD_BARRAY_X+SHIELD_BARRAY_WD,MFD_VIEW_WID);
      }
      else for (i = 0; i < v; i++)
      {
         int id = (i == SHIELD_SETTING(s) && (s & WARE_ON)) ? REF_IMG_LitShield0 : REF_IMG_UnlitShield0;
         grs_bitmap* bm = lock_bitmap_from_ref(id+i);
         short x = SHIELD_BARRAY_X+i*xjump;
         short y = SHIELD_BARRAY_Y;
         mfd_draw_bitmap(bm,x,y);
         if (i == SHIELD_SETTING(s))
         {
            gr_set_fcolor(GREEN_BASE+2);
            ss_box(x-1,y-1,x+bm->w+1,y+bm->h+1);
         }
         mfd_add_rect(x-2,y-2,x+bm->w+2,y+bm->h+2);
         RefUnlock(id+i);
      }
      SHIELD_LAST_STATUS(mfd->id) = s;
      SHIELD_LAST_VERSION(mfd->id) = v;
   }
   POP_CANVAS();
   mfd_update_rects(mfd);
}

// ----------------------
// * ITEM MFD FOR MOTION WARE
// ----------------------

#define MOTION_BARRAY_WD  (3*MFD_VIEW_WID/4)
#define MOTION_BARRAY_X   ((MFD_VIEW_WID-MOTION_BARRAY_WD)/2)
#define MOTION_BARRAY_Y  48

#define MOTION_LAST_STATUS(mfd)  (player_struct.mfd_func_data[MFD_MOTION_FUNC][mfd])
#define MOTION_LAST_VERSION(mfd) (player_struct.mfd_func_data[MFD_MOTION_FUNC][NUM_MFDS+mfd])
#define MOTION_BARRAY_IDX 0

#define MOTION_SETTING        LAMP_SETTING
#define MOTION_SETTING_SET    LAMP_SETTING_SET

#define MOTION_BUTTONS 2

bool mfd_motion_button_handler(MFD* m, LGPoint bttn, uiEvent* ev, void* data);
errtype mfd_motion_init(MFD_Func* f);
void mfd_motionware_expose(MFD* mfd, ubyte control);

bool mfd_motion_button_handler(MFD*, LGPoint bttn, uiEvent* ev, void*)
{
   int n = CPTRIP(MOTION_HARD_TRIPLE);
   int s = player_struct.hardwarez_status[n];
//   if (bttn.x >= player_struct.hardwarez[n] || 
   if (!(ev->subtype & MOUSE_LDOWN))
      return FALSE; 
   if (MOTION_SETTING(s) == bttn.x)
   {
      use_ware(WARE_HARD,n);
      return TRUE;
   }
   if (s & WARE_ON)
      set_player_energy_spend(player_struct.energy_spend - energy_cost(n));
   MOTION_SETTING_SET(s,bttn.x);
   player_struct.hardwarez_status[n] = s;
   if (!(s & WARE_ON))
   {
//      use_ware(WARE_HARD,n);
   }
   else
   {
      motionware_mode = bttn.x + 1;
      set_player_energy_spend(min(MAX_ENERGY,(int)player_struct.energy_spend + energy_cost(n)));
   }
   mfd_notify_func(MFD_MOTION_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
   return TRUE;
}

errtype mfd_motion_init(MFD_Func* f)
{
   int cnt = 0;
   LGPoint bsize;
   LGPoint bdims;                                     
   LGRect r;
   errtype err;
   bsize.x = res_bm_width(REF_IMG_LitMotion0);
   bsize.y = res_bm_height(REF_IMG_LitMotion0);
   bdims.x = MOTION_BUTTONS;
   bdims.y = 1;
   r.ul.x = MOTION_BARRAY_X;
   r.ul.y = MOTION_BARRAY_Y;
   r.lr.x = r.ul.x + MOTION_BARRAY_WD;
   r.lr.y = r.ul.y + bsize.y;
   err = MFDBttnArrayInit(&f->handlers[cnt++],&r,bdims,bsize,mfd_motion_button_handler,NULL);
   if (err != OK) return err;
   f->handler_count = cnt;
   return OK;
}

void mfd_motionware_expose(MFD* mfd, ubyte control)
{
   int n,s,v;
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0) return;
   n = CPTRIP(MOTION_HARD_TRIPLE);
   s = player_struct.hardwarez_status[n];
   v = player_struct.hardwarez[n];
   PUSH_CANVAS(pmfd_canvas);
   mfd_clear_rects();
   ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
//KLC - chg for new art   mfd_item_micro_expose(full,MOTION_HARD_TRIPLE);
   mfd_item_micro_hires_expose(full,MOTION_HARD_TRIPLE);
   if (full)
      draw_mfd_item_spew(REF_STR_wareSpew0 + STRINGS_PER_WARE*n,v);
   if (full || s != MOTION_LAST_STATUS(mfd->id)
      || MOTION_LAST_VERSION(mfd->id) != v)
   {
      int xjump = MOTION_BARRAY_WD/MOTION_BUTTONS;
      int i;
      for (i = 0; i < min(v,MOTION_BUTTONS); i++)
      {
         int id = ((i == MOTION_SETTING(s) && (s & WARE_ON))) ? REF_IMG_LitMotion0 : REF_IMG_UnlitMotion0;
         grs_bitmap *bm = lock_bitmap_from_ref(id+i);
         short x = MOTION_BARRAY_X+i*xjump;
         short y = MOTION_BARRAY_Y;
         mfd_draw_bitmap(bm,x,y);
         if (i == MOTION_SETTING(s))
         {
            gr_set_fcolor(GREEN_BASE+2);
            ss_box(x-1,y-1,x+bm->w+1,y+bm->h+1);
         }
         mfd_add_rect(x-2,y-2,x+bm->w+2,y+bm->h+2);
         RefUnlock(id+i);
      }
      MOTION_LAST_STATUS(mfd->id) = s;
      MOTION_LAST_VERSION(mfd->id) = v;
   }
   POP_CANVAS();
   mfd_update_rects(mfd);
}

// -----------------
// TIMED GRENADE MFD
// -----------------

#define MFD_GRENADE_FUNC 10

#define GRENADE_TIME_UNIT 10 // fraction of second for each time second unit

#define GRENADE_SLIDER_X MFD_BEAM_RECT_X1
#define GRENADE_SLIDER_Y 51
#define GRENADE_SLIDER_W 70
#define GRENADE_SLIDER_H 5

#define GRENADE_SLIDER_IDX 0

#define GRENADE_MOUSE_CONSTRAINED  (player_struct.mfd_func_data[MFD_GRENADE_FUNC][7])

#define GRENADE_HIRES_CUTOFF 100

bool mfd_grenade_slider_handler(MFD* m,short val, uiEvent* ev, void* data);
bool mfd_grenade_handler(MFD* m, uiEvent* ev);
errtype mfd_grenade_init(MFD_Func* f);
void mfd_grenade_expose(MFD* mfd, ubyte control);

bool mfd_grenade_slider_handler(MFD* m,short val, uiEvent* ev, void*)
{
   uiMouseEvent* mev = (uiMouseEvent*)ev;
   int n = player_struct.actives[ACTIVE_GRENADE];
   int triple = nth_after_triple(MAKETRIP(CLASS_GRENADE,0,0),n);
   short min = TimedGrenadeProps[SCTRIP(triple)].min_time_set*GRENADE_TIME_UNIT;
   short max = TimedGrenadeProps[SCTRIP(triple)].max_time_set*GRENADE_TIME_UNIT;
   short width = GRENADE_SLIDER_W;
   short setting;

   if (max > 2*GRENADE_HIRES_CUTOFF)
   {
      width /= 2;
      if (val >= GRENADE_SLIDER_W/2)
         { val -= GRENADE_SLIDER_W/2; min = GRENADE_HIRES_CUTOFF; }
      else max = GRENADE_HIRES_CUTOFF;
   }
   setting = val*(max-min)/width + min;
   player_struct.grenades_time_setting[n] = setting;
   if (ev->subtype & MOUSE_LDOWN)
   {
      LGRect r = mfd_funcs[MFD_GRENADE_FUNC].handlers[GRENADE_SLIDER_IDX].r;
      int my=((uiMouseEvent*)ev)->pos.y;
      RECT_OFFSETTED_RECT(&r,m->rect.ul,&r);
      slider_cursor.hotspot.x=slider_cursor_bmap.w/2;
      slider_cursor.hotspot.y=(slider_cursor_bmap.h/2)+my-(r.ul.y+r.lr.y)/2;
      ui_mouse_constrain_xy(r.ul.x,my,r.lr.x-2,my);
 
      GRENADE_MOUSE_CONSTRAINED = m->id + 1;
      // Get our funky mfd-beam-phaser-setting cursor
#ifdef CURSOR_BACKUPS
   backup[20] = (uchar *)Malloc(f->bm.w * f->bm.h);
   LG_memcpy(backup[20],f->bm.bits,f->bm.w * f->bm.h);
      gr_init_bm(&backup_mfd_cursor,backup[14],BMT_FLAT8, 0, mfd_cursor.w,mfd_cursor.h);
#endif
      uiPushRegionCursor(MFD_REGION(m), &slider_cursor);
   }
   if ((mev->buttons & (1 << MOUSE_LBUTTON)) == 0)
   {
      uiCursorStack* cs;
      uiGetRegionCursorStack(MFD_REGION(m),&cs);
      uiPopCursorEvery(cs,&slider_cursor);
      
      if (GRENADE_MOUSE_CONSTRAINED)
      {
         mouse_unconstrain();
         GRENADE_MOUSE_CONSTRAINED = 0;
         mfd_notify_func(MFD_GRENADE_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,TRUE);
      }
   }
   mfd_notify_func(MFD_GRENADE_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,FALSE);
   return TRUE;
}


bool mfd_grenade_handler(MFD* m, uiEvent* ev)
{
   LGRect r = mfd_funcs[MFD_GRENADE_FUNC].handlers[GRENADE_SLIDER_IDX].r;
   RECT_MOVE(&r,m->rect.ul);
   if (!RECT_TEST_PT(&r,ev->pos))
   {
      uiCursorStack* cs;
      uiGetRegionCursorStack(MFD_REGION(m),&cs);
      uiPopCursorEvery(cs,&slider_cursor);
      mfd_notify_func(MFD_GRENADE_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,FALSE);
   }
   return FALSE;
}

        

errtype mfd_grenade_init(MFD_Func* f)
{
   int cnt = 0;
   errtype err;
   LGRect r = { { GRENADE_SLIDER_X, GRENADE_SLIDER_Y},
              { GRENADE_SLIDER_X + GRENADE_SLIDER_W,
                GRENADE_SLIDER_Y + GRENADE_SLIDER_H} };
   err = MFDSliderInit(&f->handlers[cnt++],&r,mfd_grenade_slider_handler,NULL);
   if (err != OK) return err;
   f->handler_count = cnt;
#ifdef CURSOR_BACKUPS
   backup[21] = (uchar *)Malloc(slider_bmap.w * slider_bmap.h);
   LG_memcpy(backup[21],slider_bmap.bits,slider_bmap.w * slider_bmap.h);
   gr_init_bm(&backup_slider_cursor,backup[21],BMT_FLAT8,0,slider_bmap.w,slider_bmap.h);
#endif
   return OK;
}

#define LAST_GRENADE(mfd) (player_struct.mfd_func_data[MFD_GRENADE_FUNC][mfd])
#define LAST_GRENADE_SETTING(mfd) (player_struct.mfd_func_data[MFD_GRENADE_FUNC][mfd+2])

#define GRENADE_SLIDER_BORDER MFD_BEAMWPN_STAT_BORDER
#define GRENADE_SLIDER_SETTING_COLOR MFD_BEAMWPN_STAT_CHARGE

#define TIME_TEXT_Y (GRENADE_SLIDER_Y - 8)
#define TIME_TEXT_LEN  128

void mfd_grenade_expose(MFD* mfd, ubyte control)
{
   MFD_Func* f = &mfd_funcs[MFD_GRENADE_FUNC];
   int n = player_struct.actives[ACTIVE_GRENADE];
   int triple = nth_after_triple(MAKETRIP(CLASS_GRENADE,0,0),n);
   bool full = (control & MFD_EXPOSE_FULL) || (n != LAST_GRENADE(mfd->id));
   short setting = player_struct.grenades_time_setting[n];

   if (control == 0)
   {

      uiCursorStack* cs;
      uiGetRegionCursorStack(MFD_REGION(mfd),&cs);
      uiPopCursorEvery(cs,&slider_cursor);

      if (GRENADE_MOUSE_CONSTRAINED == mfd->id + 1)
      {
         mouse_unconstrain();
         GRENADE_MOUSE_CONSTRAINED = 0;
      }
      return;
   }
   mfd_clear_rects();
   PUSH_CANVAS(pmfd_canvas);
   ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
   if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);
//KLC - chg for new art   mfd_item_micro_expose(full,triple);
   mfd_item_micro_hires_expose(full,triple);
   LAST_GRENADE(mfd->id) = n;
   if (full || LAST_GRENADE_SETTING(mfd->id) != setting)
   {
      char buf[TIME_TEXT_LEN];
      LGRect* r = &f->handlers[GRENADE_SLIDER_IDX].r;
      short min = TimedGrenadeProps[SCTRIP(triple)].min_time_set*GRENADE_TIME_UNIT;
      short max = TimedGrenadeProps[SCTRIP(triple)].max_time_set*GRENADE_TIME_UNIT;
      short width = GRENADE_SLIDER_W;
      short base = 0;
      short x;
      if (max > 2*GRENADE_HIRES_CUTOFF)
      {
         width /= 2;
         if (setting < GRENADE_HIRES_CUTOFF)
            max = GRENADE_HIRES_CUTOFF;
         else
         {
            base = GRENADE_SLIDER_W/2;
            min = GRENADE_HIRES_CUTOFF;
         }
      }
      x = (setting - min)*width/(max-min) + base;
      gr_set_fcolor(GRENADE_SLIDER_BORDER);
      ss_box(r->ul.x-1,r->ul.y-1,r->lr.x,r->lr.y);
      mfd_add_rect(r->ul.x-1,r->ul.y-1,r->lr.x,r->lr.y+1);
      gr_set_fcolor(GRENADE_SLIDER_SETTING_COLOR);
      if (!GRENADE_MOUSE_CONSTRAINED)
         draw_raw_resource_bm(REF_IMG_BeamSetting,r->ul.x + x-3, r->ul.y - 1);
      
      get_string(REF_STR_TimeSetting,buf,TIME_TEXT_LEN);
      numtostring(setting/10, buf+strlen(buf));   // itoa(setting/10,buf+strlen(buf),10);
      strcat(buf,".");
      numtostring(setting%10, buf+strlen(buf));   // itoa(setting%10,buf+strlen(buf),10);
      {
         LGRect r = { {GRENADE_SLIDER_X, TIME_TEXT_Y} ,{ MFD_VIEW_WID, GRENADE_SLIDER_Y-1} };
         mfd_partial_clear(&r);
      }
      mfd_draw_string(buf,GRENADE_SLIDER_X,TIME_TEXT_Y,GRENADE_SLIDER_BORDER,TRUE);
      LAST_GRENADE_SETTING(mfd->id) = setting;
   }
   POP_CANVAS();
   mfd_update_rects(mfd);

}


// ------------------
// * THE BIO WARE MFD
// ------------------


#define BIO_TEXT_X 29

// ---------------------------------------------------------------------------
// mfd_bioware_expose()
//
// This is the bioware, activated in the info window.  It displays stats.
// As of now, it's pretty sketchy.


#define LAST_HP(mfd) (mfd_fdata[MFD_BIOWARE_FUNC][4*mfd])
#define LAST_FATIGUE(mfd) (mfd_fdata[MFD_BIOWARE_FUNC][1+4*mfd])
#define LAST_DRUGBITS(mfd) (*(ushort*)&mfd_fdata[MFD_BIOWARE_FUNC][2+4*mfd])

// this is stolen from gamesys.c
#define MAX_FATIGUE 10000
#define MAX_HP UCHAR_MAX

#define BIO_DRUG_UP 1           // experincing normal effects
#define BIO_DRUG_DOWN 2         // experiencing after effects. 
#define BIO_DRUG_CLEAN 0

#define BITS_PER_DRUG 2


void mfd_bioware_expose(MFD *m, ubyte control)
{
   bool full = control & MFD_EXPOSE_FULL;
   int i, y = 2, triple;
   char buf2[12];
   LGRect r;

   r.ul.x = BIO_TEXT_X; r.ul.y = 20; r.lr.x = MFD_VIEW_WID; r.lr.y = MFD_VIEW_HGT;

   // turn off the bioware if there's no exposed mfd.  
   {
      extern WARE HardWare[NUM_HARDWAREZ];
      int i;
      bool on = full || control & MFD_EXPOSE;
      for (i = 0; i < NUM_MFDS; i++)
      {
         ubyte slot = player_struct.mfd_current_slots[i];
         ubyte func = mfd_get_func(i,slot);
         if (func == MFD_BIOWARE_FUNC && (control != 0 || m->id != i))
            on = TRUE;
      }
      if (on == !(player_struct.hardwarez_status[HARDWARE_BIOWARE] & WARE_ON))
      {
         use_ware(WARE_HARD,HARDWARE_BIOWARE);
      }
    }

   if (control & MFD_EXPOSE)
   {
      char *s;
      char pct[] = "%";
      short x;
      ubyte v = player_struct.hardwarez[HARDWARE_BIOWARE];
      int ref = MKREF(RES_mfdArtOverlays,MFD_ART_HUMAN);
      bool stam = player_struct.drug_status[CPTRIP(STAMINA_DRUG_TRIPLE)] > 0;

      PUSH_CANVAS(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
      mfd_clear_rects();
      if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);
      gr_set_font((grs_font*)ResLock(MFD_FONT));

      if (full)
      {
         draw_res_bm(ref, 0, 0);
         mfd_add_rect(0,0,res_bm_width(ref),res_bm_height(ref));
      }
 #ifdef BIOWARE_TITLE
      // Title
      if (full)
      {
         s = get_temp_string(REF_STR_BiowareTitle);
         mfd_draw_string(s, BIO_TEXT_X, y, GREEN_YELLOW_BASE, TRUE);
      }
      y += Y_STEP;
 #endif

      // Health
      if (full || (LAST_HP(m->id) != player_struct.hit_points))
      {
         uint hp; LGRect rest;
         s = get_temp_string(REF_STR_BiowareHealth);
         x = BIO_TEXT_X + gr_string_width(s);
         if (full)
            mfd_draw_string(s, BIO_TEXT_X, y, GREEN_YELLOW_BASE, TRUE);
         hp=(100*player_struct.hit_points+(MAX_HP/2))/MAX_HP;
         if(hp==0 && player_struct.hit_points>0) hp=1;
         numtostring(hp, buf2);    // itoa(hp,buf2,10);

         // hack to save space in display.  Use "I" for "1", knowing
         // that "I" is narrower.
         string_replace_char(buf2,'1','I');

         mfd_draw_string(buf2, x, y, GREEN_YELLOW_BASE, TRUE);
         x += gr_string_width(buf2);
         mfd_draw_string(pct,x,y,GREEN_YELLOW_BASE, TRUE);
         x += gr_string_width(pct);
         rest.ul.x=x; rest.ul.y=y;
         rest.lr.x=MFD_VIEW_WID; rest.lr.y=y+Y_STEP;
         mfd_partial_clear(&rest);
         LAST_HP(m->id) = player_struct.hit_points;
      }
      y += Y_STEP;

      // Fatigue
      if (full || stam || (LAST_FATIGUE(m->id) != 100*(uint)player_struct.fatigue/MAX_FATIGUE))
      {
         LGRect rest;
         ubyte f = 100*(uint)player_struct.fatigue/MAX_FATIGUE;
         s = get_temp_string(REF_STR_BiowareFatigue);
         x = BIO_TEXT_X + gr_string_width(s);
         if (full)
            mfd_draw_string(s, BIO_TEXT_X, y, GREEN_YELLOW_BASE, TRUE);
         if (stam)
         {
            strcpy(buf2,"--");
            mfd_draw_string(buf2, x , y, GREEN_YELLOW_BASE,TRUE);
            x+= gr_string_width(buf2);
         }
         else
         {
            numtostring(f, buf2);    // itoa(f,buf2,10);

            // hack to save space in display.  Use "I" for "1", knowing
            // that "I" is narrower.
            string_replace_char(buf2,'1','I');
            mfd_draw_string(buf2, x, y, GREEN_YELLOW_BASE, TRUE);
            x += gr_string_width(buf2);
            mfd_draw_string(pct,x,y,GREEN_YELLOW_BASE, TRUE);
            x += gr_string_width(pct);
         }
      

         rest.ul.x=x; rest.ul.y=y;
         rest.lr.x=MFD_VIEW_WID; rest.lr.y=y+Y_STEP;
         mfd_partial_clear(&rest);
         LAST_FATIGUE(m->id) = f;
      }
      y += Y_STEP;

      if (v > 1)
      {
         ushort drugbits = 0;
         for (i = 0; i < NUM_DRUGS; i++)
         {
            if (player_struct.drug_status[i] > 0)
               drugbits |= (BIO_DRUG_UP << (i*BITS_PER_DRUG));
            if (player_struct.drug_status[i] < 0)
               drugbits |= (BIO_DRUG_DOWN << (i*BITS_PER_DRUG));
         }

         if (full || drugbits != LAST_DRUGBITS(m->id))
         {
            short savey = y;
            LAST_DRUGBITS(m->id) = drugbits;
            for (i = 0; i < NUM_DRUGS; i++, drugbits >>= BITS_PER_DRUG)
            {
               ushort drugged = drugbits & ((1 << BITS_PER_DRUG) - 1);
               if (drugged != BIO_DRUG_CLEAN)
               {
                  ubyte color = (drugged == BIO_DRUG_UP) ? GREEN_BASE + 3 : GOOD_RED;
                  triple = drug2triple(i);
                  get_object_short_name(triple,buf2,sizeof(buf2));
                  mfd_draw_string(buf2,BIO_TEXT_X,y,color,TRUE);
                  y+=Y_STEP;
               }
            }
            mfd_add_rect(BIO_TEXT_X,savey,MFD_VIEW_WID,savey+Y_STEP*NUM_DRUGS);
         }
      }
      if (full)
         mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      ResUnlock(MFD_FONT);

      POP_CANVAS();
      mfd_update_rects(m);
   }


   return;
}

// -------------------
// * THE ANIMATION MFD
// -------------------

// ---------------------------------------------------------------------------
// mfd_anim_init()
//
// Open the space station resource file for animation

errtype mfd_anim_init()
{
#ifdef USING_DORKY_BROKEN_ANIM   
   if (ResOpenFile("space4.res") < 0) critical_error(CRITERR_RES|5);
#endif

   return OK;
}

// ---------------------------------------------------------------------------
// mfd_anim_expose()
//
// Strictly temporary code.  Starts or stops the space station animation.

void mfd_anim_expose(MFD *m, ubyte control)
{
#ifndef NO_DUMMIES
   MFD *dummy; ubyte dummy2; dummy = m; dummy2 = control;
#endif   
#ifdef USING_DORKY_BROKEN_ANIM
   static bool AnimOn[2];
   static ActAnim *anim[2];

   if (control & MFD_EXPOSE) {
      
      gr_set_fcolor((long)BLACK);
      ss_rect(m->rect.ul.x, m->rect.ul.y, m->rect.lr.x, m->rect.lr.y); 

      anim[m->id] = AnimPlayRegion(REF_ANIM_space4, &(m->reg), m->rect.ul, 0);
      chg_set_sta(ANIM_UPDATE);
      AnimOn[m->id] = TRUE;
   }
   else {

      AnimKill((anim[m->id]));
      AnimOn[m->id] = FALSE;
      if ((AnimOn[0] == FALSE) && (AnimOn[1] == FALSE))
         chg_unset_sta(ANIM_UPDATE);
   }
#endif
   return;
}

// SHODAN!!
// Note this expects all appropriate 2d preparation to already be done!!

errtype draw_shodan_influence(MFD *mfd, uchar amt);

errtype draw_shodan_influence(MFD *, uchar amt)
{
   char			*s = get_temp_string(SHODAN_FAILURE_STRING);
   grs_bitmap	bm;
   bm.bits = NULL;

   amt = min(NUM_SHODAN_MUGS -1,amt >> SHODAN_INTERVAL_SHIFT);
//KLC - chg for new art   draw_raw_res_bm_extract(REF_IMG_EmailMugShotBase+FIRST_SHODAN_MUG +amt,0,0);
   extract_temp_res_bitmap(&bm, REF_IMG_EmailMugShotBase+FIRST_SHODAN_MUG +amt);
   gr_bitmap(&bm, 0, 0);

   gr_set_font((grs_font*)ResLock(MFD_FONT));
   wrap_text(s,MFD_VIEW_WID);
   mfd_draw_string(s,2,2,SHODAN_COLOR,TRUE);
   ResUnlock(MFD_FONT);
   unwrap_text(s);

   mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
   return(OK);
}

// ELEVATOR PANEL MFD
// ------------------


// NOMENCLATURE: "level" refers to the internal, unique, game-system
// number for a level.  "floor" refers to the in-game floor number for
// a floor.  
 
#define NUM_ELEV_LVLS 15
#define NUM_ELEVATOR_BUTTONS 12
#define ELEV_BTTN_ROWS 4
#define ELEV_BTTN_COLS ((NUM_ELEVATOR_BUTTONS + ELEV_BTTN_ROWS - 1)/ELEV_BTTN_ROWS)
#define ELEV_BTTNS_X 11
#define ELEV_BTTNS_WD (MFD_VIEW_WID - 20)
#define ELEV_BTTNS_Y 21
#define ELEV_BTTNS_HT (MFD_VIEW_HGT - ELEV_BTTNS_Y - 2)  
#define ELEV_BTTN_HT 8 
#define ELEV_BTTN_WD 11 
#define ELEV_STATUS_Y 3
#define ELEV_STATUS_X 50

#define ELEV_STATUS_FONT RES_mediumLEDFont
#define ELEV_STATUS_COLOR (GOOD_RED)


typedef struct _elev_data
{
   ushort shownlvls;  // level shown on the button panel (bitmask)
   ushort reachlvls;  // level actually reachable        (bitmask)
   struct _mfd_specific
   {
      ubyte currlev: 4; // current level shown
      ubyte selected: 4; // currently selected button
   } stat,mfd_last[NUM_MFDS];
} elev_data_type;

uchar curr_elev_special = 0;

errtype mfd_elevator_setlev(MFD *mfd, short lev, elev_data_type *elev_data);
bool mfd_elevator_button_handler(MFD* mfd, LGPoint bttn, uiEvent* ev, void* data);
errtype mfd_elevator_init(MFD_Func* f);
void mfd_setup_elevator(ushort levmask, ushort reachmask, ushort curlevel, uchar special);
char *level_to_floor(int lev_num, char *buf);
void mfd_elevator_expose(MFD* mfd, ubyte control);

errtype mfd_elevator_setlev(MFD *mfd, short lev, elev_data_type *elev_data)
{
   elev_data->stat.currlev = lev;
   mfd_notify_func(MFD_ELEV_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, TRUE);
   mfd_update_current_slot(mfd->id,MFD_CHANGEBIT_FULL,0);
   return(OK);
}

bool mfd_elevator_button_handler(MFD* mfd, LGPoint bttn, uiEvent* ev, void*)
{
   elev_data_type* elev_data = (elev_data_type*)&player_struct.mfd_func_data[MFD_ELEV_FUNC][0];
   int b = bttn.x*ELEV_BTTN_ROWS+bttn.y;
   int c;
   ubyte l;
   ubyte reachl;
   ushort bit;
   uiMouseEvent* mort=(uiMouseEvent*)ev;

   if(!(mort->action & MOUSE_LDOWN))
      return FALSE;

   // If SHODAN has defeated us, indicate this for our expose func   
   if (curr_elev_special)
   {
      elev_data->mfd_last[mfd->id].currlev = 0xF;
      mfd_notify_func(MFD_ELEV_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, TRUE);
      mfd_update_current_slot(mfd->id,MFD_CHANGEBIT_FULL,0);
      return(OK);
   }

   for (c = 0,reachl = 0,l = 0,bit =1; l < NUM_ELEV_LVLS; l++,bit = bit << 1)
   {
      if (elev_data->reachlvls & bit)
         reachl++;
      if (elev_data->shownlvls & bit)
      {
         c++;
         if (c > b) break;
      }
   }
   if (l >= NUM_ELEV_LVLS) return TRUE;
   elev_data->stat.selected = b;
#ifdef PLAYTEST
   mprintf("Pushing button %d\n",b);
#endif
   if (!(bit & elev_data->reachlvls))
   {
      string_message_info(REF_STR_ElevatorNoMove);
#ifdef PLAYTEST
      mprintf ("Can't get to that level\n");
#endif
   }
   else
   {
      if (me_bits_music(MAP_GET_XY(PLAYER_BIN_X,PLAYER_BIN_Y)) != ELEVATOR_ZONE)
         string_message_info(REF_STR_UseTooFar); 
      else
      {
         int oldlev;
         oldlev = elev_data->stat.currlev;
         mfd_elevator_setlev(mfd, l, elev_data);
         if (!elevator_use(l, reachl-1))
            mfd_elevator_setlev(mfd, oldlev, elev_data);
      }
   }
   return TRUE;
}
 
#define TEST_ELEVPANEL

errtype mfd_elevator_init(MFD_Func* f)
{
   int cnt = 0;
   errtype err;
   LGPoint bsize = { ELEV_BTTN_WD, ELEV_BTTN_HT };
   LGPoint bdims = { ELEV_BTTN_COLS, ELEV_BTTN_ROWS} ;
   LGRect r = { { ELEV_BTTNS_X, ELEV_BTTNS_Y},            
              { ELEV_BTTNS_X + ELEV_BTTNS_WD, ELEV_BTTNS_Y + ELEV_BTTNS_HT } }; 
   err = MFDBttnArrayInit(&f->handlers[cnt++],&r,bdims,bsize,mfd_elevator_button_handler,NULL);
   if (err != OK) return err;
   f->handler_count = cnt;
#ifdef TEST_ELEVPANEL
   {
      elev_data_type* elev_data = (elev_data_type*)&player_struct.mfd_func_data[MFD_ELEV_FUNC][0];
      elev_data->shownlvls = 0xFFF;
      elev_data->reachlvls = 0xF0F;
   }
#endif 
   return OK;
}


// Does every thing but set the slot..
void mfd_setup_elevator(ushort levmask, ushort reachmask, ushort curlevel, uchar special)
{
   elev_data_type* elev_data = (elev_data_type*)&player_struct.mfd_func_data[MFD_ELEV_FUNC][0];
   elev_data->shownlvls = levmask;
   elev_data->reachlvls = reachmask;
   elev_data->stat.currlev = curlevel;
   elev_data->stat.selected = 0xFF;
   curr_elev_special = special;
   mfd_notify_func(MFD_ELEV_FUNC, MFD_INFO_SLOT, TRUE, MFD_ACTIVE, TRUE);

}

char *level_to_floor(int lev_num, char *buf)
{
   int bpos=0;
   char grov_init=toupper(get_temp_string(REF_STR_GroveWord)[0]);

   switch (lev_num)
   {
      case 0: buf[bpos++]=toupper(get_temp_string(REF_STR_ReactorWord)[0]); break;
      case 11: buf[bpos++]=grov_init; buf[bpos++]='4'; break;
      case 12: buf[bpos++]=grov_init; buf[bpos++]='1'; break;
      case 13: buf[bpos++]=grov_init; buf[bpos++]='2'; break;
      default:
         if (lev_num>=10)
         {
            buf[bpos++]='0'+((lev_num/10)%10);
            buf[bpos++]='0'+(lev_num%10);
         }
         else
            buf[bpos++]='0'+lev_num;
         break;
   }
   if (bpos!=0) buf[bpos]='\0';  // add trailing stop
   return(buf);
}

#define NUMBER_BUFSZ 3



// if we null your panel_ref, return the value
// before we nulled it.  Otherwise, return NULL.
//
ObjID panel_ref_unexpose(int mfdid, int func)
{
   bool found = FALSE;
   int id = NUM_MFDS;
   ObjID pr=player_struct.panel_ref;

   while (mfd_yield_func(func,&id))
      if (id != mfdid)
         return OBJ_NULL;
      else
         found = TRUE;

   if (found)
      check_panel_ref(TRUE);
   else player_struct.panel_ref = OBJ_NULL;
   return pr;
}


void mfd_elevator_expose(MFD* mfd, ubyte control)
{
   elev_data_type* elev_data = (elev_data_type*)&player_struct.mfd_func_data[MFD_ELEV_FUNC][0];
   char buf[NUMBER_BUFSZ];
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0) 
   {
      panel_ref_unexpose(mfd->id,MFD_ELEV_FUNC);
      return;
   }
   mfd_clear_rects();
   PUSH_CANVAS(pmfd_canvas);
   if (full) mfd_clear_view();
   if (curr_elev_special == 0)
      elev_data->mfd_last[mfd->id].currlev = 0;
   if (elev_data->mfd_last[mfd->id].currlev == 0xF)
      draw_shodan_influence(mfd, curr_elev_special);
   else
   {
      if (full || elev_data->mfd_last[mfd->id].currlev != elev_data->stat.currlev)
      {
         short w,h;
         int lev = elev_data->stat.currlev;
         gr_set_font((grs_font*)ResLock(ELEV_STATUS_FONT));
         level_to_floor(lev, buf);
         gr_string_size(buf,&w,&h);
         mfd_draw_font_string(buf,ELEV_STATUS_X-w,ELEV_STATUS_Y,ELEV_STATUS_COLOR,ELEV_STATUS_FONT,TRUE);
         elev_data->mfd_last[mfd->id].currlev = lev;
         ResUnlock(ELEV_STATUS_FONT);
      }
      if (full || elev_data->mfd_last[mfd->id].selected != elev_data->stat.selected)
      {
         int i;
         int l; 
         short w,h;
//         LGPoint bstep = { ELEV_BTTNS_WD/ELEV_BTTN_COLS,
//                        ELEV_BTTNS_HT/ELEV_BTTN_ROWS };
         gr_set_font((grs_font*)ResLock(MFD_FONT)); 
         for (i = 0,l=0; i < NUM_ELEVATOR_BUTTONS; i++)
         {
            ubyte clr;
            LGPoint bttn;

            bttn.x = ELEV_BTTNS_X + (i/ELEV_BTTN_ROWS)*(ELEV_BTTNS_WD-ELEV_BTTN_WD)/(ELEV_BTTN_COLS-1);
            bttn.y = ELEV_BTTNS_Y + (i%ELEV_BTTN_ROWS)*(ELEV_BTTNS_HT-ELEV_BTTN_HT)/(ELEV_BTTN_ROWS-1);

            if (i > 0) l++;
            for (; l < NUM_ELEV_LVLS;l++)
               if ((1 << l) & elev_data->shownlvls)
                  break;
            if (l >= NUM_ELEV_LVLS) break;
            if (!(elev_data->reachlvls & (1 << l)))
               clr = UNAVAILABLE_ITEM_COLOR;
            else if (i == elev_data->stat.selected)
               clr = SELECTED_ITEM_COLOR;
            else
               clr = ITEM_COLOR;
            gr_set_fcolor((long)clr);
            ss_box(bttn.x,bttn.y,bttn.x+ELEV_BTTN_WD,bttn.y+ELEV_BTTN_HT);
            gr_set_fcolor((long)ITEM_COLOR+2);
            ss_box(bttn.x-1,bttn.y-1,bttn.x+ELEV_BTTN_WD+1,bttn.y+ELEV_BTTN_HT+1);
            level_to_floor(l, buf);
            gr_string_size(buf,&w,&h);
            mfd_draw_string(buf,bttn.x+(ELEV_BTTN_WD-w)/2+1,bttn.y+1,clr,TRUE);
         }
         elev_data->mfd_last[mfd->id].selected = elev_data->stat.selected;
         mfd_add_rect(ELEV_BTTNS_X,ELEV_BTTNS_Y,ELEV_BTTNS_X+ELEV_BTTNS_WD,ELEV_BTTNS_Y+ELEV_BTTNS_HT);
         ResUnlock(MFD_FONT);
      }
   }
   POP_CANVAS();
   mfd_update_rects(mfd);
}

// ------------------
// KEYPAD MFD
// ------------------
 
#define NUM_KEYPAD_BUTTONS 12
#define KEYPAD_BTTN_ROWS 4
#define KEYPAD_BTTN_COLS  3
#define KEYPAD_X_MARGIN 0
#define KEYPAD_Y_MARGIN 2
#define KEYPAD_BTTNS_X 19
#define KEYPAD_BTTNS_WD (MFD_VIEW_WID-(2*KEYPAD_BTTNS_X)-1-KEYPAD_X_MARGIN)
//#define KEYPAD_BTTNS_WD (MFD_VIEW_WID - (3* KEYPAD_BTTNS_X))
#define KEYPAD_BTTNS_Y 20
#define KEYPAD_BTTNS_HT (MFD_VIEW_HGT - KEYPAD_BTTNS_Y - KEYPAD_Y_MARGIN - 1)  
#define KEYPAD_BTTN_HT 8 
#define KEYPAD_BTTN_WD 11 
#define KEYPAD_STATUS_Y 3
#define KEYPAD_STATUS_X 60

#define MAX_KEYPAD_DIGITS  3

#define KEYPAD_STATUS_FONT RES_mediumLEDFont
#define KEYPAD_STATUS_COLOR (GOOD_RED)


Boolean	gKeypadOverride = FALSE;			// When this is true, don't move the player.

typedef struct _keypad_data
{
   uchar curr_digit;
   uchar last_digit;
   uchar digits[MAX_KEYPAD_DIGITS];
   uchar special;
} keypad_data_type;

uchar keypad_num(int b);
char *keypad_name(int b, char *buf);
char *mfd_keypad_assemble(keypad_data_type *keypad_data, char *buf);
errtype mfd_keypad_input(MFD *m, char b_num);
bool keypad_hotkey_func(short keycode, ulong context, void* data);
void install_keypad_hotkeys(void);
bool mfd_keypad_handler(MFD* m, uiEvent* ev);
bool mfd_keypad_button_handler(MFD* mfd, LGPoint bttn, uiEvent* ev, void* data);
errtype mfd_keypad_init(MFD_Func* f);
void mfd_keypad_expose(MFD* mfd, ubyte control);

uchar keypad_num(int b)
{
#ifdef KEYPAD_NUM_CASE
   uchar retval;
   // as Doug points out, this case statement is expressed algorithmically,
   // but hey, this is already done, easier to modify and maybe even easier
   // to understand.
   switch(b)
   {
      case 0:  retval = 1; break;
      case 1:  retval = 4; break;
      case 2:  retval = 7; break;
      case 3:  retval = 10; break;
      case 4:  retval = 2; break;
      case 5:  retval = 5; break;
      case 6:  retval = 8; break;
      case 7:  retval = 0; break;
      case 8: retval = 3; break;
      case 9: retval = 6; break;
      case 10: retval = 9; break;
      case 11: retval = 11; break;
   }
   return(retval);
#endif

   static uchar retval[]={1,4,7,10,2,5,8,0,3,6,9,11};

   return(retval[b]);
}

// note that the b in keypad_name is already converted from keypad_num
char *keypad_name(int b, char *buf)
{
   switch(b)
   {
      case 10: strcpy(buf, "-"); break;
      case 11: strcpy(buf, "C"); break;
      default:
         numtostring(b, buf);    // itoa(b, buf, 10);
         break;
   }
   return(buf);
}

char *mfd_keypad_assemble(keypad_data_type *keypad_data, char *buf)
{
   char tmp[5];
   int i;
   strcpy(buf, "");
   for (i=0; i < keypad_data->curr_digit; i++)
   {
      strcat(buf, keypad_name(keypad_data->digits[i],tmp));
   }
   return(buf);
}


errtype mfd_keypad_input(MFD *, char b_num)
{
   keypad_data_type* keypad_data = (keypad_data_type*)&player_struct.mfd_func_data[MFD_KEYPAD_FUNC][0];
   extern errtype keypad_trigger(ObjID id, uchar digits[MAX_KEYPAD_DIGITS]);

   switch (b_num)
   {
      case 10:
         if (keypad_data->curr_digit != 0)
            keypad_data->curr_digit--;
         break;
      case 11:
         keypad_data->curr_digit = 0;
         break;
      default:
         if (keypad_data->curr_digit == MAX_KEYPAD_DIGITS)
            mfd_setup_keypad(keypad_data->special);
         keypad_data->digits[keypad_data->curr_digit] = b_num;
         keypad_data->curr_digit++;
         break;
   }
   play_digi_fx_obj(SFX_MFD_KEYPAD, 1,player_struct.panel_ref);
   if (keypad_data->special==0 && keypad_data->curr_digit == MAX_KEYPAD_DIGITS)
   {
      keypad_trigger(player_struct.panel_ref, keypad_data->digits);
   }
   mfd_notify_func(MFD_KEYPAD_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, TRUE);
   return(OK);
}

bool keypad_hotkey_func(short keycode, ulong, void*)
{
   extern MFD mfd[];
   uchar digit = kb2ascii(keycode) - '0';
   int m = NUM_MFDS;
   while(mfd_yield_func(MFD_KEYPAD_FUNC,&m))
   {
      mfd_keypad_input(&mfd[m],digit);
      return TRUE;
   }
   return FALSE;
}

void install_keypad_hotkeys(void)
{
   int i;
   for (i = 0; i < 10 ; i++)
//KLC      hotkey_add(('0'+i)|KB_FLAG_DOWN|KB_FLAG_2ND, DEMO_CONTEXT, keypad_hotkey_func, NULL);
      hotkey_add(('0'+i)|KB_FLAG_DOWN, DEMO_CONTEXT, keypad_hotkey_func, NULL);
}


bool mfd_keypad_handler(MFD* m, uiEvent* ev)
{
   bool retval = FALSE;
   char n;
   uiCookedKeyEvent *e = (uiCookedKeyEvent *)ev;

   if (e->type != UI_EVENT_KBD_COOKED)
      return(FALSE);
   if (!(e->code & KB_FLAG_DOWN))
      return(FALSE);
   n = (e->code & 0xFF) - '0';
   if ((n < 0) || (n > 9))
      return(FALSE);
   mfd_keypad_input(m, n);
   return(FALSE);
}

bool mfd_keypad_button_handler(MFD* mfd, LGPoint bttn, uiEvent* ev, void*)
{
   int b = bttn.x*KEYPAD_BTTN_ROWS+bttn.y;

   // Filter out anything that isn't a left-click down at all
   if ((ev->subtype&(MOUSE_LDOWN|UI_MOUSE_LDOUBLE))==0)
      return(FALSE);
   mfd_keypad_input(mfd, keypad_num(b));
   return TRUE;
}
 
errtype mfd_keypad_init(MFD_Func* f)
{
   int cnt = 0;
   errtype err;
   LGPoint bsize = { KEYPAD_BTTN_WD, KEYPAD_BTTN_HT };
   LGPoint bdims = { KEYPAD_BTTN_COLS, KEYPAD_BTTN_ROWS} ;
   LGRect r = { { KEYPAD_BTTNS_X, KEYPAD_BTTNS_Y},            
              { KEYPAD_BTTNS_X + KEYPAD_BTTNS_WD, KEYPAD_BTTNS_Y + KEYPAD_BTTNS_HT } }; 
   err = MFDBttnArrayInit(&f->handlers[cnt++],&r,bdims,bsize,mfd_keypad_button_handler,NULL);
   if (err != OK) return err;
   f->handler_count = cnt;
   return OK;
}


// Does every thing but set the slot..
void mfd_setup_keypad(char special)
{
   int i;
   keypad_data_type* keypad_data = (keypad_data_type*)&player_struct.mfd_func_data[MFD_KEYPAD_FUNC][0];
   keypad_data->curr_digit = 0;
   for (i=0; i < MAX_KEYPAD_DIGITS; i++)
      keypad_data->digits[i] = 0;
   keypad_data->special = special;
   mfd_notify_func(MFD_KEYPAD_FUNC, MFD_INFO_SLOT, TRUE, MFD_ACTIVE, TRUE);
}

#define NUMBER_BUFSZ 3

void mfd_keypad_expose(MFD* mfd, ubyte control)
{
   keypad_data_type* keypad_data = (keypad_data_type*)&player_struct.mfd_func_data[MFD_KEYPAD_FUNC][0];
   char buf[NUMBER_BUFSZ];
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0) 
   {
      ObjID pr;
      pr=panel_ref_unexpose(mfd->id,MFD_KEYPAD_FUNC);
      if(pr) objs[pr].info.current_frame=0;
      gKeypadOverride = FALSE;
      return;
   }
   mfd_clear_rects();
   PUSH_CANVAS(pmfd_canvas);
   if (full) mfd_clear_view();
   if ((keypad_data->special > 0) && (keypad_data->curr_digit > 0))
      draw_shodan_influence(mfd,keypad_data->special);
   else
   {
      if (full || (keypad_data->last_digit != keypad_data->curr_digit))
      {
         int i;
         short w,h;
//         LGPoint bstep = { KEYPAD_BTTNS_WD/KEYPAD_BTTN_COLS,
//                        KEYPAD_BTTNS_HT/KEYPAD_BTTN_ROWS };

         // Draw cool LED at top of MFD
         gr_set_font((grs_font*)ResLock(KEYPAD_STATUS_FONT));
         mfd_keypad_assemble(keypad_data, buf);
         gr_string_size(buf,&w,&h);
         mfd_draw_font_string(buf,KEYPAD_STATUS_X-w,KEYPAD_STATUS_Y,KEYPAD_STATUS_COLOR,KEYPAD_STATUS_FONT,TRUE);
         keypad_data->last_digit = keypad_data->curr_digit;
         ResUnlock(KEYPAD_STATUS_FONT);

         // Draw buttons
         gr_set_font((grs_font*)ResLock(MFD_FONT)); 
         for (i = 0; i < NUM_KEYPAD_BUTTONS; i++)
         {
            ubyte clr;
            LGPoint bttn;

            bttn.x = KEYPAD_BTTNS_X + (i/KEYPAD_BTTN_ROWS)*(KEYPAD_BTTNS_WD-KEYPAD_BTTN_WD)/(KEYPAD_BTTN_COLS-1);
            bttn.y = KEYPAD_BTTNS_Y + (i%KEYPAD_BTTN_ROWS)*(KEYPAD_BTTNS_HT-KEYPAD_BTTN_HT)/(KEYPAD_BTTN_ROWS-1);

            if ((keypad_data->curr_digit > 0) && (keypad_num(i) == keypad_data->digits[keypad_data->curr_digit - 1]))
               clr = SELECTED_ITEM_COLOR;
            else
               clr = ITEM_COLOR;
            gr_set_fcolor((long)clr);
            ss_box(bttn.x,bttn.y,bttn.x+KEYPAD_BTTN_WD,bttn.y+KEYPAD_BTTN_HT);
            gr_set_fcolor(ITEM_COLOR + 2);
            ss_box(bttn.x-1,bttn.y-1,bttn.x+KEYPAD_BTTN_WD+1,bttn.y+KEYPAD_BTTN_HT+1);
            keypad_name(keypad_num(i),buf);
            gr_string_size(buf,&w,&h);
            mfd_draw_string(buf,bttn.x+(KEYPAD_BTTN_WD-w)/2+1,bttn.y+1,clr,TRUE);
         }
         mfd_add_rect(KEYPAD_BTTNS_X,KEYPAD_BTTNS_Y,
            KEYPAD_BTTNS_X+KEYPAD_BTTNS_WD,KEYPAD_BTTNS_Y+KEYPAD_BTTNS_HT);
         ResUnlock(MFD_FONT);
      }
   }
   POP_CANVAS();
   mfd_update_rects(mfd);
}

/*���
// --------------------
//   HUD WARE MFD
// --------------------

#define MFD_HUD_FUNC 11

#define HUD_SETTING_MASK 0xF8
#define HUD_SETTING_SHF  3 

#define HUDWARE_STATUS (player_struct.hardwarez_status[CPTRIP(HUD_GOG_TRIPLE)])
#define HUDWARE_VERSION (player_struct.hardwarez[CPTRIP(HUD_GOG_TRIPLE)])


ushort hud_ware_bits[] = { HUD_COMPASS, HUD_DETECT_EXP, HUD_GRENADE };

#define NUM_HUDWARE_DISPLAYS (sizeof(hud_ware_bits)/sizeof(ushort))

#define HUDWARE_DISPLAY_AVAILABLE(dnum) (HUDWARE_VERSION & ( 1 << (dnum)))

// Note the use of negative logic here....
#define HUDWARE_DISPLAY_ACTIVE(dnum) (!(HUDWARE_STATUS & (1 << ((dnum) + HUD_SETTING_SHF))))
#define HUDWARE_DISPLAY_TOGGLE(dnum) (HUDWARE_STATUS ^= (1 << (dnum) + HUD_SETTING_SHF))

bool mfd_hud_button_handler(MFD* m, LGPoint bttn, uiEvent* ev, void* data)
{

   // Check to see if we actually have the specified display. 
   if (!HUDWARE_DISPLAY_AVAILABLE(bttn.y)) return FALSE;
   // Toggle the display
   if (ev->type == UI_EVENT_MOUSE &&  ev->subtype & (MOUSE_LDOWN|MOUSE_RDOWN))
      HUDWARE_DISPLAY_TOGGLE(bttn.y);

   if (HUDWARE_STATUS & WARE_ON) // update the actual hud. 
   {
      if (HUDWARE_DISPLAY_ACTIVE(bttn.y))
         hud_set(hud_ware_bits[bttn.y]);
      else
         hud_unset(hud_ware_bits[bttn.y]);
   }
   mfd_notify_func(MFD_HUD_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
   return TRUE;
}


#define HUDWARE_BUTTON_X 2
#define HUDWARE_BUTTON_Y 16
#define HUDWARE_LINE_SPACING 7 

#define HUDWARE_LAST_STATUS(mfd) mfd_fdata[MFD_HUD_FUNC][mfd]

#define HUDWARE_BOX_COLOR ITEM_COLOR
#define HUDWARE_BOX_SIZE  HUDWARE_LINE_SPACING

#ifdef HUDWARE_MFD

errtype mfd_hud_init(MFD_Func* f)
{
   errtype err;
   LGPoint bsize = { MFD_VIEW_WID, HUDWARE_LINE_SPACING};
   LGPoint bdims = { 1, NUM_HUDWARE_DISPLAYS };
   LGRect  brect = { { 0, HUDWARE_BUTTON_Y },
                  { MFD_VIEW_WID, HUDWARE_BUTTON_Y + NUM_HUDWARE_DISPLAYS*HUDWARE_LINE_SPACING}};
   int cnt = 0;
   err = MFDBttnArrayInit(&f->handlers[cnt++],&brect,bdims,bsize,mfd_hud_button_handler,NULL);
   if (err != OK) return err;
   f->handler_count = cnt;
   return OK;
}

void mfd_hud_expose(MFD* mfd, ubyte control)
{
   bool full = control & MFD_EXPOSE_FULL;
   if (control == 0) return;

   mfd_clear_rects();
   PUSH_CANVAS(pmfd_canvas);
   ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

   // Lay down the "background" 
   mfd_item_micro_expose(TRUE,HUD_GOG_TRIPLE);
   // clear rects so that we don't draw it if we don't have to
   if (!full) mfd_clear_rects();

   if (full || HUDWARE_STATUS != HUDWARE_LAST_STATUS(mfd->id))
   {
      int i;
      for (i = 0; i < NUM_HUDWARE_DISPLAYS; i++)
         if (HUDWARE_DISPLAY_AVAILABLE(i))
         {
            short x = HUDWARE_BUTTON_X;
            short y = HUDWARE_BUTTON_Y + i * HUDWARE_LINE_SPACING;
            short textx;
            char* s;
            // draw an "x" if active
            if (HUDWARE_DISPLAY_ACTIVE(i))
            {
               gr_set_fcolor(GOOD_RED);
               ss_int_line(x,y,x+HUDWARE_BOX_SIZE-1,y+HUDWARE_BOX_SIZE-1);
               ss_int_line(x,y+HUDWARE_BOX_SIZE-1,x+HUDWARE_BOX_SIZE-1,y);
            }
            gr_set_fcolor(HUDWARE_BOX_COLOR);
            ss_box(x,y,x+HUDWARE_BOX_SIZE,y+HUDWARE_BOX_SIZE);
            gr_set_font((grs_font*)ResLock(MFD_FONT));
            s = get_temp_string(REF_STR_HudBase+i);
            textx = (MFD_VIEW_WID - HUDWARE_BOX_SIZE - gr_string_width(s))/2 + HUDWARE_BOX_SIZE;
            mfd_draw_string(s,textx,y+1,gr_get_fcolor(),TRUE);
            ResUnlock(MFD_FONT);
            mfd_add_rect(x,y,MFD_VIEW_WID,y+HUDWARE_LINE_SPACING);
         }
      HUDWARE_LAST_STATUS(mfd->id) = HUDWARE_STATUS;
   }
   POP_CANVAS();
   mfd_update_rects(mfd);
}

void hudware_update_status(bool on)
{
   int i;
   for (i = 0; i < NUM_HUDWARE_DISPLAYS; i++)
      if (HUDWARE_DISPLAY_ACTIVE(i) && on)
         hud_set(hud_ware_bits[i]);
      else
         hud_unset(hud_ware_bits[i]);
}

#endif // HUDWARE_MFD
*/


// ----------------------------------------------------------
// THE GOOFY SEVERED HEAD MFD
// ----------------------------------------------------------

void severed_head_expose(MFD* mfd, ubyte control);

void severed_head_expose(MFD* mfd, ubyte control)
{
   bool full = control & MFD_EXPOSE_FULL;
   if (full)
   {
      grs_bitmap bm;
      ubyte headnum = player_struct.actives[ACTIVE_GENERAL];
      ObjID head = player_struct.inventory[headnum];
      int mug;
      uint trip=ID2TRIP(head);

      if (head == OBJ_NULL || !(trip == HEAD_TRIPLE || trip==HEAD2_TRIPLE))
         return;
            // clear update rects
      mfd_clear_rects();
      // set up canvas
      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
            // Clear the canvas by drawing the background bitmap
      if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         gr_bitmap(&mfd_background, 0, 0);

      mug = REF_IMG_EmailMugShotBase + objSmallstuffs[objs[head].specID].data1;
      bm.bits = NULL;
      extract_temp_res_bitmap(&bm,mug);
//KLC - chg for new art	   ss_bitmap(&bm,(MFD_VIEW_WID-bm.w)/2,(MFD_VIEW_HGT-bm.h)/2);
      gr_bitmap(&bm,(SCONV_X(MFD_VIEW_WID)-bm.w)/2, (SCONV_Y(MFD_VIEW_HGT)-bm.h)/2);
       
      // draw the name
      mfd_draw_string(get_object_long_name(ID2TRIP(head),NULL,0), X_MARGIN, 2, GREEN_YELLOW_BASE, TRUE);
      mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      // Pop the canvas
      gr_pop_canvas();
      // Now that we've popped the canvas, we can send the 
      // updated mfd to screen
      mfd_update_rects(mfd);
   }
}

// -------------------
// * GENERIC BLANK MFD
// -------------------

// ---------------------------------------------------------------------------
// mfd_expose_blank()
//
// Draw whatever we're supposed to draw if we're looking at an empty slot.




void mfd_expose_blank(MFD *m, ubyte control)
{
   if (full_game_3d)
   {
      full_visible &= ~visible_mask(m->id);
      chg_set_sta(FULLSCREEN_UPDATE);
      return;
   }
   if ((control & MFD_EXPOSE) && !full_game_3d) {

      PUSH_CANVAS(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
 
      draw_blank_mfd();
   
      POP_CANVAS();
      mfd_update_display(m, 0, 0, MFD_VIEW_WID, MFD_VIEW_HGT);
   }

   return;
}

// ---------------------------------------------------------------------------
//                  CALLS FROM OTHER MODULES TO THE MFD SYSTEM
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// set_inventory_mfd()
//
// Called by the inventory whenever a new drug, ware, something is clicked
// on in the inventory panel.

ulong catbasetrips [MFD_INV_CATEGORIES] =
{
   0,
   MAKETRIP(CLASS_DRUG,0,0),
   MAKETRIP(CLASS_HARDWARE,0,0),
   MAKETRIP(CLASS_GRENADE,0,0),
   MAKETRIP(CLASS_AMMO,0,0),
   MAKETRIP(CLASS_GUN,0,0),
   0, // general inventory
   MAKETRIP(CLASS_SOFTWARE,SOFTWARE_SUBCLASS_OFFENSE,0),
   MAKETRIP(CLASS_SOFTWARE,SOFTWARE_SUBCLASS_DEFENSE,0),
   MAKETRIP(CLASS_SOFTWARE,SOFTWARE_SUBCLASS_ONESHOT,0),
};

// look, another array that should not exist. 
ubyte catactives[] =
{
   0,
   ACTIVE_DRUG,
   ACTIVE_HARDWARE,
   ACTIVE_GRENADE,
   ACTIVE_CART,
   ACTIVE_WEAPON,
   ACTIVE_GENERAL,
   ACTIVE_COMBAT_SOFT,
   ACTIVE_DEFENSE_SOFT,
   ACTIVE_MISC_SOFT,
};

ubyte activecats[] =
{
   MFD_INV_WEAPON,
   MFD_INV_GRENADE,
   MFD_INV_DRUG,
   MFD_INV_AMMO,
   MFD_INV_HARDWARE,
   MFD_INV_SOFT_COMBAT,
   MFD_INV_SOFT_DEFENSE,
   MFD_INV_SOFT_MISC,
   MFD_INV_GENINV,
   0,
};



extern void set_current_active(int);
void update_item_mfd(void);
bool mfd_distance_remove(ubyte slot_func);
bool mfd_target_qual(void);
bool mfd_automap_qual(void);
bool mfd_weapon_qual(void);

void set_inventory_mfd(ubyte obclass, ubyte type, bool grab)
{
   int i;
   ubyte func = MFD_EMPTY_FUNC;
   ubyte slot;
   MFD_Status stat;
   bool classhit = FALSE;

   switch(obclass) {

      case MFD_INV_WEAPON:

         // The "grab" arg is TRUE in case blanked out by an inventory drop
         func = MFD_WEAPON_FUNC; slot = MFD_WEAPON_SLOT; stat = MFD_ACTIVE;
         if (type == MFD_INV_NOTYPE)
         {
            stat = MFD_EMPTY;
         }
         break;

      case MFD_INV_NULL:

         if (type == MFD_INV_NOTYPE) break;
         for (i = 0; i < NUM_MFDS; i++) {
            MFDSetCurrItemClass(i,obclass);
         }
         func = MFD_ITEM_FUNC; slot = MFD_ITEM_SLOT; stat = MFD_EMPTY;
         break;

      default:
         for (i = 0; i < NUM_MFDS; i++)
         {
            if (MFDGetCurrItemClass(i) == obclass)
               classhit = TRUE;               
            if (type != MFD_INV_NOTYPE)
               MFDSetCurrItemClass(i,obclass);
         }
         slot = MFD_ITEM_SLOT; stat = MFD_ACTIVE;
         if (type == MFD_INV_NOTYPE)
         {
            if (classhit)
            {
               mfd_notify_func(MFD_EMPTY_FUNC,MFD_ITEM_SLOT,TRUE,MFD_EMPTY,TRUE);
               player_struct.actives[catactives[obclass]] = 0;
            }
         }
         else if (obclass == MFD_INV_GENINV && player_struct.inventory[type] == OBJ_NULL)
         {
            mfd_notify_func(MFD_EMPTY_FUNC, MFD_ITEM_SLOT, grab, MFD_EMPTY, TRUE);
         }
         else 
         {
            ulong opnum = (obclass != MFD_INV_GENINV) ? OPTRIP(catbasetrips[obclass]) + type : OPNUM(player_struct.inventory[type]);
            func = ObjProps[opnum].mfd_id;
            if (func == MFD_EMPTY_FUNC) func = MFD_ITEM_FUNC;
            set_current_active(catactives[obclass]);
         }
         break;

   }
   if (func != MFD_EMPTY_FUNC)
   {
      mfd_notify_func(func, slot, grab, stat, TRUE);
#ifdef RAISE_ON_SELECT
      if (full_game_3d)
      {
         int i;
         for (i = 0; i < NUM_MFDS; i++)
         {
            if (player_struct.mfd_current_slots[i] == MFD_ITEM_SLOT)
            {
#ifdef STEREO_SUPPORT
               if (convert_use_mode == 5)
               {
                  full_visible = FULL_MFD_MASK(i);
               }
               else
#endif
                  full_visible |= FULL_MFD_MASK(i);
         }
      }
#endif 

   }

   // THEN we check to see if we need to take over the info mfd

   switch(obclass) {

      case MFD_INV_HARDWARE:

         switch(type) {

            case HARDWARE_BIOWARE:

               if (WareActive(player_struct.hardwarez_status[HARDWARE_BIOWARE]))
                  mfd_notify_func(MFD_BIOWARE_FUNC, MFD_INFO_SLOT, TRUE, MFD_ACTIVE, TRUE);

               break;

         }

         break;
   }

   if (obclass!= MFD_INV_NULL && type != MFD_INV_NOTYPE)
      player_struct.actives[catactives[obclass]] = type;

   return;
}

void update_item_mfd(void)
{
   ubyte curr = player_struct.current_active;
   if (curr != NULL_ACTIVE)
   {
      ubyte obclass = activecats[curr];
      ubyte type = player_struct.actives[curr];
      ulong opnum = (obclass != MFD_INV_GENINV) ? OPTRIP(catbasetrips[obclass]) + type : OPNUM(player_struct.inventory[type]);
      int func = ObjProps[opnum].mfd_id;
      if (func != MFD_EMPTY_FUNC)
      {
         mfd_notify_func(func,MFD_ITEM_FUNC,TRUE,MFD_ACTIVE,TRUE);
      }
   }
}

bool mfd_distance_remove(ubyte slot_func)
{
   switch (slot_func)
   {
      case MFD_KEYPAD_FUNC:
      case MFD_FIXTURE_FUNC:
      case MFD_ELEV_FUNC:
      case MFD_BARK_FUNC:
      case MFD_ACCESSPANEL_FUNC:
      case MFD_GUMP_FUNC:
      case MFD_GRIDPANEL_FUNC:
         return TRUE;
   }
   return FALSE;
}

// -------
// DEFAULT MFD FUNC QUALIFYING FUNCTIONS
bool mfd_target_qual(void)
{
   return(player_struct.hardwarez[HARDWARE_TARGET]>0);
}

bool mfd_automap_qual(void)
{
   return(player_struct.hardwarez[HARDWARE_AUTOMAP]>0);
}

bool mfd_weapon_qual(void)
{
   return(player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]].type!=EMPTY_WEAPON_SLOT);
}   

// --------------------------------------------------------
// THE STATIC MFD_FUNCS ARRAY

extern void mfd_view360_expose(MFD* mfd, ubyte control);
extern void mfd_dummy_expose(MFD* mfd, ubyte control);
extern void mfd_fixture_expose(MFD* mfd, ubyte control);
extern bool mfd_fixture_handler(MFD* mfd, uiEvent* e);
extern void mfd_emailmug_expose(MFD* mfd, ubyte control);
extern bool mfd_emailmug_handler(MFD* mfd, uiEvent* e);
extern errtype mfd_emailware_init(MFD_Func* f);
extern void mfd_emailware_expose(MFD*,ubyte);
extern void mfd_plotware_expose(MFD*,ubyte);
extern errtype mfd_plotware_init(MFD_Func* f);
extern void mfd_bark_expose(MFD*,ubyte);
extern errtype mfd_accesspanel_init(MFD_Func* f);
extern bool mfd_accesspanel_handler(MFD* mfd, uiEvent* ev);
extern void mfd_accesspanel_expose(MFD* mfd, ubyte control);
extern errtype mfd_gridpanel_init(MFD_Func* f);
extern void mfd_gridpanel_expose(MFD* mfd, ubyte control);
extern bool mfd_gridpanel_handler(MFD* mfd, uiEvent* ev);
extern void mfd_targetware_expose(MFD* mfd, ubyte control);
extern bool mfd_targetware_handler(MFD* mfd, uiEvent* ev);
extern void mfd_gump_expose(MFD* mfd, ubyte control);
extern bool mfd_gump_handler(MFD* mfd, uiEvent* ev);
extern void mfd_accesscard_expose(MFD* mfd, ubyte control);
extern void mfd_biohelp_expose(MFD* mfd, ubyte control);
extern errtype mfd_biohelp_init(MFD_Func* f);
extern bool mfd_biohelp_handler(MFD* mfd, uiEvent* ev);
extern void mfd_cspace_expose(MFD* mfd, ubyte control);
extern void mfd_viewhelp_expose(MFD* mfd, ubyte control);
extern errtype mfd_viewhelp_init(MFD_Func* f);
extern void mfd_gear_expose(MFD* mfd, ubyte control);
extern bool mfd_gear_handler(MFD* mfd, uiEvent* ev);


#define PANEL_PRIORITY  37


MFD_Func mfd_funcs[MFD_NUM_FUNCS] =
{
// MFD_EMPTY_FUNC   0
	{ mfd_expose_blank,  NULL, NULL, 255, MFD_NOSAVEREST},
// MFD_ITEM_FUNC    1
   	{ mfd_item_expose,  mfd_item_handler, mfd_item_init,40},
// MFD_MAP_FUNC     2
	{ mfd_map_expose,  mfd_map_handler, mfd_map_init, 20, MFD_INCREMENTAL},
// MFD_TARGET_FUNC  3
	{ mfd_target_expose,  mfd_target_handler, NULL, 21},
// MFD_ANIM_FUNC    4
	{ mfd_expose_blank,  NULL, NULL, 255},
// MFD_WEAPON_FUNC  5 
	{ mfd_weapon_expose,  mfd_weapon_handler, mfd_weapon_init, 25},
// MFD_BIOWARE_FUNC 6
	{ mfd_bioware_expose, NULL, NULL,50, MFD_NOSAVEREST},
// MFD_LANTERN_FUNC 7
	{ mfd_lanternware_expose, NULL, mfd_lanternware_init, 38},
// MFD_3DVIEW_FUNC  8
	{ mfd_view360_expose, NULL, NULL, 25, MFD_NOSAVEREST},
// MFD_ELEV_FUNC    9
	{ mfd_elevator_expose, NULL, mfd_elevator_init, PANEL_PRIORITY, MFD_NOSAVEREST},
// MFD_GRENADE_FUNC 10 
	{ mfd_grenade_expose, mfd_grenade_handler, mfd_grenade_init, 32},
// MFD_HUD_FUNC     11
	{ mfd_expose_blank, },
// MFD_FIXTURE_FUNC 12 
	{ mfd_fixture_expose, mfd_fixture_handler, NULL, 32, MFD_NOSAVEREST},  
// MFD_KEYPAD_FUNC  13
	{ mfd_keypad_expose, mfd_keypad_handler, mfd_keypad_init, PANEL_PRIORITY, MFD_NOSAVEREST },
// MFD_EMAILMUG_FUNC 14 
	{ mfd_emailmug_expose, mfd_emailmug_handler, NULL, 60, MFD_NOSAVEREST},
// MFD_EMAILWARE_FUNC 15
	{ mfd_emailware_expose, NULL, mfd_emailware_init, 60},
// MFD_PLOTWARE_FUNC  16
	{ mfd_plotware_expose, NULL, mfd_plotware_init, 55},
// MFD_BARK_FUNC 17
	{ mfd_bark_expose, NULL, NULL, 255, MFD_NOSAVEREST},
// MFD_ACCESSPANEL_FUNC 18
	{ mfd_accesspanel_expose, mfd_accesspanel_handler, mfd_accesspanel_init, PANEL_PRIORITY, MFD_INCREMENTAL|MFD_NOSAVEREST },
// MFD_SHIELD_FUNC 19
	{ mfd_shieldware_expose, mfd_shield_handler, mfd_shield_init, 36 },
// MFD_MOTION_FUNC 20
	{ mfd_motionware_expose, NULL, mfd_motion_init, 36 },
// MFD_SEVERED_HEAD_FUNC 21
	{ severed_head_expose, NULL, NULL, 250 },
// MFD_TARGETWARE_FUNC 22
	{ mfd_targetware_expose, mfd_targetware_handler, NULL, 40 },
// MFD_GUMP_FUNC 23
	{ mfd_gump_expose, mfd_gump_handler, NULL, 40, MFD_NOSAVEREST },
// MFD_CARD_FUNC 24 
	{ mfd_accesscard_expose, NULL, NULL, 40},
// MFD_BIOHELP_FUNC 25 
	{ mfd_biohelp_expose, mfd_biohelp_handler, mfd_biohelp_init, 40},
// MFD_GRIDPANEL_FUNC 26
	{ mfd_gridpanel_expose, mfd_gridpanel_handler, mfd_gridpanel_init, PANEL_PRIORITY, MFD_NOSAVEREST },
// MFD_GAMES_FUNC 27  
	{ mfd_games_expose, mfd_games_handler, mfd_games_init, PANEL_PRIORITY, MFD_INCREMENTAL|MFD_NOSAVEREST },
// MFD_CYBERSPACE_FUNC 28
   	{ mfd_cspace_expose, NULL, NULL, 40},
// MFD_VIEWHELP_FUNC 29
	{ mfd_viewhelp_expose, NULL, mfd_viewhelp_init, 40},
// MFD_GEAR_FUNC 30
	{ mfd_gear_expose, mfd_gear_handler, NULL, 40}
};
 


