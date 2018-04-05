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
 * $Source: n:/project/lib/src/3d/RCS/clip.c $
 * $Revision: 1.6 $
 * $Author: kaboom $
 * $Date: 1994/03/23 11:48:45 $
 *
 * Polygon and line clipping routines ripped off from freefall and
 * hacked to use inverted y coordinates.
 */

#include <string.h>

#include "fix.h"
#include "3d.h"
#include "2d.h"
#include "GlobalV.h"

#define NEXTI(x) ( ((x)+1==n) ? 0 : (x)+1 )

// these indicate that you have clipped it there
#define CC_OFF_X 16
#define CC_OFF_Y 32
#define CC_MASK (0xff-(CC_OFF_X|CC_OFF_Y))

void g3_intersect(void);

void g3_left_intersect(void);
void g3_top_intersect(void);
void g3_right_intersect(void);
void g3_bottom_intersect(void);
void g3_back_intersect(void);
// void project_point(g3s_point *src[],int n);

static g3s_point tbuff[20];
static int tnum;
//static fix _d = 65536/18;
static fix _d = 1;
static fix _a,_b,_c;
static fix num,den;           // num/den is dist from s to interseciton
static g3s_point *_tmp;
static g3s_point *s;          // start of line segment
static g3s_point *e;          // end of line segment

