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
 * $Source: r:/prj/cit/src/RCS/invent.c $
 * $Revision: 1.240 $
 * $Author: dc $
 * $Date: 1994/11/22 15:58:48 $
 *
 */

// Source code for inventory manipulation / display

#include <string.h>

#include "invent.h"
#include "objprop.h"
#include "objwpn.h"
#include "objwarez.h"
#include "objsim.h"
#include "tools.h"
#include "colors.h"
#include "player.h"
#include "weapons.h"
#include "drugs.h"
#include "grenades.h"
#include "wares.h"
#include "cybstrng.h"
#include "gamestrn.h"
#include "sideicon.h"
#include "gameloop.h"
#include "loops.h"
#include "input.h"
#include "mfdext.h"
#include "objbit.h"
#include "fullscrn.h"
#include "screen.h"
#include "cit2d.h"
#include "gr2ss.h"
#include "criterr.h"

#include "hkeyfunc.h"
#include "objload.h"
#include "invpages.h"
#include "sfxlist.h"
#include "musicai.h"
#include "emailbit.h"
#include "popups.h"

#include "otrip.h"
#include "gamescr.h"
#include "amap.h"
#include "citres.h"

//#include <inp6d.h>
//#include <i6dvideo.h>


/***************************************/ 
/* INVENTORY DISPLAY MODULE            */
/*-------------------------------------*/ 
/*---------------------------------------------
The inventory display is arranged into a number of
/pages/, each of which can contain one or more
/lists/.  In general, a list shows "what types of
a particular thing you have," and "how many of
each do you have."  There is a list for weapons,
one for grenades, one for drugs, etc.

The contents of each list is, more or less, defined
by two arrays, and "exists" array and a "quantity"
array.  For a particular list, say, the grenades
list, exists[N] is non-zero if you have any
grenades of type N, and quant[N] is the number of
grenades of that type you have.You astutely
observe: "Why are these lists different, when the
quantity of something is always zero if you have
none of it, and thus the quantity array and the
exists array could be the same?"  The answer is:
because it's goofy. 

For each list, the inventory display maintains a
state array describing what stuff was actually
DRAWN, so it can redraw incrementally.  The
elements of the array are structures of type
/quantity_state/, defined below.

A list can be divided onto different pages of the
inventory, and each section of a list that
appears on its own page is called a /display/. 
The inventory panel figures out what to display
by looking through a huge list of all the
displays in the game.  Adding a new display is as
simple as adding a new element to this array.  Of
course, that element is a huge wonking structure
describing when and where and how to draw, use,
and select items on the display.

And now, the code...
----------------------------------------------*/

#define KEY_CODE_ESC 0x1b

// -------------
// DISPLAY STUFF
// -------------
// -------
// DEFINES
// -------
#define INVENT_CHANGED    if (_current_loop <= FULLSCREEN_LOOP) \
                            chg_set_flg(INVENTORY_UPDATE);


// colors & fonts
#define TITLE_COLOR (RED_BASE+5)
#define ITEM_COLOR  0x5A
#define SELECTED_ITEM_COLOR (0x4C)
#define BRIGHT_ITEM_COLOR   (0xE7)
#define DULL_ITEM_COLOR (0x5F)
// let us no longer pretend we have differenet fonts for everything
#define ITEM_FONT RES_tinyTechFont
#define WEAPONS_FONT ITEM_FONT

// Screen margins/locations/proportions
#define TOP_MARGIN 2 // was 15
#define LEFT_MARGIN 4
#define RIGHT_MARGIN 3
#define Y_STEP 6
#define WEAPON_X LEFT_MARGIN
#define AMMO_X 70 
#define GRENADE_LEFT_X (AMMO_X + 4)
#define GRENADE_RIGHT_X (GRENADE_LEFT_X+30)
#define DRUG_RIGHT_X (INVENTORY_PANEL_WIDTH - RIGHT_MARGIN)
#define DRUG_LEFT_X (DRUG_RIGHT_X-35)
#define AMMO_LEFT_1 WEAPON_X
#define AMMO_RIGHT_1 (AMMO_LEFT_1 + 40)
#define AMMO_LEFT_2 (AMMO_RIGHT_1 + 4)
#define AMMO_RIGHT_2 (AMMO_LEFT_2 + 40)
#define AMMO_LEFT_3 (AMMO_RIGHT_2 + 4)
#define AMMO_RIGHT_3 (AMMO_LEFT_3 + 40)
#define CENTER_X (INVENTORY_PANEL_WIDTH/2)
#define RIGHT_X INVENTORY_PANEL_WIDTH
#define ONETHIRD_X (INVENTORY_PANEL_WIDTH/3)
#define TWOTHIRDS_X (2*INVENTORY_PANEL_WIDTH/3)

// Page button defines
#define FIRST_BTTN_X (3)
#define INVENT_BTTN_Y (2)
#define INVENT_BTTN_HT 3
#define INVENT_BTTN_WD 18
#define BUTTON_X_STEP 24


// Hey, these colors are stolen from mfd
#define INVENT_BTTN_EMPTY      0xcb
#define INVENT_BTTN_FLASH_ON   0x35
#define INVENT_BTTN_FLASH_OFF  0xcb
#define INVENT_BTTN_SELECT     0x77

typedef enum {
               BttnOff = 0,
               BttnDummy = 1,
               BttnActive = 2,
               BttnFlashOff = 3,
               BttnFlashOn = 16,
               NUM_BUTTON_STATES= BttnFlashOn+1
             } invent_bttn_state;

#define FlashOn(state) (((state) & 0x1) == 0)
#define Flashing(state) ((state) >= BttnFlashOff && (state) <= BttnFlashOn)            

// mapping from button states to colors
ubyte _bttn_state2color[] = {
                              INVENT_BTTN_EMPTY,
                              INVENT_BTTN_EMPTY,
                              INVENT_BTTN_SELECT,
                              INVENT_BTTN_FLASH_OFF,
                              INVENT_BTTN_FLASH_ON,
                            };

#define bttn_state2color(state) _bttn_state2color[Flashing(state) ? BttnFlashOff + !(state & 1) : state]

#define INVENT_BTTN_FLASH_TIME 128

#define NUM_PAGE_BUTTONS  6

// Page stuff
#define WEAPON_PAGES 1
#define WEAPONS_PER_PAGE 7
#define DRUG_PAGES 1
#define DRUGS_PER_PAGE   7
#define GRENADE_PAGES 1
#define GRENADES_PER_PAGE   7
#define AMMO_PAGES 3
#define AMMO_PER_PAGE  3
#define ITEMS_PER_PAGE 7


// Misc
#define BUFSZ 50
#define NULL_ACTIVE -1  // the null active 

// Object adding return codes

typedef enum
   { 
      ADD_FAIL,        
      ADD_POP,         
      ADD_SWAP,        
      ADD_REJECT,      
      ADD_NOROOM,
      ADD_NOEFFECT
   } AddResult;


#define IS_POP_RESULT(r) ((r) == ADD_POP || (r) == ADD_NOEFFECT)

typedef struct _quantity_state
{
   ushort num;    // item number
   ubyte exist;  // do we have it;
   ubyte quant;  // quantity   
   byte pad;
} quantity_state;

// Get the correct color for an item, given its rank in the list.
typedef uchar (*color_func)(void* dp, int num);

// Get the string name of an item, given its rank in the 
// list.
typedef char* (*name_func)(void* dp, int num, char* buf);

// Get the string quantity of an item, given its rank in the 
// list,and its actual quantity.
typedef char* (*quant_string_func)(struct _inventory_display_list* dp, int num, int quant, char* buf);

// Draw a display
typedef void (*draw_func)(struct _inventory_display_list *dp);

// Select an item given its rank in the list
typedef bool (*select_func)(struct _inventory_display_list *dp, int itemnum);

// Use an item given its rank in the list.
typedef bool (*use_func)(struct _inventory_display_list *dp, int itemnum);

// Add an object to a list 
typedef ubyte (*add_func)(struct _inventory_display_list *dp, int itemnum, ObjID* obj,bool select);

// Remove and object from a list
typedef void (*drop_func)(struct _inventory_display_list *dp, int itemnum);

typedef struct _inventory_display_list
{
   ushort pgnum;             // Page number this display is on
   short relnum;            // Relative page number for this list.
   short left, right;       // Left and right pixel edge
   short top;               // Top pixel margin
   ubyte titlecolor;        // Title color.  Duh.
   ubyte listcolor;         // color for list items. if greater than 239,
                            // it's a color func. 
   ushort first;             // first list item to start at. 
   ushort pgsize;            // Number of items in each list page
   ushort listlen;           // Number of total list items
   int titlenum;            // String number of title.
   int activenum;           // index into player_struct.actives
   int offset;              // offset of quantities into player struct
   int mfdtype;             // inventory time for set_inventory_mfd
   name_func name;          // given a type number, give us the name
   quant_string_func quant; // Given an item number, give us the quantity string
   draw_func draw;          // draw this inv_display
   select_func select;      // select a row
   use_func use;            // use a row
   int add_classes;         // Classes of objects that this list represents
   add_func add;            // function to add an object to this list.
   drop_func drop;          // remove an object from a row.  
   int basetrip;            // triple of item number zero.  
   int (*toidx)(int);       // convert from a triple to an index  (NOT USED) 
   // state data
   uchar dummy;             // used to be known_active
   quantity_state* lines;   // lines of state data
} inv_display;

int known_actives[NUM_ACTIVES];

#define NULL_PAGE 0xFFFF
extern inv_display inv_display_list[];

extern uchar email_color_func(void* dp, int num);

color_func color_func_list[] = { email_color_func };
#define EMAIL_COLOR_FUNC 240

// -------
// GLOBALS
// -------

// The current inventory "page" 

short inventory_page = 0;
bool show_all_actives=FALSE;

// The last page we drew 
short inv_last_page = INV_BLANK_PAGE;

LGRegion *inventory_region;
extern LGRegion *inventory_region_game, *inventory_region_full;
LGRegion** all_inventory_regions[]={&inventory_region_game, &inventory_region_full};

#define NUM_INVENT_REGIONS (sizeof(all_inventory_regions)/sizeof(LGRegion**))

static struct _weapon_list_state
{
   ubyte active;
   weapon_slot slots[WEAPON_PAGES*WEAPONS_PER_PAGE];
   ubyte ammo_available[WEAPON_PAGES*WEAPONS_PER_PAGE];
}  weapon_list;

quantity_state generic_lines[48];



// page button state
invent_bttn_state page_button_state[NUM_PAGE_BUTTONS] = { BttnOff, BttnOff, BttnOff,BttnDummy,BttnDummy, BttnOff,
                                                         };
// Last button state that was actually drawn. 
invent_bttn_state old_button_state[NUM_PAGE_BUTTONS] = { BttnDummy, BttnDummy, BttnDummy,BttnDummy,BttnDummy, BttnDummy
                                                         };

LGRegion* pagebutton_region;
 
// DRAWING STUFF
grs_bitmap inv_backgnd;
grs_canvas inv_norm_canvas;
grs_canvas inv_fullscrn_canvas;
grs_canvas inv_view360_canvas;
grs_canvas *pinv_canvas = &inv_norm_canvas;

grs_canvas inv_gamepage_canvas;
grs_canvas inv_fullpage_canvas;
grs_canvas *ppage_canvas = &inv_gamepage_canvas;

#define inv_canvas (*pinv_canvas)

#define NUM_PAGE_BTTNS NUM_PAGE_BUTTONS


#ifdef OLD_BUTTON_CURSORS
LGCursor     invent_bttn_cursors[NUM_PAGE_BTTNS];
grs_bitmap invent_bttn_bitmaps[NUM_PAGE_BTTNS];
Ref        invent_bttn_curs_ids[NUM_PAGE_BTTNS] = { REF_IMG_bmInventWeapon,
                                                    REF_IMG_bmInventHardware,
                                                    REF_IMG_bmInventGeneral,
                                                    REF_IMG_bmTargetCursor,
                                                    REF_IMG_bmInventCombatSoft,
                                                    REF_IMG_bmInventMiscSoft,
                                                  };
#else
LGCursor      invent_bttn_cursor;
grs_bitmap  invent_bttn_bitmap;
#endif

static char* cursor_strings[NUM_PAGE_BUTTONS];
static char cursor_string_buf[128];

#define BUTTON_PANEL_Y (INVENTORY_PANEL_Y + INVENTORY_PANEL_HEIGHT)

#define INVENT_BUTTON_PANEL_X (-1)
#define INVENT_BUTTON_PANEL_Y (196 - BUTTON_PANEL_Y)


// ---------------------
//  Internal Prototypes
// ---------------------
void draw_page_buttons(bool full);
ubyte add_to_some_page(ObjID obj,bool select);
void push_inventory_cursors(LGCursor* newcurs);
void pop_inventory_cursors(void);
void draw_inventory_string(char* s, int x, int y, bool clear);
void clear_inventory_region(short x1,short y1,short x2,short y2);
void draw_quant_line(char* name, char* quant, long color, bool active, short left, short right, short y);
void draw_quant_list(inv_display* dp, bool newpage);
void set_current_active(int activenum);
int get_item_at_pixrow(inv_display *dp, int row);
char* weapon_name_func(void*, int num, char* buf);
char* weapon_quant_func(int num, char* buf);
void draw_weapons_list(inv_display *dp);
bool inventory_select_weapon(inv_display* dp, int w);
bool weapon_use_func(inv_display* dp, int w);
ubyte weapons_add_func(inv_display* dp, int row, ObjID* objP,bool select);
void weapon_drop_func(inv_display* dp, int itemnum);
ubyte generic_add_func(inv_display* dp, int row, ObjID* idP,bool select);
void generic_drop_func(inv_display* dp, int row);
char* null_name_func(inv_display* dp, int n, char* buf);
static char* grenade_name_func(void* vdp, int n, char* buf);
void push_live_grenade_cursor(ObjID obj);
bool grenade_use_func(inv_display* dp, int row);
ubyte grenade_add_func(inv_display* dp, int row, ObjID* idP, bool select);
char* drug_name_func(inv_display* dp, int n, char* buf);
bool drug_use_func(inv_display* dp, int row);
char* ammo_name_func(void* , int n, char* buf);
void hardware_add_specials(int n, int ver);
ubyte ware_add_func(inv_display* dp, int, ObjID* idP,bool select);
void ware_drop_func(inv_display* dp, int row);
char* null_quant_func(inv_display*, int, int, char* buf);
void draw_general_list(inv_display *dp);
bool general_use_func(inv_display* dp, int row);
ubyte inv_empty_trash(void);
ubyte add_access_card(inv_display* dp, ObjID* idP,bool select);
ubyte general_add_func(inv_display* dp, int row, ObjID* idP,bool select);
void remove_general_item(ObjID obj);
void general_drop_func(inv_display*, int row);
bool inv_select_general(inv_display* dp, int w);
void email_more_draw(inv_display *dp);
bool email_more_use(inv_display* dp, int);
bool email_use_func(inv_display* dp, int row);
void email_select_func(inv_display* dp, int row);
void add_email_datamunge(short mung,bool select);
ubyte email_add_func(inv_display*, int, ObjID* idP,bool select);
void email_drop_func(inv_display*, int );
char* log_name_func(void*, int num, char* buf);
bool log_use_func(inv_display* dp, int row);
void inventory_draw_page(int pgnum);
void draw_page_button_panel();
bool do_selection(inv_display* dp, int row);
void add_object_on_cursor(inv_display* dp, int row);
bool inventory_handle_leftbutton(uiEvent* ev, inv_display* dp, int row);
bool inventory_handle_rightbutton(uiEvent* ev, LGRegion* reg, inv_display* dp, int row);
bool inventory_mouse_handler(uiEvent* ev, LGRegion* r, void*);
bool pagebutton_mouse_handler(uiMouseEvent* ev, LGRegion* r, void*);
bool invent_hotkey_func(ushort, ulong, int data);
bool cycle_weapons_func(ushort, ulong, int data);
void init_invent_hotkeys(void);
void invent_language_change(void);
errtype inventory_update_screen_mode();
void inv_change_fullscreen(bool on);
void inv_update_fullscreen(bool full);
void super_drop_func(int dispnum, int row);
void super_use_func(int dispnum, int row);
void gen_log_displays(int pgnum);
void absorb_object_on_cursor(short, ulong, void*);
bool gen_inv_page(int pgnum, int *i, inv_display** dp);
bool gen_inv_displays(int *i, inv_display** dp);



