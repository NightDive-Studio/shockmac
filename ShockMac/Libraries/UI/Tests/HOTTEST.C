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
#include "lg.h"
#include "kb.h"
#include "kbcook.h"
#include "hotkey.h"

#include <stdlib.h>
#include <stdio.h>


uchar keybuf[512];

#define PING 1
#define PONG 2

bool toggle_mode(short keycode, ulong context, void* state)
{
   if (context == PING)
   {
      printf("Entering pong mode\n");
      HotkeyContext = PONG;
   }
   else
   {
      printf("Entering ping mode\n");
      HotkeyContext = PING;
   }
   return TRUE;
}


bool ping(short keycode, ulong context, void* state)
{
   printf("PING\n");
   return TRUE;
}


bool pong(short keycode, ulong context, void* state)
{
   printf("PONG %x\n",state);
   return TRUE;
}


bool quit(short keycode, ulong context, void* state)
{
   printf("Hey, look!  I'm an Origin Game!\n");
   return *(bool*)state = TRUE;
}

void main()
{
   bool done = FALSE;
   hotkey_init(10);
   hotkey_add(' '|KB_FLAG_DOWN,PING|PONG,toggle_mode,NULL);
   hotkey_add_help('x'|KB_FLAG_DOWN|KB_FLAG_CTRL,PING|PONG,quit,&done,"HELP TEXT");
//   hotkey_add_help('x'|KB_FLAG_DOWN,PING|PONG,quit,&done,"HELP TEXT");
   hotkey_add(KB_FLAG_DOWN|'p',PING,ping,NULL);
   hotkey_add(KB_FLAG_DOWN|'p',PONG,pong,(void*)0xD00F);
   HotkeyContext = PING;
   hotkey_dispatch(KB_FLAG_DOWN|'p');
   kb_init(NULL);
   kb_set_flags(kb_get_flags()|KBF_BLOCK);
   while(!done)
   {
      ushort cooked;
      bool result;
/*
      cooked = getc(stdin) |  KB_FLAG_DOWN;
      hotkey_dispatch(cooked);
*/
      kbs_event goofy = kb_next();
      if (kb_cook(goofy, &cooked, &result) == OK && result)
      {
         hotkey_dispatch(cooked);
      }

   }
   hotkey_shutdown();
}


