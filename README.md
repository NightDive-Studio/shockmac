System Shock GPL Source Code
============================
Copyright 2015-2018, Night Dive Studios, Incorporated.

This file contains the following sections:

GENERAL NOTES
LICENSE

GENERAL NOTES
=============

Game data and patching:
-----------------------

This source release does not contain any game data, the game data is still
covered by the original EULA and must be obeyed as usual.


Compiling on MacOS 9:
---------------------

A project file for Metrowerks Codewarrior is provided in 
ShockMac/Shock.µ 

You will need a PowerPC to build the Mac version of System Shock.
Fortunately, there are emulators out there that are able to run PowerPC Mac Applications, like SheepShaver.
You also need Metrowerks CodeWarrior 10 Tools to build the project and Stuffit Expander to unpack the project files and source code.
The game was initially built with an older version of CodeWarrior, but we tested it with CW 10 Gold Tools (IDE 1.7) and it was able to build a working executable.
Things you will need:

* A PowerMac or an emulator like SheepShaver;
* CodeWarrior for PowerPC (CW 10 Gold Tools works)
* The original Mac System Shock data.

About the source code storage:

The System Shock source code and project files are stored in the Macintosh file system, which implies
the usage of two forks for each file, a data fork and a resource fork (as well as a metadata section).
For that reason, we decided to include both the compressed files and uncompressed files.
The uncompressed files are stored in the ShockMac folder. Inside each folder there are two subfolders:
one named ".rsrc", which stores the resource fork for each file, and another named ".finf", which
stores the metada for every file.
The files can also be found compressed inside "ShockMac.sit" under the Stuffit format. This format
allows storeing both forks and metada information for every file.

About the source code organization:

The source code is split into three sections: Game code, Mac code and Libraries.

* Game code is related to System Shock-specific code, like enemy AI, level rendering, etc.
* Mac code is related to Macintosh-specific code, like the typical user interface and internal behavior.
* Libraries are related to code shared by other Looking Glass games, like 2D and 3D rendering, sound, input, resources, palette etc.

How to unpack the project:

In your Mac, open ShockMac.sit using a program capable of reading Stuffit packed files.
After unpacking it, a folder named ShockMac will be generated with the contents inside it.

How to compile:

1. Double-click on the CodeWarrior IDE 1.7 icon to start the IDE
2. Open one of the projects inside the folder ShockMac:Libraries
3. Click on Project -> Make to build the library.
4. Move the generated library file to the folder ShockMac:Lib
5. Close the project by clicking on File -> Close
6. Repeat steps (2) to (5) for the following projects:
    * ShockMac:Libraries:2d:2dLibPPC.µ 
    * ShockMac:Libraries:3d:3dLibPPC.µ 
    * ShockMac:Libraries:DSTRUCT:DSTRUCT.µ 
    * ShockMac:Libraries:EDMS:EDMS.µ 
    * ShockMac:Libraries:FIX:FIX.µ 
    * ShockMac:Libraries:FIXPP:FIXPP.µ 
    * ShockMac:Libraries:INPUT:INPUT.µ 
    * ShockMac:Libraries:LG:LG.µ 
    * ShockMac:Libraries:PALETTE:PALETTE.µ 
    * ShockMac:Libraries:RES:RES.µ 
    * ShockMac:Libraries:RND:RND.µ 
    * ShockMac:Libraries:SND:SND.µ 
    * ShockMac:Libraries:UI:UI.µ 
    * ShockMac:Libraries:VOX:VOX.µ 
7. Open the project ShockMac:LIB:LGHeadersPPC.µ 
8. Precompile by clicking on Project -> Precompile
9. Close the project
10. Open the project ShockMac:Shock.µ 
11. Click on Project -> Make to build the game.
12. Make sure the game resources are in the folder ShockMac:Data
13. Launch the game.

LICENSE
=======

The System Shock source code is available under the terms of the GNU
General Public License v3.0

See COPYING.txt for the GNU GENERAL PUBLIC LICENSE
