/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
//================================================================================
// ????????? ??????? - ????? ??????????????? ? ???????????? ???????
// ?????? ???? ??????????? ??????????? ? ???????????, ?? ?? ????? ????? ????????? 
// ???????????? ? mpp ? ?? ???????????? ? ??????????? ??? ????????? ??????
// ?????? ?????? ??? ????? ???????? ????? ???-?? ??????? ?? ?????? ?? ????????,
// ? ??? ??????? - ??? ??? ????? ????????, ?.?. ???? ??????????? ?????????? ? ?????
// ??? ?????
//================================================================================
#ifndef _MPP_EVENT_HANDLER_
#define	_MPP_EVENT_HANDLER_

#include "list.h"
#include "tools.h"

namespace mpp
{
//	namespace events
//	{
		template <class T>
		class dispatcher;
		/*
		??? ???????? ??????? ????? ??? ???????, ??? ??? ??? ???????
		?? ?????? ?????? ???-?? ??????.. - ????? ????? dispatcher
		??????? ? event(ev), event(ev, source). ??? ??? ???????? ???-??
		????? event(ev, source), ? event() - ?????????????? ?????
		pass event ????? ???????? - ???? ?????? ??????? ??????????? ????????? 
		??? ???????? ? ??? ?????.. ?? ????? ??????? ????? ??? ????????? ?? 
		event(ev, source) - 
		*/

		//================================================================================
		//	?????????? ???????
		//================================================================================
		template <class T>
		class handler
		{
		protected:
			dispatcher<T>	*external;	// ??????? ???????? ???????
		
		public:
			//------------------------------------------------------------
			inline	handler(dispatcher<T> *ext=NULL)
				:external		(ext)
			{
				if (external)
					external->add_internal(this);
			}

			//------------------------------------------------------------
			virtual ~handler()
			{
				if (external)
					external->remove_internal(this);
			}

			//-------------------------------------------------------------------
			// source - ????? ??? ????????? ????????? ????????
			virtual	void event(T *ev, void *source)=0;

			//-------------------------------------------------------------------
			// ????????? ???????? ???????
			virtual bool pass_event(T *ev)
			{
				return true;
			}

			//--------------------------------------------------------
			// ???????????? ???????????? ???????? ??????, ???? ????? ???? ????? ?? ??????? ? ???? ??????
			inline void clean_external()
			{
				external=NULL;
			}
		};



		//===============================================================================
		// ???????? ????????? "????" ???? ??????? ?? ???????
		//===============================================================================
		template <class T>
		class dispatcher : public handler<T>
		{
		public:
			//------------------------------------------------------------
			inline	dispatcher(dispatcher<T> *ext=NULL)
				:handler<T>(ext)
			{
			}

			//--------------------------------------------------------
			~dispatcher()
			{
				while (handler<T> *h=internals.each())
					h->clean_external();
			}	

			//--------------------------------------------------------
			void event(T *ev, void *source)
			{
				if (pass_event(ev))
					while (handler<T> *h=internals.each())
//						if (h!=source && h->pass_event(ev))
							h->event(ev, source);
			}

			//--------------------------------------------------------
			inline	void add_internal(handler<T> *h)
			{
				internals.push_back(h);
			}


			//--------------------------------------------------------
			inline	void remove_internal(handler<T> *h)
			{
				internals.remove(h);
			}

		protected:
			list<handler<T>*>	internals;	// ?????????? ??????????? ???????
		};


		//=======================================================================
		// ????? ??????????????? ??? ??????????????? ?????????? ?? ??????????? ???????
		//=======================================================================
		template <class T, class externT>
		class static_source : public T
		{
		public:
			inline	static_source()
				:T(&externT::get())
			{
			}
		};



		//=======================================================
		template<class T>
		inline void send(T *ev, handler<T> *dst, void *source)
		{
			dst->event(ev, source);
		}

//	}
}

#endif