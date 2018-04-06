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
 *	MacHeaders.c
 *
 *	Script to generate the 'MacHeaders<xxx>' precompiled header for Metrowerks C/C++.
 *  Copyright © 1993 metrowerks inc.  All rights reserved.
 */

/*
 *	Required for c-style toolbox glue function: c2pstr and p2cstr
 *	the inverse operation (pointers_in_A0) is performed at the end ...
 */

#ifndef powerc
 #pragma pointers_in_D0							
#endif

/*
 *	To allow the use of ToolBox calls which have now become obsolete on PowerPC, but
 *	which are still needed for System 6 applications, we need to #define OBSOLETE.  If
 *	your application will never use these calls then you can comment out this #define.
 *	NB: This is only for 68K ...
 */

#if !defined(powerc) && !defined(OBSOLETE)
 #define OBSOLETE	1
#endif

/*
 *	Metrowerks-specific definitions
 *
 *	These definitions are commonly used but not in Apple's headers. We define
 *	them in our precompiled header so we can use the Apple headers without modification.
 */

#define PtoCstr		p2cstr
#define CtoPstr		c2pstr
#define PtoCString	p2cstr
#define CtoPString	c2pstr

#define topLeft(r)	(((Point *) &(r))[0])
#define botRight(r)	(((Point *) &(r))[1])

#define TRUE		true
#define FALSE		false

#ifndef powerc
 #include <MixedMode.h>
 long GetCurrentA5(void)
  ONEWORDINLINE(0x200D);
#endif

/*
 *	Apple #include files
 *
 *	Uncomment any additional #includes you want to add to MacHeaders.
 */

//	#include <ADSP.h>
	#include <AEObjects.h>
	#include <AEPackObject.h>
	#include <AERegistry.h>
	#include <AEUserTermTypes.h>
//	#include <AIFF.h>
	#include <Aliases.h>
	#include <AppleEvents.h>
//	#include <AppleGuide.h>
	#include <AppleScript.h>
//	#include <AppleTalk.h>
//	#include <ASDebugging.h>
//	#include <ASRegistry.h>
//	#include <Balloons.h>
//	#include <CMApplication.h>
//	#include <CMComponent.h>
//	#include <CodeFragments.h>
	#include <ColorPicker.h>
//	#include <CommResources.h>
//	#include <Components.h>
	#include <ConditionalMacros.h>
//	#include <Connections.h>
//	#include <ConnectionTools.h>
	#include <Controls.h>
//	#include <ControlStrip.h>
//	#include <CRMSerialDevices.h>
//	#include <CTBUtilities.h>
//	#include <CursorCtl.h>
//	#include <CursorDevices.h>
//	#include <DatabaseAccess.h>
//	#include <DeskBus.h>
	#include <Devices.h>
	#include <Dialogs.h>
//	#include <Dictionary.h>
//	#include <DisAsmLookup.h>
//	#include <Disassembler.h>
	#include <DiskInit.h>
//	#include <Disks.h>
//	#include <Displays.h>
//	#include <Drag.h>
//	#include <Editions.h>
//	#include <ENET.h>
//	#include <EPPC.h>
//	#include <ErrMgr.h>
	#include <Errors.h>
	#include <Events.h>
//	#include <fenv.h>
	#include <Files.h>
//	#include <FileTransfers.h>
//	#include <FileTransferTools.h>
	#include <FileTypesAndCreators.h>
//	#include <Finder.h>
//	#include <FixMath.h>
	#include <Folders.h>
	#include <Fonts.h>
//	#include <fp.h>
//	#include <FragLoad.h>
//	#include <FSM.h>
	#include <Gestalt.h>
//	#include <HyperXCmd.h>
//	#include <Icons.h>
//	#include <ImageCodec.h>
//	#include <ImageCompression.h>
//	#include <IntlResources.h>
//	#include <Language.h>
	#include <Lists.h>
	#include <LowMem.h>
//	#include <MachineExceptions.h>
//	#include <MacTCP.h>
//	#include <MediaHandlers.h>
	#include <Memory.h>
	#include <Menus.h>
//	#include <MIDI.h>
	#include <MixedMode.h>
//	#include <Movies.h>
//	#include <MoviesFormat.h>
//	#include <Notification.h>
//	#include <OSA.h>
//	#include <OSAComp.h>
//	#include <OSAGeneric.h>
	#include <OSUtils.h>
	#include <Packages.h>
//	#include <Palettes.h>
//	#include <Picker.h>
//	#include <PictUtil.h>
//	#include <PictUtils.h>
	#include <PLStringFuncs.h>
//	#include <Power.h>
//	#include <PPCToolbox.h>
	#include <Printing.h>
	#include <Processes.h>
//	#include <QDOffscreen.h>
	#include <Quickdraw.h>
//	#include <QuickdrawText.h>
//	#include <QuickTimeComponents.h>
	#include <Resources.h>
//	#include <Retrace.h>
//	#include <ROMDefs.h>
#ifndef powerc
//	#include <SANE.h>
#endif
	#include <Scrap.h>
//	#include <Script.h>
//	#include <SCSI.h>
	#include <SegLoad.h>
//	#include <Serial.h>
//	#include <ShutDown.h>
//	#include <Slots.h>
//	#include <Sound.h>
//	#include <SoundComponents.h>
//	#include <SoundInput.h>
//	#include <Speech.h>
	#include <StandardFile.h>
//	#include <Start.h>
	#include <Strings.h>
//	#include <Terminals.h>
//	#include <TerminalTools.h>
	#include <TextEdit.h>
//	#include <TextServices.h>
	#include <TextUtils.h>
//	#include <Threads.h>
//	#include <Timer.h>
	#include <ToolUtils.h>
//	#include <Translation.h>
//	#include <TranslationExtensions.h>
	#include <Traps.h>
//	#include <TSMTE.h>
	#include <Types.h>
//	#include <Unmangler.h>
//	#include <Video.h>
	#include <Windows.h>
//	#include <WorldScript.h>


/*
 *	required for c-style toolbox glue function: c2pstr and p2cstr
 *	(match the inverse operation at the start of the file ...
 */

#ifndef powerc
 #pragma pointers_in_A0
#endif
