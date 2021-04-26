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
		//	������ ��������� �� �������
		// ���� ������ ������������, �� �������� ���������
		//======================================================================
		template <int degree, class type>
		class state_vector
			:public handler<cycle>
		{
		private:
			type	mult;	// ��������� ��� �������� ��������� (��������� �� ������ ��������� �
							// ������� �����)
			type	*ptr;	// ��������� �� ��������� �������� (��� ����������)
			type	zero;	// ����
			type	X[degree];
			handler<state_vector>	*state_handler;

			//===============================================================
			void event(cycle *c, void*)
			{
				type	prev=X[0];

				// ���� ������ 2-� ����� ����������, ����� �������
				for (int i=degree; --i>0;)
					X[i-1]+=X[i];	// ��� ������������� ����� �� ������������ ���������
				
				if (prev!=X[0])
					state_handler->event(this, this);	// ������� ������� �� ��������� ��������� �������
			}

		public:
			//===============================================================
			// ���� �������� ��������� � �������
			inline	state_vector(dispatcher<cycle> *ext_c, handler<state_vector<degree, type> > *ext_sv, const type &one_in_sec, const type &_zero)
				:handler<cycle>							(ext_c)
				,state_handler							(ext_sv)
				,mult	(one_in_sec/application::get().cycle_frequency)
				,ptr	(NULL)
				,zero	(_zero)
			{
				// �������� ���������� �����������, �� ������� �������� ������� ���� ������
				for (int i=degree; --i>=0;)
					X[i]=zero;
			}

			//=======================================================
			// ���������� - �.�. �������� ��� ����� �������� ���������
			inline void stop()
			{
				for (int i=degree; --i>0;)
					X[i]=zero;
			}

			//=======================================================
			// ��������� ��� ��������� ����� (�.�. ���������� ������ ���� ��� ���������)
			// �� ��������� � �������������
			// �������� �� �������
			inline	state_vector<degree, type> &operator[](int d)
			{
				ASSERT(degree<d, "state vector: index out of range");
				ptr=X+d;
				return *this;
			}

			//-------------------------------------------------------
			// �������� ���������
			inline	state_vector<degree, type> &position()
			{
				ptr=X;
				return *this;
			}

			//-------------------------------------------------------
			// �������� ��������
			inline	state_vector<degree, type> &speed()
			{
				ptr=X+1;
				return *this;
			}

			//-------------------------------------------------------
			// �������� ���������
			inline	state_vector<degree, type> &accelerate()
			{
				ASSERT(degree<3, "state vector: index out of range");
				ptr=X+2;
				return *this;
			}

			//-------------------------------------------------------
			// ���������� ���������� �������� � ����
			inline	operator type() const
			{
				ASSERT(ptr==NULL, "state_vector: not select value for return");
				// �.�. ����������� �������� ��������, ����� ����� ����� ������������ ��������,
				// �� ����� ������ ���� ���������
				type	tmp=*ptr;
				tmp*=mult;
				DEBUG_ONLY(ptr=NULL);
				return tmp;
			}

			//-------------------------------------------------------
			// ���������� ���������� �������� � ����
			inline	type out()
			{
				ASSERT(ptr==NULL, "state_vector: not select value for return");
				// �.�. ����������� �������� ��������, ����� ����� ����� ������������ ��������,
				// �� ����� ������ ���� ���������
				type	tmp=*ptr;
				tmp*=mult;
				DEBUG_ONLY(ptr=NULL);
				return tmp;
			}

			//-------------------------------------------------------
			// ������ ���� � ��������� ��������
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

		// �������� ����� � ��������� ��� ���������� ���������
//		typedef	state_vector<3, point< Fixed<8> > >	dynamic_point;
//		typedef state_vector<3, point<float> >	dynamic_point;

		// �������� ����� � ������������ ��� ���������� ���������
//		typedef	dynamic_vector	state_vector<3, vertex<REAL> >	

		// �������� �������� ������� ����� ��������� ������ ��������� �� �����������
		// � ������ ��������� �� ��������

		// � ������ ������� ����� ��� ����� ������������ ����
		// �� ����� ���� ��� ����� ����� �������� ����������� ��������� �������
	}
}

#endif