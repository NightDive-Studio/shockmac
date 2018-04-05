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
 * GameSort.c
 *
 * $Source: r:/prj/cit/src/RCS/gamesort.c $
 * $Revision: 1.10 $
 * $Author: dc $
 * $Date: 1994/08/24 06:19:35 $
 *
 * Game system object sorting
 */

#include <stdlib.h>

#include "map.h"
#include "objects.h"
#include "objsim.h"
#include "objprop.h"
#include "objclass.h"
#include "otrip.h"
#include "frcamera.h"
#include "gameobj.h"
#include "gamesort.h"

//#include <frintern.h>
//#include <frparams.h>
//#include <frflags.h>

//#include <mprintf.h>

// want outrageous mprintf's? of course you do!
//#define SORT_SPEW

extern void show_obj(ObjID curO);

// Internal Prototypes
void sort_section(int low, int hi);
void score_objs(int o_num);
int do_part_sort(int ptype, int lo, int hi, int ploc);
void partition_sort(void);
void render_sorted_objs(void);


#define MAX_SORTED_REFS 64

#define PRT_NONE        0
#define PRT_HORIZ       0x20
#define PRT_VERT        0x40
#define PRT_BOTH        0x60
#define PRT_MASK        0x60

// this just isnt going to work
#define SCORE_SHIFT       8
#define SCORE_HALF_ENTRY  0x80
#define SCORE_REFIDX_MASK 0x1F
#define SCORE_SCORE_MASK  0xFFFFFF00

// public, rendtool sets this for camera controls
ObjID no_render_obj=-1;

static ObjID    sq_Refs[MAX_SORTED_REFS];
static uint     score_list[MAX_SORTED_REFS];        // at first holds obj_type8.objtrip24, then objscore24.exp1.partition2.refidx5
                                                    // thus we can sort it w/o losing the refidx, which looks up in sqrefs
static ushort   partition_loc[MAX_SORTED_REFS];     // for the partitions

static uchar    partition_type;
static uchar    partition_cnt;

static uint     cur_obj_num, draw_last_cnt;
static uint     osort_zc, osort_yc, osort_xc;			// KLC - changed order for our compiler (it puts them in reverse order)

// set up data space, 
void render_sort_start(void)
{
   cur_obj_num=0;
   osort_xc=fr_camera_last[0]>>8;
   osort_yc=fr_camera_last[1]>>8;
   osort_zc=fr_camera_last[2]>>(16-SLOPE_SHIFT-3);    // oh yea
}

void sort_section(int low, int hi)
{
	int iloop, oloop;
	uint tmp;

	for (oloop=hi-1; oloop>=low; oloop--)
		for (iloop=low; iloop<=oloop; iloop++)
         if (score_list[iloop]<score_list[iloop+1])
         {
				tmp=score_list[iloop];
				score_list[iloop]=score_list[iloop+1];
				score_list[iloop+1]=tmp;
			}
}

void score_objs(int o_num)
{
   ObjID cobjid;
   short objtrip;
   Obj *_os_cobj;
   extern uchar cam_mode;
   int partition=PRT_NONE, obj_type, our_score;

   cobjid=sq_Refs[o_num];
   obj_type=score_list[o_num]>>24;
   objtrip=score_list[o_num]&0xffffff;
   _os_cobj=&objs[cobjid];
   switch (obj_type)
   {
   case FAUBJ_SPECIAL:
      switch(ID2TRIP(cobjid))
      {
	   case TRIPBEAM_TRIPLE:
         partition=PRT_HORIZ;
         break;
      case FORCE_BRIJ_TRIPLE:
      case BRIDGE_TRIPLE:
      case CATWALK_TRIPLE:
         partition=PRT_VERT;
         break;
      }
      break;
   case FAUBJ_TL_POLY:
   case FAUBJ_TEXBITMAP:
   case FAUBJ_TPOLY:
      if (_os_cobj->obclass==CLASS_DOOR)
         if (((_os_cobj->loc.p+0x20)&0x7f)<0x40)
		      partition=PRT_HORIZ;
         else
            partition=PRT_VERT;
      else
         partition=-1;        // secret i am a freebie code
      break;
   }
   our_score=
      abs(osort_xc-_os_cobj->loc.x)+
      abs(osort_yc-_os_cobj->loc.y)+
      abs(osort_zc-_os_cobj->loc.z);
   our_score<<=8;    // 24 bits of score, should do for now
   // if bloodstain, want to | 0x80 here...
   if (partition>0)
   {
      our_score|=partition|o_num;
      partition_loc[partition_cnt]=o_num;
      partition_cnt++;
      partition_type|=partition;
   }   
   else if (partition<0)
   {
      int tmp=our_score|o_num;
      if (draw_last_cnt!=o_num)
      {
	      our_score=score_list[draw_last_cnt];
         if (our_score&PRT_MASK)
         {
            int i=0;
            while (i<partition_cnt)
               if (partition_loc[i]==draw_last_cnt)
                { partition_loc[i]=o_num; break; }
            if (i==partition_cnt)
               Warning(("lost my partition"));
         }
	      score_list[draw_last_cnt]=tmp;
      }
      else
      {
         our_score=tmp;
      }
      draw_last_cnt++;
   }
   else
   {
      our_score|=o_num;
   }
   score_list[o_num]=our_score;
}

