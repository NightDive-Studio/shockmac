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
#include "tngplain.h"
//#include <_ui.h>

#pragma require_prototypes off
bool tng_plain_keycooked(TNG *ptng, ushort key)
{
   bool retval = FALSE;
   IF_SET_RV(tng_cb_keycooked(ptng, key));
   return(retval);
}

bool tng_plain_mousebutt(TNG *ptng, uchar type, LGPoint loc)
{
   tng_cb_mousebutt(ptng,type,loc);
   return(TRUE);
}

bool tng_plain_signal(TNG *ptng, ushort signal)
{
   tng_cb_signal(ptng,signal);
   return(TRUE);
}
#pragma require_prototypes on

// Initializes the TNG
errtype tng_plain_init(void *ui_data, TNG *ptng, LGPoint size)
{
   TNG_plain *ppbtng;

   ppbtng = (TNG_plain *)GUI_MALLOC(ptng->ui_data, sizeof(TNG_plain));

   TNGInit(ptng,NULL,ui_data);
   ptng->type_data = ppbtng;
   ptng->keycooked = &tng_plain_keycooked;
   ptng->mousebutt = &tng_plain_mousebutt;
   ptng->signal = &tng_plain_signal;

   ppbtng->tng_data = ptng;
   ppbtng->size = size;
   return(OK);
}

// Deallocate all memory used by the TNG 
errtype tng_plain_destroy(TNG *ptng)
{
   GUI_DEALLOC(ptng->ui_data, ptng->type_data);
   return(OK);
}

// Fill in ppt with the size...
errtype tng_plain_size(TNG *ptng, LGPoint *ppt)
{
   *ppt = TNG_PL(ptng)->size;
   return(OK);
}

// Returns the current "value" of the TNG
int tng_plain_getvalue(TNG *ptng)
{
   void *dummy;   dummy = ptng;
   return(0);
}


