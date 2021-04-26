/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_TIMER_
#define	_MPP_TIMER_

#include "controls.h"
#include "application.h"
#include "mpp_events.h"

namespace mpp
{
	//================================================================================
	//	Класс - таймер, генерит сообщение в базовый класс
	// таймер запускается сразу после создания. при истекании времени он останавливается
	//================================================================================
	class timer : public handler<cycle>
	{
	private:
		int		n_cycles,		// колл-во циклов на истечение таймера
				counter;		// осталось циклов
		REAL	pos,			// позиция в процентах
				pos_step;		// шаг
		int		n_repeats,		// колл-во повторений перед остановкой
				repeat;			// осталось повторов
		bool	pause;
		handler<timer>	*timer_handler;

	protected:
		//---------------------------------------------------------------------
		// добавить время
		void event(cycle *c, void*)
		{
			pos+=pos_step;
			if (--counter<=0)
			{
				if (repeat==0)
					stop();
				else
				{
					repeat--;
					counter=n_cycles;
					pos=0;
				}
				timer_handler->event(this, this);
			}
		}

	public:
		const static	int infinite;

		//---------------------------------------------------------------------
		inline	timer(dispatcher<cycle> *ext_c, handler<timer> *ext_t, REAL time_period, int _n_repeats=0)
			:handler<cycle>	(ext_c)
			,timer_handler	(ext_t)
			,n_cycles	(time_period*application::get().cycle_frequency)
			,pos_step	(((REAL)1)/n_cycles)
			,counter	(n_cycles)
			,pos		(0)
			,n_repeats	(_n_repeats)
			,repeat		(_n_repeats)
			,pause		(false)
		{
		}

		//---------------------------------------------------------------------
		inline	void start()
		{
			counter=n_cycles;
			pos=0;
			repeat=n_repeats;
			unpause();
		}

		//---------------------------------------------------------------------
		inline	void unpause()
		{
			if (pause)
			{
				pause=false;
				handler<cycle>::external->add_internal(this);
			}
		}
		
		//---------------------------------------------------------------------
		inline	void stop()
		{
			pause=true;
			handler<cycle>::external->remove_internal(this);
		}

		//---------------------------------------------------------------------
		// [0, 1] текущая позиция
		inline	REAL progress() const
		{
			return pos;
		}
	};




	//======================================================================
	//	анимация
	//======================================================================
	class	movie
		:public animation
		,public timer
		,public handler<timer>
	{
	public:
		movie(const string &name, collection *coll, REAL period);
		const image &frame();
		void event(timer *t, void*){}
	};
}

#endif
