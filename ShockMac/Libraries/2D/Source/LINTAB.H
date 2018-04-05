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
 *  $Source: r:/prj/lib/src/2d/RCS/lintab.h $
 *  $Revision: 1.4 $
 *  $Author: kevin $
 *  $Date: 1994/09/06 02:25:12 $
 */

#ifndef __LINTAB_H
#define __LINTAB_H

#include "line.h"
#include "plytyp.h"
#include "lintyp.h"

/* these tables are used in the 2d, but are not visible, except though the
    vector (see lintyp.h) 
*/

extern grt_uline_fill_table *grd_uline_fill_table;
extern grt_uline_fill_table *grd_uline_fill_table_list[];

extern grt_uline_fill gen_uline_fill_table[][GRD_LINE_TYPES];  
extern grt_uline_fill flat8_uline_fill_table[][GRD_LINE_TYPES];  
extern grt_uline_fill bank8_uline_fill_table[][GRD_LINE_TYPES];  
extern grt_uline_fill bank24_uline_fill_table[][GRD_LINE_TYPES];  
extern grt_uline_fill modex_uline_fill_table[][GRD_LINE_TYPES];  

#endif
