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
 * $Source: r:/prj/cit/src/RCS/mfdpanel.c $
 * $Revision: 1.75 $
 * $Author: dc $
 * $Date: 1994/11/14 03:30:50 $
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "input.h"
#include "mfdpanel.h"
#include "mfdint.h"
#include "mfdext.h"
#include "mfddims.h"
#include "mfdgadg.h"
#include "mfdart.h"
#include "tools.h"
#include "cybstrng.h"
#include "gamescr.h"
#include "gamestrn.h"
#include "objbit.h"
#include "citres.h"

#include "objsim.h"
#include "objgame.h"

#include "colors.h"
#include "fullscrn.h"

#include "otrip.h"
#include "gr2ss.h"


#define WORKING_INC_MFDS
#define DRAW_GRID_PLUSSES

// ----------
//  PROTOTYPES
// ----------
extern errtype accesspanel_trigger(ObjID id);
errtype simple_load_res_bitmap_cursor(LGCursor* c, grs_bitmap* bmp, Ref rid);
errtype load_res_bitmap(grs_bitmap* bmp,Ref rid,bool alloc);

int wirepos_score(wirePosPuzzle *wppz);
void wirepos_setup_buttons(wirePosPuzzle *wppz);
bool wirepos_3int_init(wirePosPuzzle *wppz, int a1, int a2, int a3);
void wirepos_3int_update(wirePosPuzzle *wppz);
int wirepos_iswire(wirePosPuzzle *wppz, int wim_code);
int wirepos_rescore_n_check(wirePosPuzzle *wppz);
bool wirepos_moveto(wirePosPuzzle *wppz, int wim_code);

int mfd_slot_primary(int slot);
bool mfd_accesspanel_button_handler(MFD* mfd, LGPoint bttn, uiEvent* ev, void* data);
bool mfd_accesspanel_handler(MFD* mfd, uiEvent* ev);
errtype mfd_accesspanel_init(MFD_Func* f);
int access_help_string(wirePosPuzzle *wppz);
void mfd_accesspanel_expose(MFD* mfd, ubyte control);
void mfd_setup_wirepanel(uchar special, ObjID id);
uchar mfd_solve_wirepanel(void);
errtype mfd_gridpanel_init(MFD_Func* f);
bool mfd_gridpanel_button_handler(MFD* mfd, LGPoint bttn, uiEvent* ev, void* data);
bool mfd_gridpanel_handler(MFD* m, uiEvent* ev);
void mfd_setup_gridpanel(ObjID id);
uchar mfd_solve_gridpanel(void);
void mfd_gridpanel_set_winmove(bool check);
gpz_state gridpanel_move(LGPoint node, gridFlowPuzzle* gfpz);
void mfd_gridpanel_expose(MFD* mfd, ubyte control);

void gpz_set_grid_state(gridFlowPuzzle *gfpz, short row, short col, gpz_state val);
void gpz_uncharge_grid(gridFlowPuzzle *gfpz);
gpz_state gpz_charge_state(gpz_state s);
gpz_state gpz_uncharge_state(gpz_state s);
bool gpz_is_charged(gpz_state s);
void gpz_toggle_state(gridFlowPuzzle *gfpz, short r, short c);
gpz_state bitarray_get(short siz, short off, uint *base);
void bitarray_set(short siz, short off, ushort val, uint *base);
int gpz_search_depth(gridFlowPuzzle* gfpz);
void gpz_add_gate(gridFlowPuzzle *gfpz, ObjID me);
void gpz_perimeter_to_grid(gridFlowPuzzle *gfpz, ushort per, short* r, short* c);
bool gpz_state_charged(gridFlowPuzzle *gfpz, short r, short c);
uchar gpz_doneness(gridFlowPuzzle *gfpz);
bool gpz_propogate_charge_n_check(gridFlowPuzzle *gfpz);
void gpz_setup_buttons(gridFlowPuzzle *gfpz);

bool find_winning_move_from(gridFlowPuzzle *gfpz, gridFlowPuzzle *solved, LGPoint* move, int r0, int c0, int dep, ulong timeout);
bool find_winning_move(gridFlowPuzzle *gfpz, gridFlowPuzzle *solved, int depth, LGPoint* move, bool breadth_first, ulong timeout);
void wacky_int_line(short x1,short y1,short x2,short y2);
char* grid_help_string(gridFlowPuzzle *gfpz, char* buf, int siz);
void id_clut_init(uchar* clut);

// not so secret, is it?
// the fact that we claim to know the size of this is criminal.
extern uchar hideous_secret_game_storage[2052];

#define HSGS hideous_secret_game_storage
#define SHADOW_PANEL_SIG (*((ulong*)HSGS))
#define OUR_SIGNATURE (((ulong)'G'<<24)|((ulong)'r'<<16)|((ulong)'i'<<8)|((ulong)'d'))
#define SIGN_US() (SHADOW_PANEL_SIG=OUR_SIGNATURE)
#define WE_ARE_SIGNED() ((SHADOW_PANEL_SIG)==OUR_SIGNATURE)

#define SHADOW_PANEL_ID  (*((ObjID*)((&SHADOW_PANEL_SIG)+1)))
#define SHADOW_PANEL_LEV (*((short*)((&SHADOW_PANEL_ID)+1)))
#define SHADOW_PANEL_STOR ((uchar*)((&SHADOW_PANEL_LEV)+1))
#define MFD_GRID_CLUT ((uchar*)((SHADOW_PANEL_STOR)+sizeof(player_struct.mfd_access_puzzles)))
#define grid_help_clut MFD_GRID_CLUT
#define MFDPANEL_MEMORY_BAG (MFD_GRID_CLUT+256)
#define MFDPANEL_MEMORY_BAG_SIZE (sizeof(HSGS)-(MFDPANEL_MEMORY_BAG-HSGS))

int gpz_base_colors[] = { BLUE_BASE+10, GREEN_BASE+7, 0x24,
   GREEN_YELLOW_BASE+9, GRAY_BASE+5 };
int gpz_dk_colors[] = { 0x7E, 0x63, 0x27, 0x56, GRAY_BASE+13 };

LGCursor gridCursor;
grs_bitmap gridCursorbm;
#ifdef SVGA_SUPPORT
uchar gridCursorBits[1016]; // This should be enough, maybe... note wacky computation scaling 9x9 to apropos for 1024x768
#else
uchar gridCursorBits[81]; // This should be enough
#endif

//#define score_spew(str,v)    mprintf(str,v)
//#define score_spew2(str,v,o) mprintf(str,v,o)
#define score_spew(str,v)    
#define score_spew2(str,v,o) 

#define WIREPUZ_CODE 0
#define GRIDPUZ_CODE 1

int mfd_slot_primary(int slot)
{
   int mid;

   for(mid=0;mid<NUM_MFDS;mid++) {
      if(player_struct.mfd_current_slots[mid]==slot)
         return mid;
   }
   return -1;
}

#define MFD_MARGIN_WID 3
#define SWITCH_COLOR 2



// draws text and/or bitmap centered in current canvas,
// assumed to be an mfd.
static void draw_help_text( char* str, bool wire, void* puzzle )
{
   short sw,sh,bw=0,bh=0,x,y;
   bool save_w = mfd_string_wrap, bmap=FALSE;
   grs_bitmap foot;
   uchar bcolor;

   gr_set_font((grs_font*)ResLock(MFD_FONT));
   wrap_text(str,MFD_VIEW_WID-1-(2*MFD_MARGIN_WID));
   gr_string_size(str,&sw,&sh);
   // special annointed string gets illustrative bitmap.
   if(!wire && !(((gridFlowPuzzle*)puzzle)->gfLayout.have_won)) {
      foot.bits=MFDPANEL_MEMORY_BAG;
      bmap=(OK==load_res_bitmap(&foot,REF_IMG_GridHelpSwitch,FALSE));
      bw=foot.w;
      bh=foot.h;
   }
   x=(MFD_VIEW_WID-sw)/2;
   y=(MFD_VIEW_HGT-sh-bh)/2;
   mfd_string_wrap = FALSE;
   mfd_draw_string(str,x,y,GREEN_YELLOW_BASE+3,TRUE);
   unwrap_text(str);
   mfd_string_wrap=save_w;
   if(bmap) {
#ifdef PUZZ_DIFF_NOHELP
      if(PUZZLE_DIFFICULTY<MAX_DIFFICULTY)
         bcolor=gpz_base_colors[((gridFlowPuzzle*)puzzle)->control_alg];
      else
         bcolor=GPZ_NOALG_COLOR;
#else
      bcolor=gpz_base_colors[((gridFlowPuzzle*)puzzle)->gfLayout.control_alg];
#endif
      x=(MFD_VIEW_WID-bw)/2;
      y+=sh;
      grid_help_clut[SWITCH_COLOR]=bcolor;
      ss_clut_ubitmap(&foot,x,y,grid_help_clut);
   }

}

int wirepos_score(wirePosPuzzle *wppz)
{
   int i, sc=0, j;
   for (i=0; i<wppz->wirecnt; i++)
   {
      wirePTrg *wpt=&wppz->wires[i];
      if (wppz->scorealg!=0)
      {
         if (memcmp(&wpt->cur,&wpt->targ,sizeof(wirePos))==0)
          { sc+=P_CORRECT;  score_spew("crct %d...",i); }
         else if ((wpt->cur.lpos==wpt->targ.lpos)||(wpt->cur.rpos==wpt->targ.rpos))
          { sc+=P_POS_OK;   score_spew("pos %d...",i); }
         else if (get_delta(wpt->cur)==get_delta(wpt->targ))
          { sc+=P_DELTA_OK; score_spew("dlta %d...",i); }
         else score_spew("fail %d...",i);
      }
      else
      {
         int bsc=0;
         for (j=0; j<wppz->wirecnt; j++)
         {
            wirePTrg *tpt=&wppz->wires[j];
            if (memcmp(&wpt->cur,&tpt->targ,sizeof(wirePos))==0)
             { bsc=P_CORRECT; score_spew2("crct %d %d...",i,j); }
            else if ((wpt->cur.lpos==wpt->targ.lpos)||(tpt->cur.rpos==wpt->targ.rpos))
             { if (D_POS_OK>bsc) { bsc=D_POS_OK; score_spew2("pos %d %d...",i,j); } }
            else if (get_delta(wpt->cur)==get_delta(tpt->targ))
             { if (D_DELTA_OK>bsc) { bsc=D_DELTA_OK; score_spew2("dlta %d %d...",i,j); } }
            else score_spew2("fail %d %d...",i,j);
         }
         sc+=bsc;
      }
   }
   score_spew("sc %d\n",sc);
   sc=(sc*wppz->scale)>>WP_SCALE_SHF;
   if (player_struct.drug_status[CPTRIP(GENIUS_DRUG_TRIPLE)] > 0)
      sc += sc>>2;
   if(sc>UCHAR_MAX)
      sc=UCHAR_MAX;
   return wppz->score=sc;
}

