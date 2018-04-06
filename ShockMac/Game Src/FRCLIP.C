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
 * FrClip.c
 *
 * $Source: r:/prj/cit/src/RCS/frclip.c $
 * $Revision: 1.12 $
 * $Author: dc $
 * $Date: 1994/09/10 00:30:09 $
 *
 * Citadel Renderer
 *  various clippers for terrain, including basic cone clip and the later day
 *   tile based clipper
 *  
 * $Log: frclip.c $
 * Revision 1.12  1994/09/10  00:30:09  dc
 * this time i really think i fixed the malloc's
 * 
 * Revision 1.11  1994/09/06  03:14:58  dc
 * do cspace a little more correctly, eh?
 * 
 * Revision 1.10  1994/09/05  08:33:35  dc
 * hey how about not forgetting about cyberspace this time, eh?
 * 
 * Revision 1.9  1994/09/05  06:43:26  dc
 * span parse fixes, clear as we go
 * 
 * Revision 1.8  1994/08/30  05:51:01  dc
 * fix vanish if home square not drawn bug, othe rstuff
 * 
 * Revision 1.7  1994/08/21  03:10:06  dc
 * duh.
 * 
 * Revision 1.6  1994/08/21  03:06:55  dc
 * parameterize spawn_check, save a bit of code space
 * 
 * Revision 1.5  1994/07/28  05:53:42  dc
 * protection from span clip nightmare, what is cone doing...
 * 
 * Revision 1.4  1994/03/13  17:18:04  dc
 * doors take 38, still doesnt do minimal inclusion clips right
 * 
 * Revision 1.3  1994/03/10  00:11:03  dc
 * better door stuff, still need some stuff, though....
 * 
 * Revision 1.2  1994/01/22  18:57:39  dc
 * fix objclip to set correct bits of subclip, next switch it to flick_qclip
 */

#define __FRCLIP_SRC

#include <stdlib.h>
#include <string.h>

#include "frintern.h"
#include "frspans.h"
#include "frtables.h"
#include "fr3d.h"
#include "frquad.h"
#include "frflags.h"
#include "frparams.h"
#include "frsubclp.h"
#include "frshipm.h"

#include "cone.h"

#include "map.h"
#include "mapflags.h"
#include "tilename.h"

// pre prototyping
static void   _fr_rebuild_nVecWork(void);
static void   _fr_init_vecwork(void);
static void    clear_clip_bits(void);

// for the subtile clipper
ushort  sc_reg[NUM_SUBCLIP][SC_VEC_COUNT];
ushort *cur_sc_ptr;
uint    cur_sc_reg;

// Prototypes
void _fr_sclip_line(MapElem *sp_base, int len, int val);
void _fr_sclip_line_check_solid(MapElem *sp_base, int len, int val);
void fr_span_parse(void);
void span_fixup(void);
int fr_clip_show_all(void);
bool _fr_move_ccv_x(struct _nVecWork *nvp);
void _fr_move_along_dcode(int dircode);


#ifndef MAP_RESIZING
static uchar real_x_spans[(1<<DEFAULT_YSHF)*SPAN_MEM];
static uchar real_cone_spans[(1<<DEFAULT_YSHF)*2];
#endif

int fr_clip_freemem(void)
{
   if (x_span_lists!=NULL) DisposePtr((Ptr)x_span_lists);
   _fr_ret;
}

int fr_clip_resize(int ,int )				// x, y
{
   int i;
#ifdef MAP_RESIZING
   if (x_span_lists!=NULL) Free(x_span_lists);
   if (cone_span_list!=NULL) Free(cone_span_list);
   x_span_lists=(uchar *)Malloc(y*SPAN_MEM*sizeof(uchar));
   cone_span_list=(uchar *)Malloc(y*2*sizeof(uchar));
#else
   x_span_lists=&real_x_spans[0];
   cone_span_list=&real_cone_spans[0];
#endif
   _fr_rebuild_nVecWork();
   _fr_init_vecwork();
   _fr_dbg(if (x_span_lists==NULL) _fr_ret_val(FR_NOMEM));
   _fr_dbg(if (cone_span_list==NULL) _fr_ret_val(FR_NOMEM));
   for (i=0; i<fr_map_y; i++) span_count(i)=0;
   _fr_ret;
}

int fr_clip_frame_start(void)
{
// setup real span lists
// setup real obj stack
//   _fr_init_vecwork();
   // hmm... is this really necessary????
   LG_memset(cone_span_list,0xff,fr_map_y*2*sizeof(uchar));
   _fr_sdbg(SANITY,_fr_init_vecwork());   // hey, why not?
   _fr_ret;
}

int fr_clip_frame_end(void)
{
#ifndef CLEAR_AS_WE_GO
   clear_clip_bits();
#endif
   _fr_ret;
}

// cradle every word the falls
// into the arms of failure
void store_x_span(int y, int lx, int rx)
{
   int c_span;

   c_span= span_count(y)++;
   if (c_span>=MAX_SPANS)
   {
//      Warning(("HEY too many spans!!! y %d from %d to %d\n",y,lx,rx));  // should probably solve the problem
      span_count(y)--; c_span--;
      span_right(y,c_span)=rx;               // just add a new right edge
   }
   else
   {
	   span_left(y,c_span) =lx;
	   span_right(y,c_span)=rx;
   }
   _fr_sdbg(VECSPEW,mprintf("Put %d->%d at span %d of %d\n",lx,rx,c_span,y));

#ifdef _FR_TILEMAP
   if (fr_highlights)
   {
      LGPoint p;
      for (p.x=lx,p.y=y; p.x<=rx; p.x++)
         TileMapSetHighlight(NULL,p,0,TRUE);
   }
#endif // _FR_TILEMAP
}

void _fr_sclip_line(MapElem *sp_base, int len, int val)
{
   _fr_sdbg(SPAN_PARSE,mprintf("yep, at %x, len %x, val %x\n",sp_base,len,val));
   for (;len>0; len--, sp_base++)
      me_subclip(sp_base)=val;
}

void _fr_sclip_line_check_solid(MapElem *sp_base, int len, int val)
{
   _fr_sdbg(SPAN_PARSE,mprintf("yep, at %x, len %x, val %x\n",sp_base,len,val));
   for (;len>0; len--, sp_base++)
      if (me_tiletype(sp_base)==TILE_SOLID)
         me_subclip(sp_base)=SUBCLIP_OUT_OF_CONE;
      else
         me_subclip(sp_base)=val;

}

