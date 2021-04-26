/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _PNO_MAIN_H_
#define _PNO_MAIN_H_

#include "AdnDebugMgr.h"
#include "PACEInterfaceLib.h"
#include <size_t.h>
#include <cstddef>
//#include "StarterRsc.h"

#include "..\\..\\..\\engine\\sources\\all.h"
using namespace mpp;

extern	int InfoMessage;
extern	int ConfirmAlert;
extern	int message_box;

// Linker still looks for ARMlet_Main as entry point, but the
// "ARMlet" name is now officially discouraged.  Compare an
// contrast to "PilotMain" for 68K applications.
#define PNO_Main ARMlet_Main

// Define ENABLE_PNO_DEBUGGING as 0 to turn off debugging support
// in the PNO.  With this on, the PNO will register itself with
// the Palm OS Debugger on startup, allowing stepping through the code.
#define ENABLE_PNO_DEBUGGING 0


extern "C"	unsigned long PNO_Main(const void *emulStateP, void *userData68KP, Call68KFuncType *call68KFuncP);


#define PACE_SWAP_IN16(param)					\
	if (param) { *(UInt16 *)param = ByteSwap16((UInt16)*param); }

#define PACE_SWAP_IN32(param)					\
	if (param) { *(UInt32 *)param = ByteSwap32((UInt32)*param); }

#define PACE_SWAP_OUT16(param)					\
	if (param) { *(UInt16 *)param = ByteSwap16((UInt16)*param); }

#define PACE_SWAP_OUT32(param)					\
	if (param) { *(UInt32 *)param = ByteSwap32((UInt32)*param); }


//=========================================================================
// функции
extern	void EvtGetEvent(EventType *event, Int32 timeout);
extern	Boolean SysHandleEvent(EventPtr eventP);
extern	Err VFSVolumeEnumerate ( UInt16 *volRefNumP,  UInt32 *volIteratorP);
extern	BitmapType *BmpCreate(Coord width, Coord height, UInt8 depth, ColorTableType *colortableP, UInt16 *error);
extern	Err BmpDelete ( BitmapType *bitmapP);
extern	void *BmpGetBits( BitmapType *bitmapP);
extern	Err	DlkGetSyncInfo(void *succSyncDateP, UInt32 * lastSyncDateP,
		void *syncStateP, Char * nameBufP,
		Char * logBufP, Int32 * logLenP);
extern	void abort();


//======================================================================
//	Отвечает за разбор структуры палмовых событий
//======================================================================
class event_type
{
private:
	UInt8 buffer[sizeof(EventType)];
	
public:
	//--------------------------------------------------------------------
	inline	EventType	*get_ptr()
	{
		return (EventType*)buffer;
	}
	
	//--------------------------------------------------------------------
	UInt16 eType()
	{
		UInt16	v=*(UInt16*)buffer;
		PACE_SWAP_IN16(&v);
		return v;
	}

	//--------------------------------------------------------------------
	point<>	pen_pos()
	{
		Int16	x=*(Int16*)(buffer+4),
				y=*(Int16*)(buffer+6);
		PACE_SWAP_IN16(&x);
		PACE_SWAP_IN16(&y);
		return point<>(x, y);
	}

	//--------------------------------------------------------------------
	char key_char()
	{
		return *(char*)buffer+8;
	}

	//--------------------------------------------------------------------
	UInt16 key_code()
	{
		return (((uint16)buffer[10])<<8)+buffer[9];
	}
};


//======================================================================
extern "C"	void __unexpected(void *catchinfo);


#endif