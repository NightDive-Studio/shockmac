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
//===================================================================
// $Source: r:/prj/lib/src/snd/RCS/master.c $
// $Revision: 1.1 $
// $Author: dc $
// $Date: 1994/12/01 02:06:13 $
//===================================================================

#define __MASTER_C

//#include <stdio.h>

#include "digi.h"

uchar	snd_digi_vol=0x100;
//uchar          snd_midi_vol=MDI_DEFAULT_VOLUME;
//void cdecl   (*snd_update)(snd_digi_parms *dprm)=NULL;
void		(*snd_finish)(snd_digi_parms *dprm)=NULL;
//void cdecl   (*snd_nblock)(snd_digi_parms *dprm)=NULL;
//void cdecl   (*seq_miditrig)(snd_midi_parms *mprm, int trig_value)=NULL;
int		snd_error=SND_OK;
uchar	snd_stereo_reverse=0;

pascal void HandleSndCallBack(SndChannelPtr chan, SndCommand *cmd);


//---------------------------------------------------------
//	Install sound callback routine.
//---------------------------------------------------------
void snd_startup(void)
{
	gDigiCallBackProcPtr = NewSndCallBackProc(HandleSndCallBack);
}

//	Nothing for the Mac to do here.
/*
//---------------------------------------------------------
//	Set preferences for AIL, path for sound files.
//---------------------------------------------------------
void snd_setup(void *d_path, char *prefix)
{
   AIL_set_preference(DIG_USE_STEREO,YES);
//   AIL_set_preference(DIG_USE_16_BITS,YES);
   AIL_set_preference(DIG_HARDWARE_SAMPLE_RATE,22050);
//   AIL_set_preference(DIG_HARDWARE_SAMPLE_RATE,22254);

   if (prefix!=NULL)
      AIL_set_GTL_filename_prefix(prefix);
   snd_dpath=d_path;

   // Scan for hardware?

   // set real malloc
}
*/

//---------------------------------------------------------
//	Shutdown sound system.
//---------------------------------------------------------
void snd_shutdown(void)
{
	for (int i=0; i < _snd_smp_cnt; i++)
	{
		SndDisposeChannel(_snd_smp_prm[i].sndChan, TRUE);
		DisposePtr((Ptr)_snd_smp_prm[i].sndChan);
	}
	
	DisposeRoutineDescriptor(gDigiCallBackProcPtr);
	gDigiCallBackProcPtr = NULL;
}

//---------------------------------------------------------
//	Sound callback proc.
//---------------------------------------------------------
pascal void HandleSndCallBack(SndChannelPtr chan, SndCommand *cmd)
{
	snd_digi_parms *p = (snd_digi_parms *)cmd->param2;
	
//	printf("Call-back!   loops:%d, pan:%x, vol:%x, sample:%x\n",
//			p->loops, p->pan, p->vol, p->sample);

	// See if we need to repeat the sound.
		
	if (p->loops == -1 || p->loops > 1)
	{
		if (p->loops > 1) p->loops--;

		SndPlay(chan, (SndListHandle)p->sample, TRUE);		// Play the sound again.
		SndDoCommand(chan, cmd, FALSE);					// Call us back again.
	}
	else
	{
      		if (snd_finish)								// Do the regular callback.
      			(*snd_finish)(p);
	}
}
