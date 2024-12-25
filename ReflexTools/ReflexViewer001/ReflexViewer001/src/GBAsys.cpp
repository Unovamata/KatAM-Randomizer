#include <windows.h>
#include "GBAsys.h"

static BYTE looptable[2] = {4,8};
static BYTE bittable[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
static BYTE bittabler[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};


static UINT *Pcolortable16 ;
static UINT *Pcolortable256 ;
static void (*PCLSFUNK)(int sfc , int x , int y , int w , int h , UINT color );

void SetGBAPalette( UINT *Pict16 , UINT *Pict256 )
{
	Pcolortable16 = Pict16 ;
	Pcolortable256 = Pict256 ;
}
void SetGBADrawRoutine( void (*PiCLSFUNK)(int,int,int,int,int,UINT) , UINT *Pict16 , UINT *Pict256 )
{
	PCLSFUNK      = PiCLSFUNK ;
	SetGBAPalette( Pict16 , Pict256 ) ;
}


void DrawGBACharacter( BYTE *Pbuf , int sfc , int x , int y , int mode , bool emphasis , int zoom)
{
int ix,iy;
	if( mode < 0 || mode >= 2 )return;
	if( !mode )
	{
		for( iy=0 ; iy<8 ; iy++ )
		{
			for( ix=0 ; ix<8 ; ix++ )
			{
UINT tcolor ;
				tcolor = Pbuf[(iy*8+ix)/2] ;
				if( ix%2 )tcolor>>=4 ;
				tcolor &= 0x0F ;
				if( emphasis )
					tcolor = Pcolortable16[tcolor] ;
				PCLSFUNK( sfc , x+ix*zoom , y+iy*zoom , zoom , zoom , tcolor ) ;
			}
		}
	}
	else
	{
		for( iy=0 ; iy<8 ; iy++ )
		{
			for( ix=0 ; ix<8 ; ix++ )
			{
UINT tcolor = Pbuf[(iy*8+ix)] ;
				if( emphasis )
					tcolor = Pcolortable256[tcolor] ;
				PCLSFUNK( sfc , x+ix*zoom , y+iy*zoom , zoom , zoom , tcolor ) ;
			}
		}
	}
}