int g3_clip_line(g3s_point *src[],g3s_point *dest[])
{
   int i,j;
   byte cc;
   byte ca;
   // assume 10 points max
   g3s_point *tmp0[10];
   g3s_point *tmp1[10];
   int b;               // current destination buffer 
   g3s_point **tmps;    // pointer to the tmp buffer
   g3s_point **tmpd;    // pointer to dest buffer

   cc = src[0]->codes | src[1]->codes;
   ca = src[0]->codes & src[1]->codes;

   // if all the same, leave
   if (cc==0) {
      // copy src to dest
      LG_memcpy(dest,src,2*sizeof(g3s_point *));
      return CLIP_NONE;
   }
   if (ca!=0)
      return CLIP_ALL;

   tnum = 0;
   // only cycle through ones you need to,
   // which is usually only one side, except
   // when right up close

   tmps = src;
   tmpd = tmp0;
   b = 0;

   // Clip to _d in the z axis if it needs it
   if (cc<0) {
      s = tmps[0];
      e = tmps[1];
      j = 0;

      // If the inside one is in
      // if CC_BEHIND is set, codes is negative
      if ((s->codes&CC_BEHIND) == 0) {
         tmpd[j] = s;
         j++;
      }
      // if they intersect put the intersection
      // start in and end out or out and in
      if (((s->codes ^ e->codes)&CC_BEHIND) != 0) {
         g3_back_intersect();
         tmpd[j] = _tmp;
         j++;
      }
      if ((e->codes&CC_BEHIND) == 0) {
         tmpd[j] = e;
         j++;
      }
      // recalculate codes
      cc = tmpd[0]->codes | tmpd[1]->codes;
      if (tmpd[0]->codes & tmpd[1]->codes)
         return CLIP_ALL;
      tmps = tmpd;
      b = 1-b;
      tmpd = (b==0) ? tmp0: tmp1;
   }

   // clip to the left, 0 in the x axis
   if ((cc&CC_OFF_LEFT) != 0) {
      s = tmps[0];
      e = tmps[1];
      j = 0;

      if ((s->codes&CC_OFF_LEFT) == 0) {
         tmpd[j] = s;
         j++;
      }
      // if they intersect put the intersection
      // start in and end out or out and in
      if (((s->codes ^ e->codes)&CC_OFF_LEFT) != 0) {
         g3_left_intersect();
         tmpd[j] = _tmp;
         j++;
      }
      if ((e->codes&CC_OFF_LEFT) == 0) {
         tmpd[j] = e;
         j++;
      }
      // recalculate codes
      cc = tmpd[0]->codes | tmpd[1]->codes;
      if (tmpd[0]->codes & tmpd[1]->codes)
         return CLIP_ALL;
      tmps = tmpd;
      b = 1-b;
      tmpd = (b==0) ? tmp0: tmp1;
   }

   // Clip to 0 in the y axis, top
   if ( (cc&CC_OFF_TOP) != 0) {
      s = tmps[0];
      e = tmps[1];
      j = 0;

      if ((s->codes&CC_OFF_TOP) == 0) {
         tmpd[j] = s;
         j++;
      }
      // if they intersect put the intersection
      // start in and end out or out and in
      if (((s->codes ^ e->codes)&CC_OFF_TOP) != 0) {
         g3_top_intersect();
         tmpd[j] = _tmp;
         j++;
      }
      if ((e->codes&CC_OFF_TOP) == 0) {
         tmpd[j] = e;
         j++;
      }
      // recalculate codes
      cc = tmpd[0]->codes | tmpd[1]->codes;
      if (tmpd[0]->codes & tmpd[1]->codes)
         return CLIP_ALL;
      tmps = tmpd;
      b = 1-b;
      tmpd = (b==0) ? tmp0: tmp1;
   }

   // Clip to w in the x axis, right
   if ((cc&CC_OFF_RIGHT) != 0) {
      s = tmps[0];
      e = tmps[1];
      j = 0;

      if ((s->codes&CC_OFF_RIGHT) == 0) {
         tmpd[j] = s;
         j++;
      }
      // if they intersect put the intersection
      // start in and end out or out and in
      if (((s->codes ^ e->codes)&CC_OFF_RIGHT) != 0) {
         g3_right_intersect();
         tmpd[j] = _tmp;
         j++;
      }
      if ((e->codes&CC_OFF_RIGHT) == 0) {
         tmpd[j] = e;
         j++;
      }
      // recalculate codes
      cc = tmpd[0]->codes | tmpd[1]->codes;
      if (tmpd[0]->codes & tmpd[1]->codes & CC_MASK)
         return CLIP_ALL;
      tmps = tmpd;
      b = 1-b;
      tmpd = (b==0) ? tmp0: tmp1;
   }

   // Clip to w in the y axis, bottom
   if ((cc&CC_OFF_BOT) != 0) {
      s = tmps[0];
      e = tmps[1];
      j = 0;

      if ((s->codes&CC_OFF_BOT) == 0) {
         tmpd[j] = s;
         j++;
      }
      // if they intersect put the intersection
      // start in and end out or out and in
      if (((s->codes ^ e->codes)&CC_OFF_BOT) != 0) {
         g3_bottom_intersect();
         tmpd[j] = _tmp;
         j++;
      }
      if ((e->codes&CC_OFF_BOT) == 0) {
         tmpd[j] = e;
         j++;
      }
      tmps = tmpd;
      if (tmpd[0]->codes & tmpd[1]->codes & CC_MASK)
         return CLIP_ALL;
   }

   // project those that need it
   // if its been clipped it needs it

   // final copy to tmp
   LG_memcpy(dest,tmps,2*sizeof(g3s_point *));

   for (i=0;i<2;i++) {
      _tmp = dest[i];

      if (_tmp->p3_flags & PF_CLIPPNT) {
         // do x
         if ((_tmp->codes&CC_OFF_X)==0 || (_tmp->codes&CC_OFF_Y)!=0)
            _tmp->sx = fix_mul(_scrw,(FIX_UNIT+fix_div(_tmp->gX,_tmp->gZ)));
         else
            _tmp->sx = (_tmp->gX > 0) ? fix_make(grd_bm.w,0) : 0;
         // do y
         if ((_tmp->codes&CC_OFF_Y)==0 || (_tmp->codes&CC_OFF_X)!=0)
            _tmp->sy = fix_mul(_scrh,(FIX_UNIT-fix_div(_tmp->gY,_tmp->gZ)));
         else
            _tmp->sy = (_tmp->gY < 0) ? fix_make(grd_bm.h,0) : 0;
      }
   }

   return CLIP_NONE;
}

