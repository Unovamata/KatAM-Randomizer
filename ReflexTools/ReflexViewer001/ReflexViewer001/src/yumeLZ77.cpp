#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#define MY_MAX(a,b)		( (a)<=(b) ? (b) : (a) )

int YumeDXDecode( unsigned char **PPout , unsigned char *Pin , int *Prsize , int *Pwsize )
{
unsigned char *Ptmp ;
int tmpsize = 1024*1024 ;
	Ptmp = (unsigned char*)malloc( tmpsize ) ;
	if( !Ptmp )return -1 ;

	memset( Ptmp , 0 , tmpsize ) ;

int rpos ;
int maxsize ;
	rpos = 0 ;
	maxsize = (Pin[3]<<16) | (Pin[2]<<8) | Pin[1] ;
	rpos += 4 ;

int wpos ;
	wpos = 0 ;
	for(;;)
	{
		bool doublebreak = false ;
		if( wpos+0x100 > tmpsize )
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
int ope ;
		ope = Pin[rpos] ;
		rpos++ ;
		for( int loop=0 ; loop<8 ; loop++ )
		{
			if( !(ope&0x80) )
			{
				Ptmp[wpos] = Pin[rpos] ;
				rpos ++ ;
				wpos ++ ;
			}
			else
			{
int tmp , size , back ;
				tmp = (Pin[rpos]<<8) | Pin[rpos+1] ;
				rpos += 2 ;
				size = (tmp>>12)+3 ;
				back = (tmp&0x0FFF)+1 ;
//				if( back==1 )back = 0x1001 ;//たぶん
				if( wpos-back < 0 )
				{
					free( Ptmp ) ;
					return -1 ;
				}
				for( int i=0 ; i<size ; i++ )
				{
					Ptmp[wpos] = Ptmp[wpos-back] ;
					wpos ++ ;
				}
			}
			ope <<= 1 ;
			if( wpos >= maxsize ){ doublebreak=true ;break ;}
		}
		if( doublebreak )break ;
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


int YumeDXCode( unsigned char **PPout , unsigned char *Pin , int size )
{
unsigned char *Ptmp ;
int tmpsize = 1024*1024 ;
	Ptmp = (unsigned char*)malloc( tmpsize ) ;
	if( !Ptmp )return -1 ;

	memset( Ptmp , 0 , tmpsize ) ;

int rpos = 0 ;
int wpos = 4 ;

int op = 0 ;
int oppos = 0x80 ;

unsigned char opbuf[1+2*8+10] ;
int opbufpos = 1 ;

	Ptmp[0] = 0x10 ; //？？？
	Ptmp[1] = (size    )&0xFF ;
	Ptmp[2] = (size>>8 )&0xFF ;
	Ptmp[3] = (size>>16)&0xFF ;

	for(;;)
	{

		//連続を調べる
int maxval;
int maxpos;
		{
			maxval = -1;
			maxpos = 0;
			//i<rposのほうが自然なのだが……
			for(int i=MY_MAX(rpos-(0xFFF+1/*+1*/),0) ; i<rpos-1 ; i++ )
			{
				int q ;
				for( q=0 ; q<=0x12 ; q++)
				{
					//読み取り位置が、ソース長を超えてたら終了
					if(rpos+q >= size){break;}
					//別のデータなら終了
					if(Pin[i+q]!=Pin[rpos+q])break;
				}
				//q一致
				if(maxval < q)
				{
					maxval = q;
					maxpos = rpos-i;
//					if( maxpos == 0xFFF+2 )maxpos = 1 ; //たぶん
					if( maxval>=0x12 )break ;
				}
			}
		}
		//３以上なら、辞書ったほうが得
		if( maxval >= 3 )
		{
			int tmp ;
			if( maxval > 0xF+3 )maxval =0xF+3 ;
			tmp = ((maxval-3)<<12)|(maxpos-1) ;
			opbuf[opbufpos+0] = (tmp>>8) ;
			opbuf[opbufpos+1] = tmp&0xFF ;
			opbufpos += 2 ;
			rpos += maxval ;
			op |= oppos ;
		}
		else
		{//普通に書き込み
			if( rpos < size )
				opbuf[opbufpos+0] = Pin[rpos] ;
			else
				opbuf[opbufpos+0] = 0x00 ;
			opbufpos += 1 ;
			rpos ++ ;
		}

		oppos >>= 1 ;
		if( !oppos || rpos >= size )
		{//コマンドがいっぱいになったり、もうサイズが終わったりしたらコマンドと作用するものを書き込む
			opbuf[0] = op ;

			if( wpos+opbufpos+0x20>=tmpsize )
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
			memcpy( Ptmp+wpos , opbuf , opbufpos ) ;
			wpos += opbufpos ;
			opbufpos = 1 ;
			op = 0 ;
			oppos = 0x80 ;

			if( rpos >= size )break ;
		}
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

	return wpos ;
}

