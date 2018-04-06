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
#ifndef __FAKETIME_H
#define __FAKETIME_H 

/* total bogosity that saves having to alter 50 source files */
/* the non-time-faking parts of faketime + the new improved timer.h */

#define CIT_CYCLE 280
#define CIT_FREQ  (TMD_FREQ/CIT_CYCLE)

#define APPROX_CIT_CYCLE_HZ   256
#define APPROX_CIT_CYCLE_SHFT 8

extern volatile long *tmd_ticks;

// KLC void cit_increment_ticks ();

// KLC  #include <timer.h>

#endif /* !__FAKETIME_H */