// compute bounding box as well
extern MapElem *fr_map_base;
ushort frpipe_dist;
void fr_span_parse(void)
{
   int y, dist;
   MapElem *cur_span=fr_map_base;
   uchar *cur_span_cnt=&(span_count(0)), *cur_cone_span=&cone_span_list[0];

   frpipe_dist=0;
   for (y=0; y<fr_map_y; y++, cur_span+=fr_map_x, cur_span_cnt+=(1<<SPAN_SHIFT), cur_cone_span+=2)
      if (*cur_span_cnt)
      {
         if ((*cur_span_cnt)==1)
	      {
            _fr_sdbg(SPAN_PARSE,mprintf("sc 1 at %x, going from %x to %x and %x to %x\n",y,*cur_cone_span,span_left(y,0),span_right(y,0)+1,*(cur_cone_span+1)));
            _fr_sclip_line(cur_span+(*cur_cone_span),span_left(y,0)-*cur_cone_span,SUBCLIP_OUT_OF_CONE);
            _fr_sclip_line(cur_span+span_right(y,0)+1,(*(cur_cone_span+1))-(span_right(y,0)+1)+1,SUBCLIP_OUT_OF_CONE);
	      }
	      else
	      {  // first merge wacky scenes....
            int cur_l, cur_r, span_id=0, off_l;
            _fr_sdbg(SPAN_PARSE,for (cur_l=0; cur_l<*cur_span_cnt; cur_l++) mprintf("at %x MultiSpan %d from %x to %x\n",y,cur_l,span_left(y,cur_l),span_right(y,cur_l)));
            cur_l=span_left(y,0); cur_r=span_right(y,0); off_l=*cur_cone_span;
            while (++span_id<(*cur_span_cnt))
            {
               if (span_left(y,span_id)<=cur_r)
               {  // merge the spans into 1, but sclip clean any overlap pass first
                  _fr_sdbg(SPAN_PARSE,mprintf("at %x Cleanup from %x to %x\n",y,span_left(y,span_id),cur_r));
                  _fr_sclip_line(cur_span+span_left(y,span_id),cur_r-span_left(y,span_id)+1,SUBCLIP_FULL_TILE); // set them to no subclip
                  if (cur_r<span_right(y,span_id))
                  {
                     _fr_sdbg(SPAN_PARSE,mprintf("span merge right to %x from %x\n",span_right(y,span_id),cur_r));
                     cur_r=span_right(y,span_id);
                  }
               }
               else
               {
                  _fr_sdbg(SPAN_PARSE,mprintf("sent old span %x from %x to %x\n",y,off_l,cur_l));
                  _fr_sclip_line(cur_span+off_l,cur_l-off_l,SUBCLIP_OUT_OF_CONE);
                  off_l=cur_r+1; cur_l=span_left(y,span_id); cur_r=span_right(y,span_id);
               }
            }
            _fr_sdbg(SPAN_PARSE,mprintf("loop done at %x, now sending %x -> %x and %x -> %x\n",y,off_l,cur_l,cur_r+1,*(cur_cone_span+1)));
            _fr_sclip_line(cur_span+off_l,cur_l-off_l,SUBCLIP_OUT_OF_CONE);
            _fr_sclip_line(cur_span+cur_r+1,(*(cur_cone_span+1))-(cur_r+1)+1,SUBCLIP_OUT_OF_CONE);
	      }
         dist=abs(y-_fr_y_cen);
         if ((dist+abs(span_left(y,0)-_fr_x_cen))>frpipe_dist)
            frpipe_dist=dist+abs(span_left(y,0)-_fr_x_cen);
         if ((dist+abs(span_right(y,(*cur_span_cnt)-1)-_fr_x_cen))>frpipe_dist)
            frpipe_dist=dist+abs(span_right(y,(*cur_span_cnt)-1)-_fr_x_cen);
#ifdef CLEAR_AS_WE_GO
         *cur_span_cnt=0;
#endif
      }
      else if (*cur_cone_span!=0xff)
      {
         _fr_sdbg(SPAN_PARSE,mprintf("Cleaning up %x from %x to %x\n",y,*cur_cone_span,*(cur_cone_span+1)));
         _fr_sclip_line(cur_span+(*cur_cone_span),(*(cur_cone_span+1))-(*(cur_cone_span))+1,SUBCLIP_OUT_OF_CONE);
      }                       // note the ()'s are right, it is (right edge) - (left edge) + 1
}

void span_fixup(void)
{
   _fr_sdbg(VECSPEW,mprintf("Center was %d spans, %d -> %d and %d -> %d\n",span_count(_fr_y_cen),span_left(_fr_y_cen,0),span_right(_fr_y_cen,0),span_left(_fr_y_cen,1),span_right(_fr_y_cen,1)));
   if (span_count(_fr_y_cen)>1)
   {
	   span_count(_fr_y_cen)--;
	   span_left(_fr_y_cen,0) =min(span_left(_fr_y_cen,0),span_left(_fr_y_cen,1));
	   span_right(_fr_y_cen,0)=max(span_right(_fr_y_cen,0),span_right(_fr_y_cen,1));
   }
   else
      Warning(("Only one span at y center\n"));
   fr_span_parse();
#if _fr_defdbg(VECSPEW)
   if (_fr_dbgflg_chk(VECSPEW))
	{
	   int i,j,lc;
	   mprintf("At %d %d\n",_fr_x_cen,_fr_y_cen);
	   for (i=0, lc= -1; i<fr_map_y; i++)
	   {
	      if (span_count(i)==0)
	       { if (lc==-1) lc=i; }
	      else
	      {
	         if (lc!=-1)
	          { mprintf("y %d-%d have none\n",lc,i); lc=-1; }
		      mprintf("y%d c%d: ",i,span_count(i));
		      for (j=0; j<span_count(i); j++)
		         mprintf(" %d->%d",span_left(i,j),span_right(i,j));
		      mprintf("\n");
		   }
	   }
	   if (lc!=-1)
	      mprintf("y %d-%d have none\n",lc,i-1);
   }
#endif
}

#define STAY_ON_MAP
void cone_span_set(int y,int l,int r)
 { cone_span_list[y+y]=l; cone_span_list[y+y+1]=r; }

#ifndef CLEAR_AS_WE_GO
static void clear_clip_bits(void)
{
   int i,j;
   MapElem *mbptr=MAP_MAP, *mptr;
   for (i=0; i<fr_map_y; i++, mbptr+=fr_map_x)
      if (cone_span_left(i)!=0xff)
	      for (j=cone_span_left(i),mptr=mbptr+j;j<=cone_span_right(i);j++,mptr++)
            _me_subclip(mptr)=SUBCLIP_OUT_OF_CONE;
}
#endif

