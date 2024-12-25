#include "common.h"



LRESULT CALLBACK windowproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
	switch(message)
	{
	case WM_KEYDOWN:
	break;
	case WM_KEYUP:
	break;
	case WM_MOUSEMOVE:
	break;
	case WM_LBUTTONDOWN:
	break;
	case WM_LBUTTONUP:
	break;
	case WM_RBUTTONDOWN:
	break;
	case WM_RBUTTONUP:
	break;
	case WM_MBUTTONDOWN:
	break;
	case WM_MBUTTONUP:
	break;
	case WM_DROPFILES:
	break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
        return 0;
	break;
	case WM_DESTROY :
		PostQuitMessage(0);
	break;
	case WM_PAINT:
		return DefWindowProc(hwnd,message,wparam,lparam);
	break;
	default:
		return DefWindowProc(hwnd,message,wparam,lparam);
	}
	return 0;
}
