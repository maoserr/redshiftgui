#include "common.h"
#include "gamma.h"
#include "options.h"
#include "gui/iupgui.h"
#include "gui/iupgui_main.h"
#include "gui/iupgui_gamma.h"
#include "gui/iupgui_settings.h"

extern Hcntrl himg_redshift,himg_redshift_idle;

// Settings dialog handles
static Hnullc dialog_settings=NULL;
static Hnullc listmethod=NULL;
static Hnullc label_day=NULL;
static Hnullc label_night=NULL;
static Hnullc label_transpeed=NULL;
static Hnullc val_night=NULL;
static Hnullc val_day=NULL;
static Hnullc val_transpeed=NULL;
static Hnullc chk_min=NULL;
static Hnullc chk_disable=NULL;
static Hnullc edt_elev=NULL;

// Buffer to hold elevation map value
/*@owned@*//*@null@*/ static char *txt_val=NULL;

// Retrieves an image file, this function allocates memory.
// Caller must free() returned string.
static /*@null@*/ char *_get_image_file(char *initial_file){
	char *file;
	char *val;
	Hcntrl filedlg = IupFileDlg();
	IupSetAttribute(filedlg,"FILE",initial_file);
	IupSetAttribute(filedlg,"FILTER","*.png");
	IupSetAttribute(filedlg,"TITLE",_("Select image"));
	(void)IupPopup(filedlg,IUP_CENTER,IUP_CENTER);
	if( IupGetInt(filedlg, "STATUS")==0 ){
		val=IupGetAttribute(filedlg,"VALUE");
		if( val==NULL )
			return NULL;
		file = malloc((strlen(val)+1)*sizeof(char));
		if( file==NULL )
			return NULL;
		strcpy(file,val);
		return file;
	}else
		return NULL;
}

// Settings - temp day changed
static int _val_day_changed(Hcntrl ih){
	int val = IupGetInt(ih,"VALUE");
	int rounded = 100*((int)(val/100.0f));
	IupSetfAttribute(label_day,"TITLE","%d K",rounded);
	(void)guigamma_set_temp(rounded);
	return IUP_DEFAULT;
}

// Settings - temp night changed
static int _val_night_changed(Hcntrl ih){
	int val = IupGetInt(ih,"VALUE");
	int rounded = 100*((int)(val/100.0f));
	IupSetfAttribute(label_night,"TITLE","%d� K",rounded);
	(void)guigamma_set_temp(rounded);
	return IUP_DEFAULT;
}

// Settings - active icon changed
static int _set_active(Hcntrl ih){
	char *file=_get_image_file(_(""));
	if( file != NULL ){
		LOG(LOGVERBOSE,_("New active image: %s"),file);
		IupSetAttribute(ih,"IMAGE",file);
		free(file);
	}
	return IUP_DEFAULT;
}

// Settings - idle icon changed
static int _set_idle(Hcntrl ih){
	char *file=_get_image_file(_(""));
	if( file != NULL ){
		LOG(LOGVERBOSE,_("New idle image: %s"),file);
		IupSetAttribute(ih,"IMAGE",file);
		free(file);
	}
	return IUP_DEFAULT;
}

// Settings - transition speed changed
static int _val_speed_changed(Hcntrl ih){
	int val = IupGetInt(ih,"VALUE");
	IupSetfAttribute(label_transpeed,"TITLE","%d K/s",val);
	return IUP_DEFAULT;
}

// Setting - cancel
static int _setting_cancel(/*@unused@*/ Hcntrl ih){
	return IUP_CLOSE;
}

