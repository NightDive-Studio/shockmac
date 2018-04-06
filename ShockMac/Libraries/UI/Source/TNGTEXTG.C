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
#include "tngtextg.h"
//#include <_ui.h>
#include "tngslidr.h"
#include "fakefont.h"
#include "tngfont.h"

errtype tng_textgadget_move(TNG *ptng, short code);

// Callbacks....

#pragma require_prototypes off

// For the text tool to communicate with the rest of the universe....
void tng_textgadget_ui_display(void *ptng, LGRect *)
{
   //Spew(DSRC_UI_Textgadget, ("About to trigger display from texttool call!\n"));
   //Spew(DSRC_UI_Textgadget, ("r = (%d,%d)(%d, %d)\n",RECT_EXPAND_ARGS(r)));
   if (ptng != NULL)
      TNG_DRAWRECT((TNG *)ptng, NULL);
   //else
      //Spew(DSRC_UI_Textgadget, ("Problem was a null ptng in ui_display...\n"));
}

bool tng_textgadget_hscroll_changed(void *ui_data, void *user_data)
{
   TNG *ptng;
   void *dummy;
   dummy = ui_data;

   ptng = (TNG *)user_data;
   ptng->signal(ptng, TNG_SIGNAL_CHANGED);
   return(FALSE);
}

bool tng_textgadget_vscroll_changed(void *ui_data, void *user_data)
{
   TNG *ptng;
   void *dummy;
   dummy = ui_data;

   ptng = (TNG *)user_data;
   ptng->signal(ptng, TNG_SIGNAL_CHANGED);
   return(FALSE);
}
#pragma require_prototypes on

errtype tng_textgadget_destroy(TNG *ptng)
{
   GUI_DEALLOC(ptng->ui_data, ptng->type_data);
   return(OK);
}

// Initializes the TNG 
errtype tng_textgadget_init(void *ui_data, TNG *ptng, TNGStyle *sty, ulong options, LGPoint size, LGPoint abs_loc)
{
   TNG_textgadget *ptxtng;
   extern TTFontInfo TTTNGFontInfo;
   TTState TTs;
   TTRect TTr;
   static bool inited=FALSE;

   ptxtng = (TNG_textgadget *)GUI_MALLOC(ptng->ui_data, sizeof(TNG_textgadget));

   TNGInit(ptng,sty,ui_data);
   ptng->flags = TNG_BEVEL;
   ptng->type_data = ptxtng;
   ptng->draw_func = &tng_textgadget_2d_draw;
//   ptng->mousebutt = &tng_textgadget_mousebutt;
   ptng->keycooked = &tng_textgadget_keycooked;
   ptng->signal = &tng_textgadget_signal;

   ptxtng->tng_data = ptng;
   ptxtng->size = size;
   ptxtng->last_key = 0;
   ptxtng->hscroll_tng = NULL;
   ptxtng->vscroll_tng = NULL;
   ptxtng->options = options;

   if (inited)
      fnt_select(0);                   /* select font 0 */
   else
      fnt_init_from_style(ptng->style);          /* get the default font */
   TTs.left_m = 0;  TTs.right_m = 0;
   TTs.max_w = 0;
   TTs.mode = 0;
   if (options & TNG_TG_SINGLE_LINE)
      TTs.mode |= TTS_SINGLE;
   if (options & TNG_TG_READ_ONLY)
      TTs.mode |= TTS_READONLY;
   if (options & TNG_TG_LINE_SET)
      TTs.mode |= TTS_LINES;
   if (TTs.mode == 0)
      TTs.mode = TTS_FULL | TTS_WRAP;
   //Spew(DSRC_UI_Textgadget, ("options = %x  TTs.mode = %x\n",options,TTs.mode));
   TTr.crn.pt.x = abs_loc.x;
   TTr.crn.pt.y = abs_loc.y;
   TTr.w = size.x - (2 * TNG_TG_BORDER_WIDTH);
   if (options & TNG_TG_VERT_SCROLL)
   {
      TTr.crn.pt.x += TNG_TG_SCROLL_X;
      TTr.w -= TNG_TG_SCROLL_X;
   }
   TTr.crn.pt.x += TNG_TG_BORDER_WIDTH;
   TTr.h = size.y - (2 * TNG_TG_BORDER_WIDTH);
   TTr.crn.pt.y += TNG_TG_BORDER_WIDTH;
   if (options & TNG_TG_HORZ_SCROLL)
   {
      TTr.crn.pt.y += TNG_TG_SCROLL_Y;
      TTr.h -= TNG_TG_SCROLL_Y;
   }
   TTs.r_cnt = 1;
   TTs.last_ev = TTEV_NULL;
   ptxtng->tt=tt_full_build(&TTr,&TTs,&TTTNGFontInfo,ptng,NULL, &tng_textgadget_ui_display);

   return(OK);
}

