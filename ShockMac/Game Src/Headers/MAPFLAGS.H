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
#ifndef __MAPFLAGS_H
#define __MAPFLAGS_H
/*
 * $Source: r:/prj/cit/src/inc/RCS/mapflags.h $
 * $Revision: 1.16 $
 * $Author: minman $
 * $Date: 1994/07/30 00:18:26 $
 *
 * $Log: mapflags.h $
 * Revision 1.16  1994/07/30  00:18:26  minman
 * look rend3 settting
 * 
 * Revision 1.15  1994/07/27  04:58:40  dc
 * backdated, new set and stuff for family, some _set bug fixes
 * 
 * Revision 1.14  1994/01/22  18:57:12  dc
 * new map regieme, take 4
 * 
 * Revision 1.13  1994/01/02  17:17:14  dc
 * Initial revision
 * 
 * Revision 1.12  1993/10/21  16:42:10  mahk
 * Added peril bit
 * 
 * Revision 1.11  1993/10/18  03:35:50  dc
 * adding dumb-o @ifndef __SPEW
 * 
 * Revision 1.10  1993/09/09  03:22:01  dc
 * render bits defines
 * 
 * Revision 1.9  1993/09/06  05:56:56  mahk
 * Broke up render bits into two chunks.
 * 
 * Revision 1.8  1993/09/06  05:30:28  dc
 * Yo Yo Yo flip defines, so on
 * 
 * Revision 1.7  1993/09/06  01:58:14  mahk
 * Consolidated 8 super-special rendering bits at the bottom.  
 * Moved mirror bits over finally.
 * 
 * Revision 1.6  1993/09/02  23:07:52  xemu
 * angle me baby
 * 
 * Revision 1.5  1993/09/02  17:11:22  mahk
 * Added fambly bit.
 * 
 * Revision 1.4  1993/08/27  11:31:34  mahk
 * Added another music bit.
 * 
 * Revision 1.3  1993/08/22  17:50:55  xemu
 * added to me_bits_deconst_set accessor
 * 
 * Revision 1.2  1993/08/12  16:21:42  mahk
 * Put in accessors.
 * 
 * Revision 1.1  1993/08/12  16:11:39  mahk
 * Initial revision
 * 
 *
 */

#include "map.h"

#define OLD_MAP

#ifdef OLD_MAP

// for mirror bits, which control how the cieling and floor behave
#define OLD_MAP_MATCH          0
#define OLD_MAP_MIRROR         1
#define OLD_MAP_CFLAT          2
#define OLD_MAP_FFLAT          3
#define OLD_MIRROR_VALS        4

// for flip bits, which control texture left right mapping
#define OLD_MAP_FLIP_NOPE      0
#define OLD_MAP_FLIP_YUP       1
#define OLD_MAP_FLIP_ODD       2
#define OLD_MAP_FLIP_EVEN      3

#define OLD_MAP_VLOCK_MASK     0x1F
#define OLD_MAP_VLOCK_SHF      0

#define OLD_MAP_FLIP_MASK      0x60
#define OLD_MAP_FLIP_SHF       5
#define OLD_MAP_FLIP_FNCY_MASK 0x40    // this is the bit which indicates a parity check is necessary
#define OLD_MAP_FLIP_PRTY_MASK 0x20    // this is the bit which actual has which parity it is
#define OLD_MAP_FLIP_PRTY_SHF  MAP_FLIP_SHF

#define OLD_MAP_FAMILY_MASK    0x80
#define OLD_MAP_FAMILY_SHF     7

#define OLD_MAP_FAUXREND_MASK  0xFF

#define OLD_MAP_MIRROR_MASK    0x300
#define OLD_MAP_MIRROR_SHF     8

#define OLD_MAP_MUSIC_MASK     0x1C00
#define OLD_MAP_MUSIC_SHF      10

#define OLD_MAP_PERIL_MASK     0x2000
#define OLD_MAP_PERIL_SHF      13

