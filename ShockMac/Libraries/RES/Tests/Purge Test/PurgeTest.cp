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
//=========================================================
//  PurgeTest.cp - Program to test the memory purging capabilities of the resource
//						    library.
//=========================================================

#include <iostream.h>

#include "res.h"

//--------------------------------------
void main(void);


//---------------------------------------------------------
//  Main routine.
//---------------------------------------------------------
void main(void)
{
	StandardFileReply	reply, reply2;
	SFTypeList				typeList;
	short						filenum, filenum2;

	ResInit();
	
	cout << "Select a resource file...\n\n" ;
	
	typeList[0] = 'Sgam';
	typeList[1] = 'rsrc';
	StandardGetFile(nil, 2, typeList, &reply);
	if (!reply.sfGood)
		return;
	else
	{
		filenum = ResOpenFile(&reply.sfFile);
		cout << "Opened file, filenum = " << filenum << "\n";
	}

	cout << "Select another resource file (optional)...\n\n" ;
	filenum2 = -1;	
	StandardGetFile(nil, 2, typeList, &reply2);
	if (reply.sfGood)
	{
		filenum2 = ResOpenFile(&reply2.sfFile);
		cout << "Opened file, filenum = " << filenum2 << "\n";
	}

	{
		short		i;
		Id 			id;
		Ptr		p;
		short 	rs;
		ResDesc	*prd;
				
		for (i = 0; i < 2; i++)
		{
			for (id = ID_MIN; id <= resDescMax; id++)
			{
				prd = RESDESC(id);
				if (prd->filenum != 0)
				{
					rs = ResSize(id);
					cout << "Geting resource " << id << " (size: " << rs << ")     ";

					if (!ResInUse(id))
						cout << "Resource not in use!\n";
					else if (!ResPtr(id))
						cout << "Resource not in memory!\n";
					else
						cout << endl;
					
					p = (Ptr)ResLock(id);
					cout << "Free mem: " << FreeMem() << "    ";
					cout << "Max block size: " << MaxBlock() << endl;

					ResUnlock(id);
	
//					long  temp;
//					Delay(40, &temp);
				}
			}
			cout << "\nGo through 'em again...\n\n";
		}
		
		cout << "Closing files.\n" ;
		ResCloseFile(filenum);
		if (filenum2 != -1)
			ResCloseFile(filenum2);
	}

	ResTerm();
}