#define pegrand(peg,range) ((peg)?((range)-1):(rand()%(range)))
#define randvar(wppz) pegrand((wppz)->have_won,((wppz)->scale>>WP_SCALE_SHF)<<1)
#define wirepos_curscore(wpppppz) ((wpppppz)->score+randvar(wpppppz)-((wpppppz)->scale>>WP_SCALE_SHF))

#ifndef GAMEONLY
#define wirepos_spew(wz,csc) \
   mprintf("%d-%d, %d-%d, %d-%d, %d-%d, taps %2.2x %2.2x, wim %x tick %x sc %d cs %d\n", \
      wz->wires[0].cur.lpos, wz->wires[0].cur.rpos, \
      wz->wires[1].cur.lpos, wz->wires[1].cur.rpos, \
      wz->wires[2].cur.lpos, wz->wires[2].cur.rpos, \
      wz->wires[3].cur.lpos, wz->wires[3].cur.rpos, \
      wz->left_tap, wz->right_tap, wz->wire_in_motion, wz->wim_tick, wz->score, csc)
#else
#define wirepos_spew(wz,csc)
#endif

void wirepos_setup_buttons(wirePosPuzzle *wppz)
{
   LGPoint bsize = { ACCESSP_BTN_WD, ACCESSP_BTN_HGT };
   LGPoint bdims = { ACCESSP_BTN_COL, ACCESSP_BTN_ROW} ;
   LGRect r = { { ACCESSP_BTN_X, ACCESSP_BTN_Y},            
              { ACCESSP_BTN_X + ACCESSP_FULL_WD, ACCESSP_BTN_Y + ACCESSP_FULL_HGT } };

   bdims.y=wppz->pincnt;
   r.lr.y=ACCESSP_BTN_Y + (ACCESSP_BTN_HGT*wppz->pincnt);
   MFDBttnArrayResize(&(mfd_funcs[MFD_ACCESSPANEL_FUNC].handlers[0]),
      &r,bdims,bsize);
}

// a1 is master data, a2 target pos, a3 current pos
bool wirepos_3int_init(wirePosPuzzle *wppz, int a1, int a2, int a3)
{  // restore the puzzle, for now just init it
   int i;
   wppz->wirecnt=a1&0xf;   a1>>=4;     if (wppz->wirecnt==0) wppz->wirecnt=4;
   wppz->pincnt=a1&0xf;    a1>>=4;     if (wppz->pincnt==0)  wppz->pincnt =6;
   wppz->tscore=a1&0xff;   a1>>=8;     if (wppz->tscore==0)  wppz->tscore =128;
   wppz->scorealg=a1&0xf;  a1>>=4;
   wppz->have_won=a1&0xf;  a1>>=4;     
   wppz->scale=(256<<WP_SCALE_SHF)/(P_CORRECT*wppz->wirecnt);
   wppz->left_tap=wppz->right_tap=0;
   // on highest difficulty, always use colored wires, maybe add some
   // spurious pins.
   if(PUZZLE_DIFFICULTY==MAX_DIFFICULTY) {
      int p=wppz->pincnt;
      wppz->scorealg=1;
      wppz->pincnt=p+((player_struct.panel_ref)%(6-p+1));
      wppz->tscore=255;
   }
   for (i=0; i<wppz->wirecnt; i++)
   {
      wppz->wires[i].cur.lpos  = a3&0x7; wppz->left_tap |=(1<<(a3&7)); a3>>=3;
      wppz->wires[i].cur.rpos  = a3&0x7; wppz->right_tap|=(1<<(a3&7)); a3>>=3;
      wppz->wires[i].targ.lpos = a2&0x7; a2>>=3;
      wppz->wires[i].targ.rpos = a2&0x7; a2>>=3;
   }
   wirepos_score(wppz);
   wppz->wires_moved=1;
   wppz->last_score=wppz->score;
   wppz->wire_in_motion=0xff;
   wppz->wim_tick=wppz->wim_shown=0;
   access_primary_mfd=-1;
   return TRUE;
}

#define WIN_MASK 0x0F00000

void wirepos_3int_update(wirePosPuzzle *wppz)
{
   int p2,p4=0,cshf=0,i;
   p2=objFixtures[objs[wppz->our_id].specID].p2;
   if (wppz->have_won) p2|=WIN_MASK; else p2&=~WIN_MASK;
   for (i=0; i<wppz->wirecnt; i++, cshf+=6)
   {
      p4|=(wppz->wires[i].cur.lpos << (cshf));
      p4|=(wppz->wires[i].cur.rpos << (cshf + 3));
   }
   objFixtures[objs[wppz->our_id].specID].p2=p2;
   objFixtures[objs[wppz->our_id].specID].p4=p4;
}

int wirepos_iswire(wirePosPuzzle *wppz, int wim_code)
{
   int i, owire=wim_code&BTN_MASK;
   if (wim_code&LR_MASK)
   {
      for (i=0; i<wppz->wirecnt; i++)
         if (wppz->wires[i].cur.rpos==owire)
            return i;
   }
   else
   {
      for (i=0; i<wppz->wirecnt; i++)
         if (wppz->wires[i].cur.lpos==owire)
            return i;
   }
   return -1;
}

#define ALLOW_FLIP

bool wirepos_moveto(wirePosPuzzle *wppz, int wim_code)
{
   int wim_tap, retv;

   if (wim_code&LR_MASK) wim_tap=wppz->right_tap; else wim_tap=wppz->left_tap;
   if ( (wppz->wire_in_motion!=wim_code)&&
       ((wppz->wire_in_motion&LR_MASK)==(wim_code&LR_MASK))
#ifndef ALLOW_FLIP
      (((1<<(wim_code&BTN_MASK))&wim_tap)==0))
#endif
            )
   {
      int owire=wppz->wire_in_motion&BTN_MASK, twire=wim_code&BTN_MASK, loc;
      loc=wirepos_iswire(wppz,wppz->wire_in_motion);
      if (loc==-1)
      {
       	  retv=FALSE; 
      }
      else
      {
#ifdef ALLOW_FLIP
         if (((1<<twire)&wim_tap)!=0)
         {
            int oloc=wirepos_iswire(wppz,wim_code);
            if (wim_code&LR_MASK)
             { wppz->wires[loc].cur.rpos=twire; wppz->wires[oloc].cur.rpos=owire; }
            else
             { wppz->wires[loc].cur.lpos=twire; wppz->wires[oloc].cur.lpos=owire; }
         }
         else
#endif
         {
            wim_tap&=~(1<<owire); wim_tap|=(1<<twire);
            if (wim_code&LR_MASK)
             { wppz->right_tap=wim_tap; wppz->wires[loc].cur.rpos=twire; }
            else
             { wppz->left_tap=wim_tap; wppz->wires[loc].cur.lpos=twire; }
         }
      }
      retv=TRUE;
   }
   else retv=FALSE;
   wppz->wire_in_motion=NO_WIRE_IN_MOTION;
   return retv;
}

int wirepos_rescore_n_check(wirePosPuzzle *wppz)
{  // score and check, if solved, open us
   int score=wirepos_curscore(wppz);
   if (score>255) score=255;
   if ((wppz->have_won==0)&&(score>=wppz->tscore))
   {
      accesspanel_trigger(player_struct.panel_ref);
      wppz->have_won=1;
      wirepos_3int_update(wppz);
      return -score;
   }
   return score;
}