#define get_part_val(tval,ob,which) \
   switch (which) \
   { \
      case 0: tval=ob->loc.x; break; \
      case 1: tval=ob->loc.y; break; \
      case 2: tval=ob->loc.z; break; \
   }

int do_part_sort(int ptype, int lo, int hi, int ploc)
{
   Obj *cur_obj, *part_obj;
   int  cur_val,  part_val, pdir, cam_val, near_f, loidx, hiidx;
	int  ptptr[2], ptdelta[2], tmpstore=-1, mloc;

   part_obj=&objs[sq_Refs[ploc]];
   pdir=(ptype==PRT_VERT)?2:(part_obj->loc.h&0x40)?0:1;
   get_part_val(part_val,part_obj,pdir);
   cam_val=*((&osort_xc)+pdir);
   near_f=(cam_val<part_val);

   if ((lo<=ploc)&&(ploc<hi))
   {
      tmpstore=score_list[ploc];
      if (ploc<--hi)
         score_list[ploc]=score_list[hi];
   }
   else
      Warning(("Partition not in list"));

	if (near_f)
	 { ptptr[0] = hi-1; ptdelta[0] = -1; ptptr[1] =   lo;	ptdelta[1] =  1; loidx=1; hiidx=0; }
	else
	 { ptptr[0] =   lo; ptdelta[0] =  1; ptptr[1] = hi-1; ptdelta[1] = -1; loidx=0; hiidx=1; }

   // ok, this checks in a massively gruesomely redundant way, rewrite for real in asm
   while (ptptr[loidx]<=ptptr[hiidx])
   {
	   cur_obj=&objs[sq_Refs[score_list[ptptr[0]]&SCORE_REFIDX_MASK]];
	   get_part_val(cur_val,cur_obj,pdir);
      while (cur_val<part_val)
      {
         ptptr[0]+=ptdelta[0];
         if (ptptr[loidx]>ptptr[hiidx])
            goto done_psort;
		   cur_obj=&objs[sq_Refs[score_list[ptptr[0]]&SCORE_REFIDX_MASK]];
	      get_part_val(cur_val,cur_obj,pdir);
      }

      cur_obj=&objs[sq_Refs[score_list[ptptr[1]]&SCORE_REFIDX_MASK]];
      get_part_val(cur_val,cur_obj,pdir);
      while (cur_val>part_val)
	   {
         ptptr[1]+=ptdelta[1];
         if (ptptr[loidx]>ptptr[hiidx])
            goto done_psort;
		   cur_obj=&objs[sq_Refs[score_list[ptptr[1]]&SCORE_REFIDX_MASK]];
	      get_part_val(cur_val,cur_obj,pdir);
      }

      {
         int tmpptr=score_list[ptptr[0]];
         score_list[ptptr[0]]=score_list[ptptr[1]];
         score_list[ptptr[1]]=tmpptr;
         ptptr[0]+=ptdelta[0];      // now move on in
         ptptr[1]+=ptdelta[1];
      }
   }

done_psort:

   mloc=ptptr[loidx];
   if (tmpstore!=-1)
    { score_list[hi++]=score_list[mloc]; score_list[mloc]=tmpstore; }

   return mloc;
}

void partition_sort(void)
{
   if ((partition_cnt>1)||(partition_type==PRT_BOTH))
;//KLC      Warning(("Dual partitions\n"));
   else
   {
      int mloc;
#ifdef COW_COW
      if (cur_obj_num==partition_loc[0])
         sort_section(draw_last_cnt,cur_obj_num-1);
      else if (draw_last_cnt==partition_loc[0])
         sort_section(draw_last_cnt+1,cur_obj_num);
      else
#endif // __FEAR__ the COW COW
      {
	      mloc=do_part_sort(partition_type,draw_last_cnt,cur_obj_num,partition_loc[0]);
	      sort_section(draw_last_cnt,mloc-1);
	      sort_section(mloc+1,cur_obj_num-1);
      }
   }
}

// go through, do the sort, call show_obj a lot
// for now, just do oscore straight...
void render_sorted_objs(void)
{
   extern int _fdt_x, _fdt_y;
   int i;
   partition_cnt=partition_type=draw_last_cnt=0;
   if (cur_obj_num>1)
   {
	   for (i=0; i<cur_obj_num; i++)
         score_objs(i);
      if (partition_cnt)
         partition_sort();
      else
		   sort_section(draw_last_cnt,cur_obj_num-1);
	   for (i=0; i<cur_obj_num; i++)
	   	   show_obj(sq_Refs[score_list[i]&SCORE_REFIDX_MASK]);
   }
   else if (cur_obj_num==1)
      show_obj(sq_Refs[0]);
   cur_obj_num=0;
}

void sort_show_obj(ObjID cobjid)
{
   short objtrip;
   int obj_type;

   if (cur_obj_num>=MAX_SORTED_REFS) return;
   if (cobjid==no_render_obj) return;
   objtrip=OPNUM(cobjid);
   obj_type=ObjProps[objtrip].render_type;
   if (obj_type==FAUBJ_NOOBJ) return;
   score_list[cur_obj_num]=(obj_type<<24)+objtrip;
   sq_Refs[cur_obj_num]=cobjid;
   cur_obj_num++;
}
