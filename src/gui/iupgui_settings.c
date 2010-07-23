#include "common.h"
#include "iup.h"
#include "options.h"
#include "iupgui_main.h"
#include "iupgui_gamma.h"
#include "iupgui_settings.h"

// Settings dialog handles
static Ihandle *dialog_settings=NULL;
static Ihandle *listmethod=NULL;
static Ihandle *label_day=NULL;
static Ihandle *label_night=NULL;
static Ihandle *label_transpeed=NULL;
static Ihandle *val_night=NULL;
static Ihandle *val_day=NULL;
static Ihandle *val_transpeed=NULL;
static Ihandle *chk_min=NULL;
static Ihandle *chk_disable=NULL;
static Ihandle *edt_elev=NULL;

// Settings - temp day changed
static int _val_day_changed(Ihandle *ih){
	int val = IupGetInt(ih,"VALUE");
	int rounded = 100*((int)(val/100.0f));
	IupSetfAttribute(label_day,"TITLE","%d° K",rounded);
	guigamma_set_temp(rounded);
	return IUP_DEFAULT;
}

// Settings - temp night changed
static int _val_night_changed(Ihandle *ih){
	int val = IupGetInt(ih,"VALUE");
	int rounded = 100*((int)(val/100.0f));
	IupSetfAttribute(label_night,"TITLE","%d° K",rounded);
	guigamma_set_temp(rounded);
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
	guigamma_check(ih);
	return IUP_CLOSE;
}

// Setting - save
static int _setting_save(Ihandle *ih){
	int vday = 100*((int)(IupGetInt(val_day,"VALUE")/100.0f));
	int vnight = 100*((int)(IupGetInt(val_night,"VALUE")/100.0f));
	char *method = IupGetAttribute(
			listmethod,IupGetAttribute(listmethod,"VALUE"));
	gamma_method_t oldmethod = opt_get_method();
	char *name_randr=gamma_get_method_name(GAMMA_METHOD_RANDR);
	char *name_vidmode=gamma_get_method_name(GAMMA_METHOD_VIDMODE);
	char *name_wingdi=gamma_get_method_name(GAMMA_METHOD_WINGDI);
	int min = !strcmp(IupGetAttribute(chk_min,"VALUE"),"ON");
	int disable = !strcmp(IupGetAttribute(chk_disable,"VALUE"),"ON");

	LOG(LOGVERBOSE,_("New day temp: %d, new night temp: %d"),vday,vnight);
	if( opt_get_method() != oldmethod ){
		LOG(LOGINFO,_("Gamma method changed to %s"),method);
		gamma_state_free(oldmethod);
		if( !gamma_init_method(opt_get_screen(),opt_get_crtc(),
				opt_get_method())){
			LOG(LOGERR,_("Unable to set new gamma method, reverting..."));
			if(!gamma_init_method(opt_get_screen(),opt_get_crtc(),
					opt_get_method())){
				LOG(LOGERR,_("Unable to revert to old method."));
				guimain_set_exit(RET_FUN_FAILED);
			}
		}
	}
	if( strcmp(method,name_randr) == 0 )
		opt_set_method(GAMMA_METHOD_RANDR);
	else if( strcmp(method,name_vidmode) == 0 )
		opt_set_method(GAMMA_METHOD_VIDMODE);
	else if( strcmp(method,name_wingdi) == 0 )
		opt_set_method(GAMMA_METHOD_WINGDI);
	else
		LOG(LOGERR,_("Unknown method set"));

	opt_set_min(min);
	opt_set_disabled(disable);
	opt_set_temperatures(vday,vnight);
	opt_set_transpeed(IupGetInt(val_transpeed,"VALUE"));
	opt_write_config();
	guigamma_check(ih);
	return IUP_CLOSE;
}

// Create methods selection frame
static Ihandle *_settings_create_methods(void){
	// Number of methods available
	int methods=0;
	char list_count[3];
	Ihandle *vbox_method,*frame_method;

	// Method selection
	listmethod = IupList(NULL);
	IupSetAttribute(listmethod,"DROPDOWN","YES");
	IupSetAttribute(listmethod,"EXPAND","HORIZONTAL");
#ifdef ENABLE_RANDR
	snprintf(list_count,3,"%d",++methods);
	IupSetAttribute(listmethod,list_count,_("RANDR"));
	if( opt_get_method() == GAMMA_METHOD_RANDR )
		IupSetfAttribute(listmethod,"VALUE","%d",methods);
#endif
#ifdef ENABLE_VIDMODE
	snprintf(list_count,3,"%d",++methods);
	IupSetAttribute(listmethod,list_count,_("VidMode"));
	if( opt_get_method() == GAMMA_METHOD_VIDMODE )
		IupSetfAttribute(listmethod,"VALUE","%d",methods);
#endif
#ifdef ENABLE_WINGDI
	snprintf(list_count,3,"%d",++methods);
	IupSetAttribute(listmethod,list_count,_("WinGDI"));
	if( opt_get_method() == GAMMA_METHOD_WINGDI )
		IupSetfAttribute(listmethod,"VALUE","%d",methods);
#endif
	vbox_method = IupVbox(
			listmethod,
			NULL);
	frame_method = IupFrame(vbox_method);
	IupSetAttribute(frame_method,"TITLE","Backend");
	return frame_method;
}