// ---------------------
// DISPLAY LIST ROUTINES 
// ---------------------

// --------
// ROUTINES
// --------

void push_inventory_cursors(LGCursor* newcurs)
{
   int i;

   for(i=0;i<NUM_INVENT_REGIONS;i++) {
      uiPushRegionCursor(*(all_inventory_regions[i]),newcurs);
   }
}

void pop_inventory_cursors(void)
{
   int i;

   for(i=0;i<NUM_INVENT_REGIONS;i++) {
      uiPopRegionCursor(*(all_inventory_regions[i]));
   }
}


// draw a string in relative coordinates
void draw_inventory_string(char* s, int x, int y, bool clear)
{
   short w,h;
   short a,b,c,d;

   gr_string_size(s,&w,&h);
   if (w <= 0 || h <= 0 || strlen(s) == 0) return;
   STORE_CLIP(a,b,c,d);
//   Warning(("draw_string clip %d %d %d %d\n",x-1,y-1,x+w,y+h));
   ss_safe_set_cliprect(x-1,y-1,x+w,y+h);
   if (!full_game_3d)
   {

      LGRect r;
      r.ul.x = x-1;
      r.ul.y = y-1;
      r.lr.x = x + w;
      r.lr.y = y + h;
      RECT_MOVE(&r,MakePoint(INVENTORY_PANEL_X,INVENTORY_PANEL_Y));
      uiHideMouse(&r);
      if (clear)
//KLC - chg for new art      	ss_bitmap(&inv_backgnd,0,0);
	 	gr_bitmap(&inv_backgnd, 0, 0);
      draw_shadowed_string(s,x,y,FALSE);
      uiShowMouse(&r);
   }
   else
   {
      long oldcolor = gr_get_fcolor();
      if (clear)
      {
         gr_set_fcolor(0);
         ss_rect(x,y,x+w,y+h-1);
      }
      gr_set_fcolor(oldcolor);
      draw_shadowed_string(s,x,y,TRUE);
   }
   RESTORE_CLIP(a,b,c,d);
}

void clear_inventory_region(short x1,short y1,short x2,short y2)
{
   LGRect r;
   short a,b,c,d;

   if (!full_game_3d)
   {
      r.ul.x = x1; r.ul.y = y1;
      r.lr.x = x2; r.lr.y = y2;
      STORE_CLIP(a,b,c,d);
      ss_safe_set_cliprect(x1,y1,x2,y2);
      RECT_MOVE(&r,MakePoint(INVENTORY_PANEL_X,INVENTORY_PANEL_Y));
      uiHideMouse(&r);
//KLC - chg for new art      ss_bitmap(&inv_backgnd,0,0);
	  gr_bitmap(&inv_backgnd, 0, 0);
      uiShowMouse(&r);
      RESTORE_CLIP(a,b,c,d);
   }
   else
   {
      gr_set_fcolor(0);
      ss_rect(x1-1,y1,x2+1,y2);
   }
}

// Draw a single line of the weapons list
void draw_quant_line(char* name, char* quant, long color, bool active, short left, short right, short y)
{
   short	ht, wd;
   
   gr_string_size(name, &wd, &ht); 
   wd = gr_string_width(quant);
   clear_inventory_region(left,y,right,y+ht);
   if (active)
      gr_set_fcolor(SELECTED_ITEM_COLOR);
   else if(color<256)
      gr_set_fcolor(color);
   draw_inventory_string(name,left,y,FALSE);
   draw_inventory_string(quant,right-wd,y,FALSE);
}


void draw_quant_list(inv_display* dp, bool newpage)
{
   int wtype;
   int cnt;
   int y, i;
   char buf[BUFSZ];
   ubyte *quant = (ubyte*)&player_struct + dp->offset;
   ubyte *exist = quant;
   int active = (dp->activenum == NULL_ACTIVE) ? -1 : player_struct.actives[dp->activenum];
   int known_active = (dp->activenum == NULL_ACTIVE) ? -1 : known_actives[dp->activenum];
   quantity_state* line = dp->lines + dp->relnum*dp->pgsize;

   // Hey, what if we don't have our active item anymore...
   if (active >= 0 && active < dp->listlen && quant[active] == 0)
   {
      player_struct.actives[dp->activenum] = active = MFD_INV_NOTYPE;
      set_inventory_mfd(dp->mfdtype,MFD_INV_NOTYPE,TRUE);
   }
   if (newpage && dp->titlenum != REF_STR_Null)
   {
      gr_set_fcolor(dp->titlecolor);
      get_string(dp->titlenum,buf,BUFSZ);
      draw_inventory_string(buf,dp->left,dp->top,TRUE);
   }
   if (dp->first != 0) wtype = dp->first;
   else 
      for(wtype = 0, cnt = 0; wtype < dp->listlen && cnt < dp->pgsize*dp->relnum; wtype++)
      {
         if (exist[wtype] > 0) cnt++;
      }
   for (y = dp->top + Y_STEP, i = 0, cnt = 0; cnt < dp->pgsize; i++,wtype++)
   {
      if (wtype >= dp->listlen)
      {
         if(line->num < dp->listlen)
         {
            clear_inventory_region(dp->left,y,dp->right,y+Y_STEP);
         }
         y+= Y_STEP;
         line->num = wtype;
         line++;
         cnt++;
         continue;
      }
      if (exist[wtype] != 0)
      {
         bool newactive = active != known_active;
         // note the hack for combat softwares
         bool curractive = ((show_all_actives&&dp->pgnum==INV_MAIN_PAGE)||player_struct.current_active==dp->activenum) || dp->activenum == ACTIVE_COMBAT_SOFT;
         bool changed = newpage
                        || newactive
                        || (line->num != wtype)
                        || (line->quant != quant[wtype])
                        || (line->exist != exist[wtype]);
         line->num = wtype;
         line->quant = quant[wtype];
         line->exist = exist[wtype];
         if (changed)
         {
            char buf[BUFSZ] =  "";
            char buf2[BUFSZ] = "";
            bool is_active = wtype == active && curractive;
            uchar col;

            if (dp->name != NULL)
               dp->name(dp,wtype,buf);
            if (dp->quant != NULL)
               dp->quant(dp,wtype,quant[wtype],buf2);
            col=dp->listcolor;
            if(col>239) col=(color_func_list[col-240])(dp,wtype);
            draw_quant_line(buf,buf2,col,is_active,dp->left,dp->right,y);
         }
         cnt++;
         line++;
         y+=Y_STEP;
      }
   }
}


void set_current_active(int activenum)
{
   int old = player_struct.current_active;
   known_actives[old] = -1;
   player_struct.current_active = activenum;
   known_actives[activenum] = -1;
}

// Get the item at a particular pixel y
int get_item_at_pixrow(inv_display *dp, int row)
{
   int linenum = dp->relnum*dp->pgsize;
   int ipanel_y;
   int y_step;
   int r1,r2;

#ifdef STEREO_SUPPORT
   if (convert_use_mode == 5)
   {
      switch (i6d_device)
      {
         case I6D_CTM:
            ipanel_y = 1;
            y_step = Y_STEP << 2;
            break;
         case I6D_VFX1:
            ipanel_y = INVENTORY_PANEL_Y >> 1;
//            ipanel_y = (200 - inv_fullscrn_canvas.bm.h);
//            y_step = Y_STEP << 1;
            y_step = 10;
            break;
         default:
            break;
      }
   }
   else
#endif
   {
      ipanel_y = INVENTORY_PANEL_Y;
      y_step = Y_STEP;
   }
   r1 = row;
   row -= ipanel_y + dp->top + y_step;
   if (row < 0) return -1;
   r2 = row;
   row /= y_step;
   if (row >= dp->pgsize) return -1;
//   Warning(("row = %d = (%d - %d + %d + %d) = %d / %d\n",row,r1,ipanel_y,dp->top,y_step,r2,y_step));

   return linenum+row;
}

// --------------------
// WEAPON DISPLAY FUNCS
// --------------------

#define WEAP_CLASSES (1 << CLASS_GUN)
#define WEAP_TRIP    MAKETRIP(CLASS_GUN,0,0)

char* weapon_name_func(void *, int num, char* buf)
{
   weapon_slot* ws = &player_struct.weapons[num];
   get_weapon_name(ws->type,ws->subtype,buf);
   return buf;
}

char* weapon_quant_func(int num, char* buf)
{
   int triple, num_ammo_types, ammo_subclass;
   ubyte ammo_types[3];
   int i = 0;
   weapon_slot* ws = &player_struct.weapons[num];
   bool energy = is_energy_weapon(ws->type);

   if (is_handtohand_weapon(ws->type))
   {
      buf[0] = '\0';
      return buf;
   }

   if ((!energy) && (ws->type != GUN_SUBCLASS_BEAMPROJ))
   {
      int ammo = ws->ammo;
      if (ws->ammo_type == EMPTY_WEAPON_SLOT)
      {
         ammo=0;
      }   
      
      get_available_ammo_type(ws->type,ws->subtype,&num_ammo_types,ammo_types,&ammo_subclass);

      triple = MAKETRIP(CLASS_AMMO,ammo_subclass,ws->ammo_type);
      if (ammo > 0) {
         buf[i++] = AMMO_TYPE_LETTER((CPTRIP(triple)));
         buf[i++] = ' ';
      }
      if(ammo==0 && num_ammo_types>0)
         get_string(REF_STR_AmmoLoad,buf,BUFSZ);
      else
      	numtostring(ammo, buf+i);
         //itoa(ammo,buf+i,10);
   }
   else
   {
      if (ws->heat > OVERHEAT_THRESHOLD)
         get_string(REF_STR_GunHot,buf,BUFSZ);
      else if (OVERLOAD_VALUE(ws->setting))
         get_string(REF_STR_AmmoOver,buf,BUFSZ);
      else if (ws->heat > WARM_THRESHOLD)
         get_string(REF_STR_GunWarm,buf,BUFSZ);
      else
         get_string(REF_STR_GunOK,buf,BUFSZ);
   }
   return buf;                      
}

void draw_weapons_list(inv_display *dp)
{
   bool newpage = inv_last_page != inventory_page;
   char buf[BUFSZ];
   int i,s;
   short y;
   gr_set_font((grs_font*)ResLock(WEAPONS_FONT));

   if (newpage)
   {
      gr_set_fcolor(dp->titlecolor);
      get_string(REF_STR_WeaponTitle,buf,BUFSZ);
      draw_inventory_string(buf,WEAPON_X,TOP_MARGIN,TRUE);
      get_string(REF_STR_AmmoTitle,buf,BUFSZ);
      draw_inventory_string(buf,AMMO_X-gr_string_width(buf),TOP_MARGIN,TRUE);
   }
   s = dp->relnum*WEAPONS_PER_PAGE;
   y = TOP_MARGIN + Y_STEP;
   for (i = 0; i < WEAPONS_PER_PAGE && s < NUM_WEAPON_SLOTS; i++,s++,y+=Y_STEP)
   {
      int num_ammo_types, dummy1;
      ubyte dummy2[3];
      bool avail;
      bool newactive = weapon_list.active != player_struct.actives[ACTIVE_WEAPON];
      bool changed;
      
      get_available_ammo_type(weapon_list.slots[s].type,weapon_list.slots[s].subtype,&num_ammo_types,dummy2,&dummy1);
      avail=(num_ammo_types>0);
      changed = newpage ||
                     (avail != weapon_list.ammo_available[s]) ||
                     memcmp(&weapon_list.slots[s],&player_struct.weapons[s],sizeof(weapon_slot)) != 0 ||  
                     (newactive && s == weapon_list.active) ||
                     (newactive && s == player_struct.actives[ACTIVE_WEAPON]) ;
      weapon_list.slots[s] = player_struct.weapons[s];
      weapon_list.ammo_available[s] = avail;
      if (!changed) continue;
      if (weapon_list.slots[s].type != EMPTY_WEAPON_SLOT)
      {
         char name[BUFSZ];
         char quant[BUFSZ];
         weapon_name_func(dp,s,name);
         weapon_quant_func(s,quant);
         draw_quant_line(name,quant,dp->listcolor,s == player_struct.actives[ACTIVE_WEAPON],WEAPON_X,AMMO_X,y);
      }
      else clear_inventory_region(WEAPON_X,y,AMMO_X,y+Y_STEP);
   }
   weapon_list.active = player_struct.actives[ACTIVE_WEAPON];
   ResUnlock(WEAPONS_FONT);
}


bool inventory_select_weapon(inv_display* dp, int w)
{
   bool retval = FALSE;
   int aw = player_struct.actives[ACTIVE_WEAPON];
#ifndef NO_DUMMIES
   inv_display* newdisp; newdisp = dp;
#endif // NO_DUMMIES
   if (player_struct.weapons[w].type == EMPTY_WEAPON_SLOT) goto out;
   if (aw != w)
   {
      weapon_slot* ws = &player_struct.weapons[w];
      play_digi_fx(SFX_INVENT_SELECT,1);
      change_selected_weapon(w);
      player_struct.last_fire = 0;
      player_struct.fire_rate = weapon_fire_rate(ws->type,ws->subtype);
   }
   player_struct.actives[ACTIVE_WEAPON] = w;
   set_inventory_mfd(MFD_INV_WEAPON,w,TRUE);
   set_inventory_mfd(MFD_INV_AMMO,0,TRUE);
   mfd_notify_func(NOTIFY_ANY_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,TRUE);
   INVENT_CHANGED;
   retval = TRUE;
out: 
   return retval;   
}

bool weapon_use_func(inv_display* dp, int w)
{
   if (player_struct.weapons[w].type == EMPTY_WEAPON_SLOT) return FALSE;
   inventory_select_weapon(dp,w);
   mfd_change_slot(mfd_grab_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT),MFD_WEAPON_SLOT);
   return TRUE;
}

ubyte weapons_add_func(inv_display* dp, int row, ObjID* objP,bool select)
{
   ubyte retval = ADD_REJECT;
   ObjID obj = *objP;
   weapon_slot* ws;
   weapon_slot tmp;
   ObjSpecID spec;
   play_digi_fx(SFX_INVENT_ADD, 1);
   row += dp->pgsize*dp->relnum;
   if (player_struct.weapons[NUM_WEAPON_SLOTS-1].type == EMPTY_WEAPON_SLOT)
      row = NUM_WEAPON_SLOTS-1;
   if (row < 0 || row >= NUM_WEAPON_SLOTS ||
      player_struct.weapons[row].type == EMPTY_WEAPON_SLOT)
   {
      for (row = 0; row < NUM_WEAPON_SLOTS; row++)
         if (player_struct.weapons[row].type == EMPTY_WEAPON_SLOT)
            break;
      if (row >= NUM_WEAPON_SLOTS)
      {
         string_message_info(REF_STR_InvNoRoom);
         return ADD_FAIL;
      }
   }
   ws = &player_struct.weapons[row];
   tmp = *ws;
   spec = objs[obj].specID;


   ws->type = objs[obj].subclass;
   ws->subtype = objs[obj].info.type;
   ws->ammo = objGuns[spec].ammo_count;
   ws->ammo_type = objGuns[spec].ammo_type;
   if (player_struct.actives[ACTIVE_WEAPON] == row)
      set_inventory_mfd(MFD_INV_WEAPON,row,TRUE);
   if (tmp.type != EMPTY_WEAPON_SLOT)
   {
      objs[obj].subclass = tmp.type;
      objs[obj].info.type = tmp.subtype;
      objGuns[spec].ammo_type = tmp.ammo_type;
      objGuns[spec].ammo_count = tmp.ammo;
      retval = ADD_SWAP;
   }
   else
   {
      obj_destroy(obj);
      retval = ADD_POP;
   }
   if (select)
      inventory_select_weapon(dp, row);

   if (player_struct.actives[ACTIVE_WEAPON] == row)
   {
      set_inventory_mfd(MFD_INV_WEAPON,row,TRUE);
      mfd_notify_func(MFD_WEAPON_FUNC,MFD_WEAPON_SLOT,TRUE,MFD_ACTIVE,TRUE);
   }
   // in case ammo mfd
   mfd_notify_func(NOTIFY_ANY_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,TRUE);
   return retval;
}

