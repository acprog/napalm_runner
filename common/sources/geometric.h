/***********************************************************

  Здесь содержатся классы точек, прямоугольных областей

***********************************************************/

#ifndef _MPP_GEOMETRIC_H_
#define _MPP_GEOMETRIC_H_

#include "fixed.h"

namespace	mpp
{
	//========================================================================================
	//	точка на плоскости
	//========================================================================================
	template <class X=int, class Y=X>
	class	point
	{
	public:
		//=============================================
		X	x;
		Y	y;
		
		//=============================================
		inline	point()
			:x(0)
			,y(0)
		{
		}

		//-------------------------------------
		inline	point(X _x, Y _y)
			:x(_x)
			,y(_y)
		{
		}

/*		//-------------------------------------
		inline	point(const point<X, Y> &_point)
			:x(_point.x)
			,y(_point.y)
		{
		}
*/
		//-------------------------------------
		template<class X2, class Y2>
		inline	point(const point<X2, Y2> &p)
			:x((X)p.x)
			,y((Y)p.y)
		{
		}

		//-------------------------------------
		template<class X2, class Y2>
		inline	const point<X, Y> &operator=(const point<X2, Y2> &_point)
		{
			x=_point.x;
			y=_point.y;
			return *this;
		}

		//-------------------------------------
		inline	const point<X, Y> &operator()(X _x, Y _y)
		{
			x=_x;
			y=_y;
			return *this;
		}

		//-------------------------------------
		inline	const point<X, Y> &operator+=(const point<X, Y> &p)
		{
			x+=p.x;
			y+=p.y;
			return *this;
		}

		//-------------------------------------
		inline	const point<X, Y> &operator-=(const point<X, Y> &p)
		{
			x-=p.x;
			y-=p.y;
			return *this;
		}

		//-------------------------------------
		inline	point<X, Y> operator+(const point<X, Y> &p) const
		{
			return point<X, Y>(x+p.x, y+p.y);
		}


		//-------------------------------------
		inline	point<X, Y> operator-(const point<X, Y> &p) const
		{
			return point<X, Y>(x-p.x, y-p.y);
		}

		//-------------------------------------
		inline	const point<X, Y> &operator+=(X dpos)
		{
			x+=dpos;
			y+=dpos;
			return *this;
		}

		//-------------------------------------
		inline	const point<X, Y> &operator-=(X dpos)
		{
			x-=dpos;
			y-=dpos;
			return *this;
		}

		//-------------------------------------
		inline	point<X, Y> operator+(X dpos)
		{
			return point<X, Y>(x+dpos, y+dpos);
		}

		//-------------------------------------
		inline	point<X, Y> operator-(X dpos)
		{
			return point<X, Y>(x-dpos, y-dpos);
		}

		//-------------------------------------
		inline	const point<X, Y> &operator*=(const point<X, Y> &mult)
		{
			x*=mult.x;
			y*=mult.y;
			return *this;
		}

		//-------------------------------------
		inline	const point<X, Y> &operator*=(int mult)
		{
			x*=mult;
			y*=mult;
			return *this;
		}

		//-------------------------------------
		inline	const point<X, Y> &operator*=(REAL mult)
		{
			x*=mult;
			y*=mult;
			return *this;
		}

		//-------------------------------------
		inline	const point<X, Y> &operator*=(double mult)
		{
			x*=mult;
			y*=mult;
			return *this;
		}

		//-------------------------------------
		inline	const point<X, Y> &operator/=(const point<X, Y> &divd)
		{
			x/=divd.x;
			y/=divd.y;
			return *this;
		}

		//-------------------------------------
		inline	const point<X, Y> &operator/=(int divd)
		{
			x/=divd;
			y/=divd;
			return *this;
		}

		//-------------------------------------
		inline	const point<X, Y> &operator/=(REAL divd)
		{
			x/=divd;
			y/=divd;
			return *this;
		}

		//-------------------------------------
		inline	const point<X, Y> &operator/=(double divd)
		{
			x/=divd;
			y/=divd;
			return *this;
		}

		//-------------------------------------
		template <class T>
		inline	point<X, Y> operator*(T mult) const
		{
			return point<X, Y>(*this)*=mult;
		}

		//-------------------------------------
		template <class T>
		inline	point<X, Y> operator/(T divd) const
		{
			return point<X, Y>(*this)/=divd;
		}

		//-------------------------------------
		inline point<X, Y> &shift(const point<> &dpos)
		{
			x+=dpos.x;
			y+=dpos.y;
			return *this;
		}

		//-------------------------------------
		inline	point<X, Y> x_offset(X dx) const
		{
			return point<X, Y>(x+dx, y);
		}

