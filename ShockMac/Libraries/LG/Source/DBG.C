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
//		DBG.C		Debug/reporting system
//		Rex E. Bradford (REX)
//
//	OVERVIEW
//
//		This module implements a debugging/error reporting system.
//		This system allows debugging code and debug reporting (spew)
//		messages to be:
//
//		1) Included in code and executing (turned on)
//		2) Included in code but not executing (turned off)
//		3) Excluded from the object code altogether.
//
//		Furthermore, control of execution of debug code and spew may be
//		turned on and off at runtime at a fine level of detail, by putting
//		different instances of such code into a set of categories (banks
//		and slots), each with independent control.
//		
//		If a set of debug code is turned on, it is executed.  If a spew
//		message is turned on, it is sent to the reporting system, where
//		it may be logged on the monochrome screen and/or logged to a file.
//		There are 3 additional levels of reporting, Error, WarnUser, and
//		Warning:
//
//			Error - Severe error: message is logged to mono screen and all
//						logfiles, program is shut down, and message is printed
//						on console.  Cannot be compiled out of code.
//
//			WarnUser - User warning: sent to user function, if any.  Then
//						logs to mono screen and all logfiles.  Cannot be compiled
//						out of code.
//
//			Warning - Warning to developer: message, prefixed with "WARNING: ",
//						is logged to mono screen & all logfiles.  Compiled into
//						code if WARN_ON is set.
//
//			Spew - Information reporting: may or may not be logged to mono
//						screen and/or a logfile, based on "gates" which control
//						what spew is logged and where.  Compiled into code if
//						SPEW_ON is set.
//
//	DEFINITIONS
//
//		Some definitions:
//
//		BANK - Debug statements and Spew messages are associated a particular
//				"debug bank".  The debug system allows 32 independent banks.
//				For instance, one bank might be assigned to the 2D library.
//
//		SLOT - Within each bank, 27 "slots" are available.  Each slot has
//				independent control over gating of debug code and spew.
//				A given debug statement or reporting message may be marked
//				with a single bank and slot, or may be marked as belonging
//				to multiple slots within the same bank.  In our 2D library
//				bank example, one slot might be assigned to clipping reports,
//				another to polygon statistics, another to canvas
//				change calls, etc.
//
//		SOURCE - Debug code or a spew message has a "source", which is a
//				combination of a bank and one or more slots.  These are packed
//				into a 32-bit ulong (high 5 bits bank, low 27 bits are slot
//				bits).  A source represents a logical "anding" of the slots.
//				In our 2D bank example, suppose we assigned another slot to
//				"massive detail".  We might then do some polygon reporting
//				with just the polygon slot as source, and other more detailed
//				reporting with a source set to the combination of polygons
//				and massive detail slots.
//
//		GATE - Each bank/slot combination features a "debug gate" for debug
//				messages, a "mono" gate and "func" gate and logfile for spew.
//				These gates come into play assuming the debug code or spew
//				message has not been compiled out.  If a debug source has
//				multiple slots selected in it, then the debug gates for all
//				slots must be set to let the code execute.  Similarly, for
//				mono logging to occur, the mono gates for all slots in the
//				source must be set (in our 2D example, to look at high-detail
//				polygon info we must have both the polygon slot's mono gate and
//				the "massive detail" slot's mono gate set.
//					Logfile logging works a little differently.  If a source
//				has multiple slots selected, the spew goes to all such logfiles.
//					The 3 gates and logfile are described further below:
//
//				DBG - Determines whether debug code associated with this
//						bank/slot is executed or not.
//
//				MONO - Determines whether spew associated with this bank/slot
//						is logged to mono.
//
//				FUNC - Determines whether spew associated with this bank/slot
//						is sent to the "user" routine, if any is installed.
//
//				LOGFILE - Determines file which spew associated with this
//						bank/slot is logged to.
//
//				Note then, that for a spew message to be logged somewhere,
//				the following must be true:
//
//				1) The file must be compiled with the SPEW_ON flag set,
//					so that the code is compiled in.
//				2) For the spew to be logged to mono, all MONO gates in the
//					source must be set.
//				3) For the spew to be sent to the "user" function, all FUNC
//					gates in the source must be set, and a user function must
//					be installed.
//				4) For the spew to be logged to a file, at least one slot
//					in the source must be attached to a logfile.
//
//				Note that a spew message is mono-logged only if all gates in
//				the source are set ("and").  In order to generate a spew when
//				any of the gates is set ("or"), it is neccessary to use multiple
//				Spew() statements, one for each slot.  This may cause the spew
//				to be repeated in logfiles, however.
//
//	DEBUG CODE & REPORTING MESSAGES
//
//		On, then, to the important macros to be used:
//
//		DBG(src,{code})	- Executes code if DBG gate is set for all of
//								slots in the source (and DBG_ON is set)
//
//		Error(ecode, msg,...)		- Logs error to mono, func, and all logfiles,
//								shuts down program, displays message on console,
//								exits with error code.
//
//		WarnUser(msg,...)	- Logs to mono, func, and all logfiles.
//
//		Warning((msg,...))	- Logs message, with "WARNING: " prefix, to
//								mono and all logfiles.  Note double-parentheses
//								to make the doofy macro work.
//
//		Spew(src,(msg,...)) - Logs to mono if all relevant MONO gates are set;
//								logs to user function if all relevant FUNC gates are
//								set; logs to logfiles attached to ALL slots in src.
//								Note the double-parentheses again.
//
//		To compile these out of code altogether, use these defines:
//
//			DBG_ON	- If set, include DBG()'s, else compile out
//			WARN_ON	- If set, include Warning()'s, else compile out
//			SPEW_ON	- If set, include Spew()'s, else compile out
//
//		It is not possible to compile out Error()'s and WarnUser()'s.
//
//	SETTING GATES
//
//		The following macros and functions are provided to set the gates:
//
//		DbgSetDbg(bank,slots)	- Sets the DBG mask for the given bank.
//		DbgGetDbg(bank)			- Gets the DBG mask for the given bank.
//		DbgSetMono(bank,slots)	- Sets the MONO mask for the given bank.
//		DbgGetMono(bank)			- Gets the MONO mask for the given bank.
//		DbgSetFunc(bank,slots)	- Sets the FUNC mask for the given bank.
//		DbgGetFunc(bank)			- Gets the FUNC mask for the given bank.
//
//	LOGFILES
//
//		While it appears that each bank/slot combination may have its own
//		logfile, in reality there is a limit of 16 open logfiles, which may
//		be further reduced by the host file system.  The management of
//		unique logfile names and opening and closing of files as the
//		names change is handled transparently.  When the log path is
//		changed, all logfiles are closed and reopened in the new directory.
//
//	OTHER FUNCTIONS
//
//		Other useful routines & macros include:
//
//		DbgInit()					- Auto-loads "debug.dbg" config file from
//											current directory.  Not needed unless
//											this is desired; mono config screen
//											does this automatically first time called up.
//
//		DbgSetLogPath(path)     - Set path to put all logfiles in.  As
//											this closes all logfiles, it should be
//											done before setting logfile names.
//
//		DbgSetLogFile(src,name) - Set logfile for the specified source, and
//											opens the file if it is not already open.
//
//		Exit(errcode,msg)			- Shut down gracefully; call this instead
//											of exit().
//
//		AtExit(func)				- Add function to list of routines to be
//											called during exit shutdown.
//
//		DbgSetExitMsg(str)		- Set message to be displayed upon exit.
//
//		DbgSetReportRoutine(func) - Set "user routine" to be called on
//											messages.  This function is passed
//											the report type (ERROR/WARNUSER/WARNING/SPEW)
//											as well as the message, so it can act
//											on the message type.
//
//	CONFIG FILES
//
//		The manipulation of gates by user code is tiresome, so two alternative
//		methods, which go hand in hand, are provided for making life easier.
//		The first is the ability to load and save "debug config files"; the
//		second is user control via a mono screen dialog box.
//			Config files serve two purposes:
//
//			1) To name banks and slots (these names are used in the mono
//				debug screen to be discussed subsequently.
//			2) To pre-set gates and filenames for the various bank/slots.
//
//		Debug config files feature the following statements:
//
//		BANKNAME banknum bankname - Associate bank number with a name.
//
//		SLOTNAME slotnum slotname - Associate slot number in current bank
//			(last BANKNAME or BANK statement seen) encountered with a name.
//
//		LOGPATH path - Set the directory path for logfiles to go in.
//
//		BANK {name | number} [DBG] [MONO] [FILE filename] -
//			Set gates and logfiles for the specified bank.  All slots
//			within the bank are set.
//
//		SLOT {name | number} [DBG] [MONO] [FILE filename] -
//			Set gates and logfile for the specified slot in the current
//			bank (last BANKNAME or BANK statement seen).  Sets one slot's
//			gates only.
//
//		Empty lines are allowed, and anything following // is ignored.
//
//		These functions manage config files:
//
//		DbgAddConfigPath(path) adds a directory the the path used to
//			search for config files.
//
//		DbgLoadConfig(fname) loads and processes a config file, setting
//			the specified names and gates and logfiles.  Logfiles are
//			automatically opened if they have not been seen before.
//			Multiple config files may be processed.
//
//		DbgSaveConfig(fname) saves all gate and logfile information to
//			the specified file.  Bank and slot names are not saved; these
//			should be supplied in one or more separate config files which
//			are created by a text editor.
//
//	MONO SCREEN CONFIG FUNCTION
//
//		The function DbgMonoConfig() should be assigned to a hotkey in
//		any program using the debug system.  This pops up on the mono
//		screen a display which allows manipulation of all gates and
//		logfiles, as well as the log path.  Bank and slot names appear
//		but are not editable.
//
//	SOURCES AND THE DBGMAKEH TOOL
//
//		The debug and reporting macros require a source to be specified
//		as their first argument.  These sources are a combination of
//		a bank and one or more slots.  The following macro:
//
//		DBGSRC(bank,slotmask) - Creates a source from the specified bank
//			and slotmask.  For instance:
//
//		DBGSRC(4,0x0400) creates a source associated with bank 4, with slot
//			bit 10 set, and
//		DBGSRC(6,0x0001 | 0x0004) creates a source associated with bank 6,
//			with bits 0 and 2 set.
//
//		Bank numbers and slot masks need to be specified symbolically, we
//		want to type:
//
//			DBG(DBGSRC(MYBANK,MYSLOT1),{...});
//		and:
//			Spew(DBGSRC(MYBANK,MYSLOT1), ("Spew message %d\n", 999));
//
//		or even better:
//
//			#define MYSRC1 (DBGSRC(MYBANK,MYSLOT1))
//			DBG(MYSRC1,{...});
//		and:
//			Spew(MYSRC1, ("Spew message\n"));
//
//		The tool DBGMAKEH is supplied to make a header file from a debug
//		config file.  It looks only at BANKNAME and SLOTNAME statements,
//		and produces a header file with the appropriate constants:
//
//		DBGMAKEH configfile hfile bankPrefix slotPrefix srcPrefix
//
//		where:
//
//			configfile	= full pathname of config file to be processed
//			hfile			= full pathname of hfile to be created
//			bankPrefix	= prefix chars to bank names (such as "DB_" or "")
//			slotPrefix	= prefix chars to slot names (such as "DS_" or "")
//			srcPrefix	= prefix chars to source names (such as "DSRC_" or "")
//
//	CONVENTIONS
//
//		By convention, the following files will be created and used:
//
//		LIBDBG.DBG	- Contains BANKNAMES and SLOTNAMES for LookingGlass
//					libraries, which will occupy banks 0 thru 15.  This file
//					is located in n:\project\lib\release, which is pointed to
//					by the environment variable "LGLIBDIR".
//
//		LIBDBG.H	- Generated from LIB.DBG (using DBGMAKEH tool), has #define's
//					for bank/slot/src constants.  This file is also in the
//					n:\project\lib\release dir.
//
//		projdbg.DBG		- Contains BANKNAMES and SLOTNAMES for a given program,
//					which will occupy banks 16 thru 31.  This file should be
//					located in the main project directory (such as \project\ff\src),
//					which is pointed to by environment variable "PROJDIR"
//
//		projdbg.H	- Generated from projdbg.DBG (using DBGMAKEH tool), has
//					#defines for bank/slot/src constants.  This file
//					should be located in the same directory as the .DBG file.
//		
//		Then, the startup code in a program should contain:
//
//			DbgInit() - optional, will auto-load "debug.dbg" debug settings
//
//		Library modules which include DBG and Spew macros must include:
//
//			#include "libdbg.h"
//
//		And program modues which include DBG and Spew macros must include:
//
//			#include "projdbg.h", or something like "..\..\..\projdbg.h"
//										(if the project dir is not in THE include path)
//

