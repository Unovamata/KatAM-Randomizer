#include "common.h"

void TaskModeEnemy::Finit()
{
	return ;
}
void TaskModeEnemy::Fmain()
{
	return ;
}
void TaskModeEnemy::Fdest()
{
	return ;
}
void TaskModeEnemy::Fdraw()
{
Task0810 *Pparent ;

	Pparent = GetParentTask() ;
	if( !Pparent )return ;
	int room ;
	room = ((ModeManager*)Pparent)->GetShortRoom() ;


	int adr ;
	adr = BASE_ADR_ENEMY_DATA + room * 4 ;
	adr = GETROM32(adr) ;
	adr = GETROM32(adr) ;

int gx,gy ;
char buf[32] ;
	GetGlobalPosition( &gx , &gy , 0 ) ;
	for( int i=0 ; i<256 ; i++ )
	{
		if( GETROM16(adr) == 0 )break ;
		int tx,ty ;

		tx = gx + GETROM16(adr+6) ;
		ty = gy + GETROM16(adr+8) ;

#define ET_SIZE	16
		tx -= ET_SIZE/2 ;
		ty -= ET_SIZE/2 ;
		mdo.Cls( MDO3normal , SFC_BACK , tx , ty , ET_SIZE , ET_SIZE , myRGB(31,31,0) ) ;

		sprintf_s( buf , sizeof(buf) , "%02X" , GETROM8(adr+12) ) ;
		mdo.Text( SFC_BACK , buf , tx    , ty , ET_SIZE , ET_SIZE/2 , 0 ) ;
		adr += 0x24 ;
	}		

/*

int adr;
	adr  = ADR_FIRST_ROOM ;
	adr += room * 4 ;
	adr  = GETROM32( adr ) ;

int noenemy ;
	noenemy = GETROM16( adr+0x3C ) ;
	noenemy = min( noenemy , 128 ) ;

	adr = GETROM32( adr+0x48 ) ;
	for( int i=0 ; i<noenemy ; i++ )
	{
int tx,ty ;
char buf[32] ;
		GetGlobalPosition( &tx , &ty , 0 ) ;
BMPD color ;
		color = myRGB(31,31,0) ;
//		if( selectedet == etnumber )color = myRGB(31,20,0) ;

#define ET_SIZE	16
		tx += GETROM16(adr+4) ;
		ty += GETROM16(adr+6) ;
		tx -= ET_SIZE/2 ;
		ty -= ET_SIZE/2 ;
		mdo.Cls( MDO3normal , SFC_BACK , tx , ty , ET_SIZE , ET_SIZE , color ) ;

		// TTTT TTTT KKKK KKKK PPPP PPPP ?MMP PPPP
		//T...タイプ
		//K...種類
		//P...パラメータ
		//M...マルチプレイ　　00=1P 01=2P 10=3P 11=4P  からそれぞれ存在するようになる
		sprintf_s( buf , sizeof(buf) , "%02X" , GETROM8(adr+1) ) ;
		mdo.Text( SFC_BACK , buf , tx    , ty , ET_SIZE , ET_SIZE/2 , 0 ) ;
		sprintf_s( buf , sizeof(buf) , "%X" , GETROM8(adr+0) ) ;
		BBOXMOJI( SFC_BACK , buf , tx , ty-12 , 12 , 6 , RGB(255,255,255) ) ;
		sprintf_s( buf , sizeof(buf) , "%XP" , ((GETROM8(adr+3)>>5)&3)+1 ) ;
		BBOXMOJI( SFC_BACK , buf , tx+ET_SIZE , ty , 12 , 6 , RGB(255,255,255) ) ;
		sprintf_s( buf , sizeof(buf) , "%X" , GETROM16(adr+2)&0x1FFF ) ;
		BBOXMOJI( SFC_BACK , buf , tx , ty+ET_SIZE , 12 , 6 , RGB(255,255,255) ) ;
		if( GETROM8(adr+3)&0x80 )
		{
			sprintf_s( buf , sizeof(buf) , "★" ) ;
			BBOXMOJI( SFC_BACK , buf , tx , ty+ET_SIZE*2 , 12 , 6 , RGB(255,255,255) ) ;
		}

		adr += 0x08 ;
	}		
*/
}
