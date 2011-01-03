#include "common.h"
#include "systemtime.h"
#include "solar.h"
#include "gamma.h"
#include "options.h"
#include "gui/iupgui.h"
#include "gui/iupgui_main.h"
#include "gui/iupgui_settings.h"
#include "gui/iupgui_location.h"
#include "gui/iupgui_gamma.h"

extern int dim_back_w, dim_back_h, dim_sun_w, dim_sun_h;
extern Hcntrl himg_redshift_idle;
extern Hcntrl himg_redshift;
extern Hcntrl himg_sunback,himg_sun;

// Main dialog handles
static Hnullc dialog=NULL;
static Hnullc infotitle[4]={NULL,NULL,NULL,NULL};
static Hnullc infovals[4]={NULL,NULL,NULL,NULL};
static Hnullc lbl_elevation=NULL;
static Hnullc chk_color=NULL;
static Hnullc chk_bright=NULL;
static Hnullc val_manual=NULL;
static Hnullc val_bright=NULL;
static Hnullc lbl_sun=NULL;
static Hnullc btn_preview=NULL;

// exit status
static int exit_stat=RET_FUN_SUCCESS;

// Sets exit status
int guimain_set_exit(int exit){
	exit_stat = exit;
	return exit_stat;
}

// Gets exit status
int guimain_exit_normal(void){
	return exit_stat;
}

// Sets sun position
static void _set_sun_pos(double elevation){
	double sunx,suny,x,y;
	char *phase;
	/* Position of center of sun relative to (0,0) */
	sunx = cos(RAD(elevation))*(double)(dim_back_w/2-dim_sun_w/2);
	suny = -sin(RAD(elevation))*(double)(dim_back_h/2-dim_sun_h/2);
	LOG(LOGVERBOSE,_("Backdrop dims: %dx%d, sun dims: %dx%d"),
			dim_back_w,dim_back_h,dim_sun_w,dim_sun_h);
	/* Offset sun image by center of background and dimension of image */
	x = (dim_back_w/2)+sunx-dim_sun_w/2;
	y = (dim_back_h/2)+suny-dim_sun_h/2;
	LOG(LOGVERBOSE,_("Sun coords: %.2fx%.2f"),x,y);
	IupSetfAttribute(lbl_sun,"CX","%d",(int)x);
	IupSetfAttribute(lbl_sun,"CY","%d",(int)y);
	IupSetAttribute(lbl_sun,"ZORDER","TOP");
	if( elevation > 0 )
		phase="Day";
	else
		phase="Night";
	IupSetfAttribute(lbl_elevation,"TITLE",_("%s: %.1f"),phase,elevation);
	if( lbl_sun!=NULL )
		IupRefresh(lbl_sun);
}

// Timer function
static double preview_start;
static double currelev;
static int preview_cnt;
static int _preview_timer(Hcntrl ih){
	const double step=1.0;
	int currtemp;
	currelev-=step;
	++preview_cnt;
	if(currelev<SOLAR_MIN_ANGLE)
		currelev+=360;
	currtemp = gamma_calc_temp(currelev,opt_get_temp_day(),opt_get_temp_night());
	LOG(LOGINFO,_("Elevation: %f -> %d"),currelev,currtemp);
	(void)guigamma_set_temp(currtemp);
	_set_sun_pos(currelev);
	IupSetfAttribute(infovals[0],"TITLE",_("%d K"),guigamma_get_temp());
	if( (double)preview_cnt >= 360/step ){
		(void)guigamma_check(ih);
		IupSetAttribute(ih,"RUN","NO");
		IupSetAttribute(btn_preview,"VISIBLE","YES");
	}
	return IUP_DEFAULT;
}

// Preview mode
static int _main_preview(/*@unused@*/ Hcntrl ih){
	double now;
	static Hcntrl timer_prev=NULL;
	if( !timer_prev)
		timer_prev = IupTimer();
	LOG(LOGINFO,_("Previewing cycle"));
	if ( !systemtime_get_time(&now) ){
		LOG(LOGERR,_("Unable to read system time."));
		return IUP_DEFAULT;
	}
	preview_start = solar_elevation(now,opt_get_lat(),
			opt_get_lon());
	currelev = preview_start;
	preview_cnt=0;
	IupSetAttribute(btn_preview,"VISIBLE","NO");
	(void)IupSetCallback(timer_prev,"ACTION_CB",(Icallback)_preview_timer);
	IupSetAttribute(timer_prev,"TIME","20");
	IupSetAttribute(timer_prev,"RUN","YES");
	return IUP_DEFAULT;
}