#define OLD_MAP_DECONST_MASK   0x4000
#define OLD_MAP_DECONST_SHF    14

#define OLD_MAP_FRIEND_MASK    0x8000
#define OLD_MAP_FRIEND_SHF     15

#define OLD_MAP_F_LIGHT_MASK   0x0F0000
#define OLD_MAP_F_LIGHT_SHF    16

#define OLD_MAP_C_LIGHT_MASK   0xF00000
#define OLD_MAP_C_LIGHT_SHF    20

#define OLD_MAP_REND4_MASK     0x0F000000
#define OLD_MAP_REND4_SHF      24

#define OLD_MAP_REND3_MASK     0x70000000
#define OLD_MAP_REND3_SHF      28

#define OLD_MAP_REND_MASK      (MAP_REND3_MASK|MAP_REND4_MASK)
#define OLD_MAP_REND_SHF       24

#define OLD_MAP_SEEN_MASK    0x80000000
#define OLD_MAP_SEEN_SHF     31
//#define OLD_MAP_SEEN_MASK      0x40000000
//#define OLD_MAP_SEEN_SHF       30

#define OLD_MAP_MATCH_X       (0<<OLD_MAP_MIRROR_SHF)
#define OLD_MAP_MIRROR_X      (1<<OLD_MAP_MIRROR_SHF)
#define OLD_MAP_CFLAT_X       (2<<OLD_MAP_MIRROR_SHF)
#define OLD_MAP_FFLAT_X       (3<<OLD_MAP_MIRROR_SHF)

#define ome_bits_vlock(me_ptr)   ((ome_flags(me_ptr) & OLD_MAP_VLOCK_MASK)   >> OLD_MAP_VLOCK_SHF)
#define ome_bits_flip(me_ptr)    ((ome_flags(me_ptr) & OLD_MAP_FLIP_MASK)    >> OLD_MAP_FLIP_SHF)
#define ome_bits_family(me_ptr)  ((ome_flags(me_ptr) & OLD_MAP_FAMILY_MASK)  >> OLD_MAP_FAMILY_SHF)
#define ome_bits_friend(me_ptr)  ((ome_flags(me_ptr) & OLD_MAP_FRIEND_MASK)  >> OLD_MAP_FRIEND_SHF)
#define ome_bits_mirror(me_ptr)  ((ome_flags(me_ptr) & OLD_MAP_MIRROR_MASK)  >> OLD_MAP_MIRROR_SHF)
#define ome_bits_mirror_x(me_ptr) (ome_flags(me_ptr) & OLD_MAP_MIRROR_MASK)
#define ome_bits_music(me_ptr)   ((ome_flags(me_ptr) & OLD_MAP_MUSIC_MASK)   >> OLD_MAP_MUSIC_SHF)
#define ome_bits_peril(me_ptr)   ((ome_flags(me_ptr) & OLD_MAP_PERIL_MASK)   >> OLD_MAP_PERIL_SHF)
#define ome_bits_hit(me_ptr)     ((ome_flags(me_ptr) & OLD_MAP_HIT_MASK)     >> OLD_MAP_HIT_SHF)
#define ome_bits_deconst(me_ptr) ((ome_flags(me_ptr) & OLD_MAP_DECONST_MASK) >> OLD_MAP_DECONST_SHF)
#define ome_bits_rend(me_ptr)    ((ome_flags(me_ptr) & OLD_MAP_REND_MASK)    >> OLD_MAP_REND_SHF)
#define ome_bits_rend4(me_ptr)   ((ome_flags(me_ptr) & OLD_MAP_REND4_MASK)    >> OLD_MAP_REND4_SHF)
#define ome_bits_rend3(me_ptr)   ((ome_flags(me_ptr) & OLD_MAP_REND3_MASK)    >> OLD_MAP_REND3_SHF)
#define ome_bits_fauxrend(me_ptr) (ome_flags(me_ptr) & OLD_MAP_FAUXREND_MASK)
#define ome_bits_seen_p(me_ptr)   (ome_flags(me_ptr) & OLD_MAP_SEEN_MASK)