bool mfd_accesspanel_button_handler(MFD* mfd, LGPoint bttn, uiEvent* ev, void*)
{
   wirePosPuzzle *wppz=(wirePosPuzzle *)&player_struct.mfd_access_puzzles[0];
   int wim_code;

   if(mfd->id != access_primary_mfd)
      return TRUE;

   if ((ev->subtype&(MOUSE_LDOWN|UI_MOUSE_LDOUBLE))==0)
      return TRUE;

   if (wppz->have_won)
      return TRUE;

   wppz->wires_moved=1;

   wim_code=bttn.y+(bttn.x?LR_MASK:0);

   // parse input, move appropriately
   if (wppz->wire_in_motion!=NO_WIRE_IN_MOTION)
   {  // put it down
      if (wirepos_moveto(wppz,wim_code))
      {
         wirepos_score(wppz);
      }
   }
   else  // pick it up
   {
      if (wirepos_iswire(wppz,wim_code)!=-1)
      {
         wppz->wire_in_motion=wim_code;
      }
   }
//   wirepos_spew(wppz,53);
   mfd_notify_func(MFD_ACCESSPANEL_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
   wirepos_3int_update(wppz);
   return TRUE;
}

char mfd_type_accesspanel(ObjID id)
{
   int p2;

   p2=objFixtures[objs[id].specID].p2;
   p2=(p2>>28)&0xF;

   if(p2==WIREPUZ_CODE)
      return MFD_ACCESSPANEL_FUNC;
   else {
      return MFD_GRIDPANEL_FUNC;
   }
}

char mfd_setup_accesspanel(uchar special, ObjID id)
{
   void mfd_setup_wirepanel(uchar special, ObjID id);
   void mfd_setup_gridpanel(ObjID id);
   char func;

   func=mfd_type_accesspanel(id);

   if(func==MFD_ACCESSPANEL_FUNC)
      mfd_setup_wirepanel(special, id);
   else {
      mfd_setup_gridpanel(id);

      SIGN_US();
      SHADOW_PANEL_ID=id;
      SHADOW_PANEL_LEV=player_struct.level;
      memcpy(SHADOW_PANEL_STOR,player_struct.mfd_access_puzzles,sizeof(player_struct.mfd_access_puzzles));
   }
   return func;
}

uchar mfd_solve_accesspanel(ObjID id)
{
   int p2;
   bool retval;

   p2=objFixtures[objs[id].specID].p2;
   p2=(p2>>28)&0xF;

   if(p2==WIREPUZ_CODE)
      retval = mfd_solve_wirepanel();
   else {
      retval = mfd_solve_gridpanel();
      // okay, grid panels trigger in the expose func, when they
      // show you you've won, whereas wire puzzles trigger when they
      // score.  So, only in this case have we not triggered yet.

      if(retval==EPICK_SOLVED)
         accesspanel_trigger(player_struct.panel_ref);
   }
   return(retval);
}
   

// Does every thing but set the slot..
// Special is how much SHODAN has defeated us by
void mfd_setup_wirepanel(uchar special, ObjID id)
{
   int p2,p3,p4;
   wirePosPuzzle *wppz=(wirePosPuzzle *)&player_struct.mfd_access_puzzles[0];
   p2=objFixtures[objs[id].specID].p2;
   p3=objFixtures[objs[id].specID].p3;
   p4=objFixtures[objs[id].specID].p4;
   if (p3==0) p3=0x1c342a50;
   if (p4==0) p4=0x0cb530a1;
   wppz->special=special;
   wppz->our_id=id;
   wirepos_3int_init(wppz,p2,p3,p4);
   mfd_notify_func(MFD_ACCESSPANEL_FUNC, MFD_INFO_SLOT, TRUE, MFD_ACTIVE, TRUE);
}

bool mfd_solve_wirepanel()
{
   wirePosPuzzle *wppz=(wirePosPuzzle*)&player_struct.mfd_access_puzzles[0];
   int wire, swapper, targ;
   int score;

   if(wppz->have_won) {
      return EPICK_PRESOL;
   }

   for(wire=0;wire<wppz->wirecnt;wire++) {
#define MINIMAL_SOLUTION
#ifdef MINIMAL_SOLUTION
      // move one wire to target, swapping it with a later wire if
      // necessary (we should never need to swap with a previous
      // wire, since they are already in their target positions, 
      // which should never share pins with our target position.

      targ=wppz->wires[wire].targ.lpos;
      for(swapper=wire+1;swapper<wppz->wirecnt;swapper++) {
         if(wppz->wires[swapper].cur.lpos == targ) {
            wppz->wires[swapper].cur.lpos = wppz->wires[wire].cur.lpos;
            wppz->wires[wire].cur.lpos = targ;
         }
      }
      if(wppz->wires[wire].cur.lpos!=targ)
         wppz->wires[wire].cur.lpos=targ;

      targ=wppz->wires[wire].targ.rpos;
      for(swapper=wire+1;swapper<wppz->wirecnt;swapper++) {
         if(wppz->wires[swapper].cur.rpos == targ) {
            wppz->wires[swapper].cur.rpos = wppz->wires[wire].cur.rpos;
            wppz->wires[wire].cur.rpos = targ;
         }
      }
      if(wppz->wires[wire].cur.rpos!=targ)
         wppz->wires[wire].cur.rpos=targ;

      // rescore, and if target score has been achieved, we are done.
      wirepos_score(wppz);
      score=wirepos_rescore_n_check(wppz);
      if(wppz->have_won) {
         wppz->score=score>0?score:-score;
         wirepos_3int_update(wppz);
         break;
      }
#else
      wppz->wires[wire].cur = wppz->wires[wire].targ;
#endif
   }

#ifndef MINIMAL_SOLUTION
      // rescore, and if target score has been achieved, we are done.
   wirepos_score(wppz);
   score=wirepos_rescore_n_check(wppz);
   if(wppz->have_won) {
      wppz->score=score>0?score:-score;
      wirepos_3int_update(wppz);
   }
#endif

   mfd_notify_func(MFD_ACCESSPANEL_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
   // always return success: if we've gotten to this point without solving
   // the puzzle, it should be because of randomness in rescore_n_check
   return(EPICK_SOLVED);
//   return(wppz->have_won?EPICK_SOLVED:EPICK_FAILED);
}
   
static uchar wire_base[MAX_P_WIRES]={0x35,0x22,0x78,0x4B,0x5A,0x41};
#define ACCESSP_WIRE_DULL  3

#define ACCESSP_BLINK_MASK 0x40

#define accessp_score_rect_f(f,ls,rs,off) \
   f(ACCESSP_SCORE_X-off+((ls)>>ACCESSP_SCORE_SHF),ACCESSP_SCORE_YT-off,ACCESSP_SCORE_X+off+((rs)>>ACCESSP_SCORE_SHF),ACCESSP_SCORE_YB+off)

#define accessp_score_rect(ls,rs,off) \
   ACCESSP_SCORE_X-off+((ls)>>ACCESSP_SCORE_SHF),ACCESSP_SCORE_YT-off,ACCESSP_SCORE_X+off+((rs)>>ACCESSP_SCORE_SHF),ACCESSP_SCORE_YB+off

#define accessp_score_cmp(sc1,sc2,cmp) \
   ((sc1>>ACCESSP_SCORE_SHF) cmp (sc2>>ACCESSP_SCORE_SHF))

// if no mfd is on the given slot, return -1
// else, return the id of the lowest-numbered
// mfd with this slot.
errtype mfd_accesspanel_init(MFD_Func* f)
{
   int cnt = 0;
   errtype err;
   LGPoint bsize = {ACCESSP_BTN_WD, ACCESSP_BTN_HGT};
   LGPoint bdims = {ACCESSP_BTN_COL, ACCESSP_BTN_ROW};
   LGRect r = { {ACCESSP_BTN_X, ACCESSP_BTN_Y},
              {ACCESSP_BTN_X + ACCESSP_FULL_WD, ACCESSP_BTN_Y + ACCESSP_FULL_HGT} };
   err = MFDBttnArrayInit(&f->handlers[cnt++],&r,bdims,bsize,mfd_accesspanel_button_handler,NULL);
   if (err != OK) return err;
   f->handler_count = cnt;
#ifdef WORKING_INC_MFDS
   player_struct.mfd_func_status[MFD_ACCESSPANEL_FUNC]|=1<<4;
#endif
   return OK;
}

bool mfd_accesspanel_handler(MFD*, uiEvent*)
{
   extern bool mfd_gridpanel_handler(MFD*,uiEvent*);
#ifdef EPICK_ON_CURSOR_TRY
   extern bool try_use_epick(ObjID panel, ObjID cursor_obj);

   if (ev->type != UI_EVENT_MOUSE || !(ev->subtype & (MOUSE_LDOWN|UI_MOUSE_LDOUBLE))) return FALSE;

   return(try_use_epick(player_struct.panel_ref, object_on_cursor));
#else
   return(FALSE);
#endif
}

int access_help_string(wirePosPuzzle *wppz)
{
   if(wppz->have_won)
      return(REF_STR_PanelSolved);
   else
      return(REF_STR_WirePuzzHelp+(wppz->wire_in_motion!=NO_WIRE_IN_MOTION));
}

void mfd_accesspanel_expose(MFD* mfd, ubyte control)
{
   void mfd_clear_view(void);
   wirePosPuzzle *wppz=(wirePosPuzzle *)&player_struct.mfd_access_puzzles[0];
   bool full = control & MFD_EXPOSE_FULL;
   if (control & MFD_EXPOSE) // Time to draw stuff
   {
      int cscore;
      bool primary;

      if((full && access_primary_mfd<0) || player_struct.mfd_current_slots[access_primary_mfd]!=MFD_INFO_SLOT) {
         full = TRUE;
         access_primary_mfd = mfd->id;
      }

      primary=(mfd->id == access_primary_mfd);

      if(primary) {
         if ((cscore=wirepos_rescore_n_check(wppz))<0)
         { mfd_notify_func(MFD_ACCESSPANEL_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE); cscore=-cscore; }
         cscore=wirepos_curscore(wppz); if (cscore<0) cscore=-cscore; if (cscore>255) cscore=255;
      }
      mfd_clear_rects();
      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      if(!primary) {
         // draw help string, if different from last string or
         // if full expose.
         int prev = last_access_help_string;
         char buf[80];

         last_access_help_string=access_help_string(wppz);
         if(full || last_access_help_string!=prev) {
            mfd_clear_view();
            get_string(last_access_help_string, buf, 80);
            draw_help_text( buf, TRUE, wppz );
         }
         gr_pop_canvas();
         mfd_update_rects(mfd);
         return;
      }
      if (full)
      {
         wirepos_setup_buttons(wppz);
         mfd_clear_view();  // draw full score (should color based on score), every other line?
         gr_set_fcolor(ACCESSP_SCORE_BOR);
         accessp_score_rect_f(ss_box,0,255,1);
         gr_set_fcolor(ACCESSP_SCORE_COL);
         accessp_score_rect_f(ss_rect,0,cscore,0);
         gr_set_fcolor(ACCESSP_TARG_COL);
         ss_vline(ACCESSP_SCORE_X+(wppz->tscore>>ACCESSP_SCORE_SHF)-1,ACCESSP_SCORE_YT,ACCESSP_SCORE_YB-1);
      }
      else if (accessp_score_cmp(wppz->last_score,cscore,<))
      {  // draw more score delta
         gr_set_fcolor(ACCESSP_SCORE_COL);
         accessp_score_rect_f(ss_rect,wppz->last_score,cscore,0);
         mfd_add_rect(accessp_score_rect(wppz->last_score,cscore,0));
         if (accessp_score_cmp(wppz->tscore,wppz->last_score,>=)&&accessp_score_cmp(cscore,wppz->tscore,>=))
         {
            gr_set_fcolor(ACCESSP_TARG_COL);
            ss_vline(ACCESSP_SCORE_X+(wppz->tscore>>ACCESSP_SCORE_SHF)-1,ACCESSP_SCORE_YT,ACCESSP_SCORE_YB-1);
         }
      }
      else if (accessp_score_cmp(wppz->last_score,cscore,>))
      {  // put back correct part of background
         accessp_score_rect_f(ss_safe_set_cliprect,cscore,wppz->last_score,0);
         if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
            gr_bitmap(&mfd_background, 0, 0);
         ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
         mfd_add_rect(accessp_score_rect(cscore,wppz->last_score,0));
         if (accessp_score_cmp(cscore,wppz->tscore,<=)&&accessp_score_cmp(wppz->tscore,wppz->last_score,<=))
         {
            gr_set_fcolor(ACCESSP_TARG_COL);
            ss_vline(ACCESSP_SCORE_X+(wppz->tscore>>ACCESSP_SCORE_SHF)-1,ACCESSP_SCORE_YT,ACCESSP_SCORE_YB-1);
         }
      }
      wppz->last_score=cscore;
      if (full||wppz->wires_moved)
      {  // draw wires
         int i;
         int lry=ACCESSP_BTN_Y+(ACCESSP_BTN_HGT*wppz->pincnt);
         wppz->wires_moved=0;
         if (!full)
         {
            ss_safe_set_cliprect(ACCESSP_BTN_X,ACCESSP_BTN_Y,ACCESSP_BTN_X+ACCESSP_FULL_WD,lry);
            if (!full_game_3d)
//KLC - chg for new art         ss_bitmap(&mfd_background, 0, 0);
         		 gr_bitmap(&mfd_background, 0, 0);
            mfd_add_rect(ACCESSP_BTN_X,ACCESSP_BTN_Y,ACCESSP_BTN_X+ACCESSP_FULL_WD,lry);
            ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
         }
         for (i=0; i<wppz->pincnt; i++)
         {
            gr_set_fcolor(ACCESSP_PIN_COL);
            ss_box(ACCESSP_BTN_X+1,ACCESSP_BTN_Y+(ACCESSP_BTN_HGT*i),
                     ACCESSP_BTN_X+ACCESSP_BTN_WD,ACCESSP_BTN_Y-1+(ACCESSP_BTN_HGT*(i+1)));
            ss_box(ACCESSP_BTN_X+ACCESSP_FULL_WD-ACCESSP_BTN_WD,ACCESSP_BTN_Y+(ACCESSP_BTN_HGT*i),
                     ACCESSP_BTN_X+ACCESSP_FULL_WD-1,ACCESSP_BTN_Y-1+(ACCESSP_BTN_HGT*(i+1)));
            gr_set_fcolor(ACCESSP_PIN_COL+6);
            ss_box(ACCESSP_BTN_X,ACCESSP_BTN_Y+(ACCESSP_BTN_HGT*i)-1,
                     ACCESSP_BTN_X+ACCESSP_BTN_WD+1,ACCESSP_BTN_Y+(ACCESSP_BTN_HGT*(i+1)));
            ss_box(ACCESSP_BTN_X+ACCESSP_FULL_WD-ACCESSP_BTN_WD-1,ACCESSP_BTN_Y+(ACCESSP_BTN_HGT*i)-1,
                     ACCESSP_BTN_X+ACCESSP_FULL_WD,ACCESSP_BTN_Y+(ACCESSP_BTN_HGT*(i+1)));
         }
         gr_set_fcolor(ACCESSP_CHIP_COL);
         ss_box(ACCESSP_BTN_X+ACCESSP_BTN_WD,ACCESSP_BTN_Y,
            ACCESSP_BTN_X+ACCESSP_FULL_WD-ACCESSP_BTN_WD,lry-1);
         gr_set_fcolor(ACCESSP_CHIP_COL+7);
         ss_box(ACCESSP_BTN_X+ACCESSP_BTN_WD+1,ACCESSP_BTN_Y+1,
            ACCESSP_BTN_X+ACCESSP_FULL_WD-ACCESSP_BTN_WD-1,lry-1-1);
         for (i=0; i<wppz->wirecnt; i++)
         {
            int lfti,rghi;
            int basecol;

            lfti=wppz->wires[i].cur.lpos;
            rghi=wppz->wires[i].cur.rpos;
            basecol=wire_base[ wppz->scorealg==0?(player_struct.panel_ref&3):i ];
            gr_set_fcolor(basecol);
            ss_thick_int_line(ACCESSP_BTN_X+(ACCESSP_BTN_WD/2),ACCESSP_BTN_Y+(ACCESSP_BTN_HGT*lfti)+(ACCESSP_BTN_HGT/2)-1,
                        ACCESSP_BTN_X+ACCESSP_FULL_WD-1-(ACCESSP_BTN_WD/2),ACCESSP_BTN_Y+(ACCESSP_BTN_HGT*rghi)+(ACCESSP_BTN_HGT/2)-1);
            gr_set_fcolor(basecol+3);
            ss_thick_int_line(ACCESSP_BTN_X+(ACCESSP_BTN_WD/2),ACCESSP_BTN_Y+(ACCESSP_BTN_HGT*lfti)+(ACCESSP_BTN_HGT/2),
                        ACCESSP_BTN_X+ACCESSP_FULL_WD-1-(ACCESSP_BTN_WD/2),ACCESSP_BTN_Y+(ACCESSP_BTN_HGT*rghi)+(ACCESSP_BTN_HGT/2));
         }
      }
      if ((wppz->wire_in_motion!=NO_WIRE_IN_MOTION)&&(((*tmd_ticks)&ACCESSP_BLINK_MASK)!=wppz->wim_tick))
      {  // if already shown but not in motion, it is already redrawn, baby
         int i=wppz->wire_in_motion&BTN_MASK;
         wppz->wim_tick=(*tmd_ticks)&ACCESSP_BLINK_MASK;
         if ((wppz->wim_tick==ACCESSP_BLINK_MASK)||(wppz->wire_in_motion==NO_WIRE_IN_MOTION))
          { gr_set_fcolor(0); wppz->wim_shown=0; }
         else
          { gr_set_fcolor(ACCESSP_PIN_COL-2); wppz->wim_shown=1; }
         if (wppz->wire_in_motion&LR_MASK)
         {
            ss_rect(ACCESSP_BTN_X+ACCESSP_FULL_WD-ACCESSP_BTN_WD+1,ACCESSP_BTN_Y+1+(ACCESSP_BTN_HGT*i),
                     ACCESSP_BTN_X+ACCESSP_FULL_WD-1-1,ACCESSP_BTN_Y-1-1+(ACCESSP_BTN_HGT*(i+1)));
            mfd_add_rect(ACCESSP_BTN_X+ACCESSP_FULL_WD-ACCESSP_BTN_WD+1,ACCESSP_BTN_Y+1+(ACCESSP_BTN_HGT*i),
                     ACCESSP_BTN_X+ACCESSP_FULL_WD-1-1,ACCESSP_BTN_Y-1-1+(ACCESSP_BTN_HGT*(i+1)));
         }
         else
         {
            ss_rect(ACCESSP_BTN_X+1+1,ACCESSP_BTN_Y+1+(ACCESSP_BTN_HGT*i),
                     ACCESSP_BTN_X+ACCESSP_BTN_WD-1,ACCESSP_BTN_Y-1-1+(ACCESSP_BTN_HGT*(i+1)));
            mfd_add_rect(ACCESSP_BTN_X+1+1,ACCESSP_BTN_Y+1+(ACCESSP_BTN_HGT*i),
                     ACCESSP_BTN_X+ACCESSP_BTN_WD-1,ACCESSP_BTN_Y-1-1+(ACCESSP_BTN_HGT*(i+1)));
         }
      }
      gr_pop_canvas();
      // Now that we've popped the canvas, we can send the 
      // updated mfd to screen
      mfd_update_rects(mfd);
#ifndef WORKING_INC_MFDS
      mfd_notify_func(MFD_ACCESSPANEL_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
#endif
   }
   else
   {
      ObjID obj = panel_ref_unexpose(mfd->id,MFD_ACCESSPANEL_FUNC);
      if (obj != OBJ_NULL)
      {
         objs[obj].info.current_frame = 0;
      }


      return ;      
   }
}


// returns the "charged" version of the given state, if any.  For states
// which cannot be charged (empty and open) return the given state.
//
gpz_state gpz_charge_state(gpz_state s)
{
   if(s==GPZ_EMPTY)
      return s;
   return((gpz_state)((int)s | 1));
}

// returns the "uncharged" version of the given state, if any.  Note that
// empty and open are their own uncharged states.
gpz_state gpz_uncharge_state(gpz_state s)
{
   if(s==GPZ_OPEN)
      return s;
   return((gpz_state)((int)s & ~1));
}

// returns TRUE iff s is a "charged" state.
bool gpz_is_charged(gpz_state s)
{
   return(s!=GPZ_OPEN && (s&1));
}

void gpz_toggle_state(gridFlowPuzzle *gfpz, short r, short c)
{
   gpz_state s;

   s=gpz_get_grid_state(gfpz,r,c);
   s=gpz_uncharge_state(s);
   if(s==GPZ_OPEN)
      gpz_set_grid_state(gfpz,r,c,GPZ_CLOSED);
   else if(s==GPZ_CLOSED)
      gpz_set_grid_state(gfpz,r,c,GPZ_OPEN);
}

// bitarray_get()
// Treating "base" as an array of elements of given size "siz", get
// element number "off".  Works for bizzare sizes like 3 bits, packing.
// elements within ints and across int boundaries.
//
gpz_state bitarray_get(short siz, short off, uint *base)
{
   short loc, shft, ans, over;

   loc=(siz*off)/(sizeof(uint)*8);
   shft=(siz*off)%(sizeof(uint)*8);

   // get from appropriate offset
   ans=base[loc] >> shft;

   // did we cross a word boundary?
   over=siz-((sizeof(uint)*8)-shft);
   if(over>0) {
      ans=ans & ((1<<(siz-over))-1);

      ans|=base[loc+1] << (siz-over);
   }
      
   // mask down to siz bits
   ans=ans & ((1<<siz)-1);

   return (gpz_state)ans;
}

// bitarray_set()
// Treating "base" as an array of elements of given size "siz", set
// element number "off" to specified value "val."  Works for bizzare sizes
// like 3 bits, packing elements within an int and across int boundaries.
//
void bitarray_set(short siz, short off, ushort val, uint *base)
{
   short loc, shft, over;

   loc=(siz*off)/(sizeof(uint)*8);
   shft=(siz*off)%(sizeof(uint)*8);

   // restrict to valid range
   val &= (1<<siz)-1;

   // clear destination bits
   base[loc] &= ~(((1<<siz)-1)<<shft);
   // set new bits
   base[loc] |= val<<shft;

   // did we cross a word boundary?
   over=siz-((sizeof(uint)*8)-shft);
   if(over>0) {
      base[loc+1] &= ~((1<<over)-1);
      base[loc+1] |= val>>(siz-over);
   }
}

errtype mfd_gridpanel_init(MFD_Func* f)
{
   bool mfd_gridpanel_button_handler(MFD* mfd, LGPoint bttn, uiEvent* ev, void* data);
   int cnt = 0;
   errtype err;
   LGPoint bsize = { GRIDP_BTN_WD, GRIDP_BTN_HGT };
   LGPoint bdims = { GRIDP_BTN_COL, GRIDP_BTN_ROW} ;
   LGRect r = { { GRIDP_BTN_X, GRIDP_BTN_Y},            
              { GRIDP_BTN_X + GRIDP_FULL_WD, GRIDP_BTN_Y + GRIDP_FULL_HGT } }; 
   err = MFDBttnArrayInit(&f->handlers[cnt++],&r,bdims,bsize,mfd_gridpanel_button_handler,NULL);
   if (err != OK) return err;
   f->handler_count = cnt;
   gridCursorbm.bits = gridCursorBits;
   return OK;      
}

#define GPZ_GOOD_SEARCH_DEPTH 4
int gpz_search_depth(gridFlowPuzzle* gfpz)
{
   if(gfpz->gfLayout.control_alg==GPZ_SIMPLE)
      return(gfpz->gfLayout.rows*gfpz->gfLayout.cols);
   else
      return(GPZ_GOOD_SEARCH_DEPTH);
}

//
// When I lie next to you
// I shiver and shake.
// Tell me you love me,
// I dream I'm awake.
//

// attempts to add a gate to a grid puzzle in such a way as to keep
// it solvable.  Finds candidates for this new gate and tries them
// out with the same solver used by in-game "logic probes".  If it can
// solve the puzzle with the new gate, it accepts the new gate.
//
// Times out in this attempt after 1 second.
//
void gpz_add_gate(gridFlowPuzzle *gfpz, ObjID me)
{
   int r,c,count,rr,cc,n;
   gpz_state adj,tmp;
   ulong timeout;
   ushort seed=(ushort)me;

   // if already opened panel once, don't bother.
   // note that this isn't really correct, because there exists
   // a panel that you might click on without opening it, because
   // it has a comparator.  However, we secretly know that the
   // puzzle in this case is not one which can get a new gate
   // anyway, so this happens to work out.  Beware.
   
   if(objs[me].info.inst_flags & OLH_INST_FLAG)
      return;

   timeout=*tmd_ticks+(CIT_CYCLE);

   gpz_uncharge_grid(gfpz);
   // find a node which could possibly be charged, and has
   // at least three adjacent nodes which could possibly be charged.
   // try changing it to a gate and seeing if you can solve the
   // puzzle.
   for(rr=0;rr<gfpz->gfLayout.rows;rr++) {
      for(cc=0;cc<gfpz->gfLayout.cols;cc++) {
         // note that tight_loop is getting called in the time-consuming
         // part of this, find_winning_move, so hopefully we don't have
         // to call it here.

         // change our origin from 0,0 so we don't always put 
         // gates in the upper left.
         n=rr+cc*gfpz->gfLayout.rows;
         n=(n+seed)%(gfpz->gfLayout.rows*gfpz->gfLayout.cols);
         r=n%gfpz->gfLayout.rows;
         c=n/gfpz->gfLayout.rows;
         if(gpz_get_grid_state(gfpz,r,c)==GPZ_EMPTY)
            continue;
         // count adjacent chargeable nodes.
         {
            count=0;
            adj=gpz_get_grid_state(gfpz,r-1,c);
            count+=(adj!=GPZ_EMPTY);
            adj=gpz_get_grid_state(gfpz,r+1,c);
            count+=(adj!=GPZ_EMPTY);
            adj=gpz_get_grid_state(gfpz,r,c-1);
            count+=(adj!=GPZ_EMPTY);
            adj=gpz_get_grid_state(gfpz,r,c+1);
            count+=(adj!=GPZ_EMPTY);
         }
         if(count>=3) {
            tmp=gpz_get_grid_state(gfpz,r,c);
            gpz_set_grid_state(gfpz,r,c,GPZ_GATE);
            if(find_winning_move(gfpz,NULL,gpz_search_depth(gfpz),NULL,FALSE,timeout)) {
               // it's solvable.  Keep the change.
               return;
            }
            else if(*tmd_ticks>timeout) {
               gpz_set_grid_state(gfpz,r,c,tmp);
               return;
            }
            gpz_set_grid_state(gfpz,r,c,tmp);
         }
      }
   }
   return;
}

bool mfd_solve_gridpanel()
{
   gridFlowPuzzle *gfpz=(gridFlowPuzzle*)&player_struct.mfd_access_puzzles[0];
   gridFlowPuzzle solved;
   int search_depth;
   bool found, shadow;
   ObjID id=gfpz->gfLayout.our_id;
   uchar temp[sizeof(player_struct.mfd_access_puzzles)];
   // for slow machines; don't give probe more than a couple of
   // seconds to solve a panel.  This time limit is set to 0 to
   // mean no time limit.
   long logic_probe_timeout=*tmd_ticks+EPICK_TIMEOUT;

   if(gfpz->gfLayout.have_won)
      return EPICK_PRESOL;

   shadow=(WE_ARE_SIGNED() && SHADOW_PANEL_ID==id && SHADOW_PANEL_LEV==player_struct.level && !(objFixtures[objs[id].specID].p2 & 0x10000) );
   if(shadow) {
      memcpy(temp,player_struct.mfd_access_puzzles,sizeof(player_struct.mfd_access_puzzles));
      memcpy(player_struct.mfd_access_puzzles,SHADOW_PANEL_STOR,sizeof(player_struct.mfd_access_puzzles));
   }

   search_depth=gpz_search_depth(gfpz);

   found = find_winning_move(gfpz,&solved,search_depth,NULL,FALSE,logic_probe_timeout);
   if(found && solved.gfLayout.have_won) {
      *gfpz=solved;
      gpz_4int_update(gfpz);
      mfd_notify_func(MFD_GRIDPANEL_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
   }
   else if(shadow)
      memcpy(player_struct.mfd_access_puzzles,temp,sizeof(player_struct.mfd_access_puzzles));

   return(gfpz->gfLayout.have_won?EPICK_SOLVED:EPICK_FAILED);
}

// gets state of grid node at coordinates row,col.  If row or col is off
// the grid, then GPZ_EMPTY is returned.
//
#define GPZ_PERIM_TYPEMASK 0x10
#define GPZ_PERIM_SIDEMASK 0x08
gpz_state gpz_get_grid_state(gridFlowPuzzle *gfpz, short row, short col)
{
   void gpz_perimeter_to_grid(gridFlowPuzzle *gfpz, ushort per, short* r, short* c);
   short r,c,*q;

   if(row<0 || col<0 || row>=(gfpz->gfLayout.rows) || col>=(gfpz->gfLayout.cols)) {
      gpz_perimeter_to_grid(gfpz,gfpz->gfLayout.src,&r,&c);
      if(gfpz->gfLayout.src & GPZ_PERIM_TYPEMASK) q=&c; else q=&r;
      *q+=(gfpz->gfLayout.src & GPZ_PERIM_SIDEMASK)?1:-1;

      if(row==r && c==col) return GPZ_CLOSED_CHARGED;
      
      gpz_perimeter_to_grid(gfpz,gfpz->gfLayout.dest,&r,&c);
      if(gpz_is_charged(gpz_get_grid_state(gfpz,r,c))) {
         if(gfpz->gfLayout.dest & GPZ_PERIM_TYPEMASK) q=&c; else q=&r;
         *q+=(gfpz->gfLayout.dest & GPZ_PERIM_SIDEMASK)?1:-1;

         if(row==r && c==col) return GPZ_CLOSED_CHARGED;
      }

      return(GPZ_EMPTY);
   }
   return(bitarray_get(GRIDP_STATE_BITS,col+row*(gfpz->gfLayout.cols),gfpz->states));
}

// sets state of grid node at coordinates row, col.  No effect if row or
// col is off the grid.
void gpz_set_grid_state(gridFlowPuzzle *gfpz, short row, short col, gpz_state val)
{
   if(row<0 || col<0 || row>=(gfpz->gfLayout.rows) || col>=(gfpz->gfLayout.cols))
      return;
   bitarray_set(GRIDP_STATE_BITS,col+row*(gfpz->gfLayout.cols),val,gfpz->states);
}

void gpz_uncharge_grid(gridFlowPuzzle *gfpz)
{
   int r,c;
   gpz_state s;

   for(r=0;r<gfpz->gfLayout.rows;r++) {
      for(c=0;c<gfpz->gfLayout.cols;c++) {
         s=gpz_get_grid_state(gfpz,r,c);
         gpz_set_grid_state(gfpz,r,c,gpz_uncharge_state(s));
      }
   }
}

void gpz_perimeter_to_grid(gridFlowPuzzle *gfpz, ushort per, short* r, short* c)
{
   ushort rr,cc,edge;

   edge=(per>>3)&3;
   per=per&7;

   rr=gfpz->gfLayout.rows; cc=gfpz->gfLayout.cols;

   switch(edge) {
      case 0:
         *r=0; *c=per;
         break;
      case 1:
         *r=rr-1; *c=per;
         break;
      case 2:
         *c=0; *r=per;
         break;
      case 3:
         *c=cc-1; *r=per;
         break;
   }
   return;
}

bool gpz_state_charged(gridFlowPuzzle *gfpz, short r, short c)
{
   return(gpz_is_charged(gpz_get_grid_state(gfpz,r,c)));
}

#ifdef SHOW_DONENESS
// Find the shortest Manhatten distance from a charged node to the
// destination node.  Converts this to a rating in the range 0 to
// 255 by normalizing to the greatest Manhatten distance from the
// destination of any node in the puzzle.

uchar gpz_doneness(gridFlowPuzzle *gfpz)
{
   int farthest, doneness, dist;
   short r,c,dr,dc;

   doneness=INT_MAX;
   farthest=INT_MIN;

   gpz_perimeter_to_grid(gfpz,gfpz->gfLayout.dest,&dr,&dc);

   for(r=0;r<gfpz->gfLayout.rows;r++) {
      for(c=0;c<gfpz->gfLayout.cols;c++) {

         dist=abs(r-dr)+abs(c-dc);
         if(dist>farthest)
            farthest=dist;
         if(gpz_state_charged(gfpz,r,c) && dist<doneness)
            doneness=dist;
      }
   }

   if(doneness==INT_MAX) return 0;

   return(UCHAR_MAX*(farthest-doneness)/farthest);
}
#endif

// recalculates "current" or "charge" flow through grid.  Returns TRUE
// iff destination node is charged.
//
bool gpz_propogate_charge_n_check(gridFlowPuzzle *gfpz)
{
   bool flow;
   short r,c,src_r,src_c;
   gpz_state s;
   extern errtype accesspanel_trigger(ObjID id);

   // uncharge whole grid
   gpz_uncharge_grid(gfpz);

   // charge source node
   gpz_perimeter_to_grid(gfpz,gfpz->gfLayout.src,&r,&c);

   s=gpz_get_grid_state(gfpz,r,c);
   if(s==GPZ_CLOSED||s==GPZ_FULL)
      gpz_set_grid_state(gfpz,r,c,gpz_charge_state(gpz_get_grid_state(gfpz,r,c)));

   // propogate charges until the propogatin's done.  This must terminate
   // 'cause we only set "flow" when we charge a node, and eventually we
   // will run out of nodes to charge.
   do {
      flow=FALSE;

      for(r=0;r<gfpz->gfLayout.rows;r++)
      {
         for(c=0;c<gfpz->gfLayout.cols;c++)
         {
         	   int	sum;
            s=gpz_get_grid_state(gfpz,r,c);
            src_r=r; src_c=c;
            switch(s)
            {
               case GPZ_CLOSED:
               case GPZ_FULL:
               case GPZ_GATE:
               	sum = (int)gpz_state_charged(gfpz, --src_r, src_c);
               	sum += (int)gpz_state_charged(gfpz, ++src_r, --src_c);
               	sum += (int)gpz_state_charged(gfpz, ++src_r, ++src_c);
               	sum += (int)gpz_state_charged(gfpz, --src_r, ++src_c);
               	if (s == GPZ_GATE)
               	{
               		if (sum >= 2)
               		{
	                     gpz_set_grid_state(gfpz,r,c,gpz_charge_state(s));
	                     flow=TRUE;
	                  }
               	}
               	else
               	{
               		if (sum > 0)
               		{
	                     gpz_set_grid_state(gfpz,r,c,gpz_charge_state(s));
	                     flow=TRUE;
	                  }
               	}
                  break;
/*
                  if(gpz_state_charged(gfpz,--src_r,src_c)
                     || gpz_state_charged(gfpz,++src_r,--src_c)
                     || gpz_state_charged(gfpz,++src_r,++src_c)
                     || gpz_state_charged(gfpz,--src_r,++src_c)) {
                     gpz_set_grid_state(gfpz,r,c,gpz_charge_state(s));
                     flow=TRUE;
                  }
                  break;
                  if(gpz_state_charged(gfpz,--src_r,src_c)
                     + gpz_state_charged(gfpz,++src_r,--src_c)
                     + gpz_state_charged(gfpz,++src_r,++src_c)
                     + gpz_state_charged(gfpz,--src_r,++src_c) >= 2) {
                     gpz_set_grid_state(gfpz,r,c,gpz_charge_state(s));
                     flow=TRUE;
                  }
                  break;
*/
               // default if state is already charged or cannot be charged.
               default:
                  break;
            }
         }
      }
   } while (flow);

   // do we win?
   gpz_perimeter_to_grid(gfpz,gfpz->gfLayout.dest,&r,&c);
   s=gpz_get_grid_state(gfpz,r,c);
   return(gpz_is_charged(s));
}

bool find_winning_move_from(gridFlowPuzzle *gfpz, gridFlowPuzzle *solved, LGPoint* move, int r0, int c0, int dep, ulong timeout)
{
   gpz_state gridpanel_move(LGPoint node,gridFlowPuzzle *gfpz);
   int r,c,real_c0=c0;
   gpz_state s, pass;
   LGPoint try_it;
   // this will never put more copies on the stack at a time than our total search depth 
   // (currently no more than 4) at 32 bytes per copy.
   gridFlowPuzzle copy;

   copy = *gfpz;

   // try all moves, working to the right and then down. Since moves are
   // commutative and reversible, we need only try moves in this one
   // particular order.  Try moving open circuits to closed circuits
   // first as a heuristic.  Also, don't bother with the second pass
   // (changing closed circuits to open circuits) if we have the simple
   // scoring algorithm.

   for(pass=GPZ_OPEN;pass!=GPZ_EMPTY;) {
      c0=real_c0;
      for(r=r0;r<gfpz->gfLayout.rows && dep>0;r++) {
         for(c=c0;c<gfpz->gfLayout.cols && dep>0;c++) {
            tight_loop(FALSE);
            if(timeout!=0 && *tmd_ticks>timeout) {
               return(FALSE);
            }
            s = gpz_uncharge_state(gpz_get_grid_state(&copy,r,c));
            if(s==pass) {
               try_it.x=c;
               try_it.y=r;
               gridpanel_move(try_it,&copy);

               if(gpz_propogate_charge_n_check(&copy)) {
                  if(move) *move=try_it;
                  if(solved) {
                     *solved=copy;
                     solved->gfLayout.have_won=TRUE;
                  }
                  return TRUE;
               }
               if(gfpz->gfLayout.control_alg==GPZ_SIMPLE) {
                  dep--;
               }
               else if(dep>1) {
                  int new_r0=r, new_c0=c+1;
                  
                  if(new_c0>=gfpz->gfLayout.cols) {
                     new_c0=0;
                     new_r0++;
                  }
                  if(new_r0<gfpz->gfLayout.rows && find_winning_move_from(&copy,solved,move,new_r0,new_c0,dep-1,timeout)) {
                     return TRUE;
                  }
                  else {
                     // undo this move and try another.
                     memcpy(copy.states,gfpz->states,sizeof(copy.states));
                  } 
               }
               else
                  memcpy(copy.states,gfpz->states,sizeof(copy.states));
            }
         }     
         c0=0;
      }
      if(pass==GPZ_OPEN && gfpz->gfLayout.control_alg!=GPZ_SIMPLE) {
         // solutions requiring steps of opening closed circuits
         // are likely to be inferior.  Do not search as deep.
         if(dep==1)
            pass=GPZ_EMPTY;
         else {
            dep--;
            pass=GPZ_CLOSED;
         }
      }
      else
         pass=GPZ_EMPTY; // in other words, stop.
   }
   return FALSE;
}


// be very careful with depth here; search time can increase exponentially
// with depth, ya know.
//
bool find_winning_move(gridFlowPuzzle *gfpz, gridFlowPuzzle *solved, int depth, LGPoint* move, bool breadth_first, ulong timeout)
{
   int d;

   if(breadth_first) {
      // brain-damaged breadth-first search by successively deep
      // depth first searches.
      for(d=1;d<=depth;d++)
         if(find_winning_move_from(gfpz, solved, move, 0, 0, d, timeout))
            return TRUE;
      return FALSE;
   }
   else {
      return(find_winning_move_from(gfpz, solved, move, 0, 0, depth, timeout));
   }
}

void gpz_setup_buttons(gridFlowPuzzle *gfpz)
{                                                   
   LGPoint bsize = { GRIDP_BTN_WD, GRIDP_BTN_HGT };
   LGPoint bdims = { GRIDP_BTN_COL, GRIDP_BTN_ROW} ;
   LGRect rct = { { GRIDP_BTN_X, GRIDP_BTN_Y},            
              { GRIDP_BTN_X + GRIDP_FULL_WD, GRIDP_BTN_Y + GRIDP_FULL_HGT } };

   bdims.x=gfpz->gfLayout.cols;
   bdims.y=gfpz->gfLayout.rows;
   rct.ul.x=(MFD_VIEW_WID-(GRIDP_BTN_WD*gfpz->gfLayout.cols))/2+GRIDP_X_OFFSET;
   rct.ul.y=(MFD_VIEW_HGT-(GRIDP_BTN_HGT*gfpz->gfLayout.rows))/2+GRIDP_Y_OFFSET;
   rct.lr.x=rct.ul.x+(GRIDP_BTN_WD*gfpz->gfLayout.cols);
   rct.lr.y=rct.ul.y+(GRIDP_BTN_HGT*gfpz->gfLayout.rows);
   MFDBttnArrayResize(&(mfd_funcs[MFD_GRIDPANEL_FUNC].handlers[0]),
      &rct,bdims,bsize);
}

void gpz_4int_init(gridFlowPuzzle *gfpz,uint,uint p2,uint p3,uint)
{
   uint state_init[4];
   int r,c,rr,cc,soff,easified=0;
   gpz_state		si;
   // strip reserved bits from p2
   p2 &= 0xFFFF;

   if(!objs[p2].active || objs[p2].obclass!=CLASS_TRAP) {
      memset(state_init,0,sizeof(state_init));
   }
   else {
      ObjTrap* other=&objTraps[objs[p2].specID];

      state_init[3]=other->p1;
      state_init[2]=other->p2;
      state_init[1]=other->p3;
      state_init[0]=other->p4;
   }

   gfpz->gfLayout.have_won = p3&0xF;   p3=p3>>4;
   gfpz->gfLayout.src = p3&0xFF;       p3=p3>>8;
   gfpz->gfLayout.dest = p3&0xFF;      p3=p3>>8;
   gfpz->gfLayout.cols = cc = p3&0xF;  p3=p3>>4;
   gfpz->gfLayout.rows = rr = p3&0xF;  p3=p3>>4;
   gfpz->gfLayout.control_alg = p3&0xF;

   gfpz->gfLayout.winmove_f = 0;
   mfd_gridpanel_set_winmove(FALSE);

#ifdef GRIDP_AUTO_SOLVE
   gfpz->gfLayout.solve_me = 0;
#endif

   soff=0;
   for(r=0;r<rr;r++) {
      for(c=0;c<cc;c++) {
         si=bitarray_get(GRIDP_STATE_BITS,soff++,state_init);
         si=gpz_uncharge_state(si);
         if(PUZZLE_DIFFICULTY<=1) {
            switch(si) {
               case GPZ_GATE:
                  si=GPZ_FULL;
                  break;
               case GPZ_EMPTY:
                  if(((player_struct.panel_ref+(easified++))&3)==0)
                     si=GPZ_OPEN;
                  break;
            }
         }
         gpz_set_grid_state(gfpz,r,c,si);
      }
   }
   if(PUZZLE_DIFFICULTY==MAX_DIFFICULTY) {
      gpz_add_gate(gfpz,player_struct.panel_ref);
   }

   grid_primary_mfd=-1;
}

#define GPZ_WIN_MASK 1
void gpz_4int_update(gridFlowPuzzle *gfpz)
{
   uint init_st[4], p2;
   int nodecount, node;
   gpz_state s;

   nodecount=gfpz->gfLayout.rows * gfpz->gfLayout.cols;
   memset(init_st,0,sizeof(init_st));

   for(node=0;node<nodecount;node++) {
      s=bitarray_get(GRIDP_STATE_BITS,node,gfpz->states);
      s=gpz_uncharge_state(s);
      bitarray_set(GRIDP_STATE_BITS,node,s,init_st);
   }

   p2=objFixtures[objs[gfpz->gfLayout.our_id].specID].p2;
   p2=p2&0xFFFF;
   if(!objs[p2].active || objs[p2].obclass!=CLASS_TRAP) {
      return;
   }
   else {
      ObjTrap* other=&objTraps[objs[p2].specID];

      other->p1=init_st[3];
      other->p2=init_st[2];
      other->p3=init_st[1];
      other->p4=init_st[0];
   }

   if(gfpz->gfLayout.have_won) {
      uint p3;

      p3=objFixtures[objs[gfpz->gfLayout.our_id].specID].p3;
      p3=p3|GPZ_WIN_MASK;
      objFixtures[objs[gfpz->gfLayout.our_id].specID].p3=p3;
   }
}

// executes a move at the given node in the given grid puzzle.
// returns the state of the given node after the move.
//
gpz_state gridpanel_move(LGPoint node, gridFlowPuzzle* gfpz)
{
   short r,c,sum,dif;
   gpz_state s;

   s=gpz_get_grid_state(gfpz,node.y,node.x);
   s=gpz_uncharge_state(s);

   if(s==GPZ_OPEN || s==GPZ_CLOSED) {
      switch((gpz_control)gfpz->gfLayout.control_alg) {
         case GPZ_SIMPLE:
            gpz_toggle_state(gfpz,node.y,node.x);
            break;
         case GPZ_KING:
            for(r=node.y-1;r<=node.y+1;r++) {
               for(c=node.x-1;c<=node.x+1;c++) {
                  gpz_toggle_state(gfpz,r,c);
               }
            }
            break;
         case GPZ_QUEEN:
         case GPZ_ROOK:
            for(r=0;r<gfpz->gfLayout.rows;r++) {
               gpz_toggle_state(gfpz,r,node.x);
            }
            for(c=0;c<gfpz->gfLayout.cols;c++) {
               if(c==node.x) continue;
               gpz_toggle_state(gfpz,node.y,c);
            }
            if(gfpz->gfLayout.control_alg==GPZ_ROOK) break; // queen falls through to bishop.
         case GPZ_BISH:
            sum=node.x+node.y;
            dif=node.x-node.y;
            for(r=0;r<gfpz->gfLayout.rows;r++) {
               gpz_toggle_state(gfpz,r,sum-r);
               gpz_toggle_state(gfpz,r,dif+r);
            }
            // make sure with bishop & queen to toggle center only once.
            gpz_set_grid_state(gfpz,node.y,node.x,s);
            gpz_toggle_state(gfpz,node.y,node.x);
            break;
      }
   }

   s=gpz_get_grid_state(gfpz,node.y,node.x);
   
   return(s);
}

void mfd_gridpanel_set_winmove(bool check)
{
   gridFlowPuzzle *gfpz;

   if(check && player_struct.mfd_all_slots[MFD_INFO_SLOT]!=MFD_GRIDPANEL_FUNC) return;

   gfpz=(gridFlowPuzzle*)&player_struct.mfd_access_puzzles[0];

   if(!gfpz->gfLayout.have_won && player_struct.drug_status[CPTRIP(GENIUS_DRUG_TRIPLE)]>0) {
      bool gotone;
      LGPoint winner;
      short diff;

      if(gfpz->gfLayout.control_alg==GPZ_SIMPLE)
         diff=99;
      else
         diff=(PUZZLE_DIFFICULTY==MAX_DIFFICULTY)?2:3;

      gotone=find_winning_move(gfpz,NULL,diff,&winner, TRUE, *tmd_ticks+EPICK_TIMEOUT);

      if(gotone) {
         gfpz->gfLayout.winmove_c=winner.x;
         gfpz->gfLayout.winmove_r=winner.y;
      }
      gfpz->gfLayout.winmove_f=gotone;

      mfd_notify_func(MFD_GRIDPANEL_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
   }
}

bool mfd_gridpanel_button_handler(MFD* mfd, LGPoint bttn, uiEvent* ev, void*)
{
   gridFlowPuzzle *gfpz=(gridFlowPuzzle *)&player_struct.mfd_access_puzzles[0];
   gpz_state s;

   if(mfd->id != grid_primary_mfd)
      return TRUE;

   if ((ev->subtype&(MOUSE_LDOWN|UI_MOUSE_LDOUBLE))==0)
      return TRUE;

   if (gfpz->gfLayout.have_won)
      return TRUE;

#ifdef GRIDP_AUTO_SOLVE
   if(!gfpz->gfLayout.solve_me)
      gfpz->gfLayout.solve_me=TRUE;
#endif

   gfpz->gfLayout.winmove_f=0;
   s=gridpanel_move(bttn,gfpz);
   s=gpz_uncharge_state(s);

#ifdef GRIDP_AUTO_SOLVE
   mfd_notify_func(MFD_GRIDPANEL_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
#endif

   if(s==GPZ_OPEN || s==GPZ_CLOSED) {
      gpz_4int_update(gfpz);
      mfd_notify_func(MFD_GRIDPANEL_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
   }

   mfd_gridpanel_set_winmove(FALSE);

   return TRUE;
}

// Does every thing but set the slot..
void mfd_setup_gridpanel(ObjID id)
{
   uint p1,p2,p3,p4;
   gridFlowPuzzle *gfpz=(gridFlowPuzzle *)&player_struct.mfd_access_puzzles[0];
   p1=objFixtures[objs[id].specID].p1;
   p2=objFixtures[objs[id].specID].p2;
   p3=objFixtures[objs[id].specID].p3;
   p4=objFixtures[objs[id].specID].p4;
   
   gfpz->gfLayout.our_id=id;
   gpz_4int_init(gfpz,p1,p2,p3,p4);
   mfd_notify_func(MFD_GRIDPANEL_FUNC, MFD_INFO_SLOT, TRUE, MFD_ACTIVE, TRUE);
}


bool mfd_gridpanel_handler(MFD* m, uiEvent* ev)
{
   extern bool mfd_gridpanel_handler(MFD*,uiEvent*);
   uiCursorStack* cs;
   gridFlowPuzzle *gfpz=(gridFlowPuzzle *)&player_struct.mfd_access_puzzles[0];
   int rr=gfpz->gfLayout.rows;
   int cc=gfpz->gfLayout.cols;
   LGRect r;

   r.ul.x=(MFD_VIEW_WID-(GRIDP_BTN_WD*cc))/2+GRIDP_X_OFFSET;
   r.ul.y=(MFD_VIEW_HGT-(GRIDP_BTN_HGT*rr))/2+GRIDP_Y_OFFSET;
   r.lr.y= r.ul.y + rr*GRIDP_BTN_HGT;
   r.lr.x= r.ul.x + cc*GRIDP_BTN_WD;
   RECT_MOVE(&r,m->rect.ul);
   uiGetRegionCursorStack(&m->reg,&cs);
   if (RECT_TEST_PT(&r,ev->pos))
   {
      uiPushCursorOnce(cs,&gridCursor);   
   }
   else
   {
      uiPopCursorEvery(cs,&gridCursor);
   }
#ifdef EPICK_ON_CURSOR_TRY
   extern bool try_use_epick(ObjID panel, ObjID cursor_obj);

   if (ev->type != UI_EVENT_MOUSE || !(ev->subtype & (MOUSE_LDOWN|UI_MOUSE_LDOUBLE))) return FALSE;

   return(try_use_epick(player_struct.panel_ref, object_on_cursor));
#else
   return(FALSE);
#endif



}

#define GPZ_CHARGE_COLOR (GREEN_YELLOW_BASE+3)
#define GPZ_BASE_CHARGE_COLOR 0x3
#define GPZ_RANGE_CHARGE_COLOR 5
#define GPZ_CHARGE_DK_COLOR 0x56
#define GPZ_SOURCE_COLOR (GREEN_YELLOW_BASE+1)
#define GPZ_BOX_COLOR (0xb0+9)
#define GPZ_BACK_COLOR (0xb0+15)
#define GPZ_WIN_COLOR (RED_BASE+7)
#define GPZ_NOALG_COLOR 0xA4

void wacky_int_line(short x1,short y1,short x2,short y2)
{
   short delt;

   if(x1==x2) {
      delt=(y2<y1)?-1:1;
      for(;y1!=y2+delt;y1+=delt) {
         ss_set_pixel(GPZ_BASE_CHARGE_COLOR+(256-x1-y1)%GPZ_RANGE_CHARGE_COLOR,x1,y1);
      }
   }
   else {
      delt=(x2<x1)?-1:1;
      for(;x1!=x2+delt;x1+=delt) {
         ss_set_pixel(GPZ_BASE_CHARGE_COLOR+(256-x1-y1)%GPZ_RANGE_CHARGE_COLOR,x1,y1);
      }
   }
}

char* grid_help_string(gridFlowPuzzle *gfpz, char* buf, int siz)
{
   int l;
   char* ret=buf;

   if(gfpz->gfLayout.have_won)
      get_string(REF_STR_PanelSolved,buf,siz);
   else {
      get_string(REF_STR_GridPuzzHelp,buf,siz);
      l=strlen(buf);
      buf+=l; siz-=l;
      if (player_struct.drug_status[CPTRIP(GENIUS_DRUG_TRIPLE)] > 0)
         get_string(REF_STR_GridPuzzSide0+gfpz->gfLayout.control_alg, buf, siz);
      else
         get_string(REF_STR_GridPuzzSideMay, buf, siz);
   }
   return(ret); 
}

// initializes color look-up table to identity function
void id_clut_init(uchar* clut)
{
   int i;

   for(i=0;i<256;i++) {
      clut[i]=i;
   }
}

// I remember the feeling
// my hands in your hair

void mfd_gridpanel_expose(MFD* mfd, ubyte control)
{
   void mfd_clear_view(void);
   int rr,cc,r,c,x,y,ulx,uly,lry;
   int bcolor;
   short sc, sr, p;
   gpz_state s, nearstate;
   gridFlowPuzzle *gfpz=(gridFlowPuzzle *)&player_struct.mfd_access_puzzles[0];
   bool full = control & MFD_EXPOSE_FULL;

#ifdef SVGA_SUPPORT
   // Whatta hack!
   if (convert_use_mode)
      full = TRUE;
#endif

   rr=gfpz->gfLayout.rows;
   cc=gfpz->gfLayout.cols;

   ulx=(MFD_VIEW_WID-(GRIDP_BTN_WD*cc))/2+GRIDP_X_OFFSET;
   uly=(MFD_VIEW_HGT-(GRIDP_BTN_HGT*rr))/2+GRIDP_Y_OFFSET;


   if (control == 0)
   {
      uiCursorStack* cs;
      uiGetRegionCursorStack(&mfd->reg,&cs);
      uiPopCursorEvery(cs,&gridCursor);
   }

   if (control & MFD_EXPOSE) // Time to draw stuff
   {
      short dr, dc;
      bool win, primary, winblink=FALSE;

      if(gfpz->gfLayout.winmove_f) {
         mfd_notify_func(MFD_GRIDPANEL_FUNC, MFD_INFO_SLOT, FALSE, MFD_ACTIVE, FALSE);
      }

      if((full && grid_primary_mfd<0) || player_struct.mfd_current_slots[grid_primary_mfd]!=MFD_INFO_SLOT) {
         full = TRUE; // if we weren't full exposing before, we are now
         grid_primary_mfd = mfd->id;
      }

      primary=(mfd->id == grid_primary_mfd);

      if(primary) {

         id_clut_init(grid_help_clut);
         if(full)
            gpz_setup_buttons(gfpz);
         win=gpz_propogate_charge_n_check(gfpz);
         if ((gfpz->gfLayout.have_won==0)&& win)
         {
            accesspanel_trigger(player_struct.panel_ref);
            gfpz->gfLayout.have_won=1;
            gpz_4int_update(gfpz);
         }
      }

      mfd_clear_rects();
      gr_push_canvas(pmfd_canvas);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);

      if(!primary) {
         char buf[120];
         grid_help_string(gfpz,buf,120);
         mfd_clear_view();
         draw_help_text( buf, FALSE, gfpz );
         gr_pop_canvas();
         mfd_update_rects(mfd);
         return;
      }

#ifdef PUZZ_DIFF_NOHELP
      // set color of wires to indicate control algorithm, unless we're
      // on hardest difficulty.
      if(PUZZLE_DIFFICULTY<MAX_DIFFICULTY)
         bcolor=gpz_base_colors[gfpz->gfLayout.control_alg];
      else
         bcolor=GPZ_NOALG_COLOR;
#else
      bcolor=gpz_base_colors[gfpz->gfLayout.control_alg];
#endif
     
      if(gfpz->gfLayout.have_won) full=TRUE;
      lry=uly+rr*GRIDP_BTN_HGT;

      if(full) {
         load_res_bitmap_cursor(&gridCursor,&gridCursorbm,REF_IMG_RookSymbol+gfpz->gfLayout.control_alg,FALSE);
         mfd_clear_view();
      }

      ss_safe_set_cliprect(ulx,uly,ulx+cc*GRIDP_BTN_WD,lry+7);
      draw_res_bm(REF_IMG_CircuitBack,ulx,uly);
      ss_safe_set_cliprect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
      gr_set_fcolor(GPZ_BOX_COLOR);
      ss_box(ulx-1,uly-1,ulx+cc*GRIDP_BTN_WD+1,lry+1);
      gr_set_fcolor(bcolor);
      ss_box(ulx-2,uly-2,ulx+cc*GRIDP_BTN_WD+2,lry+2);
      mfd_add_rect(ulx-2,uly-2,ulx+cc*GRIDP_BTN_WD+2,lry+2);

      ss_box(ulx-2,lry+3,ulx+cc*GRIDP_BTN_WD+2,lry+7);
      gr_set_fcolor(ACCESSP_SCORE_COL);
      ss_rect(ulx-1,lry+4,ulx+1+gpz_doneness(gfpz)*cc*GRIDP_BTN_WD/UCHAR_MAX,lry+6);
      mfd_add_rect(ulx-2,lry+3,ulx+cc*GRIDP_BTN_WD+2,lry+7);

      if(full) {
         gpz_perimeter_to_grid(gfpz,gfpz->gfLayout.src,&sr,&sc);
         p=(gfpz->gfLayout.src>>3)&3;
         switch(p) {
            case 0: sr=-1; break;
            case 1: sr=rr; break;
            case 2: sc=-1; break;
            case 3: sc=cc; break;
         }
         x=sc*GRIDP_BTN_WD+ulx;
         y=sr*GRIDP_BTN_HGT+uly;
       
         gr_set_fcolor(GREEN_YELLOW_BASE+2);
         ss_box(x+1,y+1,x+GRIDP_BTN_WD-1,y+GRIDP_BTN_HGT-1);
         gr_set_fcolor(5);
         ss_box(x+2,y+2,x+GRIDP_BTN_WD-2,y+GRIDP_BTN_HGT-2);
         gr_set_fcolor(7);
         ss_rect(x+3,y+3,x+GRIDP_BTN_WD-3,y+GRIDP_BTN_HGT-3);

         gpz_perimeter_to_grid(gfpz,gfpz->gfLayout.dest,&sr,&sc);
         p=(gfpz->gfLayout.dest>>3)&3;
         switch(p) {
            case 0: sr=-1; break;
            case 1: sr=rr; break;
            case 2: sc=-1; break;
            case 3: sc=cc; break;
         }
         x=sc*GRIDP_BTN_WD+ulx;
         y=sr*GRIDP_BTN_HGT+uly;
         gr_set_fcolor(bcolor);
         ss_box(x+1,y+1,x+GRIDP_BTN_WD-1,y+GRIDP_BTN_HGT-1);
         if(gfpz->gfLayout.have_won)
            gr_set_fcolor(GPZ_CHARGE_COLOR);
         else
            gr_set_fcolor(GPZ_BACK_COLOR);
         ss_rect(x+2,y+2,x+GRIDP_BTN_WD-2,y+GRIDP_BTN_HGT-2);
         mfd_add_rect(0,0,MFD_VIEW_WID,MFD_VIEW_HGT);
      }

      for(r=0;r<rr;r++) {
         for(c=0;c<cc;c++) {
            x=c*GRIDP_BTN_WD+ulx;
            y=r*GRIDP_BTN_HGT+uly;
            s=gpz_get_grid_state(gfpz,r,c);
            winblink=(!gfpz->gfLayout.have_won && gfpz->gfLayout.winmove_f && gfpz->gfLayout.winmove_c==c
               && gfpz->gfLayout.winmove_r==r && (*tmd_ticks)&(1<<7));
            if(gpz_is_charged(s)) {
               short sc, sr;
               gpz_perimeter_to_grid(gfpz,gfpz->gfLayout.src,&sr,&sc);
               if(sc==c && sr==r)
                  gr_set_fcolor(GPZ_SOURCE_COLOR);
               else
                  gr_set_fcolor(GPZ_CHARGE_COLOR);
            }
            else
               gr_set_fcolor(bcolor);
            if(winblink) gr_set_fcolor(GPZ_WIN_COLOR);
            switch(s) {
               case GPZ_EMPTY:
                  break;
               case GPZ_FULL_CHARGED:
               case GPZ_FULL:
                  ss_rect(x,y,x+GRIDP_BTN_WD,y+GRIDP_BTN_HGT);
                  break;
               case GPZ_OPEN:
                  ss_int_line(x+1,y+1,x+GRIDP_BTN_WD-2,y+GRIDP_BTN_HGT-2);
                  ss_int_line(x+GRIDP_BTN_WD-2,y+1,x+1,y+GRIDP_BTN_HGT-2);
                  if(s==GPZ_OPEN) break;
               case GPZ_CLOSED:
               case GPZ_CLOSED_CHARGED:
                  gr_set_fcolor(winblink?GPZ_WIN_COLOR:bcolor);
                  for(dr=-1;dr<=1;dr++) {
                     for(dc=-1;dc<=1;dc++) {
                        if(dr!=0 && dc!=0) continue;
                        nearstate=gpz_get_grid_state(gfpz,r+dr,c+dc);
#ifndef DRAW_GRID_PLUSSES
                        if(nearstate==GPZ_EMPTY) continue;
#endif
                        ss_int_line(x+(GRIDP_BTN_WD/2),y+(GRIDP_BTN_HGT/2),
                              x+(dc+1)*(GRIDP_BTN_WD/2),y+(dr+1)*(GRIDP_BTN_HGT/2));
                     }
                  }
                  gr_set_fcolor(winblink?GPZ_WIN_COLOR:GPZ_CHARGE_COLOR);
                  for(dr=-1;dr<=1;dr++) {
                     for(dc=-1;dc<=1;dc++) {
                        if(dr!=0 && dc!=0) continue;
                        nearstate=gpz_get_grid_state(gfpz,r+dr,c+dc);
                        if(gpz_is_charged(nearstate) ||
                           (gpz_is_charged(s) && nearstate==GPZ_GATE)) {
                           wacky_int_line(x+(GRIDP_BTN_WD/2),y+(GRIDP_BTN_HGT/2),
                              x+(dc+1)*(GRIDP_BTN_WD/2),y+(dr+1)*(GRIDP_BTN_HGT/2));
                        }
                     }
                  }
                  break;
#ifdef GPZ_GATES
               case GPZ_GATE:
                  ss_box(x,y,x+GRIDP_BTN_WD,y+GRIDP_BTN_HGT);
                  gr_set_fcolor(gpz_dk_colors[gfpz->gfLayout.control_alg]);
                  ss_rect(x+1,y+1,x+GRIDP_BTN_WD-1,y+GRIDP_BTN_HGT-1);
                  break;
               case GPZ_GATE_CHARGED:
                  ss_box(x,y,x+GRIDP_BTN_WD,y+GRIDP_BTN_HGT);
                  gr_set_fcolor(GPZ_CHARGE_DK_COLOR);
                  ss_rect(x+1,y+1,x+GRIDP_BTN_WD-1,y+GRIDP_BTN_HGT-1);
                  break;
#endif
            }
         }
      }

      gr_pop_canvas();

   }
   else
   {
      ObjID obj = panel_ref_unexpose(mfd->id,MFD_GRIDPANEL_FUNC);
      if (obj != OBJ_NULL)
      {
         objs[obj].info.current_frame = 0;
      }
   }
   // Now that we've popped the canvas, we can send the 
   // updated mfd to screen
   mfd_update_rects(mfd);

#ifdef GRIDP_AUTO_SOLVE
   {
      uiEvent ev;
      LGPoint bttn;

      if(!gfpz->gfLayout.have_won) {
         bttn.x = rand()%cc;
         bttn.y = rand()%rr;

         ev.subtype=MOUSE_LDOWN;

         mfd_gridpanel_button_handler(NULL,bttn,&ev,NULL);
      }
   }
#endif

}
