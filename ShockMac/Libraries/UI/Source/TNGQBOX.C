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
//#include <fcntl.h>
//#include <io.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "tngapp.h"
#include "tngqbox.h"
//#include <_ui.h>
#include "tngslidr.h"
#include "tngtextg.h"

// Externs
extern int _tt_do_event(long tt_event);

// #include <mprintf.h>

TNG *current_tng;
int num_slots;

// Protoytpes
errtype tng_increm_slot(TNG *ptng, int quan);
errtype tng_decrem_slot(TNG *ptng, int quan);
bool tng_quickbox_scroll_changed(void *ui_data, void *user_data);
bool quickbox_fix_text_changed(QuickboxSlot *qbs);
bool quickbox_uint_text_changed(QuickboxSlot *qbs);
bool tng_quickbox_text_changed(void *ui_data, void *user_data);
errtype tng_draw_qb_int_slot(TNG *ptng, QuickboxSlot *curp, LGRect r);
errtype tng_draw_qb_text_slot(TNG *ptng, QuickboxSlot *curp, LGRect r);
errtype tng_draw_qb_bool_slot(TNG *ptng, QuickboxSlot *curp, LGRect r);
errtype tng_draw_qb_pb_slot(TNG *ptng, QuickboxSlot *curp, LGRect r);
errtype gad_qbox_display_slot(QuickboxSlot *qbs, bool recurse);
errtype _draw_text(TNG *ptng, char *txt, int x_coord, int y_coord);
int _text_width(TNG *ptng, char *t);
int _label_extent(TNG *ptng);
int _total_extent(TNG *ptng);
errtype _slot_rectangle(TNG *qb, int slot_num, LGRect *slot_rect);

errtype tng_increm_slot(TNG *ptng, int quan)
{
   QuickboxSlot *pqs;
   int *val_i;
   uint *val_ui;
   short *val_s;
   bool *val_b;
   ubyte *val_by;

   pqs = QB_CURRENT(ptng);
   if (pqs->vartype == QB_INT_SLOT)
   {
      val_i = (int *)(pqs->var);
      *val_i = *val_i + quan;
      if ((pqs->options & QB_BOUNDED) && (*val_i > (int)pqs->p2))
         *val_i = (int)pqs->p2;
      if ((pqs->options & QB_CYCLE) && (*val_i > (int)pqs->p2))
         *val_i = (int)pqs->p1;
      if ((pqs->options & QB_STRINGSET) && (*val_i > (int)pqs->p2))
         *val_i = (int)0;
   }
   if (pqs->vartype == QB_UINT_SLOT)
   {
      val_ui = (uint *)(pqs->var);
      *val_ui = *val_ui + quan;
      if ((pqs->options & QB_BOUNDED) && (*val_ui > (uint)pqs->p2))
         *val_ui = (uint)pqs->p2;
      if ((pqs->options & QB_CYCLE) && (*val_ui > (uint)pqs->p2))
         *val_ui = (uint)pqs->p1;
      if ((pqs->options & QB_STRINGSET) && (*val_ui > (uint)pqs->p2))
         *val_ui = (uint)0;
   }
   if (pqs->vartype == QB_SHORT_SLOT)
   {
      val_s = (short *)(pqs->var);
      *val_s = *val_s + (short)quan;
      if ((pqs->options & QB_BOUNDED) && (*val_s > (int)pqs->p2))
         *val_s = (short)pqs->p2;
      if ((pqs->options & QB_CYCLE) && (*val_s > (int)pqs->p2))
         *val_s = (short)pqs->p1;
      if ((pqs->options & QB_STRINGSET) && (*val_s > (int)pqs->p2))
         *val_s = (short)0;
   }
   if (pqs->vartype == QB_BYTE_SLOT)
   {
      val_by = (ubyte *)(pqs->var);
      *val_by = *val_by + (ubyte)quan;
      if ((pqs->options & QB_BOUNDED) && (*val_by > (int)pqs->p2))
         *val_by = (ubyte)pqs->p2;
      if ((pqs->options & QB_CYCLE) && (*val_by > (int)pqs->p2))
         *val_by = (ubyte)pqs->p1;
      if ((pqs->options & QB_STRINGSET) && (*val_by > (int)pqs->p2))
         *val_by = (ubyte)0;
   }
   if (pqs->vartype == QB_BOOL_SLOT)
   {
      val_b = (bool *)(pqs->var);
      if (*val_b)
         *val_b = FALSE;
      else
         *val_b = TRUE;
   }
   pqs->tng->signal(pqs->tng,TNG_SIGNAL_CHANGED);
   gad_qbox_display_slot(pqs, TRUE);
   return(OK);
}

errtype tng_decrem_slot(TNG *ptng, int quan)
{
   QuickboxSlot *pqs;
   int *val_i;
   uint *val_ui;
   short *val_s;
   bool *val_b;
   ubyte *val_by;

   pqs = QB_CURRENT(ptng);
   if (pqs->vartype == QB_INT_SLOT)
   {
      val_i = (int *)(pqs->var);
      *val_i = *val_i - quan;
      if ((pqs->options & QB_BOUNDED) && (*val_i < (int)pqs->p1))
         *val_i = (int)pqs->p1;
      if ((pqs->options & QB_CYCLE) && (*val_i < (int)pqs->p1))
         *val_i = (int)pqs->p2;
      if ((pqs->options & QB_STRINGSET) && (*val_i < 0))
         *val_i = (int)pqs->p2;
   }
   if (pqs->vartype == QB_UINT_SLOT)
   {
      uint *orig = (uint *)(pqs->var);
      val_ui = (uint *)(pqs->var);
      *val_ui = *val_ui - quan;
      if ((pqs->options & QB_BOUNDED) && (*val_ui < (uint)pqs->p1))
         *val_ui = (uint)pqs->p1;
      if ((pqs->options & QB_CYCLE) && (*val_ui < (uint)pqs->p1))
         *val_ui = (uint)pqs->p2;
      if ((pqs->options & QB_STRINGSET) && (*val_ui > *orig))
         *val_ui = (uint)pqs->p2;
   }
   if (pqs->vartype == QB_SHORT_SLOT)
   {
      val_s = (short *)(pqs->var);
      *val_s = (*val_s) - (short)quan;
      if ((pqs->options & QB_BOUNDED) && (*val_s < (int)pqs->p1))
         *val_s = (short)pqs->p1;
      if ((pqs->options & QB_CYCLE) && (*val_s < (int)pqs->p1))
         *val_s = (short)pqs->p2;
      if ((pqs->options & QB_STRINGSET) && (*val_s < 0))
         *val_s = (short)pqs->p2;
   }
   if (pqs->vartype == QB_BYTE_SLOT)
   {
      val_by = (ubyte *)(pqs->var);
      if ((pqs->options & QB_BOUNDED) && (*val_by < (int)pqs->p1 + (ubyte)quan))
         *val_by = (ubyte)pqs->p1;
      else if ((pqs->options & QB_CYCLE) && (*val_by < (int)pqs->p1 + (ubyte)quan))
         *val_by = (ubyte)pqs->p2;
      else if ((pqs->options & QB_STRINGSET) && (*val_by < (ubyte)quan))
         *val_by = (ubyte)pqs->p2;
      else
         *val_by = *val_by - (ubyte)quan;
   }
   if (pqs->vartype == QB_BOOL_SLOT)
   {
      val_b = (bool *)(pqs->var);
      if (*val_b)
         *val_b = FALSE;
      else
         *val_b = TRUE;
   }
   pqs->tng->signal(pqs->tng,TNG_SIGNAL_CHANGED);
   gad_qbox_display_slot(pqs, TRUE);
   return(OK);
}