// Toggles manual override
static int _toggle_manual(Hcntrl ih, int state){
	if( dialog==NULL ){
		LOG(LOGERR,_("Fatal error, dialog does not exist!"));
		return IUP_DEFAULT;
	}
	if( state ){
		guigamma_disable();
		IupSetAttribute(val_manual,"VISIBLE","YES");
		IupSetfAttribute(val_manual,"VALUE","%d",guigamma_get_temp());
		IupSetAttributeHandle(dialog,"TRAYIMAGE",himg_redshift_idle);
	}else{
		guigamma_enable();
		(void)guigamma_check(ih);
		IupSetAttribute(val_manual,"VISIBLE","OFF");
		IupSetAttributeHandle(dialog,"TRAYIMAGE",himg_redshift);
	}
	return IUP_DEFAULT;
}

// Toggles brightness
static int _toggle_bright(Hcntrl ih, int state){
	if( dialog==NULL ){
		LOG(LOGERR,_("Fatal error, dialog does not exist!"));
		return IUP_DEFAULT;
	}
	if( state ){
		IupSetAttribute(val_bright,"VISIBLE","YES");
		//IupSetfAttribute(val_manual,"VALUE","%d",guigamma_get_temp());
	}else{
		IupSetAttribute(val_bright,"VISIBLE","OFF");
	}
	return IUP_DEFAULT;
}

// Change temperature manually
static int _manual_temp(Hcntrl ih){
	int val = IupGetInt(ih,"VALUE");
	int rounded = 100*((int)(val/100.0f));
	LOG(LOGVERBOSE,_("Setting manual temperature: %d"),rounded);
	(void)guigamma_set_temp(rounded);
	guimain_update_info();
	return IUP_DEFAULT;
}

// Change brightness
static int _bright(Hcntrl ih){
	float val = IupGetFloat(ih,"VALUE");
	LOG(LOGVERBOSE,_("Setting brightness: %f"),val);
	(void)opt_set_brightness(val);
	(void)guigamma_set_temp(guigamma_get_temp());
	return IUP_DEFAULT;
}

// Toggles main dialog (and also "Hide" button callback)
static int _toggle_main_dialog(/*@unused@*/ Hcntrl ih){
	// If dialog needs to be positioned
	static int positioned=0;
	// Single down click
	char *visible;
	if( dialog==NULL ){
		LOG(LOGERR,_("Fatal error, dialog handle not defined."));
		return IUP_DEFAULT;
	}

	visible=IupGetAttribute(dialog,"VISIBLE");
	if( (visible!=NULL) && strcmp(visible,"YES")==0 )
		IupSetAttribute(dialog,"HIDETASKBAR","YES");
	else{
		if(!positioned){
			char *currsize=IupGetAttribute(dialog,"RASTERSIZE");
			positioned=1;
			(void)IupShowXY(dialog,IUP_RIGHT,IUP_BOTTOM);
			if( currsize!=NULL )
				IupSetAttribute(dialog,"MINSIZE",currsize);
			IupRefresh(dialog);
		}else
			IupSetAttribute(dialog,"HIDETASKBAR","NO");
		guimain_update_info();
	}
	return IUP_DEFAULT;
}

// Tray click callback
static int _tray_click(Hcntrl ih, int but, int pressed,
		/*@unused@*/ int dclick)
{
	// static Hcntrl menu_tray=NULL;
	switch (but){
		case 1:
			if( pressed )
				(void)_toggle_main_dialog(ih);
			break;
		default:
			if( pressed ){
				int state;
				char *val=IupGetAttribute(chk_color,"VALUE");
				if( (chk_color==NULL)||(val==NULL) ){
					LOG(LOGERR,_("Checkbox undefined handle."));
					return IUP_DEFAULT;
				}
				
				state = strcmp(val,"ON");
				if( state )
					IupSetAttribute(chk_color,"VALUE","ON");
				else
					IupSetAttribute(chk_color,"VALUE","OFF");
				(void)_toggle_manual(ih,state);
				//// Bring up menu
				//if( !menu_tray ){
				//	Hcntrl mitem_toggle,
				//			*mitem_settings,
				//			*mitem_about;
				//	mitem_toggle = IupItem(_("Hide/Show"),NULL);
				//	IupSetCallback(mitem_toggle,"ACTION",_toggle_main_dialog);
				//	mitem_settings = IupItem(_("Settings"),NULL);
				//	IupSetCallback(mitem_settings,"ACTION",guisettings_show);
				//	mitem_about = IupItem(_("About"),NULL);
				//	IupSetCallback(mitem_about,"ACTION",_show_about);
				//	menu_tray = IupMenu(
				//			mitem_toggle,
				//			mitem_settings,
				//			IupSeparator(),
				//			mitem_about,
				//			NULL);
				//	IupMap(menu_tray);
				//}
				//IupPopup(menu_tray,IUP_MOUSEPOS,IUP_MOUSEPOS);
				//// Need a workaround on GTK2 because MOUSEPOS doesn't
				//// seem to work on lower bar
				//IupDestroy(menu_tray);
				//menu_tray = NULL;
			}
			break;
	}
	return IUP_DEFAULT;
}

