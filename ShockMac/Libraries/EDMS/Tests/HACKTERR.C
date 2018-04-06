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
// Look Ma, a hacked terrain function

#include "fix.h"
#include "error.h"
#include "mprintf.h"

typedef struct {
   // Ceiling slope
   fix cx,cy,cz;

   // Floor slope
   fix fx,fy,fz;

   // Wall gradient
   fix wx,wy,wz;
} TerrainData;

TerrainData terrain_info;

#define CSLOPE_SET(a,b,c) { terrain_info.cx = (a); terrain_info.cy = (b); terrain_info.cz = (c); }
#define FSLOPE_SET(a,b,c) { terrain_info.fx = (a); terrain_info.fy = (b); terrain_info.fz = (c); }
#define WGRAD_SET(a,b,c) { terrain_info.wx = (a); terrain_info.wy = (b); terrain_info.wz = (c); }
#define WGRAD_ADD(a,b,c) { terrain_info.wx += (a); terrain_info.wy += (b); terrain_info.wz += (c); }

typedef struct {
   uchar tiletype;
   uchar tmaps[3];
   short flags;
   uchar heights[2];
   uchar param;
   short objRef;
} MapElem;

#define me_tiletype(me_ptr)   ((me_ptr)->tiletype)
#define me_tmap(me_ptr,idx)   ((me_ptr)->tmaps[idx])
#define me_tmap_flr(me_ptr)   (me_ptr)->tmaps[TMAP_FLR]
#define me_tmap_wall(me_ptr)  (me_ptr)->tmaps[TMAP_WALL]
#define me_tmap_ceil(me_ptr)  (me_ptr)->tmaps[TMAP_CEIL]
#define me_objref(me_ptr)     ((me_ptr)->objRef)
#define me_flags(me_ptr)      ((me_ptr)->flags)
#define me_height_flr(me_ptr)  ((me_ptr)->heights[HGT_FLOOR])
#define me_height_ceil(me_ptr) ((me_ptr)->heights[HGT_CEIL])
#define me_param(me_ptr)       ((me_ptr)->param)
#define me_height(me_ptr,idx)  ((me_ptr)->heights[idx])
#define me_bits_mirror(me_ptr)  (me_flags(me_ptr) & MAP_MIRROR_MASK)
#define me_bits_music(me_ptr)   ((me_flags(me_ptr) & MAP_MUSIC_MASK) >> MAP_MUSIC_SHF)
#define me_bits_hit(me_ptr)   ((me_flags(me_ptr) & MAP_HIT_MASK) >> MAP_HIT_SHF)
#define me_light_flr(me_ptr)  (((me_ptr)->lights & MAP_F_LIGHT_MASK) >> MAP_F_LIGHT_SHF)
#define me_light_ceil(me_ptr)  (((me_ptr)->lights & MAP_C_LIGHT_MASK) >> MAP_C_LIGHT_SHF)

#define me_tiletype_set(me_ptr,v)      ((me_ptr)->tiletype = (v))
#define me_tmap_flr_set(me_ptr,v)      ((me_ptr)->tmaps[TMAP_FLR] = (v))
#define me_tmap_wall_set(me_ptr,v)     ((me_ptr)->tmaps[TMAP_WALL] = (v))
#define me_tmap_ceil_set(me_ptr,v)     ((me_ptr)->tmaps[TMAP_CEIL] = (v))
#define me_objref_set(me_ptr,v)        ((me_ptr)->objRef = (v))
#define me_flags_set(me_ptr,v)         ((me_ptr)->flags= (v))
#define me_height_flr_set(me_ptr,v)    ((me_ptr)->heights[HGT_FLOOR]= (v))
#define me_height_ceil_set(me_ptr,v)   ((me_ptr)->heights[HGT_CEIL]= (v))
#define me_param_set(me_ptr,v)         ((me_ptr)->param= (v))
#define me_height_set(me_ptr,idx,v)    ((me_ptr)->heights[idx] = (v))
#define me_tmap_set(me_ptr,idx,v)      ((me_ptr)->tmaps[idx] =(v))
#define me_bits_mirror_set(me_ptr,v)   (me_flags_set(me_ptr,(me_flags(me_ptr) & ~MAP_MIRROR_MASK) | (v)))
#define me_bits_music_set(me_ptr,v)    (me_flags_set(me_ptr,(me_flags(me_ptr) & ~MAP_MUSIC_MASK) | ((v) << MAP_MUSIC_SHF)  ))
#define me_bits_hit_set(me_ptr,v)      (me_flags_set(me_ptr,(me_flags(me_ptr) & ~MAP_HIT_MASK) | ((v) << MAP_HIT_SHF)  ))
#define me_light_flr_set(me_ptr,v)     ((me_ptr)->lights = (((me_ptr)->lights & ~MAP_F_LIGHT_MASK) | ((v) << MAP_F_LIGHT_SHF)))
#define me_light_ceil_set(me_ptr,v)    ((me_ptr)->lights = (((me_ptr)->lights & ~MAP_C_LIGHT_MASK) | ((v) << MAP_C_LIGHT_SHF)))




