#include "common.h"
#include "iup.h"
#include "options.h"
#include "gamma.h"

// Icon loading function
Ihandle *redshift_get_icon(void);
Ihandle *redshift_get_idle_icon(void);

// Main dialog handles
static Ihandle *dialog=NULL;
static Ihandle *infotitle[4]={NULL,NULL,NULL,NULL};
static Ihandle *infovals[4]={NULL,NULL,NULL,NULL};
static Ihandle *icon_app=NULL;
static Ihandle *icon_idle=NULL;

// Settings dialog handles
static Ihandle *dialog_settings=NULL;
static Ihandle *label_day=NULL;
static Ihandle *label_night=NULL;
static Ihandle *label_transpeed=NULL;
static Ihandle *val_night=NULL;
static Ihandle *val_day=NULL;
static Ihandle *val_transpeed=NULL;

// Stored original temperature
int stored_temp=DEFAULT_DAY_TEMP;

// Toggles main dialog (and also "Hide" button callback)
static int _toggle_main_dialog(Ihandle *ih){
	// Single down click
	char *visible=IupGetAttribute(dialog,"VISIBLE");
	if( visible && strcmp(visible,"YES")==0 )
		IupSetAttribute(dialog,"HIDETASKBAR","YES");
	else
		IupSetAttribute(dialog,"HIDETASKBAR","NO");
	return IUP_DEFAULT;
}

// Updates info display
static void _update_info(void){
	IupSetfAttribute(infovals[0],"TITLE",_("%d° K"),
		gamma_state_get_temperature(opt_get_method()));
	IupSetfAttribute(infovals[1],"TITLE",_("%d° K"),opt_get_temp_day());
	IupSetfAttribute(infovals[2],"TITLE",_("%d° K"),opt_get_temp_night());
	IupSetfAttribute(infovals[3],"TITLE",_("%.2f° Lat, %.2f° Lon"),
			opt_get_lat(),opt_get_lon());
}

// Tray click callback
static int _tray_click(Ihandle *ih, int but, int pressed, int dclick){
	if( but == 1 && pressed )
		_toggle_main_dialog(ih);
	return IUP_DEFAULT;
}

// Settings - temp day changed
static int _val_day_changed(Ihandle *ih){
	int val = IupGetInt(ih,"VALUE");
	int rounded = 100*((int)(val/100.0f));
	IupSetfAttribute(label_day,"TITLE","%d° K",rounded);
	gamma_state_set_temperature(opt_get_method(),rounded,opt_get_gamma());
	return IUP_DEFAULT;
}

// Settings - temp night changed
static int _val_night_changed(Ihandle *ih){
	int val = IupGetInt(ih,"VALUE");
	int rounded = 100*((int)(val/100.0f));
	IupSetfAttribute(label_night,"TITLE","%d° K",rounded);
	gamma_state_set_temperature(opt_get_method(),rounded,opt_get_gamma());
	return IUP_DEFAULT;
}

// Settings - transition speed changed
static int _val_speed_changed(Ihandle *ih){
	int val = IupGetInt(ih,"VALUE");
	IupSetfAttribute(label_transpeed,"TITLE","%d° K/s",val);
	return IUP_DEFAULT;
}

// Setting - cancel
static int _setting_cancel(Ihandle *ih){
	gamma_state_set_temperature(opt_get_method(),stored_temp,opt_get_gamma());
	return IUP_CLOSE;
}

// Setting - save
static int _setting_save(Ihandle *ih){
	int vday = 100*((int)(IupGetInt(val_day,"VALUE")/100.0f));
	int vnight = 100*((int)(IupGetInt(val_night,"VALUE")/100.0f));

	opt_set_temperatures(vday,vnight);
	opt_set_transpeed(IupGetInt(val_transpeed,"VALUE"));
	gamma_state_set_temperature(opt_get_method(),stored_temp,opt_get_gamma());
	return IUP_CLOSE;
}