errtype tng_textgadget_init2(TNG *ptng)
{
   TNG_textgadget *ptxtng;
   int id;
   LGPoint sloc, ssize;

   ptxtng = TNG_TG(ptng);

   if (ptxtng->options & TNG_TG_HORZ_SCROLL)
   {
      sloc.x = TNG_TG_SCROLL_X + TNG_TG_BORDER_WIDTH;
      sloc.y = TNG_TG_BORDER_WIDTH;
      ssize.x = ptxtng->size.x - sloc.x - TNG_TG_BORDER_WIDTH;
      ssize.y = TNG_TG_SCROLL_Y;
      TNG_CREATE_SLIDER(ptng->ui_data, sloc, &(ptxtng->hscroll_tng), ptng->style, TNG_SL_HORIZONTAL, 0, 100, 0, 5, ssize);
      tng_install_callback(ptxtng->hscroll_tng, TNG_EVENT_SIGNAL, TNG_SIGNAL_CHANGED, &tng_textgadget_hscroll_changed, ptng, &id);
   }
   else
   {
      ptxtng->hscroll_tng = NULL;
   }
   if (ptxtng->options & TNG_TG_VERT_SCROLL)
   {
      sloc.x = TNG_TG_BORDER_WIDTH;
      sloc.y = TNG_TG_SCROLL_Y + TNG_TG_BORDER_WIDTH;
      ssize.x = TNG_TG_SCROLL_X;
      ssize.y = ptxtng->size.y - sloc.y - TNG_TG_BORDER_WIDTH;
      TNG_CREATE_SLIDER(ptng->ui_data, sloc, &(ptxtng->vscroll_tng), ptng->style, TNG_SL_VERTICAL, 0, 100, 0, 5, ssize);
      tng_install_callback(ptxtng->vscroll_tng, TNG_EVENT_SIGNAL, TNG_SIGNAL_CHANGED, &tng_textgadget_vscroll_changed, ptng, &id);
   }
   else
   {
      ptxtng->vscroll_tng = NULL;
   }

   return(OK);
}

// Draw the specified parts (may be all) of the TNG at screen coordinates loc
// assumes all appropriate setup has already been done!
errtype tng_textgadget_2d_draw(TNG *ptng, ushort , LGPoint loc)
{
   TNG_textgadget *ptxtng;
   LGRect r;
   TextTool *t;

   //Spew(DSRC_UI_Textgadget, ("TNG Textgadget 2d Draw at (%d, %d) -- partmask = %x\n",loc.x,loc.y,partmask));
   TNG_IF_OBSCURED(ptng)
   {
      return(OK);
   }
   ptng->signal(ptng, TNG_SIGNAL_EXPOSE);
   ptxtng = TNG_TG(ptng);
   TNGDrawBase(ptng, loc, ptxtng->size);
   r.ul.x = loc.x + TNG_TG_BORDER_WIDTH;
   r.ul.y = loc.y + TNG_TG_BORDER_WIDTH; 
   t = TNG_TG_TT(ptng);
   if (t != NULL)
      tt_show_all(t);

   return(OK);
}

