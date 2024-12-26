
class TaskAppManager : public Task0810
{
public:
	TaskAppManager( Task0810 *Pparent , TaskP itaskp , bool handle_enabled ) : Task0810( Pparent , itaskp , handle_enabled )
	{
		this->InitFuncCalled() ;
		Finit();
	}
	TaskAppManager() : Task0810() {;};
protected:
	void Finit() ;
	void Fmain() ;
	void Fdest() ;
	void Fdraw() ;
private:
} ;

class TaskFrameInit : public Task0810
{
public:
	TaskFrameInit( Task0810 *Pparent , TaskP itaskp , bool handle_enabled ) : Task0810( Pparent , itaskp , handle_enabled )
	{
		this->InitFuncCalled() ;
		Finit();
	}
	TaskFrameInit() : Task0810() {;};
protected:
	void Finit(){;} ;
	void Fmain(){;} ;
	void Fdest(){;} ;
	void Fdraw(){;} ;
private:
} ;
class TaskFrameEnd : public Task0810
{
public:
	TaskFrameEnd( Task0810 *Pparent , TaskP itaskp , bool handle_enabled ) : Task0810( Pparent , itaskp , handle_enabled )
	{
		this->InitFuncCalled() ;
		Finit();
	}
	TaskFrameEnd() : Task0810() {;};
	void RequestRedraw(){isredraw=1;};
	void RequestFlip(){isflip=1;};
protected:
	void Finit(){isredraw=isflip=true;} ;
	void Fmain() ;
	void Fdest(){;} ;
	void Fdraw(){;} ;
private:
	int isredraw ;
	int isflip ;
} ;
