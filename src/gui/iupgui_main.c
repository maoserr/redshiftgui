#include "common.h"
#include <iup.h>
#include "systemtime.h"
#include "solar.h"
#include "options.h"
#include "iupgui_main.h"
#include "iupgui_settings.h"
#include "iupgui_location.h"
#include "iupgui_gamma.h"

// Main dialog handles
static Ihandle *dialog=NULL;
static Ihandle *infotitle[4]={NULL,NULL,NULL,NULL};
static Ihandle *infovals[4]={NULL,NULL,NULL,NULL};
static Ihandle *lbl_elevation=NULL;
static Ihandle *chk_manual=NULL;
static Ihandle *val_manual=NULL;
static Ihandle *val_bright=NULL;
static Ihandle *lbl_sun=NULL;

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

// Show about dialog
static int _show_about(Ihandle *ih){
	Ihandle *dialog_about = IupSetAtt(NULL,IupMessageDlg(),
			"TITLE",_("About Redshift GUI"),
			"VALUE",
			_("Homepage:\nhttp://www.mao-yu.com/projects/redshiftgui/\n\n"
				"Written by Mao Yu\n"
				"Redshift GUI is based on Redshift by Jon Lund Steffensen\n"
				"This program uses IUP and libcURL\n"
				"Licensed under the GPL license"),NULL);
	IupPopup(dialog_about,IUP_CENTER,IUP_CENTER);
	IupDestroy(dialog_about);
	return IUP_DEFAULT;
}

// Toggles manual override
static int _toggle_manual(Ihandle *ih, int state){
	if( state ){
		extern Ihandle *himg_redshift_idle;
		guigamma_disable();
		IupSetAttribute(val_manual,"VISIBLE","YES");
		IupSetfAttribute(val_manual,"VALUE","%d",guigamma_get_temp());
		IupSetAttributeHandle(dialog,"TRAYIMAGE",himg_redshift_idle);
	}else{
		extern Ihandle *himg_redshift;
		guigamma_enable();
		guigamma_check(ih);
		IupSetAttribute(val_manual,"VISIBLE","OFF");
		IupSetAttributeHandle(dialog,"TRAYIMAGE",himg_redshift);
	}
	return IUP_DEFAULT;
}

// Change temperature manually
static int _manual_temp(Ihandle *ih){
	int val = IupGetInt(ih,"VALUE");
	int rounded = 100*((int)(val/100.0f));
	LOG(LOGVERBOSE,_("Setting manual temperature: %d"),rounded);
	guigamma_set_temp(rounded);
	guimain_update_info();
	return IUP_DEFAULT;
}

// Change brightness
static int _bright(Ihandle *ih){
	float val = IupGetFloat(ih,"VALUE");
	LOG(LOGVERBOSE,_("Setting brightness: %f"),val);
	opt_set_gamma(val,val,val);
	guigamma_set_temp(guigamma_get_temp());
	return IUP_DEFAULT;
}

// Toggles main dialog (and also "Hide" button callback)
static int _toggle_main_dialog(Ihandle *ih){
	// If dialog needs to be positioned
	static int positioned=0;
	// Single down click
	char *visible=IupGetAttribute(dialog,"VISIBLE");
	if( visible && strcmp(visible,"YES")==0 )
		IupSetAttribute(dialog,"HIDETASKBAR","YES");
	else{
		if(!positioned){
			positioned=1;
			IupShowXY(dialog,IUP_RIGHT,IUP_BOTTOM);
		}else
			IupSetAttribute(dialog,"HIDETASKBAR","NO");
		guimain_update_info();
	}
	return IUP_DEFAULT;
}

