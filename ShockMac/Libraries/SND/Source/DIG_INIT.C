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
// $Source: r:/prj/lib/src/snd/RCS/dig_init.c $
// $Revision: 1.1 $
// $Author: dc $
// $Date: 1994/12/01 02:05:42 $
//
// initializers for the new digital system
//===================================================================

#include "digi.h"

//--------------------------
// Globals
//--------------------------
snd_digi_parms	_snd_smp_prm[SND_MAX_SAMPLES];
int					_snd_smp_cnt;
SndCallBackUPP  gDigiCallBackProcPtr;

//--------------------------
// Internal Prototypes
//--------------------------
SndChannelPtr CreateSndChannel(void);


//---------------------------------------------------------
// For Mac version:  Allocate all sound channels here.
//---------------------------------------------------------
int snd_start_digital(void)
{
	SndChannelPtr	scPtr;
	int					i;
	
	for (i = 0; i < SND_MAX_SAMPLES; i++)
	{
		scPtr = CreateSndChannel();
		if (scPtr)
			_snd_smp_prm[i].sndChan = scPtr;
		else
			break;
	};

	_snd_smp_cnt = i;
	return (SND_OK);
}

// Not needed for Mac version.
/*
int snd_set_digital_channels(int chan_cnt)
{
   _digi_top;
   if (chan_cnt>SND_MAX_SAMPLES)
      return SND_NOT_SUPPORTED;
   if (_snd_smp_cnt>chan_cnt)
   {  // have to lower allocation
      int i;
      for (i=_snd_smp_cnt-1; (i>=chan_cnt); i--)
         switch (AIL_sample_status(_snd_smp_hnd[i]))
         {
         default:
            snd_end_sample(i);
            AIL_release_sample_handle(_snd_smp_hnd[i]);
            break;
         case SMP_FREE:
#ifdef SND_ANAL_DEBUG
            Warning(("Hey, why do we have a handle for a FREE sample\n"));
#endif
            break;
         case SMP_DONE:
            AIL_release_sample_handle(_snd_smp_hnd[i]);
            break;
         }
   }
   else if (_snd_smp_cnt<chan_cnt)
      for (;_snd_smp_cnt<chan_cnt; _snd_smp_cnt++)
         if ((_snd_smp_hnd[_snd_smp_cnt]=AIL_allocate_sample_handle(_snd_digi))==NULL)
            return SND_OUT_OF_MEMORY;
         else
         {   
            AIL_init_sample(_snd_smp_hnd[_snd_smp_cnt]);
            AIL_set_sample_user_data(_snd_smp_hnd[_snd_smp_cnt],0,_snd_smp_cnt);
         }
   _digi_ret;
}
*/

int snd_stop_digital(void)
{
   snd_kill_all_samples();
//   AIL_uninstall_DIG_driver(snd_digi);
//   snd_digi=NULL;
   return SND_OK;
}

//---------------------------------------------------------
// CreateSndChannel -	Mac function to create a sound channel and return its ptr.
//---------------------------------------------------------
SndChannelPtr CreateSndChannel(void)
{
	SndChannelPtr	scPtr;
	OSErr				err;
	
	scPtr = SndChannelPtr(NewPtr(sizeof(SndChannel)));
	if (scPtr)
	{
		scPtr->qLength = 16;
		err = SndNewChannel(&scPtr, sampledSynth, initStereo, gDigiCallBackProcPtr);
		if (err != noErr)
		{
			DisposePtr(Ptr(scPtr));
			scPtr = NULL;
		}
		else
			scPtr->userInfo = 0;
	}
	
	return(scPtr);
}