#define ome_light_flr(me_ptr)    ((ome_flags(me_ptr) & OLD_MAP_F_LIGHT_MASK) >> OLD_MAP_F_LIGHT_SHF)
#define ome_light_ceil(me_ptr)   ((ome_flags(me_ptr) & OLD_MAP_C_LIGHT_MASK) >> OLD_MAP_C_LIGHT_SHF)
#define ome_light_flr_set(me_ptr,v)    ((me_ptr)->flags = (((me_ptr)->flags & ~OLD_MAP_F_LIGHT_MASK) | ((v) << OLD_MAP_F_LIGHT_SHF)))
#define ome_light_ceil_set(me_ptr,v)   ((me_ptr)->flags = (((me_ptr)->flags & ~OLD_MAP_C_LIGHT_MASK) | ((v) << OLD_MAP_C_LIGHT_SHF)))

// hmm something like this perhaps
//#define ome_light_flr(me_ptr)      ( (((uchar *)&(ome_flags(me_ptr)))+1) & 0x0f )
//#define ome_light_ceil(me_ptr)     ( (((uchar *)&(ome_flags(me_ptr)))+1) & 0xf0 )

#define ome_bits_rend_set(me_ptr,v)     (ome_flags_set(me_ptr,(ome_flags(me_ptr) & ~OLD_MAP_REND_MASK) | ((v) << OLD_MAP_REND_SHF)  ))
#define ome_bits_music_set(me_ptr,v)    (ome_flags_set(me_ptr,(ome_flags(me_ptr) & ~OLD_MAP_MUSIC_MASK) | ((v) << OLD_MAP_MUSIC_SHF)  ))
#define ome_bits_hit_set(me_ptr,v)      (ome_flags_set(me_ptr,(ome_flags(me_ptr) & ~OLD_MAP_HIT_MASK) | ((v) << OLD_MAP_HIT_SHF)  ))
#define ome_bits_deconst_set(me_ptr,v)  (ome_flags_set(me_ptr,(ome_flags(me_ptr) & ~OLD_MAP_DECONST_MASK) | ((v) << OLD_MAP_DECONST_SHF)  ))
#define ome_bits_mirror_set(me_ptr,v)   (ome_flags_set(me_ptr,(ome_flags(me_ptr) & ~OLD_MAP_MIRROR_MASK) | (v) << OLD_MAP_MIRROR_SHF  ))
#define ome_bits_seen_set(me_ptr)       (ome_flags(me_ptr)|= OLD_MAP_SEEN_MASK)
#define ome_bits_seen_clear(me_ptr)     (ome_flags(me_ptr)&=~OLD_MAP_SEEN_MASK)

#endif

// FLAG 1
// for mirror bits, which control how the cieling and floor behave
#define MAP_MATCH          0
#define MAP_MIRROR         1
#define MAP_CFLAT          2
#define MAP_FFLAT          3
#define MIRROR_VALS        4

// for flip bits, which control texture left right mapping
#define MAP_FLIP_NOPE      0
#define MAP_FLIP_YUP       1
#define MAP_FLIP_ODD       2
#define MAP_FLIP_EVEN      3

#define MAP_VLOCK_MASK     0x1F
#define MAP_VLOCK_SHF      0

#define MAP_FLIP_MASK      0x60
#define MAP_FLIP_SHF       5
#define MAP_FLIP_FNCY_MASK 0x40    // this is the bit which indicates a parity check is necessary
#define MAP_FLIP_PRTY_MASK 0x20    // this is the bit which actual has which parity it is
#define MAP_FLIP_PRTY_SHF  MAP_FLIP_SHF