static void set_clip_bits(void)
{
   int i;
   MapElem *mbptr=MAP_MAP, *mptr, *rptr;
   for (i=0; i<fr_map_y; i++, mbptr+=fr_map_x)
      if (cone_span_left(i)!=0xff)
	      for (mptr=mbptr+cone_span_left(i),rptr=mbptr+cone_span_right(i);mptr<=rptr;mptr++)
            _me_subclip(mptr)=SUBCLIP_FULL_TILE;
}

#if _fr_defdbg(NO_CONE)
void set_full_cone(void)
{
}
#endif

// satan got her tongue
// now, it's undone
int fr_clip_cone(void)
{
   simple_cone_clip_pass();
//   _fr_ndbg(NO_CONE,simple_cone_clip_pass());
//   _fr_sdbg(NO_CONE,set_full_cone());
   set_clip_bits();

#if _fr_defdbg(VECSPEW)
   if (_fr_dbgflg_chk(VECSPEW))
	{
	   int i,lc;
	   mprintf("Cone from %d %d\n",_fr_x_cen,_fr_y_cen);
	   for (i=0, lc= -1; i<fr_map_y; i++)
	   {
         if (cone_span_left(i)==0xff)
	       { if (lc==-1) lc=i; }
	      else
	      {
	         if (lc!=-1)
	          { mprintf("y %d-%d have none\n",lc,i); lc=-1; }
		      mprintf("y%d: %d->%d\n",i,cone_span_left(i),cone_span_right(i));
		   }
	   }
	   if (lc!=-1)
	      mprintf("y %d-%d have none\n",lc,i-1);
      mprintf("left (%x,%x), right (%x,%x)\n",span_intersect[0],span_intersect[1],span_intersect[2],span_intersect[3]);
      mprintf("vec tl (%x,%x), vec tr (%x,%x)\n",
         span_lines[2],span_lines[3],     
         span_lines[4],span_lines[5]);    
      mprintf("vec bl (%x,%x), vec br (%x,%x)\n",
         span_lines[0],span_lines[1],
         span_lines[6],span_lines[7]);
      mprintf("Note eye @ %x %x, ang %x %x %x\n",coor(EYE_X),coor(EYE_Y),coor(EYE_H),coor(EYE_P),coor(EYE_B));
   }
#endif
   _fr_ret;
}

// these two are a little unstoked at the moment
int fr_clip_show_all(void)
{
   MapElem *cur_span=fr_map_base;
#ifndef REALLY_ALL
   uchar *cur_cone_span=&cone_span_left(0);
   int i, dist;
   for (i=0; i<fr_map_y; i++, cur_span+=fr_map_x, cur_cone_span+=2)
      if ((*cur_cone_span)!=0xff)
      {
#ifndef CLEAR_AS_WE_GO
         store_x_span(i,*cur_cone_span,*(cur_cone_span+1));
#endif
         _fr_sclip_line_check_solid(cur_span+(*cur_cone_span),*(cur_cone_span+1)-*(cur_cone_span)+1,SUBCLIP_FULL_TILE);
         dist=abs(i-_fr_y_cen);
         if ((dist+abs(*cur_cone_span-_fr_x_cen))>frpipe_dist)
            frpipe_dist=dist+abs(*cur_cone_span-_fr_x_cen);
         if ((dist+abs(*(cur_cone_span+1)-_fr_x_cen))>frpipe_dist)
            frpipe_dist=dist+abs(*(cur_cone_span-1)-_fr_x_cen);
      }
#else
   int y;
   for (y=0; y<fr_map_y; y++)
   {
#ifndef CLEAR_AS_WE_GO
      store_x_span(y,0,fr_map_x-1);
#endif
      _fr_sclip_line_check_solid(cur_span+(*cur_cone_span),*(cur_cone_span+1)-*(cur_cone_span)+1,SUBCLIP_FULL_TILE);
   }
   if (_fr_x_cen<(fr_map_x>>1)) dist=fr_map_x-_fr_x_cen; else dist=_fr_x_cen-fr_map_x;
   if (_fr_y_cen<(fr_map_y>>1)) dist+=fr_map_y-_fr_y_cen; else dist=_fr_y_cen-fr_map_y;
#endif
   _fr_ret;
}

// when i hear the word security, i reach for my shotgun
typedef struct {
   fix loc[3];          // current location of vector
   fix deltas[3];       // current steps for vector
   MapElem *mptr;       // current map pointer
   uchar flags;         // inuse, LR, pointer to self
   uchar nxtv;          // for now, points to next
   fix oldx;            // x at last y crossing
   fix len;             // current vector length
} FrClipVec;

struct _nVecWork {
   fix remx;            // how far to go in x
   short mapstep[2];    // how to move in map when going by x, y
   uchar inface[2];     // inface for x and y
   fix stepy;           // how far to go in y for a remx
   uchar move_x;        // do we move in x?
   uchar dircode;       // so we can get back out
};

#define nVW_XDIR 2
#define nVW_YDIR 1

#define nVW_NXNY 0
#define nVW_NXPY 1
#define nVW_PXNY 2
#define nVW_PXPY 3

#define _fixp1 ( fix_make(1,0))
#define _fixn1 (-fix_make(1,0))
static fix locstep[4][2]={{_fixn1,_fixn1},{_fixn1,_fixp1},{_fixp1,_fixn1},{_fixp1,_fixp1}};
static fix edgestep[4][2]={{0,0},{0,_fixp1-1},{_fixp1-1,0},{_fixp1-1,_fixp1-1}};
static char rmmod[4]={-1,-1,1,1};             /* mod to rem to get to next full square */
static short new_del_mapstep[2]={-32,32};     /* wants to be wall_adds[2] and then wall_adds[0] */
static MapElem *eye_mptr;

#define FRVECSELF  0x3F
#define FRVECMASK  0x3F
#define FRVECUSE   0x80
#define FRVECL     0x40
#define FRVECR     0x00
#define FRVECDIR   0x40

#define MS_X       0
#define MS_Y       1

#define MAX_CLIP_VEC 16
static FrClipVec allclipv[MAX_CLIP_VEC];
static FrClipVec *ccv;
static char vechead=0, ffreevec=0, lastvec=-1, endvec=MAX_CLIP_VEC-1;

struct _nVecWork _nVP[4]=
{
  {0xdeadbeef,-1,-64,3,2,0xdeadbeef,0xff,nVW_NXNY}, {0xdeadbeef,-1, 64,3,0,0xdeadbeef,0xff,nVW_NXPY},
  {0xdeadbeef, 1,-64,1,2,0xdeadbeef,0xff,nVW_PXNY}, {0xdeadbeef, 1, 64,1,0,0xdeadbeef,0xff,nVW_PXPY}
};

