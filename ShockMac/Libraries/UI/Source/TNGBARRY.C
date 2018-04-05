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
#include <stdio.h>
#include <string.h>

#include "barrykey.h"
#include "tngbarry.h"
//#include <_ui.h>
#include "tngslidr.h"
#include "kbcook.h"
//#include <mprintf.h>

//-------------------------
// PROTOTYPES
//-------------------------
errtype tng_buttonarray_move(TNG *ptng, short code);
bool tng_buttonarray_hscroll_changed(void *ui_data, void *user_data);
bool tng_buttonarray_vscroll_changed(void *ui_data, void *user_data);


// Callbacks....
bool tng_buttonarray_hscroll_changed(void *ui_data, void *user_data)
{
   TNG *ptng, *scroll_tng;
   void *dummy;
   dummy = ui_data;

   ptng = (TNG *)user_data;
   scroll_tng = TNG_BA(ptng)->hscroll_tng;
   TNG_BA_OFFSET(ptng).x = TNG_SL(scroll_tng)->value;
   TNG_DRAWPART(ptng, TNG_ALLPARTS);
   ptng->signal(ptng, TNG_SIGNAL_CHANGED);
   return(FALSE);
}

bool tng_buttonarray_vscroll_changed(void *ui_data, void *user_data)
{
   TNG *ptng, *scroll_tng;
   void *dummy;
   dummy = ui_data;

   ptng = (TNG *)user_data;
   scroll_tng = TNG_BA(ptng)->vscroll_tng;
   TNG_BA_OFFSET(ptng).y = TNG_SL(scroll_tng)->max - TNG_SL(scroll_tng)->value;
   TNG_DRAWPART(ptng, TNG_ALLPARTS);
   ptng->signal(ptng, TNG_SIGNAL_CHANGED);
   return(FALSE);
}

errtype tng_buttonarray_destroy(TNG *ptng)
{
   DisposePtr((Ptr)TNG_BA(ptng)->selected);
   DisposePtr((Ptr)TNG_BA(ptng)->matrix);
   GUI_DEALLOC(ptng->ui_data, ptng->type_data);
   return(OK);
}

// Initializes the TNG 
errtype tng_buttonarray_init(void *ui_data, TNG *ptng, TNGStyle *sty, ushort options, LGPoint msize, LGPoint wsize, LGPoint bsize,
   int num_sel)
{
   TNG_buttonarray *pbatng;
   int i,j;

   pbatng = (TNG_buttonarray *)GUI_MALLOC(ptng->ui_data, sizeof(TNG_buttonarray));

   TNGInit(ptng,sty,ui_data);
   ptng->flags = TNG_BEVEL;
   ptng->type_data = pbatng;
   ptng->draw_func = &tng_buttonarray_2d_draw;
   ptng->mousebutt = &tng_buttonarray_mousebutt;
   ptng->keycooked = &tng_buttonarray_keycooked;
   ptng->signal = &tng_buttonarray_signal;

   pbatng->tng_data = ptng;
   if (wsize.y < msize.y)
      pbatng->scroll_size.x = TNG_BA_SCROLL_X;
   else
      pbatng->scroll_size.x = 0;
   if (wsize.x < msize.x)
      pbatng->scroll_size.y = TNG_BA_SCROLL_Y;
   else
      pbatng->scroll_size.y = 0;
   if (options & TNG_BA_TIGHTPACK)
   {
      pbatng->spacing = 1;
   }
   else if (options & TNG_BA_LOOSEPACK)
   {
      pbatng->spacing = 2;
   }
   else if (num_sel <=1)
   {
      pbatng->spacing = 1;
   }
   else
   {
      pbatng->spacing = 2;
   }
   pbatng->size.x = (wsize.x * (bsize.x + TNG_BA_SPACING(ptng))) + pbatng->scroll_size.x + TNG_BA_SELECT_SIZE + (2 * TNG_BA_BORDER_WIDTH);
   pbatng->size.y = (wsize.y * (bsize.y + TNG_BA_SPACING(ptng))) + pbatng->scroll_size.y + TNG_BA_SELECT_SIZE + (2 * TNG_BA_BORDER_WIDTH);

   pbatng->bsize = bsize;
   pbatng->msize = msize;
   pbatng->wsize = wsize;
   if (options == TNG_BA_NO_OPTIONS)
      pbatng->options = TNG_BA_OUTLINE_MODE;
   else
      pbatng->options = options;
   pbatng->offset = tngZeroPt;
   pbatng->lsel = pbatng->offset;
   pbatng->num_selectable = num_sel;
   pbatng->matrix = (TNGButtonArrayElement *)NewPtr(sizeof(TNGButtonArrayElement) * (msize.x * msize.y));
   pbatng->selected = (bool *)NewPtr(sizeof(bool) * (msize.x * msize.y));
   for (i=0; i<msize.x; i++)
   {
      for (j=0; j<msize.y; j++)
      {
         pbatng->matrix[i + (j * msize.x)].type = NULL_TYPE;
         pbatng->matrix[i + (j * msize.x)].disp_data = NULL;
         pbatng->selected[i + (j * msize.x)] = FALSE;
      }
   }
   return(OK);
}