int g3_clip_polygon(int n,g3s_point *src[],g3s_point *dest[])
{
   int i,j,k;
   byte cc;
   // assume 10 points max
   g3s_point *tmp0[10];
   g3s_point *tmp1[10];
   int b;               // current destination buffer 
   g3s_point **tmps;    // pointer to the tmp buffer
   g3s_point **tmpd;    // pointer to dest buffer

   for (i=0,cc=0; i<n; ++i) cc |= src[i]->codes;

   // if all the same, leave
   if (cc == 0) {
      // copy src to dest
      LG_memcpy(dest,src,n*sizeof(g3s_point *));
      return n;
   }

   tnum = 0;
   // only cycle through ones you need to,
   // which is usually only one side, except
   // when right up close

   tmps = src;
   tmpd = tmp0;
   b = 0;

   // Clip to _d in the z axis if it needs it
   if (cc<0) {
      for (i=j=0;i<n;i++) {
         s = tmps[i];
         k = NEXTI(i);
         e = tmps[k];

         // If the inside one is in
         // if CC_BEHIND is set, codes is negative
         if ( (s->codes&CC_BEHIND) == 0 ) {
            tmpd[j] = tmps[i];
            j++;
         }
         // if they intersect put the intersection
         // start in and end out or out and in
         if ( ((s->codes ^ e->codes)&CC_BEHIND) != 0 ) {
            g3_back_intersect();
            tmpd[j] = _tmp;
            j++;
         }
      }
      // recalculate codes
      n = j;
      for(i=0,cc=0; i<n; ++i) cc |= tmpd[i]->codes;
      tmps = tmpd;
      b = 1-b;
      tmpd = (b==0) ? tmp0: tmp1;
   }

   // clip to the left, 0 in the x axis
   if ( (cc&CC_OFF_LEFT) != 0) {
      for (i=j=0;i<n;i++) {
         s = tmps[i];
         k = NEXTI(i);
         e = tmps[k];

         if ( (s->codes&CC_OFF_LEFT) == 0 ) {
            tmpd[j] = tmps[i];
            j++;
         }
         // if they intersect put the intersection
         // start in and end out or out and in
         if ( ((s->codes ^ e->codes)&CC_OFF_LEFT) != 0 ) {
            g3_left_intersect();
            tmpd[j] = _tmp;
            j++;
         }
      }
      // recalculate codes
      n = j;
      for(i=0,cc=0; i<n; ++i) cc |= tmpd[i]->codes;
      tmps = tmpd;
      b = 1-b;
      tmpd = (b==0) ? tmp0: tmp1;
   }

   // Clip to 0 in the y axis, top
   if ( (cc&CC_OFF_TOP) != 0) {
      for (i=j=0;i<n;i++) {
         s = tmps[i];
         k = NEXTI(i);
         e = tmps[k];

         if ( (s->codes&CC_OFF_TOP) == 0 ) {
            tmpd[j] = tmps[i];
            j++;
         }
         // if they intersect put the intersection
         // start in and end out or out and in
         if ( ((s->codes ^ e->codes)&CC_OFF_TOP) != 0 ) {
            g3_top_intersect();
            tmpd[j] = _tmp;
            j++;
         }
      }
      n = j;
      // recalculate codes
      for(i=0,cc=0; i<n; ++i) cc |= tmpd[i]->codes;
      tmps = tmpd;
      b = 1-b;
      tmpd = (b==0) ? tmp0: tmp1;
   }

   // Clip to w in the x axis, right
   if ( (cc&CC_OFF_RIGHT) != 0) {
      for (i=j=0;i<n;i++) {
         s = tmps[i];
         k = NEXTI(i);
         e = tmps[k];

         if ( (s->codes&CC_OFF_RIGHT) == 0 ) {
            tmpd[j] = tmps[i];
            j++;
         }
         // if they intersect put the intersection
         // start in and end out or out and in
         if ( ((s->codes ^ e->codes)&CC_OFF_RIGHT) != 0 ) {
            g3_right_intersect();
            tmpd[j] = _tmp;
            j++;
         }
      }
      // copy dest to the src just to be funny
      n = j;
      for(i=0,cc=0; i<n; ++i) cc |= tmpd[i]->codes;
      tmps = tmpd;
      b = 1-b;
      tmpd = (b==0) ? tmp0: tmp1;
   }

   // Clip to w in the y axis, bottom
   if ( (cc&CC_OFF_BOT) != 0) {
      for (i=j=0;i<n;i++) {
         s = tmps[i];
         k = NEXTI(i);
         e = tmps[k];

         if ( (s->codes&CC_OFF_BOT) == 0 ) {
            tmpd[j] = tmps[i];
            j++;
         }
         // if they intersect put the intersection
         // start in and end out or out and in
         if ( ((s->codes ^ e->codes)&CC_OFF_BOT) != 0 ) {
            g3_bottom_intersect();
            tmpd[j] = _tmp;
            j++;
         }
      }
      n = j;
      tmps = tmpd;
   }

   // project those that need it
   // if its been clipped it needs it

   // final copy to tmp
   LG_memcpy(dest,tmps,n*sizeof(g3s_point *));

   for (i=0;i<n;i++) {
      _tmp = dest[i];

      if ( _tmp->p3_flags & PF_CLIPPNT) {
         // do x
         if ((_tmp->codes&CC_OFF_X)==0 || (_tmp->codes&CC_OFF_Y)!=0)
            _tmp->sx = fix_mul(_scrw,(FIX_UNIT+fix_div(_tmp->gX,_tmp->gZ)));
         else
            _tmp->sx = (_tmp->gX > 0) ? fix_make(grd_bm.w,0) : 0;
         // do y
         if ((_tmp->codes&CC_OFF_Y)==0 && (_tmp->codes&CC_OFF_X)!=0)
            _tmp->sy = fix_mul(_scrh,(FIX_UNIT-fix_div(_tmp->gY,_tmp->gZ)));
         else
            _tmp->sy = (_tmp->gY < 0) ? fix_make(grd_bm.h,0) : 0;
      }
   }

   return j;
}