#define MAP_FAMILY_MASK    0x80
#define MAP_FAMILY_SHF     7

#define MAP_FAUXREND_MASK  0xFF

// FLAG 2
#define MAP_FRIEND_MASK    0x1
#define MAP_FRIEND_SHF     0
#define MAP_FRIEND_FULL_MASK (MAP_FRIEND_MASK<<(MAP_FRIEND_SHF+8))

#define MAP_DECONST_MASK   0x2
#define MAP_DECONST_SHF    1

#define MAP_MIRROR_MASK    0xC
#define MAP_MIRROR_SHF     2

#define MAP_PERIL_MASK     0x10
#define MAP_PERIL_SHF      4

#define MAP_MUSIC_MASK     0xE0
#define MAP_MUSIC_SHF      5

// FLAG 3
#define MAP_F_LIGHT_MASK   0x0F
#define MAP_F_LIGHT_SHF    0

#define MAP_REND4_MASK     0xF0
#define MAP_REND4_SHF      4

// FLAG 4
#define MAP_C_LIGHT_MASK   0xF
#define MAP_C_LIGHT_SHF    0

#define MAP_REND3_MASK     0x70
#define MAP_REND3_SHF      4

#define MAP_SEEN_MASK      0x80
#define MAP_SEEN_SHF       7

#define me_bits_vlock_x(me_ptr)         (me_flag1(me_ptr)&MAP_VLOCK_MASK)
#define me_bits_vlock(me_ptr)           (me_bits_vlock_x(me_ptr)>>MAP_VLOCK_SHF)
#define me_bits_flip_x(me_ptr)          (me_flag1(me_ptr)&MAP_FLIP_MASK)
#define me_bits_flip(me_ptr)            (me_bits_flip_x(me_ptr)>>MAP_FLIP_SHF)
#define me_bits_family_x(me_ptr)        (me_flag1(me_ptr)&MAP_FAMILY_MASK)
#define me_bits_family(me_ptr)          (me_bits_family_x(me_ptr)>>MAP_FAMILY_SHF)
#define me_bits_fauxrend                 me_flag1

#define me_bits_friend_x(me_ptr)        (me_flag2(me_ptr)&MAP_FRIEND_MASK)
#define me_bits_friend(me_ptr)          (me_bits_friend_x(me_ptr)>>MAP_FRIEND_SHF)
#define me_bits_mirror_x(me_ptr)        (me_flag2(me_ptr)&MAP_MIRROR_MASK)
#define me_bits_mirror(me_ptr)          (me_bits_mirror_x(me_ptr)>>MAP_MIRROR_SHF)
#define me_bits_music_x(me_ptr)         (me_flag2(me_ptr)&MAP_MUSIC_MASK)
#define me_bits_music(me_ptr)           (me_bits_music_x(me_ptr)>>MAP_MUSIC_SHF)
#define me_bits_peril_x(me_ptr)         (me_flag2(me_ptr)&MAP_PERIL_MASK)
#define me_bits_peril(me_ptr)           (me_bits_peril_x(me_ptr)>>MAP_PERIL_SHF)
#define me_bits_deconst_x(me_ptr)       (me_flag2(me_ptr)&MAP_DECONST_MASK)
#define me_bits_deconst(me_ptr)         (me_bits_deconst_x(me_ptr)>>MAP_DECONST_SHF)

#define me_bits_rend4_x(me_ptr)         (me_flag3(me_ptr)&MAP_REND4_MASK)
#define me_bits_rend4(me_ptr)           (me_bits_rend4_x(me_ptr)>>MAP_REND4_SHF)
#define me_light_flr_x(me_ptr)          (me_flag3(me_ptr)&MAP_F_LIGHT_MASK)
#define me_light_flr(me_ptr)            (me_light_flr_x(me_ptr)>>MAP_F_LIGHT_SHF)

