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
 * $Source: r:/prj/lib/src/lg/rcs/tmpalloc.c $
 * $Revision: 1.4 $
 * $Author: kaboom $
 * $Date: 1994/08/03 23:09:14 $
 *
 * Routines for controlling temporary memory buffer.
 *
 * This file is part of the 2d library.
 */

#include "dbg.h"
#include "memall.h"
#include "tmpalloc.h"
//#include <_lg.h>

/* arbitrary size for buffer.  used if a buffer isn't explicitly set. */
#define TEMP_BUF_SIZE 16384

/* memstack to use for temporary memory requests. */
static MemStack *temp_mem_stack=NULL;

/* TRUE if buffer is allocated by temp_mem_init. */
static bool stack_dynamic=FALSE;

MemStack *temp_mem_get_stack(void)
{
   return temp_mem_stack;
}

/* sets the memstack to be used by the temporary memory routines to ms.
   if ms is NULL, it attempts to allocate a dynamic buffer of size given
   by TEMP_BUF_SIZE.  returns 0 if all is well, nonzero if there is an
   error. */
int temp_mem_init(MemStack *ms)
{
   if (ms==NULL) {
      /* allocate memstack struct and buffer dynamically. */
//      Spew(DSRC_LG_Tempmem,
//           ("TempMemInit: dynamically allocating stack of %d bytes\n",
//            TEMP_BUF_SIZE));
//      if ((ms=(MemStack *)Malloc(sizeof(MemStack)+TEMP_BUF_SIZE))==NULL) {
      if ((ms=(MemStack *)NewPtr(sizeof(MemStack)+TEMP_BUF_SIZE))==NULL) {
         Warning(("TempMemInit: can't allocate dynamic buffer.\n"));
         return -1;
      }
      stack_dynamic=TRUE;
      ms->baseptr=(void *)(ms+1);
      ms->sz=TEMP_BUF_SIZE;
      MemStackInit(ms);
      temp_mem_stack=ms;      /* save pointer to temp memstack */
      return 0;
   } else {
      /* use passed in memstack. */
      temp_mem_stack=ms;
      return 0;
   }
}

/* sets the memstack used by the temporary memory routines to NULL.
   if the buffer was allocated dynamically, it's freed. */
int temp_mem_uninit(void)
{
   if (stack_dynamic==TRUE) {
//      Spew(DSRC_LG_Tempmem,
//           ("TempMemUninit: freeing dynamically allocated stack\n"));
//      free(temp_mem_stack);
      DisposePtr((Ptr)temp_mem_stack);
      stack_dynamic=FALSE;
   }
   temp_mem_stack=NULL;
   return 0;
}

/* allocate a temporary buffer of size n from temp_mem_stack. */
void *temp_malloc(long n)
{
   if (temp_mem_stack==NULL)
      if (temp_mem_init(NULL)!=0)
         return NULL;
   return MemStackAlloc(temp_mem_stack,n);
}

/* resize temporary buffer pointed to by p to be new size n. */
void *temp_realloc(void *p,long n)
{
   return MemStackRealloc(temp_mem_stack,p,n);
}

/* free temporary buffer pointed to by p. */
int temp_free(void *p)
{
   return MemStackFree(temp_mem_stack,p)==FALSE;
}

#ifdef DBG_ON
/* the spewing versions of the temporary memory routines print out
   additional information about the call to the real routine, including
   the file name and line number where the call was made. */
int temp_spew_mem_init(MemStack *ms,char *file,int line)
{
   int r;
   r=temp_mem_init(ms);
   Spew(DSRC_LG_Tempmem,
        ("TempMemInit: stack: %p rval: %d (file: %s line: %d)\n",
         temp_mem_stack,r,file,line));
   return r;
}

int temp_spew_mem_uninit(char *file,int line)
{
   int r;

   r=temp_mem_uninit();
   Spew(DSRC_LG_Tempmem,
        ("TempMemUninit rval: %d (file: %s line: %s)\n",
         r,file,line));
   return r;
}

void *temp_spew_malloc(long size,char *file,int line)
{
   void *p;
   p=temp_malloc(size);
   Spew(DSRC_LG_Tempmem,
        ("TempMalloc:  p: 0x%x  size: %d  (file: %s line: %d)\n",
         p,size,file,line));
   return p;
}

void *temp_spew_realloc(void *ptr,long size,char *file,int line)
{
   void *p;
   p=temp_realloc(ptr,size);
   Spew(DSRC_LG_Tempmem,
        ("TempRealloc: p: 0x%x  pold: 0x%x  size: %d  (file: %s line: %d)\n",
         p,ptr,size,file,line));
   return p;
}

int temp_spew_free(void *ptr,char *file,int line)
{
   int r;
   r=temp_free(ptr);
	Spew(DSRC_LG_Tempmem,
        ("TempFree:    p: 0x%x  (file: %s line: %d)\n",
         ptr,file,line));
   return r;
}
#endif /* DBG_ON */