// Updates info display
void guimain_update_info(void){
	double now;
	if ( !systemtime_get_time(&now) ){
		LOG(LOGERR,_("Unable to read system time."));
	}else{
		float lat=opt_get_lat();
		float lon=opt_get_lon();
		double elevation;
		/* Current angular elevation of the sun */
		elevation = solar_elevation(now,lat,lon);
		LOG(LOGVERBOSE,_("Elevation - now: %f"),
				elevation);
		_set_sun_pos(elevation);
	}

	IupSetfAttribute(infovals[0],"TITLE",_("%d K"),guigamma_get_temp());
	IupSetfAttribute(infovals[1],"TITLE",_("%d K"),opt_get_temp_day());
	IupSetfAttribute(infovals[2],"TITLE",_("%d K"),opt_get_temp_night());
	IupSetfAttribute(infovals[3],"TITLE",_("%.2f Lat, %.2f Lon"),
			opt_get_lat(),opt_get_lon());
}


// Icon loading function
extern Hcntrl redshift_get_icon(void);
extern Hcntrl redshift_get_idle_icon(void);

// Create sun frame
static Hcntrl _main_create_sun(void){
	Hcntrl lbl_backsun,cbox_sun,framesun;
	// Create Sun control
	lbl_backsun = IupLabel(NULL);
	IupSetAttributeHandle(lbl_backsun,"IMAGE",himg_sunback);
	IupSetAttribute(lbl_backsun,"CX","0");
	IupSetAttribute(lbl_backsun,"CY","0");
	lbl_sun = IupLabel(NULL);
	IupSetAttributeHandle(lbl_sun,"IMAGE",himg_sun);
	IupSetfAttribute(lbl_sun,"CX","%d",0);
	IupSetfAttribute(lbl_sun,"CY","%d",0);
	// Preview
	btn_preview = IupButton(_("Preview"),NULL);
	IupSetfAttribute(btn_preview,"MINSIZE","%dx%d",60,24);
	(void)IupSetCallback(btn_preview,"ACTION",(Icallback)_main_preview);
	cbox_sun = IupCbox(
			lbl_backsun,
			lbl_sun,
			NULL);
	lbl_elevation = IupLabel(_("N/A: 0"));
	// Create frame containing the sun control
	framesun = IupFrame(IupSetAttributes(
			IupVbox(cbox_sun,
				IupFill(),
				lbl_elevation,
				btn_preview,
				NULL),"MARGIN=5"));
	IupSetAttribute(framesun,"TITLE",_("Sun elevation"));
	return framesun;
}

// Create info frame
static Hcntrl _main_create_info(void){
	Hcntrl  frameinfo,
			fvboxtitle,
			fvboxinfo;
	// Info display
	infotitle[0]=IupLabel(_("Current:"));
	infotitle[1]=IupLabel(_("Day:"));
	infotitle[2]=IupLabel(_("Night:"));
	infotitle[3]=IupLabel(_("Location:"));
	IupSetAttribute(infotitle[0],"EXPAND","HORIZONTAL");
	IupSetAttribute(infotitle[1],"EXPAND","HORIZONTAL");
	IupSetAttribute(infotitle[2],"EXPAND","HORIZONTAL");
	IupSetAttribute(infotitle[3],"EXPAND","HORIZONTAL");
	infovals[0]=IupLabel(NULL);
	infovals[1]=IupLabel(NULL);
	infovals[2]=IupLabel(NULL);
	infovals[3]=IupLabel(NULL);
	guimain_update_info();
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
	IupSetAttribute(frameinfo,"TITLE",_("Status"));
	return frameinfo;
}

