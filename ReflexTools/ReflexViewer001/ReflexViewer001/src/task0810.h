/*
	いろいろ破綻した、自称タスクシステムのルーチン。
	本当にタスクシステムなんだかよくわからない。
	を、すこし改良したタスクシステム。のようなもの。
*/

#include <windows.h>
#include "TaskP.h"

#ifndef TASK0810_HEADER_INCLUDED
#define TASK0810_HEADER_INCLUDED


#define	TASK0810_HANDLE_TABLE_SIZE_BITS		9
#define	TASK0810_HANDLE_TABLE_SIZE			(1<<TASK0810_HANDLE_TABLE_SIZE_BITS)
#define TASK0810_HANDLE_MASK				(TASK0810_HANDLE_TABLE_SIZE-1)
#define TASK0810_HANDLE_MASK_DUP			(~TASK0810_HANDLE_MASK)


typedef unsigned __int32 TASKHANDLE ;

//タスクのフラグ
#define	TASK_SLEEP					0x0001
#define	TASK_SLEEP_PROCESS			0x0002
#define	TASK_SLEEP_DRAW				0x0004
#define TASK_HOLD_HANDLE			0x0010
#define TASK_DEATH_PRESERVED		0x0020
#define TASK_INIT_CALLED			0x0040
#define TASK_PROCESS_BEFORE_CHILD	0x0100
#define TASK_PROCESS_AFTER_CHILD	0x0200
#define TASK_DRAW_BEFORE_CHILD		0x0400
#define TASK_DRAW_AFTER_CHILD		0x0800
#define TASK_CALLED_BEFORE_CHILD	0x1000

#define	TASK_DEFAULT				(TASK_PROCESS_BEFORE_CHILD | TASK_DRAW_AFTER_CHILD )


class Task0810 ;

class Task0810
{
public:
	Task0810( Task0810 *Pparent , TaskP itaskp , bool handle_enabled ) ;
	Task0810() ;
	~Task0810(){;};
//	void * operator new(size_t size){return handle ;};

	void SetChildPointer( Task0810 *Pdest , Task0810 **PPp , Task0810 **PPn ) ;
	void DetachChild( Task0810 *Pdest ) ;
	void SetNextPointer( Task0810 *Pdest ) ;
	void SetPrepPointer( Task0810 *Pdest ) ;

	TaskP GetKind(){return kind;}
	TASKHANDLE GetHandle(){return handle;}

	void DoCN() ; //子供と、次のメイン関数も実行。子供の前後に自分の関数も実行
	void DrawCN() ; //子供と、次のメインの描画。子供の前後に自分も描画。
	void KillP( TaskP org , TaskP end ) ;
	void Kill( Task0810 *Pdest ) ;

	//タスクを寝かす
	void Sleep(){flag|=TASK_SLEEP;};
	//タスクを起こす
	void Wake(){flag&=~TASK_SLEEP;};
	//タスク処理を寝かす
	void SleepProcess(){flag|=TASK_SLEEP_PROCESS;};
	//タスク処理を起こす
	void WakeProcess(){flag&=~TASK_SLEEP_PROCESS;};
	//タスク描画を寝かす
	void SleepDraw(){flag|=TASK_SLEEP_DRAW;};
	//タスク描画を起こす
	void WakeDraw(){flag&=~TASK_SLEEP_DRAW;};
	//自殺予約
	void Suicide(){flag|=TASK_DEATH_PRESERVED;};

	void SetProcessOrderPandC( bool ProcessBeforeChild , bool ProcessAfterChild )
	{
		flag &= ~(TASK_PROCESS_BEFORE_CHILD|TASK_PROCESS_AFTER_CHILD) ;
		flag |= (ProcessBeforeChild*TASK_PROCESS_BEFORE_CHILD)|(ProcessAfterChild*TASK_PROCESS_AFTER_CHILD ) ;
	}
	void SetDrawOrderPandC( bool DrawBeforeChild , bool DrawAfterChild )
	{
		flag &= ~(TASK_DRAW_BEFORE_CHILD|TASK_DRAW_AFTER_CHILD) ;
		flag |= (DrawBeforeChild*TASK_DRAW_BEFORE_CHILD)|(DrawAfterChild*TASK_DRAW_AFTER_CHILD ) ;
	}


	//もう初期化関数は呼んだ
	void InitFuncCalled(){flag|=TASK_INIT_CALLED;};

	TASKHANDLE SearchTask( TaskP ifrom , TaskP ifor = TP_NOVALUE );
	static Task0810 *SolveHandle( TASKHANDLE ihandle ) ;

	//親をたどっていき、x,yを足し続ける
	//たどる回数を指定したいときは、depthを与える（デフォルトでは、ルートまで）
	void GetGlobalPosition( INT32 *Pox , INT32 *Poy , int depth=0 )
	{
Task0810 *Ptmp;
__int32 tx,ty;
		tx = ty = 0 ;
		for( Ptmp = this ; Ptmp ; Ptmp=Ptmp->PparentTask )
		{
			tx += Ptmp->x ;
			ty += Ptmp->y ;
			depth -- ;
			if( !depth ) break;
		}
		*Pox = tx ;
		*Poy = ty ;
	}

#ifdef _DEBUG
	Task0810 *GetPprepTask  (){return PprepTask  ;}
	Task0810 *GetPnextTask  (){return PnextTask  ;}
	Task0810 *GetPCheadTask (){return PCheadTask ;}
	Task0810 *GetPCtailTask (){return PCtailTask ;}
	Task0810 *GetPparentTask(){return PparentTask;}
	static void GetHandleState( int *PValidHandle , int *PNOHandle , Task0810 ***PAPptr ) ;
#endif

protected:
	__int32 x;
	__int32 y;

	virtual void Finit(){;}; //タスク作成時関数
	virtual void Fmain(){;}; //タスク関数
	virtual void Fdest(){;}; //デストラクタ関数
	virtual void Fdraw(){;}; //描画関数

	//ところでprepってなんだろ。
	//なにか覚え間違えの可能性が……
	Task0810 *GetPrepTask(){ return PprepTask ; }
	Task0810 *GetNextTask(){ return PnextTask ; }
	Task0810 *GetCheadTask(){ return PCheadTask ; }
	Task0810 *GetCtailTask(){ return PCtailTask ; }
	Task0810 *GetParentTask(){ return PparentTask ; }
private:
	TaskP kind;                 //TCBの優先・種類

	unsigned __int16 flag;      //フラグ

	TASKHANDLE handle ;

	Task0810 *PprepTask;        //前のタスク
	Task0810 *PnextTask;        //次のタスク
	Task0810 *PCheadTask;       //子供の先頭
	Task0810 *PCtailTask;       //子供の最後尾
	Task0810 *PparentTask;      //親

	void Dispose() ;
	void Die() ;

	static TASKHANDLE handle_next ;
};


#endif /* TASK0810_HEADER_INCLUDED */
