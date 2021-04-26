/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "pnomain.h"
#include "palmos5.h"


int InfoMessage=0;
int ConfirmAlert=0;
int message_box=0;
int abort_programm=0;

int errno=0;


static	struct EmulStateType 
{
	UInt32 instr;
	UInt32 regData[8];
	UInt32 regAddress[8];
	UInt32 regPC;
}*g_emulStateP=NULL;

static	Call68KFuncType *g_call68KFuncP=NULL;

void *glue_new;

#define	PACE_EMULSTATE	g_emulStateP


#if PACE_INTERFACE_AS_CLASS

	#define PACE_CLASS_WRAPPER(rv)		rv PACEInterface::
	#define PACE_CALLBACK_PTR			m_call68KFuncP
	#define PACE_EMULSTATE				m_emulStateP

#else

	#define PACE_CLASS_WRAPPER(rv)		extern "C" rv 
	#define PACE_CALLBACK_PTR			g_call68KFuncP
	#define PACE_EMULSTATE				g_emulStateP

#endif


#define PACE_PARAMS_INIT()						\
	UInt8 params[] = {
	
#define PACE_PARAMS_ADD8(param)					\
	(UInt8)(param),								\
	0,

#define PACE_PARAMS_ADD16(param)				\
	(UInt8)((UInt16)(param) >> 8),				\
	(UInt8)(param),

#define PACE_PARAMS_ADD32(param)				\
	(UInt8)((UInt32)(param) >> 24),				\
	(UInt8)((UInt32)(param) >> 16),				\
	(UInt8)((UInt32)(param) >> 8),				\
	(UInt8)(param),

// basic form
#define PACE_EXEC(trap, returnType)				\
	};											\
	return ((returnType)((PACE_CALLBACK_PTR)(	\
		static_cast<void *>(PACE_EMULSTATE),	\
		PceNativeTrapNo(trap),					\
		&params,								\
		sizeof(params))));

// pointer returning variant	
#define PACE_EXEC_RP(trap, returnType)			\
	};											\
	return ((returnType)((PACE_CALLBACK_PTR)(	\
		static_cast<void *>(PACE_EMULSTATE),	\
		PceNativeTrapNo(trap),					\
		&params,								\
		sizeof(params) | kPceNativeWantA0)));

// no return value variant
#define PACE_EXEC_NRV(trap)						\
	};											\
	PACE_CALLBACK_PTR(							\
		static_cast<void *>(PACE_EMULSTATE),	\
		PceNativeTrapNo(trap),					\
		&params,								\
		sizeof(params));
		
// no parameters version
#define PACE_EXEC_NP(trap, returnType)			\
	return ((returnType)((PACE_CALLBACK_PTR)(	\
		static_cast<void *>(PACE_EMULSTATE),	\
		PceNativeTrapNo(trap),					\
		NULL, 0)));

// no parameters, returns pointer
#define PACE_EXEC_NP_RP(trap, returnType)		\
	return ((returnType)((PACE_CALLBACK_PTR)(	\
		static_cast<void *>(PACE_EMULSTATE),	\
		PceNativeTrapNo(trap),					\
		NULL, kPceNativeWantA0)));

// no parameters, returns nothing
#define PACE_EXEC_NP_NRV(trap)					\
	(void)PACE_CALLBACK_PTR(					\
		static_cast<void *>(PACE_EMULSTATE),	\
		PceNativeTrapNo(trap),					\
		NULL, 0);

// basic form, delayed return
#define PACE_EXEC_DELAY(trap, returnType)		\
	};											\
	returnType retval =							\
		((returnType)((PACE_CALLBACK_PTR)(		\
		static_cast<void *>(PACE_EMULSTATE),	\
		PceNativeTrapNo(trap),					\
		&params,								\
		sizeof(params))));

// basic form, delayed return, returns pointer
#define PACE_EXEC_DELAY_RP(trap, returnType)	\
	};                                          \
    returnType retval =                         \
        ((returnType)((PACE_CALLBACK_PTR)(      \
        static_cast<void *>(PACE_EMULSTATE),    \
        PceNativeTrapNo(trap),                  \
        &params,                                \
        sizeof(params) | kPceNativeWantA0)));

// return from a delayed call
#define PACE_RETURN()							\
	return retval;