		//-------------------------------------
		inline	point<X, Y> y_offset(Y dy) const
		{
			return point<X, Y>(x, y+dy);
		}

		//-------------------------------------
		inline	bool operator==(const point<X, Y> &p) const
		{
			return x==p.x && y==p.y;
		}

		//-------------------------------------
		inline	bool operator!=(const point<X, Y> &p) const
		{
			return x!=p.x || y!=p.y;
		}
	};	// point










	//========================================================================================
	//	размер на плоскости
	//========================================================================================
	template <class X=int, class Y=X>
	class	size
	{
	public:
		X	width;
		Y	height;
		
		//-------------------------------------
		inline	size()
			:width(0)
			,height(0)
		{
		}

		//-------------------------------------
		inline	size(X _width, Y _height)
			:width(_width)
			,height(_height)
		{
		}

		//-------------------------------------
		inline	size(const size &s)
			:width(s.width)
			,height(s.height)
		{
		}

		//-------------------------------------
		inline	X area() const
		{
			return width*height;
		}

		//-------------------------------------
		template <class T>
		inline	const size<X, Y> &operator*=(T mult)
		{
			width*=mult;
			height*=mult;
			return *this;
		}

		//-------------------------------------
		template <class T>
		inline	const size<X, Y> &operator/=(T divd)
		{
			width/=divd;
			height/=divd;
			return *this;
		}

		//-------------------------------------
		template <class T>
		inline	size<X, Y> operator*(T mult) const
		{
			return size<X, Y>(width*mult, height*mult);
		}

		//-------------------------------------
		template <class T>
		inline	size<X, Y> operator/(T divd) const
		{
			return size<X, Y>(width/divd, height/divd);
		}

		//-------------------------------------
		inline size<X, Y> &operator+=(size<X, Y> &s)
		{
			width+=s.width;
			height+=s.height;
			return *this;
		}

		//-------------------------------------
		inline size<X, Y> operator+(size<X, Y> &s) const
		{
			return size<X, Y>(width+s.width, height+s.height);
		}

		//-------------------------------------
		inline size<X, Y> &operator+=(X ds)
		{
			width+=ds;
			height+=ds;
			return *this;
		}

		//-------------------------------------
		inline size<X, Y> operator+(X ds) const
		{
			return size<X, Y>(width+ds, height+ds);
		}

		//-------------------------------------
		inline	bool inside(const point<X, Y> &p) const
		{
			return	p.x>=0 && p.x<width && p.y>=0 && p.y<height;
		}

		//-------------------------------------
		inline	point<X, Y> force_inside(const point<X, Y> &p) const
		{
			point<>	p2=p;

			if (p2.x<0)
				p2.x=0;
			else if (p2.x>=width)
				p2.x=width-1;

			if (p2.y<0)
				p2.y=0;
			else if (p2.y>=height)
				p2.y=height-1;

			return	p2;
		}
	};	// size










	//========================================================================================
	//	прямоугольная область
	//========================================================================================
	template <class X=int, class Y=X>
	class	rect : public point<X, Y>, public size<X, Y>
	{
	public:
		//-------------------------------------------------------
		inline	rect()
		{
		}

		//-------------------------------------------------------
		inline	rect(const point<X, Y> &p)
			:point<X, Y>	(p)
		{
		}

		//-------------------------------------------------------
		inline	rect(const size<X, Y> &s)
			:size<X, Y>	(s)
		{
		}

		//-------------------------------------------------------
		inline	rect(const rect<X, Y> &_rect)
			:point<X, Y>(_rect)
			,size<X, Y>(_rect)
		{
		}

		//-------------------------------------------------------
		inline	rect(X _left_pos, Y _up_pos, X _width, Y _height)
			:point<X, Y>(_left_pos, _up_pos)
			,size<X, Y>(_width, _height)
		{
		}

		//-------------------------------------------------------
		inline	rect(const point<X, Y> &_left_up, X _width, Y _height)
			:point<X, Y>	(_left_up)
			,size<X, Y>		(_width, _height)
		{
		}

		//-------------------------------------------------------
		inline	rect(const point<X, Y> &_left_up, const size<X, Y> &_size)
			:point<X, Y>	(_left_up)
			,size<X, Y>		(_size)
		{
		}

		//-------------------------------------------------------
		inline	rect(const point<X, Y> &_left_up, const point<X, Y> &_right_down)
			:point<X, Y>	(_left_up)
			,size<X, Y>		(_right_down.x-_left_up.x, _right_down.y-_left_up.y)
		{
		}

		//-------------------------------------------------------
		inline	void set_left_top(const point<X, Y> &p)
		{
			width+=x-p.x;
			height+=y-p.y;
			(*this)=p;
		}

		//-------------------------------------------------------
		inline	void set_left_bottom(const point<X, Y> &p)
		{
			width+=x-p.x;
			height=p.y-y;
			x=p.x;
		}

