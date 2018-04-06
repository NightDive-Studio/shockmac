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
#ifndef __TEXTMAPS_H
#define __TEXTMAPS_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/textmaps.h $
 * $Revision: 1.45 $
 * $Author: xemu $
 * $Date: 1994/11/01 09:19:48 $
 *
 * 
 *
 */

// Includes

// Defines

#define NUM_TEXTURE_SIZES  4

//#define NUM_LOADED_TEXTURES 64
#define NUM_LOADED_TEXTURES 54
#define GAME_TEXTURES       400
#define NUM_ANIMATING_TEXTURES   0
#define NUM_STATIC_TEXTURES (GAME_TEXTURES - NUM_ANIMATING_TEXTURES)
#define MAX_LOADED_BITMAPS  NUM_LOADED_TEXTURES * NUM_TEXTURE_SIZES

/*  These are for reference, in case the #defines below need to be recomputed.
They are hardwired to avoid needless dependencies.

#define TEXTURE_128_ID      RES_bmTextureMap128
#define TEXTURE_64_ID       RES_bmTextureMap64
#define TEXTURE_32_ID       RES_bmTextureMap32
#define TEXTURE_16_ID       RES_bmTextureMap16

#define TEXTURE_SMALL_ID    RES_smallTextureMaps
*/

#define TEXTURE_128_ID      1000
#define TEXTURE_64_ID       707
#define TEXTURE_32_ID       77
#define TEXTURE_16_ID       76

#define MAX_SMALL_TMAPS 128
#define TEXTURE_SMALL_ID    321

#define TEXTURE_128_INDEX  0
#define TEXTURE_64_INDEX   1
#define TEXTURE_32_INDEX   2
#define TEXTURE_16_INDEX   3

#define SMALLEST_SIZE_INDEX   TEXTURE_128_INDEX
#define LARGEST_SIZE_INDEX    TEXTURE_16_INDEX

#define TEXTPROP_VERSION_NUMBER     9
#define TEXTPROP_FILENAME "textprop.dat"

#define NUM_ANIM_TEXTURE_GROUPS  4

// for new regieme
#define NUM_STATIC_TMAPS NUM_LOADED_TEXTURES
#define SIZE_STATIC_TMAP ((64*64)+(32*32)+(16*16))
#define SIZE_BIG_TMAP   (128*128)

typedef struct {
   // These are indices into the texture_bitmaps array
   // Note that for animating textures, the entry and the
   // next 7 are all reserved for that texture.
   int size_index[NUM_TEXTURE_SIZES];
   ubyte sizes_loaded;
} TextureMap;

typedef struct {
   char family_texture;
   char target_texture;
   short resilience;
   short distance_mod;
   char friction_climb;
   char friction_walk;
   char force_dir;
   char anim_group;
   char group_pos;
} TextureProp;

#define ANIMTEXTURE_CYCLE  0x01

#define ANIMTEXTURE_REVERSED  0x80

typedef struct {
   short anim_speed;
   short time_remainder;
   char current_frame;
   char num_frames;
   char flags;
} AnimTextureData;

#define SUPER_MOD(val, modval) ((modval)?(val%modval):val)     // make sure modval is non-zero before moding

// new rob
#define ANIMTEXT_BASE(tid) (tid - textprops[(tid)].group_pos)
#define ANIMTEXT_FRAME(tid) \
   SUPER_MOD((animtextures[textprops[(tid)].anim_group].current_frame + textprops[(tid)].group_pos), \
      animtextures[textprops[(tid)].anim_group].num_frames)
#define GET_TEXTURE_INDEX(tid,size)   texture_array[ANIMTEXT_BASE(tid)+ANIMTEXT_FRAME(tid)].size_index[(size)]

// Prototypes
void load_textures();
errtype load_alternate_textures();
errtype bitmap_array_unload(int *num_bitmaps, grs_bitmap *arr[]);
errtype Init_Lighting(void);
errtype load_master_texture_properties();
errtype unload_master_texture_properties();
errtype clear_texture_properties();

#define SHADING_TABLE_FNAME       "shadtabl.dat"
#define SHADING_TABLE_AMBER_FNAME "ambrtabl.dat"
#define SHADING_TABLE_BW_FNAME    "bwtabl.dat"

// Globals

#ifdef __TEXTMAPS_SRC
short loved_textures[NUM_LOADED_TEXTURES];
TextureProp textprops[NUM_LOADED_TEXTURES];
AnimTextureData animtextures[NUM_ANIM_TEXTURE_GROUPS];
uchar shading_table[256 * 16];
uchar bw_shading_table[256*16];
TextureProp *texture_properties;
#else
extern short loved_textures[NUM_LOADED_TEXTURES];
extern TextureProp textprops[NUM_LOADED_TEXTURES];
extern AnimTextureData animtextures[NUM_ANIM_TEXTURE_GROUPS];
extern uchar shading_table[256 * 16];
extern uchar bw_shading_table[256*16];
extern TextureProp *texture_properties;
#endif

#endif // __TEXTMAPS_H