#define fm_x_sz(fm_ptr)        ((fm_ptr)->x_size)
#define fm_y_sz(fm_ptr)        ((fm_ptr)->y_size)
#define fm_z_shft(fm_ptr)      ((fm_ptr)->z_shft)
#define fm_x_shft(fm_ptr)      ((fm_ptr)->x_shft)                              
#define fm_y_shft(fm_ptr)      ((fm_ptr)->y_shft)
#define fm_map(fm_ptr)         ((fm_ptr)->map)

#define MAP_TYPES 32
#define MAP_NUM_TMAP1 128
#define MAP_NUM_TMAP2 32
#define MAP_NUM_TMAP3 16
#define MAP_HEIGHTS 32
#define MAP_PARAMS 16

#define MAP_MIRROR_MASK 0x3
#define MAP_MIRROR_SHF 0
#define MAP_MATCH  0
#define MAP_MIRROR 1
#define MAP_CFLAT  2
#define MAP_FFLAT  3
#define MIRROR_VALS 4

#define MAP_MUSIC_MASK 0x3C
#define MAP_MUSIC_SHF  2

#define MAP_HIT_MASK 0x40
#define MAP_HIT_SHF  6

#define MAP_F_LIGHT_MASK 0x0F00
#define MAP_F_LIGHT_SHF 8

#define MAP_C_LIGHT_MASK 0xF000
#define MAP_C_LIGHT_SHF 12



#define TMAP_FLR  0
#define TMAP_WALL 1
#define TMAP_CEIL 2

#define HGT_CEIL        0
#define HGT_FLOOR       1

#define TILE_SOLID         0
#define TILE_OPEN          1
#define TILE_SOLID_NW      2
#define TILE_SOLID_NE      3
#define TILE_SOLID_SE      4
#define TILE_SOLID_SW      5
#define TILE_SLOPEUP_N     6
#define TILE_SLOPEUP_E     7
#define TILE_SLOPEUP_S     8
#define TILE_SLOPEUP_W     9
#define TILE_SLOPECC_NW    10
#define TILE_SLOPECC_SW    11
#define TILE_SLOPECC_NE    12
#define TILE_SLOPECC_SE    13
#define TILE_SLOPECV_NW    14
#define TILE_SLOPECV_SW    15
#define TILE_SLOPECV_NE    16
#define TILE_SLOPECV_SE    17
#define TILE_DSPLIT_NW     18
#define TILE_DSPLIT_NE     19
#define TILE_DSPLIT_SW     20
#define TILE_DSPLIT_SE     21
#define TILE_OCT_NS        22
#define TILE_OCT_EW        23
#define TILE_TRI_NS        24
#define TILE_TRI_EW        25
#define TILE_1Q_NW2E       26
#define TILE_1Q_NW2S       27
#define TILE_1Q_SW2E       28
#define TILE_1Q_SW2N       29
#define TILE_1Q_NE2W       30
#define TILE_1Q_NE2S       31
#define TILE_1Q_SE2W       32
#define TILE_1Q_SE2N       33
#define TILE_3Q_NW2E       34
#define TILE_3Q_NW2S       35
#define TILE_3Q_SW2E       36
#define TILE_3Q_SW2N       37
#define TILE_3Q_NE2W       38
#define TILE_3Q_NE2S       39
#define TILE_3Q_SE2W       40
#define TILE_3Q_SE2N       41
#define TILE_VSPLIT        42
#define TILE_HALVED_NSW    43 
#define TILE_HALVED_NSE    44
#define TILE_HALVED_EWN    45
#define TILE_HALVED_EWS    46
#define TILE_SLIMWALL_N    47
#define TILE_SLIMWALL_S    48
#define TILE_SLIMWALL_E    49
#define TILE_SLIMWALL_W    50

