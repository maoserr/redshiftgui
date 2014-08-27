#include "common.h"
#include "gamma.h"
#include "options.h"
#include "gui/iupgui.h"
#include "gui/iupgui_main.h"
#include "gui/iupgui_settings.h"
#include "gui/iupgui_gamma.h"
#include "thirdparty/stb_image.h"

extern unsigned char png_redshift[];
extern unsigned int png_redshift_size;

extern unsigned char png_redshift_idle[];
extern unsigned int png_redshift_idle_size;

extern unsigned char png_sunback[];
extern unsigned int png_sunback_size;

extern unsigned char png_sun[];
extern unsigned int png_sun_size;

Ihandle *himg_redshift,*himg_redshift_idle,
		*himg_sunback,*himg_sun;

int dim_back_w, dim_back_h, dim_sun_w, dim_sun_h;

// Load images
static void _load_icons(/*@unused@*/ char *active,/*@unused@*/ char *idle){
	int w=0;
	int h=0;
	int n=0;
	unsigned char *img;

	img = stbi_load_from_memory(
			png_redshift,(int)png_redshift_size,
			&w,&h,&n,0);
	LOG(LOGVERBOSE,_("Main icon size: %dx%dx%d"),w,h,n);
	himg_redshift = IupImageRGBA(w,h,img);
	(void)IupSetHandle("MAIN_ICON",himg_redshift);
	IupSetAttribute(NULL,"ICON","MAIN_ICON");
	stbi_image_free(img);

	img = stbi_load_from_memory(
			png_redshift_idle,(int)png_redshift_idle_size,
			&w,&h,&n,0);
	himg_redshift_idle = IupImageRGBA(w,h,img);
	stbi_image_free(img);

	img = stbi_load_from_memory(
			png_sunback,(int)png_sunback_size,
			&dim_back_w,&dim_back_h,&n,0);
	himg_sunback = IupImageRGBA(dim_back_w,dim_back_h,img);
	stbi_image_free(img);

	img = stbi_load_from_memory(
			png_sun,(int)png_sun_size,&dim_sun_w,&dim_sun_h,&n,0);
	himg_sun = IupImageRGBA(dim_sun_w,dim_sun_h,img);
	stbi_image_free(img);
}

// Unload images
static void _unload_icons(void){
	IupDestroy(himg_redshift);
	IupDestroy(himg_redshift_idle);
	IupDestroy(himg_sunback);
	IupDestroy(himg_sun);
}

// Home page callback
static int _btn_home(/*@unused@*/ Ihandle *ih){
	(void)IupHelp(STR(PACKAGE_HOME));
	return IUP_DEFAULT;
}

// Bug report callback
static int _btn_bug(/*@unused@*/ Ihandle *ih){
	(void)IupHelp(STR(PACKAGE_BUGREPORT));
	return IUP_DEFAULT;
}

// Close callback
static int _btn_close(/*@unused@*/ Ihandle *ih){
	return IUP_CLOSE;
}

// About dialog
int gui_about(/*@unused@*/ Ihandle *ih){
	Ihandle *dialog;
	Ihandle *lbl_icon;
	Ihandle *hbox_title;
	Ihandle *txt_area;
	Ihandle *vbox_top;
	Ihandle *btn_home,*btn_bug,*btn_close;
	Ihandle *hbox_buttons;
	Ihandle *vbox_all;

	lbl_icon = IupLabel(NULL);
	IupSetAttributeHandle(lbl_icon,"IMAGE",himg_redshift);
	hbox_title = IupHbox(lbl_icon,IupLabel(_("RedshiftGUI  ")),
			IupLabel(STR(PACKAGE_VER)),NULL);

	txt_area = IupSetAtt(NULL,IupText(NULL),
			"EXPAND","YES","MULTILINE","YES",
			"VISIBLELINES","5",
			"SCROLLBAR","VERTICAL",
			"READONLY","YES",
			"VALUE",
			_("Homepage:\n"
				STR(PACKAGE_HOME)
				"\n\n"
				"Written by Mao Yu\n"
				"Redshift GUI is based on Redshift by Jon Lund Steffensen\n"
				"This program uses IUP and libcURL\n"
				"Licensed under the GPL license"),NULL);
	btn_home = IupButton(_("Homepage"),NULL);
	IupSetfAttribute(btn_home,"MINSIZE","%dx%d",60,24);
	(void)IupSetCallback(btn_home,"ACTION",(Icallback)_btn_home);

	btn_bug = IupButton(_("Bug report"),NULL);
	IupSetfAttribute(btn_bug,"MINSIZE","%dx%d",60,24);
	(void)IupSetCallback(btn_bug,"ACTION",(Icallback)_btn_bug);

	btn_close = IupButton(_("Close"),NULL);
	IupSetfAttribute(btn_close,"MINSIZE","%dx%d",60,24);
	(void)IupSetCallback(btn_close,"ACTION",(Icallback)_btn_close);

	hbox_buttons = IupHbox(btn_home,btn_bug,IupFill(),btn_close,NULL);
	vbox_top = IupSetAtt(NULL,IupVbox(hbox_title,txt_area,NULL),
			"MARGIN","5x5",NULL);

	vbox_all = IupVbox(
			IupSetAtt(NULL,IupFrame(vbox_top),
				"TITLE",_("About"),NULL),
			hbox_buttons,NULL);
	IupSetAttribute(vbox_all,"NMARGIN","5x5");
	
	dialog = IupDialog(vbox_all);
	IupSetAttribute(dialog,"TITLE",_("About Dialog"));
	IupSetAttribute(dialog,"SIZE","250x150");
	IupSetAttributeHandle(dialog,"ICON",himg_redshift);
	(void)IupPopup(dialog,IUP_CENTER,IUP_CENTER);
	IupDestroy(dialog);
	return IUP_DEFAULT;
}

// Popup an message dialog
int gui_popup(char *title,char *msg,char *type){
	Ihandle *popup = IupSetAtt(NULL,IupMessageDlg(),
			"TITLE",title,
			"VALUE",msg,
			"DIALOGTYPE",type,
			NULL);
	(void)IupPopup(popup,IUP_CENTER,IUP_CENTER);
	IupDestroy(popup);
	return RET_FUN_SUCCESS;
}

// Main GUI code
int iup_gui(int argc, char *argv[]){
	(void)IupOpen( &argc,&argv );
	_load_icons("","");
	guimain_dialog_init();
	guigamma_init_timers();
	if( opt_get_disabled() )
		guigamma_disable();
	(void)IupMainLoop();
	guigamma_end_timers();
	_unload_icons();
	IupClose();
	if(!guimain_exit_normal()){
		LOG(LOGERR,_("An error occurred."));
		return RET_FUN_FAILED;
	}else
		return RET_FUN_SUCCESS;
}