bool tng_quickbox_scroll_changed(void *ui_data, void *user_data)
{
   QuickboxSlot *qbs;
   TNG *ptng;
#ifndef NO_DUMMIES
   void *dummy; dummy = ui_data;
#endif

   qbs = (QuickboxSlot *)user_data;
   ptng = qbs->tng;
//   Spew(DSRC_UI_Quickbox, ("Hey, scrollbar changed...\n"));
   if (qbs->vartype == QB_INT_SLOT)
      *((int *)(qbs->var)) = TNG_SL(qbs->aux_tng)->value;
   if (qbs->vartype == QB_UINT_SLOT)
      *((uint *)(qbs->var)) = TNG_SL(qbs->aux_tng)->value;
   else if (qbs->vartype == QB_SHORT_SLOT)
      *((short *)(qbs->var)) = (short)TNG_SL(qbs->aux_tng)->value;
   else if (qbs->vartype == QB_BYTE_SLOT)
      *((ubyte *)(qbs->var)) = (ubyte)TNG_SL(qbs->aux_tng)->value;
   ptng->signal(ptng,TNG_SIGNAL_CHANGED);
   gad_qbox_display_slot(qbs, TRUE);
   return(FALSE);
}

bool quickbox_fix_text_changed(QuickboxSlot *qbs)
{
   char* stringval = TNG_TX_GETLINE(qbs->aux_tng,0);
   *((fix *)(qbs->var)) = fix_from_float(atof(stringval));
   return TRUE;
}

bool quickbox_uint_text_changed(QuickboxSlot *qbs)
{
   char *convstring;
   uint atoival, newval;
   short base,i,cap;
   bool okay;

   base = 10;
   //Spew(DSRC_UI_Quickbox, ("Hey, at top of parsing algorithm!\n"));
   if (qbs->options & QB_HEX) {  base = 16; cap = 10; }
   if (qbs->options & QB_OCTAL) {  base = 8; cap = 8; }
   if (qbs->options & QB_BINARY) {   base = 2; cap = 2; }
   convstring = TNG_TX_GETLINE(qbs->aux_tng, 0);
   atoival = 0;
   okay = TRUE;
   for (i=0; i<strlen(convstring); i++)
   {
      convstring[i] = toupper(convstring[i]);
      switch(base)
      {
         case 16:
            if ((convstring[i] < '0') || ((convstring[i] > '9') && (convstring[i] < 'A')) || (convstring[i] > 'F'))
               okay = FALSE;
            break;
         case 8:
            if ((convstring[i] < '0') || (convstring[i] > '7'))
               okay = FALSE;
            break;
         case 2:
            if ((convstring[i] < '0') || (convstring [i] > '1'))
               okay = FALSE;
            break;
      }
   }
   if (okay)
   {
      //Spew(DSRC_UI_Quickbox, ("convstring = %s base = %d\n", convstring,base));
      for (i=0; i<strlen(convstring); i++)
      {
         //Spew(DSRC_UI_Quickbox, ("i = %d  convstring[i] = %c(%d)\n",i,convstring[i],convstring[i]));
         if ((convstring[i] - '0') < cap)
         {
            newval = pow(base,strlen(convstring)-1-i) * (convstring[i] - '0');
            atoival += newval;
            //Spew(DSRC_UI_Quickbox, ("%d^%d * (%d) = %d * %d = %d\n",
            //   base,strlen(convstring)-1-i,convstring[i]-'0',pow(base,strlen(convstring) - 1 - i),convstring[i]-'0',newval));
         }
         else if ((base == 16) && (convstring[i] >= 'A'))
         {
            newval = pow(16,strlen(convstring)-1-i) * (convstring[i] - 'A' + 10);
            atoival += newval;
            //Spew(DSRC_UI_Quickbox, ("hex: 16^%d * (%d) = %d * %d = %d\n",
            //   strlen(convstring)-1-i,convstring[i]-'A'+10,pow(16,strlen(convstring)-1-i),convstring[i]-'A'+10,newval));
         }
      }
   }
   else
      atoival = 0;
   *((uint *)(qbs->var)) = atoival; 

   return TRUE;
}

bool tng_quickbox_text_changed(void *ui_data, void *user_data)
{
   QuickboxSlot *qbs;
   TNG *ptng;
   int atoival, base, i, cap,newval;
   char *convstring;
   bool okay;

#ifndef NO_DUMMIES
   void *dummy; dummy = ui_data;
#endif

   qbs = (QuickboxSlot *)user_data;
   ptng = qbs->tng;
   //Spew(DSRC_UI_Quickbox, ("Hey, text changed to: %s\n",TNG_TX_GETLINE(qbs->aux_tng,0)));
   if (qbs->vartype == QB_TEXT_SLOT)
   {
      if (!(qbs->options & QB_RD_ONLY))
         strcpy(((char *)(qbs->var)), TNG_TX_GETLINE(qbs->aux_tng, 0));
   }
   if (qbs->vartype == QB_FIX_SLOT)
      quickbox_fix_text_changed(qbs);
   else if (qbs->vartype == QB_UINT_SLOT)
      quickbox_uint_text_changed(qbs);
   else if ((qbs->vartype == QB_INT_SLOT) || (qbs->vartype == QB_BYTE_SLOT) || (qbs->vartype == QB_SHORT_SLOT))
   {
      base = 10;
      //Spew(DSRC_UI_Quickbox, ("Hey, at top of parsing algorithm!\n"));
      if (qbs->options & QB_HEX) {  base = 16; cap = 10; }
      if (qbs->options & QB_OCTAL) {  base = 8; cap = 8; }
      if (qbs->options & QB_BINARY) {   base = 2; cap = 2; }
      convstring = TNG_TX_GETLINE(qbs->aux_tng, 0);
      if (base == 10)
      {
         atoival = atoi(convstring);
      }
      else
      {
         atoival = 0;
         okay = TRUE;
         for (i=0; i<strlen(convstring); i++)
         {
            convstring[i] = toupper(convstring[i]);
            switch(base)
            {
               case 16:
                  if ((convstring[i] < '0') || ((convstring[i] > '9') && (convstring[i] < 'A')) || (convstring[i] > 'F'))
                     okay = FALSE;
                  break;
               case 8:
                  if ((convstring[i] < '0') || (convstring[i] > '7'))
                     okay = FALSE;
                  break;
               case 2:
                  if ((convstring[i] < '0') || (convstring [i] > '1'))
                     okay = FALSE;
                  break;
            }
         }
         if (okay)
         {
            //Spew(DSRC_UI_Quickbox, ("convstring = %s base = %d\n", convstring,base));
            for (i=0; i<strlen(convstring); i++)
            {
               //Spew(DSRC_UI_Quickbox, ("i = %d  convstring[i] = %c(%d)\n",i,convstring[i],convstring[i]));
               if ((convstring[i] - '0') < cap)
               {
                  newval = pow(base,strlen(convstring)-1-i) * (convstring[i] - '0');
                  atoival += newval;
                  //Spew(DSRC_UI_Quickbox, ("%d^%d * (%d) = %d * %d = %d\n",
                  //   base,strlen(convstring)-1-i,convstring[i]-'0',pow(base,strlen(convstring) - 1 - i),convstring[i]-'0',newval));
               }
               else if ((base == 16) && (convstring[i] >= 'A'))
               {
                  newval = pow(16,strlen(convstring)-1-i) * (convstring[i] - 'A' + 10);
                  atoival += newval;
                  //Spew(DSRC_UI_Quickbox, ("hex: 16^%d * (%d) = %d * %d = %d\n",
                  //   strlen(convstring)-1-i,convstring[i]-'A'+10,pow(16,strlen(convstring)-1-i),convstring[i]-'A'+10,newval));
               }
            }
         }
         else
            atoival = 0;
      }
      if ((qbs->options & QB_BOUNDED) || (qbs->options & QB_CYCLE))
      {
         if (atoival < (int)qbs->p1)
            atoival = (int)qbs->p1;
         if (atoival > (int)qbs->p2)
            atoival = (int)qbs->p2;
      }
      //Spew(DSRC_UI_Quickbox, ("**** atoival = %d(%u) vs %d(%u)\n",atoival,atoival,(ubyte)atoival, (ubyte)atoival));
      switch (qbs->vartype)
      {
         case QB_INT_SLOT:  *((int *)(qbs->var)) = (int)atoival; break;
         case QB_UINT_SLOT:  *((uint *)(qbs->var)) = (uint)atoival; break;
         case QB_SHORT_SLOT:  *((short *)(qbs->var)) = (short)atoival; break;
         case QB_BYTE_SLOT:  *((ubyte *)(qbs->var)) = (ubyte)atoival; break;
      }
      //Spew(DSRC_UI_Quickbox, ("2:atoival = %d(%u)\n",*((ubyte *)(qbs->var)), *((ubyte *)(qbs->var))));
   }
   ptng->signal(ptng,TNG_SIGNAL_CHANGED);
   gad_qbox_display_slot(qbs, TRUE);
   return(FALSE);
}

