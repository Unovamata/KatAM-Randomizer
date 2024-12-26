class TaskModeDoor : public Task0810
{
public:
	TaskModeDoor( Task0810 *Pparent , TaskP itaskp , bool handle_enabled ) : Task0810( Pparent , itaskp , handle_enabled )
	{
		this->InitFuncCalled() ;
		Finit();
	}
	TaskModeDoor() : Task0810() {;};
protected:
	void Finit() ;
	void Fmain() ;
	void Fdest() ;
	void Fdraw() ;
private:
	static const int BASE_ADR_COVER_DATA = 0xD2E74C ;

} ;