/*
* $Header: r:/prj/lib/src/lg/rcs/dbg.c 1.18 1994/09/07 17:20:10 dfan Exp $
* $Log: dbg.c $
 * Revision 1.18  1994/09/07  17:20:10  dfan
 * Spews would not get sent to a file unless mono flag was on too
 * 
 * Revision 1.17  1994/08/12  17:05:19  jak
 * Split up decl of DbgSetReportRoutine() to make
 * Externed some things for use by dbgpp.cc
 * ?
 * .?
 * .e
 * .v
 * Split DbgHandleC() off DbgHandle()
 * Externed some things for use by dbgpp.cc
 * Split up decl of DbgSetReportRoutine() to make
 * c++ compiler happy
 * 
 * Revision 1.16  1994/08/11  10:26:59  dfan
 * DbgSpewTest would succeed if any slot in src's bank had
 * a file attached to it
 * 
 * Revision 1.15  1994/08/09  13:53:05  kaboom
 * Changed call to vsprintf() to lg_sprintf().
 * 
 * Revision 1.14  1994/02/28  10:53:31  rex
 * Moved DbgGetKey() from dbgcfg.c to here
 * 
 * Revision 1.13  1993/07/16  14:29:53  rex
 * Doubled Report routine's msg buffer to 1024 bytes (it's on the stack,
 * so I hope this is ok).
 * 
 * Revision 1.12  1993/07/09  09:33:21  rex
 * Added #ifdef's to delete most of code when DBG_ON and SPEW_ON undefined
 * 
 * Revision 1.11  1993/03/25  12:54:10  rex
 * Changed report handler to put tag on print msg too.
 * 
 * Revision 1.10  1993/02/25  12:53:04  rex
 * Removed exit code and put in exit.c
 * 
 * Revision 1.9  1993/02/16  13:41:16  rex
 * Fixed 2 bugs (WARNING/WARNUSER tags reversed, wasn't logging WARNINGS)
 * 
 * Revision 1.8  1993/02/04  20:04:10  rex
 * Changed DbgExit() to Exit(), etc.
 * 
 * Revision 1.7  1993/02/04  13:17:55  rex
 * Fixed bug in DbgSetLogFile()
 * 
 * Revision 1.6  1993/02/02  17:07:25  rex
 * Fixed logpath (".\", not "..\")
 * 
 * Revision 1.5  1993/02/02  16:47:24  rex
 * Changed starting dbgLogPath to "..\\" (instead of empty)
 * 
 * Revision 1.4  1993/02/02  11:58:51  rex
 * Fixed strncpy() bug in slot name
 * 
 * Revision 1.3  1993/01/29  17:30:39  rex
 * Added arg to Error()
 * 
 * Revision 1.2  1993/01/29  10:01:38  rex
 * Changed to use Malloc() instead of malloc()
 * 
 * Revision 1.1  1993/01/29  09:47:56  rex
 * Initial revision
 * 
*/

