#include "common.h"



class ModeManager : public Task0810
{
public:
	ModeManager( Task0810 *Pparent , TaskP itaskp , bool handle_enabled ) : Task0810( Pparent , itaskp , handle_enabled )
	{
		this->InitFuncCalled() ;
		Finit();
	}
	ModeManager() : Task0810() {;};
	int GetShortRoom(){ return curshortroom ; } ;
	int GetLongRoom(){ return curlongroom ; } ;
protected:
	void Finit() ;
	void Fmain() ;
	void Fdest() ;
	void Fdraw() ;
private:
	static const int MAX_SHORT_ROOM = 0x11F ;
	static const int MAX_LONG_ROOM = 0x3D7 ;
	static const int SIZE_OF_LONG_ROOM_PARAMETER = 0x28 ;
	static const int MAX_BG_CHR = 0x22 ;


	static const int BASE_ADR_LROOM_PARAMETER =	0x08903338 ;
	static const int BASE_ADR_BG_PAL =			0x08D2E6A4 ;
	static const int BASE_ADR_SROOM_PARAM =		0x08D2E74C ;
	static const int BASE_ADR_SROOM_APPERANCE =	0x08D2F93C ;
	static const int BASE_ADR_BG_CHR =			0x08D2FDB8 ;
	static const int BASE_ADR_COVER_DATA =		0xD2E74C ;


	int SolveShortRoom( int shortroom ) ;
	int GetOffset_LongRoomParam() ;
	void ExpRoom() ;
	void RenderChr() ;

	int mode ;
	int drawmode ;
	bool isshowbrokenchip ;
	int curshortroom ;
	int curlongroom ;

	BYTE *Pview ;
	int  viewsize ;
	BYTE *Phit ;
	int  hitsize ;
//	BYTE *Pvariableno ;
//	int  variablenosize ;



} ;

