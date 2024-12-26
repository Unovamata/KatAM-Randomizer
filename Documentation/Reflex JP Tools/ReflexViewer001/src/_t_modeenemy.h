class TaskModeEnemy : public Task0810
{
public:
	TaskModeEnemy( Task0810 *Pparent , TaskP itaskp , bool handle_enabled ) : Task0810( Pparent , itaskp , handle_enabled )
	{
		this->InitFuncCalled() ;
		Finit();
	}
	TaskModeEnemy() : Task0810() {;};
protected:
	void Finit() ;
	void Fmain() ;
	void Fdest() ;
	void Fdraw() ;
private:
	static const int BASE_ADR_ENEMY_DATA = 0xD2EBC8 ;
} ;