// Initializes the TNG 
errtype tng_quickbox_init(void *ui_data, TNG *ptng, TNGStyle *sty, ushort options, LGPoint slot_size, LGPoint spacing, LGPoint border,
   Ref left_id, Ref right_id)
{
   TNG_quickbox *pqbtng;

   pqbtng = (TNG_quickbox *)GUI_MALLOC(ptng->ui_data, sizeof(TNG_quickbox));
   //Spew(DSRC_UI_Quickbox, ("Starting quickbox init...\n"));

   TNGInit(ptng,sty,ui_data);
   ptng->flags = TNG_BEVEL;
   ptng->type_data = pqbtng;
   ptng->draw_func = &tng_quickbox_2d_draw;
   ptng->mousebutt = &tng_quickbox_mousebutt;
   ptng->keycooked = &tng_quickbox_keycooked;
   ptng->signal = &tng_quickbox_signal;

   pqbtng->tng_data = ptng;
   pqbtng->size = tngZeroPt;
   pqbtng->slot_size = slot_size;
   pqbtng->spacing = spacing;
   pqbtng->border = border;
   pqbtng->options = options;
   pqbtng->internal_margin = -1;
   pqbtng->aux_size = TNG_QB_DEFAULT_SLIDER_SIZE;
   pqbtng->slots = NULL;
   pqbtng->current_slot = NULL;
   pqbtng->size.x = slot_size.x + (2 * border.x);
   pqbtng->size.y = 2 * border.y;
   pqbtng->left_id = left_id;
   pqbtng->right_id = right_id;
   current_tng = ptng;
   return(OK);
}

// Deallocate all memory used by the TNG 
errtype tng_quickbox_destroy(TNG *ptng)
{
   GUI_DEALLOC(ptng->ui_data, ptng->type_data);
   return(OK);
}

