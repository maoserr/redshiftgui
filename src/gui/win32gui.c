#include "common.h"
#include "gamma.h"
#include "options.h"
#include <CommCtrl.h>
#include "win32/resource.h"


INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg){
		case WM_COMMAND:
			switch(LOWORD(wParam)){
			case IDCANCEL:
				SendMessage(hDlg, WM_CLOSE, 0, 0);
				return TRUE;
			}
			break;

		case WM_CLOSE:
			DestroyWindow(hDlg);
			return TRUE;

		case WM_DESTROY:
			PostQuitMessage(0);
			return TRUE;
	}
	return FALSE;
}

// Main GUI code
int win32_gui(int argc, char *argv[]){
	//guimain_dialog_init();
	//guigamma_init_timers();
	//if( opt_get_disabled() )
		//guigamma_disable();
	//(void)IupMainLoop();
	//guigamma_end_timers();
	//_unload_icons();
	//IupClose();
	HWND hDlg;
	MSG msg;
	BOOL ret;

	InitCommonControls();
	hDlg = CreateDialogParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAINDIALOG ), 0, DialogProc, 0);
	ShowWindow(hDlg, SW_SHOW);

	while((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if(ret == -1){
			LOG(LOGERR,_("An error occurred."));
			return RET_FUN_FAILED;
		}

		if(!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return RET_FUN_SUCCESS;
}