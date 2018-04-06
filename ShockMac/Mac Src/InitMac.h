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
//====================================================================================
//
//		System Shock - ©1994-1995 Looking Glass Technologies, Inc.
//
//		InitMac.h	-	Initialize Mac toolbox managers and setup the application's globals.
//
//====================================================================================


#include <Timer.h>

//---------------------
//  Time Manager defines
//---------------------
typedef struct
{
	TMTask			task;							// The actual TimeManager task structure
	long				*ticksPtr;					// Pointer to the ticks
}
ShockTask, *ShockTaskPtr;
extern ShockTask	pShockTicksTask;

#define	kShockTicksFreq		-14286		//-3571



//--------------------
//  Prototypes
//--------------------
void InitMac(void);
void FailNIL(void *);
Handle GetResourceFail(long id, short num);
void CheckConfig(void);
void ErrorDie(short stringnum);
void CleanupAndExit(void);
void SetupWindows(WindowPtr *mainWind);
void SetUpMenus(MenuHandle *theMenus, short numMenus);
void StringAlert(short stringnum);
void GetFolders(void);
void InstallShockTimers(void);
void RemoveShockTimers(void);
void StartShockTimer(void);
void StopShockTimer(void);
void HideMenuBar(void);
void ShowMenuBar(void);
