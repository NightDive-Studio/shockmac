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
 * $Source: r:/prj/lib/src/snd/RCS/mid_ops.c $
 * $Revision: 1.1 $
 * $Author: dc $
 * $Date: 1994/12/01 02:06:16 $
 */

#include <string.h>

#define __MID_OPS_C
#include "midi.h"


//---------------------------------------------------------
//  Return a reference to a Tune Player.
//---------------------------------------------------------
TunePlayer snd_get_sequence(int seq_id)
{
   return (_snd_seq_player[seq_id]);
}

/*еее
snd_midi_parms *snd_sequence_parms(int hnd_id)
{
   return &_snd_midi_prm[hnd_id];
}

#ifdef TMP_MONO_DEBUG
uchar *mono_base_b=0xb0000;
#define mono_ch(x,ch) mono_base_b[x * 2] = ch
#else
#define mono_ch(x,ch)
#endif

#ifdef DEBUG_KILL
char pickch(int val)
{
   if (val<10) return '0'+val;
   else if (val<16) return 'A'+val-10;
   else return 'X';
}
#endif

void cdecl seq_EOS_callback(SEQUENCE *S)
{
   int seq_id;
   snd_midi_parms *seq_parm;
   void cdecl (*seq_ecall)(snd_midi_parms *mprm);

   mono_ch(150,'E'); mono_ch(151,' ');
   seq_ecall=(void cdecl (*)(snd_midi_parms *))AIL_sequence_user_data(S,SND_USERDATA_FINCALL);
   if (seq_ecall!=NULL)
   {
      mono_ch(150,'X'); mono_ch(151,'X');
	   seq_id=AIL_sequence_user_data(S,0);
	   seq_parm=&_snd_midi_prm[seq_id];
      (*seq_ecall)(seq_parm);
      AIL_set_sequence_user_data(S,SND_USERDATA_FINCALL,NULL);
   }
   mono_ch(151,'e');
}

void cdecl seq_mtrig_callback(SEQUENCE *S, int channel, int value)
{
   int seq_id;
   snd_midi_parms *seq_parm;

   if (seq_miditrig!=NULL)
   {
      seq_id=AIL_sequence_user_data(S,0);
	   seq_parm=&_snd_midi_prm[seq_id];
      (*seq_miditrig)(seq_parm, value);
   }
}
*/

//---------------------------------------------------------
//  Stop playing on a single Tune Player.
//---------------------------------------------------------
void snd_end_sequence(int seq_id)
{
//еее later	seq_EOS_callback(_snd_seq_hnd[seq_id]);
	TuneStop(_snd_seq_player[seq_id], kStopTuneFade);
	TuneFlush(_snd_seq_player[seq_id]);
}

//---------------------------------------------------------
//  Stop playing on all Tune Players.
//---------------------------------------------------------
void snd_kill_all_sequences(void)
{
	int i;
	
	for (i=0; i<_snd_seq_cnt; i++)
	{
		TuneStatus	tpStatus;
		TuneGetStatus(_snd_seq_player[i], &tpStatus);		// Get the tune player status
		if (tpStatus.queueTime != 0)								// If it's still playing, then
			snd_end_sequence(i);										// stop it.
	}
}

//---------------------------------------------------------
//  Return a Tune Player that's not busy.
//---------------------------------------------------------
int snd_find_free_sequence(void)
{
	for (int i=0; i < _snd_seq_cnt; i++)
	{
		TuneStatus	tpStatus;
		TuneGetStatus(_snd_seq_player[i], &tpStatus);		// Get the tune player status
		if (tpStatus.queueTime == 0)								// If nothing's playing, then
			return i;														// this is the one we want.
	}
	return SND_PERROR;
}

/*
int snd_sequence_play(int snd_ref, uchar *seq_dat, int seq_num, snd_midi_parms *mparm)
{
   SEQUENCE *S;
   snd_midi_parms *seq_parm;
   int lpri=SND_DEF_PRI, seq_id, rv;
   if (mparm!=NULL) lpri=mparm->pri;
   if ((seq_id=snd_find_free_sequence(lpri,FALSE))==SND_PERROR)
    { snd_error=SND_NO_HANDLE; return SND_PERROR; }
   seq_parm=&_snd_midi_prm[seq_id];
   if (mparm!=NULL)
      memcpy(seq_parm,mparm,sizeof(snd_midi_parms));
   else
   {
      memset(seq_parm,0,sizeof(snd_midi_parms));
      seq_parm->pan=SND_DEF_PAN;
      seq_parm->vol=127;
      seq_parm->pri=lpri;
   }
   seq_parm->seq_num=seq_num; seq_parm->snd_ref=snd_ref; seq_parm->data=seq_dat;
   S=_snd_seq_hnd[seq_id];
   rv=AIL_init_sequence(S,seq_dat,seq_num);
   if ((rv==0)||(rv==-1))
      return SND_PERROR;
   // should set volumes and everything and such...
   AIL_start_sequence(S);
   if (seq_finish)
   {
      AIL_set_sequence_user_data(S,SND_USERDATA_FINCALL,(LONG)seq_finish);
	   AIL_register_sequence_callback(S,seq_EOS_callback);
   }
   else
      AIL_set_sequence_user_data(S,SND_USERDATA_FINCALL,(LONG)NULL);
   if (seq_miditrig)
	   AIL_register_trigger_callback(S,seq_mtrig_callback);
   return seq_id;
}
*/