// Draw the specified parts (may be all) of the TNG at screen coordinates loc
errtype tng_draw_qb_int_slot(TNG *ptng, QuickboxSlot *curp, LGRect r)
{
   int *argp;
   short *argsp;
   ubyte *argbyp;
   uint *argup;
   LGRect argrect;     // Where the actual variable should go
   short base;
   LGPoint p1,p2,p3,p4;
   char temp[100];

   // Set some variables, compute some stuff
   if (curp->vartype == QB_INT_SLOT)
      argp = (int *)curp->var;
   if (curp->vartype == QB_UINT_SLOT)
      argup = (uint *)curp->var;
   if (curp->vartype == QB_SHORT_SLOT)
      argsp = (short *)curp->var;
   if (curp->vartype == QB_BYTE_SLOT)
      argbyp = (ubyte *)curp->var;
   r.ul.x += TNG_QB(ptng)->border.x;
   r.ul.y += TNG_QB(ptng)->spacing.y;
   argrect = r;
   if (TNG_QB(ptng)->options & QB_ALIGNMENT)
      argrect.ul.x = r.ul.x + TNG_QB(ptng)->internal_margin + (2 * TNG_QB(ptng)->spacing.x);
   else
      argrect.ul.x = r.ul.x + _text_width(ptng, curp->label) + (2 * TNG_QB(ptng)->spacing.x);
   if (curp->options & QB_ARROWS)
   {
      // Goofy arrows
      if (TNG_QB(ptng)->left_id != NULL)
         argrect.ul.x += resource_bm_width(TNG_QB(ptng)->left_id);
      else
         argrect.ul.x += ptng->style->frobsize.x;
      if (TNG_QB(ptng)->right_id != NULL)
         argrect.lr.x -= resource_bm_width(TNG_QB(ptng)->right_id);
      else
         argrect.lr.x -= ptng->style->frobsize.x;
   }

   // Draw that label
   gr_set_fcolor(ptng->style->textColor);
   _draw_text(ptng, curp->label, r.ul.x, r.ul.y+1);

   // Draw in the actual value of the variable
   if (curp->vartype == QB_INT_SLOT)
   {
      if (curp->options & QB_HEX)
         sprintf(temp, "hex %x", *argp);
      else if (curp->options & QB_BINARY)
//       itoa(*argp, temp, 2);
         sprintf(temp, "%d", *argp);
      else if (curp->options & QB_OCTAL)
         sprintf(temp, "oct %o", *argp);
      else if (curp->options & QB_STRINGSET)
         strcpy(temp, *(((char **)curp->p1) + *argp));
      else
         sprintf(temp, "%d", *argp);
   }
   else if (curp->vartype == QB_UINT_SLOT)
   {
      if (curp->options & QB_HEX)
         sprintf(temp, "hex %x", *argup);
      else if (curp->options & QB_BINARY)
//       itoa(*argup, temp, 2);
         sprintf(temp, "%d", *argup);
      else if (curp->options & QB_OCTAL)
         sprintf(temp, "oct %o", *argup);
      else if (curp->options & QB_STRINGSET)
         strcpy(temp, *(((char **)curp->p1) + *argup));
      else
         sprintf(temp, "%d", *argup);
   }
   else if (curp->vartype == QB_SHORT_SLOT)
   {
      if (curp->options & QB_HEX)            // the h modifier in sprintf - is for short 
         sprintf(temp, "hex %hx", *argsp);
      else if (curp->options & QB_BINARY)
//       itoa((int)*argsp, temp, 2);
         sprintf(temp, "%d", *argsp);
      else if (curp->options & QB_OCTAL)
         sprintf(temp, "oct %ho", *argsp);
      else if (curp->options & QB_STRINGSET)
         strcpy(temp, *(((char **)curp->p1) + *argsp));
      else
         sprintf(temp, "%hd", *argsp);
   }
   else if (curp->vartype == QB_BYTE_SLOT)
   {
      if (curp->options & QB_HEX)
         sprintf(temp, "hex %x", (uint)*argbyp);
      else if (curp->options & QB_BINARY)
//       itoa((int)*argbyp, temp, 2);
         sprintf(temp, "%d", (uint)*argbyp);
      else if (curp->options & QB_OCTAL)
         sprintf(temp, "oct %o", (uint)*argbyp);
      else if (curp->options & QB_STRINGSET)
         strcpy(temp, *(((char **)curp->p1) + *argbyp));
      else
         sprintf(temp, "%u", (uint)*argbyp);
   }
   else if (curp->vartype == QB_FIX_SLOT)
   {
      sprintf(temp,"%f",fix_float(*(fix*)curp->var));
   }
   _draw_text(ptng, temp, argrect.ul.x, argrect.ul.y+1);

   // Draw in goofy arrows, if they are necessary
   if (curp->options & QB_ARROWS)
   {
      if (TNG_QB(ptng)->left_id != NULL)
      {
         if (TNG_QB(ptng)->options & QB_ALIGNMENT)
            draw_resource_bm(TNG_QB(ptng)->left_id, r.ul.x + TNG_QB(ptng)->internal_margin + TNG_QB(ptng)->spacing.x, r.ul.y);
         else
            draw_resource_bm(TNG_QB(ptng)->left_id, r.ul.x + _text_width(ptng, curp->label) + TNG_QB(ptng)->spacing.x, r.ul.y);
      }
      else
      {
         // Draw goofy left arrow
         if (TNG_QB(ptng)->options & QB_ALIGNMENT)
            base = r.ul.x + TNG_QB(ptng)->internal_margin + TNG_QB(ptng)->spacing.x;
         else
            base = r.ul.x + _text_width(ptng, curp->label) + TNG_QB(ptng)->spacing.x;
         p1.x = base + (ptng->style->frobsize.x / 2);
         p1.y = r.ul.y;  
         p2.x = base + ptng->style->frobsize.x;
         p2.y = r.ul.y + (ptng->style->frobsize.y / 2);
         p3.x = p1.x;
         p3.y = r.ul.y + ptng->style->frobsize.y;
         p4.x = base;
         p4.y = p2.y;

         if (TNG_QB(ptng)->options & QB_ALIGNMENT)
            p4.x = r.ul.x + TNG_QB(ptng)->internal_margin + TNG_QB(ptng)->spacing.x;
         else
            p4.x = r.ul.x + _text_width(ptng, curp->label) + TNG_QB(ptng)->spacing.x;
         //Spew(DSRC_UI_Quickbox, ("p1 = (%d,%d) p2 = (%d,%d) p3 = (%d,%d) p4 = (%d,%d)\n",
         //   p1.x,p1.y,p2.x,p2.y,p3.x,p3.y,p4.x,p4.y));
         gr_set_fcolor(ptng->style->textColor);
         gr_int_line(p1.x, p1.y, p4.x, p4.y);
         gr_int_line(p2.x, p2.y, p4.x, p4.y);
         gr_int_line(p3.x, p3.y, p4.x, p4.y);
      }
      if (TNG_QB(ptng)->right_id != NULL)
         draw_resource_bm(TNG_QB(ptng)->right_id, r.lr.x - resource_bm_width(TNG_QB(ptng)->right_id), r.ul.y);
      else
      {
         // Draw goofy right arrow
         p1.x = r.lr.x - (ptng->style->frobsize.x / 2) - 2;
         p1.y = r.ul.y;
         p2.x = r.lr.x - ptng->style->frobsize.x - 2;
         p2.y = r.ul.y + (ptng->style->frobsize.y / 2);
         p3.x = p1.x;
         p3.y = r.ul.y + ptng->style->frobsize.y;
         p4.x = r.lr.x - 2;
         p4.y = p2.y;
         //Spew(DSRC_UI_Quickbox, ("p1 = (%d,%d) p2 = (%d,%d) p3 = (%d,%d) p4 = (%d,%d)\n",
         //   p1.x,p1.y,p2.x,p2.y,p3.x,p3.y,p4.x,p4.y));
         gr_set_fcolor(ptng->style->textColor);
         gr_int_line(p1.x, p1.y, p4.x, p4.y);
         gr_int_line(p2.x, p2.y, p4.x, p4.y);
         gr_int_line(p3.x, p3.y, p4.x, p4.y);
      }
   }
   return(OK);
}

errtype tng_draw_qb_text_slot(TNG *ptng, QuickboxSlot *curp, LGRect r)
{
   // Should do something goofy here with text frobs...
   // If this is auto-handled by having a one-line text TNG as a child of the
   // quickbox TNG, that would be highly spiffy.

   char *argp;
   LGRect argrect;     // Where the actual variable should go
   
   // Set some variables, compute some stuff
   argp = (char *)curp->var;
   r.ul.x += TNG_QB(ptng)->border.x;
   r.ul.y += TNG_QB(ptng)->spacing.y;
   argrect = r;
   if (TNG_QB(ptng)->options & QB_ALIGNMENT)
      argrect.ul.x = r.ul.x + TNG_QB(ptng)->internal_margin + TNG_QB(ptng)->spacing.x;
   else
      argrect.ul.x = r.ul.x + _text_width(ptng, curp->label) + TNG_QB(ptng)->spacing.x;

   // Draw that label
   gr_set_fcolor(ptng->style->textColor);
   _draw_text(ptng, curp->label, r.ul.x, r.ul.y+1);

   if (curp->options & QB_RD_ONLY)
   {
      _draw_text(ptng, (char *)(curp->var),argrect.ul.x, argrect.ul.y+1);
   }
   else if (strcmp((char *)curp->var, TNG_TX_GETLINE(curp->aux_tng,0)) != 0)
   {
      // If we are out of synch, rectify the problem
      TNG_TX_CLEARLINE(curp->aux_tng,0);
      TNG_TX_ADDSTRING(curp->aux_tng, (char *)curp->var);
   }

//   if (curp->aux_tng != NULL)
//      Spew(DSRC_UI_Quickbox, ("TX_GETLINE(curp->aux_tng,0) = %s\n",TNG_TX_GETLINE(curp->aux_tng,0)));
//   Spew(DSRC_UI_Quickbox, ("argp = <%s>\n",argp));

   return(OK);
}

