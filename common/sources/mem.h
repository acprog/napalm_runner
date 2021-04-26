/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_MEM_H_
#define _MPP_MEM_H_

#include "platform.h"

namespace mpp{
    // сюдаже нужно перенести mem_file и добавить класс array<> - фиксированный массив.
    // тогда большой массив можно задать как произфодный от него.. а вот как быть с записью всеаки
    // не понятно. в любом случае обращение будет чуть проще. (а если этот массив еще представить)
    // как файл, то может будет не так не удобно.
	

	//========================================================================
	//	функции для работы с большими массивами памяти (из-за pos5)
	//========================================================================		
	namespace large_mem
	{
#ifdef _WIN32_WCE
		//---------------------------------------------------------------
		template <class T>
		class allocate
		{
		private:
			int	leng;

		public:
			allocate(int _leng)
				:leng	(_leng)
			{}

			operator T*()
			{
				return (T*)platform::get().large_allocate(sizeof(T)*leng);
			}
		};
#else
		//---------------------------------------------------------------
		template <class T>
		inline	T *allocate(int leng)
		{
			return (T*)platform::get().large_allocate(sizeof(T)*leng);
		}
#endif
		//---------------------------------------------------------------
		template<class T>
		inline	void write(T *start, T *dst, T *src, int size=1)
		{
			platform::get().large_write((void*)start, (uint8*)dst, (uint8*)src, size*sizeof(T));
		}

		//---------------------------------------------------------------
		inline void free(void *p)
		{
			platform::get().large_free(p);
		}		
	}

}


#endif	// _MPP_MEM_H_
