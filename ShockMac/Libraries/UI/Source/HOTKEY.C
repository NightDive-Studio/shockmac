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
#define __HOTKEY_SRC
#include "hotkey.h"
#include "hash.h"
//#include <_ui.h>

#ifdef HOTKEY_HELP
#include <string.h>
#endif

#define CHAIN_LENGTH 2
#define CHAIN_END -1

ulong HotkeyContext = 0xFFFFFFFF;


#pragma require_prototypes off
int hotkey_hash_func(void* v)
{
   hotkey_entry* e = (hotkey_entry*)v;
   return e->key;
}

int hotkey_equ_func(void* v1, void* v2)
{
   return ((hotkey_entry*)v1)->key - ((hotkey_entry*)v2)->key;
}
#pragma require_prototypes on


errtype hotkey_init(int tblsize)
{
   return hash_init(&hotkey_table,sizeof(hotkey_entry),tblsize,hotkey_hash_func,hotkey_equ_func);
}

errtype hotkey_add(short keycode, ulong contexts, hotkey_callback func, void* state)
{
#ifdef HOTKEY_HELP
   return(hotkey_add_help(keycode,contexts,func,state,NULL));
}

errtype hotkey_add_help(short keycode, ulong contexts, hotkey_callback func, void* state, char * /*help_text*/)
{
#endif
   hotkey_entry e,*ch;
   errtype err;
   int i;
   hotkey_link *chain;
   e.key = keycode;
   err = hash_lookup(&hotkey_table,&e,(void **)&ch);
   if (err != OK) return err;
   if (ch == NULL)
   {
//      Spew(DSRC_UI_Hotkey,("Creating new hotkey chain\n"));
      err = hash_insert(&hotkey_table,&e);
      if (err != OK) return err;
      hash_lookup(&hotkey_table,&e,(void **)&ch);
      array_init(&ch->keychain,sizeof(hotkey_link),CHAIN_LENGTH);
      ch->first = CHAIN_END;
   }
   err = array_newelem(&ch->keychain,&i);
   if (err != OK) return err;
   chain = (hotkey_link*)ch->keychain.vec;
   chain[i].context = contexts;
   chain[i].func = func;
   chain[i].state = state;
#ifdef HOTKEY_HELP
//   chain[i].help_text = NewPtr(strlen(help_text)+1);
//   strcpy(chain[i].help_text,help_text);
#endif
   chain[i].next = ch->first;
   ch->first = i;
   return OK;
}

/* KLC - not used
#ifdef HOTKEY_HELP
char *hotkey_help_text(short keycode, ulong contexts, hotkey_callback func)
{
   hotkey_entry *ch;
   errtype err;
   int i;
   hotkey_link *chain;
   err = hash_lookup(&hotkey_table,(hotkey_entry*)&keycode,(void **)&ch);
   if (err != OK) return(NULL) ;
   if (ch == NULL) return(NULL);
   chain = (hotkey_link*)ch->keychain.vec;
   for (i = ch->first; chain[i].func == func;)
   {
      chain[i].context &= ~contexts;
      if (chain[i].context == 0)
      {
            return(chain[i].help_text);
      }
   }
   for(i = ch->first; chain[i].next != CHAIN_END; i = chain[i].next)
   {
      int n = chain[i].next;
      if (chain[n].func == func)
      {
         chain[n].context &= ~contexts;
         if (chain[n].context == 0)
         {
            return(chain[n].help_text);
         }
      }
   }
   return(NULL);
}
#endif
*/

errtype hotkey_remove(short keycode, ulong contexts, hotkey_callback func)
{
   hotkey_entry *ch;
   errtype err;
   int i;
   hotkey_link *chain;
   err = hash_lookup(&hotkey_table,(hotkey_entry*)&keycode,(void **)&ch);
   if (err != OK) return err;
   if (ch == NULL) return ERR_NOEFFECT;
   chain = (hotkey_link*)ch->keychain.vec;
   for (i = ch->first; chain[i].func == func;)
   {
      chain[i].context &= ~contexts;
      if (chain[i].context == 0)
      {
         ch->first = chain[i].next;
#ifdef HOTKEY_HELP
//         DisposePtr(chain[i].help_text);
#endif // HOTKEY_HELP
         array_dropelem(&ch->keychain,i);
         i = ch->first;
      }
   }
   for(i = ch->first; chain[i].next != CHAIN_END; i = chain[i].next)
   {
      int n = chain[i].next;
      if (chain[n].func == func)
      {
         chain[n].context &= ~contexts;
         if (chain[n].context == 0)
         {
            chain[i].next = chain[n].next;
#ifdef HOTKEY_HELP
//            DisposePtr(chain[i].help_text);
#endif // HOTKEY_HELP
            array_dropelem(&ch->keychain,n);
         }
      }
   }
   return OK;
}


errtype hotkey_dispatch(short keycode)
{
   hotkey_entry *ch;
   errtype err;
   int i;
   hotkey_link *chain;
   err = hash_lookup(&hotkey_table,(hotkey_entry*)&keycode,(void **)&ch);
   if (err != OK) return err;
   if (ch == NULL) return ERR_NOEFFECT;
   chain = (hotkey_link*)ch->keychain.vec;
   for (i = ch->first; i != CHAIN_END; i = chain[i].next)
   {
//      Spew(DSRC_UI_Hotkey,("checking link %d \n",i));
      if (chain[i].context & HotkeyContext)
      {
//         Spew(DSRC_UI_Hotkey,("Succeeded context test %d\n",chain[i].context));
         if (chain[i].func(keycode,HotkeyContext,chain[i].state))
            return OK;
      }
   }
   return ERR_NOEFFECT;
}

static bool shutdown_iter_func(void* elem, void* data)
{
#ifndef NO_DUMMIES
   void *dummy = data;
#endif // NO_DUMMIES
   hotkey_entry* ch = (hotkey_entry*)elem;
/* KLC
#ifdef HOTKEY_HELP
   int i;
   hotkey_link *chain = (hotkey_link*)(ch->keychain.vec);
   
   if (ch == NULL) return FALSE;
   for (i = ch->first; i != CHAIN_END; i = chain[i].next)
   {
      DisposePtr(chain[i].help_text);
   }
#endif // HOTKEY_HELP
*/
#ifndef NO_DUMMIES
   data = dummy;
#endif // NO_DUMMIES
   array_destroy(&ch->keychain);
   return FALSE;
}

errtype hotkey_shutdown(void)
{
   hash_iter(&hotkey_table,shutdown_iter_func,NULL);
   hash_destroy(&hotkey_table);
   return OK;
}

int list_index = 0;

#ifdef GODDAMN_THIS_MESS_IS_IMPOSSIBLE
bool hotkey_list(char **item, int sort_type)
{
   void *res;
   hotkey_entry* ch;
   hotkey_link *chain;
   int i;

   hash_step(&hotkey_table, &res, &list_index);
   ch = (hotkey_entry *)res;
   if (ch == NULL) return ERR_NOEFFECT;
   chain = (hotkey_link*)ch->keychain.vec;
   strcpy(*item, "");
   for (i = ch->first; i != CHAIN_END; i = chain[i].next)
   {
      strcat(*item, 
      if (chain[i].context & HotkeyContext)
      {
         Spew(DSRC_UI_Hotkey,("Succeeded context test %d\n",chain[i].context));
         if (chain[i].func(keycode,HotkeyContext,chain[i].state))
            return OK;
      }
   }
   strcpy(*item, 
}

errtype hotkey_list_clear()
{
   list_index = 0;
}

#endif

   