errtype tng_draw_qb_bool_slot(TNG *ptng, QuickboxSlot *curp, LGRect r)
{
   bool *argp;
   LGRect argrect;     // Where the actual variable should go
   LGPoint p1,p2,p3,p4;
   char temp[100];
   short base;

   // Set some variables, compute some stuff
   argp = (bool *)curp->var;
   r.ul.x += TNG_QB(ptng)->border.x;
   r.ul.y += TNG_QB(ptng)->spacing.y;
   argrect = r;
   if (TNG_QB(ptng)->options & QB_ALIGNMENT)
      argrect.ul.x = r.ul.x + TNG_QB(ptng)->internal_margin + (2 * TNG_QB(ptng)->spacing.x);
   else
      argrect.ul.x = r.ul.x + _text_width(ptng, curp->label) + (2 * TNG_QB(ptng)->spacing.x);
   if (curp->options & QB_ARROWS)
   {
      // Goofy arrows
      if (TNG_QB(ptng)->left_id != NULL)
         argrect.ul.x += resource_bm_width(TNG_QB(ptng)->left_id);
      else
         argrect.ul.x += ptng->style->frobsize.x;
      if (TNG_QB(ptng)->right_id != NULL)
         argrect.lr.x -= resource_bm_width(TNG_QB(ptng)->right_id);
      else
         argrect.lr.x -= ptng->style->frobsize.x;
   }

   // Draw that label
   gr_set_fcolor(ptng->style->textColor);
   _draw_text(ptng, curp->label, r.ul.x, r.ul.y+1);

   // Draw in the actual value of the variable
   if (*argp)
      strcpy(temp, "True");
   else
      strcpy(temp, "False");
   _draw_text(ptng, temp, argrect.ul.x, argrect.ul.y+1);

   // Draw in goofy arrows, if they are necessary
   if (curp->options & QB_ARROWS)
   {
      if (TNG_QB(ptng)->left_id != NULL)
      {
         if (TNG_QB(ptng)->options & QB_ALIGNMENT)
            draw_resource_bm(TNG_QB(ptng)->left_id, r.ul.x + TNG_QB(ptng)->internal_margin + TNG_QB(ptng)->spacing.x, r.ul.y);
         else
            draw_resource_bm(TNG_QB(ptng)->left_id, r.ul.x + _text_width(ptng, curp->label) + TNG_QB(ptng)->spacing.x, r.ul.y);
      }
      else
      {
         // Draw goofy left arrow
         if (TNG_QB(ptng)->options & QB_ALIGNMENT)
            base = r.ul.x + TNG_QB(ptng)->internal_margin + TNG_QB(ptng)->spacing.x;
         else
            base = r.ul.x + _text_width(ptng, curp->label) + TNG_QB(ptng)->spacing.x;
         p1.x = base + (ptng->style->frobsize.x / 2);
         p1.y = r.ul.y;  
         p2.x = base + ptng->style->frobsize.x;
         p2.y = r.ul.y + (ptng->style->frobsize.y / 2);
         p3.x = p1.x;
         p3.y = r.ul.y + ptng->style->frobsize.y;
         p4.x = base;
         p4.y = p2.y;

         if (TNG_QB(ptng)->options & QB_ALIGNMENT)
            p4.x = r.ul.x + TNG_QB(ptng)->internal_margin + TNG_QB(ptng)->spacing.x;
         else
            p4.x = r.ul.x + _text_width(ptng, curp->label) + TNG_QB(ptng)->spacing.x;
         //Spew(DSRC_UI_Quickbox, ("p1 = (%d,%d) p2 = (%d,%d) p3 = (%d,%d) p4 = (%d,%d)\n",
         //   p1.x,p1.y,p2.x,p2.y,p3.x,p3.y,p4.x,p4.y));
         gr_set_fcolor(ptng->style->textColor);
         gr_int_line(p1.x, p1.y, p4.x, p4.y);
         gr_int_line(p2.x, p2.y, p4.x, p4.y);
         gr_int_line(p3.x, p3.y, p4.x, p4.y);
      }
      if (TNG_QB(ptng)->right_id != NULL)
         draw_resource_bm(TNG_QB(ptng)->right_id, r.lr.x - resource_bm_width(TNG_QB(ptng)->right_id), r.ul.y);
      else
      {
         // Draw goofy right arrow
         p1.x = r.lr.x - (ptng->style->frobsize.x / 2) - 2;
         p1.y = r.ul.y;
         p2.x = r.lr.x - ptng->style->frobsize.x - 2;
         p2.y = r.ul.y + (ptng->style->frobsize.y / 2);
         p3.x = p1.x;
         p3.y = r.ul.y + ptng->style->frobsize.y;
         p4.x = r.lr.x - 2;
         p4.y = p2.y;
         //Spew(DSRC_UI_Quickbox, ("p1 = (%d,%d) p2 = (%d,%d) p3 = (%d,%d) p4 = (%d,%d)\n",
         //   p1.x,p1.y,p2.x,p2.y,p3.x,p3.y,p4.x,p4.y));
         gr_set_fcolor(ptng->style->textColor);
         gr_int_line(p1.x, p1.y, p4.x, p4.y);
         gr_int_line(p2.x, p2.y, p4.x, p4.y);
         gr_int_line(p3.x, p3.y, p4.x, p4.y);
      }
   }
   return(OK);
}

errtype tng_draw_qb_pb_slot(TNG *ptng, QuickboxSlot *curp, LGRect r)
{
   r.ul.x += TNG_QB(ptng)->border.x;
   r.ul.y += TNG_QB(ptng)->spacing.y;
   // Draw that label, if necessary
   if (curp->p2 != NULL)
   {
      gr_set_fcolor(ptng->style->textColor);
      _draw_text(ptng, curp->label, r.ul.x, r.ul.y+1);
   }

   return(OK);
}

// assumes all appropriate setup has already been done!
errtype tng_quickbox_2d_draw(TNG *ptng, ushort , LGPoint loc)
{
   TNG_quickbox *pqbtng;
   QuickboxSlot *curp;
   LGRect r;
   
   //Spew(DSRC_UI_Quickbox, ("TNG quickbox 2d Draw at (%d, %d) -- partmask = %x\n",loc.x,loc.y,partmask));
   TNG_IF_OBSCURED(ptng)
   {
      return(OK);
   }
   ptng->signal(ptng, TNG_SIGNAL_EXPOSE);
   pqbtng = TNG_QB(ptng);
   if ((pqbtng->options & QB_ALIGNMENT) && (pqbtng->internal_margin == -1))
      return(OK);
   TNGDrawBase(ptng, loc, pqbtng->size);
   r.ul.x = loc.x + pqbtng->border.x;
   r.ul.y = loc.y + pqbtng->border.y;
   r.lr.x = r.ul.x + pqbtng->size.x - pqbtng->border.x - 1;
   r.lr.y = r.ul.y + pqbtng->size.y;

   // Let us iterate through the slots, shall we?
   curp = QB_SLOTS(ptng);
   while (curp != NULL)
   {
      gad_qbox_display_slot(curp, FALSE);
      curp = curp->next;
   }

   return(OK);
}

// Fill in ppt with the size...
errtype tng_quickbox_size(TNG *ptng, LGPoint *ppt)
{
   *ppt = TNG_QB(ptng)->size;
   return(OK);
}

// Returns the current "value" of the TNG
int tng_quickbox_getvalue(TNG *ptng)
{
   int i = 0;
   QuickboxSlot *pqs, *curr;

   curr = TNG_QB(ptng)->current_slot;
   pqs = TNG_QB(ptng)->slots;
   while ((pqs != curr) && (pqs != NULL))
   {
      i++;
      pqs = pqs->next;
   }
   if (pqs == NULL)
      i = -1;
   return(i);
}

// React appropriately for receiving the specified cooked key
bool tng_quickbox_keycooked(TNG *ptng, ushort key)
{
   int code = key & 0xff;
   bool retval =FALSE;
   QuickboxSlot *qbs, *prevp, *curp;

   qbs = QB_CURRENT(ptng);
//   Spew(DSRC_UI_Quickbox, ("keyboard CB: %x\n", code));
   if (qbs->aux_tng != NULL)
      IF_SET_RV(qbs->aux_tng->keycooked(qbs->aux_tng, key));
   if (qbs->options & QB_ARROWS)
   {
      if (code == QB_LEFT_KEY)
         IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_DECREMENT));
      if (code == QB_RIGHT_KEY)
         IF_SET_RV(ptng->signal(ptng, TNG_SIGNAL_INCREMENT));
   }
   if (code == QB_DOWN_KEY)
   {
      if (qbs->next != NULL)
      {
         QB_CURRENT(ptng) = qbs->next;
      }
      else
      {
         QB_CURRENT(ptng) = QB_SLOTS(ptng);
      }
      gad_qbox_display_slot(qbs, TRUE);
      gad_qbox_display_slot(QB_CURRENT(ptng), TRUE);
   }
   if (code == QB_UP_KEY)
   {
      prevp = NULL;
      curp = QB_SLOTS(ptng);
      while ((curp != qbs) && (curp != NULL))
      {
         prevp = curp;
         curp = curp->next;
      }
      if (prevp == NULL)
      {
         curp = QB_SLOTS(ptng);
         while ((curp != NULL) && (curp->next != NULL))
            curp = curp->next;
         QB_CURRENT(ptng) = curp;
      }
      else
      {
         QB_CURRENT(ptng) = prevp;
      }
      gad_qbox_display_slot(qbs, TRUE);
      gad_qbox_display_slot(QB_CURRENT(ptng),TRUE);
   }
   IF_SET_RV(tng_cb_keycooked(ptng, key));
   if ((code == QB_UP_KEY) || (code == QB_DOWN_KEY) || (code == QB_LEFT_KEY) || (code == QB_RIGHT_KEY))
      retval = TRUE;
   return(retval);
}

