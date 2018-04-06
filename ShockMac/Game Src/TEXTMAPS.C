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
 * $Source: r:/prj/cit/src/RCS/textmaps.c $
 * $Revision: 1.77 $
 * $Author: xemu $
 * $Date: 1994/11/21 17:38:46 $
 */

#define __TEXTMAPS_SRC

#include <string.h>

#include "ShockDialogs.h"

#include "textmaps.h"
#include "gettmaps.h"
#include "tools.h"
#include "frprotox.h"
#include "cybmem.h"
#include "criterr.h"
#include "citres.h"
#include "rendtool.h"
#include "objects.h"
#include "objstuff.h"
#include "tpolys.h"
#include "statics.h"

//#include <mprintf.h>
//#include <dpaths.h>
//#include <config.h>
//#include <_gfx.h>
//#include <_system.h>
//#include <_lg.h>

bool textures_loaded = FALSE;

#define READ(fd,x) read(fd,(char*)&(x),sizeof(x))

Id tmap_ids[NUM_TEXTURE_SIZES] = { TEXTURE_128_ID, TEXTURE_64_ID, TEXTURE_32_ID, TEXTURE_16_ID};
ushort tmap_sizes[NUM_TEXTURE_SIZES] = { 128, 64, 32, 16 };
bool all_textures=TRUE;

extern uchar tmap_big_buffer[];

// prototypes
bool set_animations(short start,short frames, uchar *anim_used);
errtype load_small_texturemaps(void);
void setup_tmap_bitmaps(void);
void free_textures(void);
errtype load_master_texture_properties(void);
errtype unload_master_texture_properties(void);
errtype clear_texture_properties(void);
errtype texture_crunch_init(void);
errtype texture_crunch_go(void);
void load_textures(void);

extern void FlipShort(short *sh);
extern void FlipLong(long *lng);


#define SET_ANIM_USED(x) anim_used[(x) >> 3] |= (1 << ((x) & 0x7))
#define CHECK_ANIM_USED(x) anim_used[(x) >> 3] & (1 << ((x) & 0x7))

#define ACTUAL_SMALL_ANIMS 101
bool set_animations(short start,short frames, uchar *anim_used)
{
   int loop;
   // Note that NUM_ACTUAL_SMALL_ANIMS contains the actual number of valid
   // animations, BUT anything in the gap between them will get caught by the
   // ResInUse call in load_small_texturemaps
   if ((start<0)||(start+frames>MAX_SMALL_TMAPS))
   {
//      mprintf("PAIN SUFFERING SET ANIM DEATH %d %d\n",start,frames);
      return FALSE;
   }
   for (loop = start; loop < start + frames; loop++)
      SET_ANIM_USED(loop);
   return TRUE;
}

errtype load_small_texturemaps(void)
{
   Id id = TEXTURE_SMALL_ID;
   char i = 0;
   extern bool obj_is_display(int triple);
   int d;
   char rv=0;
   ObjSpecID osid;
   uchar anim_used[MAX_SMALL_TMAPS/8];

   LG_memset(anim_used, 0, MAX_SMALL_TMAPS/8);

   // Figure out which animations are in use
   osid = objBigstuffs[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      if (obj_is_display(ID2TRIP(objSmallstuffs[osid].id)))
      {
         d = objBigstuffs[osid].data2;
         if ((d & TPOLY_INDEX_MASK) && ((d & TPOLY_TYPE_MASK) == 0x100))
            rv=set_animations(d & TPOLY_INDEX_MASK, objBigstuffs[osid].cosmetic_value, anim_used);
         if (((d >> 16) & TPOLY_INDEX_MASK) && ((d & TPOLY_TYPE_MASK) == 0x100))
            rv|=set_animations((d >> 16) & TPOLY_INDEX_MASK, objBigstuffs[osid].cosmetic_value, anim_used);
//         if (!rv) mprintf("Big badness for o %x, osid %x, data2 %d\n",objBigstuffs[osid].id,osid,d);
      }
      osid = objBigstuffs[osid].next;
   }
   AdvanceProgress();

   osid = objSmallstuffs[0].id;
   while (osid != OBJ_SPEC_NULL)
   {
      if (obj_is_display(ID2TRIP(objSmallstuffs[osid].id)))
      {
         d = objSmallstuffs[osid].data2;
         if ((d & TPOLY_INDEX_MASK) && ((d & TPOLY_TYPE_MASK) == 0x100))
            rv = (char) set_animations(d & TPOLY_INDEX_MASK, objSmallstuffs[osid].cosmetic_value, anim_used);
         if (((d >> 16) & TPOLY_INDEX_MASK) && ((d & TPOLY_TYPE_MASK) == 0x100))
            rv |= (char) set_animations((d >> 16) & TPOLY_INDEX_MASK, objSmallstuffs[osid].cosmetic_value, anim_used);
//         if (!rv) mprintf("Small badness for o %x, osid %x, data2 %d\n",objSmallstuffs[osid].id,osid,d);
      }
      osid = objSmallstuffs[osid].next;
   }
   AdvanceProgress();

   while(ResInUse(id+i))
   {
     if (CHECK_ANIM_USED(i))
      {
         ResLock(id+i);
         ResUnlock(id+i);
      }
      i++;
   }
   AdvanceProgress();
   return(OK);
}

