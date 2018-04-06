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
#ifndef __MFDINT_H
#define __MFDINT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/mfdint.h $
 * $Revision: 1.12 $
 * $Author: tjs $
 * $Date: 1994/09/10 23:48:52 $
 *
 * $Log: mfdint.h $
 * Revision 1.12  1994/09/10  23:48:52  tjs
 * proto panel_ref_unexpose.
 * 
 * Revision 1.11  1994/09/10  04:18:45  mahk
 * panel ref unexpose
 * 
 * Revision 1.10  1994/08/26  00:49:11  mahk
 * new mfd_string_shadow regime.
 * 
 * Revision 1.9  1994/04/25  01:43:28  xemu
 * color for unavails
 * 
 * Revision 1.8  1994/04/18  04:09:35  tjs
 * Added level overlays for map.
 * 
 * Revision 1.7  1994/03/31  02:06:08  mahk
 * Changed proto.
 * 
 * Revision 1.6  1994/02/15  11:07:42  mahk
 * Fixed a stupid typo.
 * 
 * Revision 1.5  1994/02/07  23:43:58  mahk
 * Hey, maybe you'll need this to compile.
 * 
 * Revision 1.4  1994/02/07  16:12:29  mahk
 * Transparent mfd stuff.
 * 
 * Revision 1.3  1993/12/15  13:52:39  mahk
 * Added mfd_string wrapping bool
 * 
 * Revision 1.2  1993/12/08  10:50:17  mahk
 * Added some useful stuff for mfdfunc.c
 * 
 * Revision 1.1  1993/12/08  10:32:09  mahk
 * Initial revision
 * 
 *
 */

// Includes
#include "player.h"

// ------
// MACROS
// ------

#define visible_mask(m) ((m == MFD_LEFT) ? FULL_L_MFD_MASK : FULL_R_MFD_MASK)


// ---------
// Constants
// ---------


// Button colors
#define MFD_BTTN_EMPTY   0xcb
#define MFD_BTTN_ACTIVE  0xa2
#define MFD_BTTN_FLASH   0x35
#define MFD_BTTN_SELECT  0x77
#define MFD_BTTN_UNAVAIL 0xdd

// Duration of a button flash
#define MFD_BTTN_FLASH_TIME 256

// From the MFD art resource
#define MFD_ART_HUMAN 0       
#define MFD_ART_TRIOP 1
#define MFD_ART_DRUGS 2
#define MFD_ART_CHIPS 3
#define MFD_ART_GRNDE 4
#define MFD_ART_WEAPN 5
#define MFD_ART_STATN 6
#define MFD_ART_LVL(x) (MFD_ART_STATN+1+(x))

#define mfd_fdata                player_struct.mfd_func_data
#define MFD_FONT                 RES_tinyTechFont


extern ObjID panel_ref_unexpose(int mfd_id, int func);

// ----------
// Structures
// ----------

// An MFD points to an arbitrary number of slots, which can 
// be shared by other MFD's.  A slot points to a expose/handler
// function pair, which can be shared by multiple slots.

#define NUM_MFD_HANDLERS 4

typedef bool (*MFD_handlerProc) (struct _MFD* mfd, uiEvent* ev, struct _mfd_handler *mh);
typedef bool (*MFD_SimpHandler) (struct _MFD* mfd, uiEvent* ev);

typedef struct _mfd_handler
{
   LGRect r; // Sub-rect of MFD I'm handling, in relative coordinates.
   MFD_handlerProc  proc;  // Proc we call when we get in this
                           // rect
   void* data;  // proc-specific state.
} MFDhandler;


// A button's state depends on the state of its associated slot for its
// MFD.  But: a button also keeps track of which is selected, but this
// is done through the code and not through the structure.

typedef struct {                             // Button panel structure
   LGRect       rect;
   LGRegion     reg;
   LGRegion     reg2;
} MFD_bPanel;          

typedef struct _MFD {
  ubyte       id;     
  MFD_bPanel  bttn;                          // The button panel
  LGRect        rect;
  LGRegion      reg;
  LGRegion      reg2;
} MFD; 


typedef struct _mfd_func {
//   ubyte      id;
   void       (*expose)(MFD *mfd, ubyte control);
   MFD_SimpHandler simp; // Retained for compatibility.  
   errtype    (*init)(struct _mfd_func*);
   uchar      priority;  // one is highest, zero is infinitely low
   // The following stuff is most likely to want to be zero.  so nyah
   ubyte      flags;                         // Static func-specific info
   long       last;                          // Timestamp for incremental
   int        handler_count;
   MFDhandler handlers[NUM_MFD_HANDLERS];
} MFD_Func;
      

extern void init_mfd_funcs();
extern bool mfd_view_callback_full(uiEvent *e, LGRegion *r, void *udata);
extern bool mfd_view_callback(uiEvent *e, LGRegion *r, void *udata);
extern bool mfd_button_callback(uiEvent *e, LGRegion *r, void *udata);
extern bool mfd_button_callback_kb(short keycode, ulong context, void *data);
extern bool mfd_update_current_slot(ubyte mfd_id,ubyte status,ubyte num_steps);
extern void mfd_init_funcs();
extern void mfd_set_cliprect(LGRect *r);
extern void set_mfd_func(int fnum, void *e, void *h, void *initf, ubyte flags);
extern LGPoint mfd_draw_string(char *s, short x, short y, long c, bool DrawString);
extern LGPoint mfd_draw_font_string(char *s, short x, short y, long c, int font, bool DrawString);
extern LGPoint mfd_full_draw_string(char *s, short x, short y, long c, int font, bool DrawString, bool transp);
extern void set_slot_to_func(ubyte snum, ubyte fnum, MFD_Status stat);
extern void mfd_draw_bitmap(grs_bitmap* bmp, short x, short y);
extern void mfd_partial_clear(LGRect *r);
extern void init_newmfd_button_cursors();
extern void mfd_update_display(MFD *m, short x0, short y0, short x1, short y1);
extern void mfd_clear_rects(void);
extern errtype mfd_add_rect(short x, short y, short x1, short y1);
extern void mfd_update_rects(MFD* m);
extern ubyte mfd_get_func(ubyte mfd_id, ubyte s);

extern char* get_free_frame_buffer_bits(int);
#define MFD_EXTRACT_BUF (get_free_frame_buffer_bits(-1))

// -------
// GLOBALS
// -------
extern MFD_Func mfd_funcs[MFD_NUM_FUNCS];
extern bool mfd_string_wrap;
extern ubyte mfd_string_shadow;

#define MFD_SHADOW_NEVER      0
#define MFD_SHADOW_ALWAYS     1
#define MFD_SHADOW_FULLSCREEN 2


#ifdef __NEWMFD_SRC
grs_bitmap mfd_background;
grs_canvas *pmfd_canvas;
#endif
#ifndef __NEWMFD_SRC
extern grs_bitmap mfd_background;
extern grs_canvas *pmfd_canvas;
#endif
#ifndef __NEWMFD_SRC
extern bool Flash;
#endif


// ------
// Macros
// ------

#define macro_region_create(parent,child,rect) \
   region_create(parent,child,rect,0,0,REG_USER_CONTROLLED,NULL,NULL,NULL,NULL)

#endif // __MFDINT_H
