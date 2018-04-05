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
 * $Source: n:/project/lib/src/dstruct/RCS/hash.c $
 * $Revision: 1.5 $
 * $Author: mahk $
 * $Date: 1994/01/18 08:16:06 $
 *
 * $Log: hash.c $
 * Revision 1.5  1994/01/18  08:16:06  mahk
 * Added hash_copy
 * 
 * Revision 1.4  1993/08/09  17:14:59  mahk
 * Fixed the libdbg.h thing.
 * 
 * Revision 1.3  1993/07/01  12:16:22  mahk
 * changed log2 to hashlog2 to avoid name conflict
 * 
 * Revision 1.2  1993/06/14  21:11:45  xemu
 * step func
 * 
 * Revision 1.1  1993/03/25  19:15:09  mahk
 * Initial revision
 * 
 *
 */

#include <stdlib.h>
#include <string.h>
#include "hash.h" 

//--------------------
//  Defines
//--------------------
#define HASH_EMPTY    		0
#define HASH_TOMBSTONE 	1
#define HASH_FULL      		2

#define INDEX_NOT_FOUND -1

#define FULLNESS_THRESHHOLD_PERCENT 80

#define ELEM(tbl,i)  ((void*)((tbl)->vec + (i)*(tbl)->elemsize))

//--------------------
//  Prototypes
//--------------------
int hashlog2(int x);
int expmod(int b, int e, uint m);
bool is_fermat_prime(uint n, uint numtests);
static errtype grow(Hashtable* h, int newsize);

//--------------------
//  Internal Functions
//--------------------
int hashlog2(int x)
{
   if (x < 2) return 0;
   return 1+hashlog2(x/2);
}

int expmod(int b, int e, uint m)
{
   if (e == 0) return 1;
   if (e%2 == 0)
   {
      int tmp = expmod(b,e/2,m);
      return (tmp*tmp)%m;
   }
   else
   {
      int tmp = expmod(b,e-1,m);
      return (b*tmp)%m;
   }

}

bool is_fermat_prime(uint n, uint numtests)
{
   int i;
   if (n < 3) return FALSE;
   for (i = 0; i < numtests; i++)
   {
      int a = rand()%(n-2) + 2;
      if (expmod(a,n,n) != a) return FALSE;
   }
   return TRUE;
}

errtype hash_init(Hashtable* h, int elemsize, int vecsize, Hashfunc hfunc, Equfunc efunc)
{
   int i;
//   Spew(DSRC_DSTRUCT_Hash,("hash_init(%x,%d,%d,%x,%x)\n",h,elemsize,vecsize,hfunc,efunc));
   while(!is_fermat_prime(vecsize,2)) vecsize++;
   h->elemsize = elemsize;
   h->size = vecsize;
   h->sizelog2 = hashlog2(vecsize);
   h->fullness = 0;
   h->hfunc = hfunc;
   h->efunc = efunc;
   h->statvec = (char*)NewPtr(vecsize);
   if (h->statvec == NULL) return ERR_NOMEM;
   for (i = 0; i < vecsize; i++) h->statvec[i] = HASH_EMPTY;
   h->vec = (char*)NewPtr(elemsize*vecsize);
   if (h->vec == NULL) return ERR_NOMEM;
   return OK;
}

errtype hash_copy(Hashtable* t, Hashtable* s)
{
   *t = *s;
   t->statvec = NewPtr(t->size);
   if (t->statvec == NULL) return ERR_NOMEM;
   t->vec = NewPtr(t->elemsize*t->size);
   if (t->vec == NULL) return ERR_NOMEM;
   LG_memcpy(t->vec,s->vec,t->size*t->elemsize);
   LG_memcpy(t->statvec,s->statvec,t->size);
   return OK;
}

static bool find_elem(Hashtable* h, void* elem, int* idx)
{
   bool found = FALSE;
   int hash = h->hfunc(elem);
   int index,j;
   //Spew(DSRC_DSTRUCT_Hash,("find_elem(%x,%x,%x) hash is %d\n",h,elem,idx,hash));
   for (j = 0, index = hash%h->size;  j < h->size && h->statvec[index] != HASH_EMPTY;
         j++,index = (index + (1 << hash%h->sizelog2)) % h->size)
   {
      void* myelem = (void*) ELEM(h,index);
      if (h->statvec[index] == HASH_FULL && h->efunc(elem,myelem) == 0)
      {
         found = TRUE;
         break;
      }
   }
   *idx = index;
   //Spew(DSRC_DSTRUCT_Hash,("find_elem(): index is %d \n",index));
   return found;
}

