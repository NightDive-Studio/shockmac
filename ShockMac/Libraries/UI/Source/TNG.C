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

#include "tng.h"
#include "tngapp.h"
#include "rect.h"
#include "2d.h"
//#include <_ui.h>

//	TNGDrawBase() draws base graphics for a TNG gadget.  All
//	TNG gadgets should call this function to draw their base as
//	the first thing in their draw function, unless the
//	TNG gadget's graphics fully and non-transparently occupy the
//	gadget area.  Partial-draw routines may skip this call
//	if they are drawing areas which don't need the gadget backdrop.

LGPoint tngZeroPt = {0,0};		// everybody needs a zero point sometime

TNGStyle stdTNGStyle = {
   0x25C, NULL, {6,6}, 0xff,
   0x01, 0x1d, 0,
   0x20,0x28,0x28,0x20};

errtype TNGInit(TNG *ptng, TNGStyle *sty, void *ui_data)
{
   if (sty != NULL)
      ptng->style = sty;
   else
      ptng->style = &stdTNGStyle;
   ptng->ui_data = ui_data;
   ptng->pcb = NULL;
   ptng->draw_func = &tng_cb_draw;
   ptng->mousebutt = &tng_cb_mousebutt;
   ptng->mousemove = &tng_cb_mousemove;
   ptng->keycooked = &tng_cb_keycooked;
   ptng->keyscan   = &tng_cb_keyscan;
   ptng->signal    = &tng_cb_signal;
   ptng->flags     = 0;
   return(OK);
}

void TNGDrawBase(TNG *ptng, LGPoint coord, LGPoint size)
{
	short width,height;
	TNGStyle *pstyle;
	LGRect rect;                        
   ushort flags;
	int i;

//	If bitmap background, draw it
//	Else if tiled background, draw it
//	Else if background color, draw it
//	Else if gadget has transparencies, draw dialog underneath
   
	pstyle = ptng->style;
   flags = ptng->flags;
   width = size.x;
   height = size.y; 
	if (flags & TNG_BMAPBACK)
		TNGDrawBitmapRef(pstyle->bitmapRef, coord);
	else if (flags & TNG_TILEBACK)
		TNGDrawTileMapRef(pstyle->bitmapRef, coord);
	else if (pstyle->backColor)
   {
      gr_set_fcolor(pstyle->backColor);
      gr_rect(coord.x, coord.y, coord.x + width, coord.y + height);
   }

//	If bevel border, draw it

	if ((flags & TNG_NOBORDER) == 0)
		{
		if (flags & TNG_BEVEL)
			{
	
			gr_cset_fcolor(grd_canvas,
				pstyle->bordColor[(flags & TNG_BEVELMASK) >> TNG_BEVELSHIFT]);
         gr_hline(coord.x, coord.y, coord.x + width - 1);
         gr_vline(coord.x, coord.y, coord.y + height - 1);

			gr_cset_fcolor(grd_canvas,
				pstyle->bordColor[((flags ^ TNG_INVERTBEVEL) &
					TNG_BEVELMASK) >> TNG_BEVELSHIFT]);
         gr_hline(coord.x, coord.y + height - 1, coord.x + width - 1);
         gr_vline(coord.x + width - 1, coord.y, coord.y + height - 1);
			}

//	Else if fat border, draw it

		else if (flags & TNG_FATBORDER)
			{
//			rect = *(TNG_AREARECT(ptng));
//         rect.ul = tngZeroPt;
         rect.ul = coord;
         rect.lr.x = rect.ul.x + width;
         rect.lr.y = rect.lr.x + height;
			for (i = 0; i < 4; i++)
				{
				gr_cset_fcolor(grd_canvas, pstyle->bordColor[i]);
            gr_box(rect.ul.x, rect.ul.y, rect.lr.x, rect.lr.y);
				rect.ul.x++;
				rect.ul.y++;
				rect.lr.x--;
				rect.lr.y--;
				}
			}
		}
}