// basic VFS form
#define PACE_VFS_EXEC(vfsTrap, returnType)		\
	 };											\
	PACE_EMULSTATE->regData[2] = vfsTrap;		\
	return ((returnType)((PACE_CALLBACK_PTR)(	\
		static_cast<void *>(PACE_EMULSTATE),	\
		PceNativeTrapNo(sysTrapFileSystemDispatch), \
		&params,								\
		sizeof(params))));

// basic VFS form, delayed return
#define PACE_VFS_EXEC_DELAY(vfsTrap, returnType) \
	 };											\
	PACE_EMULSTATE->regData[2] = vfsTrap;		\
	returnType retval =							\
		((returnType)((PACE_CALLBACK_PTR)(		\
		static_cast<void *>(PACE_EMULSTATE),	\
		PceNativeTrapNo(sysTrapFileSystemDispatch), \
		&params,								\
		sizeof(params))));






//======================================================================
void EvtGetEvent(EventType *event, Int32 timeout)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD32(event)
	PACE_PARAMS_ADD32(timeout)
	PACE_EXEC_NRV(sysTrapEvtGetEvent)
}



//======================================================================
Boolean SysHandleEvent(EventPtr eventP)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD32(eventP)
	PACE_EXEC(sysTrapSysHandleEvent, Boolean)
}

/*
//======================================================================
Err MemHeapFreeBytes(UInt16 heapID, UInt32* freeP, UInt32* maxP)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD16(heapID)
	PACE_PARAMS_ADD32(freeP)
	PACE_PARAMS_ADD32(maxP)
	PACE_EXEC(sysTrapMemHeapFreeBytes, Err)
}


//======================================================================
Boolean MemHeapDynamic(UInt16 heapID)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD16(heapID)
	PACE_EXEC(sysTrapMemHeapDynamic, Boolean)
}
*/

//======================================================================
BitmapType *BmpCreate(Coord width, Coord height, UInt8 depth, ColorTableType *colortableP, UInt16 *error)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD16(width)
   	PACE_PARAMS_ADD16(height)
   	PACE_PARAMS_ADD8(depth)
	PACE_PARAMS_ADD32(colortableP)
	PACE_PARAMS_ADD32(error)
	PACE_EXEC_RP(sysTrapBmpCreate, BitmapType *)
}



//======================================================================
Err BmpDelete ( BitmapType *bitmapP)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD32(bitmapP)
	PACE_EXEC(sysTrapBmpDelete, Err);
}

//======================================================================
UInt32 KeyCurrentState ()
{
	PACE_EXEC_NP(sysTrapKeyCurrentState, UInt32);
}


//======================================================================
void *BmpGetBits( BitmapType *bitmapP)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD32(bitmapP)
	PACE_EXEC_RP(sysTrapBmpGetBits, void*);
}


//======================================================================
WinHandle WinCreateOffscreenWindow (Coord width, Coord height, WindowFormatType format, UInt16 *error)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD16(width)
   	PACE_PARAMS_ADD16(height)
   	PACE_PARAMS_ADD8(format)
	PACE_PARAMS_ADD32(error)
	PACE_EXEC_RP(sysTrapWinCreateOffscreenWindow, WinHandle)
}



//======================================================================
Err VFSVolumeEnumerate ( UInt16 *vol,  UInt32 *it)
{
	PACE_SWAP_IN16(vol)
	PACE_SWAP_IN32( it )
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD32(vol)
	PACE_PARAMS_ADD32(it)
	PACE_VFS_EXEC_DELAY(vfsTrapVolumeEnumerate, Err)
	PACE_SWAP_OUT16(vol)
	PACE_SWAP_OUT32(it)
	PACE_RETURN()
}


//======================================================================
Err VFSDirEntryEnumerate(FileRef dirRef, UInt32 *dirEntryIteratorP, FileInfoType *infoP)
{
	PACE_SWAP_IN32(dirEntryIteratorP)
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD32(dirRef)
	PACE_PARAMS_ADD32(dirEntryIteratorP)
	PACE_PARAMS_ADD32(infoP)
	PACE_VFS_EXEC_DELAY(vfsTrapDirEntryEnumerate, Err)
    PACE_SWAP_OUT32(dirEntryIteratorP)
    PACE_RETURN()
}

//======================================================================
Err VFSDirCreate(UInt16 volRefNum, const Char *dirNameP)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD16(volRefNum)
	PACE_PARAMS_ADD32(dirNameP)
	PACE_VFS_EXEC_DELAY(vfsTrapDirCreate, Err)
    PACE_RETURN()
}

