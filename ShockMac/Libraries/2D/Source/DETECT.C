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
 * $Source: r:/prj/lib/src/2d/RCS/detect.c $
 * $Revision: 1.11 $
 * $Author: kevin $
 * $Date: 1994/08/16 13:16:57 $
 *
 * Routine to detect what kind of video card is present and which
 * graphics modes are available.
 *
 * This file is part of the 2d library.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// MLA #include <datapath.h"
#include "grs.h"
#include "detect.h"
#include "bitmap.h"
#include "cnvtab.h"
#include "devtab.h"
#include "idevice.h"
#include "mode.h"
#include "cnvtab.h"
#include "tabdat.h"
// MLA #include "vesa.h"

// extern 
extern void (**grd_device_table_list[])();

// ======================================================================
// Mac version of gr_detect
int gr_detect(grs_sys_info *info)
 {
	/* default to 640x480x8 standard Mac res */
	info->id_maj = 0;
	info->id_min = 0;
	info->memory = 300;
	info->modes[0] = GRM_640x480x8;
	info->modes[1] = -1;
	info->modes[2] = -1;
	info->modes[3] = -1;
	info->modes[4] = -1;

  grd_device_table = grd_device_table_list[info->id_maj];
  grd_canvas_table_list[BMT_DEVICE] = (void (**)())grd_device_table[GRT_CANVAS_TABLE];

	return(0);
 }

// ======================================================================
// PC version of gr_detect
#if 0 

char *command[] = { "type","vendor","memory","modes" };
char *card_type[] = { "vga","svga","tiga",NULL };
char *vga_vendor[] = { "standard",NULL };
char *svga_vendor[] = { "vesa","paradise","stealth","trident","tseng","video7",NULL };
char *tiga_vendor[] = { "standard",NULL };
char **card_vendor[] = { vga_vendor,svga_vendor,tiga_vendor,NULL };
Datapath datapathCfg;

int gr_detect(grs_sys_info *info)
{
   char token[80];
   FILE *fp;
   int i;
   int err=0;

   DatapathAddEnv(&datapathCfg, "CFGDIR");
   DatapathAdd(&datapathCfg, "c:/bin");
   if ((fp=DatapathOpen(&datapathCfg, "video.cfg", "r"))==NULL) {
      if (vesa_get_info(info) != 0) {
         /* default to vga. urk. */
         info->id_maj = 0;
         info->id_min = 0;
         info->memory = 256;
         info->modes[0] = GRM_320x200x8;
         info->modes[1] = GRM_320x200x8X;
         info->modes[2] = GRM_320x240x8;
         info->modes[3] = GRM_320x400x8;
         info->modes[4] = GRM_320x480x8;
      }
      else {
         int mode_val, cmode, tog;
         for (mode_val=GRM_320x200x8; mode_val<=GRM_320x480x8; mode_val++)
         {           // go add the 5 modes above if they are missing?
            cmode=0;
            while ((info->modes[cmode]!=-1)&&(info->modes[cmode]!=mode_val))
               cmode++;
            if (info->modes[cmode]==-1)
             { info->modes[cmode++]=mode_val; info->modes[cmode]=-1; }
         }
         for (mode_val=cmode-1; mode_val>=0; mode_val--)
            for (tog=0; tog<mode_val; tog++)
               if (info->modes[tog]>info->modes[tog+1]) {
                  int tmp=info->modes[tog+1];
                  info->modes[tog+1]=info->modes[tog];
                  info->modes[tog]=tmp;
               }
      }
   } else {
      while (!feof(fp)) {
         fscanf(fp, "%s", token);
         for (i=0; command[i]!=NULL; i++)
            if (!stricmp(command[i], token))
               break;
         switch (i) {
         case 0:           /* type */
            fscanf(fp, "%s", token);
            for (i=0; card_type[i]!=NULL; i++)
               if (!stricmp(card_type[i], token))
                  info->id_maj = i;
            break;
         case 1:           /* vendor */
            fscanf(fp, "%s", token);
            for (i=0; card_vendor[info->id_maj][i]!=NULL; i++)
               if (! stricmp(card_vendor[info->id_maj][i], token))
                  info->id_min = i;
            break;
         case 2:           /* memory */
            fscanf(fp, "%s", token);
            info->memory = atoi (token);
            break;
         case 3:           /* modes */
            i = 0;
            while (fscanf(fp, "%s", token)!=EOF)
               info->modes[i++] = atoi(token);
            info->modes[i] = -1;
            break;
         default:
            err = 1;
         }
      }
      fclose (fp);
   }
   if (err==0) {
      grd_device_table = grd_device_table_list[info->id_maj];
      grd_canvas_table_list[BMT_DEVICE] =
         (void (**)())grd_device_table[GRT_CANVAS_TABLE];
   }
   return err;
}
#endif

