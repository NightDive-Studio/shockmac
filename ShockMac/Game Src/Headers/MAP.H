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
#ifndef __MAP_H
#define __MAP_H

/*
 * 
 *
 * $Source: r:/prj/cit/src/inc/RCS/map.h $
 * $Revision: 1.41 $
 * $Author: minman $
 * $Date: 1994/08/28 02:38:51 $
 *  
 * $Log: map.h $
 * Revision 1.41  1994/08/28  02:38:51  minman
 * hey / we have a correct macro for changing fixang to objang
 * 
 * Revision 1.40  1994/07/21  16:28:25  dc
 * im a moron, forgot to add reshifting to flags and all last night
 * now ALL MUST PAY
 * AND PAY
 * BLOOD
 * HAH HAH HAH HAH GURGLE
 * 
 * Revision 1.39  1994/07/21  01:51:20  dc
 * wanted to change some #defines
 * 
 * Revision 1.38  1994/01/26  23:34:08  mahk
 * Changed hazard bit stuff.
 * 
 * Revision 1.37  1994/01/22  18:56:59  dc
 * new map regieme, take 4
 * 
 * Revision 1.36  1994/01/02  17:17:09  dc
 * indoor terrain renderer
 * 
 * Revision 1.35  1993/10/18  03:35:49  dc
 * adding dumb-o @ifndef __SPEW
 * 
 * Revision 1.34  1993/09/08  00:59:21  xemu
 * increase map number...EIT!
 * sorry, had to do it for loved_textures
 * 
 * Revision 1.33  1993/09/06  05:57:19  mahk
 * Map has been packed.
 * 
 * Revision 1.31  1993/09/05  21:01:52  dc
 * added the great "pile of translations"
 * soon tilenames will get taken out, but not yet
 * 
 */
#include "schedtyp.h"

#define MAP_TYPES 32
#define MAP_NUM_TMAP1 128
#define MAP_NUM_TMAP2 32
#define MAP_NUM_TMAP3 16
#define MAP_HEIGHTS 32
#define MAP_PARAMS 16

#define TMAP_FLR  0
#define TMAP_WALL 1
#define TMAP_CEIL 2

#define HGT_CEIL        0
#define HGT_FLOOR       1

#define LITE_FLOOR      0
#define LITE_CEIL       1

#define MAP_SCHEDULE_GAMETIME 0
#define NUM_MAP_SCHEDULES     1

#define MAP_VERSION_NUMBER ((int)11)
// so we auto convert from it
#define OLD_MAP
#define OLD_MAP_VERSION_NUMBER ((int)10)

// probably should kick it up to 16 bytes, expand one of the bitfields to three uchars instead
typedef struct _map_element {
//   struct _bitfields
//   {
//      ushort tiletype:6;
//      ushort flr_height:5;
//      ushort ceil_height:5;
//   };
   uchar tiletype;         // 2 free bits
   uchar flr_rotnhgt;      // 1 free bit
   uchar ceil_rotnhgt;     // 1 free bit
   uchar param;
   short objRef;
//   struct _lighting
//   {
//      uchar floor:4;
//      uchar ceil:4;
//   } templight;
   ushort tmap_ccolor;
//   union _space
//      {
//         struct _tmaps
//         {
//            ushort floor:5;
//            ushort ceil:5;
//            ushort wall:6;
//         } real;
//         struct _cybcolors
//         {
//            uchar floor;
//            uchar ceil;
//         } cyber;
//      } space;
   uchar flag4;			// KLC swapped around
   uchar flag3;
   uchar flag2;
   uchar flag1;      // rename these, perhaps
//   ulong flags;
   uchar sub_clip;
   uchar clearsolid;
   uchar flick_qclip;
   uchar templight;
//   struct _render_info {
//      uchar sub_clip;
//      uchar clear;
//      uchar rotflr:2;
//      uchar rotceil:2;
//      uchar flicker:4;
//   } rinfo;
} MapElem;


#ifdef OLD_MAP
typedef struct _omap_element {
   struct _bitfields
   {
      ushort tiletype:6;
      ushort flr_height:5;
      ushort ceil_height:5;
   };
   uchar param;
   struct _lighting
   {
      uchar floor:4;
      uchar ceil:4;
   } templight;
   union _space
      {
         struct _tmaps
         {
            ushort floor:5;
            ushort ceil:5;
            ushort wall:6;
         } real;
         struct _cybcolors
         {
            uchar floor;
            uchar ceil;
         } cyber;
      } space;
   ulong flags;
   short objRef;
   struct _render_info {
      uchar sub_clip;
      uchar clear;
      uchar rotflr:2;
      uchar rotceil:2;
      uchar flicker:4;
   } rinfo;
} oMapElem;