// Setting - save
static int _setting_save(/*@unused@*/ Hcntrl ih){
	int vday,vnight;
	char *method;
	gamma_method_t oldmethod = opt_get_method();
	gamma_method_t newmethod;
	int min=0;
	int disable=0;
	char *min_str,*dis_str,*elev_map,*method_cnt;
	int methodsuccess=0;

	if( (val_day==NULL)
			||(val_night==NULL)
			||(listmethod==NULL)
			||(chk_min==NULL)
			||(chk_disable==NULL)
			||(edt_elev==NULL)
			||(val_transpeed==NULL) )
	{
		LOG(LOGERR,_("Fatal error: handles not created."));
		return IUP_DEFAULT;
	}
	dis_str = IupGetAttribute(chk_disable,"VALUE");
	method_cnt = IupGetAttribute(listmethod,"VALUE");
	min_str = IupGetAttribute(chk_min,"VALUE");
	vday = 100*((int)(IupGetInt(val_day,"VALUE")/100.0f));
	vnight = 100*((int)(IupGetInt(val_night,"VALUE")/100.0f));
	elev_map = IupGetAttribute(edt_elev,"VALUE");

	if( method_cnt!=NULL )
		method = IupGetAttribute(listmethod,method_cnt);
	else
		method=NULL;
	if( min_str!=NULL )
		min = (strcmp(min_str,"ON")==0);
	if( dis_str!=NULL )
		disable = (strcmp(dis_str,"ON")==0);

	LOG(LOGVERBOSE,_("New day temp: %d, new night temp: %d"),vday,vnight);
	if( method!=NULL ){
		if( !(newmethod=gamma_lookup_method(method)) ){
			LOG(LOGERR,_("Invalid method selected"));
		}else{
			if( newmethod != oldmethod ){
				LOG(LOGINFO,_("Gamma method changed to %s"),method);
				(void)gamma_state_free();
				if( !gamma_init_method(opt_get_screen(),opt_get_crtc(),
						newmethod)){
					LOG(LOGERR,_("Unable to set new gamma method, reverting..."));
					if(!gamma_init_method(opt_get_screen(),opt_get_crtc(),
							oldmethod)){
						LOG(LOGERR,_("Unable to revert to old method."));
					}
				}else
					methodsuccess=1;
			}else
				methodsuccess=1;
		}
		if( !methodsuccess ){
			(void)gui_popup(_("Error"),
					_("There was an error setting the new method.\nSettings NOT saved."),
					"ERROR");
			return IUP_CLOSE;
		}
		(void)opt_set_method(newmethod);
	}else{
		LOG(LOGERR,_("Method selections invalid."));
	}

	if( (elev_map!=NULL)&&(!opt_parse_map(elev_map)) ){
		(void)gui_popup(_("Error"),
				_("Unable to parse new temperature map,\nPlease try again."),
				"ERROR");
		return IUP_DEFAULT;
	}
	(void)opt_set_min(min);
	(void)opt_set_disabled(disable);
	(void)opt_set_temperatures(vday,vnight);
	(void)opt_set_transpeed(IupGetInt(val_transpeed,"VALUE"));
	opt_write_config();
	return IUP_CLOSE;
}

// Create methods selection frame
static Hcntrl _settings_create_methods(void){
	// Number of methods available
	int avail_methods=0;
	char list_count[3];
	gamma_method_t method;
	char *method_name;
	Hcntrl vbox_method,frame_method;

	// Method selection
	listmethod = IupList(NULL);
	IupSetAttribute(listmethod,"DROPDOWN","YES");
	IupSetAttribute(listmethod,"EXPAND","HORIZONTAL");
	for( method=GAMMA_METHOD_AUTO; method<GAMMA_METHOD_MAX; ++method){
		method_name = gamma_get_method_name(method);
		if( (strcmp(method_name,"None")!=0) ){
			(void)snprintf(list_count,3,"%d",++avail_methods);
			IupSetAttribute(listmethod,list_count,method_name);
			if( opt_get_method() == method )
				IupSetfAttribute(listmethod,"VALUE","%d",avail_methods);
		}
	}
	vbox_method = IupVbox(
			listmethod,
			NULL);
	frame_method = IupFrame(vbox_method);
	IupSetAttribute(frame_method,"TITLE","Backend");
	return frame_method;
}

