#include "common.h"


void ModeManager::Finit()
{
	RequestRedraw() ;
	SetDrawOrderPandC( true , false ) ;



	mode = -1 ;
	drawmode = 0 ;
	isshowbrokenchip = false ;
	curshortroom = 0 ;
	curlongroom = SolveShortRoom( curshortroom ) ;

	Pview = NULL ;
	Phit = NULL ;
//	Pvariableno = NULL ;

	ExpRoom() ;
	RenderChr() ;

	return ;
}
void ModeManager::Fmain()
{
	bool chr_redraw = false ;

	if( KeyScroll( &curshortroom , KC_F6 , KC_F5 , 1 , 16 , 0 ) )
	{
		RotateCorrect( &curshortroom , MAX_SHORT_ROOM ) ;
		curlongroom = SolveShortRoom( curshortroom ) ;

		RequestRedraw() ; 
		ExpRoom() ;

		chr_redraw = true ;
		mode = -1 ;
	} ;

	if( chr_redraw )
	{
		RenderChr() ;
	}

	if( KeyPush( KC_TAB ) )
	{
		if( KeyOff( KC_CTRL ) )
		{
			drawmode ++ ;
			RotateCorrect( &drawmode , 2 ) ;
			RequestRedraw() ;
		}
		else
		{
			isshowbrokenchip = !isshowbrokenchip ;
			RequestRedraw() ;
		}
	}

	{
		int pmode = mode ;
		if( mode==-1 )mode = 0 ;
		KeyScroll( &mode , KC_F2 , KC_F1 , 1 , 1 , 0 ) ;
		RotateCorrect( &mode , 1 ) ;
		if( pmode != mode )
		{
			KillP( TP_MODE_START , TP_MODE_END ) ;
			switch( mode )
			{
			case 0:
				//面倒なので、二つとも呼ぶ
				//現在は描画しかしていないので問題なし
				//なんなら、編集できたとしても問題ないかも？
				new TaskModeEnemy( this , TP_MODE_ENEMY , true ) ; 
				new TaskModeDoor ( this , TP_MODE_DOOR  , true ) ; 
			break ;
			}
			RequestRedraw() ;
		}
	}

	if( KeyScroll( &x , KC_S , KC_A , 16 , 6 , 0 ) ){ RequestRedraw() ; } ;
	if( KeyScroll( &y , KC_Z , KC_W , 16 , 6 , 0 ) ){ RequestRedraw() ; } ;


/*
//ハンドルがきちんと解放されているかが心配なのでテスト
	if( KeyPush( KC_SPACE ) )
	{
		char buf[64] ;
		int a,b ;
		Task0810 **PPc ;
		Task0810::GetHandleState( &a , &b , &PPc ) ;
		sprintf_s( buf , 64 , "%X/%X" , a , b ) ;
		SetWindowText( hWnd , buf ) ;
	}
//*/
	return ;
}
void ModeManager::Fdest()
{
	return ;
}
void ModeManager::Fdraw()
{
	{
		char buf[64] ;
		sprintf_s( buf , 64 , "SRoom:%02X(LRoom:%X)" , curshortroom , curlongroom ) ;
		SetWindowText( hWnd , buf ) ;
	}
	if( curlongroom < 0 )return ;

	//パレットを展開
	BMPD pal[256] ;
	{
		int adr ;
		adr = BASE_ADR_LROOM_PARAMETER + curlongroom * SIZE_OF_LONG_ROOM_PARAMETER ;
		adr += 0x16 ;
		adr = GETROM16( adr ) ;
		adr = BASE_ADR_BG_PAL + adr*4 ;
		adr = GETROM32( adr ) ;
		adr = GETROM32( adr ) ;
		for( int i=0 ; i<256 ; i++ )
		{
			int t,r,g,b ;
			t = GETROM16( adr + i*2 ) ;
			r = t & 0x1F ;
			g = (t>>5) & 0x1F ;
			b = (t>>10) & 0x1F ;
			pal[i] = myRGB(r,g,b) ;
		}
	}

	int mwidth , mheight ;
	{
		int adr ;
		adr = GETROM32( BASE_ADR_SROOM_APPERANCE + curshortroom*4 ) ;
		mwidth  = GETROM16( adr ) ;
		mheight = GETROM16( adr+2 ) ;
	}
	if( !mwidth || !mheight )return ;

	if( viewsize < mwidth*mheight*2 )return ;
	if( hitsize  < mwidth*mheight/4 )return ;
	assert( viewsize == mwidth*mheight*2 ) ;
	assert( hitsize == mwidth*mheight/4 ) ;

	{
	int gx,gy ;
		GetGlobalPosition( &gx , &gy ) ;
		MDO3Opt topt = *MDO3normal ;
		topt.flag |= MDO3F_USE_COLOR_TABLE ;
		topt.flag &= ~MDO3F_COLORKEY ;
		MDO3Opt half = *MDO3normal ;
		half.flag |= MDO3F_BLEND ;
		half.alpha = 0xB0 ;
		for( int i=0 ; i<mwidth*mheight ; i++ )
		{
			int tx,ty ;
			tx = gx + i%mwidth * 8 ;
			ty = gy + i/mwidth * 8 ;
int tiledata ;
			tiledata = Pview[i*2] | ( Pview[i*2+1]<<8 ) ;
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
			mdo.Blt( &topt , SFC_BACK , tx , ty , 8 , 8 , SFC_CHR , chrno%16*8 , chrno/16*8 ) ;

			if( drawmode==1 )
			{//あたり判定表示；面倒だけどこれも４分割して
				int offset = (i%mwidth/2)+(i/mwidth/2)*(mwidth/2) ;
				int hitno = Phit[offset] ;
				int lx = (i%mwidth%2)*8 ;
				int ly = (i/mwidth%2)*8 ;
				mdo.Cls( &half , SFC_BACK , tx , ty , 8 , 8 , myRGB(16,16,16) ) ;
				mdo.Blt( MDO3normal , SFC_BACK , tx , ty , 8 , 8 , SFC_CHIP_INFO , hitno%16*16+lx , hitno/16*16+ly ) ;
			}
			if( isshowbrokenchip )
			{
				mdo.Cls( &half , SFC_BACK , tx , ty , 8 , 8 , myRGB(16,16,16) ) ;
			}
		}
	}

	//被せレイヤー表示（文字通り上から被せることで対応している）
	if( isshowbrokenchip )
	{
		int gx,gy ;
		GetGlobalPosition( &gx , &gy ) ;
		int adr ;
		adr = BASE_ADR_COVER_DATA + curshortroom * 4 ;
		adr = GETROM32(adr) ;
		adr = GETROM32(adr+4) ;
		MDO3Opt half = *MDO3normal ;
		half.flag |= MDO3F_BLEND ;
		half.alpha = 0xB0 ;

		for( int i=0 ; i<1024 ; i++ )
		{
			int size ;
			int tx,ty ;
			size = GETROM16( adr + 4 ) ;
			if( !size )break ;
			if( GETROM8(adr) != 1 )
			{
				adr += size ;
				continue ;
			}
			tx = GETROM8( adr + 2 ) ;
			ty = GETROM8( adr + 3 ) ;
			tx *= 16 ;
			ty *= 16 ;
			tx += gx ;
			ty += gy ;

			for( int q=0 ; q<4 ; q++ )
			{
MDO3Opt topt = *MDO3normal ;
				topt.flag |= MDO3F_USE_COLOR_TABLE ;
				topt.flag &= ~MDO3F_COLORKEY ;
int tiledata ;
				tiledata = GETROM16( adr + 8 + q*2 ) ;
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
				mdo.Blt( &topt , SFC_BACK , tx+q%2*8 , ty+q/2*8 , 8 , 8 , SFC_CHR , chrno%16*8 , chrno/16*8 ) ;
			}
			if( drawmode==1 )
			{
				int hitno = GETROM8(adr+0x10) ;
				mdo.Cls( &half , SFC_BACK , tx , ty , 16 , 16 , myRGB(16,16,16) ) ;
				mdo.Blt( MDO3normal , SFC_BACK , tx , ty , 16 , 16 , SFC_CHIP_INFO , hitno%16*16 , hitno/16*16 ) ;
			}
			adr += size ;

		}
	

	}
}


