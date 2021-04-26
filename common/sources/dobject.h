/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_DOBJECT_H_
#define _MPP_DOBJECT_H_

#include "string.h"

namespace mpp
{
	//========================================================================
	//	динамический объект. для реализации автоматической сборки мусора
	//========================================================================
	class dobject
	{
	public:
		//-------------------------------------------------------------
		inline dobject()
			:ref_count	(0)
		{
		}

		//-------------------------------------------------------------
		inline dobject(const dobject &src)
			: ref_count (0)
		{
		}

		//-------------------------------------------------------------
		virtual ~dobject()
		{
			ASSERT(ref_count!=0, "attempt to destroy dobject with ref_count="+ref_count);
		}

		//-------------------------------------------------------------
		inline void add_ref()
		{
			++ref_count;
		}

		//-------------------------------------------------------------
  		inline void del_ref()
  		{
			ASSERT(ref_count==0, "ref_count already 0");
			if(--ref_count<=0)
				delete this;
		}
		
	private:
		int ref_count;

//		void operator delete(void *p) { ::delete(p); }	// деструктор вызывается только из самого dobject
	};



	//==================================================================
	//	"умный" аказатель на dobject
	//==================================================================
	template<class T>
	class pointer
	{
	public:
		//-------------------------------------------------------------
  		inline pointer()
			:ptr(NULL)
  		{
  		}

		//-------------------------------------------------------------
  		inline pointer(T *p)
			:ptr(p)
  		{
  			if (ptr)
  				ptr->add_ref();
  		}

		//-------------------------------------------------------------
		inline pointer(const pointer &p)
			:ptr(p.ptr)
		{
  			if (ptr)
  				ptr->add_ref();
		}

		//-------------------------------------------------------------
		inline pointer& operator =(const pointer &p)
		{
    		if(ptr)
    			ptr->del_ref();
    		if (ptr=p.ptr)
    			ptr->add_ref();
			return *this;
		}

		//-------------------------------------------------------------
		inline pointer& operator =(T *p)
		{
    		if(ptr)
    			ptr->del_ref();
    		if (ptr=p)
    			ptr->add_ref();
			return *this;
		}

		//-------------------------------------------------------------
		inline ~pointer()
		{
    		if(ptr)
    		{
    			ptr->del_ref();
    			ptr=NULL;
			}
		}

		//-------------------------------------------------------------
		inline operator T*() const   { return ptr;  }
		inline T& operator*() const  { return *ptr; }
		inline T* operator->() const { return ptr;  }
		inline T* get() const 		  { return ptr;  }

	private:
		T		*ptr;
	};
}

#endif // _MPP_DOBJECT_H_
