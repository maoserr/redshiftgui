#include "common.h"
#include "options.h"
#include "location.h"
#include "iupgui.h"
#include "iupgui_main.h"
#include "iupgui_gamma.h"
#include "iupgui_location.h"

extern Ihandle *himg_redshift;

/*@null@*/ static Ihandle *dialog_location=NULL;
/*@null@*/ static Ihandle *list_method=NULL;
/*@null@*/ static Ihandle *lbl_status=NULL;
/*@null@*/ static Ihandle *vbox_method=NULL;
/*@null@*/ static Ihandle *edt_lat=NULL;
/*@null@*/ static Ihandle *edt_lon=NULL;
/*@null@*/ static Ihandle *run_task=NULL;

// Extra controls not always visible
/*@null@*/ static Ihandle *edt_address=NULL;
/*@null@*/ static Ihandle *btn_address=NULL;
/*@null@*/ static Ihandle *hbox_address=NULL;

// Runs geocode hostip callback
static int _run_geocode(/*@unused@*/ Ihandle *ih)
{
	float lat,lon;
	char city[100];

	// Check if controls have been created
	if( (lbl_status==NULL)
			|| (list_method==NULL)
			|| (edt_lat==NULL)
			|| (edt_lon==NULL) ){
		LOG(LOGERR,_("Function called without GUI!"));
		return IUP_DEFAULT;
	}

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

// Runs geocode with geobytes callback
static int _run_geobytes(/*@unused@*/ Ihandle *ih){
	float lat,lon;
	char city[100];
	LOG(LOGVERBOSE,_("Running geobytes subfunction."));
	IupSetAttribute(run_task,"RUN","NO");
	IupSetAttribute(list_method,"VISIBLE","YES");
	IupSetAttribute(list_method,"VALUE","0");
	if(!location_geocode_geobytes(&lat,&lon,city,100)){
		IupSetAttribute(lbl_status,"APPEND",_("Unable to download data"));
		return IUP_DEFAULT;
	}

	IupSetfAttribute(lbl_status,"APPEND",_("city: %s"),city);
	IupSetfAttribute(lbl_status,"APPEND",_("lat/lon: %.2f,%.2f"),lat,lon);
	IupSetAttribute(lbl_status,"APPEND",_("Geobytes limits maximum of 20 lookups per hour"));
	IupSetfAttribute(edt_lat,"VALUE","%f",lat);
	IupSetfAttribute(edt_lon,"VALUE","%f",lon);
	return IUP_DEFAULT;
}

// Clears address on focus
static int _address_clear(/*@unused@*/ Ihandle *ih){
	IupSetAttribute(edt_address,"VALUE","");
	return IUP_DEFAULT;
}

// Address lookup function
static int _address_lookup(/*@unused@*/ Ihandle *ih){
	char *result = IupGetAttribute(edt_address,"VALUE");
	float lat,lon;
	char city[100];
	int rescode;

	if( result == NULL )
		return IUP_DEFAULT;

	IupSetAttribute(list_method,"VISIBLE","YES");
	IupSetAttribute(list_method,"VALUE","0");
	IupSetAttribute(lbl_status,"VALUE","");
	IupSetfAttribute(lbl_status,"APPEND",
		_("Downloading info, this may be slow..."));
	rescode=location_address_lookup(result,&lat,&lon,city,100);
	// Destroy address box
	if( hbox_address )
		IupDestroy(hbox_address);
	hbox_address = btn_address = edt_address = NULL;
	if( dialog_location )
		IupRefresh(dialog_location);
	if(!rescode){
		IupSetAttribute(lbl_status,"APPEND",_("Unable to download data"));
		return IUP_DEFAULT;
	}
	
	IupSetfAttribute(lbl_status,"APPEND",_("city: %s"),city);
	IupSetfAttribute(lbl_status,"APPEND",_("lat/lon: %.2f,%.2f"),lat,lon);
	IupSetfAttribute(edt_lat,"VALUE","%f",lat);
	IupSetfAttribute(edt_lon,"VALUE","%f",lon);

	return IUP_DEFAULT;
}

// List method changes
static int _list_method_cb(/*@unused@*/ Ihandle *ih,
		/*@unused@*/ char *text, int pos, int state){
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
			(void)IupSetCallback(run_task,"ACTION_CB",(Icallback)_run_geocode);
			IupSetAttribute(run_task,"RUN","YES");
			IupSetAttribute(list_method,"VISIBLE","NO");
		break;
		case 2:
			LOG(LOGVERBOSE,_("Running geocode with geobytes"));
			IupSetAttribute(lbl_status,"VALUE","");
			IupSetfAttribute(lbl_status,"APPEND",
				_("Downloading info, this may be slow..."));
			(void)IupSetCallback(run_task,"ACTION_CB",(Icallback)_run_geobytes);
			IupSetAttribute(run_task,"RUN","YES");
			IupSetAttribute(list_method,"VISIBLE","NO");
		break;
		case 3:{
			edt_address = IupSetAtt(NULL,IupText(NULL),
					"VALUE","Enter Address...",
					"EXPAND","HORIZONTAL",NULL);
			(void)IupSetCallback(edt_address,"GETFOCUS_CB",(Icallback)_address_clear);
			btn_address = IupSetAtt(NULL,IupButton(_("Lookup"),NULL),NULL);
			(void)IupSetCallback(btn_address,"ACTION",(Icallback)_address_lookup);
			hbox_address = IupHbox(edt_address,btn_address,NULL);
			if( (vbox_method!=NULL) 
					&& (hbox_address!=NULL)
					&& (lbl_status!=NULL) )
				(void)IupInsert(vbox_method,lbl_status,hbox_address);
			else{
				LOG(LOGERR,_("Location search controls not created!"));
				return IUP_DEFAULT;
			}
			IupSetAttribute(list_method,"VISIBLE","NO");
			(void)IupMap(hbox_address);
			IupRefresh(hbox_address);
		break;
		}
	};
	return IUP_DEFAULT;
}