errtype tng_buttonarray_init2(TNG *ptng)
{
   TNG_buttonarray *pbatng;
   int id;
   LGPoint sloc, ssize;

   ptng->signal(ptng, TNG_SIGNAL_EXPOSE);
   pbatng = TNG_BA(ptng);

   if (pbatng->scroll_size.y > 0)
   {
      sloc.x = pbatng->scroll_size.x + TNG_BA_BORDER_WIDTH;
      sloc.y = TNG_BA_BORDER_WIDTH;
      ssize.x = pbatng->size.x - sloc.x - TNG_BA_BORDER_WIDTH;
      ssize.y = pbatng->scroll_size.y;
      TNG_CREATE_SLIDER(ptng->ui_data, sloc, &(pbatng->hscroll_tng), ptng->style, TNG_SL_HORIZONTAL, 0,
         pbatng->msize.x - pbatng->wsize.x, 0, 1, ssize);
      TNG_SL(pbatng->hscroll_tng)->value = pbatng->offset.x;
      pbatng->hscroll_tng->signal(pbatng->hscroll_tng, TNG_SIGNAL_CHANGED);
      tng_install_callback(pbatng->hscroll_tng, TNG_EVENT_SIGNAL, TNG_SIGNAL_CHANGED, &tng_buttonarray_hscroll_changed, ptng, &id);
   }
   else
   {
      pbatng->hscroll_tng = NULL;
   }
   if (pbatng->scroll_size.x > 0)
   {
      sloc.x = TNG_BA_BORDER_WIDTH;
      sloc.y = pbatng->scroll_size.y + TNG_BA_BORDER_WIDTH;
      ssize.x = pbatng->scroll_size.x;
      ssize.y = pbatng->size.y - sloc.y - TNG_BA_BORDER_WIDTH;
      TNG_CREATE_SLIDER(ptng->ui_data, sloc, &(pbatng->vscroll_tng), ptng->style, TNG_SL_VERTICAL, 0,
         pbatng->msize.y - pbatng->wsize.y, 0, 1, ssize);
      TNG_SL(pbatng->vscroll_tng)->value = pbatng->msize.y - pbatng->wsize.y - pbatng->offset.y;
      pbatng->vscroll_tng->signal(pbatng->vscroll_tng, TNG_SIGNAL_CHANGED);
      tng_install_callback(pbatng->vscroll_tng, TNG_EVENT_SIGNAL, TNG_SIGNAL_CHANGED, &tng_buttonarray_vscroll_changed, ptng, &id);
   }
   else
   {
      pbatng->vscroll_tng = NULL;
   }

   return(OK);
}