// this is a gross way to do this...
#define _fr_build_clip_vec(cv, org, ray, aflags) \
 (cv)->len=0; (cv)->flags=((cv)->flags&FRVECMASK)+aflags; \
 (cv)->mptr=MAP_GET_XY(fix_int(org[0]),fix_int(org[1])); \
 (cv)->loc[0]=org[0]; (cv)->loc[1]=org[1]; (cv)->loc[2]=org[2]; cv->oldx=org[0]; \
 (cv)->deltas[0]=ray[0]; (cv)->deltas[1]=ray[1]; (cv)->deltas[2]=ray[2]

// More prototypes
bool _fr_skip_solid_right_n_back(FrClipVec *cv, fix max_loc, int y_map_step);
bool _fr_skip_space_right_n_back(FrClipVec *cv, fix max_loc, int y_map_step);
bool _fr_skip_solid_left_n_back(FrClipVec *cv, fix min_loc, int y_map_step);
void _fr_del_compute(FrClipVec *v1, FrClipVec *v2);
void _fr_spawn_check_one(FrClipVec *lv, FrClipVec *rv, bool northward);
bool _fr_move_new_dels(FrClipVec *lv, FrClipVec *rv, bool northward);
void _fr_kill_pair(FrClipVec *lv,FrClipVec *rv);
bool _fr_setup_first_pair(bool headnorth);


// these dont really work, the *org, *ray doesnt move all 3
// (*((cv)->loc))=*org; (*((cv)->deltas))=*ray;
// this gets a missing lvalue stupidity
// (cv)->loc=org; (cv)->deltas=ray;

static void _fr_rebuild_nVecWork(void)
{
   _nVP[nVW_NXNY].mapstep[MS_X]=_nVP[nVW_NXPY].mapstep[MS_X]=wall_adds[3];
   _nVP[nVW_PXNY].mapstep[MS_X]=_nVP[nVW_PXPY].mapstep[MS_X]=wall_adds[1];
   _nVP[nVW_NXNY].mapstep[MS_Y]=_nVP[nVW_PXNY].mapstep[MS_Y]=wall_adds[2];
   _nVP[nVW_NXPY].mapstep[MS_Y]=_nVP[nVW_PXPY].mapstep[MS_Y]=wall_adds[0];
   new_del_mapstep[0]=wall_adds[2]; new_del_mapstep[1]=wall_adds[0];
}

static void _fr_init_vecwork(void)
{
   int i;
   for (i=0; i<MAX_CLIP_VEC; i++)      // point at self and next
    { allclipv[i].flags=i; allclipv[i].nxtv=(i+1)&(MAX_CLIP_VEC-1); }
   vechead=0; ffreevec=0; lastvec=-1; endvec=MAX_CLIP_VEC-1;
}

#if _fr_defdbg(VECSPEW)
void print_nvp(struct _nVecWork *nvp)
{
   static char *nvm_str[4]={"nxny","nxpy","pxny","pxpy"};
   static char ifc_str[4]={'n','e','s','w'};
   mprintf(" _nVP xrem %x stepy %x. move_x %d std %s %c%c %d %d\n",
      nvp->remx,nvp->stepy,nvp->move_x,
      nvm_str[nvp->dircode],ifc_str[nvp->inface[0]],ifc_str[nvp->inface[1]],
      nvp->mapstep[0],nvp->mapstep[1]);
}

void print_fcv(FrClipVec *_ccv, int dim)
{
   if (dim==3)
	   mprintf(" _ccv @(%x,%x,%x),d(%x,%x,%x)..",
         _ccv->loc[0],    _ccv->loc[1],    _ccv->loc[2],
         _ccv->deltas[0], _ccv->deltas[1], _ccv->deltas[2]);
   else
	   mprintf(" _ccv @(%x,%x),d(%x,%x)..", _ccv->loc[0], _ccv->loc[1], _ccv->deltas[0], _ccv->deltas[1]);
   mprintf("ox%x mp%x fl%xnx%x\n",_ccv->oldx,_ccv->mptr,_ccv->flags,_ccv->nxtv);
}
#else
#define print_nvp(a)
#define print_fcv(a,b)
#endif

static uchar *_face_curedge, *_face_nxtedge;
static uchar  _face_curmask,  _face_nxtmask;
static uchar  _face_topmask,  _face_botmask;

//#define out_of_cone (me_bits_seen_p(mp)==0)
#define out_of_cone(mp) (me_subclip(mp)==SUBCLIP_OUT_OF_CONE)

// indexed by (vecside)+(dircode<<2)+xdir/ydir, with do nothings for second at Y
static uchar  _sclip_major_mask[9][2]=
{
 {FMK_SW,FMK_EW},{FMK_SW,FMK_WW},{FMK_NW,FMK_EW},{FMK_NW,FMK_WW},
 {FMK_NW,FMK_WW},{FMK_NW,FMK_EW},{FMK_SW,FMK_WW},{FMK_SW,FMK_EW},{0,0}
};

static uchar  _sclip_door_mask[9][2]=
{
 {FMK_INT_NW,FMK_INT_EW},{FMK_INT_NW,FMK_INT_WW},{FMK_INT_SW,FMK_INT_EW},{FMK_INT_SW,FMK_INT_WW},
 {FMK_INT_SW,FMK_INT_WW},{FMK_INT_SW,FMK_INT_EW},{FMK_INT_NW,FMK_INT_WW},{FMK_INT_NW,FMK_INT_EW},{0,0}
};

static uchar *_sclip_mask, *_sclip_door;

