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
 * $Source: r:/prj/lib/src/snd/RCS/midi.h $
 * $Revision: 1.1 $
 * $Author: dc $
 * $Date: 1994/12/01 02:06:33 $
 */

#ifndef __MIDI_H
#define __MIDI_H

#include "lgsndx.h"

//#define _midi_top if (snd_midi==NULL) return SND_NO_DRIVER; else _snd_midi=(MDI_DRIVER *)snd_midi
#define _midi_ret return SND_OK

//extern MDI_DRIVER     *_snd_midi;
extern TunePlayer     	 	_snd_seq_player[SND_MAX_SEQUENCES];
extern int            		 	_snd_seq_cnt;
extern snd_midi_parms 	_snd_midi_prm[];

extern TuneCallBackUPP	gTuneCBProcPtr;

extern Handle					gHeaderHdl;
extern Handle					gTuneHdl;
extern Handle					gOfsHdl;
extern long						*gOffsets;

#endif // __MIDI_H