// should really do dynamic creation of grs_bitmap *'s for the textures
// but for now, we'll be lame

extern uchar       tmap_static_mem[];
extern uchar      *tmap_dynamic_mem=NULL;

#define get_tmap_128x128(i) ((uchar *)&tmap_dynamic_mem[i*(128*128)])
#define get_tmap_64x64(i)   ((uchar *)&tmap_static_mem[i*SIZE_STATIC_TMAP])
#define get_tmap_32x32(i)   ((uchar *)&tmap_static_mem[(i*SIZE_STATIC_TMAP)+(64*64)])
#define get_tmap_16x16(i)   ((uchar *)&tmap_static_mem[(i*SIZE_STATIC_TMAP)+(64*64)+(32*32)])

// have we built the tables, do we have the extra memory, so on
static bool tmaps_setup=FALSE;


static grs_bitmap tmap_bitmaps[NUM_TEXTURE_SIZES];
void setup_tmap_bitmaps(void)
{
   int i;
   for (i=0; i<4; i++)
	   gr_init_bm(&tmap_bitmaps[i], NULL, BMT_FLAT8, 0, tmap_sizes[i], tmap_sizes[i]);
}

grs_bitmap *get_texture_map(int idx, int sz)
{
   ushort sz_add[NUM_TEXTURE_SIZES-1]={0,64*64,(64*64)+(32*32)};
   uchar *bt;
//   mprintf("Getting tmap %d, sz %d\n",idx,sz);
#ifdef DEMO
   if (sz == 2)
      sz = 1;
#endif
   if (sz==0)
      if (all_textures)
         bt=get_tmap_128x128(idx);
      else
         sz=1;
   if (sz!=0)
   {
         bt=get_tmap_64x64(idx)+sz_add[sz-1];
   }
   tmap_bitmaps[sz].bits=bt;
   return &tmap_bitmaps[sz];
}


