#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <assert.h>


#ifdef _ORE_SENYOU
//私はいくつかのソースをライブラリ化しているので……
#ifdef _DEBUG
#pragma comment(lib,"gl_d.lib")
#else
#pragma comment(lib,"gl.lib")
#endif
#endif

#include "nnsys.h"
#include "mydibobj3.h"
#include "task0810.h"
#include "myeinput.h"
#include "keycode.h"
#include "mymemorymanage.h"
#include "TaskClasses.h"


//#include "TASKGUI/taskGUI.h"

#ifdef _ORE_SENYOU
#include "HT/GBAsys.h"
#include "KirbyCode/yumeLZ77.h"
#include "KirbyCode/kagamiRL.h"
#else
#include "GBAsys.h"
#include "yumeLZ77.h"
#include "kagamiRL.h"
#endif


//アプリケーションのウインドウへのドロップを許可するときはコメントアウトを外す
//#define	MY_DROP_ACCEPT
//２重起動を（簡易）抑制するときはコメントアウト
#define	ENABLE_MULTIPLE_RUN


/*
	
	11.	サーフェイスを指す定数を作成
	12.	適当な場所でサーフェイスを作成
*/

//define

#define		COMMON_CONST_WINDOW_CLASS_NAME		_T("REFLEX_TOOL_WCN")
#define		COMMON_CONST_DEFAULT_WINDOW_TEXT	_T("Ｒｅｆｌｅｘ　Ｔｏｏｌ")

#define		ROM_NAME			"kagami.gba"
#define		DEFAULT_ROM_SIZE	(16*1024*1024)
#define		EHASH_OF_AUTHOR		(0x9B8942)

#define		WINWIDTH				780
#define		WINHEIGHT				580

#define		MAXSFC				10
#define		MAXTASK				200

#define		SFC_BACK			0
#define		SFC_CHR				1
#define		SFC_CHIP			2
#define		SFC_CHIP_INFO		3




//グローバル関数

extern int AppPrepare(HWND hwnd , HINSTANCE hinstance);
extern int AppRelease();
extern void MainRoutine(int fps);

extern void RequestRedraw();
extern void RequestFlip();


#define		KAGEMOJI(sfc,str,x,y,h,w)								{\
	mdo.Text( sfc , str , x+1 , y+1 , h , w , RGB(0,0,0) )		 ; \
	mdo.Text( sfc , str , x   , y   , h , w , RGB(255,255,255) ) ; \
	}																/**/
#define		BBOXMOJI(sfc,str,x,y,h,w,c)								{\
	mdo.Text( sfc , str , x-1 , y   , h , w , RGB(0,0,0) )		 ; \
	mdo.Text( sfc , str , x+1 , y   , h , w , RGB(0,0,0) )		 ; \
	mdo.Text( sfc , str , x   , y-1 , h , w , RGB(0,0,0) )		 ; \
	mdo.Text( sfc , str , x   , y+1 , h , w , RGB(0,0,0) )		 ; \
	mdo.Text( sfc , str , x   , y   , h , w , c ) ;                \
	}																/**/

//グローバル変数

extern HWND hWnd;
extern HINSTANCE hInstance;

extern MyDIBObj3 mdo;

extern TASKHANDLE HAM ;

extern BYTE *Prom ;
extern int romsize ;



#define ADR2ROMOFF(adr)	((unsigned int)(adr))
#define ROM(adr)		Prom[ADR2ROMOFF(adr)%romsize]


#define SETROM8(adr,val)	{ROM(adr)=(BYTE)(val);}
#define SETROM16(adr,val)	{SETROM8(adr,val);SETROM8((adr)+1,(val)>>8);}
#define SETROM24(adr,val)	{SETROM16(adr,val);SETROM8((adr)+2,(val)>>16);}
#define SETROM32(adr,val)	{SETROM16(adr,val);SETROM16((adr)+2,(val)>>16);}

#define GETROM8(adr)		(ROM(adr))
#define GETROM16(adr)		(ROM(adr)|(ROM((adr)+1)<<8))
#define GETROM24(adr)		(ROM(adr)|(ROM((adr)+1)<<8)|(ROM((adr)+2)<<16))
#define GETROM32(adr)		(ROM(adr)|(ROM((adr)+1)<<8)|(ROM((adr)+2)<<16)|(ROM((adr)+3)<<24))


#define ROME(ptr,size,adr)	((ptr)[ADR2ROMOFF(adr)%size])

#define SETROME8(ptr,size,adr,val)	{ROME(ptr,size,adr)=(BYTE)(val);}
#define SETROME16(ptr,size,adr,val)	{SETROME8(ptr,size,adr,val);SETROME8(ptr,size,(adr)+1,(val)>>8);}
#define SETROME24(ptr,size,adr,val)	{SETROME16(ptr,size,adr,val);SETROME8(ptr,size,(adr)+2,(val)>>16);}
#define SETROME32(ptr,size,adr,val)	{SETROME16(ptr,size,adr,val);SETROME16(ptr,size,(adr)+2,(val)>>16);}

#define GETROME8(ptr,size,adr)		(ROME(ptr,size,adr))
#define GETROME16(ptr,size,adr)		(ROME(ptr,size,adr)|(ROME(ptr,size,(adr)+1)<<8))
#define GETROME24(ptr,size,adr)		(ROME(ptr,size,adr)|(ROME(ptr,size,(adr)+1)<<8)|ROME(ptr,size,(adr)+2)<<16))
#define GETROME32(ptr,size,adr)		(ROME(ptr,size,adr)|(ROME(ptr,size,(adr)+1)<<8)|ROME(ptr,size,(adr)+2)<<16)|(ROME(ptr,size,(adr)+3)<<24))
