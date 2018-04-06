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
 * $Source: r:/prj/lib/src/snd/RCS/mid_init.c $
 * $Revision: 1.1 $
 * $Author: dc $
 * $Date: 1994/12/01 02:06:15 $
 */

#define __MID_INIT_C

#include "midi.h"

//--------------------------
//  Globals
//--------------------------
//MDI_DRIVER     	*_snd_midi=NULL;
TunePlayer       		_snd_seq_player[SND_MAX_SEQUENCES];
int             				_snd_seq_cnt;
snd_midi_parms  	_snd_midi_prm[SND_MAX_SEQUENCES];

void (*seq_finish)(long seq_ind) = NULL;
TuneCallBackUPP	gTuneCBProcPtr;

//--------------------------
//  Prototypes
//--------------------------
pascal void HandleTuneCallBack(const TuneStatus *status, long refCon);


//---------------------------------------------------------
//  Nothing for Mac to do here (none of that nasty sound card mess, thank God).
//---------------------------------------------------------
int snd_start_midi(void)
{
/*
   lib_PARMS libp, *lbpp;
   char *driv_file;

   if (snd_midi!=NULL)
      return SND_DRIVER_ALREADY;
   midi_card->cap=SND_CAP_MIDI;
   if ((driv_file=load_lib_PARMS(midi_card,&libp))==NULL)
      return SND_NO_DRIVER_NAME;
   lbpp=parm_eval(midi_card,&libp);
   if ((snd_midi=(LG_SND_DRIVER *)AIL_install_MDI_driver_file(_snd_get_file(driv_file),lbpp))==NULL)
      return SND_CANT_INIT_DRIVER;
   if (!parm_checkup(lbpp))
    { snd_stop_midi(); return SND_CANT_FIND_CARD; }
*/
	gTuneCBProcPtr = NewTuneCallBackProc(HandleTuneCallBack);

	_snd_seq_cnt = 0;
	_midi_ret;
}

//---------------------------------------------------------
// For Mac version:  Allocate all sound channels here.
//---------------------------------------------------------
int snd_set_midi_sequences(int seq_cnt)
{
//   _midi_top;
	if (seq_cnt > SND_MAX_SEQUENCES)
		return SND_NOT_SUPPORTED;

/* Only called once in the game, so this doesn't apply.
	if (_snd_seq_cnt > seq_cnt)  							// See if we have to lower allocation
	{
		for (int i=0; (i<_snd_seq_cnt)&&(seq_cnt<_snd_seq_cnt); )
			switch (AIL_sequence_status(_snd_seq_hnd[i]))
			{
			case SMP_DONE:
				AIL_release_sequence_handle(_snd_seq_hnd[i]);
				if (i<--_snd_seq_cnt)
					_snd_seq_hnd[i]=_snd_seq_hnd[_snd_seq_cnt];
				break;
#ifdef SND_ANAL_DEBUG
			case SMP_FREE:
				Warning(("Hey, why do we have a handle for a FREE sample\n")); break;
#endif
			default:
				i++; break;
			}
	}
	else 
*/
	if (_snd_seq_cnt < seq_cnt)
	{
		for (;_snd_seq_cnt < seq_cnt; _snd_seq_cnt++)
		{
			if ((_snd_seq_player[_snd_seq_cnt] = OpenDefaultComponent(kTunePlayerType, 0))==NULL)
				return SND_OUT_OF_MEMORY;
//еее Should we do something here?
//			else
//				AIL_set_sequence_user_data(_snd_seq_hnd[_snd_seq_cnt],0,_snd_seq_cnt);
		}
	}
	_midi_ret;
}

//---------------------------------------------------------
//  Stop playing and shut down all the tune players.
//---------------------------------------------------------
int snd_stop_midi(void)
{
	snd_kill_all_sequences();
//	AIL_uninstall_MDI_driver(snd_midi);
//	snd_midi=NULL;
	for (int i=0; i<_snd_seq_cnt; i++)
	{
		CloseComponent(_snd_seq_player[i]);
	}
	
	DisposeRoutineDescriptor(gTuneCBProcPtr);
	gTuneCBProcPtr = NULL;
	
	return SND_OK;
}

//--------------------------------------------------------------------------
//	Call-back routine.  Get's called when tune is finished.
//--------------------------------------------------------------------------
pascal void HandleTuneCallBack(const TuneStatus *, long refCon)
{
	if (seq_finish)
		(*seq_finish)(refCon);
}