// Note that i & j here are in WINDOW coordinates, NOT matrix coords!
errtype tng_buttonarray_draw_button(TNG *ptng, int i, int j)
{
   LGRect brect, isect, r;
   Ref id;
   int ci,cj;
   TNGButtonArrayElement el;
   int diff;
   TNG_buttonarray *pbatng = TNG_BA(ptng);
   LGPoint loc = TNG_ABSLOC(ptng);
   LGRect clip;

   r.ul.x = loc.x + TNG_BA_BORDER_WIDTH;
   r.ul.y = loc.y + TNG_BA_BORDER_WIDTH; 
   r.lr.x = r.ul.x + pbatng->size.x;
   r.lr.y = r.ul.y + pbatng->size.y;
   ci = i + pbatng->offset.x;
   cj = j + pbatng->offset.y;
   //Spew(DSRC_UI_Buttonarray, ("drawing button at matrix coords (%d, %d) loc = (%d,%d)\n",ci,cj,loc.x,loc.y));
   el = pbatng->matrix[ci + (cj * pbatng->msize.x)];
   brect.ul.x = r.ul.x + pbatng->scroll_size.x + TNG_BA_SELECT_SIZE + ((pbatng->bsize.x + TNG_BA_SPACING(ptng)) * i);
   brect.ul.y = r.ul.y + pbatng->scroll_size.y + TNG_BA_SELECT_SIZE + ((pbatng->bsize.y + TNG_BA_SPACING(ptng)) * j);
   brect.lr.x = brect.ul.x + pbatng->bsize.x;
   brect.lr.y = brect.ul.y + pbatng->bsize.y;
   uiHideMouse(&brect);
   STORE_CLIP(clip.ul.x,clip.ul.y,clip.lr.x,clip.lr.y);
//   if (RectSect(&brect, &clip, &isect))
   isect = brect;
      gr_set_cliprect(isect.ul.x - 1, isect.ul.y -1, isect.lr.x +1, isect.lr.y +1);
   TNGDrawBase(ptng, loc, pbatng->size);
   //Spew(DSRC_UI_Buttonarray, ("isect = (%d, %d) - (%d,%d)\n",RECT_EXPAND_ARGS(&isect)));
   diff = (RectHeight(&brect) - TNG_BA_CHECKBOX_SIZE) / 2;
   if ((pbatng->lsel.x == i) && (pbatng->lsel.y == j))
   {
      gr_set_fcolor(ptng->style->altBackColor);
      gr_box(brect.ul.x - TNG_BA_SELECT_SIZE, brect.ul.y - TNG_BA_SELECT_SIZE,
            brect.lr.x + TNG_BA_SELECT_SIZE, brect.lr.y + TNG_BA_SELECT_SIZE);
   }
   if (pbatng->selected[ci + (cj * pbatng->msize.x)])
   {
      gr_set_fcolor(ptng->style->altTextColor);
      if (pbatng->options & TNG_BA_CHECKBOX_MODE)
      {
         gr_int_line(brect.ul.x, brect.ul.y + diff,
            brect.ul.x + TNG_BA_CHECKBOX_SIZE, brect.ul.y + diff + TNG_BA_CHECKBOX_SIZE);
         gr_int_line(brect.ul.x, brect.ul.y + diff + TNG_BA_CHECKBOX_SIZE,
            brect.ul.x + TNG_BA_CHECKBOX_SIZE, brect.ul.y + diff);
      }
      else
      {
         gr_box(brect.ul.x - TNG_BA_SELECT_SIZE,  brect.ul.y - TNG_BA_SELECT_SIZE ,
            brect.lr.x + TNG_BA_SELECT_SIZE , brect.lr.y + TNG_BA_SELECT_SIZE);
      }
   }
   if (pbatng->options & TNG_BA_CHECKBOX_MODE)
   {
      gr_set_fcolor(ptng->style->textColor);
      gr_box(brect.ul.x, brect.ul.y + diff, brect.ul.x + TNG_BA_CHECKBOX_SIZE + 1, brect.ul.y + diff + TNG_BA_CHECKBOX_SIZE + 1);
      brect.ul.x += TNG_BA_CHECKBOX_SIZE + 2;
   }
   gr_set_cliprect(isect.ul.x, isect.ul.y, isect.lr.x, isect.lr.y);
   switch(el.type)
   {
      case COLORED_TYPE:
         gr_set_fcolor(((int)(el.disp_data)));
         gr_rect(brect.ul.x,brect.ul.y,brect.lr.x,brect.lr.y);
         break;
      case TEXT_TYPE:
         gr_set_fcolor(ptng->style->textColor);
         TNG_DRAW_TEXT(ptng, (char *)el.disp_data, brect.ul.x, brect.ul.y);
         break;
      case RESOURCE_TYPE:
         id = *((Ref *)(el.disp_data));
         draw_resource_bm(id, brect.ul.x, brect.ul.y);
         break;
      case CALLBACK_TYPE:
      {
         LGPoint loc;
         ButtonDrawCallback* cb = (ButtonDrawCallback*)el.disp_data;
         loc.x = i;
         loc.y = j;
         cb->func(&brect,loc,cb->data);
         break;
      }
   }
   RESTORE_CLIP(clip.ul.x,clip.ul.y,clip.lr.x,clip.lr.y);
   uiShowMouse(&brect);
   return(OK);
}

