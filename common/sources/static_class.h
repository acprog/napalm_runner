/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_STATIC_CLASS_
#define	_MPP_STATIC_CLASS_

#include "string.h"
#include "dobject.h"
	
namespace mpp
{
	//===============================================================
	// шаблон статического класса - может иметь только один экземпл€р
	//===============================================================
	template<class T>
	class static_class
	{
	private:
		//=======================================================
		inline static T *instance(bool set=false, T *new_ptr=NULL)
		{
			static  T *ptr=NULL;
			if (set)
				ptr=new_ptr;
			return ptr;
		}

	protected:
		//=======================================================
		inline  static_class()
		{
			erase();
			instance(true, (T*)this);
		}

		//=======================================================
		virtual ~static_class()
		{
			ASSERT (&get()!=this, "can't remove static class - 2 objects");
			instance(true, NULL);
		}

	public:
		//=======================================================
		inline  static  T &get()
		{
			ASSERT(!exist(), "static class not have instance");
			return *instance();
		}

		//=======================================================
		inline static bool exist()
		{
			return instance()!=NULL;
		}

		//=======================================================
		inline  static  void erase()
		{
			if (exist())
			{
				delete instance();
				instance(true, NULL);
			}
		}
	};
}

#endif