//	TNGDrawBitmapRef() draws a bitmap of any kind, given ref.
//
//		ref = reference to bitmap (high word is res, low word is frame #)
//		pt  = point in current canvas at which to place u.l. of bitmap

void TNGDrawBitmapRef(Ref ref, LGPoint pt)
{
	FrameDesc *pfd;
	LGRect bitRect;
	LGRect cvRect;

	pfd = (FrameDesc *) RefLock(ref);
	pfd->bm.bits = (uchar *) (pfd + 1);

	bitRect.ul = pt;
	bitRect.lr.x = bitRect.ul.x + pfd->bm.w;
	bitRect.lr.y = bitRect.ul.y + pfd->bm.h;
	cvRect.ul.x = grd_clip.left;
	cvRect.ul.y = grd_clip.top;
	cvRect.lr.x = grd_clip.right + 1;
	cvRect.lr.y = grd_clip.bot + 1;
   gr_bitmap(&pfd->bm, pt.x, pt.y);

	RefUnlock(REFID(ref));
}

//	-----------------------------------------------------------
//
//	TNGDrawTileMapRef() draws a bitmap repeated in the clip area.
//
//		ref = reference to bitmap (high word is res, low word is frame #)
//		pt  = point at which to render first bitmap; this routine tiles
//				from this point to l.r. of cliprect.

void TNGDrawTileMapRef(Ref ref, LGPoint pt)
{
	FrameDesc *pfd;
	short startx;

	pfd = (FrameDesc *) RefLock(ref);
	pfd->bm.bits = (uchar *) (pfd + 1);

	while ((pt.y + pfd->bm.h) < grd_clip.top)
		pt.y += pfd->bm.h;
	while ((pt.x + pfd->bm.w) < grd_clip.left)
		pt.x += pfd->bm.w;

	while (pt.y < grd_clip.bot)
		{
		startx = pt.x;
		while (pt.x < grd_clip.right)
			{
         gr_bitmap(&pfd->bm, pt.x, pt.y);
			pt.x += pfd->bm.w;
			}
		pt.x = startx;
		pt.y += pfd->bm.h;
		}

	RefUnlock(REFID(ref));
}

errtype TNGDrawText(Id id, char *text, int x, int y)
{
   gr_set_font((grs_font *)ResLock(id));
   gr_string(text, x, y);
   ResUnlock(id);
   return(OK);
}

// --------------------------------------------
// Callback Functions
// --------------------------------------------

errtype tng_install_callback(TNG *ptng, ushort event_type, ushort cond, TNGCallback cbfn, void *user_data, int *pid)
{
   TNG_CB *tngcb, *curp, *oldp;
   int oldid = 0;

   if (ptng == NULL)
   {
//      Spew(DSRC_UI_Bounds, ("Attempted to install callback on NULL TNG!\n"));
      return(ERR_NULL);
   }

   tngcb = (TNG_CB *)NewPtr(sizeof(TNG_CB));
   tngcb->condition = cond;
   tngcb->user_data = user_data;
   tngcb->cb = cbfn;
   tngcb->event_type = event_type;
   tngcb->next_cb = NULL;

   curp = ptng->pcb;
   oldp = NULL;
   while (curp != NULL)
   {
      oldp = curp;
      curp = curp->next_cb;
   }
   if (oldp == NULL)
      ptng->pcb = tngcb;
   else
   {
      oldp->next_cb = tngcb;
      oldid = oldp->id + 1;
   }
   tngcb->id = oldid;
   *pid = oldid;
   return (OK);
}

errtype tng_uninstall_callback(TNG *ptng, int id)
{
   TNG_CB *curp, *oldp;

   if (ptng == NULL)
   {
//      Spew(DSRC_UI_Bounds, ("Attempted to remove callback from null TNG!\n"));
      return(ERR_NULL);
   }

   curp = ptng->pcb;
   oldp = NULL;
   while (curp->id != id)
   {
      oldp = curp;
      curp = curp->next_cb;
      if (curp == NULL)
         return(ERR_RANGE);
   }
   if (oldp == NULL)
   {
      ptng->pcb = curp->next_cb;
   }
   else
   {
      oldp->next_cb = curp->next_cb;
   }
   DisposePtr((Ptr)curp);
   return(OK);
}