// Draw the specified parts (may be all) of the TNG at screen coordinates loc
// assumes all appropriate setup has already been done!
errtype tng_buttonarray_2d_draw(TNG *ptng, ushort partmask, LGPoint loc)
{
   TNG_buttonarray *pbatng;
   int i,j;

#ifndef NO_DUMMIES
   ushort dummy;   dummy = partmask;
#endif

   //Spew(DSRC_UI_Buttonarray, ("TNG Buttonarray 2d Draw at (%d, %d) -- partmask = %x\n",loc.x,loc.y,partmask));
   TNG_IF_OBSCURED(ptng)
   {
      return(OK);
   }
   pbatng = TNG_BA(ptng);
   TNGDrawBase(ptng, loc, pbatng->size);

   // Iterate through the visible buttons, displaying each as we get to them
   for (i=0; i<pbatng->wsize.x; i++)
   {
      for (j=0; j<pbatng->wsize.y; j++)
      {
         if (!(pbatng->spacing < 2) || !(TNG_BA_SELECTED(ptng, i + TNG_BA_OFFSET(ptng).x,j+TNG_BA_OFFSET(ptng).y)))
            tng_buttonarray_draw_button(ptng,i,j);
      }
   }
   if (pbatng->spacing < 2)
   {
      tng_buttonarray_draw_button(ptng, TNG_BA_LSEL(ptng).x, TNG_BA_LSEL(ptng).y);
      //Spew(DSRC_UI_Buttonarray, ("redrawing selected buttons for 2d draw...\n"));
      for (i=0; i<pbatng->wsize.x; i++)
      {
         for (j=0; j<pbatng->wsize.y; j++)
         {
            //Spew(DSRC_UI_Buttonarray, ("(off: %d,%d)(%d,%d) -- %d\n",TNG_BA_OFFSET(ptng).x, TNG_BA_OFFSET(ptng).y,
            //   i,j,TNG_BA_SELECTED(ptng, i + TNG_BA_OFFSET(ptng).x, j + TNG_BA_OFFSET(ptng).y)));
            if (TNG_BA_SELECTED(ptng,i + TNG_BA_OFFSET(ptng).x,j+ TNG_BA_OFFSET(ptng).y))
               tng_buttonarray_draw_button(ptng,i,j);
         }
      }
   }
   return(OK);
}

// Fill in ppt with the size...
errtype tng_buttonarray_size(TNG *ptng, LGPoint *ppt)
{
   *ppt = TNG_BA(ptng)->size;
   return(OK);
}

// Returns the current "value" of the TNG
int tng_buttonarray_getvalue(TNG *ptng)
{
   return(TNG_BA(ptng)->num_selectable);
}

// React appropriately for receiving the specified cooked key
bool tng_buttonarray_keycooked(TNG *ptng, ushort key)
{
   ushort code = key ^ KB_FLAG_DOWN;
   bool retval = FALSE;

   code = key & 0xff;
//   Spew(DSRC_UI_Buttonarray, ("code = %x\n",code));
/*
{
	char buff[100];
	sprintf(buff+1, "code = %x\0",code);
	buff[0] = strlen(buff+1);
	DebugStr((uchar *)buff);
}
*/
   switch(code)
   {
      case TNG_BA_RETURN_KEY:
         IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_SELECT));
         break;
      case TNG_BA_UP_KEY:
      case TNG_BA_DOWN_KEY:
      case TNG_BA_LEFT_KEY:
      case TNG_BA_RIGHT_KEY:
         tng_buttonarray_move(ptng, code);
         retval = TRUE;
         break;
      case TNG_BA_SCROLL_UP_KEY:
      case TNG_BA_SCROLL_DOWN_KEY:
      case TNG_BA_SCROLL_LEFT_KEY:
      case TNG_BA_SCROLL_RIGHT_KEY:
         TNG_BA_LASTKEY(ptng) = code;
         IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_SCROLL));
         break;
   }
   IF_SET_RV(tng_cb_keycooked(ptng, key));

   return(retval);   
}