typedef struct {
   int x_size, y_size;
   int x_shft, y_shft, z_shft;
   oMapElem *map;
   bool cyber;
   int x_scale, y_scale, z_scale;
   Schedule sched[NUM_MAP_SCHEDULES];
} oFullMap;

#define ome_tiletype(me_ptr)             ((me_ptr)->tiletype)
#define ome_tmap_flr(me_ptr)             ((me_ptr)->space.real.floor)
#define ome_tmap_wall(me_ptr)            ((me_ptr)->space.real.wall)
#define ome_tmap_ceil(me_ptr)            ((me_ptr)->space.real.ceil)
#define ome_tmap(me_ptr,idx)             (((idx) == TMAP_FLR) ? me_tmap_flr(me_ptr) : (((idx) == TMAP_CEIL) ? me_tmap_ceil(me_ptr) : me_tmap_wall(me_ptr)))
#define ome_objref(me_ptr)               ((me_ptr)->objRef)
#define ome_flags(me_ptr)                ((me_ptr)->flags)
#define ome_height_flr(me_ptr)           ((me_ptr)->flr_height)
#define ome_height_ceil(me_ptr)          ((me_ptr)->ceil_height)
#define ome_param(me_ptr)                ((me_ptr)->param)
#define ome_height(me_ptr,idx)           (((idx) == HGT_FLOOR) ? me_height_flr(me_ptr) : me_height_ceil(me_ptr))
#define ome_cybcolor_flr(me_ptr)         ((me_ptr)->space.cyber.floor)
#define ome_cybcolor_ceil(me_ptr)        ((me_ptr)->space.cyber.ceil)
#define ome_templight_flr(me_ptr)        ((me_ptr)->templight.floor)
#define ome_templight_ceil(me_ptr)       ((me_ptr)->templight.ceil)
#define ome_subclip(me_ptr)              ((me_ptr)->rinfo.sub_clip)
#define ome_clearsolid(me_ptr)           ((me_ptr)->rinfo.clear)
#define ome_rotflr(me_ptr)               ((me_ptr)->rinfo.rotflr)
#define ome_rotceil(me_ptr)              ((me_ptr)->rinfo.rotceil)
#define ome_flicker(me_ptr)              ((me_ptr)->rinfo.flicker)
#endif

typedef struct {
   int x_size, y_size;
   int x_shft, y_shft, z_shft;
   MapElem *map;
   bool cyber;
   int x_scale, y_scale, z_scale;
   Schedule sched[NUM_MAP_SCHEDULES];
} FullMap;

#define _me_normal_x(me_ptr,strname,maskname) ((me_ptr)->##strname##&(##maskname##_MASK))
#define _me_normal_n(me_ptr,strname,maskname) (_me_normal_x(me_ptr,strname,maskname)>>##maskname##_SHF)

#define _me_tiletype(me_ptr)             ((me_ptr)->tiletype)

#define MAP_HGT_MASK                     0x1f
#define MAP_HGT_SHF                      0
#define _me_height_flr(me_ptr)           _me_normal_x(me_ptr,flr_rotnhgt,MAP_HGT)
#define _me_height_ceil(me_ptr)          _me_normal_x(me_ptr,ceil_rotnhgt,MAP_HGT)
#define _me_height(me_ptr,idx)           (((idx) == HGT_FLOOR) ? me_height_flr(me_ptr) : me_height_ceil(me_ptr))

#define _me_param(me_ptr)                ((me_ptr)->param)
#define _me_objref(me_ptr)               ((me_ptr)->objRef)

