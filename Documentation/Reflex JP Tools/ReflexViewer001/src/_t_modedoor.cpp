#include "common.h"

void TaskModeDoor::Finit()
{
	return ;
}
void TaskModeDoor::Fmain()
{
	return ;
}
void TaskModeDoor::Fdest()
{
	return ;
}
void TaskModeDoor::Fdraw()
{
Task0810 *Pparent ;

	Pparent = GetParentTask() ;
	if( !Pparent )return ;
	int room ;
	room = ((ModeManager*)Pparent)->GetShortRoom() ;


	int adr ;
	adr = BASE_ADR_COVER_DATA + room * 4 ;
	adr = GETROM32(adr) ;
	adr = GETROM32(adr+4) ;

int gx,gy ;
char buf[32] ;
	GetGlobalPosition( &gx , &gy , 0 ) ;



	for( int i=0 ; i<1024 ; i++ )
	{
int size ;
int tx,ty ;
		size = GETROM16( adr + 4 ) ;
		if( !size )break ;
		if( GETROM8(adr) != 3 )
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

		mdo.Cls( MDO3normal , SFC_BACK , tx , ty , 16 , 16 , myRGB(14,22,31) ) ;
		sprintf_s( buf , sizeof(buf) , "%03X" , GETROM16( adr+8 ) ) ;
		mdo.Text( SFC_BACK , buf , tx-1    , ty , 16 , 6 , 0 ) ;

		sprintf_s( buf , sizeof(buf) , "(%02X,%02X)%02X" , GETROM8( adr+8+2 ) , GETROM8( adr+8+3 ) , GETROM8( adr+8+4 ) );
		BBOXMOJI( SFC_BACK , buf , tx    , ty+16 , 14 , 7 , RGB(255,255,255) ) ;
		adr += size ;

	}
/*
Task0810 *Pparent ;

	Pparent = GetParentTask() ;
	if( !Pparent )return ;
	int room ;
	room = ((ModeManager*)Pparent)->GetCurRoom() ;

int adr;
	adr  = ADR_FIRST_ROOM ;
	adr += room * 4 ;
	adr  = GETROM32( adr ) ;

int nodoor ;

	nodoor = GETROM16( adr+0x3A ) ;
	adr  = GETROM32( adr+0x44 ) ;

	//ï Ç…Ç¢Ç¢ÇØÇ«ÅAÇUÇTÇTÇRÇTå¬Ç∆Ç©Ç…Ç»ÇÈÇ∆ÉAÉåÇæÇ©ÇÁÅcÅc
	nodoor = min( nodoor , 64 ) ;

#define DT_SIZE		16
	for( int i=0 ; i<nodoor ; i++ )
	{
int tx,ty ;
char buf[32] ;

		GetGlobalPosition( &tx , &ty , 0 ) ;
		tx += GETROM16( adr+2 )*16 ;
		ty += GETROM16( adr+4 )*16 ;
BMPD color ;
		color = myRGB(13,22,31) ;
//		if( selectedet == etnumber )color = myRGB(31,20,0) ;
		mdo.Cls( MDO3normal , SFC_BACK , tx , ty , DT_SIZE , DT_SIZE , color ) ;

		if( GETROM16(adr+10)<0x100 )
		{
			sprintf_s( buf , sizeof(buf) , "%02X" , GETROM8(adr+10) ) ;
			mdo.Text( SFC_BACK , buf , tx    , ty , DT_SIZE , DT_SIZE/2 , 0 ) ;
		}
		else
		{
			sprintf_s( buf , sizeof(buf) , "%02X" , GETROM8(adr+11) ) ;
			mdo.Text( SFC_BACK , buf , tx    , ty , DT_SIZE/2 , DT_SIZE/2 , 0 ) ;
			sprintf_s( buf , sizeof(buf) , "%02X" , GETROM8(adr+10) ) ;
			mdo.Text( SFC_BACK , buf , tx    , ty+DT_SIZE/2 , DT_SIZE/2 , DT_SIZE/2 , 0 ) ;
		}
		sprintf_s( buf , sizeof(buf) , "Å®%0X(%0X,%0X)" , GETROM16(adr+0) , GETROM16(adr+6) , GETROM16(adr+8) ) ;
		BBOXMOJI( SFC_BACK , buf , tx , ty+DT_SIZE , 12 , 6 , RGB(255,255,255) ) ;
		adr += 12 ;
	}
*/
}