// React appropriately for receiving the specified mouse button event
bool tng_buttonarray_mousebutt(TNG *ptng, uchar type, LGPoint loc)
{
   LGPoint curr;
   TNG_buttonarray *pbatng;
   int i,j;
   int a,b;
   bool retval = FALSE;

   if (type & TNG_MOUSE_LDOWN)
   {
      pbatng = TNG_BA(ptng);
      curr = loc;
      curr.x -= (pbatng->scroll_size.x + TNG_BA_BORDER_WIDTH);
      curr.y -= (pbatng->scroll_size.y + TNG_BA_BORDER_WIDTH);
      i=0; j=0;
      while (curr.x > (TNG_BA_BSIZE(ptng).x + TNG_BA_SPACING(ptng)))
      {
         curr.x -= TNG_BA_BSIZE(ptng).x + TNG_BA_SPACING(ptng);
         i++;
      }
      while (curr.y > (TNG_BA_BSIZE(ptng).y + TNG_BA_SPACING(ptng)))
      {
         curr.y -= TNG_BA_BSIZE(ptng).y + TNG_BA_SPACING(ptng);
         j++;
      }
      //Spew(DSRC_UI_Buttonarray, ("i = %d  j = %d  WSIZE = (%d,%d)\n",i,j,TNG_BA_WSIZE(ptng).x, TNG_BA_WSIZE(ptng).y));
      if ((i < TNG_BA_WSIZE(ptng).x) && (j < TNG_BA_WSIZE(ptng).y))
      {
         a = TNG_BA_LSEL(ptng).x;
         b = TNG_BA_LSEL(ptng).y;
         TNG_BA_LSEL(ptng).x = i;
         TNG_BA_LSEL(ptng).y = j;
         //Spew(DSRC_UI_Buttonarray, ("TESTING DRAW!! = %d\n",TNG_FOREIGN_OBSCURED(ptng)));
         TNG_IF_FOREIGN_UNOBSCURED(ptng)
         {
            tng_buttonarray_draw_button(ptng,a,b);
         }
         else
         {
            TNG_DRAWPART(ptng, TNG_ALLPARTS);
         }
         IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_SELECT));
      }
   }
   IF_SET_RV(tng_cb_mousebutt(ptng,type,loc));
   retval = TRUE;
   return(retval);
}

// Handle incoming signals
bool tng_buttonarray_signal(TNG *ptng, ushort signal)
{
   bool retval = FALSE;
   //Spew(DSRC_UI_Buttonarray, ("Buttonarray Received signal: %x\n",signal));
   if (signal & TNG_SIGNAL_SELECT)
      tng_buttonarray_select(ptng);
   if (signal & TNG_SIGNAL_SCROLL)
      tng_buttonarray_scroll(ptng);
   IF_SET_RV(tng_cb_signal(ptng, signal));
   retval = TRUE;
   return(retval);
}
// -----------------------------

