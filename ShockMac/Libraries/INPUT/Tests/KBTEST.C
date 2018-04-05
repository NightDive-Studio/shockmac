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
 * $Source: r:/prj/lib/src/input/test/RCS/kbtest.c $
 * $Revision: 1.5 $
 * $Author: kaboom $
 * $Date: 1994/08/15 18:11:40 $
 */

#include <stdio.h>
#include <stdlib.h>

#include "lg.h"
#include "kb.h"

void show_keyarray(void)
{
/*
   int i,j;

   for (i=0; i<8; i++) {
      for (j=0; j<32; j++)
         if (kb_state(i*32+j)==KBS_DOWN)
            mput('*', j*2, i*2);
         else
            mput(' ', j*2, i*2);
   }
*/
	printf("   %d\n", kb_state(0x31));
}

main()
{
   int n=0;
   kbs_event event;

   printf("Start typing...\n");
   kb_startup(NULL);
//   kb_set_state(0x01,KBA_REPEAT);
//   kb_set_state(0x17,KBA_REPEAT);
//   kb_set_state(0x54,KBA_SIGNAL);
   show_keyarray();
   while (n < 10)
   {
      event = kb_next();
      if (event.code!=0xff)
      {
         printf("key %x %s",event.code,(event.state==KBS_UP)?"up":"down");
         show_keyarray();
         n++;
      }
      else
      {
//         event.state = 0;
//         event.code = 1;
//         kb_generate(event);
      }
   }

   kb_shutdown();
}
