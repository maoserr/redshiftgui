
#include "common.h"
#include "iup.h"
#include "options.h"

static void _init_main_dialog( int show ){
	Ihandle *dialog, *framesun,
			*framebuttons, *frameinfo;

	framesun = IupFrame(NULL);
	framebuttons = IupFrame(NULL);
	frameinfo = IupFrame(NULL);

	dialog = IupDialog(
		IupHbox(
			framesun,
			IupVbox(
				framebuttons,
				frameinfo,
				NULL),
			NULL
			)
		);
	IupSetAttribute(dialog,"TITLE",_("Redshift GUI"));
	IupSetfAttribute(dialog,"SIZE","%dx%d",300,150);
	if( show )
		IupShow(dialog);
}

int iup_gui(int argc, char *argv[]){
	IupOpen( &argc,&argv );
	_init_main_dialog(1);
	IupMainLoop();
	IupClose();
	return RET_FUN_SUCCESS;
}