#define TILE_TYPES         64 // (TILE_SLOPEUP_W+1) 

#define open_tile  { TILE_OPEN, {0, 0, 0}, 0, {0, 0}, 0, 0 }

#define full_tile  { TILE_SOLID, {0, 0, 0}, 0, {0, 0}, 0, 0 }

MapElem da_map[6][6] = {
  { full_tile,  full_tile,  full_tile, full_tile, full_tile, full_tile} ,
  { full_tile,  open_tile,  open_tile, open_tile, open_tile, full_tile} ,
  { full_tile,  open_tile,  open_tile, open_tile, open_tile, full_tile} ,
  { full_tile,  open_tile,  open_tile, open_tile, open_tile, full_tile} ,
  { full_tile,  open_tile,  open_tile, open_tile, open_tile, full_tile} ,
  { full_tile,  full_tile,  full_tile, full_tile, full_tile, full_tile} };

#define MAP_GET_XY(x,y) &(da_map[x][y]);

#define NORTH_PLANE  0
#define SOUTH_PLANE  1
#define EAST_PLANE  2
#define WEST_PLANE  3

int num_vert_plane_facelets(MapElem *curr, MapElem *adj, short plane_id, fix xy, fix z)
{
   int retval = 0;
   if (me_tiletype(adj) == TILE_SOLID)
      return(1);
   if (me_height_flr(curr) < me_height_flr(adj))
      retval++;
   if (me_height_ceil(curr) > me_height_ceil(adj))
      retval++;
   return(retval);
}

errtype physics_apply_tile_contents(MapElem *tile, fix x_off, fix y_off, fix z_off, fix rad)
{
   return(OK);
}

errtype physics_apply_vert_plane(short plane_id, fix xy, fix z, fix rad, fix dist, MapElem *curr, MapElem *adj_me)
{
   int num_f;
   fix quan;
   char fixtemp1[15],fixtemp2[15],fixtemp3[15];

//   Spew(DSRC_PHYSICS_Updates, ("{adj=%d d=%d dst=%x.%x}",me_tiletype(adj_me),plane_id,EXPAND_FIX(dist)));
   num_f = num_vert_plane_facelets(curr, adj_me, plane_id,xy,z);
   if (num_f > 0)
   {
      quan = FIX_UNIT - fix_div(dist, rad);

      fix_sprint(fixtemp1,dist);
      fix_sprint(fixtemp2,rad);
      fix_sprint(fixtemp3,quan);
//      mprintf("dist = %s rad = %s 1-(d/r) = %s",fixtemp1,fixtemp2,fixtemp3);
      switch(plane_id)
      {
         case NORTH_PLANE: WGRAD_ADD(0,-quan,0); break;
         case SOUTH_PLANE: WGRAD_ADD(0,quan,0); break;
         case EAST_PLANE:  WGRAD_ADD(-quan,0,0); break;
         case WEST_PLANE:  WGRAD_ADD(quan,0,0); break;
      }
   }
   return(OK);
}