// assumes ccv is us
// moves until next y span is hit, i guess
// perhaps will have to learn about it's partner vector?
// strip the gloss from a beauty queen
bool _fr_move_ccv_x(struct _nVecWork *nvp)
{
   int tt, oflow;
   bool move_in_y=TRUE, move_in_x=TRUE;

   // should be saving off texture cuts some day!!
   ccv->loc[1]+=nvp->stepy;
   tt=me_tiletype(ccv->mptr);
   if (fr_obj_block(ccv->mptr,_sclip_door,(int *)ccv->loc)||
       ((_face_curedge[tt<<2]==0xff)||(me_clearsolid(ccv->mptr)&_face_curmask)))
   {  // these really have to get wacky and learn about partial obscuration
      move_in_x=move_in_y=FALSE;
      _fr_sdbg(VECSPEW,mprintf("move_x(top): hit our tile\n"));
   }
   else
   {
      _fr_sdbg(VECSPEW,mprintf("move_x(top): sub_clip or %x, old %x\n",_sclip_mask[0],me_subclip(ccv->mptr)));
      _me_subclip(ccv->mptr)|=_sclip_mask[0];
      ccv->mptr+=nvp->mapstep[0];
      tt=me_tiletype(ccv->mptr);
      if ((_face_nxtedge[tt<<2]==0xff)||(me_clearsolid(ccv->mptr)&_face_nxtmask)||out_of_cone(ccv->mptr))
      {  // these really have to get wacky and learn about partial obscuration
         move_in_x=move_in_y=FALSE;
	      ccv->mptr-=nvp->mapstep[0];
         _fr_sdbg(VECSPEW,mprintf("move_x top: hit other tile\n"));
      }   
      else
      {  // correct for new setup
         ccv->loc[0]+=nvp->remx+rmmod[nvp->dircode];
         nvp->remx=locstep[nvp->dircode][0];
		   nvp->stepy=fix_mul_div(ccv->deltas[1],nvp->remx,ccv->deltas[0]);
         _fr_sdbg(VECSPEW,mprintf("move_x top: move and recompute step\n"));
      }
   }
   _fr_sdbg(VECSPEW,{mprintf("move_x top:\n");print_fcv(ccv,2);print_nvp(nvp);});

   oflow=ccv->loc[1]&0xffff;
   while (move_in_x)
   {
      oflow+=nvp->stepy;
      if (oflow&0xffff0000)
      {  // perhaps should get more elegant, but i mean really, how could it?
         move_in_x=FALSE; move_in_y=TRUE;
         _fr_sdbg(VECSPEW,mprintf("move_x(while): reached y\n"));
      }
      else
      {
         ccv->loc[1]+=nvp->stepy;
	      // can we leave the square there?
	      tt=me_tiletype(ccv->mptr);
	      // no matter what, we can get out of ourselves?
         if (fr_obj_block(ccv->mptr,_sclip_door,(int *)ccv->loc)||
             ((_face_curedge[tt<<2]==0xff)||(me_clearsolid(ccv->mptr)&_face_curmask)))
         {  // these really have to get wacky and learn about partial obscuration
	         move_in_x=move_in_y=FALSE;
            _fr_sdbg(VECSPEW,mprintf("move_x(while): hit our tile\n"));
         }
	      else
	      {
            _fr_sdbg(VECSPEW,mprintf("move_x(while): sub_clip or %x, old %x\n",_sclip_mask[0],me_subclip(ccv->mptr)));
            _me_subclip(ccv->mptr)|=_sclip_mask[0];
		      ccv->mptr+=nvp->mapstep[0];
		      tt=me_tiletype(ccv->mptr);
            if ((_face_nxtedge[tt<<2]==0xff)||(me_clearsolid(ccv->mptr)&_face_nxtmask)||out_of_cone(ccv->mptr))
            {  // these really have to get wacky and learn about partial obscuration
	            move_in_x=move_in_y=FALSE; 
	   	      ccv->mptr-=nvp->mapstep[0];
               _fr_sdbg(VECSPEW,mprintf("move_x(while): hit other tile\n"));
	         }
	         else
	         {
	            ccv->loc[0]+=locstep[nvp->dircode][0];
               _fr_sdbg(VECSPEW,mprintf("move_x: moving ccv\n"));
	         }
	      }
      }
      _fr_sdbg(VECSPEW,{mprintf("move_x(while):\n");print_fcv(ccv,2);print_nvp(nvp);});
   }

// should go back to this, it should add to oflow, non add to loc[1] in loop, then do loc[1]|=(oflow-stepy)
//   if (move_in_y) ccv->loc[1]-=nvp->stepy;      // undo the step that went too far(tm)

   return move_in_y;
   // compute intersection
   // if not clear internally, migrate to clear point, recompute
   //   if now clear internally, move mptr and vec to next square
   //   if not, backup pointers, return FALSE
   // check crossin clear, if clear return TRUE
   // if not clear, migrate to next clear point, recompute
   //   if now clear, return TRUE
   //   if not, backup pointers if appropriate, return FALSE
}