// Assumes that lsel has been set to the selection-elect
errtype tng_buttonarray_select(TNG *ptng)
{
   int a,b;
   int i,j;
   int i2,j2;
   int totnum = 0;

   a = TNG_BA_LSEL(ptng).x + TNG_BA_OFFSET(ptng).x;
   b = TNG_BA_LSEL(ptng).y + TNG_BA_OFFSET(ptng).y;
   //Spew(DSRC_UI_Buttonarray, ("BUTTONARRAY BUTTON  SELECTED at %d, %d!\n",a,b));
   for (i=0; i < TNG_BA_MSIZE(ptng).x; i++)
   {
      for (j=0; j < TNG_BA_MSIZE(ptng).y ;j++)
      {
         if (TNG_BA_SELECTED(ptng, i, j))
            totnum++;
      }
   }
   if (TNG_BA_SELECTED(ptng,a,b) == TRUE)
   {
      TNG_BA_SELECTED(ptng,a,b) = FALSE;
      TNG_IF_FOREIGN_UNOBSCURED(ptng)
      {
         tng_buttonarray_draw_button(ptng,TNG_BA_LSEL(ptng).x,TNG_BA_LSEL(ptng).y);
      }
      else
      {
         TNG_DRAWPART(ptng, TNG_ALLPARTS);
      }
   }
   else if ((TNG_BA_NUMSEL(ptng) < 2) || (totnum < TNG_BA_NUMSEL(ptng)))
   {
      if (TNG_BA_NUMSEL(ptng) == 1)
      {
         for (i=0; i < TNG_BA_MSIZE(ptng).x; i++)
         {
            for (j=0; j < TNG_BA_MSIZE(ptng).y ;j++)
            {
               if (TNG_BA_SELECTED(ptng,i,j))
               {
                  TNG_BA_SELECTED(ptng,i,j) = FALSE;
                  i2 = i - TNG_BA_OFFSET(ptng).x;
                  j2 = j - TNG_BA_OFFSET(ptng).y;
                  if ((i2 >=0) && (i2 <= TNG_BA_WSIZE(ptng).x) &&
                      (j2 >=0) && (j2 <= TNG_BA_WSIZE(ptng).y))
                  {
                     TNG_IF_FOREIGN_UNOBSCURED(ptng)
                     {
                        tng_buttonarray_draw_button(ptng,i2,j2);
                     }
                  }
               }
            }
         }
      }
      TNG_BA_SELECTED(ptng,a,b) = TRUE;
      TNG_IF_FOREIGN_UNOBSCURED(ptng)
      {
         tng_buttonarray_draw_button(ptng,TNG_BA_LSEL(ptng).x,TNG_BA_LSEL(ptng).y);
      }
      else
      {
         TNG_DRAWPART(ptng, TNG_ALLPARTS);
      }
   }
   ptng->signal(ptng, TNG_SIGNAL_CHANGED);
   return(OK);
}

// Assumes that lastkey contains the hex code of the 
// appropriate kind of scroll key.
errtype tng_buttonarray_scroll(TNG *ptng)
{
   short code;
   TNG *which_bar;
   code = TNG_BA_LASTKEY(ptng);
   which_bar = NULL;
   switch (code)
   {
      case TNG_BA_SCROLL_UP_KEY:
         which_bar = TNG_BA(ptng)->vscroll_tng;
         TNG_SL(which_bar)->value = TNG_SL(which_bar)->value + TNG_BA_WSIZE(ptng).y;
         if (TNG_SL(which_bar)->value > TNG_SL(which_bar)->max)
            TNG_SL(which_bar)->value = TNG_SL(which_bar)->max;
         break;
      case TNG_BA_SCROLL_DOWN_KEY:
         which_bar = TNG_BA(ptng)->vscroll_tng;
         TNG_SL(which_bar)->value -= TNG_BA_WSIZE(ptng).y;
         if (TNG_SL(which_bar)->value < TNG_SL(which_bar)->min)
            TNG_SL(which_bar)->value = TNG_SL(which_bar)->min;
         break;
      case TNG_BA_SCROLL_LEFT_KEY:
         which_bar = TNG_BA(ptng)->hscroll_tng;
         TNG_SL(which_bar)->value -= TNG_BA_WSIZE(ptng).x;
         if (TNG_SL(which_bar)->value < TNG_SL(which_bar)->min)
            TNG_SL(which_bar)->value = TNG_SL(which_bar)->min;
         break;
      case TNG_BA_SCROLL_RIGHT_KEY:
         which_bar = TNG_BA(ptng)->hscroll_tng;
         TNG_SL(which_bar)->value += TNG_BA_WSIZE(ptng).x;
         if (TNG_SL(which_bar)->value > TNG_SL(which_bar)->max)
            TNG_SL(which_bar)->value = TNG_SL(which_bar)->max;
         break;
   }
   if (which_bar != NULL)
      which_bar->signal(which_bar, TNG_SIGNAL_CHANGED);

   return(OK);
}