// Fill in ppt with the size...
errtype tng_textgadget_size(TNG *ptng, LGPoint *ppt)
{
   *ppt = TNG_TG(ptng)->size;
   return(OK);
}

// Returns the current "value" of the TNG
int tng_textgadget_getvalue(TNG *ptng)
{
   return(TNG_TG(ptng)->last_key);
}

// React appropriately for receiving the specified cooked key
bool tng_textgadget_keycooked(TNG *ptng, ushort key)
{
   short code;
   bool retval = FALSE;
/*еее

   code = key & 0xff;
   //Spew(DSRC_UI_Textgadget, ("%x was typed!\n",code));
   tt_parse_char(TNG_TG_TT(ptng),key);
   switch(code)
   {
      case TNG_TG_RETURN_KEY:
         IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_SELECT));
         break;
      case TNG_TG_SCROLL_UP_KEY:
      case TNG_TG_SCROLL_DOWN_KEY:
      case TNG_TG_SCROLL_LEFT_KEY:
      case TNG_TG_SCROLL_RIGHT_KEY:
         TNG_TG_LASTKEY(ptng) = code;
         IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_SCROLL));
         break;
   }
//   Spew(DSRC_UI_Textgadget, ("About to tt_parse_char...\n"));
   IF_SET_RV(tng_cb_keycooked(ptng, key));
   retval = TRUE;
*/
   return(retval);   
}

// React appropriately for receiving the specified mouse button event
bool tng_textgadget_mousebutt(TNG *ptng, uchar type, LGPoint loc)
{
   return(tng_cb_mousebutt(ptng,type,loc));
}

// Handle incoming signals
bool tng_textgadget_signal(TNG *ptng, ushort signal)
{
   bool retval = FALSE;
   //Spew(DSRC_UI_Textgadget, ("Textgadget Received signal: %x\n",signal));
   if (signal & TNG_SIGNAL_CHANGED)
      TNG_DRAWPART(ptng, TNG_ALLPARTS);
   if (signal & TNG_SIGNAL_SCROLL)
      tng_textgadget_scroll(ptng);
   IF_SET_RV(tng_cb_signal(ptng, signal));
   retval = TRUE;
   return(retval);
}
// -----------------------------


// Assumes that lastkey contains the hex code of the 
// appropriate kind of scroll key.
errtype tng_textgadget_scroll(TNG *ptng)
{
/*еее
   short code;
   TNG *which_bar;
   bool increm;
   code = TNG_TG_LASTKEY(ptng);
   which_bar = NULL;
   switch (code)
   {
      case TNG_TG_SCROLL_UP_KEY:
         which_bar = TNG_TG(ptng)->vscroll_tng;
         increm = TRUE;
         break;
      case TNG_TG_SCROLL_DOWN_KEY:
         which_bar = TNG_TG(ptng)->vscroll_tng;
         increm = FALSE;
         break;
      case TNG_TG_SCROLL_LEFT_KEY:
         which_bar = TNG_TG(ptng)->hscroll_tng;
         increm = FALSE;
         break;
      case TNG_TG_SCROLL_RIGHT_KEY:
         which_bar = TNG_TG(ptng)->hscroll_tng;
         increm = TRUE;
         break;
   }
   if (which_bar != NULL)
   {
      if (increm)
         which_bar->signal(which_bar, TNG_SIGNAL_INCREMENT);
      else
         which_bar->signal(which_bar, TNG_SIGNAL_DECREMENT);
      ptng->signal(ptng, TNG_SIGNAL_CHANGED);
   }
*/
   return(OK);
}

errtype tng_textgadget_addstring(TNG *ptng, char *s)
{
   region_begin_sequence();
   tt_parse_string(TNG_TG_TT(ptng), s);
   region_end_sequence(TRUE);
   return(OK);
}
