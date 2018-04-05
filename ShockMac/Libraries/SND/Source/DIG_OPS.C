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
//===========================================================================
// $Source: r:/prj/lib/src/snd/RCS/dig_ops.c $
// $Revision: 1.1 $
// $Author: dc $
// $Date: 1994/12/01 02:06:05 $
//
// operations performable with the digital system
//===========================================================================

#define __DIG_OPS_C

#include <String.h>
#include "digi.h"

//--------------------
//  Internal Prototypes
//--------------------
int snd_find_free_handle(uchar smp_pri, bool check_only);


//--------------------------------------------------------------------------
//  Get the parameters for a sound channel.
//--------------------------------------------------------------------------
snd_digi_parms *snd_sample_parms(int hnd_id)
{
   return &_snd_smp_prm[hnd_id];
}

/*еее Callback
static void cdecl smp_EOS_callback(SAMPLE *S)
{
   int hnd_id;
   snd_digi_parms *our_p;
   void cdecl (*smp_ecall)(snd_digi_parms *dprm);

   hnd_id=AIL_sample_user_data(S,0);
   our_p=&_snd_smp_prm[hnd_id];
   if (our_p->flags&SND_FLG_INUSE)
   {
	   smp_ecall=(void cdecl (*)(snd_digi_parms *))AIL_sample_user_data(S,7);
	   AIL_set_sample_user_data(S,7,NULL);
	   if (smp_ecall!=NULL)
	      (*smp_ecall)(our_p);
   }
#ifdef SND_ANAL_DEBUG
   else
      snd_error=SND_NO_HANDLE;
#endif
   our_p->flags=0;
}

#ifdef USED
static void cdecl smp_EOB_callback(SAMPLE *S)
{
   int hnd_id;
   snd_digi_parms *our_p;

   if (snd_update!=NULL)
   {
	   hnd_id=AIL_sample_user_data(S,0);
	   our_p=&_snd_smp_prm[hnd_id];
      (*snd_update)(our_p);
   }
}
#endif
*/

//--------------------------------------------------------------------------
//  Stop playing sound on a sound channel.
//--------------------------------------------------------------------------
void snd_end_sample(int hnd_id)
{
	SndCommand	sc;
	
	sc.cmd = flushCmd;														// Flush any remaining commands.
	sc.param1 = 0;
	sc.param2 = 0;
	SndDoImmediate(_snd_smp_prm[hnd_id].sndChan, &sc);
	
	sc.cmd = quietCmd;														// Turn off the current command.
	sc.param1 = 0;
	sc.param2 = 0;
	SndDoImmediate(_snd_smp_prm[hnd_id].sndChan, &sc);

	if (snd_finish)															// Since we've flushed all commands, we
		(*snd_finish)(&_snd_smp_prm[hnd_id]);				// have to do callback ourselves.
}

//--------------------------------------------------------------------------
//  Shut off all sounds currently playing.
//--------------------------------------------------------------------------
void snd_kill_all_samples(void)
{
	int 			i;
	SCStatus	stat;

	for (i=0; i<_snd_smp_cnt; i++)
	{
		SndChannelStatus(_snd_smp_prm[i].sndChan, sizeof(SCStatus), &stat);
		if (stat.scChannelBusy || stat.scChannelPaused)
			snd_end_sample(i);
	}
}

//--------------------------------------------------------------------------
//  Call this to check for free sound handles, returns a handle id.
//--------------------------------------------------------------------------
int snd_find_free_handle(uchar smp_pri, bool check_only)
{
	int 			i;
	SCStatus	stat;

	// Return an available channel, if any.
	for (i=0; i<_snd_smp_cnt; i++)
	{
		SndChannelStatus(_snd_smp_prm[i].sndChan, sizeof(SCStatus), &stat);
		if (!stat.scChannelBusy && !stat.scChannelPaused)
			return i;
	}

	// If there are no available channels, return one with a lower priority.
	for (i=0; i<_snd_smp_cnt; i++)
	{
		int lp = _snd_smp_prm[i].pri;
		if (lp < smp_pri)
		{
			if (!check_only)
				snd_end_sample(i);
			return i;
		}
	}

	// No channels available.
	return SND_PERROR;
}


