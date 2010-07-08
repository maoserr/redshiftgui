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

// Load images
static void _load_icons(void){
	int w,h,n;
	unsigned char *img;
	
	img = stbi_load_from_memory(
			png_redshift,1971,
			&w,&h,&n,0);
	LOG(LOGVERBOSE,_("Main icon size: %dx%dx%d"),w,h,n);
	himg_redshift = IupImageRGBA(w,h,img);
	stbi_image_free(img);

	img = stbi_load_from_memory(
			png_redshift_idle,1998,
			&w,&h,&n,0);
	himg_redshift_idle = IupImageRGBA(w,h,img);
	stbi_image_free(img);

	img = stbi_load_from_memory(
			png_sunback,1342,
			&w,&h,&n,0);
	himg_sunback = IupImageRGBA(w,h,img);
	stbi_image_free(img);

	img = stbi_load_from_memory(
			png_sun,1762,&w,&h,&n,0);
	himg_sun = IupImageRGBA(w,h,img);
	stbi_image_free(img);
}

// Main GUI code
int iup_gui(int argc, char *argv[]){
	IupOpen( &argc,&argv );
	_load_icons();
	guimain_dialog_init(1);
	guigamma_init_timers();
	IupMainLoop();
	IupClose();
	return RET_FUN_SUCCESS;
}
