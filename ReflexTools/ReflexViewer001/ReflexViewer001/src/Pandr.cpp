#include "common.h"


static void PrepareCalledTM() ;

MyDIBObj3 mdo(MAXSFC , WINWIDTH , WINHEIGHT );

TASKHANDLE HAM ;

BYTE *Prom ;
int romsize ;



int AppPrepare(HWND hwnd , HINSTANCE hinstance)
{
	Task0810 *Ptmp ;
	Ptmp = new TaskAppManager( NULL , TP_APP_MANAGER , true ) ;
	HAM = Ptmp->GetHandle() ;

	return 0;
}
int AppRelease()
{

	return 0;
}