		//-------------------------------------------------------
		inline	void set_right_top(const point<X, Y> &p)
		{
			width=p.x-x;
			height+=y-p.y;
			y=p.y;
		}

		//-------------------------------------------------------
		inline	void set_right_bottom(const point<X, Y> &p)
		{
			width=p.x-x;
			height=p.y-y;
		}

		//-------------------------------------------------------
		inline	X left() const
		{
			return x;
		}

		//-------------------------------------------------------
		inline	X right() const
		{
			return x+width;
		}

		//-------------------------------------------------------
		inline	Y top() const
		{
			return y;
		}

		//-------------------------------------------------------
		inline	Y bottom() const
		{
			return y+height;
		}

		
		//-------------------------------------------------------
		inline	point<X, Y> left_top() const
		{
			return *this;
		}

		//-------------------------------------------------------
		inline	point<X, Y> left_bottom() const
		{
			return point<X, Y>(x, y+height);
		}

		//-------------------------------------------------------
		inline	point<X, Y> right_top() const
		{
			return point<X, Y>(x+width, y);
		}

		//-------------------------------------------------------
		inline	point<X, Y> right_bottom() const
		{
			return point<X, Y>(x+width, y+height);
		}

		//-------------------------------------
		inline	bool inside(const point<X, Y> &p) const
		{
			return	p.x>=x && p.x<x+width && p.y>=y && p.y<y+height;
		}

		//-------------------------------------
		inline	point<X, Y> force_inside(const point<X, Y> &p) const
		{
			point<>	p2=p;

			if (p2.x<x)
				p2.x=x;
			else if (p2.x>=x+width)
				p2.x=x+width-1;

			if (p2.y<y)
				p2.y=y;
			else if (p2.y>=y+height)
				p2.y=y+height-1;

			return	p2;
		}
	};	// rect




/*

	//========================================================================================
	//	точка в пространстве
	//========================================================================================
	template <class X=float, class Y=X, class Z=Y>
	class vector : public point<X, Y>
	{
	public:
		Z	z;
		
		//-------------------------------------------------------------------------
		inline	vector()
			:z(0)
		{
		}

		//-------------------------------------------------------------------------
		inline	vector(const X *xyz)
			:point(xyz[0], (Y)xyz[1])
			,z	((Z)xyz[2])
		{
		}

		//-------------------------------------------------------------------------
		inline	vector(X _x, Y _y, Z _z=0)
			:point(_x, _y)
			,z(_z)
		{
		}

		//-------------------------------------------------------------------------
		inline	vector(const vector &v)
			:point(v)
			,z(v.z)
		{
		}

		//-------------------------------------------------------------------------
		inline	const vector<X, Y, Z> &set(X _x, Y _y, Z _z=0)
		{
			x=_x;
			y=_y;
			z=_z;
			return *this;
		}

		//-------------------------------------------------------------------------
		inline	operator const X*() const
		{
			return (const X*)&x;
		}

		//-------------------------------------------------------------------------
		inline	const vector<X, Y, Z> &operator=(X *xyz)
		{
			x=xyz[0];
			y=(Y)xyz[1];
			z=(Z)xyz[2];
			return *this;
		}

		//-------------------------------------------------------------------------
		inline	const vector<X, Y, Z> &operator-=(const vector<X, Y, Z> &v)
		{
			x-=v.x;
			y-=v.y;
			z-=v.z;
			return *this;
		}

		//-------------------------------------------------------------------------
		template<class T>
		inline	const vector<X, Y, Z> &operator*=(T mult)
		{
			x*=mult;
			y*=mult;
			z*=mult;
			return *this;
		}
		
//		point<T> xy(T x_c=1, T y_c=1);
//		point<T> yx(T x_c=1, T y_c=1);
//		point<T> xz(T x_c=1, T y_c=1);
//		point<T> zx(T x_c=1, T y_c=1);
//		point<T> yz(T x_c=1, T y_c=1);
//		point<T> zy(T x_c=1, T y_c=1);
	};








	//========================================================================================
	//	размер в пространстве
	//========================================================================================
	template <class X=int, class Y=X, class Z=Y>
	class	volume : public size<X, Y>
	{
	public:
		Z	length;
		
		//-------------------------------------
		inline	volume()
			:length	(0)
		{
		}

		//-------------------------------------
		inline	volume(X _width, Y _height, Z _length=0)
			:size		(_width, _height)
			,length	(_length)
		{
		}

		//-------------------------------------
		inline	volume(const volume &s)
			:size		(s)
			,length	(s.length)
		{
		}
	};	// volume
*/
}	// mpp
#endif /*_MPP_GEOMETRIC_H_*/