void load_textures(void)
{
   grs_bitmap *cur_bm;
   int i, n, c;
   errtype retval = OK;
   int atext_tmp=1;

   {
      if (start_mem < EXTRA_TMAP_THRESHOLD)
         atext_tmp = 0;
      else
      {
         //Spew(DSRC_SYSTEM_Memory, ("Sufficient Memory detected for Loading 128s!\n"));
         atext_tmp = 1;
      }
   }

   //Spew(DSRC_GFX_Texturemaps, ("all_textures = %d\n",all_textures));
   all_textures=atext_tmp;
   //Spew(DSRC_GFX_Texturemaps, ("GAME_TEXTURES = %d\n",GAME_TEXTURES));

   if (!tmaps_setup)
   {
      if (all_textures) // get our butts some memory
         tmap_dynamic_mem = tmap_big_buffer;

      setup_tmap_bitmaps();
      tmaps_setup=TRUE;
   }

   for (c=0; c< NUM_LOADED_TEXTURES; c++)
   {
      i = loved_textures[c];
      if (!ResInUse(TEXTURE_64_ID + i))
      {
         //Warning(("Hey, invalid texture in palette! slot %d = %d\n",c,i));
         i = 0;
      }  // Set local properties
      for (n = SMALLEST_SIZE_INDEX; n<NUM_TEXTURE_SIZES; n++)
      {
         if ((n != TEXTURE_128_INDEX) || all_textures)
         {
#ifdef DEMO
            if (n == TEXTURE_32_INDEX)
               break;
#endif
            cur_bm=get_texture_map(c,n);

            // This is a BLATANT hack to get around the 1 Meg limit in the resource system
            if ((n == TEXTURE_128_INDEX) || (n == TEXTURE_64_INDEX))
            {
               if (ResInUse(tmap_ids[n] + i))
               {
                  retval = load_res_bitmap(cur_bm, MKREF(tmap_ids[n] + i,0), FALSE);
                  cur_bm->flags = 0;
               }
               else
               {
                  //Warning(("Hey, ResInUse failed in tmap_load and i'm so blue (%d,%d,%x)\n",n,i,tmap_ids[n]+i));
                  // should abort !!!
               }
            }
            else
            {
               retval = load_res_bitmap(cur_bm, MKREF(tmap_ids[n],i), FALSE);
               cur_bm->flags = 0;
            }
            if ((cur_bm->w != tmap_sizes[n]) || (cur_bm->h != tmap_sizes[n]))
            {
               //Warning(("Incorrect size in tmap %d! (%d)(%d x %d) vs (%d x %d)\n",i,c,cur_bm->w,
               //   cur_bm->h, tmap_sizes[n], tmap_sizes[n]));
               // should abort !!!
//               DBG(DSRC_GFX_Texturemaps, {
//                              gr_set_fcolor(0);
//                              gr_rect(0,0,320,200);
//                              gr_bitmap(cur_bm, 0, 0);
//               });
            }
         }
	  	 AdvanceProgress();
      }
   }
   // Load in texture properties for all textures
   load_master_texture_properties();
   AdvanceProgress();
   
   // Copy the appropriate things into textprops
   for (i=0; i < NUM_LOADED_TEXTURES; i ++)
      textprops[i] = texture_properties[loved_textures[i]];
   AdvanceProgress();
   
   // Get rid of the big set
   unload_master_texture_properties();
   textures_loaded = TRUE;
   game_fr_reparam(all_textures,-1,-1);
   AdvanceProgress();
}

void free_textures(void)
{
#ifndef SVGA_CUTSCENES
   if (all_textures&&(tmap_dynamic_mem==NULL))
      DisposePtr((Ptr)tmap_dynamic_mem);
#endif
   tmaps_setup=FALSE;
}

errtype bitmap_array_unload(int *num_bitmaps, grs_bitmap *arr[])
{
   int i;

   if (*num_bitmaps == 0)
      return(ERR_NOEFFECT);

   //Spew(DSRC_SYSTEM_Memory, ("Freeing %d bitmaps...\n",*num_bitmaps));
   for (i=0; i<*num_bitmaps; i++)
   {
//      Spew(DSRC_SYSTEM_Memory, ("%d ",i));
      DisposePtr((Ptr)arr[i]->bits);   
      DisposePtr((Ptr)arr[i]);
   }
   *num_bitmaps = 0;
   return(OK);
}

bool empty_bitmap(grs_bitmap *bmp)
{
   uchar *cur=&bmp->bits[0], *targ=cur+(bmp->w*bmp->h);
   while (cur<targ)
      if (*cur++!=0)
         return FALSE;
   return TRUE;
}

