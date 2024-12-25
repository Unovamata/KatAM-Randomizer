#ifndef GBA_SYS_HEADER_INCLUDED
#define GBA_SYS_HEADER_INCLUDED


/*
mode0...16
mode1...256
*/
void SetGBAPalette( UINT *Pict16 , UINT *Pict256 ) ;
void SetGBADrawRoutine( void (*PiCLSFUNK)(int,int,int,int,int,UINT) , UINT *Pict16 , UINT *Pict256 ) ;
extern void DrawGBACharacter( BYTE *Pbuf , int sfc , int x , int y , int mode , bool emphasis , int zoom);


#endif GBA_SYS_HEADER_INCLUDED