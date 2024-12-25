#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

#ifdef _ORE_SENYOU
//私はいくつかのソースをライブラリ化しているので……
#ifdef     _DEBUG

#ifdef         _UNICODE
#pragma comment(lib,"glw_d.lib")
#else          /*_UNICODE*/
#pragma comment(lib,"gl_d.lib")
#endif         /*_UNICODE*/

#else      /*_DEBUG*/

#ifdef         _UNICODE
#pragma comment(lib,"glw.lib")
#else          /*_UNICODE*/
#pragma comment(lib,"gl.lib")
#endif         /*_UNICODE*/

#endif     /*_DEBUG*/

#endif /*_ORE_SENYOU*/

#include "mydibobj3.h"
#include "mymemorymanage.h"
#include "KirbyCode/yumeLZ77.h"
#include "HT/GBAsys.h"