void _fr_move_along_dcode(int dircode)
{
   bool move_in_y=TRUE;
   struct _nVecWork *nvp=&_nVP[dircode];

   if (dircode&nVW_XDIR) nvp->remx=_fixp1-1-fix_frac(ccv->loc[0]);
   else                  nvp->remx=-fix_frac(ccv->loc[0]);

   if (ccv->deltas[1]==0)                 // no y delta
	 {	nvp->move_x=TRUE; nvp->stepy=0; }   // lets do the flat line thing
   else if (ccv->deltas[0]==0)            // just do a single y step
      nvp->move_x=FALSE;                  // no need to set step or anything
   else                                   // replace with wacky table neg?
   {                                      // get the signs right, as it were
      if (nvp->remx)
	      switch (dircode)
	      {           // these just arent right
		   case nVW_NXNY: nvp->move_x=(-nvp->remx*ccv->deltas[1] > (fix_frac(ccv->loc[1]))*(ccv->deltas[0])); break;           // flip sign for --
		   case nVW_NXPY: nvp->move_x=( nvp->remx*ccv->deltas[1] > (_fixp1-1-fix_frac(ccv->loc[1]))*(ccv->deltas[0])); break; // flip xd, sign
 		   case nVW_PXNY: nvp->move_x=(-nvp->remx*ccv->deltas[1] < (fix_frac(ccv->loc[1]))*ccv->deltas[0]); break;          // flip yd, sign
		   case nVW_PXPY: nvp->move_x=( nvp->remx*ccv->deltas[1] < (_fixp1-1-fix_frac(ccv->loc[1]))*ccv->deltas[0]); break;    // all things good   
	      }
      else nvp->move_x=TRUE;
      if (nvp->move_x)
	      nvp->stepy=fix_mul_div(nvp->remx,ccv->deltas[1],ccv->deltas[0]);
   }

   // this is dumb, we want the second to be based on primary
   if (ccv->mptr==eye_mptr) // ( () &&((nvp->dircode&nVW_YDIR)==0))
   {
      _sclip_mask=&_sclip_major_mask[8][0];
//      _sclip_door=&_sclip_door_mask[8][0];
   }
   else
	{
      _sclip_mask=(&_sclip_major_mask[0][0])+((ccv->flags&FRVECDIR)>>3)+(dircode+dircode);
//      _sclip_door=(&_sclip_door_mask[0][0])+((ccv->flags&FRVECDIR)>>3)+(dircode+dircode);
   }
   _sclip_door=(&_sclip_door_mask[0][0])+((ccv->flags&FRVECDIR)>>3)+(dircode+dircode);

   _fr_sdbg(VECSPEW,{mprintf("move_along(new_vec): sclip offs %d d%d (%d,%d)\n",_sclip_mask-_sclip_major_mask,dircode,*_sclip_mask,*(_sclip_mask+1));print_fcv(ccv,2);print_nvp(nvp);});

   if (nvp->move_x)
   {
      if (dircode&nVW_XDIR)
       { _face_curedge=&face_obstruct[0][1]; _face_nxtedge=&face_obstruct[0][3]; _face_curmask=FMK_INT_EW; _face_nxtmask=FMK_INT_WW; }
      else
       { _face_curedge=&face_obstruct[0][3]; _face_nxtedge=&face_obstruct[0][1]; _face_curmask=FMK_INT_WW; _face_nxtmask=FMK_INT_EW; }
      move_in_y=_fr_move_ccv_x(nvp);
   }

   // the more he likes me, the more i drink
   // i think the more i drink the more he likes me
   if (move_in_y)
   {     // actually move in y
      fix nstp;
      if (dircode&nVW_YDIR) nstp=_fixp1-1-fix_frac(ccv->loc[1]);
      else                  nstp=-fix_frac(ccv->loc[1]);
      nstp=fix_mul_div(nstp,ccv->deltas[0],ccv->deltas[1]);
      ccv->loc[0]+=nstp;
      ccv->loc[1]&=0xffff0000;      // this will go away in asm, as all we need to do is set the bottom
      ccv->loc[1]+=edgestep[dircode][1];
      _fr_sdbg(VECSPEW,mprintf("move_y(if): sub_clip or %x, old %x\n",_sclip_mask[1],me_subclip(ccv->mptr)));
      _me_subclip(ccv->mptr)|=_sclip_mask[1];
      _fr_sdbg(VECSPEW,{mprintf("move_y(if): nstp %x\n",nstp);print_fcv(ccv,2);});
   }
   else
   {     // pop up to top of square, for now, no f_o usage
      // wow this is a slow dumb way to do this, eh?
      ccv->loc[1]&=0xffff0000;           // the ands go away in assembler, as we need only set the bottom
      ccv->loc[0]&=0xffff0000;
      ccv->loc[0]+=edgestep[dircode][0];
      ccv->loc[1]+=edgestep[dircode][1];
#ifdef SWITCH_IS_FASTER_MAYBE
      switch (dircode)
      {
	      case nVW_NXNY: ccv->loc[0]&=0xffff0000; ccv->loc[1]&=0xffff0000; break;
	      case nVW_NXPY: ccv->loc[0]&=0xffff0000; ccv->loc[1]|=0x0000ffff; break;
	      case nVW_PXNY: ccv->loc[0]|=0x0000ffff; ccv->loc[1]&=0xffff0000; break;
	      case nVW_PXPY: ccv->loc[0]|=0x0000ffff; ccv->loc[1]|=0x0000ffff; break;
      }
#endif
#ifdef ANOTHER_WACKY_WAY
      if (dircode&nVW_XDIR) ccv->loc[0]|=0x0000ffff; else ccv->loc[0]&=0xffff0000;
      if (dircode&nVW_YDIR) ccv->loc[1]|=0x0000ffff; else ccv->loc[1]&=0xffff0000;
#endif
      _fr_sdbg(VECSPEW,mprintf("move_y(else): sub_clip or %x, old %x\n",_sclip_mask[0],me_subclip(ccv->mptr)));
      _me_subclip(ccv->mptr)|=_sclip_mask[0];
      _fr_sdbg(VECSPEW,{mprintf("move_y(else):\n");print_fcv(ccv,2);});
   }
   _fr_sdbg(VECSPEW,{mprintf("move_along(end):\n");print_fcv(ccv,2);print_nvp(nvp);});
}

#define _fr_move_along_hn_p(x)  _fr_move_along_dcode(((ccv->deltas[0]>0)?nVW_XDIR:0)+x)

static uchar *_face_topedge, *_face_botedge;

// partial tiles? who knows when

// note fullwise, at the moment the subclip&face_topmask in middle gets doors, whereas the
//  subclip==SUBCLIP_OUT_OF_CONE gets itself, though in reality and face_botmask would do same thing, i think

// wow is this a total mess, goddamn
#define is_solid() \
            ((_face_topedge[(me_tiletype(cv->mptr)<<2)]==0xff)|| \
             (me_clearsolid(cv->mptr)&_face_topmask)|| \
             (me_subclip(cv->mptr)&_face_topmask)|| \
             (_face_botedge[(me_tiletype((cv->mptr+y_map_step))<<2)]==0xff)|| \
             (me_clearsolid((cv->mptr+y_map_step))&_face_botmask)|| \
             (me_subclip(cv->mptr+y_map_step)==SUBCLIP_OUT_OF_CONE))

#define is_space() \
            ((_face_topedge[(me_tiletype(cv->mptr)<<2)]!=0xff)&& \
             ((me_clearsolid(cv->mptr)&_face_topmask)==0)&& \
             ((me_subclip(cv->mptr)&_face_topmask)==0)&& \
             (_face_botedge[(me_tiletype((cv->mptr+y_map_step))<<2)]!=0xff)&& \
             ((me_clearsolid((cv->mptr+y_map_step))&_face_botmask)==0))

// note how pretty this looks till you look at the is_solid macro
bool _fr_skip_solid_right_n_back(FrClipVec *cv, fix max_loc, int y_map_step)
{
   if (is_solid()) 
   {
      cv->loc[0]&=0xffff0000;
      cv->loc[0]+=_fixp1;  
      cv->mptr+=1;
      while (is_solid()&&(cv->loc[0]<max_loc))
       { cv->loc[0]+=_fixp1; cv->mptr+=1; }
   }
   return (cv->loc[0]>=max_loc);
}

bool _fr_skip_space_right_n_back(FrClipVec *cv, fix max_loc, int y_map_step)
{
   if (is_space()) 
   {
      cv->loc[0]&=0xffff0000;
      cv->loc[0]+=_fixp1;  
      cv->mptr+=1;
      while (is_space()&&(cv->loc[0]<max_loc))
       { cv->loc[0]+=_fixp1; cv->mptr+=1; }
   }
   return (cv->loc[0]>=max_loc);
}

bool _fr_skip_solid_left_n_back(FrClipVec *cv, fix min_loc, int y_map_step)
{
   if (is_solid()) 
   {
      cv->loc[0]|=0x0000ffff;
      cv->loc[0]+=_fixn1;
      cv->mptr+=-1;
      while (is_solid()&&(min_loc<cv->loc[0]))
       { cv->loc[0]+=_fixn1; cv->mptr+=-1; }
   }
   return (cv->loc[0]<=min_loc);
}

#define DT_SHFT (8)
#define DT_FAKE ((1<<DT_SHFT)-1)
#define fixup_delta(dlta) if ((dlta)>0) dlta=((dlta)+DT_FAKE)>>DT_SHFT; else (dlta)=((dlta)-DT_FAKE)>>DT_SHFT;

