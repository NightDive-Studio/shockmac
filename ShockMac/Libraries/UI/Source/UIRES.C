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
 * $Source: r:/prj/lib/src/ui/RCS/uires.c $
 * $Revision: 1.2 $
 * $Author: mahk $
 * $Date: 1994/11/18 17:56:19 $
 *
 */

#include "uires.h"
#include <string.h>

// Prototypes
errtype master_load_bitmap_from_res(grs_bitmap *bmp, Id id_num, int i, RefTable *rt,
									bool tmp_mem, LGRect *anchor, uchar *p);

struct _uirestempbuffer uiResTempBuffer;

errtype master_load_bitmap_from_res(grs_bitmap *bmp, Id id_num, int i, RefTable *rt, bool tmp_mem, LGRect *anchor, uchar *p)
{
   Ref rid;
   FrameDesc *f;
   bool alloced_fdesc = FALSE;
//   extern int memcount;

   if(!RefIndexValid(rt,i)) {
//      Warning(("Bitmap index %i invalid!\n",i));
      return(ERR_FREAD);
   }

   rid = MKREF(id_num,i);
   if (uiResTempBuffer.mem == NULL || RefSize(rt,i) > uiResTempBuffer.size)
   {

//      Warning(("damn, we have to malloc...need %d, buffer = %d\n",RefSize(rt,i),uiResTempBuffer.size));
      f = (FrameDesc *)NewPtr(RefSize(rt,i));
      alloced_fdesc = TRUE;
   }
   else
   {
      f = (FrameDesc *)uiResTempBuffer.mem;
   }
//   memcount += RefSize(rt,i);
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
      p = (uchar*)(f+1);

//   memcount += f->bm.w * f->bm.h * sizeof(uchar);
   if (!tmp_mem) memcpy(p,f+1,f->bm.w * f->bm.h * sizeof(uchar));
   *bmp = f->bm;
   bmp->bits = p;
   if (alloced_fdesc)
      DisposePtr((Ptr)f);
   return(OK);
}

errtype uiLoadRefBitmapCursor(LGCursor* c, grs_bitmap* bmp, Ref rid, bool alloc)
{
   errtype retval = OK;  
   LGRect anchor;
//   extern int memcount;
   bool buffer_snag = FALSE;

   int numrefs = ResNumRefs(REFID(rid));
   int tsize = REFTABLESIZE(numrefs);

   RefTable *rt = NULL;
   if (uiResTempBuffer.mem != NULL && tsize <= uiResTempBuffer.size)
   {
      rt = (RefTable*)uiResTempBuffer.mem;
      uiResTempBuffer.mem += tsize;
      uiResTempBuffer.size -= tsize;
      ResExtractRefTable(REFID(rid),rt,tsize);
      buffer_snag = TRUE;
   }
   else rt = ResReadRefTable(REFID(rid));
   retval = master_load_bitmap_from_res(bmp, REFID(rid), REFINDEX(rid), rt, FALSE, &anchor,(alloc) ? NULL : bmp->bits);
   if (buffer_snag)
   {
      uiResTempBuffer.mem = (char*)rt;
      uiResTempBuffer.size += tsize;
   }
   else ResFreeRefTable(rt);
   retval = uiMakeBitmapCursor(c,bmp,anchor.ul);
   return retval;
}
