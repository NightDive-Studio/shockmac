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
 * $Source: n:/project/lib/src/input/test/RCS/testcook.c $
 * $Revision: 1.3 $
 * $Author: kaboom $
 * $Date: 1993/12/01 06:50:50 $
 */

#include <stdio.h>
#include <stdlib.h>

#include "lg.h"
#include "kb.h"
#include "kbcook.h"

void main()
{
   ushort cooked;
   bool result;
   kb_init(NULL);
   kb_set_flags(KBF_BLOCK);
//   kb_set_state(0x01, KBA_REPEAT);
//   kb_set_state(0x17, KBA_REPEAT);
	
	printf("This program reports key-code information.\n");
	printf("For the Mac, the 'cmd' key maps to the PC's 'ctrl' key\n");
	printf("and the 'option' key maps to the PC's 'alt' key.\n");
	printf("Start typing (press 'option-x' to quit)...\n\n");
	
   while (cooked != ('x' | KB_FLAG_DOWN | KB_FLAG_ALT))
   {
      kbs_event ev = kb_next();
      printf("Got raw event <%d,0x%x>\n",ev.state,ev.code);
      kb_cook(ev,&cooked,&result);
      if (result)
      {
         if (cooked & KB_FLAG_DOWN) printf ("DOWN ");
         else printf ("UP ");
         if (cooked & KB_FLAG_CTRL) printf("CTRL ");
         if (cooked & KB_FLAG_ALT)  printf("ALT ");
         if (cooked & KB_FLAG_SHIFT) printf("SHIFT ");
         if (cooked & KB_FLAG_2ND) printf("2ND ");

         if (!(cooked & KB_FLAG_SPECIAL))
            printf("ASCII %c\n",cooked&0xFF);
         else printf("SPECIAL %x\n",cooked&0xFF);
      }
   }
   kb_close();
   
	printf("Done.\n");
}
