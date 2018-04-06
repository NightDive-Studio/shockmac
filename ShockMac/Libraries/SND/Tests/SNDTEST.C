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

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "lg.h"
#include "kb.h"
#include "lgsndx.h"

bool	gShowCallback = FALSE;
bool	gCallbackFlag = FALSE;
Handle	sndHdl[9];

void parm_move(uchar *parm, int dir);
Handle LoadSound(short resID);
void test_snd_callback(snd_digi_parms *p);
void DoMouseSndTest(void);


void test_snd_callback(snd_digi_parms *p)
{
	if (gShowCallback)
		gCallbackFlag = TRUE;
//		printf("Call-back!   loops:%d, pan:%x, vol:%x, sample:%x\n",
//				p->loops, p->pan, p->vol, p->sample);
}

void parm_move(uchar *parm, int dir)
{
   if (dir>0)
   {
      if (*parm<80) *parm+=10;
      else *parm+=5;
      if (*parm>127) *parm=127;
   }
   else
   {
      if (*parm>40) *parm-=10;
      else if (*parm>5) *parm-=5;
      else *parm=0;
   }
}

Handle LoadSound(short resID)
{
	Handle	sndHdl = GetResource('snd ', resID);
	if (sndHdl)
		DetachResource(sndHdl);
	return (sndHdl);
}

void main(int argc, char *argv[])
{
   bool 	inloop = TRUE;
   int 		dres;
//   int		mres;
//   int		msc[8][2];
   snd_digi_parms spc, *cur_parm=NULL;
   char 	spos = 0;
   char 	*s;
   uchar	loopc = 0;
   char		cur_dc = 4;
   char		ans[10];
   kbs_event event;
   
   snd_startup();
   
//   mres=snd_start_midi(&test_card[1]);
//   printf("SMid was %x for %s (%d) at %x %d %d %d\n",mres,test_card[1].dname,test_card[1].type,
//      test_card[1].io,test_card[1].irq,test_card[1].dma_8bit,test_card[1].dma_16bit);
//   printf("Channel return %x\n",snd_set_midi_sequences(8));

   snd_start_digital();

   snd_finish = test_snd_callback;
//   seq_finish=goofy_spanish_callback;

   spc.pan = SND_DEF_PAN;
   spc.vol = 0x100;
   spc.pri = SND_DEF_PRI;
   spc.flags = 0;
   spc.loops = 1;

   sndHdl[0] = LoadSound(218);
   sndHdl[1] = LoadSound(219);
   sndHdl[2] = LoadSound(287);
   sndHdl[3] = LoadSound(302);
   sndHdl[4] = LoadSound(306);
   sndHdl[5] = LoadSound(311);
   sndHdl[6] = LoadSound(206);
   sndHdl[7] = LoadSound(216);
   sndHdl[8] = LoadSound(228);
   
      printf("Sound Tester.  Use the following keys:\n");
      printf("1 - 9: Play a sound\n");
      printf("+,-:   Change volume\n");
      printf("<,>:   Pan left and right\n");
      printf("[,]:   Change loop count\n");
      printf("k:     Kill all sounds\n");
      printf("c:     Toggle call-back reporting\n");
      printf("m:     Mouse vol/pan test\n");
      printf("q:     Quit\n\n");

      kb_startup(NULL);
      
	   do {
	   		if (gCallbackFlag)
	   		{
	   			printf("Callback!\n");
	   			gCallbackFlag = FALSE;
	   		}
	   		
      		 event = kb_next();
      		 if (event.code!=0xff)
				 switch (event.ascii)
		         {
		            case 'q':
		            	inloop=FALSE;
		            	break;
		            case 'k':
		            	snd_kill_all_samples();
		            	printf("Killed all sounds.\n");
		            	break;
		            case 'c':
		            	gShowCallback = !gShowCallback;
		            	if (gShowCallback)
		            		printf("Callback reporting on.\n");
		            	else
		            		printf("Callback reporting off.\n");
		            	break;
		            case 'm':
		            	DoMouseSndTest();
		            	break;
		            case '-':
		            	if (spc.vol > 0)
		            		spc.vol -= 0x20;
		            	printf("New volume: %3.3X\n", spc.vol);
		            	break;
		            case '+':
		            	if (spc.vol < 0x100)
		            		spc.vol += 0x20;
		            	printf("New volume: %3.3X\n", spc.vol);
		            	break;
		            case '<':
		            	parm_move(&spc.pan, -1);
		            	printf("New pan: %2.2X\n", spc.pan);
		            	break;
		            case '>':
		            	parm_move(&spc.pan,  1);
		            	printf("New pan: %2.2X\n", spc.pan);
		            	break;
		            case '[':
		            	if (spc.loops > -1)
		            		spc.loops--;
		            	printf("New loops: %d\n", spc.loops);
		            	break;
		            case ']':
		            	spc.loops++;
		            	printf("New loops: %d\n", spc.loops);
		            	break;
		//            case '[': if (spos>0) spos-=2; // fall through, add 1, then print
		//            case ']': if (spos<40) spos++; printf("Now seq %d\n",spos); break;
					case '1':
						snd_sample_play(0, 0, (uchar *)*sndHdl[0], &spc);
						break;
					case '2':
						snd_sample_play(0, 0, (uchar *)*sndHdl[1], &spc);
						break;
					case '3':
						snd_sample_play(0, 0, (uchar *)*sndHdl[2], &spc);
						break;
					case '4':
						snd_sample_play(0, 0, (uchar *)*sndHdl[3], &spc);
						break;
					case '5':
						snd_sample_play(0, 0, (uchar *)*sndHdl[4], &spc);
						break;
					case '6':
						snd_sample_play(0, 0, (uchar *)*sndHdl[5], &spc);
						break;
					case '7':
						snd_sample_play(0, 0, (uchar *)*sndHdl[6], &spc);
						break;
					case '8':
						snd_sample_play(0, 0, (uchar *)*sndHdl[7], &spc);
						break;
					case '9':
						snd_sample_play(0, 0, (uchar *)*sndHdl[8], &spc);
						break;
		         }
	   } while (inloop);
   
   snd_kill_all_samples();
//   snd_kill_all_sequences();
   snd_shutdown();
   kb_shutdown();
}


//------------------------------------------------------------------------------
void DoMouseSndTest()
{
	snd_digi_parms	sdp;
	snd_digi_parms	*p;
	int		hnd_id;
	Point	mp;
	
	printf("Click mouse to stop test.\n");

	// Play a background sound over and over.
	
	sdp.pan = SND_DEF_PAN;
	sdp.vol = 0x100;
	sdp.pri = SND_DEF_PRI;
	sdp.flags = 0;
	sdp.loops = -1;
	hnd_id = snd_sample_play(0, 0, (uchar *)*sndHdl[8], &sdp);
	
	// Get the current parameters now that the sound's playing.
	
	p = snd_sample_parms(hnd_id);
	
	// Modify the volume and panning based on mouse position.
	
	while (!Button())
	{
		GetMouse(&mp);
		LocalToGlobal(&mp);
		p->vol = mp.v * 0x100 / 480;
		p->pan = mp.h * 0x7F / 640;
		snd_sample_reload_parms(p);
	}
	
	snd_end_sample(hnd_id);
}