bool tng_cb_mousebutt(TNG *ptng, uchar type, LGPoint loc)
{
   TNG_CB *tngcb;
   bool retval = FALSE;
   bool newret;

   tngcb = ptng->pcb;
   while (tngcb != NULL)
   {
//      Spew(DSRC_TNG_Callback, ("In while part of tng_cb_mousebutt...\n"));
      if (tngcb->event_type == TNG_EVENT_MOUSE)
      {
         newret = FALSE;
         if (type & tngcb->condition)
         {
            ptng->cb_data = &loc;
            newret = tngcb->cb(ptng->ui_data, tngcb->user_data);
         }
         if (newret)
            retval = newret;
      }
      tngcb = tngcb->next_cb;
   }
   return(retval);
}

bool tng_cb_keycooked(TNG *ptng, ushort key)
{
   TNG_CB *tngcb;
   bool retval = FALSE;
   bool newret;

   tngcb = ptng->pcb;
   while (tngcb != NULL)
   {
      if (tngcb->event_type == TNG_EVENT_KBD_COOKED)
      {
         newret = FALSE;
         if (key == tngcb->condition)
         {
            ptng->cb_data = (void *)key;
            newret = tngcb->cb(ptng->ui_data, tngcb->user_data);
         }
         if (newret)
            retval = newret;
      }
      tngcb = tngcb->next_cb;
   }
   return(retval);
}

bool tng_cb_signal(TNG *ptng, ushort signal)
{
   TNG_CB *tngcb;
   bool retval = FALSE;
   bool newret;

   tngcb = ptng->pcb;
   while (tngcb != NULL)
   {
      if (tngcb->event_type == TNG_EVENT_SIGNAL)
      {
         newret = FALSE;
         if (signal & tngcb->condition)
         {
            ptng->cb_data = (void *)signal;
            newret = tngcb->cb(ptng->ui_data, tngcb->user_data);
         }
         if (newret)
            retval = newret;
      }
      tngcb = tngcb->next_cb;
   }
   return(retval);
}

bool tng_cb_keyscan(TNG *ptng, ushort scan)
{
   TNG_CB *tngcb;
   bool retval = FALSE;
   bool newret;

   tngcb = ptng->pcb;
   while (tngcb != NULL)
   {
      if (tngcb->event_type == TNG_EVENT_KBD_RAW)
      {
         newret = FALSE;
         if (scan == tngcb->condition)
         {
            ptng->cb_data = (void *)scan;
            newret = tngcb->cb(ptng->ui_data, tngcb->user_data);
         }
         if (newret)
            retval = newret;
      }
      tngcb = tngcb->next_cb;
   }
   return(retval);
}

bool tng_cb_mousemove(TNG *ptng, LGPoint loc)
{
   TNG_CB *tngcb;
   bool retval = FALSE;
   bool newret;
   LGPoint dummy;
   dummy = loc;

   tngcb = ptng->pcb;
   while (tngcb != NULL)
   {
      if (tngcb->event_type == TNG_EVENT_MOUSE_MOVE)
      {
         ptng->cb_data = &loc;
         newret = tngcb->cb(ptng->ui_data, tngcb->user_data);
         if (newret)
            retval = newret;
      }
      tngcb = tngcb->next_cb;
   }
   return(retval);
}

errtype tng_cb_draw(TNG *ptng, ushort partmask, LGPoint loc)
{
#ifndef NO_DUMMIES
   LGPoint dummy; ushort dummy2; dummy2 = partmask; dummy = loc;
#endif
   ptng->signal(ptng, TNG_SIGNAL_EXPOSE);
   return(OK);
}

LGPoint tng_absloc(TNG* ptng)
{
   LGRegion* r= ((Gadget*)(ptng->ui_data))->rep;
   LGPoint pt;
   pt.x = r->abs_x;
   pt.y = r->abs_y;
   return pt;
}
