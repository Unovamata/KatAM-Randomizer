#include <windows.h>
#include <assert.h>

int KagamiDecode( BYTE **PPout , BYTE *Psrc , int *Prsize , int *Pwsize )
{
unsigned char *Ptmp ;
int tmpsize = 1024*1024 ;
	Ptmp = (unsigned char*)malloc( tmpsize ) ;
	if( !Ptmp )return -1 ;
	memset( Ptmp , 0 , tmpsize ) ;


int expsize ;
		expsize = (Psrc[3]<<16) | (Psrc[2]<<8) | Psrc[1] ;
int wpos , rpos ;
		wpos = 0 ;
		rpos = 4 ;

int ope,len,val;
		ope = -1 ;

		for(;;)
		{
#define READ(dest)		{(dest)=Psrc[rpos] ;rpos++;}
			if( ope == -1 )
			{
				READ( ope ) ;
				len = ope&0x7F ;
				if( ope&0x80 )
				{
					ope = 1 ;
					len += 3 ;
					READ( val ) ;
				}
				else
				{
					ope = 0 ;
					len ++ ;
				}
			}

			int writevalue ;
			switch( ope )
			{
			case 0:
				{
					READ( writevalue ) ;
					len-- ;
				}
			break ;
			case 1:
				writevalue = val ;
//				WRITE( val ) ;
				len-- ;
			break ;
			}

			if( wpos>=tmpsize )
			{
				tmpsize *= 2 ;
				unsigned char *Pp;
				Pp = Ptmp ;
				Ptmp = (unsigned char*)realloc( Ptmp , tmpsize ) ;
				if( !Ptmp )
				{
					free( Pp ) ;
					return -1 ;
				}
			}

			Ptmp[wpos] = writevalue ;
			wpos ++ ;

			if( !len ) ope = -1 ;

			assert( wpos<=expsize ) ;
			if( wpos==expsize )break ;
#undef READ
		}

unsigned char *Pfo ;
	Pfo = (unsigned char*)malloc( wpos ) ;
	if( !Pfo )
	{
		free( Ptmp ) ;
		return -1 ;
	}
	memcpy( Pfo , Ptmp , wpos ) ;
	free( Ptmp ) ;
	(*PPout) = Pfo ;

	if( Prsize ) (*Prsize) = rpos ;
	if( Pwsize )(*Pwsize) = wpos ;
	return wpos ;
}


#if 0
int KagamiCode( BYTE **PPout , BYTE *Psrc , int srcsize , int padding )
{
/*
	//‘S‘R–¢Š®¬
	BYTE *Pbuf ;

	Pbuf = (BYTE*) malloc( srcsize+srcsize/0x70+0x100 ) ;
int wpos ;
	wpos = 0 ;
#define WRITE(data)		{Pbuf[wpos]=data;wpos++;}

	for( int i=0 ; i<srcsize ; i++ )
	{
		int contcount = 1 ;
		for( int q=1 ; q<srcsize-i ; q++ )
		{
			if( Psrc[0] != Psrc[q] )break ;
			contcount ++ ;
		}
		if( contcount>=3 )
		{
			if( contcount>=0x82 )contcount = 0x82 ;
			WRITE( contcount-3 ) ;
			WRITE( Psrc[0] ) ;
			i += contcount ;
			i-- ;
			continue ;
		}

	}
#undef WRITE
	free( Pbuf ) ;
	return 0 ;
*/

	BYTE *Pbuf ;
	Pbuf = (BYTE*) malloc( srcsize+srcsize/0x70+0x200 ) ;

	int wpos ;
	wpos = 0 ;

	int nc_org = -1 ;
#define		NC_PROCESS(end)									;\
	if( nc_org != -1 )										\
	{														\
		int size ;											\
		size = (end)-nc_org+1 ;								\
		for( int i=0 ; size>0 ; i++,size-=0x80 )			\
		{													\
			int csize ;										\
			csize = min( 0x80 , size ) ;					\
			Pbuf[wpos] = csize-1 ;							\
			wpos ++ ;										\
			memcpy( Pbuf+wpos , Psrc+nc_org+i*0x80 , csize ) ;		\
			wpos += csize ;									\
		}													\
		nc_org = -1 ;										\
	}														/**/

	int i ;
	for( i=0 ; i<srcsize ; i++ )
	{
		int q ;
		for( q=i+1 ; q<srcsize ; q++ )
		{
			if( Psrc[i] != Psrc[q] )break ;
		}
		if( q-i >= 3 )
		{
			NC_PROCESS(i-1) ;
			int csize , csize_rem ;
			csize = min( q-i , 0x7F+3 ) ;
			csize_rem = csize ;
			if( padding && csize>=0x10/* && wpos>=0x300 */)
			{
				if( padding==10 )
				{
					Pbuf[wpos] = 0x03 ;
					wpos++ ;
					for( int il=0 ; il<4 ; il++ )
					{
						Pbuf[wpos] = Psrc[i] ;
						wpos++ ;
					}
					csize_rem -= 4 ;
					padding -= 5 ;
				}
				else if( padding>=9 )
				{
					Pbuf[wpos] = 0x07 ;
					wpos++ ;
					for( int il=0 ; il<8 ; il++ )
					{
						Pbuf[wpos] = Psrc[i] ;
						wpos++ ;
					}
					csize_rem -= 8 ;
					padding -= 9 ;
				}
				else if( padding>=2 )
				{
					Pbuf[wpos] = padding-2 ;
					wpos++ ;
					for( int il=0 ; il<padding-1 ; il++ )
					{
						Pbuf[wpos] = Psrc[i] ;
						wpos++ ;
					}
					csize_rem -= (padding-1) ;
					padding = 0 ;
				}

			}
			Pbuf[wpos] = 0x80|(csize_rem-3) ;
			wpos ++ ;
			Pbuf[wpos] = Psrc[i] ;
			wpos ++ ;
			i += csize ;
			i -- ;
			continue ;
		}
		if( nc_org == -1 )nc_org=i ;
	}
	NC_PROCESS(i-1) ;

	(*PPout) = (BYTE*) malloc( wpos ) ;
	memcpy( (*PPout) , Pbuf , wpos ) ;
	free( Pbuf ) ;
	if( padding )return -1 ;
	return wpos ;

}


#endif