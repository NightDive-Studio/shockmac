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
// flags
#define FR_HACKCAM_SHFT   31
#define FR_HACKCAM_MASK   (0x1<<FR_HACKCAM_SHFT)
#define FR_PALETTE_SHFT   28           // which palette to output through
#define FR_PALETTE_MASK   (0x7<<FR_PALETTE_SHFT)
#define FR_WINDOWD_SHFT   24           // what window dressing to use
#define FR_WINDOWD_MASK   (0xf<<FR_WINDOWD_SHFT)
#define FR_CURFREE_SHFT   20
#define FR_CURFREE_MASK   (0xf<<FR_CURFREE_SHFT)
#define FR_NOTRANS_SHFT   19           // 
#define FR_NOTRANS_MASK   (0x1<<FR_NOTRANS_SHFT)
#define FR_OVERLAY_SHFT   16           // what is being overlayed/ie. hand art or pings
#define FR_OVERLAY_MASK   (0x7<<FR_OVERLAY_SHFT)
#define FR_SOLIDFR_SHFT   13           // are we just a solid this frame
#define FR_SOLIDFR_MASK   (0x7<<FR_SOLIDFR_SHFT)
#define FR_DOHFLIP_SHFT   12
#define FR_DOHFLIP_MASK   (0x1<<FR_DOHFLIP_SHFT)
#define FR_OWNBITS_SHFT   11           // your canvas and all came from you
#define FR_OWNBITS_MASK   (0x1<<FR_OWNBITS_SHFT)
#define FR_TITLEBR_SHFT   9            // should we be showing a title
#define FR_TITLEBR_MASK   (0x3<<FR_TITLEBR_SHFT)
#define FR_SHOWALL_SHFT   8
#define FR_SHOWALL_MASK   (0x1<<FR_SHOWALL_SHFT)
#define FR_DOUBLEB_SHFT   7            // are we double buffering
#define FR_DOUBLEB_MASK   (0x1<<FR_DOUBLEB_SHFT)
#define FR_CURVIEW_SHFT   5
#define FR_CURVIEW_MASK   (0x3<<FR_CURVIEW_SHFT)
#define FR_PICKUPM_SHFT   4
#define FR_PICKUPM_MASK   (0x1<<FR_PICKUPM_SHFT)
#define FR_NORENDR_SHFT   3
#define FR_NORENDR_MASK   (0x1<<FR_NORENDR_SHFT)
#define FR_SFX_SHFT       0
#define FR_SFX_MASK       (0x7<<FR_SFX_SHFT)

// yet another way to have multiple views
#define FR_CURVIEW_STRT   (0<<FR_CURVIEW_SHFT)
#define FR_CURVIEW_LEFT   (1<<FR_CURVIEW_SHFT)
#define FR_CURVIEW_BACK   (2<<FR_CURVIEW_SHFT)
#define FR_CURVIEW_RGHT   (3<<FR_CURVIEW_SHFT)

// Hey, some solid stuff
#define FR_SOLIDFR_NORMAL  (0 << FR_SOLIDFR_SHFT)
#define FR_SOLIDFR_STATIC  (1 << FR_SOLIDFR_SHFT)
#define FR_SOLIDFR_SLDCLR  (2 << FR_SOLIDFR_SHFT)
#define FR_SOLIDFR_SLDKEEP (3 << FR_SOLIDFR_SHFT)

#ifndef __GAMEREND_SRC
extern uchar fr_solidfr_color;
#else
uchar fr_solidfr_color;
#endif

// Cool warping effects on the screen, and other draw hacks
#define FR_SFX_NONE        (0 << FR_SFX_SHFT)
#define FR_SFX_VHOLD       (1 << FR_SFX_SHFT)
#define FR_SFX_HHOLD       (2 << FR_SFX_SHFT)
#define FR_SFX_STATIC      (3 << FR_SFX_SHFT)
#define FR_SFX_SHAKE       (4 << FR_SFX_SHFT)
#define FR_SFX_SHIELD      (5 << FR_SFX_SHFT)
#define FR_SFX_TELEPORT    (6 << FR_SFX_SHFT)

// Overlays -- currently this is only SHODAN
#define FR_OVERLAY_NONE    (0 << FR_OVERLAY_SHFT)
#define FR_OVERLAY_SHODAN  (1 << FR_OVERLAY_SHFT)

// Palette setup
#define FR_PALETTE_BW      (1 << FR_PALETTE_SHFT)
#define FR_PALETTE_GREEN   (2 << FR_PALETTE_SHFT)
#define FR_PALETTE_ORANGE  (3 << FR_PALETTE_SHFT)

#define FR_PALETTE_IR      (FR_PALETTE_BW|FR_MONOCHR_MASK)
#define FR_PALETTE_LOWTECH (FR_PALETTE_GREEN|FR_MONOCHR_MASK)

// Hack cameras
#define FR_HACKCAM_FLAG    (1 << FR_HACKCAM_SHFT)

#define _fr_get_pal_idx(f) ((f&FR_PALETTE_MASK)>>FR_PALETTE_SHFT)

#define FR_MONOPAL_MASK    (FR_PALETTE_MASK|FR_MONOCHR_MASK)

// it was unintentional, when i spit in your beer
// im overinfluenced, by movies
#define FR_DBG_DBG_ALL     (0xffffffff)

#define FR_DBG_MONO_MAP    (1 <<  1)
#define FR_DBG_MONO_LIST   (1 <<  2)
#define FR_DBG_NO_MATH     (1 <<  3)
#define FR_DBG_NO_REND     (1 <<  4)
#define FR_DBG_NO_2D       (1 <<  5)
#define FR_DBG_PT_SPEW     (1 <<  6)
#define FR_DBG_SHOW_BASE   (1 <<  7)
#define FR_DBG_SANITY      (1 <<  8)
#define FR_DBG_NO_RTF      (1 <<  9)
#define FR_DBG_CURSOR      (1 << 10)
#define FR_DBG_ANAL_CHK    (1 << 11)
#define FR_DBG_ALTCAM      (1 << 12)
#define FR_DBG_VECSPEW     (1 << 13)
#define FR_DBG_VECTRACK    (1 << 14)
#define FR_DBG_POLY_MODE   (1 << 15)
#define FR_DBG_NO_SUB_CLIP (1 << 16)
#define FR_DBG_STATS       (1 << 17)
#define FR_DBG_SPAN_PARSE  (1 << 18)
#define FR_DBG_NO_CONE     (1 << 19)
#define FR_DBG_NO_TILE     (1 << 20)
#define FR_DBG_OBJ_TALK    (1 << 21)
#define FR_DBG_LOC_TMAPS   (1 << 22)
#define FR_DBG_SHOW_PICKUP (1 << 23)
#define FR_DBG_LIST_TILES  (1 << 24)
#define FR_DBG_NEW_PTS     (1 << 25)

//#define FR_DBG_STATIC_FLG  (FR_DBG_SANITY|FR_DBG_SHOW_PICKUP|FR_DBG_VECSPEW|FR_DBG_VECTRACK)

#ifndef FR_DBG_STATIC_FLG
#define FR_DBG_STATIC_FLG  (0)
#endif


// actual behavior controls, cause heck, why not
#define CLEAR_AS_WE_GO

