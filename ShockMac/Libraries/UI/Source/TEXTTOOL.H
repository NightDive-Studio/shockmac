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
#ifndef __TEXTTOOL_H
#define __TEXTTOOL_H

// defines 
#include "lg_types.h"
#include "rect.h"

// These are all stolen from kbcook.h, for purposes of independence, emancipation, and the New World Order
// **HOWEVER** make sure they stay up to date or perhaps you will be hosed!

#define TEXTTOOL_CNV_CTRL     (1<<(1+8)) // KB_FLAG_CTRL can be set
#define TEXTTOOL_CNV_ALT      (1<<(2+8)) // KB_FLAG_SHIFT can be set
#define TEXTTOOL_CNV_SPECIAL  (1<<(3+8)) // KB_FLAG_SPECIAL is set
#define TEXTTOOL_CNV_SHIFT    (1<<(4+8)) // KB_FLAG_SHIFT can be set
#define TEXTTOOL_CNV_2ND      (1<<(5+8)) // KB_FLAG_2ND is set
#define TEXTTOOL_CNV_NUMLOCK  (1<<(6+8)) // affected by numlock
#define TEXTTOOL_CNV_CAPSLOCK (1<<(7+8)) // affected by capslock

#define TEXTTOOL_CNV_CAPSLOCK_SHF (7+8)


#define TEXTTOOL_KB_FLAG_DOWN    (1<<(0+8))
#define TEXTTOOL_KB_FLAG_CTRL    TEXTTOOL_CNV_CTRL
#define TEXTTOOL_KB_FLAG_ALT     TEXTTOOL_CNV_ALT
#define TEXTTOOL_KB_FLAG_SPECIAL TEXTTOOL_CNV_SPECIAL
#define TEXTTOOL_KB_FLAG_SHIFT   TEXTTOOL_CNV_SHIFT
#define TEXTTOOL_KB_FLAG_2ND     TEXTTOOL_CNV_2ND


#define TEXTTOOL_KB_DOWN_SHF     8
#define TEXTTOOL_KB_CTRL_SHF     9 
#define TEXTTOOL_KB_ALT_SHF     10
#define TEXTTOOL_KB_SPECIAL_SHF 11 
#define TEXTTOOL_KB_SHIFT_SHF   12
#define TEXTTOOL_KB_2ND_SHF     13 

// okay, real text tool defines now...
#define TTF_REPLACE    0x01    // replace any existing text (overwrite)
#define TTF_INSFRONT   0x02    // insert text at front of line
#define TTF_INSEND     0x04    // at end of line
#define TTF_INSWHOLE   0x08    // insert the text as a whole new line

#define TTFI_FIXED     0x01    // For TextToolFontInfo
#define TTFI_SPACE     0x02  
#define TTFI_PROP      0x04
#define TTFI_FREE      0x80    // should tt_toast free the TTFI

#define TTS_WRAP       0x0001  // For EditState Mode
#define TTS_OVER       0x0002  // overstrike/insert mode toggle
#define TTS_CGROW      0x0004
#define TTS_COLUMN     0x0008  // column vs. wrap blocks
#define TTS_COMMAND    0x0010  // waiting for a command character
#define TTS_FULL       0x1000  // full screen editing
#define TTS_SINGLE     0x2000  // One line editing
#define TTS_LINES      0x4000  // a set of lines, final resting one is return
#define TTS_READONLY   0x8000  // is it, or isnt it

#define TTS_MODE       0xF000

// internal defines
#define TTC_REBUILD    (-2)    // For TextToolCheats
#define TTC_NOTHING    (-1)

#define TTC_FLG_RET    0x01    // return at EOL

#define TTL_INIT       0x02    // Initial size of a line
#define TTL_BASE       0x10    // Amount to add w/realloc

#define TTPC_
#define TTPC_SAME      (-3)    // stay in the same place
#define TTPC_BOL       (0)     // beginning of line
#define TTPC_EOL       (-1)    // end of line, as you probably could tell

#define TTWL_FIRST     (1)
#define TTWL_LAST      (2)
#define TTWL_CUR       (3)

#define TTCHG_NOCHANGE 0x00
#define TTCHG_CURSOR   0x01
#define TTCHG_REDRAW   0x02

// the event masks
#define _FULL_M        TTS_FULL
#define _SNGL_M        TTS_SINGLE
#define _LINE_M        TTS_LINES
#define _RO_M          TTS_READONLY
#define _ALL_M         (_SNGL_M|_FULL_M|_LINE_M|_RO_M)
#define _NOSNGL_M      (_ALL_M&(~_SNGL_M))
#define _NORO_M        (_ALL_M&(~_RO_M))

