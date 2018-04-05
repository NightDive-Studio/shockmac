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
 * FrPts.c
 *
 * $Source: r:/prj/cit/src/RCS/frpts.c $
 * $Revision: 1.3 $
 * $Author: dc $
 * $Date: 1994/09/05 06:43:59 $
 *
 * Citadel Renderer
 *  point allocation, delta list maintenance, so on
 *  
 * $Log: frpts.c $
 * Revision 1.3  1994/09/05  06:43:59  dc
 * diamond pipe, second wrong pipe, various fixes
 * 
 * Revision 1.2  1994/04/21  06:34:09  dc
 * this might fix the wacky memory trash problem
 * 
 * Revision 1.1  1994/01/02  17:12:08  dc
 * Initial revision
 * 
 */

#include <string.h>

#define __FRPTS_SRC
#include "map.h"

#include "frintern.h"
#include "frparams.h"
#include "frflags.h"

#define ptLst(i)     pt_lsts[i]
#define ptLstx(i,x)  (*(ptLst(i)+x))
#define ptRow(i)     pt_rowv[i]
#define ptRowx(i,x)  (*(ptRow(i)+x))

#define pt_set_vec(x,y,z)     _pt_vec.xyz[0]=fix_make(x,0);_pt_vec.xyz[1]=fix_make(0,0); _pt_vec.xyz[2]=fix_make(z,0)
#define pt_mk_point(pt)       pt=g3_transform_point(&_pt_vec)

#ifndef MAP_RESIZING
#define _fr_pt_wid   (fm_x_sz(moose)+1)   // note we secretly know this arg wont be used
static g3s_phandle   pt_lsts[2][_fr_pt_wid];
static ushort        pt_rowv[2][_fr_pt_wid];
#else
static g3s_phandle *pt_lsts[2];
static ushort *pt_rowv[2];
static int    _fr_pt_wid=0;
#endif
g3s_phandle *_fr_ptbase, *_fr_ptnext;  /* global place to get points from */

// i drank so much tea, i wrote my letters in kanji
// round and round the block i walked, pretending you were with me
int fr_pts_frame_start(void)
{
   LG_memset(*(pt_rowv+0),0xffff,_fr_pt_wid*sizeof(ushort));
   LG_memset(*(pt_rowv+1),0xffff,_fr_pt_wid*sizeof(ushort));
   _fr_ret;
}

int fr_pts_freemem(void)
{
#ifdef MAP_RESIZING
   int i;
   if (_fr_pt_wid==0) _fr_ret_val(FR_NO_NEED);
   for (i=0; i<3; i++)
    { Free(*(pt_rowv+i)); Free(*(pt_lsts+i)); }
   _fr_pt_wid=0;
#endif
   _fr_ret;
}

//#pragma disable_message(202)
int fr_pts_resize(int , int )					// x, y
{
#ifdef MAP_RESIZING
   int i;
   fr_pts_freemem();
   _fr_pt_wid=x+1;
   for (i=0; i<3; i++)
   {
      if (((*(pt_rowv+i))=Malloc(x*sizeof(ushort)))==NULL)      _fr_ret_val(FR_NOMEM);
      if (((*(pt_lsts+i))=Malloc(x*sizeof(g3s_phandle)))==NULL) _fr_ret_val(FR_NOMEM);
   }
#endif
   _fr_ret;
}
//#pragma enable_message(202)

// something goofy used by renderer
void dumb_hack_for_now(int x, int y);
void dumb_hack_for_now(int x, int y)
{
   bool tran_sv=FALSE, d=TRUE;
   ushort *_cur_rowv, *_nxt_rowv;
   g3s_phandle *_cur_pt, *_fr_curb, *_fr_curn;
   g3s_vector   _pt_vec;

   _fr_sdbg(NEW_PTS,mprintf("dhon %d %d...",x,y));

   if (pt_rowv[0][x]==y) // go from [0]
      tran_sv=TRUE;
   else if (pt_rowv[1][x]==y)
   {
      tran_sv=TRUE;
      d=FALSE;
   }
   else
   {
      if (*(pt_rowv[0]+x)==0xffff) // create a new one
      {
         pt_set_vec(x,0,y);
         pt_mk_point(ptLstx(0,x));
         _fr_sdbg(NEW_PTS,mprintf("New pt... "));
      }
      else
      {
         fix df=fix_make(y-*(pt_rowv[0]+x),0);
         _cur_pt=ptLst(0)+x;
         g3_add_delta_z(*_cur_pt,df);
         _fr_sdbg(NEW_PTS,mprintf("Move pt by %x at %d last %d ",df,x,*(pt_rowv[0]+x)));
      }
   }
   _fr_sdbg(NEW_PTS,if (tran_sv) mprintf("Found %d...",d));

   _fr_ptbase=ptLst(d?0:1); _cur_rowv=ptRow(d?0:1)+x;
   _fr_ptnext=ptLst(d?1:0); _nxt_rowv=ptRow(d?1:0)+x;
   _fr_curb=_fr_ptbase+x;
   _fr_curn=_fr_ptnext+x;
   _fr_sdbg(NEW_PTS,mprintf("rose %d %d %d %d...",*_cur_rowv,*(_cur_rowv+1),*_nxt_rowv,*(_nxt_rowv+1)));

   // now *_fr_ptbase is at (x,y)
   if (*(_cur_rowv+1)!=y)
      if (*(_cur_rowv+1)!=0xffff)
       { g3_replace_add_delta_x(*_fr_curb,*(_fr_curb+1),fix_make(1,0)); _fr_sdbg(NEW_PTS,mprintf("x+1 replace")); }
      else
       { *(_fr_curb+1)=g3_copy_add_delta_x(*_fr_curb,fix_make(1,0)); _fr_sdbg(NEW_PTS,mprintf("x+1 copy")); }

   if (*(_nxt_rowv)!=y+1)
      if (*(_nxt_rowv)!=0xffff)
       { g3_replace_add_delta_z(*_fr_curb,*_fr_curn,fix_make(1,0)); _fr_sdbg(NEW_PTS,mprintf("y replace")); }
      else
       { *_fr_curn=g3_copy_add_delta_z(*_fr_curb,fix_make(1,0));_fr_sdbg(NEW_PTS,mprintf("y copy")); }

   if (*(_nxt_rowv+1)!=y+1)
      if (*(_nxt_rowv+1)!=0xffff)
       { g3_replace_add_delta_x(*_fr_curn,*(_fr_curn+1),fix_make(1,0)); _fr_sdbg(NEW_PTS,mprintf("y+1 replace")); }
      else
       { *(_fr_curn+1)=g3_copy_add_delta_x(*_fr_curn,fix_make(1,0)); _fr_sdbg(NEW_PTS,mprintf("y+1 copy")); }

   *_cur_rowv=y;   *(_cur_rowv+1)=y;
   *_nxt_rowv=y+1; *(_nxt_rowv+1)=y+1;

   _fr_sdbg(NEW_PTS,mprintf("\n"));
}