errtype Init_Lighting(void)
{
   Handle	res;
   int		i;
   
   // Read in the standard shading table
   res = GetResource('shad',1000);
   if (!res) return(ERR_FOPEN);
   BlockMove(*res, shading_table,(256 * 16));
   ReleaseResource(res);

// MLA- changed this to use resources
/*   char shad_path[255];
   int num_shad,fd,i;

   // Read in the standard shading table
   if (!DatapathFind(&DataDirPath,SHADING_TABLE_FNAME,shad_path))
   {
      //Warning(("Could not find lighting table(%s)!!\n",SHADING_TABLE_FNAME));
      return(ERR_FOPEN);
   }
   fd = open(shad_path, O_RDONLY|O_BINARY);
   if (fd < 0)
   {
      //Warning(("Could not load lighting table(%s)!!\n",shad_path));
      return(ERR_FREAD);
   }
   num_shad = read(fd,shading_table,(256 * 16));
   if (num_shad < (256 * 16))
   {
      //Warning(("Read only %d values from shading_table\n"));
   }
   close(fd);*/
   
   for (i=0; i<256*16; i+=256)
   {
      shading_table[i]=0;  			// i love our shading table
      shading_table[i+255]=0xFF;  	// KLC - so do I
   }
 
   // Now choose this one as our normal shading table   
   gr_set_light_tab(shading_table);

   // now read bw shading table
   res = GetResource('shad',1001);
   if (!res) return(ERR_FOPEN);
   BlockMove(*res, bw_shading_table,(256 * 16));
   ReleaseResource(res);
   
// MLA- changed this to use resources
/*   if (!DatapathFind(&DataDirPath,SHADING_TABLE_BW_FNAME,shad_path))
   {
      //Warning(("Could not find lighting table(%s)!!\n",SHADING_TABLE_BW_FNAME));
      return(ERR_FOPEN);
   }
   fd = open(shad_path, O_RDONLY|O_BINARY);
   if (fd < 0)
   {
      //Warning(("Could not load lighting table(%s)!!\n",shad_path));
      return(ERR_FREAD);
   }
   // how about loading these into an array, not separate symbols?

   num_shad = read(fd,bw_shading_table,(256 * 16));
   if (num_shad < (256 * 16))
   {
      //Warning(("Read only %d values from bw_shading_table\n"));
   }
   close(fd);
   */
   
   for (i=0; i<256*16; i+=256)
      bw_shading_table[i]=0;  // i love our shading table

   fr_set_cluts(shading_table,bw_shading_table,bw_shading_table,bw_shading_table);

   return(OK);
}

errtype load_master_texture_properties(void)
{
   int version,i;
   Handle res;
      
   texture_properties = (TextureProp *)NewPtr(GAME_TEXTURES * sizeof(TextureProp));

   // Load Properties from disk
   clear_texture_properties();
   
   res = GetResource('tprp',1000);
   if (res)
    {
     FlipLong((long *) (*res));
     version =  * (int *) (*res);
     if (version == TEXTPROP_VERSION_NUMBER)
      {
       // copy out the structs, fixing the 11->12 byte size difference
       for (i=0; i<363; i++)
         texture_properties[i] = * (TextureProp *) (*res+4+(i*11));
      	
       // fix shorts in texture_properties
       for (i=0; i<GAME_TEXTURES; i++)
        {
         FlipShort(&texture_properties[i].resilience);
         FlipShort(&texture_properties[i].distance_mod);
        }
      }
	 ReleaseResource(res);
    }
// MLA- changed this to use resources
/*   if (DatapathFind(&DataDirPath, levname, path))
   {
      fd = open(path, O_RDONLY|O_BINARY);
      if (fd != -1)
      {
         sz += READ(fd, version);
         if (version == TEXTPROP_VERSION_NUMBER)
         {
            sz += read(fd, texture_properties, GAME_TEXTURES * 10);
         }
//         else
//            Warning(("Bad Texture Properties version number (%d).  Current = %d.\n",version,TEXTPROP_VERSION_NUMBER));
         close(fd);
      }
    }*/
    
   return(OK);
}

errtype unload_master_texture_properties(void)
{
   DisposePtr((Ptr)texture_properties);
   texture_properties = NULL;
   return(OK);
}

errtype clear_texture_properties(void)
{
   int i;

   // clear it all
   LG_memset(texture_properties, 0, sizeof (TextureProp) * GAME_TEXTURES);

   // set the stuff that needs values besides zero
   for (i=0; i<GAME_TEXTURES; i++)
   {
      texture_properties[i].family_texture = i;
      texture_properties[i].target_texture = i;
      texture_properties[i].resilience = 10;
   }
   return(OK);
}

//#define TEXTURE_CRUNCH_HACK

#ifdef TEXTURE_CRUNCH_HACK