void _fr_del_compute(FrClipVec *v1, FrClipVec *v2)
{
   v1->deltas[0]=v1->loc[0]-coor(EYE_X); v1->deltas[1]=v1->loc[1]-coor(EYE_Y);
   fixup_delta(v1->deltas[0]);
   fixup_delta(v1->deltas[1]);
   v2->deltas[0]=v2->loc[0]-coor(EYE_X); v2->deltas[1]=v2->loc[1]-coor(EYE_Y);
   fixup_delta(v2->deltas[0]);
   fixup_delta(v2->deltas[1]);
   v1->oldx=v1->loc[0]; v2->oldx=v2->loc[0];
}

void _fr_spawn_check_one(FrClipVec *lv, FrClipVec *rv, bool northward)
{
   FrClipVec *tmpr, *tmpl;
   short cur_mapstep=new_del_mapstep[northward];
   int nxts;

   while (1)                     // really, run till we find the right edge
   {
	   tmpr=allclipv+ffreevec;
	   nxts=tmpr->nxtv;
   	*tmpr=*lv;                 // perhaps should back up a mapstep and check for other half tiles first
	   tmpr->nxtv=nxts; tmpr->flags=ffreevec;           // fixup next and self
	   if (!_fr_skip_space_right_n_back(tmpr,rv->loc[0],cur_mapstep))    // really should do render bit setup here...
	   {             // ok lv->rv->?, and tmpr->tmpl->vecx, ffreevec->tmpr... we want lv->tmpr->tmpl->rv, ffreevec->vecx
	      _fr_sdbg(VECSPEW,{mprintf("new_dels(spawn): found ");print_fcv(tmpr,2);});
	      tmpl=allclipv+nxts;     // tmpl is next free vec, as nxts is tmpr->nxtv and tmpr was ffreevec
	      ffreevec=tmpl->nxtv;    // point ffreevec at tmpl's old next, as tmpl and tmpr are from free list
	      *tmpl=*tmpr;
	      tmpl->nxtv=lv->nxtv;    // point tmpl->rv, lv's old friend
	      lv->nxtv=tmpr->flags;   // we havent or'red in real flag data, so flags is currently just self for tmpr
	      tmpl->flags=tmpr->nxtv; // tmpr->nxtv never changes, it is always pointing at tmpl, so tmpl can self set with it
#if _fr_defdbg(SANITY)
	      if (_fr_skip_solid_right_n_back(tmpl,rv->loc[0],cur_mapstep))  // this is true
	         mprintf("new_dels(spawn) ERR: found middle with no right edge\n");
#else
	      _fr_skip_solid_right_n_back(tmpl,rv->loc[0],cur_mapstep);
#endif
	      tmpr->loc[0]--; tmpr->mptr--; // note that tmpl is a-ok
         if (northward)
          { tmpr->flags|=FRVECUSE|FRVECL; tmpl->flags|=FRVECUSE|FRVECR; }
         else
          { tmpl->flags|=FRVECUSE|FRVECL; tmpr->flags|=FRVECUSE|FRVECR; }
	      _fr_sdbg(VECSPEW,{mprintf("new_dels(spawn): generated\n");print_fcv(tmpr,2);print_fcv(tmpl,2);});
	      (allclipv+lastvec)->nxtv=ffreevec;
	      _fr_del_compute(tmpl,tmpr);
	      lv=tmpl;                // move scan start over appropriately
	   }
      else
         return;
   }
}

// returns FALSE if there are no vectors left in this branch, else true
// ok. northward is done super grossly at the moment
// really, this has to spawn new vectors as well

// i want no part of their death culture
// i just want to go the beach
bool _fr_move_new_dels(FrClipVec *lv, FrClipVec *rv, bool northward)
{
	int lm, rm;

	lm=min(lv->oldx,lv->loc[0]);
   rm=max(rv->oldx,rv->loc[0]);
   store_x_span(fix_int(lv->loc[1]),fix_int(lm),fix_int(rm));
   _fr_sdbg(VECSPEW, mprintf("new_dels: setting span %d to %x,%x - o: %x %x c: x %x %x y %x %x m %x\n",span_count(fix_int(lv->loc[1])),lm,rm,lv->oldx,rv->oldx,lv->loc[0],rv->loc[0],lv->loc[1],rv->loc[1],lv->mptr));

   if (_fr_skip_solid_right_n_back(lv,rv->loc[0],new_del_mapstep[northward])) return FALSE;
   if (_fr_skip_solid_left_n_back(rv,lv->loc[0],new_del_mapstep[northward])) { _fr_sdbg(SANITY,mprintf("new_dels ERR: closure from right\n")); return FALSE; }

   lv->loc[1]+=rmmod[northward+1];  // if we are keeping the vectors, move their y coordinates appropriately
   rv->loc[1]+=rmmod[northward+1];
   _fr_del_compute(lv,rv);

//   (*_fr_spawn_check)(lv,rv);
   _fr_spawn_check_one(lv,rv,northward);

   lm=lv->flags&FRVECSELF;   // now move everyones map coordinates
   rm=rv->nxtv;
   for (;lm!=rm;lm=(allclipv+lm)->nxtv) (allclipv+lm)->mptr+=new_del_mapstep[northward];
   return TRUE;
}

// remembering you fallen into my arms
// crying for the death of your heart
// you were stone white so delicate lost in the cold
// you were always so lost in the dark
void _fr_kill_pair(FrClipVec *lv,FrClipVec *rv)
{
   int lft_self=lv->flags&FRVECSELF, lft_pt=vechead;
// should sanity check for too many vectors here and in spawn code
   if (vechead==lft_self)
   {
      if (rv->nxtv!=ffreevec)
      {
	      vechead=rv->nxtv;    // if not, leave vechead at base for simplicities sake, i guess?
         allclipv[endvec].nxtv=vechead;
      }
      else lastvec=-1;
   }
   else
   {
	   while ((lft_pt!=ffreevec)&&(allclipv[lft_pt].nxtv!=lft_self))
	      lft_pt=allclipv[lft_pt].nxtv;          // go find who points at lv
	   _fr_sdbg(SANITY,if (lft_pt==ffreevec) mprintf("kill_pair(lft_pt) ERR: no lft pt\n"));
	   allclipv[lft_pt].nxtv=rv->nxtv;
      if (lastvec==lv->nxtv)
         lastvec=lft_pt;
   }
   rv->nxtv=ffreevec; ffreevec=lv->flags&FRVECMASK; rv->flags&=FRVECSELF; lv->flags&=FRVECSELF;
   if (lastvec!=-1)
	   (allclipv+lastvec)->nxtv=ffreevec;
}

