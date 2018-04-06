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
#ifndef __SCHEDTYPE_H
#define __SCHEDTYPE_H

/*
 * $Source: q:/inc/RCS/schedtyp.h $
 * $Revision: 1.2 $
 * $Author: xemu $
 * $Date: 1993/09/02 23:08:32 $
 *
 * $Log: schedtyp.h $
 * Revision 1.2  1993/09/02  23:08:32  xemu
 * angle me baby
 * 
 * Revision 1.1  1993/08/18  11:52:55  mahk
 * Initial revision
 * 
 *
 */

// Includes

// This is the only thing defined here. To minimize dependency.
typedef struct _schedule
{
   PQueue queue;
}  Schedule;

#endif // __SCHEDTYPE_H

