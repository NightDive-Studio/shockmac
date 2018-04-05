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
** stacktst.c
**
** Test of stack.c
**
** $Header: n:/project/lib/src/lg/rcs/stacktst.c 1.3 1993/12/20 13:40:28 ept Exp $
** $Log: stacktst.c $
 * Revision 1.3  1993/12/20  13:40:28  ept
 * Added test of Realloc
 * 
 * Revision 1.2  1993/09/13  12:40:43  dfan
 * ptr and size were reserved words in assembler
 * 
 * Revision 1.1  1993/09/13  11:11:03  dfan
 * Initial revision
 * 
*/

#include "memall.h"
#include <stdio.h>

MemStack ms;

void main ()
{
   long size = 65536;
   void *p1, *p2, *p3, *p4, *p5;
   long s1, s2, s3, s4, s5;

   ms.topptr = (void *)NewPtr(size);
   if (ms.topptr == NULL) printf ("malloc failed\n");
   ms.sz = size;
   MemStackInit (&ms);
   
   printf ("%p (%ld)\n", ms.topptr, ms.sz);

   s1 = 32768;
   p1 = MemStackAlloc (&ms, s1);
   printf ("%p:%ld\n", p1, s1);

   s2 = 32768;
   p2 = MemStackAlloc (&ms, s2);
   printf ("%p:%ld\n", p2, s2);

   MemStackFree (&ms, p2);
   printf ("Freed p2\n");

   s3 = 20000;
   p3 = MemStackAlloc (&ms, s3);
   if (p3 == NULL) printf ("malloc failed\n");
   printf ("%p:%ld\n", p3, s3);

   s4 = 10000;
   p4 = MemStackRealloc (&ms, p3, s4);
   if (p4 == NULL) printf ("realloc failed\n");
   printf ("%p:%ld\n", p4, s4);

   s5 = 50000;
   p5 = MemStackRealloc (&ms, p4, s5);
   if (p5 == NULL) printf ("realloc failed\n");
   printf ("%p:%ld\n", p5, s5);
}