// Shows the settings dialog
static int _show_settings(Ihandle *ih){
	Ihandle *listmethod,
			*vbox_method,
			*frame_method,

			*vbox_day,
			*frame_day,

			*vbox_night,
			*frame_night,

			*vbox_transpeed,
			*frame_speed,

			*button_cancel,
			*button_save,
			*hbox_buttons,
			*vbox_all;
	
	// Store temperature
	stored_temp = gamma_state_get_temperature(opt_get_method());

	// Method selection
	listmethod = IupList(NULL);
	IupSetAttribute(listmethod,"DROPDOWN","YES");
	IupSetAttribute(listmethod,"EXPAND","YES");
	vbox_method = IupVbox(
			listmethod,
			NULL);
	frame_method = IupFrame(vbox_method);
	IupSetAttribute(frame_method,"TITLE","Backend");

	// Day temperature
	label_day = IupLabel(NULL);
	IupSetfAttribute(label_day,"TITLE",_("%d° K"),opt_get_temp_day());
	val_day = IupVal(NULL);
	IupSetAttribute(val_day,"EXPAND","YES");
	IupSetfAttribute(val_day,"MIN","%d",MIN_TEMP);
	IupSetfAttribute(val_day,"MAX","%d",MAX_TEMP);
	IupSetfAttribute(val_day,"VALUE","%d",opt_get_temp_day());
	IupSetCallback(val_day,"VALUECHANGED_CB",(Icallback)_val_day_changed);
	vbox_day = IupVbox(
			label_day,
			val_day,
			NULL);
	IupSetAttribute(vbox_day,"MARGIN","5");
	frame_day = IupFrame(vbox_day);
	IupSetAttribute(frame_day,"TITLE",_("Day Temperature"));

	// Night temperature
	label_night = IupLabel(NULL);
	IupSetfAttribute(label_night,"TITLE",_("%d° K"),opt_get_temp_night());
	val_night = IupVal(NULL);
	IupSetAttribute(val_night,"EXPAND","YES");
	IupSetfAttribute(val_night,"MIN","%d",MIN_TEMP);
	IupSetfAttribute(val_night,"MAX","%d",MAX_TEMP);
	IupSetfAttribute(val_night,"VALUE","%d",opt_get_temp_night());
	IupSetCallback(val_night,"VALUECHANGED_CB",(Icallback)_val_night_changed);
	vbox_night = IupVbox(
			label_night,
			val_night,
			NULL);
	IupSetAttribute(vbox_night,"MARGIN","5");
	frame_night = IupFrame(vbox_night);
	IupSetAttribute(frame_night,"TITLE","Night Temperature");

	// Transition speed
	label_transpeed = IupLabel(NULL);
	IupSetfAttribute(label_transpeed,"TITLE",_("%d° K/s"),opt_get_trans_speed());
	val_transpeed = IupVal(NULL);
	IupSetAttribute(val_transpeed,"EXPAND","YES");
	IupSetfAttribute(val_transpeed,"MIN","%d",MIN_SPEED);
	IupSetfAttribute(val_transpeed,"MAX","%d",MAX_SPEED);
	IupSetfAttribute(val_transpeed,"VALUE","%d",opt_get_trans_speed());
	IupSetCallback(val_transpeed,"VALUECHANGED_CB",(Icallback)_val_speed_changed);
	vbox_transpeed = IupVbox(
			label_transpeed,
			val_transpeed,
			NULL);
	IupSetAttribute(vbox_transpeed,"MARGIN","5");
	frame_speed = IupFrame(vbox_transpeed);
	IupSetAttribute(frame_speed,"TITLE","Transition Speed");

	// Buttons
	button_cancel = IupButton(_("Cancel"),NULL);
	IupSetfAttribute(button_cancel,"MINSIZE","%dx%d",60,24);
	IupSetCallback(button_cancel,"ACTION",(Icallback)_setting_cancel);
	button_save = IupButton(_("Save"),NULL);
	IupSetfAttribute(button_save,"MINSIZE","%dx%d",60,24);
	IupSetCallback(button_save,"ACTION",(Icallback)_setting_save);
	hbox_buttons = IupHbox(
			button_cancel,
			button_save,
			NULL);

	// Box containing all
	vbox_all = IupVbox(
			frame_method,
			frame_day,
			frame_night,
			frame_speed,
			hbox_buttons,
			NULL);
	IupSetfAttribute(vbox_all,"NMARGIN","%dx%d",5,5);
	IupSetAttribute(vbox_all,"ALIGNMENT","ARIGHT");

	dialog_settings=IupDialog(vbox_all);
	IupSetAttribute(dialog_settings,"TITLE",_("Settings"));
	IupSetfAttribute(dialog_settings,"RASTERSIZE","%dx%d",200,300);
	IupSetAttributeHandle(dialog_settings,"ICON",icon_app);
	IupPopup(dialog_settings,IUP_CENTER,IUP_CENTER);

	return IUP_DEFAULT;
}