void weapon_drop_func(inv_display* dp, int itemnum)
{
   weapon_slot *ws;
   ObjID obj;
   ObjSpecID spec;
   int it;

   if (itemnum < 0 || itemnum >= dp->listlen) return;
   ws = &player_struct.weapons[itemnum];
   if (ws->type == EMPTY_WEAPON_SLOT) return;
   obj = obj_create_base(MAKETRIP(CLASS_GUN,ws->type,ws->subtype));
   if (obj == OBJ_NULL)
   {
      return;
   }
   spec = objs[obj].specID;
   objGuns[spec].ammo_type = ws->ammo_type;
   objGuns[spec].ammo_count = ws->ammo;
   push_cursor_object(obj);
   // preserve selected weapon.
   if(itemnum<player_struct.actives[ACTIVE_WEAPON])
      player_struct.actives[ACTIVE_WEAPON]--;
   for (it=itemnum+1; it < NUM_WEAPON_SLOTS;it++)
   {
      player_struct.weapons[it-1] = player_struct.weapons[it];
   }
   player_struct.weapons[NUM_WEAPON_SLOTS-1].type = EMPTY_WEAPON_SLOT;
   if(itemnum>0 && player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]].type==EMPTY_WEAPON_SLOT)
      player_struct.actives[ACTIVE_WEAPON]--;

   // in case ammo mfd
   mfd_notify_func(NOTIFY_ANY_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, TRUE);
   // In case the weapons MFD was looking at that weapon, nix it
   if (player_struct.weapons[player_struct.actives[ACTIVE_WEAPON]].type == EMPTY_WEAPON_SLOT)
      mfd_notify_func(MFD_EMPTY_FUNC, MFD_WEAPON_SLOT, TRUE, MFD_EMPTY, TRUE);
   else
      set_inventory_mfd(MFD_INV_WEAPON,player_struct.actives[ACTIVE_WEAPON],TRUE);
   INVENT_CHANGED;
}

// -------------
// GENERIC FUNCS
// -------------
extern int nth_after_triple(int,uchar);


static char* generic_name_func(void* vdp, int num, char* buf)
{
   int trip = nth_after_triple(((inv_display *)vdp)->basetrip,num);
   get_object_short_name(trip,buf,BUFSZ);
   return buf;
}

static void generic_draw_list(inv_display *dp)
{
   bool newpage = inv_last_page != inventory_page;
   gr_set_font((grs_font*)ResLock(ITEM_FONT));

   draw_quant_list(dp,newpage);
   ResUnlock(ITEM_FONT);
}

ubyte generic_add_func(inv_display* dp, int row, ObjID* idP,bool select)
{
   ObjID id = *idP;
   int trip = ID2TRIP(id);
   int n = OPTRIP(trip) - OPTRIP(dp->basetrip);
   int obclass = objs[id].obclass;
#ifndef NO_DUMMIES
   int guf; guf = row;
#endif // NO_DUMMIES
   play_digi_fx(SFX_INVENT_ADD, 1);
   if (n >= 0 && n < dp->listlen)
   {
      ubyte* quants = (ubyte *)&player_struct + dp->offset;
      quants[n]++;
      if (dp->activenum != NULL_ACTIVE)
      {
         if (select)
         {
            player_struct.actives[dp->activenum] = n;
            play_digi_fx(SFX_INVENT_SELECT,1);
         }
         if (select || n == player_struct.actives[dp->activenum])
            set_inventory_mfd(dp->mfdtype,n,TRUE);
      }
      obj_destroy(id);
      // This is a special-case hack for cartridges.
      if(obclass==CLASS_AMMO) {
         mfd_notify_func(NOTIFY_ANY_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, TRUE);
         mfd_notify_func(MFD_WEAPON_FUNC, MFD_WEAPON_SLOT, FALSE, MFD_ACTIVE, TRUE);
         INVENT_CHANGED;
      }
      return ADD_POP;
   }
   return ADD_REJECT;
}

void generic_drop_func(inv_display* dp, int row)
{
   int itemnum = dp->lines[row].num;
   ObjID obj;
   int triple;
   ubyte* quant;
   if (itemnum < 0 || itemnum >= dp->listlen) return;
   quant = (ubyte *)&player_struct + dp->offset;
   if (quant[itemnum] == 0) return;
   quant[itemnum]--;
   triple = nth_after_triple(dp->basetrip,itemnum);
   obj = obj_create_base(triple);
   if (obj == OBJ_NULL)
   {
      return;
   }
   push_cursor_object(obj);
   if (dp->activenum != NULL_ACTIVE && player_struct.actives[dp->activenum] == itemnum && quant[itemnum] == 0)
   {
      player_struct.actives[dp->activenum] = 0xFF;
      set_inventory_mfd(dp->mfdtype,MFD_INV_NOTYPE,FALSE);
   }
   INVENT_CHANGED;
}

static char* generic_quant_func(inv_display* dp, int n, int q, char* buf)
{
#ifndef NO_DUMMIES
   int dummy ;
   dummy = n + (int)dp;
#endif //NO_DUMMIES
 //  itoa(q,buf,10);
   numtostring(q, buf);
   return buf;
}

char* null_name_func(inv_display* dp, int n, char* buf)
{
#ifndef NO_DUMMIES   
   int goof; goof = (int)dp + n;
#endif // NO_DUMMIES
   *buf = '\0';
   return buf;
}
 
// -------------
// GRENADE FUNCS
// -------------

#define GREN_CLASSES (1 << CLASS_GRENADE)
#define GREN_TRIP    MAKETRIP(CLASS_GRENADE,0,0)

static char* grenade_name_func(void *, int n, char* buf)
{
   return get_grenade_name(n,buf);
}

extern uiSlab fullscreen_slab;
extern uiSlab main_slab;

grs_bitmap grenade_bmap;
#ifdef SVGA_SUPPORT
char grenade_bmap_buffer[8700];
#else
char grenade_bmap_buffer[700];
#endif

void push_live_grenade_cursor(ObjID obj)
{
   short w,h;
   extern LGCursor object_cursor;
   char live_string[22];
#ifdef CURSOR_BACKUPS
   extern grs_bitmap backup_object_cursor;
   extern uchar *backup[NUM_BACKUP_BITS];
#endif
#ifdef SVGA_SUPPORT
   uchar old_over = gr2ss_override;
   short temp;
#endif
   grs_canvas cursor_canvas;
   LGPoint hotspot;
   grs_bitmap* bmap = bitmaps_2d[OPNUM(obj)];
   void* bits = grenade_bmap_buffer;

   grenade_bmap = *bmap;
   gr_set_font((grs_font*)ResLock(ITEM_FONT));
   get_string(REF_STR_WordLiveGrenade,live_string,sizeof(live_string));
   gr_string_size(live_string,&w,&h);
   w++;h++; // compensate for shadowing
#ifdef SVGA_SUPPORT
   gr2ss_override = OVERRIDE_ALL;
   ss_set_hack_mode(2, &temp);
   if (convert_use_mode != 0)
   {
      grenade_bmap.w = max(bmap->w,w);
      grenade_bmap.h = bmap->h+h;
      grenade_bmap.w = SCONV_X(grenade_bmap.w);
      grenade_bmap.h = SCONV_Y(grenade_bmap.h);
   }
   else
   {
#endif
      grenade_bmap.w = max(bmap->w,w);
      grenade_bmap.h = bmap->h+h;
#ifdef SVGA_SUPPORT
   }
#endif
//   mprintf("bsize = %d, w * h = %d\n",sizeof(grenade_bmap_buffer), grenade_bmap.w * grenade_bmap.h);
   if (sizeof(grenade_bmap_buffer) < grenade_bmap.w*grenade_bmap.h)
      critical_error(0x3006);

   gr_init_bitmap(&grenade_bmap,(uchar *)bits,grenade_bmap.type,grenade_bmap.flags,grenade_bmap.w,grenade_bmap.h);
   gr_init_canvas(&cursor_canvas,(uchar *)bits,BMT_FLAT8,grenade_bmap.w,grenade_bmap.h);
   gr_push_canvas(&cursor_canvas);
   gr_set_font((grs_font*)ResGet(ITEM_FONT));
   gr_clear(0);
#ifdef SVGA_SUPPORT
   if (convert_use_mode > 0)
   {
      ss_bitmap(bmap,(INV_SCONV_X(grenade_bmap.w) - bmap->w)/2,0);
      gr_set_fcolor(0x4c);
      draw_shadowed_string(live_string,(INV_SCONV_X(grenade_bmap.w) - w)/2,
         INV_SCONV_Y(grenade_bmap.h) - h,TRUE);
   }
   else
   {
#endif
      ss_bitmap(bmap,(grenade_bmap.w - bmap->w)/2,0);
      gr_set_fcolor(0x4c);
      draw_shadowed_string(live_string,(grenade_bmap.w - w)/2+1,
         grenade_bmap.h - h,TRUE);
#ifdef SVGA_SUPPORT
   }
#endif
   ResUnlock(ITEM_FONT);            
   gr_pop_canvas();
#ifdef SVGA_SUPPORT
   ss_set_hack_mode(0,&temp);
   gr2ss_override = old_over;
   if (convert_use_mode != 0)
   {
      hotspot.x = grenade_bmap.w/2;
      hotspot.y = grenade_bmap.h/2;
   }
   else
   {
#endif
      hotspot.x = grenade_bmap.w/2;
      hotspot.y = grenade_bmap.h/2;
#ifdef SVGA_SUPPORT
   }
#endif
   uiHideMouse(NULL);
   uiMakeBitmapCursor(&object_cursor,&grenade_bmap,hotspot);
   uiPushSlabCursor(&fullscreen_slab,&object_cursor);
   uiPushSlabCursor(&main_slab,&object_cursor);
   uiShowMouse(NULL);
   object_on_cursor = obj;
   input_cursor_mode = INPUT_OBJECT_CURSOR;
}

bool grenade_use_func(inv_display* dp, int row)
{
   int itemnum = dp->lines[row].num;
   ObjID obj;
   int triple;
   ubyte* quant;
   ObjSpecID spec;
   if (itemnum < 0 || itemnum >= dp->listlen) return(FALSE);
   quant = (ubyte *)&player_struct + dp->offset;
   if (quant[itemnum] == 0) return(FALSE);
   quant[itemnum]--;
   triple = nth_after_triple(dp->basetrip,itemnum);
   obj = obj_create_base(triple);
   if (obj == OBJ_NULL)
   {
      return(FALSE);
   }
   if (dp->activenum != NULL_ACTIVE && player_struct.actives[dp->activenum] == itemnum && quant[itemnum] == 0)
   {
      player_struct.actives[dp->activenum] = 0xFF;
      set_inventory_mfd(dp->mfdtype,MFD_INV_NOTYPE,FALSE);
   }
   INVENT_CHANGED;
   push_live_grenade_cursor(obj);
   spec = objs[obj].specID;
   activate_grenade(spec);
   return(TRUE);
}

ubyte grenade_add_func(inv_display* dp, int row, ObjID* idP, bool select)
{
   extern errtype string_message_info(int strnum);
   ObjSpecID sid = objs[*idP].specID;
   play_digi_fx(SFX_INVENT_ADD, 1);
   if (objGrenades[sid].flags & GREN_ACTIVE_FLAG)
   {
      string_message_info(REF_STR_InvLiveGrenade);
      return ADD_FAIL;
   }
   return generic_add_func(dp,row,idP,select);
}

// ----------
// DRUG FUNCS
// ----------
#define DRUG_CLASSES (1 << CLASS_DRUG)
#define DRUG_TRIP MAKETRIP(CLASS_DRUG,0,0)

char* drug_name_func(inv_display* dp, int n, char* buf)
{
#ifndef NO_DUMMIES
   inv_display* dummy; dummy = dp;
#endif // NO_DUMMIES
   return get_drug_name(n,buf);
}


bool drug_use_func(inv_display* dp, int row)
{
   bool retval = FALSE;
   int n = dp->lines[row].num;
   if (n < dp->listlen)
   {
      drug_use(n);
      set_inventory_mfd(dp->mfdtype,n,TRUE);
      retval = TRUE;
      INVENT_CHANGED;
   }
   return retval;
}


// ----------
// AMMO FUNCS
// ----------
#define AMMO_CLASSES (1 << CLASS_AMMO)
#define AMMO_TRIP    MAKETRIP(CLASS_AMMO,0,0)

char* ammo_name_func(void* , int n, char* buf)
{
   int triple;

   buf[0] = AMMO_TYPE_LETTER(n);
   buf[1] = ' ';
   triple = get_triple_from_class_nth_item(CLASS_AMMO,n);
   get_object_short_name(triple,buf+2,31);
   return buf;
}


// ---------
// HARDWARES
// ---------
#define HARD_CLASSES (1 << CLASS_HARDWARE)
#define HARD_TRIP    MAKETRIP(CLASS_HARDWARE,0,0)
#define HARDWARE_PAGES 2


static char* ware_name_func(void* vdp,int n, char* buf)
{
   int type;
   switch(((inv_display *)vdp)->activenum)
   {
   case ACTIVE_HARDWARE:
      type = WARE_HARD;
      break;
   case ACTIVE_COMBAT_SOFT:
      type = WARE_SOFT_COMBAT;
      break;
   case ACTIVE_DEFENSE_SOFT:
      type = WARE_SOFT_DEFENSE;
      break;
   case ACTIVE_MISC_SOFT:
      type = WARE_SOFT_MISC;
      break;
   }
   get_ware_name(type,n,buf,BUFSZ);
   return NULL;
}

static bool ware_use_func(inv_display* dp,int row)
{
   int waretype;
   int t = dp->lines[row].num;
   if (t >= dp->listlen) return FALSE;
   switch(dp->activenum)  
   {
   case ACTIVE_HARDWARE:
      waretype = WARE_HARD;
      break;
   case ACTIVE_COMBAT_SOFT:
      waretype = WARE_SOFT_COMBAT;
      break;
   case ACTIVE_DEFENSE_SOFT:
      waretype = WARE_SOFT_DEFENSE;
      break;
   case ACTIVE_MISC_SOFT:
      waretype = WARE_SOFT_MISC;
      break;
   }
   use_ware(waretype,t);
   return TRUE;
}


