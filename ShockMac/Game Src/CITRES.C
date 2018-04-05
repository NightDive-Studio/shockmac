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
 * $Source: r:/prj/cit/src/RCS/citres.c $
 * $Revision: 1.20 $
 * $Author: xemu $
 * $Date: 1994/11/07 13:22:58 $
 *
 */
#include <string.h>

#include "citres.h"
#include "criterr.h"
#include "gr2ss.h"
#include "statics.h"

// Internal Prototypes
errtype master_load_bitmap_from_res(grs_bitmap *bmp, Id id_num, int i, RefTable *rt, bool tmp_mem, 
															LGRect *anchor, uchar *p);


grs_bitmap *lock_bitmap_from_ref_anchor(Ref r, LGRect *anchor)
{
   FrameDesc *f;
   if (r == 0)
      return(NULL);
   f = (FrameDesc *)RefLock(r);
   if (f == NULL)
   {
//      Warning(("Could not lock bitmap %d!",r));
      return(NULL);
   }
   f->bm.bits = (uchar *)(f + 1);
   if (anchor != NULL)
      *anchor = f->anchorArea;
//   DBG((DSRC_GFX_Anim),
//   {
//      ss_bitmap(&(f->bm),0,0);
//   });
   return(&(f->bm));
}


grs_bitmap *get_bitmap_from_ref_anchor(Ref r, LGRect* anchor)
{
   grs_bitmap* retval = lock_bitmap_from_ref_anchor(r,anchor);
   RefUnlock(r);
   return retval;
}

#pragma mark -

#pragma scheduling off
#pragma global_optimizer off

errtype master_load_bitmap_from_res(grs_bitmap *bmp, Id id_num, int i, RefTable *rt, bool tmp_mem, LGRect *anchor, uchar *p)
{
   Ref rid;
   FrameDesc *f;
   bool alloced_fdesc = FALSE;
   extern int memcount;

   if(!RefIndexValid(rt,i))
   {
//      Warning(("Bitmap index %i invalid!\n",i));
      return(ERR_FREAD);
   }

   rid = MKREF(id_num,i);
   if (RefSize(rt,i) > FRAME_BUFFER_SIZE)
   {
//      Warning(("damn, we have to malloc...need %d, buffer = %d\n",RefSize(rt,i),FRAME_BUFFER_SIZE));
      f = (FrameDesc *)NewPtr(RefSize(rt,i));
      alloced_fdesc = TRUE;
   }
   else
   {
//      mprintf("look, Refsize is only %d!\n",RefSize(rt,i));
      f = (FrameDesc *)frameBuffer;
   }
   memcount += RefSize(rt,i);
   if (f == NULL)
   {
//      Warning(("Could not load bitmap from resource #%d!\n",id_num));
      return(ERR_FREAD);
   }
   RefExtract(rt,rid,f);
   if (anchor != NULL)
      *anchor = f->anchorArea;
   if (!tmp_mem && p == NULL)
      p = (uchar *)NewPtr(f->bm.w * f->bm.h * sizeof(uchar));
   if (tmp_mem)
      p = (uchar *)(f+1);

   memcount += f->bm.w * f->bm.h * sizeof(uchar);
   if (!tmp_mem) LG_memcpy(p,f+1,f->bm.w * f->bm.h * sizeof(uchar));
//���
if (bmp == NULL)
	DebugStr("\pTrying to assign to a null bmp pointer!");
//���
   *bmp = f->bm;
   bmp->bits = p;
   if (alloced_fdesc)
      DisposePtr((Ptr)f);
   return(OK);
}

#pragma scheduling reset
#pragma global_optimizer reset


#pragma mark -

errtype load_bitmap_from_res(grs_bitmap *bmp, Id id_num, int i, RefTable *rt, bool /*transp*/, LGRect *anchor, uchar *p)
{
   return master_load_bitmap_from_res(bmp,id_num,i,rt,FALSE,anchor,p);
}



errtype load_res_bitmap(grs_bitmap* bmp, Ref rid,bool alloc)
{
   errtype retval;
   RefTable *rt = ResReadRefTable(REFID(rid));
   retval = master_load_bitmap_from_res(bmp, REFID(rid), REFINDEX(rid), rt, FALSE, NULL,(alloc) ? NULL : bmp->bits);
   ResFreeRefTable(rt);
   return(retval);
}