// Create day temp slider frame
static Hcntrl _settings_create_day_temp(void){
	Hcntrl vbox_day,frame_day;
	// Day temperature
	label_day = IupLabel(NULL);
	IupSetfAttribute(label_day,"TITLE",_("%d K"),opt_get_temp_day());
	val_day = IupVal(NULL);
	IupSetAttribute(val_day,"EXPAND","HORIZONTAL");
	IupSetfAttribute(val_day,"MIN","%d",MIN_TEMP);
	IupSetfAttribute(val_day,"MAX","%d",MAX_TEMP);
	IupSetfAttribute(val_day,"VALUE","%d",opt_get_temp_day());
	(void)IupSetCallback(val_day,"VALUECHANGED_CB",(Icallback)_val_day_changed);
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
static Hcntrl _settings_create_night_temp(void){
	Hcntrl vbox_night,frame_night;
	// Night temperature
	label_night = IupLabel(NULL);
	IupSetfAttribute(label_night,"TITLE",_("%d K"),opt_get_temp_night());
	val_night = IupVal(NULL);
	IupSetAttribute(val_night,"EXPAND","HORIZONTAL");
	IupSetfAttribute(val_night,"MIN","%d",MIN_TEMP);
	IupSetfAttribute(val_night,"MAX","%d",MAX_TEMP);
	IupSetfAttribute(val_night,"VALUE","%d",opt_get_temp_night());
	(void)IupSetCallback(val_night,"VALUECHANGED_CB",(Icallback)_val_night_changed);
	vbox_night = IupVbox(
			label_night,
			val_night,
			NULL);
	IupSetAttribute(vbox_night,"MARGIN","5");
	frame_night = IupFrame(vbox_night);
	IupSetAttribute(frame_night,"TITLE","Night Temperature");
	return frame_night;
}

// Create startup frame
static Hcntrl _settings_create_startup(void){
	Hcntrl frame_startup;
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

// Create icons frame
static Hcntrl _settings_create_icons(void){
	Hcntrl frame_icons;
	Hcntrl button_active;
	Hcntrl button_idle;
	Hcntrl label_txt=IupLabel(_("Please use\n32x32 png"));
	
	button_active = IupButton(NULL,NULL);
	IupSetAttributeHandle(button_active,"IMAGE",himg_redshift);
	(void)IupSetCallback(button_active,"ACTION",(Icallback)_set_active);
	button_idle = IupButton(NULL,NULL);
	IupSetAttributeHandle(button_idle,"IMAGE",himg_redshift_idle);
	(void)IupSetCallback(button_idle,"ACTION",(Icallback)_set_idle);
	frame_icons = IupFrame(
			IupSetAttributes(
				IupHbox(button_active,button_idle,label_txt,IupFill(),NULL),
				"MARGIN=5")
		);
	IupSetAttribute(frame_icons,"TITLE",_("Icons"));
	return frame_icons;
}

// Create events frame
static Hcntrl _settings_create_events(void){
	Hcntrl frame_events;
	Hcntrl edt_command;
	Hcntrl label_txt=IupLabel(_("Custom command on temperature change:"));

	edt_command = IupText(NULL);
	frame_events = IupFrame(
			IupSetAttributes(
				IupVbox(label_txt,edt_command,NULL),
				"MARGIN=5")
		);
	IupSetAttribute(frame_events,"TITLE",_("Events"));
	return frame_events;
}

// Create weather frame
static Hcntrl _settings_create_weather(void){
	Hcntrl frame_weather;
	Hcntrl chk_enable;
	Hcntrl spn_interval;
	Hcntrl label_spn=IupLabel(_("Update interval (minutes)"));
	Hcntrl label_txt=IupLabel(_("Weather based on lat/lon."));

	chk_enable = IupToggle(_("Enable"),NULL);
	spn_interval = IupSetAttributes(IupText(NULL),
			"VISIBLE=NO,SPIN=YES,SPINVALUE=60,"
			"SPINMIN=10,SPINMAX=180");
	frame_weather = IupFrame(
			IupSetAttributes(
				IupVbox(label_txt,chk_enable,
					IupHbox(label_spn,spn_interval,NULL),NULL),
				"MARGIN=5")
			);
	IupSetAttribute(frame_weather,"TITLE",_("Weather"));
	return frame_weather;
}

// Create logging frame
static Hcntrl _settings_create_logging(void){

}

// Create gamma frame

// Create additional gamma frame

// Create transition speed slider frame
static Hcntrl _settings_create_tran(void){
	Hcntrl vbox_transpeed, frame_speed;
	// Transition speed
	label_transpeed = IupLabel(NULL);
	IupSetfAttribute(label_transpeed,"TITLE",_("%d K/s"),opt_get_trans_speed());
	val_transpeed = IupVal(NULL);
	IupSetAttribute(val_transpeed,"EXPAND","HORIZONTAL");
	IupSetfAttribute(val_transpeed,"MIN","%d",MIN_SPEED);
	IupSetfAttribute(val_transpeed,"MAX","%d",MAX_SPEED);
	IupSetfAttribute(val_transpeed,"VALUE","%d",opt_get_trans_speed());
	(void)IupSetCallback(val_transpeed,"VALUECHANGED_CB",(Icallback)_val_speed_changed);
	vbox_transpeed = IupVbox(
			label_transpeed,
			val_transpeed,
			NULL);
	IupSetAttribute(vbox_transpeed,"MARGIN","5");
	frame_speed = IupFrame(vbox_transpeed);
	IupSetAttribute(frame_speed,"TITLE","Transition Speed");
	return frame_speed;
}

// Create solar elevations frame
static Hcntrl _settings_create_elev(void){
	Hcntrl lbl_elev,frame_elev;
	int size;
	pair *map = opt_get_map(&size);
	int i;

	// Assume a size of 20 char per line
#define LINE_SIZE 20
	if( txt_val!=NULL )
		free(txt_val);
	txt_val = (char*)malloc(size*sizeof(char)*LINE_SIZE+1);
	if( txt_val==NULL )
		LOG(LOGERR,_("Unable to allocate edit memory"));

	for( i=0; i<size; ++i ){
		// Use up LINE_SIZE of buffer, with NULL terminator being overwritten on
		// next loop
		(void)snprintf(txt_val+LINE_SIZE*i,LINE_SIZE+1,"%9.2f,%7.2f%%;\n",
				map[i].elev,map[i].temp);
	}
	edt_elev = IupSetAtt(NULL,IupText(NULL),
			"EXPAND","YES","MULTILINE","YES",
			"VISIBLELINES","4",
			"SCROLLBAR","VERTICAL",NULL);
	if( txt_val!=NULL )
		/*@i@*/IupSetAttribute(edt_elev,"VALUE",txt_val);

	lbl_elev = IupSetAtt(NULL,
			IupLabel(_("Enter comma separated list of\n"
					"elevation to temperature pairs,\n"
					"in between values are linearly \n"
					"interpolated.\n"
					"(0% - night, 100% - day temp.)")),"WORDWRAP","YES",NULL);
	frame_elev=IupFrame(IupSetAtt(NULL,
				IupVbox(edt_elev,lbl_elev,NULL),
				"MARGIN","5",NULL)
				);

	IupSetAttribute(frame_elev,"TITLE",_("Temperature map"));
	/*@i@*/return frame_elev;
}

// Creates settings dialog
static void _settings_create(void){
	Hcntrl frame_method,
			frame_day,
			frame_night,
			frame_startup,
			frame_icons,
			frame_speed,
			frame_elev,

			tabs_all,

			button_cancel,
			button_save,
			hbox_buttons,
			vbox_all;

	frame_method = _settings_create_methods();
	frame_day = _settings_create_day_temp();
	frame_night = _settings_create_night_temp();
	frame_startup = _settings_create_startup();
	//frame_icons = _settings_create_icons();
	frame_speed = _settings_create_tran();
	frame_elev = _settings_create_elev();

	// Tabs containing settings
	tabs_all = IupTabs(
			IupVbox(
				frame_method,
				frame_day,
				frame_night,
				frame_startup,
				//frame_icons,
				NULL),
			IupVbox(
				frame_speed,
				frame_elev,
				NULL),
			NULL);
	(void)IupSetAttributes(tabs_all,"TABTITLE0=Basic,"
			"TABTITLE1=Transition");

	// Buttons
	button_cancel = IupButton(_("Cancel"),NULL);
	(void)IupSetCallback(button_cancel,"ACTION",(Icallback)_setting_cancel);
	IupSetfAttribute(button_cancel,"MINSIZE","%dx%d",60,24);
	button_save = IupButton(_("Save"),NULL);
	(void)IupSetCallback(button_save,"ACTION",(Icallback)_setting_save);
	IupSetfAttribute(button_save,"MINSIZE","%dx%d",60,24);
	hbox_buttons = IupHbox(
			button_cancel,
			button_save,
			NULL);

	// Box containing all
	vbox_all = IupVbox(
			tabs_all,
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
int guisettings_show(Hcntrl ih){
	if( !dialog_settings )
		_settings_create();
	if( dialog_settings!=NULL ){
		(void)IupPopup(dialog_settings,IUP_CENTER,IUP_CENTER);
		IupDestroy(dialog_settings);
	}
	if( txt_val )
		free(txt_val);
	dialog_settings = NULL;
	txt_val = NULL;
	(void)guigamma_check(ih);
	if( !guimain_exit_normal() )
		return IUP_CLOSE;
	return IUP_DEFAULT;
}

