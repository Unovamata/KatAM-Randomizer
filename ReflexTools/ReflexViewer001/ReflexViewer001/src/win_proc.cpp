#include "common.h"



LRESULT CALLBACK windowproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
static bool cap[2]={false,false};
	switch(message)
	{
	case WM_KEYDOWN:
		//OutputDebugString(Int2Char(wparam));
		ProOn(wparam);
	break;
	case WM_KEYUP:
		ProOff(wparam);
	break;
	case WM_MOUSEMOVE:
		ProMMove(lparam);
	break;
	case WM_LBUTTONDOWN:
		ProMOn(MB_L);
		if( ! cap[1] ) SetCapture( hwnd ) ;
		cap[0] = true; 
	break;
	case WM_LBUTTONUP:
		ProMOff(MB_L);
		if( ! cap[1] ) ReleaseCapture() ;
		cap[0] = false; 
	break;
	case WM_RBUTTONDOWN:
		ProMOn(MB_R);
		if( ! cap[0] ) SetCapture( hwnd ) ;
		cap[1] = true; 
	break;
	case WM_RBUTTONUP:
		ProMOff(MB_R);
		if( ! cap[0] ) ReleaseCapture() ;
		cap[1] = false; 
	break;
	case WM_MBUTTONDOWN:
		ProMOn(MB_C);
	break;
	case WM_MBUTTONUP:
		ProMOff(MB_C);
	break;
#ifdef	MY_DROP_ACCEPT
	case WM_DROPFILES:
//		DropRoutine(wparam);
	break;
#endif
	case WM_CLOSE:
		DestroyWindow(hwnd);
        return 0;
	break;//”O
	case WM_DESTROY :
		PostQuitMessage(0);
	break;
	case WM_PAINT:
		RequestFlip();
		return DefWindowProc(hwnd,message,wparam,lparam);
	break;
	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}
	return 0;
}
