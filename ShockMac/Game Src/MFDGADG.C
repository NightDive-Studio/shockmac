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
 * $Source: n:/project/cit/src/RCS/mfdgadg.c $
 * $Revision: 1.5 $
 * $Author: mahk $
 * $Date: 1994/02/16 09:26:50 $
 *
 * $Log: mfdgadg.c $
 * Revision 1.5  1994/02/16  09:26:50  mahk
 * Hey, fixed my goofy bugs.
 * 
 * Revision 1.4  1994/02/16  09:19:57  mahk
 * Wrote goofy resizing code.
 * 
 * Revision 1.3  1994/01/15  15:12:04  mahk
 * Changed buttonarray spacing. 
 * 
 * Revision 1.2  1993/10/20  05:41:33  mahk
 * Added a slider gadget.
 * 
 * Revision 1.1  1993/09/15  10:54:30  mahk
 * Initial revision
 * 
 *
 */

#include <stdlib.h>

#include "mfdgadg.h"

extern void mouse_unconstrain(void);
bool mfd_buttonarray_handlerproc(MFD* mfd, uiEvent* ev, MFDhandler* h);
bool mfd_slider_handler(MFD* mfd, uiMouseEvent* ev, MFDhandler* h);

// =======================
// BUTTON ARRAYS
// =======================

// ---------
// INTERNALS
// ---------


typedef struct _mfd_bttnarray
{
   LGPoint bdims;  // Diminsions of button array
   LGPoint bspace; // pixel spacing between buttons.
   LGPoint bsize;   // pixel size of buttons.  
   MFDBttnCallback cb;
   void* cbdata;
}  MFDBttnArray;


bool mfd_buttonarray_handlerproc(MFD* mfd, uiEvent* ev, MFDhandler* h)
{
   MFDBttnArray* ba = (MFDBttnArray*)(h->data);
   LGPoint bttn;
   LGPoint pos;
   if (ev->type != UI_EVENT_MOUSE || !(ev->subtype & ~MOUSE_MOTION))
      return FALSE;
   pos.x = ev->pos.x - mfd->rect.ul.x - h->r.ul.x;
   pos.y = ev->pos.y - mfd->rect.ul.y - h->r.ul.y;
   // Make sure its on a button, not a space. 
   if (pos.x % (ba->bspace.x + ba->bsize.x) >= ba->bsize.x
    || pos.y % (ba->bspace.y + ba->bsize.y) >= ba->bsize.y)
      return FALSE;
   bttn.x = min(pos.x / (ba->bspace.x + ba->bsize.x),ba->bdims.x-1);
   bttn.y = min(pos.y / (ba->bspace.y + ba->bsize.y),ba->bdims.y-1);
   return ba->cb(mfd,bttn,ev,ba->cbdata);
}

// ---------
// EXTERNALS
// ---------

errtype MFDBttnArrayInit(MFDhandler* h, LGRect* r, LGPoint bdims, LGPoint bsize, MFDBttnCallback cb, void* cbdata)
{
   MFDBttnArray* ba = (MFDBttnArray *)NewPtr(sizeof(MFDBttnArray));
   if (ba == NULL) return ERR_NOMEM;
   if (bsize.x < 1 || bsize.y < 1) return ERR_RANGE;
   h->r = *r;
   h->data = ba;
   h->proc = mfd_buttonarray_handlerproc;
   ba->bdims = bdims;
   ba->bsize = bsize;
   if (bdims.x > 1)
      ba->bspace.x = (RectWidth(r) -bsize.x)/(bdims.x-1) - bsize.x;
   else ba->bspace.x = RectWidth(r) - bsize.x;
   if (bdims.y > 1)
      ba->bspace.y = (RectHeight(r)-bsize.y)/(bdims.y-1) - bsize.y;
   else ba->bspace.y = RectHeight(r) - bsize.y;
   ba->cb = cb;
   ba->cbdata = cbdata;
   return OK;
}

errtype MFDBttnArrayShutdown(MFDhandler* h)
{
   DisposePtr((Ptr)h->data);
   h->proc = NULL;
   return OK;
}

errtype MFDBttnArrayResize(MFDhandler* h, LGRect* r, LGPoint bdims, LGPoint bsize)
{
   MFDBttnArray* ba = (MFDBttnArray*)h->data;
   if (bsize.x < 1 || bsize.y < 1) return ERR_RANGE;
   h->r = *r;
   ba->bdims = bdims;
   ba->bsize = bsize;
   if (bdims.x > 1)
      ba->bspace.x = (RectWidth(r) -bsize.x)/(bdims.x-1) - bsize.x;
   else ba->bspace.x = RectWidth(r) - bsize.x;
   if (bdims.y > 1)
      ba->bspace.y = (RectHeight(r)-bsize.y)/(bdims.y-1) - bsize.y;
   else ba->bspace.y = RectHeight(r) - bsize.y;
   return OK;
}



// ======================
//        SLIDERS
// ======================

// ---------
// INTERNALS
// ---------

typedef struct _mfd_slider
{
   MFDSliderCallback cb;
   void* data;
   bool bttndown; 
}  MFDSlider;


bool mfd_slider_handler(MFD* mfd, uiMouseEvent* ev, MFDhandler* h)
{
   short x = mfd->rect.ul.x + h->r.ul.x;
   short y = mfd->rect.ul.y + h->r.ul.y;
   bool retval = TRUE;
   MFDSlider* sl = (MFDSlider*)(h->data);
   LGPoint pos = ev->pos;
   pos.x -= x;
   pos.y -= y;
   if (ev->type != UI_EVENT_MOUSE && ev->type != UI_EVENT_MOUSE_MOVE)
      return FALSE;
   if (ev->action & MOUSE_LDOWN)
   {
      mouse_constrain_xy(x,y,x+RectWidth(&h->r)-1,y+RectHeight(&h->r)-1);
      sl->bttndown = TRUE;
   }
   if (sl->bttndown)
   {
      retval = sl->cb(mfd,pos.x,(uiEvent*)ev,sl->data);
   }
   if (!(ev->buttons & (1 << MOUSE_LBUTTON)))
   {
      sl->bttndown = FALSE;
      mouse_unconstrain();
   }
   return retval;
}

// ---------
// EXTERNALS
// ---------

errtype MFDSliderInit(MFDhandler* h, LGRect* r, MFDSliderCallback cb, void* data)
{
   MFDSlider* sl = (MFDSlider *)NewPtr(sizeof(MFDSlider));
   if (sl == NULL) return ERR_NOMEM;
   h->r = *r;
   h->data = sl;
   h->proc =(MFD_handlerProc) mfd_slider_handler;
   sl->cb = cb;
   sl->data = data;
   sl->bttndown = FALSE;
   return OK;
}
