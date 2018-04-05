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
//=======================================================================
//
// ConvertIMAGE - Code to convert a PC IMAGE resource to the Mac.
//              	by Ken Cobb.
//
//=======================================================================

#include "lg_types.h"
#include "res.h"
#include "rect.h"

typedef struct {
   uchar *bits;      
   uchar type;       
   uchar align;      
   ushort flags;     
   short w;          
   short h;          
   ushort row;       
   uchar wlog;       
   uchar hlog;       
} grs_bitmap;

typedef struct {
   grs_bitmap bm;       // embedded bitmap, bm.bits set to NULL
   union {
      LGRect updateArea;  // update area (for anims)
      LGRect anchorArea;  // area to anchor sub-bitmap
      LGPoint anchorPt;   // point to anchor from
      } u;
   long pallOff;        // offset to pallette
                        // bitmap's bits follow immediately
} FrameDesc;

//-----------------------------------------------------------------------
void ConvertIMAGE(Ptr dataPtr, long size)
{
	FrameDesc	*fd = (FrameDesc *)dataPtr;

	fd->bm.flags = SwapShortBytes(fd->bm.flags);
	fd->bm.w = SwapShortBytes(fd->bm.w);
	fd->bm.h = SwapShortBytes(fd->bm.h);
	fd->bm.row = SwapShortBytes(fd->bm.row);

	fd->u.updateArea.ul.x = SwapShortBytes(fd->u.updateArea.ul.x);
	fd->u.updateArea.ul.y = SwapShortBytes(fd->u.updateArea.ul.y);
	fd->u.updateArea.lr.x = SwapShortBytes(fd->u.updateArea.lr.x);
	fd->u.updateArea.lr.y = SwapShortBytes(fd->u.updateArea.lr.y);

	fd->pallOff = SwapLongBytes(fd->pallOff);

	// For type 4 images (RSD encoded), scan through the data and
	// convert any shorts or longs.
	
	if (fd->bm.type == 4)
	{
		unsigned short	s;
		unsigned char*	cp;
		
		cp = (unsigned char *)(fd + 1);	// Point to the beginning
													// of image data
		while (TRUE)
		{
			if (*cp == 0)					// If it's a RUN.
				cp += 3;						// move past the length and char bytes
			
			else if (*cp == 0x80)		// If it's a LONG OPCODE
			{
				cp++;
												// Read and swap the short
				s = (unsigned short)SwapShortBytes(*(short *)cp);
				*(short *)cp = s;
				
				cp += 2;						// and move past it

				if (s == 0)					// We're outta here!
					return;
					
				if (s >= 0x8000)			// If it's a LONG RUN or LONG DUMP
				{
					if (s >= 0xC000)		// If it's a LONG RUN
						cp++;					// move past the run char byte
					else
					{
						s &= 0x7FFF;		// Else it's a LONG DUMP, so move
						cp += s;				// past the bytes.
					}
				}
				// LONG SKIP has no arguments to move past.
			}
			
			else if (*cp > 0x80)			// If it's a SKIP
				cp++;							// move past this byte.
			
			else								// Else it's a DUMP
				cp += *cp + 1;				// so scan past the count and bytes
		}
	}
}