// take care of analyzing
void g3_intersect(void)
{
   fix rs,gs,bs;
   fix re,ge,be;
   grs_rgb c;

   _tmp->p3_flags = s->p3_flags | PF_CLIPPNT | PF_PROJECTED;

   if ((_tmp->p3_flags & PF_RGB) != 0) {
      c = s->rgb;
      rs = (c<<12)&0x3ff000;
      gs = (c<<1) &0x3ff000;
      bs = (c>>10)&0x3ff000;

      c = e->rgb;
      re = (c<<12)&0x3ff000;
      ge = (c<<1) &0x3ff000;
      be = (c>>10)&0x3ff000;

      rs+=fix_mul_div(re-rs,num,den);
      gs+=fix_mul_div(ge-gs,num,den);
      bs+=fix_mul_div(be-bs,num,den);

      _tmp->rgb = ((rs&0x3ff000)>>12) | ((gs&0x3ff000)>>1) | ((bs&0x3ff000)<<10);

   } else {
      if ((_tmp->p3_flags & PF_U) != 0)
         // zany shifts for shorts
         _tmp->uv.u=s->uv.u+fix_mul_div(e->uv.u-s->uv.u,num,den);

      if ((_tmp->p3_flags & PF_V) != 0)
         // zany shifts for shorts
         _tmp->uv.v=s->uv.v+fix_mul_div(e->uv.v-s->uv.v,num,den);
   }
   if ((_tmp->p3_flags & PF_I) != 0)
      // zany shifts for shorts
      _tmp->i=s->i+fix_mul_div(e->i-s->i,num,den);
}

void g3_back_intersect(void)
{
   _tmp = &tbuff[tnum++];

   _a = e->gX - s->gX;
   _b = e->gY - s->gY;
   _c = e->gZ - s->gZ;

   num=_d-s->gZ; den=_c;
   _tmp->gX=s->gX+fix_mul_div(_a,num,den);
   _tmp->gY=s->gY+fix_mul_div(_b,num,den);
   _tmp->gZ=_d;

   _tmp->codes = ((_tmp->gX >= _tmp->gZ) ? CC_OFF_RIGHT : ((_tmp->gX <= -_tmp->gZ) ? CC_OFF_LEFT : 0))
      | ((_tmp->gY >= _tmp->gZ) ? CC_OFF_TOP: ((_tmp->gY <= -_tmp->gZ) ? CC_OFF_BOT :  0));
      
   g3_intersect();
}