// React appropriately for receiving the specified mouse button event
bool tng_quickbox_mousebutt(TNG *ptng, uchar type, LGPoint loc)
{
   int localy, tw;
   int slotcount = 0;
   TNG_quickbox *pqbtng;
   QuickboxSlot *oldcur, *curp;
   bool retval = FALSE;

   if ((type == TNG_MOUSE_LDOWN) || (type == TNG_MOUSE_RDOWN))
   {
      pqbtng = TNG_QB(ptng);
      localy = loc.y - pqbtng->border.y;
      curp = QB_SLOTS(ptng);
      while ((curp != NULL) && (localy > (pqbtng->spacing.y + pqbtng->slot_size.y)))
      {
         localy -= pqbtng->spacing.y + pqbtng->slot_size.y;
         slotcount++;
         curp = curp->next;
      }
      if (curp != NULL)
      {
         if (localy <= pqbtng->slot_size.y)
         {
            oldcur = QB_CURRENT(ptng);
            QB_CURRENT(ptng) = curp;
            gad_qbox_display_slot(oldcur, TRUE);
            gad_qbox_display_slot(curp, TRUE);
         }
         if (curp->options & QB_ARROWS)
         {
            if (curp->vartype == QB_BOOL_SLOT)
            {
               ptng->signal(ptng, TNG_SIGNAL_INCREMENT);
            }
            else
            {
               if (TNG_QB(ptng)->options & QB_ALIGNMENT)
                  tw = TNG_QB(ptng)->internal_margin;
               else
                  tw = _text_width(ptng, curp->label);
               if ((pqbtng->left_id != NULL) &&
                  (loc.x < resource_bm_width(pqbtng->left_id) + tw + pqbtng->border.x + (2 * pqbtng->spacing.x)))
               {
                  if (type == TNG_MOUSE_LDOWN)
                     ptng->signal(ptng, TNG_SIGNAL_DECREMENT);
                  else
                     tng_decrem_slot(ptng, 10);
               }
               else if (loc.x < tw + pqbtng->border.x + ptng->style->frobsize.x + (2 * pqbtng->spacing.x))
               {
                  if (type == TNG_MOUSE_LDOWN)
                     ptng->signal(ptng, TNG_SIGNAL_DECREMENT);
                  else
                     tng_decrem_slot(ptng, 10);
               }
               if ((pqbtng->right_id != NULL) &&   
                  (loc.x > pqbtng->size.x - pqbtng->border.x - resource_bm_width(pqbtng->right_id) - 2 ))
               {
                  if (type == TNG_MOUSE_LDOWN)
                     ptng->signal(ptng, TNG_SIGNAL_INCREMENT);
                  else
                     tng_increm_slot(ptng, 10);
               }
               else if (loc.x > pqbtng->size.x - pqbtng->border.x - ptng->style->frobsize.x - 2)
               {
                  if (type == TNG_MOUSE_LDOWN)
                     ptng->signal(ptng, TNG_SIGNAL_INCREMENT);
                  else
                     tng_increm_slot(ptng, 10);
               }
            }
         }
      }
   }
   IF_SET_RV(tng_cb_mousebutt(ptng,type,loc));
//   retval = TRUE;
   return(retval);
}

// Handle incoming signals
bool tng_quickbox_signal(TNG *ptng, ushort signal)
{
   bool retval = FALSE;
   if (signal & TNG_SIGNAL_INCREMENT)
      tng_increm_slot(ptng, 1);
   if (signal & TNG_SIGNAL_DECREMENT)
      tng_decrem_slot(ptng, 1);
//   if (signal & TNG_SIGNAL_CHANGED)
//      TNG_DRAW(ptng);
   IF_SET_RV(tng_cb_signal(ptng,signal));
   retval = TRUE;
   return(retval);
}

// Add a line to a quickbox.  slot_type describes the type of slot, var is a pointer to the variable to be
// displaying, and slot_options describes any additional modifiers to the qbox.  Note that some bizarre-o 
// combinations of options and types might not be implemented.
errtype tng_quickbox_add(char *label, int slot_type, void *var, ulong slot_options)
{
   return(tng_quickbox_add_parm(label,slot_type,var,slot_options,NULL,NULL));
}

// Just like gad_qbox_add but allows two parameters to be set for the slot.  Certain slot options require
// this form of accessing.
errtype tng_quickbox_add_parm(char *label, int slot_type, void *var, ulong slot_options, void *parm1, void *parm2)
{
   QuickboxSlot *newslot, *curp;
   
   newslot = (QuickboxSlot *)NewPtr(sizeof(QuickboxSlot));
   
   // Fill out newslot
   num_slots++;
   newslot->options = slot_options;
   newslot->vartype = slot_type;
   newslot->var = var;
   newslot->p1 = parm1;
   newslot->p2 = parm2;
   newslot->tng = current_tng;
   newslot->next = NULL;
   newslot->aux_tng = NULL;
   newslot->aux_size = tngZeroPt;
   newslot->label = (char *)NewPtr(sizeof(char) * 100);
   strcpy(newslot->label, label);

   // Update size of overall box
   if (num_slots != 1)
      TNG_QB(current_tng)->size.y += TNG_QB(current_tng)->spacing.y;
   TNG_QB(current_tng)->size.y += TNG_QB(current_tng)->slot_size.y;

   // Add at end of current box's list of slots
   if (QB_CURRENT(current_tng) == NULL)
      QB_CURRENT(current_tng) = newslot;
   curp = QB_SLOTS(current_tng);
   while ((curp != NULL) && (curp->next != NULL))
      curp = curp->next;

   if (curp == NULL)
      QB_SLOTS(current_tng) = newslot;
   else
      curp->next = newslot;

   return (OK);
}

