#include "common.h"


extern LRESULT CALLBACK windowproc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam);
void MDOProc() ;


HWND hWnd ;
HINSTANCE hInstance ;

int WINAPI WinMain(HINSTANCE hinstance,HINSTANCE hpinstance,
				   LPSTR lpszcmdline,int ncmdshow)
{
	HWND hwnd = NULL;
	WNDCLASS wc;
	MSG msg;
	wc.lpszClassName=_T("TMP_WND_CN") ;
	wc.lpszMenuName=NULL;
	wc.hInstance=hinstance;
	wc.lpfnWndProc=windowproc;
	wc.hCursor=LoadCursor(NULL,IDC_ARROW);
	wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.style=CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;

	if(!RegisterClass(&wc))
		return 1;
	if((hwnd = CreateWindowEx(
		0, wc.lpszClassName, _T("") ,
		WS_CAPTION | WS_SYSMENU, 0, 0,
		10 + GetSystemMetrics(SM_CXFIXEDFRAME) * 2,
		10 + GetSystemMetrics(SM_CYFIXEDFRAME) * 2 + GetSystemMetrics(SM_CYCAPTION),
		HWND_DESKTOP, NULL, hinstance, NULL)) == NULL)
	{
		return -1;
	}


	ShowWindow(hwnd,ncmdshow);
	UpdateWindow(hwnd);
	RedrawWindow(hwnd,NULL,NULL,RDW_INTERNALPAINT | RDW_UPDATENOW);

	hInstance = hinstance ;
	hWnd = hwnd ;

	MDOProc() ;

	DestroyWindow( hwnd ) ;

	while( GetMessage(&msg , NULL , 0 , 0) )
	{
		TranslateMessage(&msg) ;
		DispatchMessage(&msg) ;
	}

	return msg.wParam;
}
















#define ROM(addr)			(pRom[(unsigned int)(addr)%iRomSize])
#define ROMOFFSET(addr)		((unsigned int)(addr)%iRomSize)

#define SETROM8(addr,val)	{ROM(addr)=(BYTE)(val);}
#define SETROM16(addr,val)	{SETROM8 (addr,val);SETROM8 ((addr)+1,(val)>> 8);}
#define SETROM24(addr,val)	{SETROM16(addr,val);SETROM8 ((addr)+2,(val)>>16);}
#define SETROM32(addr,val)	{SETROM16(addr,val);SETROM16((addr)+2,(val)>>16);}

#define GETROM8(addr)		(ROM(addr))
#define GETROM16(addr)		(ROM(addr)|(ROM((addr)+1)<<8))
#define GETROM24(addr)		(ROM(addr)|(ROM((addr)+1)<<8)|(ROM((addr)+2)<<16))
#define GETROM32(addr)		(ROM(addr)|(ROM((addr)+1)<<8)|(ROM((addr)+2)<<16)|(ROM((addr)+3)<<24))

#define ROM_NAME            _T("kagami.gba")


#define SFC_CHR 1
#define SFC_DYNAMIC 2
#define MAXSFC 10


BYTE *pRom ;
int iRomSize ;
MyDIBObj3 mdo( MAXSFC , 128 , 128 ) ;


void CLSFUNC( int sfc , int x , int y , int w , int h , UINT color )
{
	mdo.Cls( MDO3WINAPI , sfc , x , y , w , h , color ) ;
}


void MDOProc()
{

	iRomSize = LoadMemoryFromFile( ROM_NAME , &pRom ) ;
	if( iRomSize <= 0 )
	{
		MessageBox( hWnd , _T("ROM\"") ROM_NAME _T("\"読み込みに失敗") , _T("エラー") , MB_OK ) ;
		return ;
	}

	mdo.Initialize( hWnd , hInstance );
	SetGBADrawRoutine( CLSFUNC , NULL , NULL ) ;

	BMPD pal[256] ;
	BYTE *Pimage ;

	mdo.CreateSurface( SFC_CHR , 128 , 128*4 ) ;

	MessageBox( hWnd , _T("処理を開始します。\nそれなりに時間がかかります") , _T("報告") , MB_OK ) ;

	for( int i=0 ; i<0x24 ; i++ )
	{
		int adr ;

		adr = 0xD2FE40 + i*4 ;
		adr = GETROM32( adr ) ;

		//ビットマップ
		{
			int rsize , wsize ;
			int iOffset ;
			iOffset = GETROM32( adr+0x8 ) ;
			YumeDXDecode( &Pimage , &ROM( iOffset ) , ROMOFFSET(-1)-ROMOFFSET(iOffset) , &rsize , &wsize ) ;
			for( int i=0 ; i<wsize/32 ; i++ )
			{
				DrawGBACharacter( Pimage+i*32 , SFC_CHR , i%16*8 , i/16*8 , 0 , false , 1 ) ;
			}
			free( Pimage ) ;
		}
		//パレット
		{
			int tadr ;
			tadr = GETROM32( adr+0x10 ) ;
			for( int i=0 ; i<128 ; i++ )
			{
				int tmp ;
				tmp = GETROM16( tadr ) ;
				int r,g,b ;
				r = (tmp>> 0) & 0x1F ;
				g = (tmp>> 5) & 0x1F ;
				b = (tmp>>10) & 0x1F ;
				pal[0x60+i] = myRGB(r,g,b) ;
				tadr += 2 ;
			}
		}

		{
			int width , height ;
			int cadr ;
			width  = GETROM16( adr + 0 ) ;
			height = GETROM16( adr + 2 ) ;
			cadr   = GETROM32( adr+0x18 ) ;

			mdo.CreateSurface( SFC_DYNAMIC , width*8 , height*8 ) ;

			MDO3Opt topt = *MDO3normal ;
			topt.flag |= MDO3F_USE_COLOR_TABLE ;
			topt.flag &= ~MDO3F_COLORKEY ;
			for( int t=0 ; t<width*height ; t++ )
			{
				int tx,ty ;
				tx = t%width*8 ;
				ty = t/width*8 ;
				int tiledata ;
				tiledata = GETROM16(cadr) ;

				int chrno ;
				int palno ;
				palno = tiledata>>12 ;
				chrno = tiledata&0x03FF ;
				topt.flag &= ~(MDO3F_X_MIRROR|MDO3F_Y_MIRROR) ;
				if( tiledata & 0x0400 )
					topt.flag |= MDO3F_X_MIRROR ;
				if( tiledata & 0x0800 )
					topt.flag |= MDO3F_Y_MIRROR ;
					topt.PBMPD = &pal[16*palno+16*0] ;
				mdo.Blt( &topt , SFC_DYNAMIC , tx , ty , 8 , 8 , SFC_CHR , chrno%16*8 , chrno/16*8 ) ;

				cadr += 2 ;
			}

			_TCHAR filename[64] ;
			_stprintf_s( filename , _countof(filename) , _T("sight%02d[%02X].bmp") , i , i ) ;
			mdo.SaveBitmapFile( filename , SFC_DYNAMIC ) ;
			mdo.ReleaseSurface( SFC_DYNAMIC ) ;
		}
	}
	MessageBox( hWnd , _T("処理が終了しました") , _T("報告") , MB_OK ) ;


}