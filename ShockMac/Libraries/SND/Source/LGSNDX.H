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
// $Source: r:/prj/lib/src/snd/RCS/lgsndx.h $
// $Revision: 1.1 $
// $Author: dc $
// $Date: 1994/12/01 02:06:32 $
//
// header for new simpleotron sound library
//===================================================================

#ifndef __LGSNDX_H
#define __LGSNDX_H

#include <QuickTimeComponents.h>
#include <Sound.h>
#include "lg.h"

//--------------------------
//	Types
//--------------------------
typedef struct
{
   SndChannelPtr	sndChan;	// Ptr to Mac sound channel.
   uchar			pan;
   uchar			pri;
   ushort			vol;
   uchar			flags;
   int			snd_ref;
   int			loops;
   Handle			sample;		// Handle to Mac 'snd ' resource.
   void			*data;
   int			len;
} snd_digi_parms;

typedef struct
{
   uchar	pan;
   uchar	vol;
   uchar	pri;
   uchar	flags;
   int	snd_ref;
   void	*data;
   int	seq_num;
   void	*internal_form;
} snd_midi_parms;

//typedef void LG_SND_DRIVER;

//--------------------------
//	Globals
//--------------------------
//extern LG_SND_DRIVER *snd_digi;
//extern LG_SND_DRIVER *snd_midi;
extern uchar	snd_digi_vol;
extern uchar          snd_midi_vol;
//extern void cdecl   (*snd_update)(snd_digi_parms *dprm);
extern void (*snd_finish)(snd_digi_parms *dprm);
//extern void cdecl   (*snd_nblock)(snd_digi_parms *dprm);
extern void (*seq_finish)(long seq_ind);
//extern void cdecl   (*seq_miditrig)(snd_midi_parms *mprm, int trig_value);
extern int		snd_error;
//extern int            snd_genmidi_or_not;
//extern uchar          snd_stereo_reverse;

//--------------------------
//	Defines
//--------------------------
//#define snd_sample_ptr_from_parms(ps)   ((SAMPLE *)ps->internal_form)
//#define snd_sequence_ptr_from_parms(ps) ((SEQUENCE *)ps->internal_form)
//#define snd_sample_ptr_from_id(pid)     ((SAMPLE *)snd_get_sample(pid))
#define snd_sequence_ptr_from_id(pid)   ((SEQUENCE *)snd_get_sequence(pid))

//--------------------------
//	Prototypes
//--------------------------
void  snd_startup(void);
void  snd_setup(void *d_path, char *prefix);
void  snd_shutdown(void);
int   snd_set_midi_sequences(int chan_cnt);
int   snd_start_digital(void);
int   snd_stop_digital(void);
int   snd_set_digital_channels(int chan_cnt);
int   snd_start_midi(void);
int   snd_stop_midi(void);

int   snd_sample_play(int snd_ref, int len, uchar *smp, snd_digi_parms *dprm);
void  snd_end_sample(int hnd_id);
snd_digi_parms *snd_sample_parms(int hnd_id);
//void *snd_get_sample(int hnd_id);
void  snd_kill_all_samples(void);
void  snd_sample_reload_parms(snd_digi_parms *sdp);

int snd_find_free_sequence(void);
//int   snd_sequence_play(int snd_ref, uchar *seq_dat, int seq_num, snd_midi_parms *mparm);
//snd_midi_parms *snd_sequence_parms(int hnd_id);
TunePlayer snd_get_sequence(int seq_id);
void  snd_end_sequence(int seq_id);
void  snd_kill_all_sequences(void);

//char *snd_load_raw(char *fname, int *ldat);
// Mac only routines.
OSErr snd_load_theme(FSSpec *specPtr, TunePlayer thePlayer);
void snd_release_current_theme(void);

//--------------------------
//	Parm + flag defines for digital
//--------------------------
#define SND_DEF_PRI		0x3f
#define SND_PARM_NULL	0xff
#define SND_DEF_PAN 		64

#define SND_FLG_DOUBLE_BUFFER	0x01
#define SND_FLG_INUSE          		0x08
#define SND_FLG_RAWDATA        		0x80
#define SND_FLG_RAWD_STEREO    	0x40
#define SND_FLG_SPEED          		0x30
#define SND_FLG_RAWMASK        	0xF0

//--------------------------
//	Size/scale defines
//--------------------------
#define SND_MAX_SAMPLES		8			// For Mac version.
#define SND_MAX_SEQUENCES 	8			//еее Can I really handle this many?

//--------------------------
//	Misc defines
//--------------------------
#define SND_HND_FIELD          0       // where in user data handle is stored

//--------------------------
//	Capabilities field
//--------------------------
#define SND_CAP_DIGI		1
#define SND_CAP_MIDI		2
#define SND_CAP_GENMIDI	4
#define SND_CAP_STEREO	8
#define SND_CAP_GAIN		16

//--------------------------
//	Return/Error codes
//--------------------------
#define SND_OK					0x0000

// error codes
#define SND_GENERIC_ERROR	0x0001
#define SND_OUT_OF_MEMORY	0x0002
#define SND_NO_DRIVER			0x0003
#define SND_NOT_SUPPORTED	0x0004

// specialized errors
#define SND_DRIVER_ALREADY	0x0100
#define SND_NO_DRIVER_NAME	0x0101
#define SND_CANT_INIT_DRIVER	0x0102
#define SND_CANT_FIND_CARD	0x0103

#define SND_NO_HANDLE 		0x0200
                                               
// to use with functions which want to return 0->n values
#define SND_PERROR			-1

#endif // __LGSNDX_H
