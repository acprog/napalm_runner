#ifndef _MPP_PHYSICS_
#define _MPP_PHYSICS_

#include "float.h"
#include "mpp_events.h"
#include "application.h"

namespace mpp
{
	namespace physics
	{
		//======================================================================
		//	Вектор состояния во времени
		// если объект переместился, то посылает сообщение
		//======================================================================
		template <int degree, class type>
		class state_vector
			:public handler<cycle>
		{
		private:
			type	mult;	// множитель для значений состояния (зависящий от единиц измерения и
							// частоты цикла)
			type	*ptr;	// указатель на выбранное значение (для интерфейса)
			type	zero;	// ноль
			type	X[degree];
			handler<state_vector>	*state_handler;

			//===============================================================
			void event(cycle *c, void*)
			{
				type	prev=X[0];

				// если ввести 2-е явные переменные, будет быстрее
				for (int i=degree; --i>0;)
					X[i-1]+=X[i];	// для эффективности здесь не используется умножение
				
				if (prev!=X[0])
					state_handler->event(this, this);	// послать событие об изменении положения объекта
			}

		public:
			//===============================================================
			// одно значение состояния в секунду
			inline	state_vector(dispatcher<cycle> *ext_c, handler<state_vector<degree, type> > *ext_sv, const type &one_in_sec, const type &_zero)
				:handler<cycle>							(ext_c)
				,state_handler							(ext_sv)
				,mult	(one_in_sec/application::get().cycle_frequency)
				,ptr	(NULL)
				,zero	(_zero)
			{
				// возможно избыточная конструкция, но поможет избежать разного рода ошибок
				for (int i=degree; --i>=0;)
					X[i]=zero;
			}

			//=======================================================
			// остановить - т.е. затираем все кроме текущего положения
			inline void stop()
			{
				for (int i=degree; --i>0;)
					X[i]=zero;
			}

			//=======================================================
			// интерфейс для обращения извне (т.к. внутренний формат бытр для рассчетов)
			// но неприятен в использовании
			// значение по индексу
			inline	state_vector<degree, type> &operator[](int d)
			{
				ASSERT(degree<d, "state vector: index out of range");
				ptr=X+d;
				return *this;
			}

			//-------------------------------------------------------
			// значение положения
			inline	state_vector<degree, type> &position()
			{
				ptr=X;
				return *this;
			}

			//-------------------------------------------------------
			// значение скорости
			inline	state_vector<degree, type> &speed()
			{
				ptr=X+1;
				return *this;
			}

			//-------------------------------------------------------
			// значение ускорения
			inline	state_vector<degree, type> &accelerate()
			{
				ASSERT(degree<3, "state vector: index out of range");
				ptr=X+2;
				return *this;
			}

			//-------------------------------------------------------
			// приведение выбранного значения к типу
			inline	operator type() const
			{
				ASSERT(ptr==NULL, "state_vector: not select value for return");
				// т.к. возвращение текущего значения, думаю более часто используемая операция,
				// то здесь должно быть умножение
				type	tmp=*ptr;
				tmp*=mult;
				DEBUG_ONLY(ptr=NULL);
				return tmp;
			}

			//-------------------------------------------------------
			// приведение выбранного значения к типу
			inline	type out()
			{
				ASSERT(ptr==NULL, "state_vector: not select value for return");
				// т.к. возвращение текущего значения, думаю более часто используемая операция,
				// то здесь должно быть умножение
				type	tmp=*ptr;
				tmp*=mult;
				DEBUG_ONLY(ptr=NULL);
				return tmp;
			}

			//-------------------------------------------------------
			// запись типа в выбранное значение
			inline void operator=(type t)
			{
				ASSERT(ptr==NULL, "state_vector: not select value for =");
				type	tmp=t;
				tmp/=mult;
				*ptr=tmp;
				DEBUG_ONLY(ptr=NULL);
				//return t;
			}
		};

		// динамика точки в плоскости при задаваемом ускорении
//		typedef	state_vector<3, point< Fixed<8> > >	dynamic_point;
//		typedef state_vector<3, point<float> >	dynamic_point;

		// динамика точки в пространстве при постоянном ускорении
//		typedef	dynamic_vector	state_vector<3, vertex<REAL> >	

		// динамика простого объекта будет содержать вектор состояния по перемещению
		// и вектор состояния по вращению

		// к такому объекту можно уже будет прикладывать силы
		// по сумме всех сил можно будет выяснить необходимое поведение объекта
	}
}

#endif