void hardware_add_specials(int n, int ver)
{
   extern WARE HardWare[NUM_HARDWAREZ];
   extern void gamescr_bio_func(void);
   switch (n)
   {
   case HARDWARE_AUTOMAP:
      mfd_notify_func(MFD_MAP_FUNC, MFD_MAP_SLOT, TRUE, MFD_ACTIVE, TRUE);
      {
         int i;
         for (i=0; i<NUM_O_AMAP; i++)
            amap_version_set(i,player_struct.hardwarez[HARDWARE_AUTOMAP]);
      }
      break;
   case HARDWARE_TARGET:
      mfd_notify_func(MFD_TARGET_FUNC, MFD_TARGET_SLOT, TRUE, MFD_ACTIVE, TRUE);
      break;
   case HARDWARE_SHIELD:
   {
      extern int energy_cost(int warenum);
      extern void shield_set_absorb(void);
      int ener=0;
      if(WareActive(player_struct.hardwarez_status[CPTRIP(SHIELD_HARD_TRIPLE)]))
         ener=energy_cost(CPTRIP(SHIELD_HARD_TRIPLE));
      SHIELD_SETTING_SET(player_struct.hardwarez_status[n],ver-1);
      if(ener) {
         shield_set_absorb();
         ener=energy_cost(CPTRIP(SHIELD_HARD_TRIPLE))-ener;
         set_player_energy_spend(min(MAX_ENERGY,player_struct.energy_spend+ener));
      }
      mfd_notify_func(MFD_SHIELD_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);
   }
      break;
   case HARDWARE_ENVIROSUIT:
      {
         extern void zoom_to_lean_meter(void);
         if (_current_loop == GAME_LOOP)
            gamescr_bio_func();
         zoom_to_lean_meter();
      }
      break;
   // Flash the email sideicon the first time you pick up the data reader to tell the player he has a log.
   // After that, it is never flashed again if a new log is picked up.
   case HARDWARE_EMAIL:
      player_struct.hardwarez_status[HARDWARE_EMAIL] |= WARE_FLASH;
      QUESTBIT_ON(0x12c);
      break;
   }
   if (HardWare[n].sideicon != SI_NONE)
   {
      extern void zoom_to_side_icon(LGPoint from, int icon);
      LGPoint from;
      ui_mouse_get_xy(&from.x,&from.y);
      zoom_to_side_icon(from,HardWare[n].sideicon);
   }
}

ubyte ware_add_func(inv_display* dp, int, ObjID* idP,bool select)
{
   ObjID id = *idP;
   int trip = ID2TRIP(id);
   bool oneshot; 
   bool bigstuff_fake=FALSE;
   int n;
   extern bool shameful_obselete_flag;

   if (global_fullmap->cyber && (objs[id].obclass == CLASS_BIGSTUFF))
   {
      bigstuff_fake = TRUE;
      trip = MAKETRIP(CLASS_SOFTWARE, objBigstuffs[objs[id].specID].data1, objBigstuffs[objs[id].specID].data2);
   }
   n = OPTRIP(trip) - OPTRIP(dp->basetrip);
   oneshot = TRIP2CL(trip) == CLASS_SOFTWARE && TRIP2SC(trip) == SOFTWARE_SUBCLASS_ONESHOT;
   if (TRIP2CL(trip) == CLASS_SOFTWARE && TRIP2SC(trip) == SOFTWARE_SUBCLASS_DATA)
      return ADD_REJECT;
   if (n >= 0 && n < dp->listlen)
   {
      ubyte ver;
      ubyte* quants = (ubyte *)&player_struct + dp->offset;
      if (bigstuff_fake)
         ver = objBigstuffs[objs[id].specID].cosmetic_value;
      else
         ver = (TRIP2CL(trip) == CLASS_HARDWARE) ?
            objHardwares[objs[id].specID].version
            : objSoftwares[objs[id].specID].version;

      if (trip == GAMES_TRIPLE)
         quants[n] |= ver;
      else if (oneshot) quants[n]++;
      else if (quants[n] >= ver)
      {
         string_message_info(REF_STR_AlreadyHaveOne);
         shameful_obselete_flag=TRUE;
         return ADD_NOEFFECT;
      }
      else quants[n] = ver;

      play_digi_fx(SFX_INVENT_WARE, 1);
      if (select)
      {
         player_struct.actives[dp->activenum] = n;
      }
      if (select || n == player_struct.actives[dp->activenum])
         set_inventory_mfd(dp->mfdtype,n,TRUE);
      obj_destroy(id);
 
      // Tell the side icons that things may no longer be what they were
//      side_icon_expose_all();

      // If we picked up an automapper unit, let mfd slot know
      if (TRIP2CL(trip) == CLASS_HARDWARE)
      {
         hardware_add_specials(n,ver);
      }                          
      return ADD_POP;
   }
   return ADD_REJECT;

}

void ware_drop_func(inv_display*, int)
{
#ifndef GAMEONLY
   int itemnum = dp->lines[row].num;
   ObjID obj;
   int triple;
   bool oneshot;
   ubyte* quant;
   extern int nth_after_triple(int,uchar);
   if (itemnum < 0 || itemnum >= dp->listlen) return;
   quant = (ubyte *)&player_struct + dp->offset;
   if (quant[itemnum] == 0) return;
   triple = nth_after_triple(dp->basetrip,itemnum);
   oneshot = TRIP2CL(triple) == CLASS_SOFTWARE && TRIP2SC(triple) == SOFTWARE_SUBCLASS_ONESHOT;
   obj = obj_create_base(triple);
   if (obj == OBJ_NULL)
   {
      return;
   }
   if (dp->mfdtype == MFD_INV_HARDWARE)
      objHardwares[objs[obj].specID].version = quant[itemnum];
   else
      objSoftwares[objs[obj].specID].version = quant[itemnum];
   // If the ware was on, turn it off
   if ((dp->mfdtype == MFD_INV_HARDWARE) &&
       (player_struct.hardwarez_status[itemnum] & WARE_ON))
      use_ware(WARE_HARD, itemnum); // actually toggles, not uses
   if (oneshot) quant[itemnum]--;
   else quant[itemnum] = 0;
   push_cursor_object(obj);
   if (player_struct.actives[dp->activenum] == itemnum)
   {
      player_struct.actives[dp->activenum] = 0xFF;
      // Tell the item mfd that what it was looking at may no longer be there
      set_inventory_mfd(dp->mfdtype,MFD_INV_NOTYPE,FALSE);
   }
   INVENT_CHANGED;


   // Tell the side icons that things are no longer what they were
   side_icon_expose_all();


   mfd_notify_func(NOTIFY_ANY_FUNC, MFD_ITEM_SLOT, FALSE, MFD_ACTIVE, FALSE);

   // If we no longer have an automapper, let the mfd know
   if (player_struct.hardwarez[HARDWARE_AUTOMAP] == 0)
      mfd_notify_func(MFD_EMPTY_FUNC, MFD_MAP_SLOT, TRUE, MFD_EMPTY, TRUE);
#endif // !GAME_ONLY
}


char* null_quant_func(inv_display*, int, int, char* buf)
{
   *buf = '\0';
   return buf;
}


// -----
// SOFTS
// -----

#define SOFT_PAGES 2
#define SOFT_CLASSES (1 << CLASS_SOFTWARE)
#define COMSOFT_TRIP MAKETRIP(CLASS_SOFTWARE,SOFTWARE_SUBCLASS_OFFENSE,0)
#define DEFSOFT_TRIP MAKETRIP(CLASS_SOFTWARE,SOFTWARE_SUBCLASS_DEFENSE,0)
#define MISCSOFT_TRIP MAKETRIP(CLASS_SOFTWARE,SOFTWARE_SUBCLASS_ONESHOT,0)


#define VERSION_PREFIX (get_temp_string(REF_STR_VersionPrefix)[0])

// QUANTS *******************************
static char* soft_quant_func(inv_display*, int, int q, char* buf)
{
   int l=1;
   buf[0] = VERSION_PREFIX;
   if (q<10)
      buf[l++]=q+'0';
   buf[l]='\0';
   return buf;
}


// COMPUTRON SUPPORT

#define CTRON_WD 10

#ifdef COMPUTRONS
char* computron_quant_func(inv_display* dp, int n, int q, char* buf)
{
#ifdef REALLY_DO_COMPUTRONS
   ubyte exists, ctrons;

   switch (dp->mfdtype)     
   {
   case MFD_INV_SOFT_COMBAT:
      exists = player_struct.softs.combat[n];
      ctrons = player_struct.softs_ctrons.combat[n];
      break;
   case MFD_INV_SOFT_DEFENSE:
      exists = player_struct.softs.defense[n];
      ctrons = player_struct.softs_ctrons.defense[n];
      break;
   case MFD_INV_SOFT_MISC:
      exists = player_struct.softs.misc[n];
      ctrons = player_struct.softs_ctrons.misc[n];
      break;
   }
   if (exists == 0 || ctrons == 0)
      *buf = '\0';
   else
      itoa(ctrons,buf,10);
#endif
   *buf = '\0';
   return buf;
}
#endif // COMPUTRONS

// ------------------------------
// GENERAL INVENTORY -- FUN! FUN! 
// ------------------------------

#define GENERAL_CLASSES 0xFFFFFF80
static ObjID general_lines[NUM_GENERAL_SLOTS];


#define GARBAGE_COLOR TITLE_COLOR


void draw_general_list(inv_display *dp)
{
   bool newpage = inv_last_page != inventory_page;
   char buf[BUFSZ];
   int i,s;
   short y;
   ubyte active =  player_struct.actives[dp->activenum];
   ubyte known_active = known_actives[dp->activenum];
   bool newactive = known_active != active;

   gr_set_font((grs_font*)ResLock(ITEM_FONT));

   if (newpage)
   {
      gr_set_fcolor(dp->titlecolor);
      get_string(dp->titlenum,buf,BUFSZ);
      draw_inventory_string(buf,dp->left,dp->top,TRUE);
   }
   s = dp->relnum*dp->pgsize;
   y = dp->top + Y_STEP;
   for (i = 0; i < dp->pgsize && s < dp->listlen; i++,s++,y+=Y_STEP)
   {
      bool curractive = player_struct.current_active == dp->activenum;
      bool changed = newpage
                     || general_lines[s] != player_struct.inventory[s]
                     || newactive;
      general_lines[s] = player_struct.inventory[s];
      if (!changed) continue;
      if (general_lines[s] != OBJ_NULL)
      {
         ulong color = dp->listcolor;
         char name[BUFSZ];
         get_object_short_name(ID2TRIP(general_lines[s]),name,BUFSZ);
         if (!(ObjProps[OPNUM(general_lines[s])].flags & INVENTORY_GENERAL))
            color = GARBAGE_COLOR;
         draw_quant_line(name,"",color,s == active && curractive,dp->left,dp->right,y);
      }
      else clear_inventory_region(dp->left,y,dp->right,y+Y_STEP);
   }
   ResUnlock(ITEM_FONT);
}


bool general_use_func(inv_display* dp, int row)
{
   ObjID id = player_struct.inventory[row];
   extern errtype object_use(ObjID id, bool in_inv, ObjID cursor_obj);
   if (id == OBJ_NULL) return FALSE;
   if (ObjProps[OPNUM(id)].flags & OBJECT_USE_NOCURSOR)
   {
      object_use(id,TRUE,object_on_cursor);
      // only change mfd if using object did not destroy it.
      if (player_struct.current_active == dp->activenum
          && row <= player_struct.actives[dp->activenum])
         set_inventory_mfd(dp->mfdtype,row,TRUE);
      if(player_struct.inventory[row]==id) {
         set_inventory_mfd(dp->mfdtype,row,TRUE);
         mfd_change_slot(mfd_grab_func(MFD_ITEM_FUNC,MFD_ITEM_SLOT),MFD_ITEM_SLOT);
      }
   }
   else
      if (dp->drop != NULL)
         dp->drop(dp,row);
   return TRUE;
}

ubyte inv_empty_trash(void)
{
   bool found = FALSE;
   ubyte non_trash = 0;
   ubyte trash;
   ubyte last_trash = NUM_GENERAL_SLOTS;
   // find the first trash object
   for (trash = 0; trash < NUM_GENERAL_SLOTS; trash++)
   {
      ObjID id = player_struct.inventory[trash];
      if (id != OBJ_NULL && !(ObjProps[OPNUM(id)].flags & INVENTORY_GENERAL))
      {
         found = TRUE;
         break;
      }
   }
   if (!found) return last_trash;
   // find the next non-trash object
   for (non_trash = trash; non_trash < NUM_GENERAL_SLOTS; non_trash++)
   {
      ObjID id = player_struct.inventory[non_trash];
      if (id != OBJ_NULL && (ObjProps[OPNUM(id)].flags & INVENTORY_GENERAL))
         break;
   }
   // iterate through, destroying trash. 
   for (; trash < NUM_GENERAL_SLOTS; trash++)
   {
      ObjID id = player_struct.inventory[trash];
      bool is_trash = !(ObjProps[OPNUM(id)].flags & INVENTORY_GENERAL);
      if (is_trash)
         obj_destroy(id);
      if (is_trash || id == OBJ_NULL)
      {
         if (non_trash < NUM_GENERAL_SLOTS)
         {
            player_struct.inventory[trash] = player_struct.inventory[non_trash];
            player_struct.inventory[non_trash] = OBJ_NULL;
            for (; non_trash < NUM_GENERAL_SLOTS; non_trash++)
            {
               ObjID id = player_struct.inventory[non_trash];
               if (id != OBJ_NULL && (ObjProps[OPNUM(id)].flags & INVENTORY_GENERAL))
                  break;
            }
         }
         else
         {
            player_struct.inventory[trash] = OBJ_NULL;
            last_trash = min(last_trash,trash);
         }
      }
   }
   return last_trash;
}

ubyte add_access_card(inv_display* dp, ObjID* idP,bool select)
{
   ubyte retval = ADD_NOEFFECT;
   int i,d1,old_d1,gain;
   ObjID cards;
   for (i = 0; i < dp->listlen; i++)
   {
      if (ID2TRIP(player_struct.inventory[i]) == GENCARDS_TRIPLE
         || player_struct.inventory[i] == OBJ_NULL)
         break;
   }
   if (i >= dp->listlen)
      i = inv_empty_trash();
   if (i >= dp->listlen)
      return ADD_FAIL;

   // Extract the data out of the old card, so that we can copy it
   // correctly into the new set of cards.  Destroy the old one first
   // so that if we are right on the border of number of cards available
   // in the universe, we don't die.
   d1 = objSmallstuffs[objs[*idP].specID].data1;
   obj_destroy(*idP);

   if (player_struct.inventory[i] == OBJ_NULL)
   {
      cards = obj_create_base(GENCARDS_TRIPLE);
      if (cards == OBJ_NULL)
         return ADD_FAIL;
      player_struct.inventory[i] = cards;
      retval = ADD_POP;
   }
   else cards = player_struct.inventory[i];
   old_d1=objSmallstuffs[objs[cards].specID].data1;
   objSmallstuffs[objs[cards].specID].data1 |= d1;
   if (select && dp->select != NULL)
      dp->select(dp,i);
   gain=d1&(~old_d1);
   if(gain==0)
      string_message_info(REF_STR_AccessCardNoGain);
   else
   {
      char gainbuf[80], bitname[16];
      int l,bitgot;

      get_string(REF_STR_AccessCardNewGain,gainbuf,80);
      l=strlen(gainbuf);

      bitgot=0;
      while(gain!=0)
      {
         if(gain&1)
         {
            get_string(MKREF(RES_accessCards,bitgot<<1),bitname,sizeof(bitname));
            if(l+strlen(bitname)+1<80)
            {
               strcat(gainbuf,bitname);
               strcat(gainbuf," ");
               l+=strlen(bitname)+1;
            }
         }
         gain=gain>>1;
         bitgot=bitgot+1;
      }
      gainbuf[l]='\0';

      message_info(gainbuf);
   }
   return retval;
}

ubyte general_add_func(inv_display* dp, int row, ObjID* idP,bool select)
{
   play_digi_fx(SFX_INVENT_ADD, 1);
   if ((objs[*idP].obclass == CLASS_SMALLSTUFF) && 
      ((objs[*idP].subclass == SMALLSTUFF_SUBCLASS_CARDS) || (ID2TRIP(*idP) == CYBERCARD_TRIPLE)))
      return add_access_card(dp,idP,select);
   if (player_struct.inventory[NUM_GENERAL_SLOTS-1] == OBJ_NULL)
      row = NUM_GENERAL_SLOTS-1;
   if (row < 0 || row >= dp->listlen
      || player_struct.inventory[row] == OBJ_NULL)
      for (row = 0; row < dp->listlen; row++)
         if (player_struct.inventory[row] == OBJ_NULL)
            break;
   if (row >= dp->listlen) row = inv_empty_trash();
   if (row >= dp->listlen) return ADD_NOROOM;
   else
   {
      ObjID tmp = player_struct.inventory[row];
      // if we're trying to swap with the "access cards" object, 
      // use the next object instead.  Since there's only one "access cards" 
      // object, this works.
      if (ID2TRIP(tmp) == GENCARDS_TRIPLE)
      {
         row = (row + 1)%dp->listlen;
         tmp = player_struct.inventory[row];
      }
      player_struct.inventory[row] = *idP;
      if (select && dp->select != NULL)
         dp->select(dp,row);
      if (tmp != OBJ_NULL && ID2TRIP(tmp) != GENCARDS_TRIPLE)
      {
         *idP = tmp;
         return ADD_SWAP;
      }
      return ADD_POP;
   }
}