#define MAP_TM_FLOOR_MASK                0xF800
#define MAP_TM_FLOOR_SHF                 11
#define MAP_TM_CEIL_MASK                 0x07C0
#define MAP_TM_CEIL_SHF                  6
#define MAP_TM_WALL_MASK                 0x003F
#define MAP_TM_WALL_SHF                  0
#define _me_tmap_flr_x(me_ptr)           _me_normal_x(me_ptr,tmap_ccolor,MAP_TM_FLOOR)
#define _me_tmap_flr(me_ptr)             _me_normal_n(me_ptr,tmap_ccolor,MAP_TM_FLOOR)
#define _me_tmap_ceil_x(me_ptr)          _me_normal_x(me_ptr,tmap_ccolor,MAP_TM_CEIL)
#define _me_tmap_ceil(me_ptr)            _me_normal_n(me_ptr,tmap_ccolor,MAP_TM_CEIL)
#define _me_tmap_wall_x(me_ptr)          _me_normal_x(me_ptr,tmap_ccolor,MAP_TM_WALL)
#define _me_tmap_wall(me_ptr)            _me_normal_n(me_ptr,tmap_ccolor,MAP_TM_WALL)
#define _me_tmap(me_ptr,idx)             (((idx) == TMAP_FLR) ? me_tmap_flr(me_ptr) : (((idx) == TMAP_CEIL) ? me_tmap_ceil(me_ptr) : me_tmap_wall(me_ptr)))
#define _me_cybcolor_flr(me_ptr)       (*((uchar *)(&((me_ptr)->tmap_ccolor))))
#define _me_cybcolor_ceil(me_ptr)      (*((uchar *)(&((me_ptr)->tmap_ccolor))+1))

// note this returns a long of form f4f3f2f1, so flag4 is most significant, as it were
#define _me_flags(me_ptr)              (*((ulong *)(&((me_ptr)->flag4))))		// KLC changed
#define _me_flag1(me_ptr)                ((me_ptr)->flag1)
#define _me_flag2(me_ptr)                ((me_ptr)->flag2)
#define _me_flag3(me_ptr)                ((me_ptr)->flag3)
#define _me_flag4(me_ptr)                ((me_ptr)->flag4)

#define _me_subclip(me_ptr)              ((me_ptr)->sub_clip)
#define _me_clearsolid(me_ptr)           ((me_ptr)->clearsolid)

#define MAP_ROT_MASK                     0x60
#define MAP_ROT_SHF                      5
#define _me_rotflr_x(me_ptr)             _me_normal_x(me_ptr,flr_rotnhgt,MAP_ROT)
#define _me_rotflr(me_ptr)               _me_normal_n(me_ptr,flr_rotnhgt,MAP_ROT)
#define _me_rotceil_x(me_ptr)            _me_normal_x(me_ptr,ceil_rotnhgt,MAP_ROT)
#define _me_rotceil(me_ptr)              _me_normal_n(me_ptr,ceil_rotnhgt,MAP_ROT)

#define MAP_HAZARD_MASK                  0x80
#define MAP_HAZARD_SHF                   7
#define _me_hazard_bio_x(me_ptr)         _me_normal_x(me_ptr,flr_rotnhgt,MAP_HAZARD)
#define _me_hazard_bio(me_ptr)           _me_normal_n(me_ptr,flr_rotnhgt,MAP_HAZARD)
#define _me_hazard_rad_x(me_ptr)         _me_normal_x(me_ptr,ceil_rotnhgt,MAP_HAZARD)
#define _me_hazard_rad(me_ptr)           _me_normal_n(me_ptr,ceil_rotnhgt,MAP_HAZARD)

#define MAP_FLICKER_MASK                 0xF0
#define MAP_FLICKER_SHF                  4
#define MAP_QUICKCLIP_MASK               0x0F
#define MAP_QUICKCLIP_SHF                0
#define _me_flicker_x(me_ptr)            _me_normal_x(me_ptr,flick_qclip,MAP_FLICKER)
#define _me_flicker(me_ptr)              _me_normal_n(me_ptr,flick_qclip,MAP_FLICKER)
#define _me_quickclip_x(me_ptr)          _me_normal_x(me_ptr,flick_qclip,MAP_QUICKCLIP)
#define _me_quickclip(me_ptr)            _me_normal_n(me_ptr,flick_qclip,MAP_QUICKCLIP)

#define MAP_TLGHT_CEIL_MASK              0xF0
#define MAP_TLGHT_CEIL_SHF               4
#define MAP_TLGHT_FLOOR_MASK             0x0F
#define MAP_TLGHT_FLOOR_SHF              0
#define _me_templight_ceil_x(me_ptr)     _me_normal_x(me_ptr,templight,MAP_TLGHT_CEIL)
#define _me_templight_ceil(me_ptr)       _me_normal_n(me_ptr,templight,MAP_TLGHT_CEIL)
#define _me_templight_flr_x(me_ptr)      _me_normal_x(me_ptr,templight,MAP_TLGHT_FLOOR)
#define _me_templight_flr(me_ptr)        _me_normal_n(me_ptr,templight,MAP_TLGHT_FLOOR)

