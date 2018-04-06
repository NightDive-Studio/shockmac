#include <Windows.h>	// for GetNewWindow()
#include <Fonts.h>		// for InitFonts()
#include <Dialogs.h>	// for InitDialogs, paramtext & Alert
#include <Quickdraw.h>	// for InitGraf

/* defines */
#define kChooseAMessageAlertID 128
#define kDisplayMessageAlertID 129

/* globals */
const char *gHelloString = "Hello, World!";
const char *gGoodbyeString = "Goodbye, Cruel World!";
struct QDGlobals qd;

void main()
{
	short itemHit;		// for the "Choose a Message" Alert
	
	/* Initialize program and put up an alert */
	/* Note the horrible user interface here.  No menus, no
	"real" event loop, etc. */
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();

	/* Wait for the user to choose� */
	itemHit = Alert(kChooseAMessageAlertID, nil);
	
	/* Call the assembly language routine to handle the user's choice.
		This is an example of an assembly language routine
		being called from (and passing data to) a C routine.  */
	ClickHandler(itemHit);
	
}

void DisplayAlert(char *message) {
/* this routine doesn't care which message the Asm routine
   passes it.  It just displays it in an alert. 
   This is an example of a C routine being called from an
   assembly language routine.*/
	short itemHit;
	
	paramtext(message, "", "", "");
	itemHit = Alert(kDisplayMessageAlertID, nil);
}
