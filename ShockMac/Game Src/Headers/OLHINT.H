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
#ifndef __OLHINT_H
#define __OLHINT_H

/*
 * $Source: r:/prj/cit/src/inc/RCS/olhint.h $
 * $Revision: 1.3 $
 * $Author: mahk $
 * $Date: 1994/08/22 01:14:44 $
 *
 */

#include "objects.h"

#define SCAN_RATIO 3

#define SCAN_WID (SCREEN_VIEW_WIDTH/SCAN_RATIO)
#define SCAN_HGT (SCREEN_VIEW_HEIGHT/SCAN_RATIO)

#define OLH_WRAP_WID 100


typedef struct _olh_data
{
   ObjID obj;
   LGPoint loc;
} olh_data;

extern olh_data olh_object;

extern bool olh_candidate(ObjID obj);



#endif // __OLHINT_H

