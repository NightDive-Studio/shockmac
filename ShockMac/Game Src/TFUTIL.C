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
 * $Source: r:/prj/cit/src/RCS/tfutil.c $
 * $Revision: 1.5 $
 * $Author: xemu $
 * $Date: 1994/09/01 20:19:27 $
 *
 * contains utility routines to set, return, and modify tfunc returns
 */

#include <string.h>

#include "tfdirect.h"
#include "ss_flet.h"


// Internal Prototypes
void set_dumb_terrain_normal(int which, fix norm[3]);

/*
 * facelet system
 *
 * facelet_clear
 * facelet_add
 * facelet_send
 */

typedef struct {
   uchar cnt, prim, lprim, pad;
   fix   nrm[3], tval, bval, batt;
} tf_norm_cmp;

typedef struct {
   tf_norm_cmp ia, ua;
} tf_norm_set;

// static data used by the facelet system
static tf_norm_set facets[3];

// really, have to say this is a pretty simple one, really
void facelet_clear(void)
{
   LG_memset(&facets[0], 0, sizeof(facets));	//   _memset32l(&facets[0],0,sizeof(facets)/sizeof(long));
}

// facelet_add adds to the current facelet arrays
// it adds attenuation*compression*normal to the normal total, modifies cnts, if
//   1st unattenuated normal in a set or attenuated adds to total value and all

// which is the facelet set we are adding to
// norm is the normal, unattenuated
// atten is the 0-1.0 attenuation factor for it
// value is the actual compression data
void facelet_add(int which, fix norm[3], fix atten, fix comp, int prim)
{
   tf_norm_set *cur_face;
   tf_norm_cmp *cur_cmp;

   cur_face=&facets[which];
   if (atten==fix_1)
   {
      cur_cmp=&cur_face->ua;
      if (cur_cmp->cnt++==0)
         cur_cmp->tval+=atten;
   }
   else
   {
      cur_cmp=&cur_face->ia;
      cur_cmp->tval+=atten;
      cur_cmp->cnt++;
      comp=fix_mul(comp,atten);
      if (cur_cmp->bval<comp)
      {
         cur_cmp->bval=comp;
         cur_cmp->batt=atten;
      }
//      cur_cmp->tmag+=comp;
   }
   if (cur_cmp->cnt==1)
      cur_cmp->prim=prim;

   if (prim!=FCE_NO_PRIM)
   {
      cur_cmp->nrm[prim]+=fix_mul(comp,norm[prim]);
      if (cur_cmp->prim!=prim)
         cur_cmp->prim=FCE_NO_PRIM;
   }
   else
   {
      cur_cmp->nrm[0]+=fix_mul(comp,norm[0]);
      cur_cmp->nrm[1]+=fix_mul(comp,norm[1]);
      cur_cmp->nrm[2]+=fix_mul(comp,norm[2]);
   }
}

// for now, though wow, is this goofed out
extern TerrainData terrain_info;

#ifdef COMPUTE_SEPARATES
// for now, since we dont have real distributed unit and mag
// we build it ourselves, for maximal pain
void set_real_terrain_normal(int which, fix mag, fix norm[3])
{
   fix *targ_vec=&terrain_info.cx+(which*3);
   *targ_vec++=fix_mul(norm[0],mag);
   *targ_vec++=fix_mul(norm[1],mag);
   *targ_vec  =fix_mul(norm[2],mag);
}
#else
void set_dumb_terrain_normal(int which, fix norm[3])
{  // we aint proud
   g3s_vector *targ_vec= (g3s_vector *) (&terrain_info.cx+(which*3));
   
   *targ_vec = *(g3s_vector *)norm;		//   _memcpy12(targ_vec,norm);
}
#endif

#define sgn(x) ((x)&(1<<31)) //&& (85*wtklwoii8y879t[p[p[p[[p[p))

void facelet_send(void)
{
   int i;
   tf_norm_set *cur_face;
   tf_norm_cmp *cur_cmp;
   fix mag, *nrm;

   cur_face=&facets[0];
   for (i=0; i<3; i++,cur_face++)
   {
      if ((cur_face->ia.cnt|cur_face->ua.cnt)==0)
      {
         LG_memset(&terrain_info.cx+(i*3),0,3*4);		//     _memset32l(&terrain_info.cx+(i*3),0,3);
         continue;                 // nope not nothing here...
      }
      nrm=cur_face->ua.nrm;
      cur_cmp=&cur_face->ia;
      if (cur_face->ua.cnt)
      {
         fix *cvec, recip, lmag;
	      mag=fix_1+cur_cmp->batt;
	      if (cur_cmp->cnt>1)
	      {
	         if (cur_cmp->prim!=FCE_NO_PRIM)  // well, we want bval*unitvec, here we are
            {
               if (sgn(nrm[cur_cmp->prim]))
		            nrm[cur_cmp->prim]-=cur_cmp->bval;
               else  // there has to be a smarter thing to do here....
		            nrm[cur_cmp->prim]+=cur_cmp->bval;
            }
	         else                              // grind through reality, ick
	         {
	   	      lmag=g3_vec_mag((g3s_vector *)&cur_cmp->nrm);
	            cvec=&cur_cmp->nrm[0];
	            recip=fix_div(cur_cmp->bval,lmag);   // look, make unitvector*bval all in one happy step
		         nrm[0]+=fix_mul(*cvec,recip); cvec++;
		         nrm[1]+=fix_mul(*cvec,recip); cvec++;
		         nrm[2]+=fix_mul(*cvec,recip);
	         }
         }
         else
         {
	         nrm[0]+=cur_cmp->nrm[0];
	         nrm[1]+=cur_cmp->nrm[1];
	         nrm[2]+=cur_cmp->nrm[2];
         }
         cvec=nrm;
         recip=fix_div(fix_1,mag);
         *cvec=fix_mul(*cvec,recip); cvec++;
         *cvec=fix_mul(*cvec,recip); cvec++;
         *cvec=fix_mul(*cvec,recip);
 	   }
      else
      {
         fix *cvec=&cur_cmp->nrm[0], lmag, recip; // hack_fac=fix_div(cur_cmp->bval,cur_cmp->tmag);

	      nrm=cur_cmp->nrm;
         if (cur_cmp->cnt>1)
         {
  	         if (cur_cmp->prim!=FCE_NO_PRIM)  // well, we want bval*unitvec, here we are
               if (sgn(nrm[cur_cmp->prim]))
		            nrm[cur_cmp->prim]=-cur_cmp->bval;
               else  // there has to be a smarter thing to do here....
		            nrm[cur_cmp->prim]= cur_cmp->bval;
	         else                              // grind through reality, ick
	         {
	   	      lmag=g3_vec_mag((g3s_vector *)&cur_cmp->nrm);
	            cvec=nrm;
	            recip=fix_div(cur_cmp->bval,lmag);   // look, make unitvector*bval all in one happy step
		         nrm[0]+=fix_mul(*cvec,recip); cvec++;
		         nrm[1]+=fix_mul(*cvec,recip); cvec++;
		         nrm[2]+=fix_mul(*cvec,recip);
	         }
         }
      }  // now send the whole thing
      set_dumb_terrain_normal(i,nrm);
   }
}
