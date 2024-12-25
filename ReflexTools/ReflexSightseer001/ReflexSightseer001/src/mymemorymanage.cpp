/*
	よく意図のわからないメモリ管理ルーチン。
*/
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <memory.h>
#include <assert.h>

#include "mymemorymanage.h"



//一息で読む量
#define READLENGTH	0x00100000

static int ret;
#define	ERRRET(tok,num)		{ret=-num;goto tok##num;}


/*	ファイルから空のptrbufにメモリを割り当てかつロード。
	ファイルサイズを返す。
	-1	バッファが空でない
	-2　一時バッファが確保できない
	-3　ファイルがオープンできない
	-4　ファイルが読めない
	-5　一時バッファを拡張できない
	-6　返すべきバッファが確保できない
*/
int LoadMemoryFromFile( LPCTSTR filename , unsigned char **ptrbuf)
{
	//バッファをチェック
	if(*ptrbuf != NULL)return -1 ;

	FILE *fp ;
	if( _tfopen_s( &fp , filename , _T("rb") ) ) return -3 ;

	int size ;
	fseek( fp , 0 , SEEK_END ) ;
	size = ftell( fp ) ;
	fseek( fp , 0 , SEEK_SET ) ;

	//実際に返すバッファを確保
	*ptrbuf = (unsigned char*)malloc(size);
	if(*ptrbuf == NULL){fclose(fp);return -6;}

	fread( *ptrbuf , 1 , size , fp ) ;

	fclose( fp ) ;
	return size ;
}

/*
	メモリをファイルに書き出す。
	0	正常終了
	-1	ファイルが開けない
	-2	ファイルに書けない
*/
int WriteFileFromMemory( LPCTSTR filename , unsigned char *data , int length )
{
FILE *fp ;
	if( _tfopen_s( &fp , filename , _T("wb") ) )ERRRET(ERRWFFM,1);
	if( fwrite( data , 1 , length , fp ) < (size_t)length )ERRRET(ERRWFFM,2);
	/*
int filehand;
	filehand = _open(filename , _O_CREAT | _O_BINARY | _O_TRUNC | _O_WRONLY , _S_IREAD | _S_IWRITE );
	if(filehand == -1)ERRRET(ERRWFFM,1);
	if(_write(filehand , data , length) < length)ERRRET(ERRWFFM,2);
	*/
	ret = 0;
ERRWFFM2:
//	_close(filehand);
	fclose( fp ) ;
ERRWFFM1:
	return ret;
}

#define		EXPAND_SIZE_AT_ONCE		0x0100
#define		BUFFER_SIZE_LIMIT		0x2000

/*
	0...正常終了
	1...終端まで読み込んで終了
	-1..読み込み失敗
	-2..メモリ確保失敗
*/
int ReadALine( int filehand , char **PPreturnbuf)
{
	char *Pbuffer=NULL;
	int buffersize=0;
	int i;
	for( i=0 ;; i++)
	{
		if(buffersize <= i)
		{
			if(!Pbuffer)
			{
				Pbuffer = (char*)malloc(sizeof(char)*EXPAND_SIZE_AT_ONCE);
			}
			else
			{
				if( buffersize >= BUFFER_SIZE_LIMIT )
				{
					Pbuffer[i-1] = '\0';
					*PPreturnbuf = Pbuffer;
					return -2;
				}
				Pbuffer = (char*)realloc(&Pbuffer , buffersize+sizeof(char)*EXPAND_SIZE_AT_ONCE);
			}
			if( !Pbuffer )
			{
				*PPreturnbuf = NULL;
				return -2;
			}
			buffersize += EXPAND_SIZE_AT_ONCE;
		}
		switch(_read( filehand , &Pbuffer[i] , 1 ))
		{
		case 1:
			if(Pbuffer[i] == '\n')
			{
				Pbuffer[i] = '\0';
				*PPreturnbuf = Pbuffer;
				return 0;
			}
		break;
		case 0:
			Pbuffer[i] = '\0';
			*PPreturnbuf = Pbuffer;
			return 1;
		break;
		default:
			Pbuffer[i] = '\0';
			*PPreturnbuf = Pbuffer;
			return 1;
		}
	}
	assert(0);
	return -1;
}

/*
	一行書く。うえのほうのと比べても手抜きなのは明白で、あまり使えない
*/
int WriteALine( int filehand , char *Pbuffer )
{
//int rv;
	_write( filehand , Pbuffer , strlen(Pbuffer) );
	return 0;
}

#if 0
int MMMOpenForTextInput( char *Pname )
{
	return _open( Pname , _O_TEXT | _O_RDONLY);
}
int MMMOpenForTextOutput( char *Pname )
{
	return _open( Pname , _O_CREAT | _O_TEXT | _O_TRUNC | _O_WRONLY , _S_IREAD | _S_IWRITE );
}
int MMMOpenForBinaryInput( char *Pname )
{
	return _open( Pname , _O_BINARY | _O_RDONLY);
}
int MMMOpenForBinaryOutput( char *Pname )
{
	return _open( Pname , _O_CREAT | _O_BINARY | _O_TRUNC | _O_WRONLY , _S_IREAD | _S_IWRITE );
}
int MMMClose( int filehand )
{
	return _close( filehand ) ;
}
#endif