#include "common.h"
#include <iup.h>
#include "options.h"
#include "iupgui_main.h"

static Ihandle *dialog_location=NULL;
static Ihandle *list_method=NULL;

// Creates location dialog
static void _location_create(void){
	Ihandle *vbox_method,
			*frame_method,

			*button_cancel,
			*button_save,
			*hbox_buttons,
			*vbox_all;
	extern Ihandle *himg_redshift;

	list_method = IupList(NULL);
	IupSetAttribute(list_method,"DROPDOWN","YES");
	IupSetAttribute(list_method,"1",_("Lookup by IP"));
	IupSetAttribute(list_method,"2",_("Lookup by Address"));
	IupSetAttribute(list_method,"3",_("Manual"));
	IupSetAttribute(list_method,"EXPAND","YES");
	IupSetAttribute(vbox_method,"MARGIN","5");
	frame_method = IupFrame(vbox_method);
	IupSetAttribute(frame_method,"TITLE",_("Method"));

	button_cancel = IupButton(_("Cancel"),NULL);
	IupSetfAttribute(button_cancel,"MINSIZE","%dx%d",60,24);
	button_save = IupButton(_("Save"),NULL);
	IupSetfAttribute(button_save,"MINSIZE","%dx%d",60,24);
	hbox_buttons = IupHbox(
			button_cancel,
			button_save,
			NULL);

	vbox_all = IupVbox(
			frame_method,
			hbox_buttons,
			NULL);
	IupSetfAttribute(vbox_all,"NMARGIN","%dx%d",5,5);
	IupSetAttribute(vbox_all,"ALIGNMENT","ARIGHT");

	dialog_location = IupDialog(vbox_all);
	IupSetAttribute(dialog_location,"TITLE",_("Location"));
	IupSetAttributeHandle(dialog_location,"ICON",himg_redshift);
}

// Shows the location dialog
int guilocation_show(Ihandle *ih){
	if( !dialog_location ){
		_location_create();
	}
	IupPopup(dialog_location,IUP_CENTER,IUP_CENTER);
	if( !guimain_exit_normal() )
		return IUP_CLOSE;
	return IUP_DEFAULT;
}

// Closes location dialog
int guilocation_close(Ihandle *ih){
	if( dialog_location ){
		IupDestroy( dialog_location );
		dialog_location = NULL;
	}
	return IUP_DEFAULT;
}
