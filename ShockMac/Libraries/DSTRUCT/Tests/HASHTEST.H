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
#ifndef _HASHTEST_H
#define _HASHTEST_H

/*
 * $Source: n:/project/lib/src/dstruct/RCS/hashtest.h $
 * $Revision: 1.1 $
 * $Author: mahk $
 * $Date: 1993/03/25 19:22:39 $
 *
 * $Log: hashtest.h $
 * Revision 1.1  1993/03/25  19:22:39  mahk
 * Initial revision
 * 
 *
 */



#define HASHELEMSIZE 8

typedef struct _hashelem
{
   char key[HASHELEMSIZE];
   int val;
} HashElem;





#endif // _HASHTEST_H