#define me_bits_rend3_x(me_ptr)         (me_flag4(me_ptr)&MAP_REND3_MASK)
#define me_bits_rend3(me_ptr)           (me_bits_rend3_x(me_ptr)>>MAP_REND3_SHF)
#define me_bits_seen_x(me_ptr)          (me_flag4(me_ptr)&MAP_SEEN_MASK)
#define me_bits_seen(me_ptr)            (me_bits_seen_x(me_ptr)>>MAP_SEEN_SHF)
#define me_light_ceil_x(me_ptr)         (me_flag4(me_ptr)&MAP_C_LIGHT_MASK)
#define me_light_ceil(me_ptr)           (me_light_ceil_x(me_ptr)>>MAP_C_LIGHT_SHF)

#define me_friend_set_x(me_ptr,v)       (me_flag2_set(me_ptr,(me_flag2(me_ptr)&~MAP_FRIEND_MASK)|(v)))
#define me_friend_set(me_ptr,v)         (me_friend_set_x(me_ptr,(v)<<MAP_FRIEND_SHF))
#define me_family_set_x(me_ptr,v)       (me_flag1_set(me_ptr,(me_flag1(me_ptr)&~MAP_FAMILY_MASK)|(v)))
#define me_family_set(me_ptr,v)         (me_family_set_x(me_ptr,(v)<<MAP_FAMILY_SHF))
#define me_flip_set_x(me_ptr,v)         (me_flag1_set(me_ptr,(me_flag1(me_ptr)&~MAP_FLIP_MASK)|(v)))
#define me_flip_set(me_ptr,v)           (me_flip_set_x(me_ptr,(v)<<MAP_FLIP_SHF))

#define me_rend3_set_x(me_ptr,v)        (me_flag4_set(me_ptr,(me_flag4(me_ptr)&~MAP_REND3_MASK)|(v)))
#define me_rend3_set(me_ptr,v)          (me_rend3_set_x(me_ptr,(v)<<MAP_REND3_SHF))

#define me_light_flr_set_x(me_ptr,v)    (me_flag3_set(me_ptr,(me_flag3(me_ptr)&~MAP_F_LIGHT_MASK)|(v)))
#define me_light_flr_set(me_ptr,v)      (me_light_flr_set_x(me_ptr,(v)<<MAP_F_LIGHT_SHF))
#define me_light_ceil_set_x(me_ptr,v)   (me_flag3_set(me_ptr,(me_flag3(me_ptr)&~MAP_C_LIGHT_MASK)|(v)))
#define me_light_ceil_set(me_ptr,v)     (me_light_ceil_set_x(me_ptr,(v)<<MAP_C_LIGHT_SHF))
#define me_bits_music_set_x(me_ptr,v)   (me_flag2_set(me_ptr,(me_flag2(me_ptr)&~MAP_MUSIC_MASK)|(v)))
#define me_bits_music_set(me_ptr,v)     (me_bits_music_set_x(me_ptr,(v)<<MAP_MUSIC_SHF))
#define me_bits_deconst_set_x(me_ptr,v) (me_flag2_set(me_ptr,(me_flag2(me_ptr)&~MAP_DECONST_MASK)|(v)))
#define me_bits_deconst_set(me_ptr,v)   (me_bits_music_set_x(me_ptr,(v)<<MAP_DECONST_SHF))
#define me_bits_mirror_set_x(me_ptr,v)  (me_flag1_set(me_ptr,(me_flag1(me_ptr)&~MAP_MIRROR_MASK)|(v)))
#define me_bits_mirror_set(me_ptr,v)    (me_bits_music_set_x(me_ptr,(v)<<MAP_MIRROR_SHF))
#define me_bits_seen_set(me_ptr)        (me_flag4(me_ptr)|=MAP_SEEN_MASK)
#define me_bits_seen_clear(me_ptr)      (me_flag4(me_ptr)&=~MAP_SEEN_MASK)

#endif // __MAPFLAGS_H

