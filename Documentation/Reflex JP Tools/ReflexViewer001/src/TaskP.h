#ifndef TASKPRIORITY_HEADER_INCLUDED
#define TASKPRIORITY_HEADER_INCLUDED
enum TaskP
{
	TP_NONE=-3,
	TP_NOVALUE=-2,
	TP_HIGHEST=-1,
////////////////////////
	TP_APP_MANAGER ,
	TP_FRAMEINIT ,

		TP_MODE_MANAGER ,
			TP_MODE_START ,
			TP_MODE_ENEMY ,
			TP_MODE_DOOR  ,

			TP_MODE_END ,
	TP_FRAMEEND ,
////////////////////////
	TP_LOWEST=0x7FFF,
};
#endif /* TASKPRIORITY_HEADER_INCLUDED */