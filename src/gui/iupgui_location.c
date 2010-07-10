#include "common.h"
#include <iup.h>
#include "options.h"
#include "location.h"
#include "iupgui_main.h"
#include "iupgui_location.h"

static Ihandle *dialog_location=NULL;
static Ihandle *list_method=NULL;
static Ihandle *lbl_status=NULL;
static Ihandle *edt_lat=NULL;
static Ihandle *edt_lon=NULL;
static Ihandle *run_task=NULL;

// Runs geocode hostip callback
static int _run_geocode(Ihandle *ih){
	float lat,lon;
	char city[100];
	LOG(LOGVERBOSE,_("Running geocode subfunction."));
	IupSetAttribute(run_task,"RUN","NO");
	IupSetAttribute(list_method,"VISIBLE","YES");
	IupSetAttribute(list_method,"VALUE","0");
	if(!location_geocode_hostip(&lat,&lon,city,100)){
		IupSetAttribute(lbl_status,"APPEND",_("Unable to download data"));
		return IUP_DEFAULT;
	}

	IupSetfAttribute(lbl_status,"APPEND",_("city: %s"),city);
	IupSetfAttribute(lbl_status,"APPEND",_("lat/lon: %.2f,%.2f"),lat,lon);
	IupSetAttribute(lbl_status,"APPEND",_("Go to http://www.hostip.info/ to correct"));
	IupSetfAttribute(edt_lat,"VALUE","%f",lat);
	IupSetfAttribute(edt_lon,"VALUE","%f",lon);
	return IUP_DEFAULT;
}

// List method changes
static int _list_method_cb(Ihandle *ih,
		char *text, int pos, int state){
	if( state == 0 )
		return IUP_DEFAULT;
	if( !run_task )
		run_task = IupSetAtt(NULL,IupTimer(),"TIME","100",NULL);
	switch(pos){
		case 1:
			LOG(LOGVERBOSE,_("Running geocode with hostip"));
			IupSetAttribute(lbl_status,"VALUE","");
			IupSetfAttribute(lbl_status,"APPEND",
				_("Downloading info, this may be slow..."));
			IupSetCallback(run_task,"ACTION_CB",(Icallback)_run_geocode);
			IupSetAttribute(run_task,"RUN","YES");
			IupSetAttribute(list_method,"VISIBLE","NO");
		break;
		case 2:{
			char buffer[100]="Test";
			LOG(LOGVERBOSE,_("Retrieving address..."));
			IupGetText(_("Input address/zip:"),buffer);
		break;
		}
	};
	return IUP_DEFAULT;
}

// Button cancel
static int _location_cancel(Ihandle *ih){
	guilocation_close(ih);
	return IUP_DEFAULT;
}

// Button save
static int _location_save(Ihandle *ih){
	float lat = IupGetFloat(edt_lat,"VALUE");
	float lon = IupGetFloat(edt_lon,"VALUE");
	opt_set_location(lat,lon);
	opt_write_config();
	guimain_update_info();
	guilocation_close(ih);
	return IUP_DEFAULT;
}

// Creates location dialog
static void _location_create(void){
	Ihandle *vbox_method,
			*frame_method,

			*lbl_lat,
			*lbl_lon,
			*vbox_loc_titles,
			*vbox_loc_values,
			*hbox_loc,
			*frame_loc,
			*button_cancel,
			*button_save,
			*hbox_buttons,
			*vbox_all;
	extern Ihandle *himg_redshift;

	// Drop down for method selection
	list_method = IupSetAtt(NULL,IupList(NULL),"DROPDOWN","YES",
		"1",_("Lookup by IP (hostip.info)"),
		"2",_("Lookup by Address/Zip"),
		"EXPAND","YES",NULL);
	IupSetCallback(list_method,"ACTION",(Icallback)_list_method_cb);
	lbl_status = IupSetAtt(NULL,IupText(NULL),"EXPAND","YES",
			"READONLY","YES",
			"MULTILINE","YES",
			"APPENDNEWLINE","YES",
			"SCROLLBAR","VERTICAL",
			"VISIBLELINES","3",
			NULL);
	vbox_method = IupVbox(list_method,lbl_status,NULL);
	IupSetAttribute(vbox_method,"MARGIN","5");
	frame_method = IupFrame(vbox_method);
	IupSetAttribute(frame_method,"TITLE",_("Method"));

	// Location input/display
	lbl_lat = IupSetAtt(NULL,IupLabel(_("Latitude:")),"EXPAND","YES",NULL);
	lbl_lon = IupSetAtt(NULL,IupLabel(_("Longitude:")),"EXPAND","YES",NULL);
	vbox_loc_titles = IupVbox(lbl_lat,lbl_lon,NULL);
	IupSetAttribute(vbox_loc_titles,"EXPAND","YES");
	edt_lat = IupSetAtt(NULL,IupText(NULL),"EXPAND","YES",
			NULL);
	edt_lon = IupSetAtt(NULL,IupText(NULL),"EXPAND","YES",
			NULL);
	IupSetfAttribute(edt_lat,"VALUE","%.2f",opt_get_lat());
	IupSetfAttribute(edt_lon,"VALUE","%.2f",opt_get_lon());
	vbox_loc_values = IupVbox(edt_lat,edt_lon,NULL);
	hbox_loc = IupHbox(vbox_loc_titles,vbox_loc_values,NULL);
	IupSetAttribute(hbox_loc,"MARGIN","5");
	frame_loc = IupFrame(hbox_loc);
	IupSetAttribute(frame_loc,"TITLE",_("Location"));
	
	// Buttons
	button_cancel = IupButton(_("Cancel"),NULL);
	IupSetfAttribute(button_cancel,"MINSIZE","%dx%d",60,24);
	IupSetCallback(button_cancel,"ACTION",(Icallback)_location_cancel);
	button_save = IupButton(_("Save"),NULL);
	IupSetfAttribute(button_save,"MINSIZE","%dx%d",60,24);
	IupSetCallback(button_save,"ACTION",(Icallback)_location_save);
	hbox_buttons = IupHbox(
			button_cancel,
			button_save,
			NULL);

	vbox_all = IupVbox(
			frame_method,
			frame_loc,
			hbox_buttons,
			NULL);
	IupSetfAttribute(vbox_all,"NMARGIN","%dx%d",5,5);
	IupSetAttribute(vbox_all,"ALIGNMENT","ARIGHT");

	dialog_location = IupDialog(vbox_all);
	IupSetAttribute(dialog_location,"TITLE",_("Location"));
	IupSetAttribute(dialog_location,"SIZE","200x");
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
	if( run_task ){
		IupDestroy( run_task );
		run_task = NULL;
	}
	return IUP_DEFAULT;
}