//--------------------------------------------------------------------------
//  Set the sound channel's pan and volume settings.
//--------------------------------------------------------------------------
void snd_sample_reload_parms(snd_digi_parms *sdp)
{
	SndChannelPtr	ourSC;
	SndCommand		sc;
	long					ls, rs;

	ourSC = sdp->sndChan;

	// Set the pan values.
	
	ls = rs = 0x100;
	if (sdp->pan < 0x40)
		rs = sdp->pan * 4;
	else if (sdp->pan > 0x40)
		ls = (0x7F - sdp->pan) * 4;
	
	// Set the volume.
	
	if (sdp->vol < 0x100)
	{
		ls = ls * sdp->vol / 0x100;
		rs = rs * sdp->vol / 0x100;
	}
	
	// Issue volume/pan command.
	
	sc.cmd = volumeCmd;
	sc.param1 = 0;
	sc.param2 = (rs << 16) + ls;
	SndDoImmediate(ourSC, &sc);
}


//--------------------------------------------------------------------------
/* sample play
 * snd_ref is what to call back to the game with when snd is done
 * len and smp are the actual digital data
 * dprm is null normally, or overrides to relevant elements
 *
 * note if digi_parms->flags says double buffer, it may not be all data
 * in fact, it will then probably be just the first buffer, or something?
 *
 * overall plan
 *  - get a free sample handle
 *    if none, try to kick out a low priority sample
 *       if this fails, go home with none
 *    if some, take it and go
 *  - AIL_init_sample to set up priorities
 *    fill in any custom fields
 *    fill in the address of the sample
 *  - if single buffer, set sample address and start_sample
 *  - if double buffer, AIL_load_sample_buffer to start the
 *    playback and add to asynch_update flag
 */
//--------------------------------------------------------------------------
int snd_sample_play(int snd_ref, int len, uchar *smp, snd_digi_parms *dprm)
{
	int 					hnd_id;
	int					lpri = SND_DEF_PRI;
	snd_digi_parms *hnd_parm;
	SndChannelPtr	ourSC;
	Handle				sndHdl;
	SndCommand		sc;

//   mprintf("SSP: %x at %x, l %x... ",snd_ref,smp,len);
	if (dprm != NULL)
		lpri = dprm->pri;
	
	if ((hnd_id = snd_find_free_handle(lpri, FALSE)) == SND_PERROR )
	{
		snd_error = SND_NO_HANDLE;
//      mprintf("no free handle error %d\n",snd_error);
		return SND_PERROR; 
	}
	
	hnd_parm = &_snd_smp_prm[hnd_id];
	ourSC = hnd_parm->sndChan;
	if (dprm != NULL)
		LG_memcpy(hnd_parm, dprm, sizeof(snd_digi_parms));
	else
	{
		LG_memset(hnd_parm, 0, sizeof(snd_digi_parms));
		hnd_parm->pan = SND_DEF_PAN;
		hnd_parm->vol = 0x100;
		hnd_parm->pri = lpri;
		hnd_parm->loops = 1;
	}
	hnd_parm->sndChan = ourSC;
	
	// For Mac version, return an error if there is no sample data.  Doesn't support
	// double-buffered sound.
	if (smp != NULL)
	{
		hnd_parm->len = len;
		sndHdl = RecoverHandle((Ptr)smp);
		hnd_parm->sample = sndHdl;
	}
	else
	{
		snd_error = SND_OUT_OF_MEMORY;
		return SND_PERROR; 
	}
	hnd_parm->snd_ref = snd_ref;

	// Set up pan and volume.
	
	snd_sample_reload_parms(hnd_parm);

	// Play the sound.
	
	HLock(sndHdl);
	SndPlay(ourSC, (SndListHandle)sndHdl, TRUE);
	
	// Call our call-back routine when the sound is done playing.
	
	sc.cmd = callBackCmd;
	sc.param1 = 1;							// Sound complete
	sc.param2 = (long)hnd_parm;		// Ptr to current sound parameters
	SndDoCommand(ourSC, &sc, FALSE);

	return hnd_id;
}