void remove_general_item(ObjID obj)
{
   extern errtype obj_tractor_beam_func(ObjID id, bool on);
   int row;
   int i;

   for (row = 0; row < NUM_GENERAL_SLOTS; row++)
      if (player_struct.inventory[row] == obj)
         break;
   if (row >= NUM_GENERAL_SLOTS) return;
   for (i = row+1; i < NUM_GENERAL_SLOTS; i++)
     player_struct.inventory[i-1] = player_struct.inventory[i];
   player_struct.inventory[NUM_GENERAL_SLOTS-1] = OBJ_NULL;

   // Turn off any active gear when it leaves our inventory.
   switch (ID2TRIP(obj))
   {
      case TRACBEAM_TRIPLE:
         if (objs[obj].info.inst_flags & CLASS_INST_FLAG)
            obj_tractor_beam_func(obj,FALSE);
         break;
   }
   if (player_struct.current_active == ACTIVE_GENERAL)
      set_inventory_mfd(MFD_INV_GENINV,player_struct.actives[ACTIVE_GENERAL],TRUE);
   mfd_notify_func(NOTIFY_ANY_FUNC,MFD_ITEM_SLOT,FALSE,MFD_ACTIVE,FALSE);
   // Redraw the panel, or setup thereof
   INVENT_CHANGED;
}


void general_drop_func(inv_display*, int row)
{
   ObjID obj = player_struct.inventory[row];
   if (obj != OBJ_NULL && ID2TRIP(obj) != GENCARDS_TRIPLE) // don't let us drop access cards.
   {
      // Put on cursor...
      push_cursor_object(obj);
      remove_general_item(obj);
   }
}

bool inv_select_general(inv_display* dp, int w)
{
   bool retval = FALSE;
   if (player_struct.inventory[w] == OBJ_NULL)
      goto out;
   player_struct.actives[dp->activenum] = w;
   set_inventory_mfd(dp->mfdtype,w,TRUE);
   INVENT_CHANGED;
   retval = TRUE;
out:
   return retval;
}

// -----
// EMAIL
// -----

errtype inventory_draw_new_page(int pgnum);
#define EMAIL_TRIP 0 // MAKETRIP(CLASS_SOFTWARE,SOFTWARE_SUBCLASS_EMAIL,0)

#define FIRST_DATA (NUM_EMAIL - NUM_DATA)

extern char* email_name_func(void* dp, int num, char* buf);

#define MORE_COLOR SELECTED_ITEM_COLOR

static bool email_morebuttons[2];

void email_more_draw(inv_display *dp)
{
   gr_set_font((grs_font*)ResLock(ITEM_FONT));

   if (dp->relnum % 2 == 1)
   {
      int i;
      int count = 0;
      for (i = 0; i < NUM_EMAIL_PROPER; i++)
         if (player_struct.email[i])
            count++;
      if (count > (dp->relnum+1)*dp->pgsize)
      {
         short y = dp->top+Y_STEP;
         char buf[50];
         get_string(REF_STR_EmailMoreRight,buf,sizeof(buf));
         draw_quant_line("",buf,MORE_COLOR,FALSE,dp->left,dp->right,y);
         email_morebuttons[1] = TRUE;
      }
      else email_morebuttons[1] = FALSE;
   }
   else if (dp->relnum != 0)
   {
      short y = dp->top + Y_STEP;
      char buf[50];
      get_string(REF_STR_EmailMoreLeft,buf,sizeof(buf));
      draw_quant_line(buf,"",MORE_COLOR,FALSE,dp->left,dp->right,y);
      email_morebuttons[0] = TRUE;
   }
   else email_morebuttons[0] = FALSE;
   ResUnlock(ITEM_FONT);
}


bool email_more_use(inv_display* dp, int)
{
   bool retval = FALSE;
   if (dp->relnum != 0 && email_morebuttons[dp->relnum % 2])
   {
      int newpage = (dp->relnum % 2 == 0) ? inventory_page - 1 : inventory_page + 1;
      inventory_page = newpage;
      INVENT_CHANGED;
      retval = TRUE;
   }
   return retval;
}


#define EMAIL_BASE_ID   RES_email0
#define TITLE_IDX       1

bool email_use_func(inv_display* dp, int row)
{
   extern void read_email(Id,int);
   bool retval = FALSE;
   int n = dp->lines[row].num;

   if (n < dp->listlen)
   {
      read_email(EMAIL_BASE_ID,n);
      retval = TRUE;
   }
   return retval;
}

extern void select_email(int num, bool scr_update);

void email_select_func(inv_display* dp, int row)
{
   int n = dp->lines[row].num;
   if (n < dp->listlen)
   {
      play_digi_fx(SFX_INVENT_SELECT,1);
      select_email(n,TRUE);
   }
}



void add_email_datamunge(short mung,bool select)
{
   extern void set_email_flags(int n);

   int n;
   bool flash_email = TRUE;
   ubyte ver;
   extern short last_email_taken;

   n = mung & 0xFF;
   ver = mung >> 8;
   switch (ver)
   {
   case EMAIL_VER:
      set_email_flags(n);
      break;
   case LOG_VER:
      {
         int lev;
         lev = n /LOGS_PER_LEVEL;
         n = NUM_EMAIL_PROPER + n;
         if (player_struct.email[n] == 0)
            player_struct.logs[lev]++;
         flash_email = FALSE;
      }
      break;
   case DATA_VER:
      flash_email = FALSE;
      n = n + NUM_EMAIL-NUM_DATA;
      break;
   }
   if (player_struct.email[n] & EMAIL_GOT) return;
   last_email_taken = ver;
   string_message_info(REF_STR_ReceiveEmail+ver);
   player_struct.email[n] |= EMAIL_GOT;
   if (flash_email) {
      player_struct.hardwarez_status[CPTRIP(VIDTEX_HARD_TRIPLE)] |= WARE_FLASH;
      QUESTBIT_ON(0x12c);
   }
   INVENT_CHANGED;
   select_email(n,select);
}

ubyte email_add_func(inv_display*, int, ObjID* idP,bool select)
{
   play_digi_fx(SFX_INVENT_ADD, 1);
   if (ID2TRIP(*idP) != EMAIL1_TRIPLE && ID2TRIP(*idP) != TEXT1_TRIPLE) return ADD_REJECT;
   add_email_datamunge(SOFTWARE_CONTENTS(objs[*idP].specID),select);
   return ADD_POP;
}


void email_drop_func(inv_display*, int )
{
   // For now, do nothing.
}


// ----
// LOGS
// ----

// I'm on your side; we are on the both side.

#define FIRST_LOG_PAGE 20

char* log_name_func(void*, int num, char* buf)
{
   return get_string(REF_STR_LogName0+num,buf,BUFSZ);
}


bool log_use_func(inv_display* dp, int row)
{
   bool retval = FALSE;
   int n = dp->lines[row].num;
   if (n < dp->listlen)
   {
      inventory_draw_new_page(FIRST_LOG_PAGE+n);
      retval = TRUE;
   }
   return retval;
}


// ---------
// INTERNALS   
// ---------

void inventory_draw_page(int pgnum)
{
   int i;
   inv_display* dpy;

   for (i = 0; gen_inv_page(pgnum,&i,&dpy); i++)
   {
      if (dpy->draw != NULL)
         dpy->draw(dpy);
   }
   for (i = 0; i < NUM_ACTIVES; i++)
      known_actives[i] = player_struct.actives[i];
}

ubyte add_to_some_page(ObjID obj,bool select)
{
   inv_display* dpy; 
   int i;
   for (i = 0; gen_inv_displays(&i,&dpy); i++)
   {
      ubyte pop;
      if (global_fullmap->cyber && (objs[obj].obclass == CLASS_BIGSTUFF))
      {
         if (!(dpy->add_classes & (1 << CLASS_SOFTWARE))) continue;
      }
      else
      {
         if (!(dpy->add_classes & (1 << objs[obj].obclass))) continue;
      }
      if (dpy->add != NULL)
         pop = dpy->add(dpy,-1,&obj,select);
      if (pop == ADD_FAIL) return pop;
      if (pop == ADD_NOROOM)
      {
         string_message_info(REF_STR_InvNoRoom);
         return pop;
      }
      if (pop != ADD_REJECT)
      {
         if (pop != ADD_NOEFFECT)
         {
            if (dpy->pgnum != inventory_page)
            {
               page_button_state[dpy->pgnum] = BttnFlashOn;
            }
            INVENT_CHANGED;
         }
         return pop;
      }
   }
   string_message_info(REF_STR_InvReject);
   return ADD_REJECT;
}

/*KLC - no longer used
void draw_page_button_panel()
{
   draw_page_buttons(TRUE);
}
*/

void draw_page_buttons(bool full)
{
   LGRect r,hider;
   int i;
   short x;
   uchar old_over = gr2ss_override;

   gr_push_canvas(ppage_canvas);
   if (full_game_3d)
      gr2ss_override = OVERRIDE_FAIL;
   else
      gr2ss_override = OVERRIDE_ALL;

   if (full)
   {
//      draw_res_bm(REF_IMG_bmInventoryButtonBackground,INVENT_BUTTON_PANEL_X, INVENT_BUTTON_PANEL_Y);
		draw_hires_resource_bm(REF_IMG_bmInventoryButtonBackground, 0, 0);
   }

   r.ul.y = INVENT_BTTN_Y;
   r.lr.y = r.ul.y + INVENT_BTTN_HT;

   for (x = FIRST_BTTN_X, i =0; i < NUM_PAGE_BUTTONS; i++, x +=BUTTON_X_STEP)
   {
      invent_bttn_state newstate = page_button_state[i];
      ulong clr;
      bool active = i == inventory_page;

      if (newstate == BttnDummy) continue;
      // Figure out what the button state really is. 
      if (active)
         newstate = BttnActive;
      else if (Flashing(newstate))
      {
         bool flashon = (player_struct.game_time / INVENT_BTTN_FLASH_TIME) %2 ;
         if (flashon != FlashOn(newstate))
         {
            newstate = (invent_bttn_state)((char)newstate - 1);
            if (newstate == BttnFlashOff)
               newstate = BttnOff;
         }
         // if (time_passes)   // We want to remember that we need to change when time starts again..
            INVENT_CHANGED;
      }
      else newstate = BttnOff;

      if (!full && newstate == old_button_state[i]) continue;

      clr = bttn_state2color(newstate);
      gr_set_fcolor(clr);
      r.ul.x = x;
      r.lr.x = x+INVENT_BTTN_WD;
      RECT_OFFSETTED_RECT(&r,MakePoint(INVENTORY_PANEL_X, BUTTON_PANEL_Y),&hider);
      uiHideMouse(&hider);
//KLC - chg for new art      ss_rect(r.ul.x,r.ul.y,r.lr.x,r.lr.y);
      gr_rect(SCONV_X(r.ul.x)+2, 2, SCONV_X(r.lr.x)+2, 10);
      uiShowMouse(&hider);
      page_button_state[i] = old_button_state[i] = newstate;
   }
   gr_pop_canvas();
   gr2ss_override = old_over;
}



// ---------
// EXTERNALS
// ---------

bool dirty_inv_canvas = FALSE;


errtype inventory_clear(void)
{
   gr_push_canvas(&inv_canvas);
   if (full_game_3d)
      gr_clear(0);
   else
   {
      LGRect r;
      r.ul.x = INVENTORY_PANEL_X;
      r.ul.y = INVENTORY_PANEL_Y;
      r.lr.x = INVENTORY_PANEL_X + INVENTORY_PANEL_WIDTH;
      r.lr.y = INVENTORY_PANEL_Y + INVENTORY_PANEL_HEIGHT;
      if (dirty_inv_canvas)
      {
         FrameDesc* f = (FrameDesc*)RefGet(REF_IMG_bmBlankInventoryPanel);
         LG_memcpy(inv_backgnd.bits,f+1,f->bm.w*f->bm.h);
         dirty_inv_canvas = FALSE;
      }
      uiHideMouse(&r);
      ss_safe_set_cliprect(0,0,INVENTORY_PANEL_WIDTH,INVENTORY_PANEL_HEIGHT);
//KLC - chg for new art      ss_bitmap(&inv_backgnd, 0,0);
	  gr_bitmap(&inv_backgnd, 0, 0);
      uiShowMouse(&r);
   }
   gr_pop_canvas();
   /* Now, you might ask "Why not just set inv_last_page = INV_BLANK_PAGE all the time?"
      And the answer is, well, the wrapper panel saves the inventory page in inv_last_page,
      so that things like load game know how to blow the saved page away */
   if (inventory_page == inv_last_page)
      inv_last_page = INV_BLANK_PAGE;
   return(OK);
}

errtype inventory_full_redraw()
{
   int i;
   inv_last_page = -1;
   for (i = 0; i < NUM_PAGE_BUTTONS; i++)
      old_button_state[i] = BttnDummy;
   return(inventory_draw());
}

errtype inventory_draw(void)
{
   bool full = inventory_page != inv_last_page;
#ifdef SVGA_SUPPORT
   uchar old_over;
   short temp;
#endif
   if (inventory_page < 0) return OK;
   gr_push_canvas(&inv_canvas);
#ifdef SVGA_SUPPORT
   old_over = gr2ss_override;
//   if (full_game_3d)
//      gr2ss_override = OVERRIDE_FONT|OVERRIDE_CLIP;
//   else
   gr2ss_override = OVERRIDE_ALL;
#endif
   if (global_fullmap->cyber)
      inventory_page = INV_SOFTWARE_PAGE;
   if (full)
      inventory_clear();
   draw_page_buttons(full_game_3d || full);
#ifdef SVGA_SUPPORT
   ss_set_hack_mode(2,&temp);
#endif
   inventory_draw_page(inventory_page);
#ifdef SVGA_SUPPORT
   ss_set_hack_mode(0,&temp);
   gr2ss_override = old_over;
#endif
   gr_pop_canvas();
   inv_last_page = inventory_page;
   return(OK);
}

errtype inventory_draw_new_page(int pgnum)
{
   inv_last_page = -1;
   inventory_page = pgnum;
   if (full_game_3d)
   {
#ifdef STEREO_SUPPORT
      if (convert_use_mode == 5)
         full_visible = FULL_INVENT_MASK;
      else
#endif
         full_visible |= FULL_INVENT_MASK;
      full_raise_region(inventory_region_full);
      chg_set_sta(FULLSCREEN_UPDATE);
   }
   return inventory_draw();

}

bool inventory_add_object(ObjID obj,bool select)
{
   ubyte result = add_to_some_page(obj,select);
   return (result != ADD_FAIL) && (result != ADD_REJECT) && (result != ADD_NOROOM);
}


// ------------
// EVENTHANDLER
// ------------

bool do_selection(inv_display* dp, int row)
{
   bool retval = FALSE;
   int w = dp->lines[row].num;
   if (w >= dp->listlen) goto out;
   if (dp->activenum == NULL_ACTIVE) goto out;
   player_struct.actives[dp->activenum] = w;
   set_inventory_mfd(dp->mfdtype,w,TRUE);
   INVENT_CHANGED;
   retval = TRUE;
out:
   return retval;
}

