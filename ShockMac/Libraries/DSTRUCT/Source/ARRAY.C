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
#include <string.h>
#include "lg.h"
#include "array.h" 

//---------
// Prototypes
//---------
errtype array_grow(Array *a, int size);


//------------------------------------------------------
//  For Mac version:  Use NewPtr and DisposePtr.

#define FREELIST_EMPTY   -1
#define FREELIST_NOTFREE -2

errtype array_init(Array* initme, int elemsize, int vecsize)
{
   if (elemsize == 0) return ERR_RANGE;
   initme->elemsize = elemsize;
   initme->vecsize = vecsize;
   initme->fullness = 0;
   initme->freehead = FREELIST_EMPTY;
   initme->freevec = (int*)NewPtr(vecsize*sizeof(int));
   if (initme->freevec == NULL) return ERR_NOMEM; 
   initme->vec = (char*) NewPtr(elemsize*vecsize);
   if (initme->vec == NULL) return ERR_NOMEM;
   return OK;
}

errtype array_grow(Array *a, int size)
{
   char* tmpvec;
   int* tmplist; 
   if (size <= a->vecsize) return OK;
   tmpvec = (char *)NewPtr(a->elemsize*size);
   if (tmpvec == NULL) return ERR_NOMEM;
   LG_memcpy(tmpvec,a->vec,a->vecsize*a->elemsize);
   tmplist = (int *)NewPtr(size*sizeof(int));
   if (tmplist == NULL) return ERR_NOMEM;
   LG_memcpy(tmplist,a->vec,a->vecsize*sizeof(int));
   DisposePtr((Ptr)a->vec);
   DisposePtr((Ptr)a->freevec);
   a->vecsize = size;
   a->vec = tmpvec;
   a->freevec = tmplist;
   return OK;
}

errtype array_newelem(Array* a, int* index)
{
   if (a->freehead != FREELIST_EMPTY)
   {
      *index = a->freehead;
      a->freehead = a->freevec[*index];
      a->freevec[*index] = FREELIST_NOTFREE;
      return OK;
   }
   if (a->fullness >= a->vecsize)
   {
      errtype err = array_grow(a,a->vecsize*2);
      if (err != OK) return err;
   }
   *index = a->fullness++;
   a->freevec[*index] = FREELIST_NOTFREE;
   return OK;
}


errtype array_dropelem(Array* a, int index)
{
   if (index >= a->fullness || a->freevec[index] != FREELIST_NOTFREE) return OK; // already freed. 
   a->freevec[index] = a->freehead;
   a->freehead = index;
   return OK;
}

errtype array_destroy(Array* a)
{
   a->elemsize = 0;
   a->vecsize = 0;
   a->freehead = FREELIST_EMPTY;
   DisposePtr((Ptr)a->freevec);
   DisposePtr((Ptr)a->vec);
   return OK;
}
