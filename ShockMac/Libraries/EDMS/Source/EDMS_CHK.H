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

// defines for master debug things

//#define SOLITON_FRAME_CNT

//#define SOLITON_DO_SL
//#define ROBOT_DO_SL
//#define CHECK_IDOF
#define CHECKING_SOLITON

// note this requires FRAME_CNT
//#define SOLITON_ALLOC_MONITOR

// actual macros and vars they set up and use

#ifdef SOLITON_FRAME_CNT
#ifndef __SOLITON_SRC
//#ifdef __cplusplus
//extern "C" {
//extern int EDMS_pfrm;    // for debugging stupidity
//}
//#else
extern int EDMS_pfrm;    // for debugging stupidity
//#endif
#else
//extern "C" {
int EDMS_pfrm=0;
//}
#endif
#endif

#ifdef ROBOT_DO_SL
#define rob_sl_at(l,x) (*((uchar *)(0xB0000+(l*2)))=x)
#define rob_sl(x)      (*((uchar *)(0xB0000+158))=x)
#else
#define rob_sl_at(l,x) 
#define rob_sl(x) 
#endif

#ifdef SOLITON_DO_SL
#define sol_sl_at(l,x) (*((uchar *)(0xB0000+(l*2)))=x)
#define sol_sl(x)      (*((uchar *)(0xB0000+158))=x)
#else
#define sol_sl_at(l,x) 
#define sol_sl(x) 
#endif