//======================================================================
Err VFSFileDelete(UInt16 volRefNum, const Char *pathNameP)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD16(volRefNum)
	PACE_PARAMS_ADD32(pathNameP)
	PACE_VFS_EXEC_DELAY(vfsTrapFileDelete, Err)
    PACE_RETURN()
}

/*
//======================================================================
Err SndStreamCreate(
   SndStreamRef *streamP,
   SndStreamMode mode,
   UInt32 sampleRate,
   SndSampleType type,
   SndStreamWidth width,
   SndStreamBufferCallback callback,
   void *callbackArgP,
   UInt32 bufferSize,
   Boolean callbackIsARM
)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD32(streamP)
   	PACE_PARAMS_ADD8(mode)
	PACE_PARAMS_ADD32(sampleRate)
	PACE_PARAMS_ADD16(type)
   	PACE_PARAMS_ADD8(width)
	PACE_PARAMS_ADD32(callback)
	PACE_PARAMS_ADD32(callbackArgP)
	PACE_PARAMS_ADD32(bufferSize)
   	PACE_PARAMS_ADD8(callbackIsARM)
	PACE_EXEC_DELAY(sysTrapSndStreamCreate, Err)
	PACE_SWAP_OUT32(streamP)
    PACE_RETURN()
}


//======================================================================
Err SndStreamStart(SndStreamRef stream)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD32(stream)
	PACE_EXEC_RP(sysTrapSndStreamStart, Err)
}

*/

//======================================================================
Err SndStreamCreate( SndStreamRef *channel, 
	SndStreamMode mode, 
	UInt32 samplerate, 
	SndSampleType type, 
	SndStreamWidth width, 
	SndStreamBufferCallback func, 
	void *userdata, 
	UInt32 buffsize, 
	Boolean armNative 
) 
{ 
	PACE_PARAMS_INIT() 
	PACE_PARAMS_ADD32(channel) 
	PACE_PARAMS_ADD8(mode) 
	PACE_PARAMS_ADD32(samplerate) 
	PACE_PARAMS_ADD16(type) 
	PACE_PARAMS_ADD8(width) 
	PACE_PARAMS_ADD32(func) 
	PACE_PARAMS_ADD32(userdata) 
	PACE_PARAMS_ADD32(buffsize) 
	PACE_PARAMS_ADD8(armNative) 
	PACE_EXEC_DELAY( sysTrapSndStreamCreate, Err ) 
	PACE_SWAP_OUT32(channel) 
	PACE_RETURN() 
} 


//======================================================================
Err SndStreamCreateExtended(
	SndStreamRef *channel,	/* channel-ID is stored here */
	SndStreamMode mode,		/* input/output, enum */
	SndFormatType format,	/* enum, e.g., sndFormatMP3 */
	UInt32 samplerate,		/* in frames/second, e.g. 44100, 48000 */
	SndSampleType type,		/* enum, e.g. sndInt16, if applicable, or 0 otherwise */
	SndStreamWidth width,	/* enum, e.g. sndMono */
	SndStreamVariableBufferCallback func,	/* function that gets called to fill a buffer */
	void *userdata,			/* gets passed in to the above function */
	UInt32 buffsize,			/* preferred buffersize, use 0 for default */
	Boolean armNative)				/* true if callback is arm native */
{ 
	PACE_PARAMS_INIT() 
	PACE_PARAMS_ADD32(channel) 
	PACE_PARAMS_ADD8(mode) 
	PACE_PARAMS_ADD32(format)
	PACE_PARAMS_ADD32(samplerate) 
	PACE_PARAMS_ADD16(type) 
	PACE_PARAMS_ADD8(width) 
	PACE_PARAMS_ADD32(func) 
	PACE_PARAMS_ADD32(userdata) 
	PACE_PARAMS_ADD32(buffsize) 
	PACE_PARAMS_ADD8(armNative) 
	PACE_EXEC_DELAY( sysTrapSndStreamCreateExtended, Err ) 
	PACE_SWAP_OUT32(channel) 
	PACE_RETURN() 
} 

//======================================================================
Err SndStreamStart(SndStreamRef channel) 
{ 
	PACE_PARAMS_INIT() 
	PACE_PARAMS_ADD32(channel) 
	PACE_EXEC( sysTrapSndStreamStart, Err ) 
} 

//======================================================================
Err SndStreamStop(SndStreamRef channel) 
{ 
	PACE_PARAMS_INIT() 
	PACE_PARAMS_ADD32(channel) 
	PACE_EXEC(sysTrapSndStreamStop, Err) 
}