void add_object_on_cursor(inv_display* dp, int row)
{
	ObjID obj = object_on_cursor;
	ubyte pop = ADD_REJECT;
	if (dp != NULL)
		pop = (dp->add_classes & (1 << TRIP2CL(ID2TRIP(object_on_cursor)))) ? ADD_POP : ADD_REJECT;
	if (pop != ADD_REJECT && dp->add != NULL)
		pop = dp->add(dp,row,&obj,FALSE);
	if (pop == ADD_NOROOM)
	{
		string_message_info(REF_STR_InvNoRoom);
		return;
	}
	if (pop == ADD_REJECT)
		pop = add_to_some_page(obj,FALSE);
	if (pop != ADD_REJECT && pop != ADD_FAIL)
	{
		INVENT_CHANGED;
		if (IS_POP_RESULT(pop) || pop == ADD_SWAP)
			pop_cursor_object();
		if (pop == ADD_SWAP)
			push_cursor_object(obj);   
		uiShowMouse(NULL);					// KLC - added to make sure the pointer changes.
	}
	if (pop == ADD_REJECT)
		string_message_info(REF_STR_InvReject);
}

bool inventory_handle_leftbutton(uiEvent* ev, inv_display* dp, int row)
{
   bool retval = FALSE;
#ifndef NO_DUMMIES
   void* dummy; dummy = ev;
#endif // NO_DUMMIES
   switch (input_cursor_mode)
   {
   case INPUT_NORMAL_CURSOR:
      if (dp != NULL && row >= 0)
      {
         if (dp->select != NULL) retval = dp->select(dp,row);
         else retval = do_selection(dp,row);
      }
      break;
   case INPUT_OBJECT_CURSOR:
      add_object_on_cursor(dp,row);
      retval = TRUE;
      break;
   }
   return retval;
}


static bool invpanel_focus = FALSE;

bool inventory_handle_rightbutton(uiEvent* ev, LGRegion* reg, inv_display* dp, int row)
{
   static int lastrow = 0;
   static inv_display* lastdp = NULL;

   bool retval = FALSE;
   LGRect r;
   bool grab= FALSE;

   if (input_cursor_mode != INPUT_NORMAL_CURSOR) return FALSE;
   if (ev->subtype & MOUSE_RDOWN && row >= 0 && dp != NULL && !invpanel_focus)
   {
      // let us know if we leave the region
      invpanel_focus = TRUE;
      uiGrabFocus(reg,UI_EVENT_MOUSE_MOVE);
      lastrow = row;
      lastdp =dp;
      retval = TRUE;
   }
   // Check to see if we've left the region and release focus.  
   region_abs_rect(reg,reg->r,&r);
   if (!RECT_TEST_PT(&r,ev->pos))
   {
      grab = TRUE;
      row = lastrow;
      dp = lastdp;
      retval = TRUE;
   }
   if (ev->subtype & MOUSE_RUP)
   {
      if (row == lastrow && dp == lastdp)
         grab = TRUE;
   }
   else if (ev->subtype & MOUSE_MOTION && (row != lastrow || dp != lastdp))
   {
      row = lastrow;
      dp = lastdp;
      grab = TRUE;
   }
   if (grab && dp != NULL && row >= 0)
   {
      bool cyber = TRIP2CL(dp->basetrip) == CLASS_SOFTWARE;
      if (cyber != global_fullmap->cyber)
      {
         extern errtype string_message_info(int);
         int str = cyber ? REF_STR_InvCybFailSoft : REF_STR_InvCybFailHard;
         string_message_info(str);
      }
      else if (dp->drop != NULL)
      {
         dp->drop(dp,row);
      }
      lastrow = -1;
      lastdp = NULL;
      retval = TRUE;
   }
   return retval;
}

#define SEARCH_MARGIN 2

bool inventory_mouse_handler(uiEvent* ev, LGRegion* r, void*)
{
   bool retval = FALSE;
   uiMouseEvent *mev = (uiMouseEvent*) ev;
   int relx;
   inv_display* dp = NULL;
   int i;
   int row = -1;
   extern bool game_paused;
#ifdef SVGA_SUPPORT
   short temp;
#endif
   if (game_paused)
      return(TRUE);

#ifdef STEREO_SUPPORT
   if (convert_use_mode == 5)
   {
      if (i6d_device == I6D_CTM)
         relx = ev->pos.x - 1;
      else
      {
         relx = (ev->pos.x - ((320-inv_fullscrn_canvas.bm.w)/2)) >> 1;
//         Warning(("relx: %d = %d - %d = %d >> 1\n",relx,ev->pos.x,((320-inv_fullscrn_canvas.bm.w)/2),
//             (ev->pos.x - ((320-inv_fullscrn_canvas.bm.w)/2))));
      }
   }
   else
#endif
   {
      relx = ev->pos.x - INVENTORY_PANEL_X;
   }
   if (invpanel_focus && !(mev->buttons & (1 << MOUSE_RBUTTON)))
   {
      uiReleaseFocus(r,UI_EVENT_MOUSE_MOVE);
      invpanel_focus = FALSE;
   }
   if (full_game_3d && !(full_visible & FULL_INVENT_MASK))
      return FALSE;
   if (full_game_3d && !(mev->buttons & (1 << MOUSE_RBUTTON)))
      {
         if (!(mev->action & ~MOUSE_MOTION))
            return FALSE;
         if (input_cursor_mode != INPUT_OBJECT_CURSOR)
         {
            bool found = FALSE;
            short rel_y;
            short x,y;
            short smx,smy;
#ifdef STEREO_SUPPORT
            if (convert_use_mode == 5)
            {
               switch (i6d_device)
               {
                  case I6D_CTM:
                     rel_y = ev->pos.y - 1;
                     break;
                  case I6D_VFX1:
                     rel_y = ev->pos.y - (INVENTORY_PANEL_Y >> 1);
//                     rel_y = ev->pos.y - (200 - inv_fullscrn_canvas.bm.h);
                     break;
                  default:
                     break;
               }
            }
            else
#endif
               rel_y = ev->pos.y - INVENTORY_PANEL_Y;
            gr_push_canvas(&inv_fullscrn_canvas);
            smx = SEARCH_MARGIN;
            smy = SEARCH_MARGIN;
#ifdef SVGA_SUPPORT  
            ss_set_hack_mode(2, &temp);
            ss_point_convert(&smx,&smy,FALSE);
#endif
            for (x = relx-smx; !found && x <= relx + smx; x++)
               for (y = rel_y - smy; !found && y <= rel_y + smy; y++)
               {
                  short usex,usey;
                  usex = x;
                  usey = y;
#ifdef SVGA_SUPPORT
                  ss_point_convert(&usex,&usey,FALSE);
#endif
                  if (gr_get_pixel(usex,usey) != 0) // found non-transparent pixel
                     found = TRUE;
               }
#ifdef SVGA_SUPPORT
            ss_set_hack_mode(0,&temp);
#endif
            gr_pop_canvas();
            if (!found)
            {
               return FALSE;
            }
         }
       }
   for (i = 0; gen_inv_page(inventory_page,&i,&dp); i++)
   {
      if (relx < dp->left || relx > dp->right)
         continue;
      row = get_item_at_pixrow(dp,ev->pos.y);
      if (row >= 0)
      {
         break;
      }
   }
   if (input_cursor_mode == INPUT_OBJECT_CURSOR && (mev->action & (MOUSE_LDOWN|MOUSE_RUP|UI_MOUSE_LDOUBLE)))
   {
      add_object_on_cursor(dp,row);
      return TRUE;
   }
   if ((mev->buttons & (1 << MOUSE_RBUTTON)) || (ev->subtype & (MOUSE_RUP|MOUSE_RDOWN)))
      if (inventory_handle_rightbutton(ev,r,dp,row))
         retval = TRUE;
   // Handle left button
   if (ev->subtype & MOUSE_LDOWN)
   {
      if (inventory_handle_leftbutton(ev,dp,row))
         retval = TRUE;
   }
   // Handle left doubleclick
   if (ev->subtype & UI_MOUSE_LDOUBLE)
      if (dp != NULL && (row >= 0) && (dp->use != NULL))
      {
         retval = dp->use(dp,row);
      }
   return retval;
}

      

int last_invent_cnum = -1; // last cursor num set for region
bool pagebutton_mouse_handler(uiMouseEvent* ev, LGRegion* r, void*)
{
   LGPoint pos = ev->pos;
   int cnum;

   if (full_game_3d 
      && (ev->buttons & (1 << MOUSE_LBUTTON)) != 0
      && (ev->action & MOUSE_LDOWN) == 0
      && uiLastMouseRegion[MOUSE_LBUTTON] != NULL 
      && uiLastMouseRegion[MOUSE_LBUTTON] != r)
   {
      uiSetRegionDefaultCursor(r,NULL);
      return FALSE;
   }

   pos.x -= INVENTORY_PANEL_X;
   pos.y -= INVENTORY_PANEL_Y;

   cnum = (pos.x - FIRST_BTTN_X) / BUTTON_X_STEP;
   if (full_game_3d && global_fullmap->cyber && cnum != INV_SOFTWARE_PAGE)
   {
      last_invent_cnum = cnum;
      uiSetRegionDefaultCursor(r,NULL);
      return FALSE;
   }


   if ((cnum != last_invent_cnum) && (cnum < NUM_PAGE_BTTNS))
   {
      LGCursor* c = &invent_bttn_cursor;
      LGPoint offset = {0,-1};

      if ((page_button_state[cnum] == BttnDummy) || !popup_cursors)
         c = NULL;
      last_invent_cnum = cnum;
#ifdef SVGA_SUPPORT
      DisposePtr((Ptr)invent_bttn_bitmap.bits);
      make_popup_cursor(c,&invent_bttn_bitmap,cursor_strings[cnum],POPUP_DOWN,TRUE,offset);
#else
      make_popup_cursor(c,&invent_bttn_bitmap,cursor_strings[cnum],POPUP_DOWN,FALSE,offset);
#endif
      uiSetRegionDefaultCursor(r,c);
   }

   if (input_cursor_mode == INPUT_OBJECT_CURSOR && (ev->action & (MOUSE_LDOWN|MOUSE_RDOWN|UI_MOUSE_LDOUBLE)))
   {
      AddResult pop = (AddResult)add_to_some_page(object_on_cursor,FALSE);
      if (IS_POP_RESULT(pop))
         pop_cursor_object();
      return TRUE;
   }

   if (page_button_state[cnum] == BttnDummy)
      return FALSE;



   if (ev->action & MOUSE_LDOWN)
   {
      int i = cnum;
      short x = FIRST_BTTN_X + i*BUTTON_X_STEP;
         if (pos.x >= x && pos.x < x + INVENT_BTTN_WD
            && page_button_state[i] != BttnDummy)
         {
            if (full_game_3d)
               if (i == inventory_page && full_visible & FULL_INVENT_MASK)
               {
                  full_visible &= ~FULL_INVENT_MASK;
               }
               else
               {
                  gr_push_canvas(pinv_canvas);
                  gr_clear(0);
                  gr_pop_canvas();
#ifdef STEREO_SUPPORT
                  if (convert_use_mode == 5)
                     full_visible = FULL_INVENT_MASK;
                  else
#endif
                     full_visible |= FULL_INVENT_MASK;
                  inv_last_page = -1;
                  full_raise_region(inventory_region_full);
                  chg_set_sta(FULLSCREEN_UPDATE);
               }
            play_digi_fx(SFX_INVENT_BUTTON,1);
            inventory_page = i;
            INVENT_CHANGED;
         }
    }
   return TRUE;
}

#define MAX_HOTKEY_PAGES  6
#define EMPTY_PAGE(i) (page_button_state[i] == BttnDummy)


bool invent_hotkey_func(ushort, ulong, int data)
{
   if (inventory_page < 0)
      inventory_page = MAX_HOTKEY_PAGES;
   if (inventory_page >= MAX_HOTKEY_PAGES)
      inventory_page = -1;
   if (data == 0)
   {
      inventory_page --;
      if (inventory_page < 0)
         inventory_page = MAX_HOTKEY_PAGES -1;
      while (EMPTY_PAGE(inventory_page))
         inventory_page--;
   }
   else
   {
      inventory_page++;
      if (inventory_page >= MAX_HOTKEY_PAGES)
         inventory_page = 0;
      while (EMPTY_PAGE(inventory_page))
         inventory_page++;
   }
   play_digi_fx(SFX_INVENT_BUTTON,1);
   if (!(full_visible & FULL_INVENT_MASK))
   {
      gr_push_canvas(pinv_canvas);
      gr_clear(0);
      gr_pop_canvas();
#ifdef SVGA_SUPPORT
      if (convert_use_mode == 5)
         full_visible = FULL_INVENT_MASK;
      else
#endif
         full_visible |= FULL_INVENT_MASK;
   }
   INVENT_CHANGED;
   return TRUE;
}

bool cycle_weapons_func(ushort, ulong, int data)
{
   if (global_fullmap->cyber)
   {
      int ac = player_struct.actives[ACTIVE_COMBAT_SOFT];
      int bound1 = (data>0) ? NUM_COMBAT_SOFTS : -1;
      int bound2 = (data>0) ? 0 : NUM_COMBAT_SOFTS-1;
      int i;
      for(i = ac+data; i != bound1; i+=data)
         if (player_struct.softs.combat[i] != 0)
            goto got_soft;
      for(i = bound2; i != ac; i+=data)
         if (player_struct.softs.combat[i] != 0)
            goto got_soft;
got_soft:
      player_struct.actives[ACTIVE_COMBAT_SOFT] = i;
      INVENT_CHANGED;
   }
   else
   {
      int aw = player_struct.actives[ACTIVE_WEAPON];

      aw+=data;
      if (aw >= NUM_WEAPON_SLOTS || player_struct.weapons[aw].type == EMPTY_WEAPON_SLOT)
         aw = 0;
      else if (aw < 0)
      {
         for(aw = NUM_WEAPON_SLOTS-1; player_struct.weapons[aw].type == EMPTY_WEAPON_SLOT && aw > 0; aw--);
      }
      inventory_select_weapon(NULL,aw);
   }
   return TRUE;
}



#define PAGEUP_KEY KEY_PAD_PGUP|KB_FLAG_DOWN
#define PAGEDN_KEY KEY_PAD_PGDN|KB_FLAG_DOWN


void init_invent_hotkeys(void)
{
/* ��� later
//   hotkey_add(PAGEUP_KEY,DEMO_CONTEXT,(hotkey_callback)invent_hotkey_func,(void*)0);
   hotkey_add(PAGEUP_KEY|KB_FLAG_2ND,DEMO_CONTEXT,(hotkey_callback)invent_hotkey_func,(void*)0);
   hotkey_add(KB_FLAG_DOWN|KB_FLAG_ALT|'[',DEMO_CONTEXT,(hotkey_callback)invent_hotkey_func,(void*)0);
//   hotkey_add(PAGEDN_KEY,DEMO_CONTEXT,(hotkey_callback)invent_hotkey_func,(void*)1);
   hotkey_add(PAGEDN_KEY|KB_FLAG_2ND,DEMO_CONTEXT,(hotkey_callback)invent_hotkey_func,(void*)1);
   hotkey_add(KB_FLAG_DOWN|KB_FLAG_ALT|']',DEMO_CONTEXT,(hotkey_callback)invent_hotkey_func,(void*)1);
*/
   hotkey_add(KEY_TAB|KB_FLAG_DOWN,DEMO_CONTEXT,(hotkey_callback)cycle_weapons_func,(void*)1);
   hotkey_add(KEY_TAB|KB_FLAG_DOWN|KB_FLAG_SHIFT,DEMO_CONTEXT,(hotkey_callback)cycle_weapons_func,(void*)-1);
}

void invent_language_change(void)
{
   load_string_array(REF_STR_InvCursor,cursor_strings,cursor_string_buf,sizeof(cursor_string_buf),NUM_PAGE_BUTTONS);
}

