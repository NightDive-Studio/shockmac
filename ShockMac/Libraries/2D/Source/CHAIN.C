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
 * $Source: r:/prj/lib/src/2d/RCS/chain.c $
 * $Revision: 1.8 $
 * $Author: kevin $
 * $Date: 1994/12/05 21:02:35 $
 *
 * Routines for chaining functions to 2D primitives.
 *
 * This file is part of the 2D library.
 *
 * $Log: chain.c $
 * Revision 1.8  1994/12/05  21:02:35  kevin
 * Reworked to avoid linking in unused function tables.
 * 
 * Revision 1.7  1994/12/02  21:51:39  kevin
 * Moved chaining globals accessed in gr_set_canvas to grd.c to avoid linking in unused code.
 * 
 * Revision 1.6  1993/12/07  11:45:15  baf
 * Renames gr_chain_add_over
 * 
 * Revision 1.5  1993/12/02  13:44:13  baf
 * Added the ability to unchain and rechain.
 * 
 * Revision 1.4  1993/11/30  20:47:08  baf
 * Chainged generic mode stuff so it can
 * be done at any time.
 * 
 * Revision 1.3  1993/11/16  23:06:33  baf
 * Added the ability to chain void functions
 * after the primitive.
 * 
 * Revision 1.2  1993/11/15  03:26:53  baf
 * Added void chaining (functions that just
 * get executed and pass on)
 * 
 * Revision 1.1  1993/11/12  09:29:49  baf
 * Initial revision
 * 
 */

#include "chain.h"
#include "chnfuncs.h"
#include "grmalloc.h"
#include "tabdat.h"
#include "bitmap.h"
#include "cnvtab.h"
#include "cnvdrv.h"
#include "icanvas.h"
#include "grnull.h"

grs_func_chain *grd_chain_table[GRD_CANVAS_FUNCS];

grs_func_chain *gr_chain_add_over(int n, void (*f)())
{
   grs_func_chain *p = (grs_func_chain *)(NewPtr(sizeof(grs_func_chain)));	// was gr_malloc
   if (grd_chain_table[n] == NULL) {
      /* First time: stash and replace primitives */
      int k;
      chn_primitives[n] = (void (**)())(NewPtr(BMT_TYPES*sizeof(void (*)())));	// was gr_malloc
      for (k=0; k<BMT_TYPES; k++)
          if (grd_canvas_table_list[k] != NULL)
            chn_primitives[n][k] = grd_canvas_table_list[k][n];
      for (k=0; k<BMT_TYPES; k++)
          if (grd_canvas_table_list[k] != NULL)
            grd_canvas_table_list[k][n] = chn_canvas_table[n];
      /* The above two loops are kept apart for a reason:
           two pointers may be the same, and we want to save the
            initial values of them all. */
   }
   /* Hook into chain */
   p->f = f;
   p->next = grd_chain_table[n];
   p->flags = 0;
   grd_chain_table[n] = p;
   return p; 
}

grs_func_chain *gr_chain_add_before(int n, void (*f)(void))
{
   grs_func_chain *p;
   p = gr_chain_add_over(n, f);
   p->flags |= 1;
   return p; 
}

grs_func_chain *gr_chain_add_after(int n, void (*f)(void))
{
   grs_func_chain *p = (grs_func_chain *)(NewPtr(sizeof(grs_func_chain)));	// was gr_malloc
   p->f = f;
   p->next = NULL;
   p->flags = CHNF_VOID | CHNF_AFTER;
   if (grd_chain_table[n] == NULL) {
      /* First time: stash and replace primitives */
      int k;
      chn_primitives[n] = (void (**)())(NewPtr(BMT_TYPES*sizeof(void (*)())));	// was gr_malloc
      for (k=0; k<BMT_TYPES; k++)
          if (grd_canvas_table_list[k] != NULL)
            chn_primitives[n][k] = grd_canvas_table_list[k][n];
      for (k=0; k<BMT_TYPES; k++)
          if (grd_canvas_table_list[k] != NULL)
            grd_canvas_table_list[k][n] = chn_canvas_table[n];
      /* The above two loops are kept apart for a reason:
           two pointers may be the same, and we want to save the
            initial values of them all. */
      grd_chain_table[n] = p;
   }
   else {
      grs_func_chain *q = grd_chain_table[n];
      while (q->next != NULL) q = q->next;
      q->next = p;
   }
   return p; 
}

void gr_unchain(int n)
{
   int k;
   /* check for unchained primitive */
   if (chn_primitives[n] == NULL) return;
   for (k=0; k<BMT_TYPES; k++)
      if (grd_canvas_table_list[k] != NULL)
         grd_canvas_table_list[k][n] = chn_primitives[n][k];
}

void gr_rechain(int n)
{
   int k;
   if (chn_primitives[n] == NULL) return;
   for (k=0; k<BMT_TYPES; k++)
      if (grd_canvas_table_list[k] != NULL)
         grd_canvas_table_list[k][n] = chn_canvas_table[n];
}

void gr_unchain_all()
{
   int k;
   for (k=0; k<GRD_CANVAS_FUNCS; k++) gr_unchain(k);
}

void gr_rechain_all()
{
   int k;
   for (k=0; k<GRD_CANVAS_FUNCS; k++) gr_rechain(k);
}


/* This returns the next function in the rest of the current chain. */
void (*chain_rest())()
{
   for (gr_current_chain = gr_current_chain->next; gr_current_chain != NULL; gr_current_chain = gr_current_chain->next) {
      if (!(gr_current_chain->flags & CHNF_VOID))
         return gr_current_chain->f;
      if (gr_current_chain->flags & CHNF_AFTER) break;
      gr_current_chain->f();
   }
   if (gr_current_primitive <= GET_PIXEL24 || gr_current_primitive >= CALC_ROW)
      return chn_primitives[gr_current_primitive][grd_pixel_index];
   return chn_primitives[gr_current_primitive][grd_canvas_index];
}

#include "canvas.h"
#include "cnvdat.h"
#include "cnvdrv.h"
#include "fcntab.h"
#include "tabdrv.h"
#include "fill.h"

void gr_force_generic()
{
   chn_flags |= CHN_GEN;
   grd_canvas_table_list[BMT_GEN]=gen_canvas_table;
   grd_function_table_list[BMT_GEN]= (grt_function_table *) gen_function_table;
   if (grd_canvas_table != NULL)
      gr_set_canvas(grd_canvas);
}

void gr_unforce_generic()
{
   chn_flags &= ~CHN_GEN;
   gr_set_canvas(grd_canvas);
}

