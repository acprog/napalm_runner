/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_TOOLS_H_
#define _MPP_TOOLS_H_

namespace mpp
{
	//===============================================================
	// базовые типы
	typedef	signed char		sint8;
	typedef	signed short	sint16;
	typedef	signed long		sint32;
	typedef	unsigned char	uint8;
	typedef	unsigned short	uint16;
	typedef	unsigned long	uint32;

	#ifndef NULL
		#define NULL 0
	#endif


	//=============================================================
	// message logs & errors
	class string;
	extern	void error_message(const char *module, int line, const string &s);
	extern	const string &log_message(const char *module, int line, const string &s);
	#undef	ERROR
	#define	ERROR(str)	mpp::error_message(__FILE__, __LINE__, string("")+str)
	#undef 	LOG
	#define	LOG(str)	mpp::log_message(__FILE__, __LINE__, string("")+str)

	#undef	ASSERT
	#ifdef	_DEBUG
		#define	ASSERT(if_true, str)	if (if_true)	ERROR(str)
   	#else
   		#define	ASSERT(if_true, str)
   	#endif


	//---------------------------------------------------------------------
	// функции общего назначения
	template<class T>
	inline	void mem_set(T *p, int leng, T value)
	{
		while (--leng>=0)
			*(p++)=value;
	}

	//---------------------------------------------------------------------
	template<class T>
	inline	void mem_cpy(T *dst, const T *src, int leng)
	{
		while (--leng>=0)
			*(dst++)=*(src++);
	}

	//---------------------------------------------------------------------
	template<class T>
	inline	void swap(T &a, T &b)
	{
		T	tmp=a;
		a=b;
		b=tmp;
	}


	//---------------------------------------------------------------------
	template<class T>
	inline	T abs(T a)
	{
		return a>=0 ? a : 0-a;
	}

	//---------------------------------------------------------------------
	template<class T>
	inline	T minimum(T a, T b)
	{
		return a<b ? a : b;
	}


	//---------------------------------------------------------------------
	template<class T>
	inline	T maximum(T a, T b)
	{
		return a>b ? a : b;
	}


	//---------------------------------------------------------------------
	template<class T>
	inline	bool between_without(T value, T a, T b)
	{
		return value>a && value<b;
	}

	//---------------------------------------------------------------------
	template<class T>
	inline	bool between_with(T value, T a, T b)
	{
		return value>=a && value<=b;
	}

	//---------------------------------------------------------------------
	// использовать для удаления указателей
	template<class T>
	inline void del_it(T &x)
	{
		if (x)
		{
			delete x;
			x=NULL;
		}
	}
};//mpp
#endif
