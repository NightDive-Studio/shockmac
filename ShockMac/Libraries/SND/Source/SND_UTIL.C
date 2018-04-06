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
#include "midi.h"

//------------------------
//  Globals
//------------------------
Handle		gHeaderHdl;
Handle		gTuneHdl;
Handle		gOfsHdl;
long			*gOffsets;


//--------------------------------------------------------------------------
//  Open the theme file and get all the resources.  (For Mac version).
//--------------------------------------------------------------------------
OSErr snd_load_theme(FSSpec *specPtr, TunePlayer thePlayer)
{
	OSErr	err;
	
	short	refNum = FSpOpenResFile(specPtr, fsRdPerm);
	if (refNum >= 0)
	{
		gHeaderHdl = GetResource('thdr', 128);
		gTuneHdl = GetResource('them', 128);
		gOfsHdl = GetResource('tofs', 128);
	
		DetachResource(gHeaderHdl);	
		DetachResource(gTuneHdl);	
		DetachResource(gOfsHdl);
		
		CloseResFile(refNum);

		// Set the tune header.
		HLock(gHeaderHdl);
		err = TuneSetHeader(thePlayer, (unsigned long *)*gHeaderHdl);
		err = TunePreroll(thePlayer);
		
		// Lock down the offsets and data.
		HLock(gTuneHdl);
		HLock(gOfsHdl);
		gOffsets = (long *)*gOfsHdl;
		
		return(0);
	}
	else
		return(-1);
}

//--------------------------------------------------------------------------
//  Releases all resources associated with a theme.  You better be dang sure there ain't no
//  tunes playing when you call this.
//--------------------------------------------------------------------------
void snd_release_current_theme(void)
{
	if (gHeaderHdl)
	{
		HUnlock(gHeaderHdl);
		DisposeHandle(gHeaderHdl);
	}
	if (gTuneHdl)
	{
		HUnlock(gTuneHdl);
		DisposeHandle(gTuneHdl);
	}
	if (gOfsHdl)
	{
		HUnlock(gOfsHdl);
		DisposeHandle(gOfsHdl);
	}
}


/*
char *snd_load_raw(char *fname, int *ldat)
{
   int fh;

   if ((fh=open(fname,O_RDONLY|O_BINARY))!=-1)
   {
      ulong len;
      uchar *ptr;
      len=lseek(fh,0,SEEK_END);
      lseek(fh,0,SEEK_SET);
      if ((ptr=(uchar *)Malloc(len))!=NULL)
      {
	      read(fh,ptr,len);
         close(fh);
	      if (ldat!=NULL)
	       { ldat[0]=len; ldat[1]=(int)ptr; }
	      return ptr;
      }
      close(fh);
   }
   if (ldat!=NULL)
    { ldat[0]=0; ldat[1]=NULL; }
   return NULL;
}
*/