void Indoor_Terrain(fix fix_x, fix fix_y, fix fix_z, fix rad)
{
   MapElem *pme, *adj;
   char ft1[15],ft2[15],ft3[15],ft4[16],ft5[16],ft6[16];
   char fixtemp1[15],fixtemp2[15],fixtemp3[15],fixtemp4[15];

   pme = MAP_GET_XY(fix_int(fix_x),fix_int(fix_y));

   // Compute Slopes
   switch(me_tiletype(pme))
   {
      case TILE_OPEN:
      case TILE_SOLID_NW:
      case TILE_SOLID_NE:
      case TILE_SOLID_SW:
      case TILE_SOLID_SE:
      case TILE_SOLID:
      default:
         if (fix_z>rad)
         {
            FSLOPE_SET(0,0,0);
         }
         else
         {
            FSLOPE_SET(0,0,FIX_UNIT - fix_div(fix_z,rad));
         }
         break;
   }

   switch(me_bits_mirror(pme) & MAP_MIRROR_MASK)
   {
#ifdef AGH
      case MAP_MIRROR:
         CSLOPE_SET(terrain_info.fx,terrain_info.fy, -terrain_info.fz);
         break;
      case MAP_MATCH:
         CSLOPE_SET(-terrain_info.fx, -terrain_info.fy, -terrain_info.fz);
         break;
      case MAP_FFLAT:
         CSLOPE_SET(-terrain_info.fx, -terrain_info.fy, -terrain_info.fz);
         FSLOPE_SET(0,0,0);
         break;
      case MAP_CFLAT:
         CSLOPE_SET(0,0,0);
         break;
#endif
      default:
         CSLOPE_SET(0,0,0);
         break;
   }

   // Compute Wall Gradient

   WGRAD_SET(0,0,0);

   // Look at tiles around us

   fix_sprint(fixtemp1,rad);
   fix_sprint(fixtemp2,fix_x);
   fix_sprint(fixtemp3,fix_y);
   fix_sprint(fixtemp4,fix_z);

//   mprintf("rad=%s (%s,%s,%s)\n", fixtemp1, fixtemp2, fixtemp3, fixtemp4);
   physics_apply_tile_contents(pme, fix_frac(fix_x), fix_frac(fix_y), fix_z, rad);

//   Spew(DSRC_PHYSICS_Updates, (" frac_y+%x.%x=%x.%x ",EXPAND_FIX(rad),EXPAND_FIX(fix_frac(fix_y) + rad)));
   if ((fix_frac(fix_y) + rad) > fix_make(1,0))  // Look North
   {
      adj = MAP_GET_XY(fix_int(fix_x), fix_int(fix_y) + 1);
      physics_apply_vert_plane(NORTH_PLANE, fix_frac(fix_x), fix_z, rad, fix_make(1,0) - fix_frac(fix_y), pme, adj);
      physics_apply_tile_contents(adj, fix_frac(fix_x), -(FIX_UNIT - fix_frac(fix_y)), fix_z, rad);
   }
   else if (fix_frac(fix_y) < rad) // Look South
   {
      adj = MAP_GET_XY(fix_int(fix_x), fix_int(fix_y) - 1);
      physics_apply_vert_plane(SOUTH_PLANE, fix_frac(fix_x), fix_z, rad, fix_frac(fix_y), pme, adj);
      physics_apply_tile_contents(adj, fix_frac(fix_x), -fix_frac(fix_y), fix_z, rad);
   }

   if ((fix_frac(fix_x) + rad) > fix_make(1,0))  // Look East
   {
      adj = MAP_GET_XY(fix_int(fix_x) + 1, fix_int(fix_y));
      physics_apply_vert_plane(EAST_PLANE, fix_frac(fix_y), fix_z, rad, fix_make(1,0) - fix_frac(fix_x), pme, adj);
      physics_apply_tile_contents(adj, -(FIX_UNIT - fix_frac(fix_x)), fix_frac(fix_y), fix_z, rad);
   }
   else if (fix_frac(fix_x) < rad) // Look West
   {
      adj = MAP_GET_XY(fix_int(fix_x) - 1, fix_int(fix_y));
      physics_apply_vert_plane(WEST_PLANE, fix_frac(fix_y), fix_z, rad, fix_frac(fix_x), pme, adj);
      physics_apply_tile_contents(adj, -fix_frac(fix_x), fix_frac(fix_y), fix_z, rad);
   }
//   Spew(DSRC_PHYSICS_Updates, ("heights:f=%x.%x c=%x.%x",fix_int(terrain_info.zf),fix_frac(terrain_info.zc)));

   fix_sprint(ft1,terrain_info.fx);
   fix_sprint(ft2,terrain_info.fy);
   fix_sprint(ft3,terrain_info.fz);
   fix_sprint(ft4,terrain_info.cx);
   fix_sprint(ft5,terrain_info.cy);
   fix_sprint(ft6,terrain_info.cz);
//   mprintf("slopes:f=(%s,%s,%s) c=(%s,%s,%s)\n",ft1,ft2,ft3,ft4,ft5,ft6);

   fix_sprint(ft1,terrain_info.wx);
   fix_sprint(ft2,terrain_info.wy);
   fix_sprint(ft3,terrain_info.wz);
//   mprintf("wgrad=(%s,%s,%s)\n",ft1,ft2,ft3);
}