#define NUM_CONVERT  93
short convert_list[NUM_CONVERT][2] = {
{0, 144,},
{3, 115,},
{9, 8,},
{11, 158,},
{13, 7,},
{15, 14},
{16, 158,},
{45, 158,},
{46, 158,},
{47, 158,},
{48, 49,},
{50, 49,},
{51, 49,},
{59, 58,},
{61, 62,},
{67, 66,},
{73, 158,},
{86, 158,},
{87, 158,},
{92, 93,},
{94, 158,},
{98, 158,},
{99, 158,},
{100, 158,},
{105, 158,},
{106, 158,},
{107, 158,},
{109, 158,},
{123, 122,},
{124, 125,},
{128,158,},
{129,158,},
{133, 132,},
{150,158,},
{151,158,},
{152, 158,},
{153,158,},
{154, 158,},
{155,158,},
{156,158,},
{169,167,},
{170, 164,},
{173,160,},
{179,158,},
{180,158,},
{181,158,},
{184,158,},
{185,158,},
{187,158,},
{194,158,},
{197,158,},
{198,158,},
{199,158,},
{200,158,},
{201,158,},
{202,158,},
{203,158,},
{204,158,},
{207,158,},
{211,158,},
{212,205,},
{213,158,},
{239,158,},
{240,241,},
{245,244,},
{247,246,},
{250,249,},
{251,249,},
{254,249,},
{255,249,},
{260,259,},
{261,158,},
{269,266,},
{275,158,},
{276,158,},
{277,266,},
{279,266,},
{282,281,},
{292,158,},
{293,158,},
{295,158,},
{296,294,},
{297,294,},
{298,294,},
{299,294,},
{300,158,},
{301,157,},
{309,158,},
{310,158,},
{311,158,},
{318,158,},
{343,158,},
{351,350,}
};

short tmap_convert[GAME_TEXTURES];
short tmap_crunch[GAME_TEXTURES];

errtype texture_crunch_init(void)
{
   int i,c;
   for (i=0; i < GAME_TEXTURES; i++)
      tmap_convert[i] = i;
   for (i=0; i < NUM_CONVERT; i++)
      tmap_convert[convert_list[i][0]] = convert_list[i][1];
   c = 0;
   for (i=0; i < GAME_TEXTURES; i++)
   {
      if (tmap_convert[i] == i)
         tmap_crunch[i] = c++;
      else
         tmap_crunch[i] = -1;
   }
   return(OK);      
}

errtype texture_crunch_go(void)
{
   int i;
   
   for (i=0; i < NUM_LOADED_TEXTURES; i++)
   {
      //Warning(("%d=%d->%d->%d\n",i,loved_textures[i],tmap_convert[loved_textures[i]],tmap_crunch[tmap_convert[loved_textures[i]]]));
      loved_textures[i] = tmap_crunch[tmap_convert[loved_textures[i]]];
   }
   load_textures();

   return(OK);
}
#endif

//#define TEXTURE_ANNIHILATION

#ifdef TEXTURE_ANNIHILATION
#define NUM_DEMO_TEXTURES 32
#pragma disable_message(202)
bool salvation_list[GAME_TEXTURES];
bool texture_annihilate_func(short keycode, ulong context, void* data)
{
   int fn;
   int i,c;
   extern int texture_fnum;

   mprintf("texture_fnum = %d\n",texture_fnum);
   if (texture_fnum == 0)
   {
      //Warning(("HEY, TEXTURE_FNUM is %d!\n",texture_fnum));
      return(TRUE);
   }

   ResCloseFile(texture_fnum);

   fn = ResEditFile("texture.res", FALSE);

   for (i=0; i < GAME_TEXTURES; i++)
      salvation_list[i] = FALSE;

   // Determine which textures are fine and happy
   for (i=0; i < NUM_DEMO_TEXTURES; i++)
   {
      salvation_list[loved_textures[i]] = TRUE;
   }

   // Annhiliate all that do not conform... except for 16x16s which are in
   // always since it's annoying to remove them!
   for (i=0; i < GAME_TEXTURES; i++)
   {
      if (!salvation_list[i])
      {
         mprintf("destroying number %d, id = %x and %x\n",i,TEXTURE_64_ID + i, TEXTURE_128_ID + i);
         if (!ResInUse(TEXTURE_64_ID + i))
            break;
         else
         {
            ResKill(TEXTURE_64_ID + i);
            ResKill(TEXTURE_128_ID + i);
         }
      }
   }

   ResPack(fn);

   ResCloseFile(fn);

   texture_fnum = ResOpenFile("texture.res");

   return(TRUE);
}
#pragma enable_message(202)
#endif