#define MAX_INV_FULL_WD(x) (fix_int(fix_mul_div(fix_make((x),0),fix_make(1024,0),fix_make(320,0))))
#define MAX_INV_FULL_HT(y) (fix_int(fix_mul_div(fix_make((y),0),fix_make(768,0),fix_make(200,0))))

LGRegion* create_invent_region(LGRegion* root, LGRegion **pbuttons, LGRegion **pinvent)
{
   static bool done_init = FALSE;
   extern void add_email_handler(LGRegion* r);
   int id;
   LGRect invrect;
   LGRegion* invreg = (LGRegion *)NewPtr(sizeof(LGRegion));
   LGRegion* pagereg = (LGRegion *)NewPtr(sizeof(LGRegion));
   FrameDesc* f;
#ifdef OLD_BUTTON_CURSORS
   LGPoint pt;
   int i;
#endif
#ifdef CURSOR_BACKUPS
   extern uchar *backup[NUM_BACKUP_BITS];
   extern grs_bitmap backup_invent_bttn_cursors[NUM_PAGE_BTTNS];
#endif

   // Create the panel region
   invrect.ul.x = INVENTORY_PANEL_X;   invrect.ul.y = INVENTORY_PANEL_Y;
   invrect.lr.x = invrect.ul.x + INVENTORY_PANEL_WIDTH;
   invrect.lr.y = invrect.ul.y + INVENTORY_PANEL_HEIGHT;
   region_create(root, invreg, &invrect, 0, 0, REG_USER_CONTROLLED|AUTODESTROY_FLAG, NULL, NULL, NULL, NULL);
   uiInstallRegionHandler(invreg,UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE,inventory_mouse_handler,NULL,&id);
   uiSetRegionDefaultCursor(invreg,NULL);
   add_email_handler(invreg);
   if (pinvent != NULL)
      *pinvent = invreg;

   // Create the pagebutton region
   invrect.ul.y = invrect.lr.y;
   invrect.lr.y = RectHeight(root->r);
   region_create(root, pagereg, &invrect, 0, 0, REG_USER_CONTROLLED|AUTODESTROY_FLAG, NULL, NULL, NULL, NULL);
   uiInstallRegionHandler(pagereg,(UI_EVENT_MOUSE|UI_EVENT_MOUSE_MOVE),(uiHandlerProc)pagebutton_mouse_handler,NULL,&id);
   uiSetRegionDefaultCursor(pagereg,&globcursor);

   if (pbuttons != NULL)
      *pbuttons = pagereg;

   if (!done_init)
   {
      done_init = TRUE;
      // Assign different cursors to different buttons in pagebutton region
      {
         grs_bitmap* bm = &invent_bttn_bitmap;
         LGCursor* c = &invent_bttn_cursor;
         LGPoint offset = {0,-1};

         invent_language_change();
         make_popup_cursor(c,bm,cursor_strings[0],POPUP_DOWN,TRUE,offset);
      }
      
      // Pull in the background bitmap
      ResLockHi(RES_gamescrGfx);
      f = (FrameDesc*)RefLock(REF_IMG_bmBlankInventoryPanel);
      inv_backgnd = f->bm;

      // This background is going to get used by the 360 ware 
      // in fullscreen mode, so we need extra bits 
//KLC      inv_backgnd.bits = (uchar *)NewPtr(MAX_INV_FULL_WD(INV_FULL_WD) * MAX_INV_FULL_HT(grd_cap->h - GAME_MESSAGE_Y));
      inv_backgnd.bits = (uchar *)NewPtr(290 * 136);					// KLC - I just happen to know what these are.
      LG_memcpy(inv_backgnd.bits,(f+1),f->bm.w * f->bm.h);
      ResUnlock(RES_gamescrGfx);
      ResUnlock(RES_gamescrGfx);

      // init the canvas
      gr_init_sub_canvas(grd_scr_canv,&inv_norm_canvas,INVENTORY_PANEL_X,INVENTORY_PANEL_Y,INVENTORY_PANEL_WIDTH,INVENTORY_PANEL_HEIGHT);
      gr_init_canvas(&inv_fullscrn_canvas,inv_backgnd.bits, BMT_FLAT8, INVENTORY_PANEL_WIDTH,INVENTORY_PANEL_HEIGHT);
      gr_init_canvas(&inv_view360_canvas,inv_backgnd.bits, BMT_FLAT8, INV_FULL_WD, INV_FULL_HT);
      gr_init_sub_canvas(grd_scr_canv,&inv_gamepage_canvas,INVENTORY_PANEL_X,BUTTON_PANEL_Y,INVENTORY_PANEL_WIDTH,grd_cap->h - BUTTON_PANEL_Y);

      uchar *p = (uchar *)NewPtr(292 * 10);											// This canvas holds an off-screen image of the
      gr_init_canvas(&inv_fullpage_canvas, p, BMT_FLAT8, 292, 10);			// inventory buttons.
      gr_push_canvas(&inv_fullpage_canvas);
      gr_clear(0);
      gr_pop_canvas();
   }
   return invreg;
}


errtype inventory_update_screen_mode()
{
   if (convert_use_mode)
   {
      gr_init_sub_canvas(grd_scr_canv, &inv_norm_canvas,174, 347, 290, 120);
      if (full_game_3d)
      {
//KLC - chg for new art         gr_init_canvas(&inv_fullscrn_canvas,inv_backgnd.bits, BMT_FLAT8, SCONV_X(INVENTORY_PANEL_WIDTH),SCONV_Y(INVENTORY_PANEL_HEIGHT));
         gr_init_canvas(&inv_fullscrn_canvas, inv_backgnd.bits, BMT_FLAT8, 290, 120);
//KLC - chg for new art         gr_init_canvas(&inv_view360_canvas,inv_backgnd.bits, BMT_FLAT8, SCONV_X(INV_FULL_WD), SCONV_Y(INV_FULL_HT));
         gr_init_canvas(&inv_view360_canvas, inv_backgnd.bits, BMT_FLAT8, 290, SCONV_Y(INV_FULL_HT));
      }
      else
      {
         gr_init_sub_canvas(grd_scr_canv,&inv_gamepage_canvas, 172, 470, 292, 10);
         gr_init_canvas(&inv_view360_canvas, inv_backgnd.bits, BMT_FLAT8, 290, SCONV_Y(INV_FULL_HT));
      }
   }
/*KLC - not used in Mac version
   else
   {
      gr_init_sub_canvas(grd_scr_canv,&inv_norm_canvas,INVENTORY_PANEL_X,INVENTORY_PANEL_Y,
         INVENTORY_PANEL_WIDTH,INVENTORY_PANEL_HEIGHT);
      if (full_game_3d)
      {
         gr_init_canvas(&inv_fullscrn_canvas,inv_backgnd.bits, BMT_FLAT8, INVENTORY_PANEL_WIDTH,INVENTORY_PANEL_HEIGHT);
         gr_init_canvas(&inv_view360_canvas,inv_backgnd.bits, BMT_FLAT8, INV_FULL_WD, INV_FULL_HT);
      }
      else
      {
         gr_init_sub_canvas(grd_scr_canv,&inv_gamepage_canvas,INVENTORY_PANEL_X,BUTTON_PANEL_Y,
            INVENTORY_PANEL_WIDTH,grd_cap->h - BUTTON_PANEL_Y);
      }
   }
*/
   return(OK);
}


extern bool inv_is_360_view(void);


void inv_change_fullscreen(bool on)
{
   if (on)
   {
      pinv_canvas = &inv_fullscrn_canvas;
      ppage_canvas = &inv_fullpage_canvas;
      gr_push_canvas(pinv_canvas);
      gr_clear(0);
      gr_pop_canvas();
      dirty_inv_canvas = TRUE;
   }
   else
   {
      int i;
      pinv_canvas = &inv_norm_canvas;
      ppage_canvas = &inv_gamepage_canvas;
      for (i = 0; i < NUM_PAGE_BUTTONS; i++)
         old_button_state[i] = BttnOff;
      if (inventory_page == INV_EMAILTEXT_PAGE)
         inventory_page = INV_MAIN_PAGE;
   }
   inv_last_page = INV_BLANK_PAGE;
   INVENT_CHANGED;
}


void inv_update_fullscreen(bool full)
{
   grs_bitmap* bm;
   short a,b,c,d;
   STORE_CLIP(a,b,c,d);
   if (full)
   {
#ifdef SVGA_SUPPORT

      if (inv_is_360_view())
      {
         ss_noscale_bitmap(&inv_view360_canvas.bm,GAME_MESSAGE_X,GAME_MESSAGE_Y);
      }
      else
      {
         inv_fullscrn_canvas.bm.flags |= BMF_TRANS;
//         ss_bitmap(&(inv_fullscrn_canvas.bm),INVENTORY_PANEL_X,INVENTORY_PANEL_Y);
/* KLC -- Shouldn't this be ifdef'd out?
         if (convert_use_mode == 5)
         {
            switch (i6d_device)
            {
               case I6D_CTM:
                  ss_noscale_bitmap(&(inv_fullscrn_canvas.bm),1,1);
                  break;
               case I6D_VFX1:
//                  ss_noscale_bitmap(&(inv_fullscrn_canvas.bm),(320-inv_fullscrn_canvas.bm.w)/2,200 - inv_fullscrn_canvas.bm.h);
                  ss_noscale_bitmap(&(inv_fullscrn_canvas.bm),(320-inv_fullscrn_canvas.bm.w)/2,INVENTORY_PANEL_Y >> 1);
                  break;
            }
         }   
         else
 */
            ss_noscale_bitmap(&(inv_fullscrn_canvas.bm),INVENTORY_PANEL_X,INVENTORY_PANEL_Y);
         inv_fullscrn_canvas.bm.flags &= ~BMF_TRANS;
      }
#else
      if (inv_is_360_view())
      {
         ss_noscale_bitmap(&inv_view360_canvas.bm,GAME_MESSAGE_X,GAME_MESSAGE_Y);
      }
      else
      {
         inv_fullscrn_canvas.bm.flags |= BMF_TRANS;
//         ss_bitmap(&(inv_fullscrn_canvas.bm),INVENTORY_PANEL_X,INVENTORY_PANEL_Y);
         ss_noscale_bitmap(&(inv_fullscrn_canvas.bm),INVENTORY_PANEL_X,INVENTORY_PANEL_Y);
         inv_fullscrn_canvas.bm.flags &= ~BMF_TRANS;
      }
#endif
   }
   region_set_invisible(inventory_region_full,!full);
   bm = &inv_fullpage_canvas.bm;
   bm->flags |= BMF_TRANS;
   if (global_fullmap->cyber)
   {
      ss_safe_set_cliprect(INVENTORY_PANEL_X+bm->w/2,BUTTON_PANEL_Y,
         INVENTORY_PANEL_X+bm->w,BUTTON_PANEL_Y+bm->h);
   }
   gr_bitmap(bm, 172, 470);		//KLC - was ss_bitmap (with scaling)
   bm->flags &= BMF_TRANS;
   RESTORE_CLIP(a,b,c,d);
}


// ----------------------
// THE DISPLAY LIST ARRAY
// ----------------------

#define FIELD_OFFSET(fld)  (int)&(((Player*) 0)->fld)


