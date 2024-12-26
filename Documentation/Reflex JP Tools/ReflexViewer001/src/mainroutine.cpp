#include "common.h"



void MainRoutine(int fps)
{
	Task0810 *Ptask ;
	Ptask = Task0810::SolveHandle( HAM ) ;
	if( Ptask )Ptask->DoCN() ;
	else DestroyWindow( hWnd ) ;
}