// Tray click callback
static int _tray_click(Ihandle *ih, int but, int pressed, int dclick){
	// static Ihandle *menu_tray=NULL;
	switch (but){
		case 1:
			if( pressed )
				_toggle_main_dialog(ih);
			break;
		default:
			if( pressed ){
				int state = strcmp(IupGetAttribute(chk_manual,"VALUE"),"ON");
				if( state )
					IupSetAttribute(chk_manual,"VALUE","ON");
				else
					IupSetAttribute(chk_manual,"VALUE","OFF");
				_toggle_manual(ih,state);
				//// Bring up menu
				//if( !menu_tray ){
				//	Ihandle *mitem_toggle,
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
				//// Need a workaround on GTK2 because MOUSEPOS doesn't seem to work on lower bar
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
	extern int dim_back_w, dim_back_h, dim_sun_w, dim_sun_h;
	if ( !systemtime_get_time(&now) ){
		LOG(LOGERR,_("Unable to read system time."));
	}else{
		double sunx,suny,x,y;
		float lat=opt_get_lat();
		float lon=opt_get_lon();
		double elevation, elevation_next;
		/* Current angular elevation of the sun */
		elevation = solar_elevation(now,lat,lon);
		elevation_next = solar_elevation(now+100,lat,lon);
		LOG(LOGVERBOSE,_("Elevation - now: %f, next: %f"),
				elevation,elevation_next);
		LOG(LOGVERBOSE,_("Backdrop dims: %dx%d, sun dims: %dx%d"),
				dim_back_w,dim_back_h,dim_sun_w,dim_sun_h);
		/* Position of center of sun relative to (0,0) */
		sunx = cos(RAD(elevation))*(double)(dim_back_w/2-dim_sun_w/2);
		suny = -sin(RAD(elevation))*(double)(dim_back_h/2-dim_sun_h/2);
		if( elevation_next > elevation )
			// Sun is rising
			sunx = -sunx;
		/* Offset sun image by center of background and dimension of image */
		x = (dim_back_w/2)+sunx-dim_sun_w/2;
		y = (dim_back_h/2)+suny-dim_sun_h/2;
		LOG(LOGVERBOSE,_("Sun coords: %.2fx%.2f"),x,y);
		IupSetfAttribute(lbl_sun,"CX","%d",(int)x);
		IupSetfAttribute(lbl_sun,"CY","%d",(int)y);
		IupSetAttribute(lbl_sun,"ZORDER","TOP");
		IupSetfAttribute(lbl_elevation,"TITLE",_("Elevation: %.1f°"),elevation);
		IupRefresh(lbl_sun);
	}

	IupSetfAttribute(infovals[0],"TITLE",_("%d° K"),guigamma_get_temp());
	IupSetfAttribute(infovals[1],"TITLE",_("%d° K"),opt_get_temp_day());
	IupSetfAttribute(infovals[2],"TITLE",_("%d° K"),opt_get_temp_night());
	IupSetfAttribute(infovals[3],"TITLE",_("%.2f° Lat, %.2f° Lon"),
			opt_get_lat(),opt_get_lon());
}


// Icon loading function
extern Ihandle *redshift_get_icon(void);
extern Ihandle *redshift_get_idle_icon(void);

// Main dialog
void guimain_dialog_init( int min ){
	Ihandle *hbox_butt,
			*button_about,
			*button_loc,
			*button_setting,
			*button_hide,
			*lbl_backsun,
			*vbox_sun,
			*framesun,
			*fvboxtitle,
			*fvboxinfo,
			*frameinfo,
			*vbox_manual,
			*framemanual,
			*framebright,
			*dhbox,
			*dvbox;
	extern Ihandle *himg_redshift,*himg_sunback,*himg_sun;

	// Buttons
	// -About
	button_about = IupButton(_("About"),NULL);
	IupSetfAttribute(button_about,"MINSIZE","%dx%d",60,24);
	IupSetCallback(button_about,"ACTION",(Icallback)_show_about);
	// -Location
	button_loc = IupButton(_("Location"),NULL);
	IupSetfAttribute(button_loc,"MINSIZE","%dx%d",60,24);
	IupSetCallback(button_loc,"ACTION",(Icallback)guilocation_show);
	// -Settings
	button_setting = IupButton(_("Settings"),NULL);
	IupSetfAttribute(button_setting,"MINSIZE","%dx%d",60,24);
	IupSetCallback(button_setting,"ACTION",(Icallback)guisettings_show);
	// -Hide to tray
	button_hide = IupButton(_("Hide"),NULL);
	IupSetfAttribute(button_hide,"MINSIZE","%dx%d",60,24);
	IupSetCallback(button_hide,"ACTION",(Icallback)_toggle_main_dialog);
	hbox_butt = IupHbox(
			button_about,
			IupFill(),
			button_loc,
			button_setting,
			button_hide,
			NULL);

	// Create Sun control
	lbl_backsun = IupLabel(NULL);
	IupSetAttributeHandle(lbl_backsun,"IMAGE",himg_sunback);
	IupSetAttribute(lbl_backsun,"CX","0");
	IupSetAttribute(lbl_backsun,"CY","0");
	lbl_sun = IupLabel(NULL);
	IupSetAttributeHandle(lbl_sun,"IMAGE",himg_sun);
	IupSetfAttribute(lbl_sun,"CX","%d",0);
	IupSetfAttribute(lbl_sun,"CY","%d",0);
	vbox_sun = IupCbox(
			lbl_backsun,
			lbl_sun,
			NULL);
	lbl_elevation = IupLabel(_("Elevation: 0"));
	// Create frame containing the sun control
	framesun = IupFrame(IupVbox(vbox_sun,IupFill(),lbl_elevation,NULL));
	IupSetAttribute(framesun,"TITLE",_("Sun elevation"));

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

	// Manual override
	chk_manual = IupSetAtt(NULL,IupToggle(_("Disable auto-adjust"),NULL)
		,"EXPAND","YES",NULL);
	IupSetCallback(chk_manual,"ACTION",(Icallback)_toggle_manual);
	val_manual = IupSetAtt(NULL,IupVal(NULL),"MIN","3400","MAX","7000",
		"VISIBLE","NO","EXPAND","HORIZONTAL",NULL);
	IupSetCallback(val_manual,"VALUECHANGED_CB",(Icallback)_manual_temp);
	vbox_manual = IupVbox(chk_manual,val_manual,NULL);
	framemanual = IupFrame(vbox_manual);
	IupSetAttribute(framemanual,"TITLE",_("Manual"));

	// Brightness
	val_bright = IupSetAtt(NULL,IupVal(NULL),"MIN","0.1","MAX","1",
		"VISIABLE","YES","EXPAND","HORIZONTAL","VALUE","1",NULL);
	IupSetCallback(val_bright,"VALUECHANGED_CB",(Icallback)_bright);
	framebright = IupFrame(IupVbox(val_bright,NULL));
	IupSetAttribute(framebright,"TITLE",_("Brightness"));

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
	//IupSetAttribute(dialog,"RESIZE","NO");
	IupSetAttribute(dialog,"MAXBOX","NO");
	IupSetAttribute(dialog,"TRAY","YES");
	IupSetAttribute(dialog,"TRAYTIP","Redshift GUI");

	IupSetAttributeHandle(dialog,"ICON",himg_redshift);
	IupSetAttributeHandle(dialog,"TRAYIMAGE",himg_redshift);
	IupSetCallback(dialog,"TRAYCLICK_CB",(Icallback)_tray_click);

	IupMap(dialog);
	if( opt_get_min() )
		IupSetAttribute(dialog,"HIDETASKBAR","YES");
	else
		IupShowXY(dialog,IUP_RIGHT,IUP_BOTTOM);

	if( opt_get_disabled() ){
		IupSetAttribute(chk_manual,"VALUE","ON");
		_toggle_manual(chk_manual,1);
	}
	IupSetAttribute(dialog,"MINSIZE",
			IupGetAttribute(dialog,"RASTERSIZE"));
}