// it's a wonderful world, with a lot of strange men
// who are standing around, and they're all wearing towels
bool _fr_setup_first_pair(bool headnorth)
{
   fix org[3], ray[3];
   int flags;

   _fr_sdbg(VECSPEW,mprintf("setup_first_pair: note vh %d ff %d\n",vechead,ffreevec));
#define FULL_360_VECTORS
#ifdef FULL_360_VECTORS
   org[0]=coor(EYE_X);
   org[1]=coor(EYE_Y);
   org[2]=coor(EYE_Z);
   ray[1]=fix_make(0,0); ray[2]=fix_make(0,0);
//   ray[0]=fix_make((headnorth)?-1:1,0);
   ray[0]=fix_make(-1,0);
   ccv=allclipv+ffreevec;
   flags=FRVECUSE; flags|=headnorth?FRVECR:FRVECL;
   _fr_build_clip_vec(ccv, org, ray, flags);
   ccv=allclipv+ccv->nxtv;
   ray[0]=fix_make(1,0);
//   ray[0]=fix_make((headnorth)?1:-1,0);
   flags=FRVECUSE; flags|=headnorth?FRVECL:FRVECR;
   _fr_build_clip_vec(ccv, org, ray, flags);
   ffreevec=ccv->nxtv;
   lastvec=ccv->flags&FRVECSELF;
#else
   if (headnorth)
   {
      if (span_lines[3]<0) return FALSE;
      ray[0]=span_lines[2]; ray[1]=span_lines[3];
   }
   else
   {
      if (span_lines[1]>0) return FALSE;
      ray[0]=span_lines[0]; ray[1]=span_lines[1];
   }
   org[2]=coor(EYE_Z);        // these are constant
   ray[2]=fix_make(0,0);
   org[0]=span_intersect[0];  // these are true for both north and south left vecs
   org[1]=span_intersect[1];

   ccv=allclipv+ffreevec;
   flags=FRVECUSE; flags|=headnorth?FRVECR:FRVECL;
   _fr_build_clip_vec(ccv, org, ray, flags);
   ccv=allclipv+ccv->nxtv;

   if (headnorth)
    { ray[0]=span_lines[4]; ray[1]=span_lines[5]; }
   else
    { ray[0]=span_lines[6]; ray[1]=span_lines[7]; }

   org[0]=span_intersect[2];
   org[1]=span_intersect[3];

   flags=FRVECUSE; flags|=headnorth?FRVECL:FRVECR;
   _fr_build_clip_vec(ccv, org, ray, flags);
   ffreevec=ccv->nxtv;
   lastvec=ccv->flags&FRVECSELF;
#endif

   // setup various revectorings
   if (headnorth)
   {
   	_face_topedge=&face_obstruct[0][0];
   	_face_botedge=&face_obstruct[0][2];
      _face_topmask=FMK_INT_NW;
      _face_botmask=FMK_INT_SW;
   }
	else
   {
      _face_topedge=&face_obstruct[0][2];
   	_face_botedge=&face_obstruct[0][0];
      _face_topmask=FMK_INT_SW;
      _face_botmask=FMK_INT_NW;
   }
   fr_clip_start(headnorth);
   return TRUE;
}

#if _fr_defdbg(VECTRACK)
void _fr_show_veclist(void)
{
   int i, cv;
   mprintf("Vec(ff%d): ",ffreevec);
   for (i=0, cv=vechead; i<MAX_CLIP_VEC; i++,cv=allclipv[cv].nxtv)
      mprintf("%1.1X%c%c ",cv,(allclipv[cv].flags&FRVECUSE)?'U':'x',(allclipv[cv].flags&FRVECL)?'L':'R');
   if (cv!=vechead) mprintf("ERROR %d!%d\n",cv,vechead); else mprintf("\n");
}
#else
#define _fr_show_veclist()
#endif

// stains on the carpet and stains on the memory
// and both of us know, how the end always is
int fr_clip_tile(void)
{
   FrClipVec *_v1,*_v2;
   int northward, nxtvec;
   // also have to do exact correct reverse order, so obj_stack works, so go north first, then south
   // sadly, new render order invalidates this

   // next, do each direction
   if (_fr_curflags&FR_SHOWALL_MASK)
      { fr_clip_show_all(); _fr_ret;}  // fill in all things
   _fr_sdbg(VECSPEW,mprintf("Frame start at %x %x\n",coor(EYE_X),coor(EYE_Y)));
   eye_mptr=MAP_GET_XY(_fr_x_cen,_fr_y_cen);
   for (northward=1; northward>=0; northward--)
   {
      // set up the initial vectors and list
      if (!_fr_setup_first_pair(northward)) continue;
      _fr_sdbg(VECSPEW,mprintf("clip_tile(for): heading %d\n",northward));
      // for each line
      do {
	      _v1=ccv=allclipv+vechead;
	      while (ccv->flags&FRVECUSE)
	      {
		      // move out left vector
	         _fr_move_along_hn_p(northward);
		         // at each square, code objects
		         // keep a right edge for internal? ick!
	
		      // move out right vector
	         _v2=ccv=allclipv+ccv->nxtv;
	         nxtvec=ccv->nxtv;                 // so if we new_dels more vecs, or kill our vec, we have the next ptr ready, eh?
	         _fr_move_along_hn_p(northward);	 // do same things, but in reverse
	
	         if (!_fr_move_new_dels(_v1,_v2,northward)) // kill off the vectors
	         {  // wow, can we do multiple here... i guess so
	            _fr_sdbg(VECSPEW,{mprintf("clip_tile(while): killing vector pair\n");print_fcv(_v1,2);print_fcv(_v2,2);});
	            _fr_kill_pair(_v1,_v2);
	         }
	         else
	         {
               nxtvec=_v2->nxtv; // could have changed
		         _fr_sdbg(VECSPEW,{mprintf("clip_tile(while): moving on to %d after pair\n",nxtvec);print_fcv(_v1,2);print_fcv(_v2,2);});
	         }
#if _fr_defdbg(VECTRACK)
            _fr_sdbg(VECTRACK,_fr_show_veclist());
#endif	
		      // store off new base span
		      // spawn/collect vectors
		      // move to next span line
	         _v1=ccv=allclipv+nxtvec;
	      }
      } while (ffreevec!=vechead);
   }
   // hit the fucking road
   span_fixup();
   _fr_ret;
}

// can you see?
// see into the back of a long black car
// pulling away from a funeral of flowers
// with my hand, between your legs
// melting

// fills dst with a wall hit by a ray cast from orig along ray
// a len!=0 is stopped at, 0 goes forever or until page fault
// each fix* is assumed to be a 3 element array
#ifdef WE_WERE_COOL
fix *fr_ray_cast(fix *org, fix *ray, fix *dst, fix len)
{
   MapElem *cur_us;

   _fr_build_clip_vec(&scratchvec,org,ray,len);
}
#endif