static int find_index(Hashtable* h, void* elem)
{
   int hash = h->hfunc(elem);
   int j;
   int index;
//   Spew(DSRC_DSTRUCT_Hash,("find_index(%x,%x) hash is %d\n",h,elem,hash));
   for (j = 0, index = hash%h->size;  j < h->size && h->statvec[index] == HASH_FULL;
       j++,index = (index + (1 << hash%h->sizelog2)) % h->size)
//         Spew(DSRC_DSTRUCT_Hash,("find_index(): found status %d\n",h->statvec[index]));
   if (j >= h->size) index = INDEX_NOT_FOUND;
//   Spew(DSRC_DSTRUCT_Hash,("find_index(): result is %d\n",index));
   return index;
}

static errtype grow(Hashtable* h, int newsize)
{
   char* oldvec = h->vec;
   char* oldstat = h->statvec;
   char *newvec, *newstat;
   int oldsize = h->size;
   int i;
//   Spew(DSRC_DSTRUCT_Hash,("grow(%x,%d)\n",h,newsize));
   for (;!is_fermat_prime(newsize,2);newsize++);
   newvec = NewPtr(newsize*h->elemsize);
   if (newvec == NULL) return ERR_NOMEM;
   newstat = NewPtr(newsize);
   if (newstat == NULL)
   {
      DisposePtr (newvec);
      return ERR_NOMEM;
   }
   h->vec = newvec;
   h->statvec = newstat;
   h->size = newsize;
   h->sizelog2 = hashlog2(newsize);
   h->fullness = 0;
   for (i = 0; i < newsize; i++) newstat[i] = HASH_EMPTY;
   for (i = 0; i < oldsize; i++)
   {
      if (oldstat[i] == HASH_FULL)
      {
         hash_insert(h,(void*)(oldvec+i*h->elemsize));
      }
   }
   DisposePtr(oldvec);
   DisposePtr(oldstat);
   return OK;
}

errtype hash_set(Hashtable* h, void* elem)
{
   int i;
//   Spew(DSRC_DSTRUCT_Hash,("hash_set(%x,%x)\n",h,elem));
   if (h->fullness*100/h->size > FULLNESS_THRESHHOLD_PERCENT)
      grow(h,h->size*2);
   if (!find_elem(h,elem,&i))
      i = find_index(h,elem);
   LG_memcpy(ELEM(h,i),elem,h->elemsize);
   h->statvec[i] = HASH_FULL;
   h->fullness++;
   return OK;
}

errtype hash_insert(Hashtable* h, void* elem)
{
   int i;
//   Spew(DSRC_DSTRUCT_Hash,("hash_insert(%x,%x)\n",h,elem));
   if (h->fullness*100/h->size > FULLNESS_THRESHHOLD_PERCENT)
      grow(h,h->size*2);
   i = find_index(h,elem);
   LG_memcpy(ELEM(h,i),elem,h->elemsize);
   h->statvec[i] = HASH_FULL;
   h->fullness++;
   return OK;
}


errtype hash_delete(Hashtable* h, void* elem)
{
   int i;
//   Spew(DSRC_DSTRUCT_Hash,("hash_delete(%x,%x)\n",h,elem));
   if (find_elem(h,elem,&i))
   {
      h->statvec[i] = HASH_TOMBSTONE;
      return OK;
   }
   return ERR_NOEFFECT;
}


errtype hash_lookup(Hashtable* h, void* elem, void** result)
{
   int i;
//   Spew(DSRC_DSTRUCT_Hash,("hash_lookup(%x,%x,%x)\n",h,elem,result));
   if (find_elem(h,elem,&i))
   {
      *result = ELEM(h,i);
   }
   else *result = NULL;
//   Spew(DSRC_DSTRUCT_Hash,("hash_lookup(): value is %x\n",*result));
   return OK;
}

errtype hash_iter(Hashtable* h, HashIterFunc ifunc, void* data)
{
   int i;
   for (i = 0; i < h->size; i++)
      if (h->statvec[i] == HASH_FULL)
         if (ifunc(ELEM(h,i),data))
            break;
   return OK;
}

errtype hash_step(Hashtable *h, void **result, int *index)
{
   while ((h->statvec[*index] != HASH_FULL) && (*index < h->size))
      (*index)++;
   if (*index == h->size)
      *result = NULL;
   else
      *result = ELEM(h,*index);
   (*index)++;
   return(OK);
}

errtype hash_destroy(Hashtable* h)
{
   h->size = 0;
   h->fullness = 0;
   DisposePtr(h->statvec);
   DisposePtr(h->vec);
   return OK;
}