errtype extract_temp_res_bitmap(grs_bitmap* bmp, Ref rid)
{
   errtype retval;
   RefTable *rt = ResReadRefTable(REFID(rid));
   retval = master_load_bitmap_from_res(bmp, REFID(rid), REFINDEX(rid), rt, TRUE, NULL,NULL);
   ResFreeRefTable(rt);
   return(retval);
}


#ifdef SIMPLER_NONEXTRACTING_WAY
errtype load_res_bitmap(grs_bitmap* bmp, Ref rid,bool alloc)
{
   errtype retval = OK;
   char* bits = bmp->bits;
   FrameDesc* f;
   int sz;
   extern int memcount;

   f = RefLock(rid);
   sz = f->bm.w *f->bm.h;
   if (alloc)
   {
      bits = Malloc(sz);
      if (bits == NULL) { retval = ERR_NOMEM; goto out; }
   }
   LG_memcpy(bits,(char*)(f+1),sz);
   *bmp = f->bm;
   bmp->bits = bits;
out:
   RefUnlock(rid);
   return retval;
}
#endif

errtype simple_load_res_bitmap(grs_bitmap* bmp, Ref rid)
{
   return load_res_bitmap(bmp,rid,TRUE);
}

#pragma mark -

errtype load_res_bitmap_cursor(LGCursor* c, grs_bitmap* bmp, Ref rid, bool alloc)
{
   errtype retval = OK;  
   LGRect anchor;
   extern int memcount;

   RefTable *rt = ResReadRefTable(REFID(rid));
#ifdef SVGA_SUPPORT
   short w,h;
   short temp;
   uchar *bits;
   grs_bitmap temp_bmp;
   grs_canvas temp_canv;
   uchar old_over = gr2ss_override;
   ss_set_hack_mode(2,&temp);
   
   gr2ss_override = OVERRIDE_ALL;
   master_load_bitmap_from_res(&temp_bmp, REFID(rid), REFINDEX(rid), rt, FALSE, &anchor,NULL);
   w = temp_bmp.w;
   h = temp_bmp.h;
   ss_point_convert(&w,&h,FALSE);
   if (alloc)
      bits = (uchar *)NewPtr(sizeof(char)*w*h);
   else
      bits = bmp->bits;
   if (temp_bmp.bits == NULL)
      critical_error(CRITERR_MEM|5);
   gr_init_bm(bmp,bits,BMT_FLAT8,BMF_TRANS,w,h);
   gr_make_canvas(bmp,&temp_canv);
   gr_push_canvas(&temp_canv);
   gr_clear(0);
   ss_bitmap(&temp_bmp,0,0);
   DisposePtr((Ptr)temp_bmp.bits);
   ResFreeRefTable(rt);
   if (convert_use_mode)
   {
      anchor.ul.x = (SCONV_X(anchor.ul.x) + SCONV_X(anchor.ul.x + 1)) / 2;
      anchor.ul.y = (SCONV_Y(anchor.ul.y) + SCONV_Y(anchor.ul.y + 1)) / 2;
   }
//   gr_set_pixel(34,anchor.ul.x,anchor.ul.y);  // test test test
   gr_pop_canvas();
   retval = uiMakeBitmapCursor(c,bmp,anchor.ul);
   ss_set_hack_mode(0,&temp);
   gr2ss_override = old_over;
#else
   retval = master_load_bitmap_from_res(bmp, REFID(rid), REFINDEX(rid), rt, FALSE, &anchor,(alloc) ? NULL : bmp->bits);
   ResFreeRefTable(rt);
   retval = uiMakeBitmapCursor(c,bmp,anchor.ul);
#endif
   return retval;
}

errtype simple_load_res_bitmap_cursor(LGCursor* c, grs_bitmap* bmp, Ref rid)
{
   return load_res_bitmap_cursor(c,bmp,rid,TRUE);
}

errtype load_hires_bitmap_cursor(LGCursor* c, grs_bitmap* bmp, Ref rid, bool alloc)
{
	errtype retval = OK;  
	LGRect anchor;
	
	RefTable *rt = ResReadRefTable(REFID(rid));
	retval = master_load_bitmap_from_res(bmp, REFID(rid), REFINDEX(rid), rt, FALSE, &anchor,
															   (alloc) ? NULL : bmp->bits);
	ResFreeRefTable(rt);
	retval = uiMakeBitmapCursor(c,bmp,anchor.ul);
}

/*
void *CitMalloc(int n)
{
   return(Malloc(n));
}

void CitFree(void *p)
{
   Free(p);
}

*/