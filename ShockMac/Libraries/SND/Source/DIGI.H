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
 * $Source: r:/prj/lib/src/snd/RCS/digi.h $
 * $Revision: 1.1 $
 * $Author: dc $
 * $Date: 1994/12/01 02:06:25 $
 *
 * ok, new slimmed down super cool sound lib thingy
 */

#ifndef __DIGI_H
#define __DIGI_H

#include "lgsndx.h"

extern snd_digi_parms  _snd_smp_prm[];
extern int             _snd_smp_cnt;

extern SndCallBackUPP  gDigiCallBackProcPtr;

#endif // __DIGI_H