// This represents that the quickbox is done being created and is ready for display, input, etc.
errtype tng_quickbox_end()
{
   int cid, slotcount = 0;
   int max, min;
   QuickboxSlot *curp;
   ulong text_options;
   LGRect *sdim;
   LGPoint aux_size;
   TNG_quickbox *pqbtng;
/*
   Spew(DSRC_UI_Quickbox, ("Beginning of the qbox_end\n"));
   if (current_tng != NULL)
      Spew(DSRC_UI_Quickbox, ("current_tng not null\n"));
   else
      Spew(DSRC_UI_Quickbox, ("current_tng IS NULL!!!! AIAIAIIAGHGGGGHHHHH!\n"));
*/
   pqbtng = TNG_QB(current_tng);

   // compute options-type stuff...
   if (pqbtng->options & QB_ALIGNMENT)
   {
     pqbtng->internal_margin = _label_extent(current_tng);
     //Spew(DSRC_UI_Quickbox, ("Yay alignment  margin = %d!!\n",pqbtng->internal_margin)); 
   }
//   else
//   {
//     Spew(DSRC_UI_Quickbox, ("There is NOOOOO alignment!!\n")); 
//   }

   if (pqbtng->options & QB_AUTOSIZE)
   {
      //Spew(DSRC_UI_Quickbox, ("Autosizing...\n"));
      pqbtng->slot_size.x = _total_extent(current_tng);
   }

  // Make children TNGs, if necessary
   curp = QB_SLOTS(current_tng);
   while (curp != NULL)
   { 
      if (curp->options & QB_STRINGSET)
      {
         curp->options |= QB_ARROWS;
         curp->p2 = (void *)((int)(curp->p2) -1);
      }
      if ((curp->options & QB_SLIDER) ||
          (curp->vartype == QB_PUSHBUTTON_SLOT) ||
          (curp->vartype == QB_TEXT_SLOT) ||
          (curp->vartype == QB_FIX_SLOT) ||
          ( ((curp->vartype == QB_SHORT_SLOT) || (curp->vartype == QB_BYTE_SLOT) || (curp->vartype == QB_INT_SLOT) || (curp->vartype == QB_UINT_SLOT))
            && !(curp->options & QB_RD_ONLY)))
      { 
         sdim = (LGRect *)NewPtr(sizeof(LGRect));
         if ((curp->options & QB_SLIDER) ||
            (((curp->vartype == QB_SHORT_SLOT) || (curp->vartype == QB_INT_SLOT) || (curp->vartype == QB_BYTE_SLOT) || (curp->vartype == QB_UINT_SLOT)
               || (curp->vartype == QB_FIX_SLOT))
               
               && !(curp->options & QB_RD_ONLY) && !(curp->options & QB_ARROWS)))
            sdim->ul.x = pqbtng->size.x - pqbtng->aux_size - pqbtng->border.x;
         else if (curp->vartype == QB_PUSHBUTTON_SLOT)
         {
            if (curp->p2 == NULL)
               sdim->ul.x = pqbtng->border.x;
            else
               sdim->ul.x = pqbtng->size.x - resource_bm_width(*((Ref *)curp->p2));
         }
         else if ((curp->vartype == QB_TEXT_SLOT) && (!(curp->options & QB_RD_ONLY)))
         {
            if (pqbtng->options & QB_ALIGNMENT)
               sdim->ul.x = pqbtng->internal_margin + pqbtng->border.x + pqbtng->spacing.x;
            else
               sdim->ul.x = _text_width(current_tng, curp->label) + pqbtng->border.x + pqbtng->spacing.x;
         }
         sdim->lr.x = pqbtng->size.x - pqbtng->border.x;
         sdim->ul.y = (slotcount * (pqbtng->slot_size.y + pqbtng->spacing.y)) + pqbtng->border.y;
         sdim->lr.y = sdim->ul.y + pqbtng->slot_size.y;
         aux_size.x = RectWidth(sdim) - 1;
         aux_size.y = RectHeight(sdim) - 1; 
         curp->aux_size = aux_size;
         //Spew(DSRC_UI_Quickbox, ("aux_size = (%d,%d)  sdim = (%d,%d)(%d,%d)\n",aux_size.x, aux_size.y,
         //   RECT_EXPAND_ARGS(sdim)));
         if (curp->options & QB_SLIDER)
         {
            min = (int)curp->p1;
            max = (int)curp->p2;
            //Spew(DSRC_UI_Quickbox, ("About to create slider, sdim = (%d, %d) (%d, %d), max = %d, min = %d\n",
            //   RECT_EXPAND_ARGS(sdim), max, min));
            TNG_CREATE_SLIDER(current_tng->ui_data, sdim->ul, &(curp->aux_tng), current_tng->style, TNG_SL_HORIZONTAL,
               min, max, *((int *)(curp->var)), 1, aux_size);
//            TNG_DRAW(curp->aux_tng);
            tng_install_callback(curp->aux_tng, TNG_EVENT_SIGNAL, TNG_SIGNAL_CHANGED, &tng_quickbox_scroll_changed, curp, &cid);   
         }
         if (curp->vartype == QB_PUSHBUTTON_SLOT)
         {
            if (curp->p2 == NULL)
               TNG_CREATE_PUSHBUTTON(current_tng->ui_data, sdim->ul,&(curp->aux_tng), current_tng->style, TEXT_TYPE, (void *)curp->label, aux_size);
            else
               TNG_CREATE_PUSHBUTTON(current_tng->ui_data, sdim->ul, &(curp->aux_tng), current_tng->style, RESOURCE_TYPE, (void *)curp->p2, aux_size);
            tng_install_callback(curp->aux_tng, TNG_EVENT_SIGNAL, TNG_SIGNAL_SELECT, (TNGCallback)curp->var, curp->p1, &cid);
         }
         if (((curp->vartype == QB_SHORT_SLOT) || (curp->vartype == QB_INT_SLOT) || (curp->vartype == QB_UINT_SLOT)
            || (curp->vartype == QB_BYTE_SLOT) || (curp->vartype == QB_FIX_SLOT))
            && !(curp->options & QB_RD_ONLY) && !(curp->options & QB_SLIDER)
            && !(curp->options & QB_ARROWS))
         {
            text_options = TNG_TG_SINGLE_LINE;
            TNG_CREATE_TEXT(current_tng->ui_data, sdim->ul, &(curp->aux_tng), current_tng->style, text_options, aux_size);
#ifdef STARTING_VALUE_STRINGS
            if (curp->vartype == QB_INT_SLOT)
               sprintf (temp, "%d", *((int *)(curp->var)));
            else if (curp->vartype == QB_UINT_SLOT)
               sprintf (temp, "%d", *((uint *)(curp->var)));
            else if (curp->vartype == QB_SHORT_SLOT)
               sprintf (temp, "%d", *((short *)(curp->var)));
            else if (curp->vartype == QB_BYTE_SLOT)
               sprintf (temp, "%d", *((ubyte *)(curp->var)));
            else if (curp->vartype == QB_FIX_SLOT)
               sprintf (temp, "%f", fix_float(*((fix *)(curp->var))));
            TNG_TX_ADDSTRING(curp->aux_tng, "");
#endif
            tng_install_callback(curp->aux_tng, TNG_EVENT_SIGNAL, TNG_SIGNAL_SELECT, tng_quickbox_text_changed, curp, &cid);
         }
         if ((curp->vartype == QB_TEXT_SLOT) && (!(curp->options & QB_RD_ONLY)))
         {
            text_options = TNG_TG_SINGLE_LINE;
            //Spew(DSRC_UI_Quickbox, ("About to make textgadg, slotcount = %d, sdim = (%d,%d)(%d,%d)\n",slotcount,RECT_EXPAND_ARGS(sdim)));
            TNG_CREATE_TEXT(current_tng->ui_data, sdim->ul, &(curp->aux_tng), current_tng->style, text_options, aux_size);
            TNG_TX_ADDSTRING(curp->aux_tng, (char *)curp->var);
            tng_install_callback(curp->aux_tng, TNG_EVENT_SIGNAL, TNG_SIGNAL_SELECT, tng_quickbox_text_changed, curp, &cid);
         }
      }
      curp = curp->next;
      slotcount++;
   }
   TNG_DRAW(current_tng);
   return(OK);
}

errtype _draw_text(TNG *ptng, char *txt, int x_coord, int y_coord)
{
   return(TNGDrawText(ptng->style->font, txt, x_coord, y_coord));
}

