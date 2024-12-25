/*
	���낢��j�]�����A���̃^�X�N�V�X�e���̃��[�`���B
	�{���Ƀ^�X�N�V�X�e���Ȃ񂾂��悭�킩��Ȃ��B
	���A���������ǂ����^�X�N�V�X�e���B�̂悤�Ȃ��́B
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

//�^�X�N�̃t���O
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

	void DoCN() ; //�q���ƁA���̃��C���֐������s�B�q���̑O��Ɏ����̊֐������s
	void DrawCN() ; //�q���ƁA���̃��C���̕`��B�q���̑O��Ɏ������`��B
	void KillP( TaskP org , TaskP end ) ;
	void Kill( Task0810 *Pdest ) ;

	//�^�X�N��Q����
	void Sleep(){flag|=TASK_SLEEP;};
	//�^�X�N���N����
	void Wake(){flag&=~TASK_SLEEP;};
	//�^�X�N������Q����
	void SleepProcess(){flag|=TASK_SLEEP_PROCESS;};
	//�^�X�N�������N����
	void WakeProcess(){flag&=~TASK_SLEEP_PROCESS;};
	//�^�X�N�`���Q����
	void SleepDraw(){flag|=TASK_SLEEP_DRAW;};
	//�^�X�N�`����N����
	void WakeDraw(){flag&=~TASK_SLEEP_DRAW;};
	//���E�\��
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


	//�����������֐��͌Ă�
	void InitFuncCalled(){flag|=TASK_INIT_CALLED;};

	TASKHANDLE SearchTask( TaskP ifrom , TaskP ifor = TP_NOVALUE );
	static Task0810 *SolveHandle( TASKHANDLE ihandle ) ;

	//�e�����ǂ��Ă����Ax,y�𑫂�������
	//���ǂ�񐔂��w�肵�����Ƃ��́Adepth��^����i�f�t�H���g�ł́A���[�g�܂Łj
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

	virtual void Finit(){;}; //�^�X�N�쐬���֐�
	virtual void Fmain(){;}; //�^�X�N�֐�
	virtual void Fdest(){;}; //�f�X�g���N�^�֐�
	virtual void Fdraw(){;}; //�`��֐�

	//�Ƃ����prep���ĂȂ񂾂�B
	//�Ȃɂ��o���ԈႦ�̉\�����c�c
	Task0810 *GetPrepTask(){ return PprepTask ; }
	Task0810 *GetNextTask(){ return PnextTask ; }
	Task0810 *GetCheadTask(){ return PCheadTask ; }
	Task0810 *GetCtailTask(){ return PCtailTask ; }
	Task0810 *GetParentTask(){ return PparentTask ; }
private:
	TaskP kind;                 //TCB�̗D��E���

	unsigned __int16 flag;      //�t���O

	TASKHANDLE handle ;

	Task0810 *PprepTask;        //�O�̃^�X�N
	Task0810 *PnextTask;        //���̃^�X�N
	Task0810 *PCheadTask;       //�q���̐擪
	Task0810 *PCtailTask;       //�q���̍Ō��
	Task0810 *PparentTask;      //�e

	void Dispose() ;
	void Die() ;

	static TASKHANDLE handle_next ;
};


#endif /* TASK0810_HEADER_INCLUDED */
