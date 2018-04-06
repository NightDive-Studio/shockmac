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
#ifndef __AMAP_H
#define __AMAP_H

// header for the real infernal automap

// defines
#define AMAP_PURE_MODE 0x0000
#define AMAP_INT_WALLS 0x0001
#define AMAP_SHOW_CRIT 0x0002
#define AMAP_SHOW_ROB  0x0004
#define AMAP_SHOW_HAZ  0x0008
#define AMAP_SHOW_FLR  0x0010
#define AMAP_SHOW_MSG  0x0020
#define AMAP_SHOW_HGT  0x0040
#define AMAP_TRACK_OBJ 0x0080
#define AMAP_SHOW_SEC  0x0100
#define AMAP_FULL_MSG  0x0200
#define AMAP_SHOW_ALL  0x0400
#define AMAP_SHOW_SENS 0x0800

#define AMAP_AVAIL_ALWAYS (AMAP_SHOW_SENS|AMAP_SHOW_FLR|AMAP_FULL_MSG)

#define AMAP_SET       1
#define AMAP_UNSET     0
#define AMAP_TOGGLE   -1

#define AMAP_PAN_N     1
#define AMAP_PAN_E     2
#define AMAP_PAN_S     3
#define AMAP_PAN_W     4
#define AMAP_DEF_DST   0x40000

#define AMAP_MAX_ZOOM  6
#define AMAP_MIN_ZOOM  1

#define AMAP_OFF_MAP   0
#define AMAP_HAVE_NOTE 1
#define AMAP_NO_NOTE   2

#define AMAP_NOTE_HACK_PTR ((void *)0xffffffff)

// really should live in the player structure....
typedef struct {
   bool   init;
   uchar  zoom;
   int    xf, yf;
   ushort lw, lh;
   ushort obj_to_follow, sensor_obj;
   ushort note_obj;
   ushort flags;
   ushort avail_flags;
   uchar  version_id;
   ushort sensor_rad;   // in obj coords
} curAMap;

// prototypes
bool amap_kb_callback(curAMap *amptr, int code);
void amap_draw(curAMap *amptr, int expose);
void amap_version_set(int id, int new_ver);
void automap_init(int version, int id);
void amap_invalidate(int id);
void *amap_loc_note_check(curAMap *amptr, int *x, int *y, int *to_do); // note the void is really a MapElem
bool amap_flags(curAMap *amptr, int flags, int set);                 // set -1 to toggle
bool amap_zoom(curAMap *amptr, bool set, int zoom_delta);
void amap_pan(curAMap *amptr, int dir, int* dist);
bool amap_get_note(curAMap *amptr, char *buf);
void amap_settings_copy(curAMap* from, curAMap* to);

// this is a mess
// it modifies x and y to be map location of click
// returns null if off map, (void*)objid if found, (void*)mapelemptr if empty
void *amap_deal_with_map_click(curAMap *amptr, int *x, int *y);

// strings
void amap_str_init(void);
char *amap_str_next(void);
void amap_str_grab(char *str);
int amap_str_deref(char *str);
char *amap_str_reref(int offs);
void amap_str_delete(char *toast_str);
void amap_str_startup(int magic_num);

#define MFD_FULLSCR_MAP 2
#define NUM_O_AMAP      MFD_FULLSCR_MAP+1

// globals
// for now

#define oAMap(mid) (&(level_gamedata.auto_maps[mid]))
//#define oAMap(mid) (auto_maps[mid])

#define amap_reset() \
do { int i; for(i=0;i<NUM_O_AMAP;i++) amap_invalidate(i); } while(0)

#define amap_note_value(objid)  (objTraps[objs[objid].specID].p4)
#define amap_note_string(objid) (amap_str_reref(amap_note_value(objid)))

#define AMAP_STRING_SIZE   2048

#endif