// Create manual frame
static Hcntrl _main_create_color(void){
	Hcntrl  vbox_manual,
			framemanual;
	// Manual override
	chk_color = IupSetAtt(NULL,IupToggle(_("Disable auto-adjust"),NULL)
		,"EXPAND","YES",NULL);
	(void)IupSetCallback(chk_color,"ACTION",(Icallback)_toggle_manual);
	val_manual = IupSetAtt(NULL,IupVal(NULL),"MIN","3400","MAX","7000",
		"VISIBLE","NO","EXPAND","HORIZONTAL",NULL);
	(void)IupSetCallback(val_manual,"VALUECHANGED_CB",(Icallback)_manual_temp);
	vbox_manual = IupVbox(chk_color,val_manual,NULL);
	framemanual = IupFrame(vbox_manual);
	IupSetAttribute(framemanual,"TITLE",_("Color"));
	return framemanual;
}

// Create brightness frame
static Hcntrl _main_create_bright(void){
	Hcntrl framebright;
	// Manual override
	chk_bright = IupSetAtt(NULL,IupToggle(_("Disable auto-adjust"),NULL)
		,"EXPAND","YES",NULL);
	(void)IupSetCallback(chk_bright,"ACTION",(Icallback)_toggle_bright);
	// Brightness
	val_bright = IupSetAtt(NULL,IupVal(NULL),"MIN","0.1","MAX","1",
		"VISIBLE","NO","EXPAND","HORIZONTAL",NULL);
	(void)IupSetCallback(val_bright,"VALUECHANGED_CB",(Icallback)_bright);
	framebright = IupFrame(IupVbox(chk_bright,val_bright,NULL));
	IupSetAttribute(framebright,"TITLE",_("Brightness"));
	return framebright;
}

// Main dialog
void guimain_dialog_init(void){
	Hcntrl  hbox_butt,
			button_about,
			button_loc,
			button_setting,
			button_hide,
			framesun,
			frameinfo,
			framemanual,
			framebright,
			dhbox,
			dvbox;

	// Create frames
	framesun = _main_create_sun();
	frameinfo = _main_create_info();
	framemanual = _main_create_color();
	framebright = _main_create_bright();

	// Buttons
	// -About
	button_about = IupButton(_("About"),NULL);
	IupSetfAttribute(button_about,"MINSIZE","%dx%d",60,24);
	(void)IupSetCallback(button_about,"ACTION",(Icallback)gui_about);
	// -Location
	button_loc = IupButton(_("Location"),NULL);
	IupSetfAttribute(button_loc,"MINSIZE","%dx%d",60,24);
	(void)IupSetCallback(button_loc,"ACTION",(Icallback)guilocation_show);
	// -Settings
	button_setting = IupButton(_("Settings"),NULL);
	IupSetfAttribute(button_setting,"MINSIZE","%dx%d",60,24);
	(void)IupSetCallback(button_setting,"ACTION",(Icallback)guisettings_show);
	// -Hide to tray
	button_hide = IupButton(_("Hide"),NULL);
	IupSetfAttribute(button_hide,"MINSIZE","%dx%d",60,24);
	(void)IupSetCallback(button_hide,"ACTION",(Icallback)_toggle_main_dialog);
	hbox_butt = IupHbox(
			button_about,
			IupFill(),
			button_loc,
			button_setting,
			button_hide,
			NULL);

	// Layout box for sun and info
	dhbox = IupHbox(framesun,IupFill(),
		IupVbox(frameinfo,framemanual,framebright,NULL),NULL);

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
	IupSetAttribute(dialog,"RESIZE","NO");
	IupSetAttribute(dialog,"MAXBOX","NO");
	IupSetAttribute(dialog,"TRAY","YES");
	IupSetAttribute(dialog,"TRAYTIP","Redshift GUI");

	IupSetAttributeHandle(dialog,"ICON",himg_redshift);
	IupSetAttributeHandle(dialog,"TRAYIMAGE",himg_redshift);
	(void)IupSetCallback(dialog,"TRAYCLICK_CB",(Icallback)_tray_click);

	(void)IupMap(dialog);
	if( opt_get_min() )
		IupSetAttribute(dialog,"HIDETASKBAR","YES");
	else
		(void)_toggle_main_dialog(dialog);

	if( opt_get_disabled() ){
		if( chk_color!=NULL){
			IupSetAttribute(chk_color,"VALUE","ON");
			(void)_toggle_manual(chk_color,1);
		}
	}
}