// implicit me_ptr and v
#define _me_merge_set(me_ptr,v,strname,maskname) \
                                         ((me_ptr)->##strname##=((me_ptr)->##strname##&~(##maskname##_MASK))|((v)<<(##maskname##_SHF)))

// Now all the set primitives
#define _me_tiletype_set(me_ptr,v)       ((me_ptr)->tiletype=(v))
#define _me_height_flr_set(me_ptr,v)     _me_merge_set(me_ptr,v,flr_rotnhgt,MAP_HGT)
#define _me_height_ceil_set(me_ptr,v)    _me_merge_set(me_ptr,v,ceil_rotnhgt,MAP_HGT)
#define _me_height_set(me_ptr,idx,v)     (((idx) == HGT_FLOOR) ? me_height_flr_set(me_ptr,v) : me_height_ceil_set(me_ptr,v))
#define _me_param_set(me_ptr,v)          ((me_ptr)->param=(v))
#define _me_objref_set(me_ptr,v)         ((me_ptr)->objRef=(v))
#define _me_tmap_flr_set(me_ptr,v)       _me_merge_set(me_ptr,v,tmap_ccolor,MAP_TM_FLOOR)
#define _me_tmap_ceil_set(me_ptr,v)      _me_merge_set(me_ptr,v,tmap_ccolor,MAP_TM_CEIL)
#define _me_tmap_wall_set(me_ptr,v)      _me_merge_set(me_ptr,v,tmap_ccolor,MAP_TM_WALL)
#define _me_tmap_set(me_ptr,idx,v)       (((idx) == TMAP_FLR) ? me_tmap_flr_set(me_ptr,v) : (((idx) == TMAP_CEIL) ? me_tmap_ceil_set(me_ptr,v) : me_tmap_wall_set(me_ptr,v)))
#define _me_cybcolor_flr_set(me_ptr,v)   (_me_cybcolor_flr(me_ptr)=(v))
#define _me_cybcolor_ceil_set(me_ptr,v)  (_me_cybcolor_ceil(me_ptr)=(v))
#define _me_flags_set(me_ptr,v)          (_me_flags(me_ptr)=(v))
#define _me_flag1_set(me_ptr,v)          ((me_ptr)->flag1=(v))
#define _me_flag2_set(me_ptr,v)          ((me_ptr)->flag2=(v))
#define _me_flag3_set(me_ptr,v)          ((me_ptr)->flag3=(v))
#define _me_flag4_set(me_ptr,v)          ((me_ptr)->flag4=(v))
#define _me_subclip_set(me_ptr,v)        ((me_ptr)->sub_clip=(v))
#define _me_clearsolid_set(me_ptr,v)     ((me_ptr)->clearsolid=(v))
#define _me_rotflr_set(me_ptr,v)         _me_merge_set(me_ptr,v,flr_rotnhgt,MAP_ROT)
#define _me_rotceil_set(me_ptr,v)        _me_merge_set(me_ptr,v,ceil_rotnhgt,MAP_ROT)
#define _me_hazard_bio_set(me_ptr,v)     _me_merge_set(me_ptr,v,flr_rotnhgt,MAP_HAZARD)
#define _me_hazard_rad_set(me_ptr,v)     _me_merge_set(me_ptr,v,ceil_rotnhgt,MAP_HAZARD)
#define _me_flicker_set(me_ptr,v)        _me_merge_set(me_ptr,v,flick_qclip,MAP_FLICKER)
#define _me_quickclip_set(me_ptr,v)      _me_merge_set(me_ptr,v,flick_qclip,MAP_QUICKCLIP)
#define _me_templight_ceil_set(me_ptr,v) _me_merge_set(me_ptr,v,templight,MAP_TLGHT_CEIL)
#define _me_templight_flr_set(me_ptr,v)  _me_merge_set(me_ptr,v,templight,MAP_TLGHT_FLOOR)

// bind the non _ version of the macros correctly
#ifdef MAP_ACCESS_COUNT
#include "mapcount.h"
#else
#include "mapnorm.h"
#endif

#ifdef __MAP_SRC
MapElem *global_map;
FullMap *global_fullmap;
#else
extern MapElem *global_map;
extern FullMap *global_fullmap;
#endif

FullMap *map_create(int xshf, int yshf,int zshf,bool cyb);
bool map_set_default(FullMap *fmap);
void map_init(void);
void map_free(void);

#define DEFAULT_XSHF 6
#define DEFAULT_YSHF 6
#define DEFAULT_ZSHF 3

#ifndef MAP_RESIZING
#define fm_x_sz(fm_ptr)     (1<<DEFAULT_XSHF)
#define fm_y_sz(fm_ptr)     (1<<DEFAULT_YSHF)
#define fm_x_shft(fm_ptr)   (DEFAULT_XSHF)
#define fm_y_shft(fm_ptr)   (DEFAULT_YSHF)
#else
#define fm_x_sz(fm_ptr)     ((fm_ptr)->x_size)
#define fm_y_sz(fm_ptr)     ((fm_ptr)->y_size)
#define fm_x_shft(fm_ptr)   ((fm_ptr)->x_shft)                              
#define fm_y_shft(fm_ptr)   ((fm_ptr)->y_shft)
#endif

#ifndef MAP_RESHIFTING
#define fm_z_shft(fm_ptr)   (DEFAULT_ZSHF)
#else
#define fm_z_shft(fm_ptr)   ((fm_ptr)->z_shft)
#endif

// look, non stupid non hardcoded defines, how wacky!
#define MAP_YSIZE           (fm_y_sz(global_fullmap))
#define MAP_XSIZE           (fm_x_sz(global_fullmap))
#define MAP_YSHF            (fm_y_shft(global_fullmap))
#define MAP_XSHF            (fm_x_shft(global_fullmap))
#define MAP_ZSHF            (fm_z_shft(global_fullmap)) 

#define fm_map(fm_ptr)      ((fm_ptr)->map)
#define MAP_MAP             (fm_map(global_fullmap))

#define FULLMAP_GET_XY(fmap,x,y) ((fmap)->map+(x)+((y)<<fm_x_shft(fmap)))
#define MAP_GET_XY(x,y)          FULLMAP_GET_XY(global_fullmap,x,y)

// who uses these?
#define MAP_ROWS 64
#define MAP_COLS 64

#define SLOPE_TOTAL   5 
#define SLOPE_SHIFT   MAP_ZSHF
#define SLOPE_SHIFT_U SLOPE_SHIFT
#define SLOPE_SHIFT_D (SLOPE_TOTAL-SLOPE_SHIFT_U)

#define MAP_SC 256
#define MAP_SH 8
#define MAP_MK 0xff
#define MAP_MS 8

#ifdef SAFE_FIX
#define obj_coord_from_fix(fixval)    ((fix_int((fixval)) & 0xFF) << 8) + (fix_frac((fixval)) >> 8)
#define obj_height_from_fix(fixval)   (fix_int((fixval) * (1<<8-SLOPE_SHIFT_D)))
#define obj_angle_from_fix(fixval)    (fix_int(fix_div((fixval), fix_2pi) * 255))
#define obj_angle_from_fixang(fixval) (fix_div((fixval), FIXANG_PI) >> 9)
#define fix_from_obj_coord(sval)      (fix_make((sval) >> 8, ((sval) & 0xFF) << 8))
#define fix_from_obj_height(oid)      (ACK)
#define fix_from_obj_height_val(hval) (ACK)
#define fix_from_obj_angle(byteval)   ((255 - fix_make(byteval,0)) / 64)
#else
#define obj_coord_from_fix(fixval)    ((int)fixval>>8)
#define obj_height_from_fix(fixval)   (fixval>>(8+SLOPE_SHIFT_D))
#define obj_angle_from_fix(fixval)    (fix_int(fix_div((fixval), fix_2pi) * 255))
#define obj_angle_from_fixang(fixval) (fix_div((fixval), FIXANG_PI) >> 9)
#define fix_from_obj_coord(sval)      ((fix)sval<<8)
#define fix_from_obj_height(oid)      ((fix)objs[(oid)].loc.z<<(8+SLOPE_SHIFT_D))
#define fix_from_obj_height_val(hval) (hval<<(8+SLOPE_SHIFT_D))
#define fix_from_obj_angle(byteval)   ((255 - fix_make(byteval,0)) / 64)
#endif

#define fix_inv2pi                    (fix_make(0,10430))
#define obj_angle_from_phys(fixinrad) (64-(((ushort)fix_div(fixinrad,fix_2pi))>>8))
#define phys_angle_from_obj(citang)   (fix_mul((64-citang)<<16,fix_2pi)>>8)
#define phys_angle_from_fixang(fang) (fixang_to_fixrad(FIXANG_PI/2 - (fang))
#define fixang_from_phys_angle(fixdingus) (FIXANG_PI/2 - fixrad_to_fixang(fixdingus))

// this stuff needs a safe fix version
#define fix_from_map_height(mht) (fix_make((mht), 0) >> SLOPE_SHIFT)
#define map_height_from_fix(fht) (fix_int(fht<<SLOPE_SHIFT))

#endif // __MAP_H

