#include "common.h"
#include <iup.h>
#include "options.h"
#include "iupgui_settings.h"
#include "iupgui_location.h"
#include "iupgui_gamma.h"

// Main dialog handles
static Ihandle *dialog=NULL;
static Ihandle *infotitle[4]={NULL,NULL,NULL,NULL};
static Ihandle *infovals[4]={NULL,NULL,NULL,NULL};
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

	return IUP_DEFAULT;
}

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

// Tray click callback
static int _tray_click(Ihandle *ih, int but, int pressed, int dclick){
	static Ihandle *menu_tray=NULL;
	switch (but){
		case 1:
			if( pressed )
				_toggle_main_dialog(ih);
			break;
		default:
			if( pressed ){
				// Bring up menu
				if( !menu_tray ){
					Ihandle *mitem_toggle,
							*mitem_settings,
							*mitem_about;
					mitem_toggle = IupItem(_("Hide/Show"),NULL);
					IupSetCallback(mitem_toggle,"ACTION",_toggle_main_dialog);
					mitem_settings = IupItem(_("Settings"),NULL);
					IupSetCallback(mitem_settings,"ACTION",guisettings_show);
					mitem_about = IupItem(_("About"),NULL);
					IupSetCallback(mitem_about,"ACTION",_show_about);
					menu_tray = IupMenu(
							mitem_toggle,
							mitem_settings,
							IupSeparator(),
							mitem_about,
							NULL);
					IupMap(menu_tray);
				}
				IupPopup(menu_tray,IUP_MOUSEPOS,IUP_MOUSEPOS);
				// Need a workaround on GTK2 because MOUSEPOS doesn't seem to work on lower bar
				IupDestroy(menu_tray);
				menu_tray = NULL;
			}
			break;
	}
	return IUP_DEFAULT;
}

// Updates info display
void guimain_update_info(void){
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
void guimain_dialog_init( int show ){
	Ihandle *hbox_butt,
			*button_loc,
			*button_setting,
			*button_hide,
			*lbl_backsun,
			*vbox_sun,
			*framesun,
			*fvboxtitle,
			*fvboxinfo,
			*frameinfo,
			*dhbox,
			*dvbox;
	extern Ihandle *himg_redshift,*himg_sunback,*himg_sun;

	// Buttons
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
	hbox_butt = IupHbox(button_loc,
			button_setting,
			button_hide,
			NULL);

	// Create Sun control
	lbl_backsun = IupLabel(NULL);
	IupSetAttributeHandle(lbl_backsun,"IMAGE",himg_sunback);
	IupSetAttribute(lbl_backsun,"CX","0");
	IupSetAttribute(lbl_backsun,"CY","0");
	IupSetAttribute(lbl_backsun,"ZORDER","BOTTOM");
	lbl_sun = IupLabel(NULL);
	IupSetAttributeHandle(lbl_sun,"IMAGE",himg_sun);
	IupSetfAttribute(lbl_sun,"CX","%d",0);
	IupSetfAttribute(lbl_sun,"CY","%d",0);
	vbox_sun = IupCbox(
			lbl_sun,
			lbl_backsun,
			NULL);
	// Create frame containing the sun control
	framesun = IupFrame(IupVbox(vbox_sun,IupFill(),NULL));
	IupSetAttribute(framesun,"TITLE",_("Sun elevation"));

	// Info display
	infotitle[0]=IupLabel(_("Current:"));
	infotitle[1]=IupLabel(_("Day:"));
	infotitle[2]=IupLabel(_("Night:"));
	infotitle[3]=IupLabel(_("Location:"));
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

	// Layout box for sun and info
	dhbox = IupHbox(framesun,IupFill(),frameinfo,NULL);

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
	IupSetCallback(dialog,"TRAYCLICK_CB",(Icallback)_tray_click);

	if( show )
		IupShowXY(dialog,IUP_RIGHT,IUP_BOTTOM);
}

