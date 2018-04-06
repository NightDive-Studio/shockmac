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
#include <stdlib.h>
#include <string.h>

#include "Shock.h"
#include "InitMac.h"
#include "criterr.h" 

/*
 * $Source: r:/prj/cit/src/RCS/criterr.c $
 * $Revision: 1.22 $
 * $Author: xemu $
 * $Date: 1994/11/26 03:37:32 $
 *
 *
 */

// -------
// DEFINES
// -------
#define GAME_NAME "System Shock" 
#define CLASS(x) ((x) >> 12)
#define TYPE(x) ((x) & 0xFFF)

// ------------------------------
// THE GLOBAL ERROR STRING ARRAYS
// ------------------------------


static char* criterr_type_messages[CRITERR_CLASSES] =
   {
      "Test", 
      "Configuration error",
      "Resource error",
      "Memory error",
      "File error",
      "Execution error",
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      "General failure"
   } ;

typedef struct _code_string
{
   short code;
   char* message;
} _code_string;

static _code_string code_messages[] =
{
   { CRITERR_TEST|1, ":  There is no cause for alarm, return to your homes."},
   { CRITERR_CFG|0, ":  Mouse driver not installed."},
   { CRITERR_RES|0, ":  Could not open string resource."},
   { CRITERR_RES|1, ":  Could not open game screen resource."},
   { CRITERR_RES|2, ":  Could not open MFD resource."},
   { CRITERR_RES|3, ":  Could not find hand art."},
   { CRITERR_RES|4, ":  Could not find palette."},
   { CRITERR_RES|5, ":  Could not open object art resource."},
   { CRITERR_RES|6, ":  Could not open side icon art resource."},
   { CRITERR_RES|7, ":  Could not open texture map resource."},
   { CRITERR_RES|8, ":  Could not find cache-loaded object art."},
   { CRITERR_RES|9, ":  Could not open 3d model resource!"},
   { CRITERR_RES|0xA, ":  Could not initialize popup cursors!"},
   { CRITERR_RES|0x10, ":  Could not find level archive!" },
   { CRITERR_MISC|0, ":  Bad object properties version number."},
   { CRITERR_MEM|0, ":  Out of memory allocating map."},
   { CRITERR_MEM|1, ":  Not enough memory to run game!"},
   { CRITERR_MEM|2, ":  Not enough memory to create critter caches!"},
   { CRITERR_MEM|3, ":  Not enough memory to load texture maps!"},
   { CRITERR_MEM|4, ":  Not enough memory to make popup cursors!"},
   { CRITERR_MEM|5, ":  Not enough memory to create SVGA cursor!"},
   { CRITERR_MEM|7, ":  Not enough memory to make email cursor!"},
   { CRITERR_MEM|8, ":  Frame buffer too small for vmail!" },
   { CRITERR_MEM|9, ":  Not enough cache memory to load bitmap!"},
   { CRITERR_FILE|0, ":  Could not load level file!"},
   { CRITERR_FILE|1, ":  Could not load level from archive!"},
   { CRITERR_FILE|2, ":  Could not find configuration file!"},
   { CRITERR_FILE|3, ":  Unknown Save Game failure!"},
   { CRITERR_FILE|4, ":  Unknown level-change writing failure!"},
   { CRITERR_FILE|5, ":  Corrupted save game!!"},
   { CRITERR_FILE|6, ":  Error saving game - likely insufficient disk space."},
   { CRITERR_FILE|7, ":  Error creating initial game - likely insufficient disk space."},
   { CRITERR_EXEC|1, ":  Cannot run directly from CD.  Run executable from hard drive."},
   { CRITERR_EXEC|2, ":  Unknown physics error!"},
};

#define NUM_CODE_MESSAGES  (sizeof(code_messages) / sizeof(_code_string))
/*
char *help_messages[] = {
"",
"Common problem solutions:",
"* Increase FILES in config.sys to 30 or more.",
"* Disable SMARTDRV write caching",
"* Use a minimal config.sys and autoexec.bat",
"",
"If none of these work, call Origin Tech Support",
"(512) 335-0440"
};

#define NUM_HELP_MESSAGES   8

// --------------------------------
// THE GLOBAL ERROR STATUS VARIABLE
// --------------------------------

static int error_status = NO_CRITICAL_ERROR;

// ---------
// INTERNALS
// ---------

void handle_critical_error(void)
{
   int i;
   char* s;
   char buf[256];
   if (error_status == NO_CRITICAL_ERROR) return;
   strcpy(buf,GAME_NAME);
   strcat(buf," can no longer run due to a fatal error.");
   puts(buf);
   strcpy(buf,"Error code "); itoa(error_status,buf+strlen(buf),16);
   puts(buf);
   s = criterr_type_messages[CLASS(error_status)];
   if (s != NULL) { puts(s);};
   for (i = 0; i < NUM_CODE_MESSAGES; i++)
   {
      if (code_messages[i].code == error_status)
      {
         puts(code_messages[i].message);
      }
   }
   for (i=0; i < NUM_HELP_MESSAGES; i++)
      puts(help_messages[i]);
}
*/
// ---------
// EXTERNALS
// ---------

/* KLC - Not needed for Mac
void criterr_init(void)
{
   error_status = NO_CRITICAL_ERROR;
   atexit(handle_critical_error);
}
*/

void critical_error(short code)
{
	char		buf[256];
	char		explain[256];
	char		*s;
	int		i, len;
	
	if (code == NO_CRITICAL_ERROR)
		return;

	if (gExtraMemory)
		DisposHandle(gExtraMemory);			// free our extra space
	
	sprintf(buf, "A fatal error has occurred in System Shock.  Error code %d.", code);
	len = strlen(buf);																	// Convert to p-string.
	BlockMove(buf, buf+1, 255);
	buf[0] = len;
	
	s = criterr_type_messages[CLASS(code)];									// Specific error message.
	if (s != NULL)
		strcpy(explain, s);
	for (i = 0; i < NUM_CODE_MESSAGES; i++)
		if (code_messages[i].code == code)
			strcat(explain, code_messages[i].message);
	len = strlen(explain);																// Convert to p-string.
	BlockMove(explain, explain+1, 255);
	explain[0] = len;
	
	ParamText((uchar *)buf, (uchar *)explain, "\p", "\p");			// Show the error.
	if (len > 0)
		StopAlert(1001, nil);
	else
		StopAlert(1000, nil);

	CleanupAndExit();																	// Get out of here.
}