//======================================================================
Err SndStreamDelete(SndStreamRef channel) 
{ 
	PACE_PARAMS_INIT() 
	PACE_PARAMS_ADD32(channel) 
	PACE_EXEC(sysTrapSndStreamDelete, Err) 
}


//======================================================================
Err SndStreamSetVolume (SndStreamRef stream, Int32 volume)
{
	PACE_PARAMS_INIT() 
	PACE_PARAMS_ADD32(stream) 
	PACE_PARAMS_ADD32(volume) 
	PACE_EXEC(sysTrapSndStreamSetVolume, Err) 
}


//======================================================================
Err	DlkGetSyncInfo(void *succSyncDateP, UInt32 * lastSyncDateP,
		void *syncStateP, Char * nameBufP,
		Char * logBufP, Int32 * logLenP)
{
	PACE_PARAMS_INIT()
	PACE_PARAMS_ADD32(succSyncDateP)
	PACE_PARAMS_ADD32(lastSyncDateP)
    PACE_PARAMS_ADD32(syncStateP)
    PACE_PARAMS_ADD32(nameBufP)
	PACE_PARAMS_ADD32(logBufP)
	PACE_PARAMS_ADD32(logLenP)
	PACE_EXEC( sysTrapDlkGetSyncInfo, Err) 
}

//======================================================================
void abort()
{
	/*
	static	bool already_app_close=false;
	if (!already_app_close)
	{
		already_app_close=true;
		application::erase();
	}

	static	bool already_platform_close=false;
	if (!already_platform_close)
	{
		already_platform_close=true;
		platform::erase();
	}
    */
	(g_call68KFuncP)(g_emulStateP, abort_programm, NULL, kPceNativeWantA0);
}


//======================================================================
unsigned long PNO_Main(const void *emulStateP, void *userData68KP, Call68KFuncType *call68KFuncP)
{
	// needed before making any OS calls using the 
	// PACEInterface library
	InitPACEInterface(emulStateP, call68KFuncP);
	g_emulStateP=const_cast<EmulStateType*>(emulStateP);
	g_call68KFuncP=call68KFuncP;

	struct	si
	{
		UInt32	width,
				height,
				depths;
		char	*path;
		int	message_box;
		uint32	no_sound;
		int	abort_programm;
	}*info=(si*)userData68KP;

	message_box=ByteSwap32(info->message_box);
	abort_programm=ByteSwap32(info->abort_programm);

	if ( (ByteSwap32(info->depths) & (1<<(sizeof(COLOR)*8-1)))==0 )
	{
		FrmCustomAlert(InfoMessage, "Unsupported screen depth", NULL, NULL);
		return -1;
	}

	size<>	screen(ByteSwap32(info->width), ByteSwap32(info->height));

	return platform::launch(new palmos5(emulStateP, call68KFuncP, screen, (char*)ByteSwap32((uint32)info->path), ByteSwap32(info->no_sound)!=0));
}



/************************************************************************/
/* Purpose..: Unexpected handler										*/
/* Input....: pointer to throw context									*/
/* Return...: --- (this function will never return)						*/
/************************************************************************/
void __unexpected(void *catchinfo)
{
	ERROR("Unhandled exception");
	abort();
}


//============================================================================================
void* operator new(std::size_t size)
{
	if (size==0)
		size=1;
	void *p=MemPtrNew(size);
	if (!p)
	{
		FrmCustomAlert(InfoMessage, "No free memory", NULL, NULL);
		abort();
	}
	return p;
}

//============================================================================================
void  operator delete(void* ptr) throw()
{
	if (ptr)
		MemPtrFree(ptr);
}


//============================================================================================
void* operator new[](std::size_t size)
{
/*
	void *p;
	if (size<0xfff0)
		p=MemPtrNew(size);
	else
	{
		
		size=ByteSwap32(size);
		p=(void*)((g_call68KFuncP)(g_emulStateP, (UInt32)glue_new, &size, 0 | kPceNativeWantA0));
	}
*/    
	if (size==0)
		size=1;
	else if (size>0xffff)
	{
    	ERROR("alloc >65k unanavailable");
    	abort();
	}

	void *p=MemPtrNew(size);
    
	if (!p)
	{
		FrmCustomAlert(InfoMessage, "No free memory", NULL, NULL);
		abort();
	}
	return p;
}

//============================================================================================
void  operator delete[](void* ptr) throw()
{
	if (ptr)
		MemPtrFree(ptr);
}
