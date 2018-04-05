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
#ifndef __PALFX_H
#define __PALFX_H
/*
 * $Source: r:/prj/cit/src/inc/RCS/palfx.h $
 * $Revision: 1.5 $
 * $Author: xemu $
 * $Date: 1994/07/19 23:25:37 $
 */

extern void palfx_fade_up(bool do_now);
extern void palfx_fade_down();
extern void palfx_init();

extern byte cyc_id0, cyc_id1, cyc_id2, cyc_id3, cyc_id4, cyc_id5;
#endif // __PALFX_H