inv_display inv_display_list[] =
{
   // Page 0, weapons, grenades, drugs
   // weapons are there own thang, they have slots and stuff...
   { 0, 0, WEAPON_X, AMMO_X, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, WEAPONS_PER_PAGE, NUM_WEAPON_SLOTS, REF_STR_WeaponTitle,
      ACTIVE_WEAPON, 0, MFD_INV_WEAPON, NULL, NULL, draw_weapons_list, inventory_select_weapon, weapon_use_func, WEAP_CLASSES,
      weapons_add_func, weapon_drop_func, WEAP_TRIP, NULL, 0, NULL},
   // grenades
   { 0, 0, GRENADE_LEFT_X, GRENADE_RIGHT_X, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, GRENADES_PER_PAGE, NUM_GRENADES,
      REF_STR_GrenadeTitle, ACTIVE_GRENADE, FIELD_OFFSET(grenades), MFD_INV_GRENADE, grenade_name_func,
      generic_quant_func, generic_draw_list,NULL, grenade_use_func, GREN_CLASSES, grenade_add_func, generic_drop_func, GREN_TRIP,
      NULL, 0, generic_lines},
   // drug
   { 0, 0, DRUG_LEFT_X, DRUG_RIGHT_X, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, DRUGS_PER_PAGE, NUM_DRUGS, REF_STR_DrugTitle,
     ACTIVE_DRUG, FIELD_OFFSET(drugs), MFD_INV_DRUG, generic_name_func, generic_quant_func, generic_draw_list, NULL, drug_use_func,
     DRUG_CLASSES, generic_add_func, generic_drop_func, DRUG_TRIP, triple2drug, 0, generic_lines+NUM_GRENADES},


   // Page 1, Hardwares. 
   { 1, 0, LEFT_MARGIN, CENTER_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, ITEMS_PER_PAGE, NUM_HARDWAREZ,
     REF_STR_HardwareTitle, ACTIVE_HARDWARE, FIELD_OFFSET(hardwarez), MFD_INV_HARDWARE, ware_name_func, soft_quant_func,
     generic_draw_list,NULL, ware_use_func, HARD_CLASSES, ware_add_func, ware_drop_func, HARD_TRIP, NULL, 0, generic_lines},
   { 1, 1, CENTER_X + LEFT_MARGIN, RIGHT_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, ITEMS_PER_PAGE, NUM_HARDWAREZ,
     REF_STR_Null, ACTIVE_HARDWARE, FIELD_OFFSET(hardwarez), MFD_INV_HARDWARE, ware_name_func, soft_quant_func, generic_draw_list,
     NULL, ware_use_func, HARD_CLASSES, ware_add_func, ware_drop_func, HARD_TRIP, NULL, 0, generic_lines},

      // Page 2. General
   {  2, 0, LEFT_MARGIN, CENTER_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, ITEMS_PER_PAGE, NUM_GENERAL_SLOTS,
      REF_STR_GeneralTitle, ACTIVE_GENERAL, NULL, MFD_INV_GENINV, NULL, NULL, draw_general_list, inv_select_general, general_use_func,
      GENERAL_CLASSES, general_add_func, general_drop_func, 0, NULL, 0, (quantity_state*)general_lines},
   {  2, 1, CENTER_X - RIGHT_MARGIN, RIGHT_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, ITEMS_PER_PAGE, NUM_GENERAL_SLOTS,
      REF_STR_Null, ACTIVE_GENERAL, NULL, MFD_INV_GENINV, NULL, NULL, draw_general_list, inv_select_general, general_use_func,
      GENERAL_CLASSES, general_add_func, general_drop_func, 0, NULL, 0, (quantity_state*)general_lines},



   // Page 2, Softwares.  
   { 5, 0, LEFT_MARGIN, CENTER_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, NUM_COMBAT_SOFTS-1, NUM_COMBAT_SOFTS-1,
     REF_STR_SoftTitle, ACTIVE_COMBAT_SOFT, FIELD_OFFSET(softs.combat), MFD_INV_SOFT_COMBAT, ware_name_func, soft_quant_func,
     generic_draw_list, NULL, ware_use_func, SOFT_CLASSES, ware_add_func, ware_drop_func, COMSOFT_TRIP, NULL, 0, generic_lines},
   { 5, 0, LEFT_MARGIN, CENTER_X-RIGHT_MARGIN, TOP_MARGIN+6*Y_STEP, TITLE_COLOR, ITEM_COLOR, 0, 1,
     1, REF_STR_Null, ACTIVE_DEFENSE_SOFT, FIELD_OFFSET(softs.defense), MFD_INV_SOFT_DEFENSE,
     ware_name_func, soft_quant_func, generic_draw_list,NULL, ware_use_func,
     SOFT_CLASSES, ware_add_func, ware_drop_func, DEFSOFT_TRIP, NULL, 0, generic_lines+NUM_COMBAT_SOFTS},
   { 5, 0, CENTER_X + LEFT_MARGIN, RIGHT_X - RIGHT_MARGIN,  TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, NUM_ONESHOT_SOFTWARE,
     NUM_ONESHOT_SOFTWARE, REF_STR_Null, ACTIVE_MISC_SOFT, FIELD_OFFSET(softs.misc), MFD_INV_SOFT_MISC, ware_name_func, generic_quant_func,
     generic_draw_list,NULL, ware_use_func, SOFT_CLASSES, ware_add_func, ware_drop_func, MISCSOFT_TRIP, NULL, 0,
     generic_lines+NUM_COMBAT_SOFTS+NUM_DEFENSE_SOFTS},
   { 5, 0, CENTER_X + LEFT_MARGIN, RIGHT_X - RIGHT_MARGIN,  TOP_MARGIN+6*Y_STEP, TITLE_COLOR, ITEM_COLOR, NUM_ONESHOT_SOFTWARE, 1,
     NUM_MISC_SOFTWARE, REF_STR_Null, ACTIVE_MISC_SOFT, FIELD_OFFSET(softs.misc), MFD_INV_SOFT_MISC, ware_name_func, null_quant_func,
     generic_draw_list,NULL, ware_use_func, SOFT_CLASSES, ware_add_func, ware_drop_func, MISCSOFT_TRIP, NULL, 0,
     generic_lines+NUM_COMBAT_SOFTS+NUM_DEFENSE_SOFTS+NUM_ONESHOT_SOFTWARE},
                                                                                 
   // Page 7 main log page
   {  7, 0, LEFT_MARGIN, CENTER_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, ITEMS_PER_PAGE, NUM_LOG_LEVELS,
      REF_STR_LogTitle, NULL_ACTIVE, FIELD_OFFSET(logs), MFD_INV_NULL, log_name_func, generic_quant_func, generic_draw_list,
      log_use_func, log_use_func, SOFT_CLASSES, email_add_func, email_drop_func, EMAIL_TRIP, NULL, 0, generic_lines},
   {  7, 1, CENTER_X + LEFT_MARGIN, RIGHT_X, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, ITEMS_PER_PAGE,
      NUM_LOG_LEVELS, REF_STR_Null, NULL_ACTIVE, FIELD_OFFSET(logs), MFD_INV_NULL, log_name_func, generic_quant_func,
      generic_draw_list, log_use_func, log_use_func, SOFT_CLASSES, email_add_func, email_drop_func, EMAIL_TRIP, NULL, 0,
      generic_lines},
#ifdef NEED_THIRD_LOGLVL_PAGE
   {  7, 2, CENTER_X + LEFT_MARGIN, RIGHT_X, TOP_MARGIN-Y_STEP, TITLE_COLOR, ITEM_COLOR, 0, ITEMS_PER_PAGE,
      NUM_LOG_LEVELS-1, REF_STR_Null, NULL_ACTIVE, FIELD_OFFSET(logs), MFD_INV_NULL, log_name_func, generic_quant_func,
      generic_draw_list, log_use_func, log_use_func, SOFT_CLASSES, email_add_func, email_drop_func, EMAIL_TRIP, NULL, 0,
      generic_lines},
#endif 

   // Page 8, Data
   {  8, 0, LEFT_MARGIN, ONETHIRD_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, FIRST_DATA, ITEMS_PER_PAGE,
      FIRST_DATA+ITEMS_PER_PAGE, REF_STR_DataTitle, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, email_name_func,
      null_quant_func, generic_draw_list, email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func,
      EMAIL_TRIP, NULL, 0, generic_lines},
   {  8, 1, ONETHIRD_X + LEFT_MARGIN, TWOTHIRDS_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, FIRST_DATA+ITEMS_PER_PAGE,
      ITEMS_PER_PAGE+1, FIRST_DATA+2*ITEMS_PER_PAGE+1,
      REF_STR_Null, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, email_name_func, null_quant_func, generic_draw_list,
      email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func, EMAIL_TRIP, NULL, 0, generic_lines},
   {  8, 2, TWOTHIRDS_X + LEFT_MARGIN, RIGHT_X, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, FIRST_DATA+2*ITEMS_PER_PAGE+1, ITEMS_PER_PAGE+1, NUM_EMAIL,
      REF_STR_Null, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, email_name_func, null_quant_func, generic_draw_list,
      email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func, EMAIL_TRIP, NULL, 0, generic_lines},


   // Ammo page, off screen. 
   { 9, 0, AMMO_LEFT_1, AMMO_RIGHT_1, TOP_MARGIN, TITLE_COLOR, ITEM_COLOR, 0, NUM_AMMO_TYPES, NUM_AMMO_TYPES, REF_STR_PistolCartTitle,
     ACTIVE_CART, FIELD_OFFSET(cartridges), MFD_INV_AMMO, ammo_name_func, generic_quant_func, generic_draw_list,NULL, NULL,
     AMMO_CLASSES, generic_add_func, generic_drop_func, AMMO_TRIP, NULL, 0, generic_lines},

   // Pages 50-52 Email
   {  50, 0, LEFT_MARGIN, CENTER_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, EMAIL_COLOR_FUNC, 0, ITEMS_PER_PAGE-1, NUM_EMAIL_PROPER,
      REF_STR_EmailTitle, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, email_name_func, null_quant_func, generic_draw_list,
      email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func, EMAIL_TRIP, NULL, 0, generic_lines},
   {  50, 0, LEFT_MARGIN, CENTER_X - RIGHT_MARGIN, TOP_MARGIN+(ITEMS_PER_PAGE-1)*Y_STEP, TITLE_COLOR, EMAIL_COLOR_FUNC, 0, ITEMS_PER_PAGE-1, NUM_EMAIL_PROPER,
      REF_STR_EmailTitle, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, NULL, NULL, email_more_draw,
      email_more_use, email_more_use, 0, NULL, NULL, 0, NULL, 0, generic_lines},

   {  50, 1, CENTER_X + LEFT_MARGIN, RIGHT_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, EMAIL_COLOR_FUNC, 0,ITEMS_PER_PAGE-1,
      NUM_EMAIL_PROPER, REF_STR_Null, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, email_name_func,
      null_quant_func, generic_draw_list, email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func,
      EMAIL_TRIP, NULL, 0, generic_lines},
   {  50, 1, CENTER_X + LEFT_MARGIN, RIGHT_X - RIGHT_MARGIN, TOP_MARGIN+(ITEMS_PER_PAGE-1)*Y_STEP, TITLE_COLOR, EMAIL_COLOR_FUNC, 0,
      ITEMS_PER_PAGE-1,
      NUM_EMAIL_PROPER, REF_STR_Null, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, NULL,NULL,
      email_more_draw, email_more_use, email_more_use, 0, NULL,NULL, EMAIL_TRIP, NULL, 0, generic_lines},

   {  51, 2, LEFT_MARGIN, CENTER_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, EMAIL_COLOR_FUNC, 0, ITEMS_PER_PAGE-1, NUM_EMAIL_PROPER,
      REF_STR_EmailTitle, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, email_name_func, null_quant_func, generic_draw_list,
      email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func, EMAIL_TRIP, NULL, 0, generic_lines},
   {  51, 2, LEFT_MARGIN, CENTER_X - RIGHT_MARGIN, TOP_MARGIN+(ITEMS_PER_PAGE-1)*Y_STEP, TITLE_COLOR, EMAIL_COLOR_FUNC, 0,
      ITEMS_PER_PAGE-1,
      NUM_EMAIL_PROPER, REF_STR_Null, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, NULL, NULL, email_more_draw,
      email_more_use, email_more_use, 0, NULL, NULL, 0, NULL, 0, generic_lines},

   {  51, 3, CENTER_X + LEFT_MARGIN, RIGHT_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, EMAIL_COLOR_FUNC, 0,ITEMS_PER_PAGE-1,
      NUM_EMAIL_PROPER, REF_STR_Null, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, email_name_func,
      null_quant_func, generic_draw_list, email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func,
      EMAIL_TRIP, NULL, 0, generic_lines},
   {  51, 3, CENTER_X + LEFT_MARGIN, RIGHT_X - RIGHT_MARGIN, TOP_MARGIN+(ITEMS_PER_PAGE-1)*Y_STEP, TITLE_COLOR, EMAIL_COLOR_FUNC, 0,
      ITEMS_PER_PAGE-1,
      NUM_EMAIL_PROPER, REF_STR_EmailTitle, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, NULL,NULL,
      email_more_draw, email_more_use, email_more_use, 0, NULL,NULL, EMAIL_TRIP, NULL, 0, generic_lines},

   {  52, 4, LEFT_MARGIN, CENTER_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, EMAIL_COLOR_FUNC, 0, ITEMS_PER_PAGE-1, NUM_EMAIL_PROPER,
      REF_STR_EmailTitle, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, email_name_func, null_quant_func, generic_draw_list,
      email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func, EMAIL_TRIP, NULL, 0, generic_lines},
   {  52, 4, LEFT_MARGIN, CENTER_X - RIGHT_MARGIN, TOP_MARGIN+(ITEMS_PER_PAGE-1)*Y_STEP, TITLE_COLOR, EMAIL_COLOR_FUNC, 0,
      ITEMS_PER_PAGE-1, NUM_EMAIL_PROPER,
      REF_STR_EmailTitle, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, NULL, NULL, email_more_draw,
      email_more_use, email_more_use, 0, NULL, NULL, 0, NULL, 0, generic_lines},

   {  52, 5, CENTER_X + LEFT_MARGIN, RIGHT_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, EMAIL_COLOR_FUNC, 0,ITEMS_PER_PAGE-1,
      NUM_EMAIL_PROPER, REF_STR_Null, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, email_name_func,
      null_quant_func, generic_draw_list, email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func,
      EMAIL_TRIP, NULL, 0, generic_lines},
   {  52, 5, CENTER_X + LEFT_MARGIN, RIGHT_X - RIGHT_MARGIN, TOP_MARGIN+(ITEMS_PER_PAGE-1)*Y_STEP, TITLE_COLOR, EMAIL_COLOR_FUNC, 0,
      ITEMS_PER_PAGE-1,
      NUM_EMAIL_PROPER, REF_STR_EmailTitle, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, NULL,NULL,
      email_more_draw, email_more_use, email_more_use, 0, NULL,NULL, EMAIL_TRIP, NULL, 0, generic_lines},

   // Page 20 logs
#define LOG_PAGE(i)  \
   {  FIRST_LOG_PAGE+i, 0, LEFT_MARGIN, CENTER_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, EMAIL_COLOR_FUNC, NUM_EMAIL_PROPER+(i)*LOGS_PER_LEVEL,      \
      ITEMS_PER_PAGE, NUM_EMAIL_PROPER+(i)*LOGS_PER_LEVEL+ITEMS_PER_PAGE, REF_STR_LogName0+i, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL,  \
      email_name_func, null_quant_func,  generic_draw_list, email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func,   \
      EMAIL_TRIP, NULL, 0, generic_lines},                                                                                                        \
   {  FIRST_LOG_PAGE+i, 2, CENTER_X + LEFT_MARGIN, RIGHT_X - RIGHT_MARGIN, TOP_MARGIN - Y_STEP, TITLE_COLOR, EMAIL_COLOR_FUNC,                          \
      NUM_EMAIL_PROPER+(i)*LOGS_PER_LEVEL+2*ITEMS_PER_PAGE+1, 1, NUM_EMAIL_PROPER+((i)+1)*LOGS_PER_LEVEL,                             \
      REF_STR_Null, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, email_name_func, null_quant_func, generic_draw_list,                       \
      email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func, EMAIL_TRIP, NULL, 0, generic_lines+ 2*ITEMS_PER_PAGE},                          \
   {  FIRST_LOG_PAGE+i, 1, CENTER_X + LEFT_MARGIN, RIGHT_X - RIGHT_MARGIN, TOP_MARGIN, TITLE_COLOR, EMAIL_COLOR_FUNC,                          \
      NUM_EMAIL_PROPER+(i)*LOGS_PER_LEVEL+ITEMS_PER_PAGE, ITEMS_PER_PAGE, NUM_EMAIL_PROPER+(i)*LOGS_PER_LEVEL+2*ITEMS_PER_PAGE+1,                            \
      REF_STR_Null, NULL_ACTIVE, FIELD_OFFSET(email), MFD_INV_NULL, email_name_func, null_quant_func, generic_draw_list,                       \
      email_use_func, email_use_func, SOFT_CLASSES, email_add_func, email_drop_func, EMAIL_TRIP, NULL, 0, generic_lines}                          \
   
   // Hey these pages MUST BE LAST.
   LOG_PAGE(0),
#ifdef EXPLICIT_LOG_PAGES
   LOG_PAGE(1),
   LOG_PAGE(2),
   LOG_PAGE(3),
   LOG_PAGE(4),
   LOG_PAGE(5),
   LOG_PAGE(6),
   LOG_PAGE(7),
   LOG_PAGE(8),
   LOG_PAGE(9),
   LOG_PAGE(10),
   LOG_PAGE(11),
   LOG_PAGE(12),
   LOG_PAGE(13),
   LOG_PAGE(14),
#endif // EXPLICIT_LOG_PAGES


 };

#define NUM_INV_DISPLAYS (sizeof(inv_display_list)/sizeof(inv_display))

#define DUMMY_LOG_INDEX (NUM_INV_DISPLAYS- 3)

void super_drop_func(int dispnum, int row)
{
   inv_display* dp=&(inv_display_list[dispnum]);
   dp->drop(dp,row);
}

void super_use_func(int dispnum, int row)
{
   inv_display* dp=&(inv_display_list[dispnum]);
   dp->use(dp,row);
}

void gen_log_displays(int pgnum)
{
   inv_display* dp = &inv_display_list[DUMMY_LOG_INDEX];
   pgnum -= FIRST_LOG_PAGE;
   if (pgnum >= 0  && pgnum < NUM_LOG_LEVELS)
   {
      dp->pgnum = FIRST_LOG_PAGE + pgnum;
      dp->first = NUM_EMAIL_PROPER + pgnum*LOGS_PER_LEVEL;
      dp->listlen = NUM_EMAIL_PROPER + ITEMS_PER_PAGE + pgnum*LOGS_PER_LEVEL;
      dp->titlenum = MKREF(RES_lognames,pgnum);
      dp++;
      dp->pgnum = FIRST_LOG_PAGE + pgnum;
      dp->first = NUM_EMAIL_PROPER + pgnum*LOGS_PER_LEVEL + 2*ITEMS_PER_PAGE+1;
      dp->listlen = NUM_EMAIL_PROPER + (pgnum+1)*LOGS_PER_LEVEL;
      dp++;
      dp->pgnum = FIRST_LOG_PAGE + pgnum;
      dp->first = NUM_EMAIL_PROPER + pgnum*LOGS_PER_LEVEL + ITEMS_PER_PAGE;
      dp->listlen = NUM_EMAIL_PROPER + pgnum*LOGS_PER_LEVEL + 2*ITEMS_PER_PAGE + 1;
   }
 }



bool gen_inv_page(int pgnum, int *i, inv_display** dp)
{
   if (*i == 0)
      gen_log_displays(pgnum);
   for (; *i < NUM_INV_DISPLAYS; (*i)++)
   {
      inv_display* idp = &inv_display_list[*i];
      if (idp->pgnum == pgnum)
      {
         *dp = idp;
         return TRUE;
      }
   }
   return FALSE;
}

#define LOG_PAGE_SHF 8


bool gen_inv_displays(int *i, inv_display** dp)
{
   if (*i == 0)
      gen_log_displays(inventory_page);
   for (; *i < NUM_INV_DISPLAYS; (*i)++)
   {
      inv_display* idp = &inv_display_list[*i];
      *dp = idp;
      return TRUE;
   }
   return FALSE;
}


void absorb_object_on_cursor(short, ulong, void*)
{
   if(object_on_cursor==NULL) return;

   if(inventory_add_object(object_on_cursor,TRUE))
      pop_cursor_object();
   return;
}

