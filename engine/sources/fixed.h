/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_FIXED_H_
#define _MPP_FIXED_H_

#include "mpp_prefs.h"

namespace mpp
{

	#define	FRACT	16

	#define ANGLE_90        16384U          // angle of 90 degrees
	#define ANGLE_PI		32768U          // angle of 180 degress
	#define ANGLE_180       32768U          // angle of 180 degress
	#define ANGLE_270       49152U          // angle of 270 degress

	#ifndef M_PI
		#define M_PI        3.14159265358979323846
	#endif

	typedef unsigned short  Angle;




	//============================================================
	//	Число с фиксированной точкой	(смещение целой части, тип)
	//============================================================
	template <int shift=FRACT, class T=long>
	class Fixed
	{
	private:
		T	value;
		
	public:
		//========================================================
		inline	Fixed()
			:value	(0)
		{}

		//--------------------------------------------------------
		inline	Fixed(int v)
			:value	( ((T)v)<<shift )
		{}

		//--------------------------------------------------------
		inline	Fixed(unsigned long v)
			:value	( ((T)v)<<shift )
		{}

		//--------------------------------------------------------
		inline	Fixed(long v)
			:value	( ((T)v)<<shift )
		{}

		//--------------------------------------------------------
		inline	Fixed(float v)
			:value	( (T)(v*(float)(1L<<shift)) )
		{}
		
		//--------------------------------------------------------
		inline	Fixed(double v)
			:value	( (T)(v*(double)(1L<<shift)) )
		{}

		//--------------------------------------------------------
		inline	Fixed(const Fixed<shift, T> &v)
			:value	( v.value )
		{}	

		//========================================================
		inline Fixed operator=(Fixed v)
		{
			value=v.value;
			return *this;
		}

		//--------------------------------------------------------
		template<class H>
		inline Fixed operator=(H v)
		{
			return (*this)=Fixed<shift, T>(v);
		}

		//========================================================
		inline	operator float() const
		{
			return ((float)value)/(float)(1L<<shift);
		}

		//--------------------------------------------------------
		template <class H>
		inline	operator H() const
		{
			return value>>shift;
		}


		//========================================================
		inline	const Fixed operator*=(Fixed v)
		{
			// частичное решение проблемы при умножении 2-х чисел -
			// использовать меньшую дробную часть
			#define	ABS(x)	((x)>=0 ? (x) : -(x))

			if ( ABS(value) > ABS(v.value) )
			{
				value>>=(shift/2);
				value*=v.value;
			}
			else
				value*=(v.value>>(shift/2));
			value>>=(shift-shift/2);

			return *this;

			#undef	ABS
		}

		//--------------------------------------------------------
		template<class H>
		const Fixed operator*=(H v)
		{
			value*=v;
			return *this;
		}

		//--------------------------------------------------------
		template <class H>
		inline	Fixed operator*(H v) const
		{
			return Fixed(*this)*=v;
		}

		//========================================================
		inline	Fixed operator/=(Fixed v)
		{
			value/=v.value;
			value<<=shift;
			return *this;
		}

	
		//--------------------------------------------------------
		template<class H>
		inline	Fixed operator/=(H v)
		{
			value/=v;
			return *this;
		}

		//--------------------------------------------------------
		template <class H>
		inline	Fixed operator/(H v) const
		{
			return Fixed(*this)/=v;
		}


		//========================================================
		inline	Fixed operator+=(Fixed v)
		{
			value+=v.value;
			return *this;
		}

		//--------------------------------------------------------
		template <class H>
		inline	Fixed operator+=(H v)
		{
			return *this+=Fixed(v);
		}		

		//--------------------------------------------------------
		template <class H>
		inline	Fixed operator+(H v) const
		{
			return Fixed(*this)+=v;
		}

		//========================================================
		inline	Fixed operator-=(Fixed v)
		{
			value-=v.value;
			return *this;
		}

		//--------------------------------------------------------
		template<class H>
		inline	Fixed operator-=(H v)
		{
			return *this-=Fixed(v);
		}

		//--------------------------------------------------------
		template <class H>
		inline	Fixed operator-(H v) const
		{
			return Fixed(*this)-=v;
		}

		//========================================================
		inline	Fixed operator>>=(int s)
		{
			value>>=s;
			return *this;
		}

		//--------------------------------------------------------
		inline	Fixed operator<<=(int s)
		{
			value<<=s;
			return *this;
		}

		//--------------------------------------------------------
		inline	Fixed operator>>(int s) const
		{
			return Fixed(*this)>>=s;
		}

		//--------------------------------------------------------
		inline	Fixed operator<<(int s) const
		{
			return Fixed(*this)<<=s;
		}

		//========================================================
		inline	bool operator>(Fixed v) const
		{
			return value>v.value;
		}

		//--------------------------------------------------------
		template<class H>
		inline	bool operator>(H v) const
		{
			return (*this)>Fixed(v);
		}

		//========================================================
		inline	bool operator>=(Fixed v) const
		{
			return value>=v.value;
		}

		//--------------------------------------------------------
		template<class H>
		inline	bool operator>=(H v) const
		{
			return (*this)>=Fixed(v);
		}

		//========================================================
		inline	bool operator<(Fixed v) const
		{
			return value<v.value;
		}

		//--------------------------------------------------------
		template<class H>
		inline	bool operator<(H v) const
		{
			return (*this)<Fixed(v);
		}

		//========================================================
		inline	bool operator<=(Fixed v) const
		{
			return value<=v.value;
		}

		//--------------------------------------------------------
		template<class H>
		inline	bool operator<=(H v) const
		{
			return (*this)<=Fixed(v);
		}

		//========================================================
		inline	bool operator==(Fixed v) const
		{
			return value==v.value;
		}

		//--------------------------------------------------------
		template<class H>
		inline	bool operator==(H v) const
		{
			return (*this)==Fixed(v);
		}


		//========================================================
		inline	bool operator!=(Fixed v) const
		{
			return value!=v.value;
		}

		//--------------------------------------------------------
		template<class H>
		inline	bool operator!=(H v) const
		{
			return (*this)!=Fixed(v);
		}
	};


	/*
	//================================================================

	float sin(float x);
	float cos(float x);

	extern  Fixed<> *sinTable;
	extern  Fixed<> *cosTable;

	#define SIN(x)	sinTable[(x)>>6]
	#define COS(x)	cosTable[(x)>>6]

	Angle atan(Fixed<> x, Fixed<> y);

	//================================================================

	void    initFixMath ();
	void    destroyFixMath ();
	*/
	//================================================================




	//---------------------------------------------------------------------
	template<int shift, class T>
	inline	Fixed<shift, T> operator-(int a, Fixed<shift, T> b)
	{
		Fixed<shift, T>	c=a;
		return c-=b;
	}
}//mpp

#endif	/* _MPP_FIXED_H_*/