#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
#include <string.h>
//#include <conio.h>

#include "dbg.h"
//#include <lgsprntf.h>
//#include <mprintf.h>
//#include <memall.h>
//#include <kb.h>
//#include <kbcook.h>
//#include <keydefs.h>


//--------------------------------------------------------
//  Displays a warning message.  If being compiled for the Shock program, the
//  message displays in an Alert box; if for test programs, it just comes out as
//  a printf.
//--------------------------------------------------------
void DoWarningMsg(char *msg)
{
#ifdef SHOCK_GAME
	Str255	message;
	
	InitCursor();
	BlockMove(msg, message+1, 255);
	message[0] = strlen(msg);
	ParamText(message, "\p", "\p", "\p");
	
	StopAlert(1000, nil);

#else

	printf("\n%s\n", msg);

#endif
}

/*
#ifdef DBG_ON

DbgBank dbgBank[NUM_DBG_BANKS];
DbgLogFile dbgLogFile[NUM_DBG_LOGFILES];
char dbgLogPath[128] = ".\\";

#endif

#ifdef SPEW_ON
static ulong spewSrc;
#endif

static void (*dbg_f_report_user)(int errType, char *msg) = NULL;

int (*f_getch)() = getch;

int errErrCode;

extern char *pExitMsg;

void DbgHandleC(int reportType, ulong src, char *fmt, __va_list __arg);

//	----------------------------------------------------------
//		REPORTING ROUTINES
//	----------------------------------------------------------
//
//	DbgReportError() does an error report.
//
//		ecode = error code to exit with
//		msg   = message

void DbgReportError(int ecode, char *msg, ...)
{
	va_list ap;

	errErrCode = ecode;
	va_start(ap, msg);
	DbgHandleC(DBG_ERROR, DBG_SLOT_MASK, msg, ap);
	va_end(ap);
}

//	----------------------------------------------------------
//
//	DbgReportWarnUser() does a warn user report.
//
//		msg = message

void DbgReportWarnUser(char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	DbgHandleC(DBG_WARNUSER, DBG_SLOT_MASK, msg, ap);
	va_end(ap);
}

#ifdef WARN_ON

//	----------------------------------------------------------
//
//	DbgReportWarning() does a warning if source gates permit.
//
//		msg = message

void DbgReportWarning(char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	DbgHandleC(DBG_WARNING, DBG_SLOT_MASK, msg, ap);
	va_end(ap);
}

#endif

#ifdef SPEW_ON

//	----------------------------------------------------------
//
//	DbgSpewTest() tests to see if spew should go, sets global for
//		subsequent spew if it happens.
//
//		src = message source
//
//	Returns: TRUE if should do spew, FALSE if not

bool DbgSpewTest(ulong src)
{
	int bank;

	bank = DBGBANK(src);
	if (((dbgBank[bank].gate[DG_MONO] & src) != (src & DBG_SLOT_MASK)) &&
		((dbgBank[bank].gate[DG_FILE] & src & DBG_SLOT_MASK) != (src & DBG_SLOT_MASK)))
			return FALSE;

	spewSrc = src;
	return TRUE;
}

//	----------------------------------------------------------
//
//	DbgDoSpew() does a spew.
//
//		msg = message

void DbgDoSpew(char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	DbgHandleC(DBG_SPEW, spewSrc, msg, ap);
	va_end(ap);
}

#endif

//	------------------------------------------------------------
//		KEYBOARD ROUTINE
//	------------------------------------------------------------
//
//	DbgGetKey() gets char from keyboard.
//
//	Returns: key

int DbgGetKey()
{
	int c;

//	Get key

	c = (*f_getch)();

//	If 0, get 2nd key value, set flags appropriately

	if (c == 0)
		{
		c = (*f_getch)();
		switch (c)
			{
			case 0x4b:
			case 0x4d:
			case 0x48:
			case 0x50:
				c |= KB_FLAG_2ND;
				break;
			}
		c |= KB_FLAG_SPECIAL;
		}

//	Return key

	return(c);
}

//	----------------------------------------------------------
//		CONTROL ROUTINES
//	----------------------------------------------------------
//	-----------------------------------------------------------
//
//	DbgSetReportRoutine() sets routine to call for reporting.
//
//		f_report_user = report routine, taking report type & msg args

void DbgSetReportRoutine(void (*f_report_user)(int reportType, char *msg))
{
	dbg_f_report_user = f_report_user;
}

#ifdef DBG_ON

//	-----------------------------------------------------------
//
//	DbgSetSlotNames() sets a set of names for bank slots.
//
//		bank     = bank
//		namelist = ptr to array of slot names, 27 in length of NULL-term

void DbgSetSlotNames(int bank, char **namelist)
{
	int slot;

	for (slot = 0; slot < NUM_DBG_SLOTS; slot++)
		{
		if (*namelist == NULL)
			break;
		DbgSetSlotName(bank, slot, *namelist);
		namelist++;
		}
}

//	------------------------------------------------------------
//
//	DbgSetSlotName() sets a single bank slot name.
//
//		bank = bank
//		slot = slot
//		name = slot name

void DbgSetSlotName(int bank, int slot, char *name)
{
	DbgBank *pbank;

//	If bank slot array unallocated, allocate & clear it

	pbank = &dbgBank[bank];
	if (pbank->ppBankSlotNames == NULL)
		{
		pbank->ppBankSlotNames = Malloc(NUM_DBG_SLOTS * (sizeof(char *)));
		if (pbank->ppBankSlotNames == NULL)
			return;
		memset(pbank->ppBankSlotNames, 0, NUM_DBG_SLOTS * sizeof(char *));
		}

//	If this slot already has a name, get rid of it

	if (pbank->ppBankSlotNames[slot])
		Free(pbank->ppBankSlotNames[slot]);

//	Allocate space for name, copy name into it, and point at it

	pbank->ppBankSlotNames[slot] = Malloc(MAX_SLOTNAMELEN + 1);
	if (pbank->ppBankSlotNames[slot] == NULL)
		return;
	strncpy(pbank->ppBankSlotNames[slot], name, MAX_SLOTNAMELEN);
	*(pbank->ppBankSlotNames[slot] + MAX_SLOTNAMELEN) = 0;
}

//	-----------------------------------------------------------
//
//	DbgFindBankName() finds a bank name (case insensitive).
//
//		name = name to find
//
//	Returns: bank number 0-31 or -1 if not found

int DbgFindBankName(char *name)
{
	int bank;

	for (bank = 0; bank < NUM_DBG_BANKS; bank++)
		{
		if (strcmpi(dbgBank[bank].bank_name, name) == 0)
			return(bank);
		}

	return(-1);
}

//	-----------------------------------------------------------
//
//	DbgFindSlotName() finds a slot name (case insensitive).
//
//		name = name to find
//
//	Returns: slot number 0-26 or -1 if not found

int DbgFindSlotName(int bank, char *name)
{
	char **ppslotname;
	int slot;

//	If slot names unallocated, not here

	if (dbgBank[bank].ppBankSlotNames == NULL)
		return(-1);

//	Else search for a match (case insensitive)

	ppslotname = dbgBank[bank].ppBankSlotNames;
	for (slot = 0; slot < NUM_DBG_SLOTS; slot++, ppslotname++)
		{
		if (*ppslotname && (strcmpi(*ppslotname, name) == 0))
			return(slot);
		}

	return(-1);
}

//	-----------------------------------------------------------
//
//	DbgSetLogPath() sets the directory used for log files.
//		All current logfiles are closed.
//
//		path = ptr to path string

void DbgSetLogPath(char *path)
{
	int bank,slot;

//	Set new path

	strcpy(dbgLogPath, path);

//	Close all logfiles

	DbgCloseLogFiles();

//	Set all bank/slot file indexes to 0

	for (bank = 0; bank < NUM_DBG_BANKS; bank++)
		{
		for (slot = 0; slot < NUM_DBG_SLOTS; slot++)
			dbgBank[bank].file_index[slot] = 0;
		dbgBank[bank].gate[DG_FILE] = 0;
		}
}

//	-----------------------------------------------------------
//
//	DbgSetLogFile() sets the log file for a set of slots in a bank.
//
//		src  = bank & slots
//		name = filename, or NULL to remove from file logging
//
//	Returns: TRUE if file opened successfully, FALSE if not

bool DbgSetLogFile(ulong src, char *name)
{
	int index,slot;
	uchar *pfi;
	DbgLogFile *pdlf;

//	Look at current entries in these slots, reducing files' ref counts

	for (slot = 0, pfi = dbgBank[DBGBANK(src)].file_index;
		slot < NUM_DBG_SLOTS; slot++, pfi++)
		{
		if ((1 << slot) & src)
			{
			if (*pfi)
				{
				pdlf = &dbgLogFile[*pfi];
				if (--pdlf->refCount == 0)
					{
					fclose(pdlf->fp);
					pdlf->name[0] = 0;
					}
				*pfi = 0;
				}
			}
		dbgBank[DBGBANK(src)].gate[DG_FILE] &= ~src;
		}

//	If empty name, we'll put 0 into the relevant slots

	if ((name == NULL) || (name[0] == 0))
		{
		index = 0;
		goto INSERT_INDEX;
		}

//	Search for existing log file with this name

	for (index = 1; index < NUM_DBG_LOGFILES; index++)
		{
		if (strcmpi(dbgLogFile[index].name, name) == 0)
			break;
		}

//	If not found, try to add new one, if no room return FALSE
//	If found empty slot, open file & insert it

	if (index == NUM_DBG_LOGFILES)
		{
		for (index = 1; index < NUM_DBG_LOGFILES; index++)
			{
			if (dbgLogFile[index].fp == NULL)
				break;
			}
		if (index == NUM_DBG_LOGFILES)
			return FALSE;
		strncpy(dbgLogFile[index].name, name, MAX_DBG_LOGFILENAME);
		if (DbgOpenLogFile(index) == FALSE)
			return FALSE;
		}

//	Got valid index, enter into all slot fields

INSERT_INDEX:

	for (slot = 0, pfi = dbgBank[DBGBANK(src)].file_index;
		slot < NUM_DBG_SLOTS; slot++, pfi++)
		{
		if ((1 << slot) & src)
			{
			*pfi = index;
			if (index > 0)
				dbgLogFile[index].refCount++;
			}
		}
	if (index)
		dbgBank[DBGBANK(src)].gate[DG_FILE] |= src;

	return TRUE;
}

//	----------------------------------------------------------
//		LOG FILES
//	----------------------------------------------------------
//
//	DbgOpenLogFile() opens a log file.
//
//		index = file index

bool DbgOpenLogFile(int index)
{
	DbgLogFile *pdlf;
	DbgBank *pbank;
	int bank,slot,len;
	char fname[128];

//	Construct name from path & filename

	strcpy(fname, dbgLogPath);
	len = strlen(fname);
	if (len && (fname[len-1] != '\\') && (fname[len-1] != '/'))
		strcat(fname, "\\");
	strcat(fname, dbgLogFile[index].name);

//	Open file

	pdlf = &dbgLogFile[index];
	pdlf->fp = fopen(fname, "w");

//	If can't open, remove any references to this file index

	if (pdlf->fp == NULL)
		{
		pdlf->name[0] = 0;
		pdlf->refCount = 0;
		for (bank = 0, pbank = dbgBank; bank < NUM_DBG_BANKS; bank++, pbank++)
			{
			for (slot = 0; slot < NUM_DBG_SLOTS; slot++)
				{
				if (pbank->file_index[slot] == index)
					{
					pbank->file_index[slot] = 0;
					pbank->gate[DG_FILE] &= ~(1 << slot);
					}
				}
			}
		return FALSE;
		}

	return(TRUE);
}

//	-----------------------------------------------------------
//
//	DbgCloseLogFiles() closes all logfiles.

void DbgCloseLogFiles()
{
	DbgLogFile *pdlf;

	for (pdlf = dbgLogFile; pdlf < &dbgLogFile[NUM_DBG_LOGFILES]; pdlf++)
		{
		if (pdlf->fp)
			{
			fclose(pdlf->fp);
			pdlf->fp = NULL;
			pdlf->refCount = 0;
			pdlf->name[0] = 0;
			}
		}
}

#endif

//	----------------------------------------------------------
//		MASTER ERROR HANDLER -- INTERNAL
//	----------------------------------------------------------
//
//	DbgHandle() and DbgHandleC() do the actual report handling.
// DbgHandle() is factored out to be used by the C++ version,
// which is called DbgHandleCpp()

#pragma off(unreferenced);

void DbgHandle(int reportType, ulong src, char *buff)
{
static bool exiting = FALSE;
	int slot,fileIndex;
	uchar *pfi;
	ulong didLogFile;

//	If user warning & user routine, call it

	if (dbg_f_report_user)
		(*dbg_f_report_user)(reportType, buff);

#ifdef DBG_ON

//	Log to monochrome screen all gates allow (or if non-spew, always do it)

	if ((reportType != DBG_SPEW) ||
		((dbgBank[DBGBANK(src)].gate[DG_MONO] & src) == (src & DBG_SLOT_MASK)))
			{
			mprintf("%s", buff);
			}

//	If not spew, write to all logfiles

	if (reportType != DBG_SPEW)
		{
		for (fileIndex = 1; fileIndex < NUM_DBG_LOGFILES; fileIndex++)
			{
			if (dbgLogFile[fileIndex].fp)
				{
				fprintf(dbgLogFile[fileIndex].fp, "%s", buff);
				fflush(dbgLogFile[fileIndex].fp);
				}
			}
		}
	else

//	Else if spew, log to relevant file (all slots in source)

		{
		for (slot = 0, didLogFile = 0, pfi = dbgBank[DBGBANK(src)].file_index;
			slot < NUM_DBG_SLOTS; slot++, pfi++)
			{
			if (src & (1 << slot))
				{
				fileIndex = *pfi;
				if (fileIndex && ((didLogFile & (1 << fileIndex)) == 0))
					{
					fprintf(dbgLogFile[fileIndex].fp, "%s", buff);
					fflush(dbgLogFile[fileIndex].fp);
					didLogFile |= (1 << fileIndex);	// did this file, don't repeat
					}
				}
			}
		}

#endif

//	If error, exit cleanly

	if ((reportType == DBG_ERROR) && !exiting)
		{
		exiting = TRUE;
		Exit(errErrCode, buff);
		}
}

char *dbgTags[] = {"","WARNING: ","WARN USER: ","ERROR: "};
void DbgHandleC(int reportType, ulong src, char *fmt, __va_list __arg)
{
	__va_list myarg;
	char buff[1024];

//	Format message into buffer

	myarg[0] = __arg[0];
	strcpy(buff, dbgTags[reportType]);
	lg_vsprintf(buff + strlen(buff), fmt, myarg);

   DbgHandle(reportType,src,buff);
}

#pragma on(unreferenced);
*/

