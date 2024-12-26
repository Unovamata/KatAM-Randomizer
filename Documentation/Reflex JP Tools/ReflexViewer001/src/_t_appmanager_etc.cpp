#include "common.h"


void CLSA( int sfc , int x , int y , int w , int h , UINT c )
{
	mdo.Cls( MDO3normal , sfc , x , y , w , h , c ) ;
}

UINT GBPAL[4] =
{
	myRGB(30,30,30) , 
	myRGB(20,20,20) , 
	myRGB(10,10,10) , 
	myRGB( 0, 0, 0) , 
} ;

void TaskAppManager::Finit()
{

	Prom = NULL ;
	romsize = LoadMemoryFromFile( ROM_NAME , &Prom ) ;
	if( romsize < 0 )
	{
		MessageBox( hWnd , ROM_NAME "が開けません" , "エラー" , MB_OK ) ;
		Suicide() ;
		return ;
	}


	{
		bool onerror = false ;
		if( romsize != DEFAULT_ROM_SIZE )
		{
				MessageBox( hWnd , "ＲＯＭのサイズが異常です" , "エラー" , MB_OK ) ;
				onerror = true ;
		}
		else
		{
			unsigned int eh = EHash( Prom , romsize ) ;
			if( eh != EHASH_OF_AUTHOR )
			{
				char str[256] ;
				sprintf_s( str , 256 , "このソフト開発者のROMのHash:%06X\n読み込まれたROMのHash:%06X\nROM内容が異なると思われます" , EHASH_OF_AUTHOR , eh ) ;
				MessageBox( hWnd , str , "エラー" , MB_OK ) ;
				onerror = true ;
			}
		}
		if( onerror )
		{
			int ans ;
			ans = MessageBox( hWnd , "それでも続けますか？" , "確認"  , MB_YESNO ) ;
			if( ans == IDNO )
			{
				Suicide() ;
				return ;
			}
		}
	}



	mdo.Initialize( hWnd , hInstance );
	mdo.CreateSurface( SFC_CHR , 128 , 128*4 ) ;
	mdo.CreateSurface( SFC_CHIP , 256 , 256*4 ) ;
	mdo.CreateSurface( SFC_CHIP_INFO , 256 , 256 ) ;
/*
	{
		mdo.Cls( MDO3normal , SFC_CHIP_INFO , 0 , 0 , 16*16 , 16*16 , myRGB(31,0,31) ) ;
		for( int i=0 ; i<256 ; i++ )
		{
			mdo.Text( SFC_CHIP_INFO , Byte2Hex(i) , i%16*16+1 , i/16*16+2 , 14 , 7 , RGB(255,255,255) ) ;
			mdo.Text( SFC_CHIP_INFO , Byte2Hex(i) , i%16*16+3 , i/16*16+2 , 14 , 7 , RGB(255,255,255) ) ;
			mdo.Text( SFC_CHIP_INFO , Byte2Hex(i) , i%16*16+2 , i/16*16+1 , 14 , 7 , RGB(255,255,255) ) ;
			mdo.Text( SFC_CHIP_INFO , Byte2Hex(i) , i%16*16+2 , i/16*16+3 , 14 , 7 , RGB(255,255,255) ) ;
			mdo.Text( SFC_CHIP_INFO , Byte2Hex(i) , i%16*16+2 , i/16*16+2 , 14 , 7 , RGB(64,64,64) ) ;
		}
		mdo.SaveBitmapFile( "chipinfo.bmp" , SFC_CHIP_INFO ) ;
	}
//*/
	mdo.LoadBitmapFile( SFC_CHIP_INFO , "chipinfo.bmp" ) ;

	SetGBADrawRoutine( CLSA , NULL , NULL ) ;

	new TaskFrameInit( this , TP_FRAMEINIT , true ) ;
	new TaskFrameEnd ( this , TP_FRAMEEND  , true ) ;

	new ModeManager  ( this , TP_MODE_MANAGER , true ) ;

	return ;
}
void TaskAppManager::Fmain()
{
	if( !SearchTask( TP_MODE_MANAGER ) )Suicide() ;
	return ;
}
void TaskAppManager::Fdest()
{
	return ;
}
void TaskAppManager::Fdraw()
{
	return ;
}

void TaskFrameEnd::Fmain()
{
	KeyMove();
	if(isredraw)
	{
		mdo.Cls( MDO3normal , 0 , 0 , 0 , WINWIDTH , WINHEIGHT , myRGB(28,31,28) );
		Task0810 *Pptr ;
		Pptr = Task0810::SolveHandle( HAM ) ;
		if( Pptr ) Pptr->DrawCN() ;
		isredraw = 0;
		isflip = 1;
	}
	if(isflip)
	{
		mdo.Flip( MDO3FT_NORMAL );
		isflip = 0;
	}
	return;
}










static TASKHANDLE h_fe ;

static bool CheckFE()
{
	if( !h_fe )
	{	
		Task0810 *Pptr ;
		Pptr = Task0810::SolveHandle( HAM ) ;
		if( !Pptr )return false ;
		h_fe = Pptr->SearchTask( TP_FRAMEEND ) ;
		if( !h_fe )return false ;
	}
	return true ;
}
void RequestRedraw()
{
	if( !CheckFE() )return ;
	Task0810 *Pptr ;
	Pptr = Task0810::SolveHandle( h_fe ) ;
	if( !Pptr )return ;
	((TaskFrameEnd*)Pptr)->RequestRedraw() ;
}
void RequestFlip()
{
	if( !CheckFE() )return ;
	Task0810 *Pptr ;
	Pptr = Task0810::SolveHandle( h_fe ) ;
	if( !Pptr )return ;
	((TaskFrameEnd*)Pptr)->RequestFlip() ;
}

