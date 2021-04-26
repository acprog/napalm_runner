#ifndef _WRAPPER_H_
#define _WRAPPER_H_

//=====================================================================
//	Оболочка на связь с 68k
//=====================================================================
class wrapper
{
protected:
	const	void	*emulState;
	Call68KFuncType *call68KFunc;

public: 
	wrapper(const void *_emulState, Call68KFuncType *_call68KFunc)
		:emulState	(_emulState)
		,call68KFunc(_call68KFunc)
	{
	}

	//=====================================================================
	virtual	~wrapper(){}

	//=====================================================================
	// смена порядка байтов
	template<class T>
	inline int endian_swap(unsigned char *b, T arg)
	{
		unsigned char	*p=(unsigned char*)&arg;

		switch(sizeof(T))
		{
		case 1:
			b[0]=p[0];
			break;

		case 2:
			b[0]=p[1];
			b[1]=p[0];
			break;
		
		case 4:
			b[0]=p[3];
			b[1]=p[2];
			b[2]=p[1];
			b[3]=p[0];
			break;

		default:
			;//throw "unknown stack param";
		}

		return sizeof(T);
	}




	//=====================================================================
	// вызов функций по колличеству аргументов в стеке
	template<class result, int func_id>
	inline result trap0()
	{
		return (result)(call68KFunc)
			(emulState, PceNativeTrapNo(func_id), NULL, 0);
	}


	//-----------------------------------------------------------------------
	template<class result, int func_id, class T0>
	inline result trap1(T0 arg0)
	{
		uint8	b[sizeof(T0)];
		endian_swap(b, arg0);
		return (result)(call68KFunc)
			(emulState, PceNativeTrapNo(func_id), b, sizeof(b)|kPceNativeWantA0);
	}


	//-----------------------------------------------------------------------
	template<class result, int func_id, class T0, class T1>
	inline result trap2(T0 arg0, T1 arg1)
	{
		uint8	b[sizeof(T0)+sizeof(T1)],
				*p=b;
		p+=endian_swap(p, arg0);
		p+=endian_swap(p, arg1);

		return (result)(call68KFunc)
			(emulState, PceNativeTrapNo(func_id), b, sizeof(b)|kPceNativeWantA0);
	}


	//-----------------------------------------------------------------------
	template<class result, int func_id, class T0, class T1, class T2>
	inline result trap3(T0 arg0, T1 arg1, T2 arg2)
	{
		uint8	b[sizeof(T0)+sizeof(T1)+sizeof(T2)],
				*p=b;
		p+=endian_swap(p, arg0);
		p+=endian_swap(p, arg1);
		p+=endian_swap(p, arg2);

		return (result)(call68KFunc)
			(emulState, PceNativeTrapNo(func_id), b, sizeof(b)|kPceNativeWantA0);
	}

	//-----------------------------------------------------------------------
	template<class result, int func_id, class T0, class T1, class T2, class T3>
	inline result trap4(T0 arg0, T1 arg1, T2 arg2, T3 arg3)
	{
		unsigned char	b[sizeof(T0)+sizeof(T1)+sizeof(T2)+sizeof(T3)],
						*p=b;
		p+=endian_swap(p, arg0);
		p+=endian_swap(p, arg1);
		p+=endian_swap(p, arg2);
		p+=endian_swap(p, arg3);

		return (result)(call68KFunc)
			(emulState, PceNativeTrapNo(func_id), b, sizeof(b)|kPceNativeWantA0);
	}

	//-----------------------------------------------------------------------
	template<class result, int func_id, class T0, class T1, class T2, class T3, class T4>
	inline result trap5(T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4)
	{
		unsigned char	b[sizeof(T0)+sizeof(T1)+sizeof(T2)+sizeof(T3)+sizeof(T4)],
						*p=b;
		p+=endian_swap(p, arg0);
		p+=endian_swap(p, arg1);
		p+=endian_swap(p, arg2);
		p+=endian_swap(p, arg3);
		p+=endian_swap(p, arg4);

		return (result)(call68KFunc)
			(emulState, PceNativeTrapNo(func_id), b, sizeof(b)|kPceNativeWantA0);
	}

	//-----------------------------------------------------------------------
	template<class result, int func_id, class T0, class T1, class T2, class T3, class T4, class T5>
	inline result trap6(T0 arg0, T1 arg1, T2 arg2, T3 arg3, T4 arg4, T5 arg5)
	{
		unsigned char	b[sizeof(T0)+sizeof(T1)+sizeof(T2)+sizeof(T3)+sizeof(T4)+sizeof(T5)],
						*p=b;
		p+=endian_swap(p, arg0);
		p+=endian_swap(p, arg1);
		p+=endian_swap(p, arg2);
		p+=endian_swap(p, arg3);
		p+=endian_swap(p, arg4);
		p+=endian_swap(p, arg5);

		return (result)(call68KFunc)
			(emulState, PceNativeTrapNo(func_id), b, sizeof(b)|kPceNativeWantA0);
	}

	//=====================================================================
	// шаблоны функций
//	#define	BmpCreate				trap5<BitmapType*,	sysTrapBmpCreate,		Coord, Coord, UInt8, ColorTableType*, UInt16 *>
//	#define BmpDelete				trap1<Err,			sysTrapBmpDelete,		BitmapType*>
//	#define	BmpGetBits				trap1<void*,		sysTrapBmpGetBits,		BitmapType*>
	#define	WinScreenLock   		trap1<UInt8*,		sysTrapWinScreenLock,	WinLockInitType>
	#define WinScreenUnlock			trap0<void,			sysTrapWinScreenUnlock>
	#define	SndPlayResource			trap3<Err,			sysTrapSndPlayResource,	SndPtr, Int32, UInt32>
	#define	SysTicksPerSecond		trap0<UInt16,		sysTrapSysTicksPerSecond>
	#define TimGetTicks				trap0<UInt32,		sysTrapTimGetTicks>
	#define EvtResetAutoOffTimer	trap0<Err,			sysTrapEvtResetAutoOffTimer>
//	#define WinScreenGetAttribute	trap2<Err,			sysTrapWinScreenGetAttribute, WinScreenAttrType, UInt32*>
	#define	WinPaintBitmap			trap3<void,			sysTrapWinPaintBitmap,	BitmapType *, Coord, Coord>
//	#define	KeyCurrentState			trap0<UInt32,		sysTrapKeyCurrentState>
};	//wrapper

#endif