// Button cancel
static int _location_cancel(/*@unused@*/ Ihandle *ih){
	return IUP_CLOSE;
}

// Button save
static int _location_save(/*@unused@*/ Ihandle *ih){
	float lat,lon;
	if( (edt_lat==NULL) || (edt_lon==NULL) ){
		LOG(LOGERR,_("Edt controls not accessible!"));
		return IUP_CLOSE;
	}
	lat = IupGetFloat(edt_lat,"VALUE");
	lon = IupGetFloat(edt_lon,"VALUE");
	(void)opt_set_location(lat,lon);
	opt_write_config();
	return IUP_CLOSE;
}

// Creates location dialog
static void _location_create(void){
	Ihandle *frame_method,

			*lbl_lat,
			*lbl_lon,
			*hbox_loc,
			*frame_loc,
			*button_cancel,
			*button_save,
			*hbox_buttons,
			*vbox_all;

	// Drop down for method selection
	list_method = IupSetAtt(NULL,IupList(NULL),"DROPDOWN","YES",
		"1",_("Lookup by IP (hostip.info)"),
		"2",_("Lookup by IP (Geobytes)"),
		"3",_("Lookup by Address/Zip"),
		"EXPAND","HORIZONTAL",NULL);
	(void)IupSetCallback(list_method,"ACTION",(Icallback)_list_method_cb);
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
	lbl_lat = IupSetAtt(NULL,IupLabel(_("Latitude:")),NULL);
	lbl_lon = IupSetAtt(NULL,IupLabel(_("Longitude:")),NULL);
	edt_lat = IupSetAtt(NULL,IupText(NULL),
			NULL);
	edt_lon = IupSetAtt(NULL,IupText(NULL),
			NULL);
	IupSetfAttribute(edt_lat,"VALUE","%.2f",opt_get_lat());
	IupSetfAttribute(edt_lon,"VALUE","%.2f",opt_get_lon());
	hbox_loc = IupHbox(lbl_lat,edt_lat,IupFill(),
			lbl_lon,edt_lon,NULL);
	IupSetAttribute(hbox_loc,"MARGIN","5");
	IupSetAttribute(hbox_loc,"ALIGNMENT","ACENTER");
	frame_loc = IupFrame(hbox_loc);
	IupSetAttribute(frame_loc,"TITLE",_("Location"));
	
	// Buttons
	button_cancel = IupButton(_("Cancel"),NULL);
	IupSetfAttribute(button_cancel,"MINSIZE","%dx%d",60,24);
	(void)IupSetCallback(button_cancel,"ACTION",(Icallback)_location_cancel);
	button_save = IupButton(_("Save"),NULL);
	IupSetfAttribute(button_save,"MINSIZE","%dx%d",60,24);
	(void)IupSetCallback(button_save,"ACTION",(Icallback)_location_save);
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
	IupSetAttribute(dialog_location,"SIZE","200x150");
	IupSetAttributeHandle(dialog_location,"ICON",himg_redshift);
}

// Shows the location dialog
int guilocation_show(Ihandle *ih){
	if( !dialog_location ){
		_location_create();
	}
	if( dialog_location==NULL ){
		LOG(LOGERR,_("Failed to create location dialog"));
		return IUP_DEFAULT;
	}
	(void)IupPopup(dialog_location,IUP_CENTER,IUP_CENTER);
	IupDestroy( dialog_location );
	dialog_location = NULL;
	(void)guigamma_check(ih);
	if( !guimain_exit_normal() )
		return IUP_CLOSE;
	return IUP_DEFAULT;
}