errtype tng_buttonarray_move(TNG *ptng, short code)
{
   TNG *hbar, *vbar;
   int a,b;
   int i,j;
   hbar = TNG_BA(ptng)->hscroll_tng;
   vbar = TNG_BA(ptng)->vscroll_tng;
   a = TNG_BA_LSEL(ptng).x;
   b = TNG_BA_LSEL(ptng).y;
   switch(code)
   {
      case TNG_BA_UP_KEY:
         if (TNG_BA_LSEL(ptng).y > 0)
            TNG_BA_LSEL(ptng).y -= 1;
         else if (vbar != NULL)
            vbar->signal(vbar, TNG_SIGNAL_INCREMENT);
         break;
      case TNG_BA_DOWN_KEY:
         if (TNG_BA_LSEL(ptng).y < (TNG_BA_WSIZE(ptng).y - 1))
            TNG_BA_LSEL(ptng).y += 1;
         else if (vbar != NULL)
            vbar->signal(vbar, TNG_SIGNAL_DECREMENT);
         break;
      case TNG_BA_LEFT_KEY:
         if (TNG_BA_LSEL(ptng).x > 0)
            TNG_BA_LSEL(ptng).x -= 1;
         else if (hbar != NULL)
            hbar->signal(hbar, TNG_SIGNAL_DECREMENT);
         break;
      case TNG_BA_RIGHT_KEY:
         if (TNG_BA_LSEL(ptng).x < (TNG_BA_WSIZE(ptng).x - 1))
            TNG_BA_LSEL(ptng).x += 1;
         else if (hbar != NULL)
            hbar->signal(hbar, TNG_SIGNAL_INCREMENT);
         break;
   }
   ptng->signal(ptng, TNG_SIGNAL_CHANGED);
   TNG_IF_FOREIGN_UNOBSCURED(ptng)
   {
      tng_buttonarray_draw_button(ptng,a,b);
      tng_buttonarray_draw_button(ptng,TNG_BA_LSEL(ptng).x,TNG_BA_LSEL(ptng).y);
      if (TNG_BA(ptng)->spacing < 2)
      {
         //Spew(DSRC_UI_Buttonarray, ("redrawing selected buttons after move...\n"));
         for (i=0; i<TNG_BA_WSIZE(ptng).x; i++)
         {
            for (j=0; j<TNG_BA_WSIZE(ptng).y; j++)
            {
               //Spew(DSRC_UI_Buttonarray, ("(off: %d,%d)(%d,%d) -- %d\n",TNG_BA_OFFSET(ptng).x, TNG_BA_OFFSET(ptng).y,
               //   i,j,TNG_BA_SELECTED(ptng, i + TNG_BA_OFFSET(ptng).x, j + TNG_BA_OFFSET(ptng).y)));
               if (TNG_BA_SELECTED(ptng, i + TNG_BA_OFFSET(ptng).x, j + TNG_BA_OFFSET(ptng).y))
                  tng_buttonarray_draw_button(ptng,i,j);
            }
         }
      }
   }
   else
   {
      TNG_DRAWPART(ptng, TNG_ALLPARTS);
   }
   
   return(OK);
}

// -----------------------------
errtype tng_buttonarray_addbutton_at(TNG *ptng, int type, void *disp_data, int coord_x, int coord_y)
{
   if ((coord_x < 0) || (coord_y < 0))
   {
      //Spew(DSRC_UI_Bounds, ("Attempted to add button at negative coordinates!\n"));
      return(ERR_RANGE);
   }
   //Spew(DSRC_UI_Buttonarray, ("Button added at %d, %d\n",coord_x, coord_y));
   TNG_BA_INDEX(ptng,coord_x,coord_y).type = type;
   TNG_BA_INDEX(ptng,coord_x,coord_y).disp_data = disp_data;
   return(OK);
}

errtype tng_buttonarray_addbutton(TNG *ptng, int type, void *disp_data)
{
   int i,j;
   for (j=0; j<TNG_BA_MSIZE(ptng).y; j++)
   {
      for (i=0; i<TNG_BA_MSIZE(ptng).x; i++)
      {
         if (TNG_BA_INDEX(ptng,i,j).type == NULL_TYPE)
         {
            tng_buttonarray_addbutton_at(ptng,type,disp_data,i,j);
            return(OK);
         }
      }
   }

   return(ERR_RANGE);
}

errtype tng_buttonarray_setoffset(TNG *ptng, int offset_x, int offset_y)
{
   if ((offset_x < 0) || (offset_y < 0))
   {
      //Spew(DSRC_UI_Bounds, ("Attempted to add button at negative coordinates!\n"));
      return(ERR_RANGE);
   }

   TNG_BA_OFFSET(ptng).x = offset_x;
   TNG_BA_OFFSET(ptng).y = offset_y;

   // Expose yourself now that you've shifted around, presumably

   TNG_DRAWPART(ptng, TNG_ALLPARTS);
   ptng->signal(ptng, TNG_SIGNAL_CHANGED);
   return(OK);
}

