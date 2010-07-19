#include "common.h"
#include <iup.h>
#include "options.h"
#include "gamma.h"
#include "iupgui_main.h"
#include "iupgui_settings.h"
#include "iupgui_gamma.h"
#include "thirdparty/stb_image.h"

extern unsigned char png_redshift[];
extern unsigned char png_redshift_idle[];
extern unsigned char png_sunback[];
extern unsigned char png_sun[];

Ihandle *himg_redshift,*himg_redshift_idle,
		*himg_sunback,*himg_sun;

int dim_back_w, dim_back_h, dim_sun_w, dim_sun_h;

// Load images
static void _load_icons(void){
	int w,h,n;
	unsigned char *img;
	
	img = stbi_load_from_memory(
			png_redshift,1971,
			&w,&h,&n,0);
	LOG(LOGVERBOSE,_("Main icon size: %dx%dx%d"),w,h,n);
	himg_redshift = IupImageRGBA(w,h,img);
	IupSetHandle("MAIN_ICON",himg_redshift);
	IupSetAttribute(NULL,"ICON","MAIN_ICON");
	stbi_image_free(img);

	img = stbi_load_from_memory(
			png_redshift_idle,1998,
			&w,&h,&n,0);
	himg_redshift_idle = IupImageRGBA(w,h,img);
	stbi_image_free(img);

	img = stbi_load_from_memory(
			png_sunback,1342,
			&dim_back_w,&dim_back_h,&n,0);
	himg_sunback = IupImageRGBA(dim_back_w,dim_back_h,img);
	stbi_image_free(img);

	img = stbi_load_from_memory(
			png_sun,1762,&dim_sun_w,&dim_sun_h,&n,0);
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

// Main GUI code
int iup_gui(int argc, char *argv[]){
	IupOpen( &argc,&argv );
	_load_icons();
	guimain_dialog_init(opt_get_min());
	guigamma_init_timers();
	if( opt_get_disabled() )
		guigamma_disable();
	IupMainLoop();
	_unload_icons();
	IupClose();
	if(!guimain_exit_normal()){
		LOG(LOGERR,_("An error occurred."));
		return RET_FUN_FAILED;
	}else
		return RET_FUN_SUCCESS;
}
