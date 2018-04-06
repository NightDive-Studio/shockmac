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
 * $Source: r:/prj/lib/src/2d/RCS/pertol.c $
 * $Revision: 1.9 $
 * $Author: kevin $
 * $Date: 1994/08/28 16:26:43 $
 * 
 * Function to set wall, floor, and linear mapping         
 * tolerances for the perspective mappers.
 * ltol and wftol should be between 0 and 7.  
 * Lower values allow more frequent use of the
 * specialized wall, floor and linear mappers.                         
 *
 * $Log: pertol.c $
 * Revision 1.9  1994/08/28  16:26:43  kevin
 * Lowered maximum tolerance to place less stress
 * on the perspective mappers.
 * 
 * Revision 1.8  1994/08/16  13:24:13  kevin
 * lowered tolerances slightly.
 * 
 * Revision 1.7  1994/07/28  16:56:30  kevin
 * Increased default tolerances.
 * 
 * Revision 1.6  1994/01/16  12:03:21  kevin
 * Added clut lighting global, general detail level stuff.
 * 
 * Revision 1.5  1994/01/05  04:34:23  kevin
 * Lowered default linear tolerance to take advantage of new linear mappers.
 * 
 * Revision 1.4  1993/12/15  02:42:14  kevin
 * Set initial tolerances to 4.
 * 
 * Revision 1.3  1993/12/14  22:38:47  kevin
 * Moved persective mapper context globals to fl8ps.c
 * 
 * Revision 1.2  1993/12/04  12:35:49  kevin
 * Added global context variable for persective mappers.
 * Don't ask me why I put it here.
 * 
 * Revision 1.1  1993/11/18  23:40:27  kevin
 * Initial revision
 * 
*/

#include "fix.h"
#include "grs.h"
#include "pertol.h"

ubyte flat8_per_ltol=5;   /* Linear tolerance     */
ubyte flat8_per_wftol=5;  /* Wall/Floor tolerance */

/* Clut Lighting tolerance: higher=more clut lighting. */
fix gr_clut_lit_tol=2*FIX_UNIT;

gr_per_detail_level gr_per_detail_list[GR_NUM_PER_DETAIL_LEVELS]=
{
   {0,0,4*FIX_UNIT},
   {3,4,2*FIX_UNIT},
   {5,5,0}
};

void gr_set_per_tol(ubyte ltol, ubyte wftol)
{
   flat8_per_ltol=(ltol&7);
   flat8_per_wftol=(wftol&7);
}

void gr_set_clut_lit_tol(fix cltol)
{
   gr_clut_lit_tol=cltol;
}

void gr_set_per_detail_level(int level)
{
   flat8_per_ltol=gr_per_detail_list[level].ltol;
   flat8_per_wftol=gr_per_detail_list[level].wftol;
   gr_clut_lit_tol=gr_per_detail_list[level].cltol;
}

void gr_set_per_detail_level_param(int ltol, int wftol, fix cltol, int level)
{
   gr_per_detail_list[level].ltol=ltol&7;
   gr_per_detail_list[level].wftol=wftol&7;
   gr_per_detail_list[level].cltol=cltol;
}