// Main dialog
static void _init_main_dialog( int show ){
	Ihandle *hbox_butt,
			*button_loc,
			*button_setting,
			*button_hide,
			*framesun,
			*fvboxtitle,
			*fvboxinfo,
			*frameinfo,
			*dhbox,
			*dvbox;
	// Buttons
	// -Location
	button_loc = IupButton(_("Location"),NULL);
	IupSetfAttribute(button_loc,"MINSIZE","%dx%d",60,24);
	// -Settings
	button_setting = IupButton(_("Settings"),NULL);
	IupSetfAttribute(button_setting,"MINSIZE","%dx%d",60,24);
	IupSetCallback(button_setting,"ACTION",(Icallback)_show_settings);
	// -Hide to tray
	button_hide = IupButton(_("Hide"),NULL);
	IupSetfAttribute(button_hide,"MINSIZE","%dx%d",60,24);
	IupSetCallback(button_hide,"ACTION",(Icallback)_toggle_main_dialog);
	hbox_butt = IupHbox(button_loc,
			button_setting,
			button_hide,
			NULL);

	// Create frame containing the sun control
	framesun = IupFrame(NULL);
	// Info display
	infotitle[0]=IupLabel(_("Current:"));
	infotitle[1]=IupLabel(_("Day:"));
	infotitle[2]=IupLabel(_("Night:"));
	infotitle[3]=IupLabel(_("Location:"));
	infovals[0]=IupLabel(NULL);
	infovals[1]=IupLabel(NULL);
	infovals[2]=IupLabel(NULL);
	infovals[3]=IupLabel(NULL);
	_update_info();
	fvboxtitle= IupVbox(
		infotitle[0],
		infotitle[1],
		infotitle[2],
		infotitle[3],
		IupFill(),
		NULL);
	IupSetAttribute(fvboxtitle,"MARGIN","5");
	fvboxinfo= IupVbox(
		infovals[0],
		infovals[1],
		infovals[2],
		infovals[3],
		IupFill(),
		NULL);
	IupSetAttribute(fvboxinfo,"MARGIN","5");
	frameinfo= IupFrame(
			IupHbox(fvboxtitle,fvboxinfo,NULL));
	IupSetAttribute(frameinfo,"TITLE","Status");

	// Layout box for sun and info
	dhbox = IupHbox(framesun,frameinfo,NULL);

	// Main layout box
	dvbox = IupVbox(
			dhbox,
			hbox_butt,
			NULL
			);
	IupSetAttribute(dvbox,"ALIGNMENT","ARIGHT");
	IupSetfAttribute(dvbox,"NMARGIN","%dx%d",5,5);

	// Create main dialog
	dialog = IupDialog(dvbox);
	IupSetAttribute(dialog,"TITLE",_("Redshift GUI"));
	IupSetfAttribute(dialog,"RASTERSIZE","%dx%d",350,180);
	IupSetfAttribute(dialog,"MINSIZE","%dx%d",350,180);
	IupSetAttribute(dialog,"TRAY","YES");
	IupSetAttribute(dialog,"TRAYTIP","Redshift GUI");

	// Load the icons
	icon_app = redshift_get_icon();
	icon_idle = redshift_get_idle_icon();
	IupSetAttributeHandle(dialog,"ICON",icon_app);
	IupSetAttributeHandle(dialog,"TRAYIMAGE",icon_app);
	IupSetCallback(dialog,"TRAYCLICK_CB",(Icallback)_tray_click);

	if( show )
		IupShowXY(dialog,IUP_RIGHT,IUP_BOTTOM);
}

int iup_gui(int argc, char *argv[]){
	IupOpen( &argc,&argv );
	_init_main_dialog(1);
	IupMainLoop();
	IupClose();
	return RET_FUN_SUCCESS;
}
