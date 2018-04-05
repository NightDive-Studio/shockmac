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
#include "objects.h"
#include "player.h"
#include "diffq.h"
#include "faketime.h"

char mfd_setup_accesspanel(uchar special, ObjID id);
char mfd_type_accesspanel(ObjID id);
uchar mfd_solve_accesspanel(ObjID id);

#define EPICK_SOLVED 0
#define EPICK_FAILED 1
#define EPICK_PRESOL 2

// 1.5 seconds
#define EPICK_TIMEOUT (3*CIT_CYCLE)/2

// ------------------
// ACCESS PANEL MFD
// ------------------
// access panel internals
#define MAX_P_WIRES 6
#define MAX_P_PINS  16

#define ACCESSP_BTN_ROW    6
#define ACCESSP_BTN_COL    2
#define ACCESSP_BTN_WD     8
#define ACCESSP_BTN_HGT    7
#define ACCESSP_BTN_X      9
#define ACCESSP_BTN_Y     15
#define ACCESSP_FULL_WD   56
#define ACCESSP_FULL_HGT  42

#define ACCESSP_SCORE_X    4
#define ACCESSP_SCORE_YT   3
#define ACCESSP_SCORE_W   64
#define ACCESSP_SCORE_YB  13
#define ACCESSP_SCORE_SHF  2
#define ACCESSP_SCORE_OFF  1

#define ACCESSP_SCORE_COL  0x5B
#define ACCESSP_SCORE_BOR  0x58
#define ACCESSP_PIN_COL    0x43
#define ACCESSP_CHIP_COL   0x40
#define ACCESSP_HOT_COL    0x40
#define ACCESSP_TARG_COL   0x35

#define get_delta(wpv) (wpv.lpos-wpv.rpos)

#define PUZZLE_DIFFICULTY QUESTVAR_GET(PUZZLE_DIFF_QVAR)
#define MAX_DIFFICULTY 3

typedef struct {
   uchar lpos;
   uchar rpos;
} wirePos;

typedef struct {
   wirePos cur;
   wirePos targ;
} wirePTrg;

typedef struct {
   uchar    wirecnt;
   uchar    pincnt;
   uchar    scale;
   uchar    score;
   wirePTrg wires[MAX_P_WIRES];     // internals for actual puzzle, 16 bytes

   uchar    left_tap, right_tap;

   uchar    last_score;             // stuff for interface, mfd layer, so on
   uchar    tscore;                 // target score
   uchar    wires_moved;
   uchar    wire_in_motion;
   uchar    wim_tick;
   uchar    wim_shown;
   uchar    scorealg;
   uchar    special;
   ObjID    our_id;
   uchar    have_won;
   uchar    pad[3];
} wirePosPuzzle;

// WP_SCALE_SHF is the number of bits of fractional precision to be used in
// the scale field of a wirePosPuzzle (which is a uchar).  Thus, if scale
// is to be a uchar, (256<<WP_SCALE_SHF)/P_CORRECT must be under 256.

#define WP_SCALE_SHF 2

#define P_CORRECT  6
#define P_DELTA_OK 3
#define P_POS_OK   2

#define D_DELTA_OK 3
#define D_POS_OK   1

#define NO_WIRE_IN_MOTION 0xff
#define BTN_MASK 0xf
#define LR_MASK  0x10





// MFD GRID ACCESS PUZZLE

#define GRIDP_BTN_WD 7
#define GRIDP_BTN_HGT 7
#define GRIDP_BTN_COL 7
#define GRIDP_BTN_ROW 6
#define GRIDP_BTN_X 8
#define GRIDP_BTN_Y 8
#define GRIDP_FULL_WD 56
#define GRIDP_FULL_HGT 42

#define GRIDP_STATE_BITS 3
#define ROUNDUP_INTS(bits) (((bits)+sizeof(uint)*8-1)/(sizeof(uint)*8))
#define GRIDP_STATE_INTS ROUNDUP_INTS(GRIDP_BTN_ROW*GRIDP_BTN_COL*GRIDP_STATE_BITS)

#define GPZ_GATES
#define SHOW_DONENESS
// #define GRIDP_AUTO_SOLVE

typedef struct {
   ObjID our_id;                        // 2 bytes
   uchar rows;			//:3;                        //  \_ 1 byte
   uchar cols;			//:3;                        //  /  between us.
   uchar control_alg;                 
   uchar src;
   uchar dest;
   uchar have_won;
   uchar winmove_r;	//:3;                   //  \_ 1 byte
   uchar winmove_c;	//:3;                   //  /
   uchar winmove_f;	//:1;                   // /
#ifdef GRIDP_AUTO_SOLVE
   uchar solve_me;
#endif
} gridFlowPuzzleLayout;

#define GRIDP_PADSIZE (sizeof(player_struct.mfd_access_puzzles)-sizeof(gridFlowPuzzleLayout)-GRIDP_STATE_INTS*sizeof(uint))

typedef struct {
   gridFlowPuzzleLayout gfLayout;
   uint states[ GRIDP_STATE_INTS ];
   uchar pad[ GRIDP_PADSIZE ];
} gridFlowPuzzle;

extern Region* mfd_regions[NUM_MFDS];

typedef enum {
   GPZ_EMPTY=0, GPZ_OPEN,
   GPZ_CLOSED, GPZ_CLOSED_CHARGED,
   GPZ_FULL, GPZ_FULL_CHARGED,
   GPZ_GATE, GPZ_GATE_CHARGED
} gpz_state;

typedef enum {
   GPZ_ROOK, GPZ_KING, GPZ_BISH, GPZ_QUEEN, GPZ_SIMPLE
} gpz_control;

#define GRIDP_X_OFFSET 0
#define GRIDP_Y_OFFSET 3

gpz_state gpz_get_grid_state(gridFlowPuzzle *gfpz, short row, short col);
void gpz_4int_init(gridFlowPuzzle *gfpz,uint p1,uint p2,uint p3,uint p4);
void gpz_4int_update(gridFlowPuzzle *gfpz);

#define last_access_help_string (*((int*)&player_struct.mfd_func_data[MFD_ACCESSPANEL_FUNC][0]))
#define access_primary_mfd (*(((char*)&player_struct.mfd_func_data[MFD_ACCESSPANEL_FUNC][0]+sizeof(int))))
#define grid_primary_mfd (*(((char*)&player_struct.mfd_func_data[MFD_GRIDPANEL_FUNC][0])))