// Create day temp slider frame
static Ihandle *_settings_create_day_temp(void){
	Ihandle *vbox_day, *frame_day;
	// Day temperature
	label_day = IupLabel(NULL);
	IupSetfAttribute(label_day,"TITLE",_("%d° K"),opt_get_temp_day());
	val_day = IupVal(NULL);
	IupSetAttribute(val_day,"EXPAND","HORIZONTAL");
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
	return frame_day;
}

// Create night temp slider frame
static Ihandle *_settings_create_night_temp(void){
	Ihandle *vbox_night,*frame_night;
	// Night temperature
	label_night = IupLabel(NULL);
	IupSetfAttribute(label_night,"TITLE",_("%d° K"),opt_get_temp_night());
	val_night = IupVal(NULL);
	IupSetAttribute(val_night,"EXPAND","HORIZONTAL");
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
	return frame_night;
}

// Create transition speed slider frame
static Ihandle *_settings_create_tran(void){
	Ihandle *vbox_transpeed, *frame_speed;
	// Transition speed
	label_transpeed = IupLabel(NULL);
	IupSetfAttribute(label_transpeed,"TITLE",_("%d° K/s"),opt_get_trans_speed());
	val_transpeed = IupVal(NULL);
	IupSetAttribute(val_transpeed,"EXPAND","HORIZONTAL");
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
	return frame_speed;
}

// Create startup frame
static Ihandle *_settings_create_startup(void){
	Ihandle *frame_startup;
	// Start minimized and/or disabled
	chk_min = IupToggle(_("Start minimized"),NULL);
	IupSetAttribute(chk_min,"EXPAND","HORIZONTAL");
	chk_disable = IupToggle(_("Start disabled"),NULL);
	IupSetAttribute(chk_disable,"EXPAND","HORIZONTAL");
	if(opt_get_min())
		IupSetAttribute(chk_min,"VALUE","ON");
	if(opt_get_disabled())
		IupSetAttribute(chk_disable,"VALUE","ON");
	frame_startup = IupFrame(IupSetAtt(NULL,
					IupVbox(chk_min,chk_disable,NULL),
					"MARGIN","5",NULL)
				);
	IupSetAttribute(frame_startup,"TITLE",_("Startup"));
	return frame_startup;
}

// Create solar elevations frame
static Ihandle *_settings_create_elev(void){
	Ihandle *frame_elev;

	edt_elev = IupSetAtt(NULL,IupText(NULL),
			"EXPAND","YES","MULTILINE","YES",
			"VISIBLELINES","4",
			"SCROLLBAR","VERTICAL",NULL);
	frame_elev=IupFrame(IupSetAtt(NULL,
				edt_elev,
				"MARGIN","5",NULL)
				);

	IupSetAttribute(frame_elev,"TITLE",_("Temperature map"));
	return frame_elev;
}

// Creates settings dialog
static void _settings_create(void){
	Ihandle *frame_method,
			*frame_day,
			*frame_night,
			*frame_speed,
			*frame_startup,
			*frame_elev,

			*button_cancel,
			*button_save,
			*hbox_buttons,
			*vbox_all;
	extern Ihandle *himg_redshift;

	frame_method = _settings_create_methods();
	frame_day = _settings_create_day_temp();
	frame_night = _settings_create_night_temp();
	frame_speed = _settings_create_tran();
	frame_startup = _settings_create_startup();
	frame_elev = _settings_create_elev();
	// Buttons
	button_cancel = IupButton(_("Cancel"),NULL);
	IupSetCallback(button_cancel,"ACTION",(Icallback)_setting_cancel);
	IupSetfAttribute(button_cancel,"MINSIZE","%dx%d",60,24);
	button_save = IupButton(_("Save"),NULL);
	IupSetCallback(button_save,"ACTION",(Icallback)_setting_save);
	IupSetfAttribute(button_save,"MINSIZE","%dx%d",60,24);
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
			frame_startup,
			frame_elev,
			IupFill(),
			hbox_buttons,
			NULL);
	IupSetfAttribute(vbox_all,"NMARGIN","%dx%d",5,5);
	IupSetAttribute(vbox_all,"ALIGNMENT","ARIGHT");

	dialog_settings=IupDialog(vbox_all);
	IupSetAttribute(dialog_settings,"TITLE",_("Settings"));
	IupSetAttribute(dialog_settings,"RASTERSIZE","250x");
	IupSetAttributeHandle(dialog_settings,"ICON",himg_redshift);
}

// Shows the settings dialog
int guisettings_show(Ihandle *ih){
	if( !dialog_settings )
		_settings_create();
	IupPopup(dialog_settings,IUP_CENTER,IUP_CENTER);
	IupDestroy(dialog_settings);
	dialog_settings = NULL;
	if( !guimain_exit_normal() )
		return IUP_CLOSE;
	return IUP_DEFAULT;
}