int ModeManager::SolveShortRoom( int shortroom )
{
	for( int i=0 ; i<MAX_LONG_ROOM ; i++ )
	{
int adr ;
		adr  = i*SIZE_OF_LONG_ROOM_PARAMETER ;
		adr += BASE_ADR_LROOM_PARAMETER ;
		adr += 0x24 ;
		adr = GETROM16(adr) ;
		if( shortroom == adr )return i ;
	}
	return -1 ;
}

void ModeManager::ExpRoom()
{
	if( Pview )free( Pview ) ;
	if( Phit  )free( Phit  ) ;
	Pview = NULL ;
	Phit = NULL ;

	{
		int adr ;
		adr = BASE_ADR_SROOM_PARAM + curshortroom*4 ;
		adr = GETROM32(adr) ;
		adr = GETROM32(adr) ;
		if( KagamiDecode( &Phit , &ROM(adr) , NULL , &hitsize ) == -1 )
		{
			hitsize = 0 ;
			return ;
		}
	}
	{
		int adr ;
		adr = BASE_ADR_SROOM_APPERANCE + curshortroom*4 ;
		adr = GETROM32(adr) ;
		adr += 0x18 ;
		adr = GETROM32(adr) ;
		if( YumeDXDecode( &Pview , &ROM(adr) , NULL , &viewsize ) == -1 )
		{
			viewsize = 0 ;
			return ;
		}
	}
}

void ModeManager::RenderChr()
{
//	mdo.LoadBitmapFile( SFC_CHIP_INFO , "chipinfo.bmp" ) ;





	mdo.Cls( MDO3normal , SFC_CHR , 0 , 0 , 128 , 128*4 , myRGB(0,0,0) ) ;
	int adr ;
	adr = BASE_ADR_LROOM_PARAMETER + SIZE_OF_LONG_ROOM_PARAMETER * curlongroom ;
	adr += 0x14 ;
	adr = GETROM16( adr ) ;
	//これで今のロングルームのＢＧ番号取得
	if( adr>=MAX_BG_CHR )return ;

	adr = adr*4 + BASE_ADR_BG_CHR ;
	adr = GETROM32( adr ) ;
	adr = GETROM32( adr ) ;

	BYTE *Ptmp ;
	int rsize , wsize ;
	if( YumeDXDecode( &Ptmp , &ROM(adr) , &rsize , &wsize ) == -1 )return ;

	for( int i=0 ; i<wsize/32 ; i++ )
	{
		DrawGBACharacter( Ptmp+i*32 , SFC_CHR , i%16*8 , i/16*8 , 0  , false , 1 ) ;
	}



	free( Ptmp ) ;
}