// Returns width of text, in pixels
int _text_width(TNG *ptng, char *t)
{
   int retval;
   gr_set_font((grs_font *)ResLock(ptng->style->font));
   retval = gr_string_width(t);   
   ResUnlock(ptng->style->font);
   return(retval);
}

errtype gad_qbox_display_slot(QuickboxSlot *qbs, bool recurse)
{
   LGRect srect;
   TNG *ptng;
   QuickboxSlot *curp;
   int slotcount = 0;

   ptng = qbs->tng;
   curp = QB_SLOTS(ptng);
   while ((curp != NULL) && (curp != qbs))
   {
      slotcount++;
      curp = curp->next;
   }
      _slot_rectangle(ptng, slotcount, &srect);
      uiHideMouse(&srect);
      gr_set_fcolor(ptng->style->backColor);
      gr_rect(srect.ul.x, srect.ul.y, srect.lr.x, srect.lr.y);
      if (curp == QB_CURRENT(ptng))
      {
         gr_set_fcolor(ptng->style->altTextColor);
         gr_box(srect.ul.x, srect.ul.y, srect.lr.x, srect.lr.y);
      }
      switch (curp->vartype)
      {
         case QB_BYTE_SLOT:
         case QB_SHORT_SLOT:
         case QB_INT_SLOT:
         case QB_UINT_SLOT:
         case QB_FIX_SLOT:
            tng_draw_qb_int_slot(ptng, curp, srect);
            break;
         case QB_TEXT_SLOT:
            tng_draw_qb_text_slot(ptng, curp, srect);
            break;
         case QB_BOOL_SLOT:
            tng_draw_qb_bool_slot(ptng, curp, srect);
            break;
         case QB_PUSHBUTTON_SLOT:
            tng_draw_qb_pb_slot(ptng, curp, srect);
            break;
      }
      if (recurse && (qbs->aux_tng != NULL))
         TNG_DRAW(qbs->aux_tng);
      uiShowMouse(&srect);
   return(OK);
}

errtype tng_quickbox_rename_slot(TNG *qb, int slot_num, char *new_name)
{
   QuickboxSlot *psl;
   int count = 0;

   psl = QB_SLOTS(qb);
   while (count < slot_num)
   {
      psl = psl->next;
      count++;
   }
   strcpy(psl->label, new_name);
   return(OK);
}

errtype _slot_rectangle(TNG *qb, int slot_num, LGRect *slot_rect)
{
   slot_rect->ul = TNG_ABSLOC(qb); 
   slot_rect->ul.y += (slot_num * (TNG_QB(qb)->slot_size.y + TNG_QB(qb)->spacing.y)) + 1;
   slot_rect->ul.x += TNG_QB(qb)->border.x;
   slot_rect->lr.x = slot_rect->ul.x + TNG_QB(qb)->size.x - (2 * TNG_QB(qb)->border.x);
   slot_rect->lr.y = slot_rect->ul.y + TNG_QB(qb)->slot_size.y + TNG_QB(qb)->spacing.y - 1;
//   mprintf ("spacing = (%d,%d)\n",TNG_QB(qb)->spacing.x, TNG_QB(qb)->spacing.y);
//   mprintf ("slot_rectangle, slot %d = (%d, %d)(%d, %d)\n",slot_num, RECT_EXPAND_ARGS(slot_rect));
   return(OK);
}

int _label_extent(TNG *ptng)
{
   QuickboxSlot *qbs;
   int retval = 0, v;

   qbs = QB_SLOTS(ptng);
   while (qbs != NULL)
   {
      if (((qbs->vartype == QB_PUSHBUTTON_SLOT) && (qbs->p2 == NULL)) ||
         ((qbs->vartype == QB_TEXT_SLOT) && (qbs->options & QB_RD_ONLY)))
         v = 0;
      else
         v = _text_width(ptng, qbs->label);
      if (v > retval)
         retval = v;
      qbs = qbs->next;
   }
   //Spew(DSRC_UI_Quickbox, ("label extent = %d\n",retval));
   return(retval);
}

int _total_extent(TNG *ptng)
{
   QuickboxSlot *qbs;
   int v, retval;
   char temp[40];
   retval = 0;

   qbs = QB_SLOTS(ptng);
   while (qbs != NULL)
   {
      if (TNG_QB(ptng)->options & QB_ALIGNMENT)
         v = TNG_QB(ptng)->internal_margin;
      else
         v = _text_width(ptng, qbs->label);
      v += TNG_QB(ptng)->spacing.x + (2 * TNG_QB(ptng)->border.x);
      switch(qbs->vartype)
      {
         case QB_SHORT_SLOT:
         case QB_BYTE_SLOT:
         case QB_INT_SLOT:
         case QB_UINT_SLOT:
         case QB_FIX_SLOT:
            if (qbs->options & QB_SLIDER)
               v += TNG_QB(ptng)->aux_size;
            if (qbs->options & QB_RD_ONLY)
            {
               if (qbs->vartype == QB_INT_SLOT)
                  sprintf(temp, "%d", *((int *)(qbs->var)));
               else if (qbs->vartype == QB_UINT_SLOT)
                  sprintf(temp, "%d", *((uint *)(qbs->var)));
               else if (qbs->vartype == QB_SHORT_SLOT)
                  sprintf(temp, "%d", *((short *)(qbs->var)));
               else if (qbs->vartype == QB_BYTE_SLOT)
                  sprintf(temp, "%d", *((ubyte *)(qbs->var)));
               else if (qbs->vartype == QB_FIX_SLOT)
                  sprintf (temp, "%f", fix_float(*((fix *)(qbs->var))));
               v += _text_width(ptng, temp);
            }
            else
            {
               v += TNG_QB(ptng)->aux_size;
            }
            if (qbs->options & QB_ARROWS)
            {
               if (TNG_QB(ptng)->left_id == NULL)
                  v += ptng->style->frobsize.x;
               else
                  v += resource_bm_width(TNG_QB(ptng)->left_id);
               if (TNG_QB(ptng)->right_id == NULL)
                  v += ptng->style->frobsize.x;
               else
                  v += resource_bm_width(TNG_QB(ptng)->right_id);
            }
            break;
         case QB_BOOL_SLOT:
            if (qbs->options & QB_SLIDER)
               v += TNG_QB(ptng)->aux_size;
            if (qbs->options & QB_RD_ONLY)
            {
               if (*((bool *)(qbs->var)))
                  sprintf(temp, "TRUE");
               else
                  sprintf(temp, "FALSE");
               v += _text_width(ptng, temp);
            }
            else
            {
               v += TNG_QB(ptng)->aux_size;
            }
            if (qbs->options & QB_ARROWS)
            {
               if (TNG_QB(ptng)->left_id == NULL)
                  v += ptng->style->frobsize.x;
               else
                  v += resource_bm_width(TNG_QB(ptng)->left_id);
               if (TNG_QB(ptng)->right_id == NULL)
                  v += ptng->style->frobsize.x;
               else
                  v += resource_bm_width(TNG_QB(ptng)->right_id);
            }
            break;
         case QB_TEXT_SLOT:
            if (!(qbs->options & QB_RD_ONLY))
               v += TNG_QB(ptng)->aux_size;
            break;
         case QB_PUSHBUTTON_SLOT:
            if (qbs->p2 != NULL)
               v += resource_bm_width(*((Ref *)(qbs->p2)));
            break;
      }
      if (v > retval)
         retval = v;
      qbs = qbs->next;
   }

   return(retval);
}























                                                            
