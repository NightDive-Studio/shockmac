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
#ifndef __FRSHIPM_H
#define __FRSHIPM_H
/*
 * $Source: n:/project/cit/src/inc/RCS/frshipm.h $
 * $Revision: 1.1 $
 * $Author: dc $
 * $Date: 1994/01/02 17:16:41 $
 *
 * Citadel renderer
 *  ship vs. not debug spew stuff
 */

#include "frctxt.h"

// have to learn about fixing fr_ret and all to be null

#define _chkNull(ptr)  
#define _chkNullcast(pt,cast) 
#define _fr_top(vr)           if ((vr)==NULL) _fr=_sr; else _fr=(fauxrend_context *)(vr)
#define _fr_top_cast(vr,cast) _fr_top(vr)
#define _fr_ret               return FR_OK
#define _fr_ret_val(v)        return v
#define _fr_dbg(exp)     
#define _fr_dbgchk(flg,exp)       	
#define _fr_sdbg(flg,exp)     
#define _fr_ndbg(flg,exp)     exp
#define _fr_defdbg(flg)       0
#define static                static
#define _fr_dbgflg_add(flg)    
#define _fr_dbgflg_tog(flg)    
#define _fr_dbgflg_chk(flg)   (0)

#endif // __FRSHIPM_H
