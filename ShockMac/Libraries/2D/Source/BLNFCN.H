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
 * $Source: n:/project/lib/src/2d/RCS/blnfcn.h $
 * $Revision: 1.1 $
 * $Author: kevin $
 * $Date: 1994/03/14 17:49:25 $
 *
 * prototypes and for the blend system
 *
 * This file is part of the 2d libarary.
 *
 * $Log: blnfcn.h $
 * Revision 1.1  1994/03/14  17:49:25  kevin
 * Initial revision
 * 
 */

#ifndef __BLNFCN_H
#define __BLNFCN_H
/* prototypes for blend table maintenance, TRUE means success, FALSE not */
bool gr_free_blend(void);
/* tab_cnt is how many blend steps, note cnt<=0 is equivalent to calling
   free blend */
bool gr_init_blend(int log_blend_levels);
#endif /* !__BLNFCN */