// the event codes
#define TTEV_NULL      (0x0000)
#define TTEV_F_CHAR    (0x01|_ALL_M)
#define TTEV_B_CHAR    (0x02|_ALL_M)
#define TTEV_F_WORD    (0x03|_ALL_M)
#define TTEV_B_WORD    (0x04|_ALL_M)
#define TTEV_F_LINE    (0x05|_NOSNGL_M)
#define TTEV_B_LINE    (0x06|_NOSNGL_M)
#define TTEV_HOME      (0x07|_ALL_M)
#define TTEV_END       (0x08|_ALL_M)
#define TTEV_CENTER    (0x09|_NOSNGL_M)
#define TTEV_REPEAT    (0x0A|_ALL_M)
#define TTEV_OVER      (0x0B|_ALL_M)
#define TTEV_INS       (0x0C|_ALL_M)
#define TTEV_DEL       (0x0D|_NORO_M)
#define TTEV_PGDN      (0x0E|_NOSNGL_M)
#define TTEV_PGUP      (0x0F|_NOSNGL_M)
#define TTEV_CUT       (0x10|_NORO_M)
#define TTEV_YANK      (0x11|_NORO_M)
#define TTEV_RET       (0x12|_ALL_M)
#define TTEV_COMMAND   (0x13|_ALL_M)
#define TTEV_TOP       (0x14|_NOSNGL_M)
#define TTEV_BACKSP    (0x15|_NORO_M)
#define TTEV_MACRO_ST  (0x16|_ALL_M)
#define TTEV_MACRO_END (0x17|_ALL_M)
#define TTEV_MACRO_PLY (0x18|_ALL_M)
#define TTEV_2_HOME    (0x19|_ALL_M)
#define TTEV_2_END     (0x1A|_ALL_M)
#define TTEV_3_HOME    (0x1B|_ALL_M)
#define TTEV_3_END     (0x1C|_ALL_M)
#define TTEV_EOL       (0x1D|_ALL_M)
#define TTEV_BOL       (0x1E|_ALL_M)
#define TTEV_KILL      (0x1F|_NORO_M)
#define TTEV_WRAP      (0x20|_FULL_M)

//#define TTEV_          

//#define TTEV_UNDO      
//#define TTEV_REDO      

// structs
typedef struct {
   int (*s_wid)(char *s), (*s_draw)(char *s, int x, int y), (*c_wid)(char c), (*c_draw)(char c, int x, int y);
   int (*s_clr)(char *s, int x, int y), (*l_clr)(int len, int x, int y), (*c_clr)(char c, int x, int y);
   int (*cursor)(int x, int y);
   int height, base_w;
   uchar type;
} TTFontInfo;

typedef struct {
   long  left_m, right_m;      // current margins
   long  max_w;                // maximum width for a line, or 0 for none
   ulong mode;                 // mode of editing
   int   r_cnt;                // current repeat count
   int   last_ev;              // last event (for multi-event stuff, like triple home)
} TTState;

typedef struct {
   long wid, stl;              // malloced width of line, length of string
   long chr[2];                // l/r edge of screen char off 
   long pix[2];                // actual pix width there
   uchar flg;                  // various text flags
} TTCheats;

typedef union {
   int c[2];                   // coord x,y
   struct {
	   int x, y;
   } pt;
} TTPoint;

typedef struct {
   TTPoint crn;
   int w, h;
} TTRect;

typedef struct {
   long max_w, max_h;          // max w/h in characters
   long cur_w, cur_h;          // cursor w/h in characters
   long disp_x, disp_y;        // x(abspix)/y(lines) of display top
   int  disp_rows;             // number of rows we really can fit on the screen
   int  cur_x, cur_y;          // x/y in scr pixels of cursor
   TTRect scr_loc;             // screen offset + size of the texttool
   void (*display_func)(void *, LGRect *);
   void *output_data;          // some appropriate output data
   char **lines;               // the lines of actual text
   TTCheats *line_info;        // offsets for current lines
   TTFontInfo *lfont;          // the font being used
   TTState es;
} TextTool;

// prototypes
TextTool *tt_full_build(TTRect *pos, TTState *es, TTFontInfo *ttf, void *output_data, char *keymap,
   void (*d_func)(void *, LGRect *));
#define tt_easy_build() tt_full_build(NULL,NULL,NULL,NULL,NULL)
bool tt_toast(TextTool *old_tt);
bool tt_set(TextTool *def_tt);
long tt_parse_char(TextTool *tt, ushort key_code);
long tt_parse_string(TextTool *tt, char *st);
char *tt_get(TextTool *tt, long line_num);
void tt_dump(TextTool *tt);
void tt_display_all(TextTool *tt, LGRect *r);
void tt_show_all(TextTool *tt);
void tt_move(TextTool *tt, int xoff, int yoff);
void tt_fill_line(TextTool *tt, int how, long line_num, char *s);
#endif