void g3_left_intersect(void)
{
   _tmp = &tbuff[tnum++];

   _a = e->gX - s->gX;
   _b = e->gY - s->gY;
   _c = e->gZ - s->gZ;

   num=-s->gZ-s->gX; den=_a+_c;
   _tmp->gY=s->gY+fix_mul_div(_b,num,den);
   _tmp->gZ=s->gZ+fix_mul_div(_c,num,den);
   _tmp->gX=-_tmp->gZ;

   _tmp->codes = ((_tmp->gY >= _tmp->gZ) ? CC_OFF_TOP: ((_tmp->gY <= -_tmp->gZ) ? CC_OFF_BOT :  0))
      | CC_OFF_X;

   g3_intersect();
}

void g3_top_intersect(void)
{
   _tmp = &tbuff[tnum++];

   _a = e->gX - s->gX;
   _b = -e->gY + s->gY;
   _c = e->gZ - s->gZ;

   num=s->gY-s->gZ; den=_b+_c;
   _tmp->gX=s->gX+fix_mul_div(_a,num,den);
   _tmp->gZ=s->gZ+fix_mul_div(_c,num,den);
   _tmp->gY=_tmp->gZ;

   _tmp->codes = ((_tmp->gX >= _tmp->gZ) ? CC_OFF_RIGHT : 0)
      | CC_OFF_Y | (s->codes & e->codes & CC_OFF_X);

   g3_intersect();
}

void g3_right_intersect(void)
{
   _tmp = &tbuff[tnum++];

   _a = e->gX - s->gX;
   _b = e->gY - s->gY;
   _c = e->gZ - s->gZ;

   num=s->gZ-s->gX; den=_a-_c;
   _tmp->gY=s->gY+fix_mul_div(_b,num,den);
   _tmp->gZ=s->gZ+fix_mul_div(_c,num,den);
   _tmp->gX=_tmp->gZ;

   _tmp->codes = ((_tmp->gY <= -_tmp->gZ) ? CC_OFF_BOT: 0)
      | CC_OFF_X | (s->codes & e->codes & CC_OFF_Y);

   g3_intersect();
}

void g3_bottom_intersect(void)
{
   _tmp = &tbuff[tnum++];

   _a = e->gX - s->gX;
   _b = -e->gY + s->gY;
   _c = e->gZ - s->gZ;

   num=s->gZ+s->gY; den=_b-_c;
   _tmp->gX=s->gX+fix_mul_div(_a,num,den);
   _tmp->gZ=s->gZ+fix_mul_div(_c,num,den);
   _tmp->gY=-_tmp->gZ;

   _tmp->codes = CC_OFF_Y | (s->codes & e->codes & CC_OFF_X);
   g3_intersect();
}

/*
void project_point(g3s_point *src[],int n)
{
   g3s_point *p;
   ubyte c;
   int i;

   for (i=0;i<n;i++) {
      c = 0;
      p = src[i];

      if ( (p->p3_flags&PF_PROJECTED) == 0) {

         // subtract, mask sign bit and shift into place
         if (p->gZ < 0) c |= CC_BEHIND;
         if (p->gX > p->gZ) c|= CC_OFF_RIGHT;
         else if (p->gX <= -p->gZ) c|= CC_OFF_LEFT;
         if (p->gY >= p->gZ) c |= CC_OFF_TOP;
         else if (p->gY <= -p->gZ) c |= CC_OFF_BOT;

         p->codes = c;

         // project if inside
         if (c==0) {
            p->sx = fix_mul(_scrw,(FIX_UNIT+fix_div(p->gX,p->gZ)));
            p->sy = fix_mul(_scrh,(FIX_UNIT-fix_div(p->gY,p->gZ)));
            p->p3_flags |= PF_PROJECTED;
         }
      }
   }
}
*/