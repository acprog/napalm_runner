#ifndef	_MPP_ARRAY_
#define	_MPP_ARRAY_

#include "tools.h"
#include "geometric.h"

namespace	mpp
{
	//==========================================================================
	// Класс линейного массива
	//==========================================================================
	template <class T>
	class array
	{
	private:
		T		*buff;
		int		s;

	public:
		//----------------------------------------------------------------------
		inline	array(int _size)
			:buff	(new T[_size])
			,s		(_size)
		{
		}

		//----------------------------------------------------------------------
		virtual	~array()
		{
			delete[] buff;
		}	

		//----------------------------------------------------------------------
		inline	T &operator[](int pos)
		{
			ASSERT(pos<0 || pos>=s, "array[ "+string(s, 10)+"]: invalid pos= ["+string(pos, 10))+"]";
			return buff[pos];
		}

		//----------------------------------------------------------------------
		inline T*operator+(int pos)
		{
			ASSERT(pos<0 || pos>=s, "array[ "+string(s, 10)+"]: invalid pos= ["+string(pos, 10))+"]";
			return buff+pos;
		}

		//----------------------------------------------------------------------
		inline operator T*()
		{
			return buff;
		}

		//----------------------------------------------------------------------
		inline	int count() const
		{
			return s;
		}
	};




	//==========================================================================
	// Класс двумерного массива
	//==========================================================================
	template <class T>
	class array2d
	{
	private:
		T		*buff, *p;
		size<>	s;
		point<>	*cursor,
				static_cursor;
		
	public:
		//----------------------------------------------------------------------
		inline	array2d(const size<> &_size)
			:buff	(new T[_size.area()])
			,s		(_size)
			,cursor	(NULL)
			,p		(NULL)
			,static_cursor	(0, 0)
		{
		}

		//----------------------------------------------------------------------
		virtual	~array2d()
		{
			delete[] buff;
			cursor=NULL;
			p=NULL;
		}	

		//----------------------------------------------------------------------
		inline	T &modify(const point<> &pos)
		{
			ASSERT(pos.x>=s.width || pos.x<0 || pos.y>=s.height || pos.y<0,
				"array[ "+s.width+"]["+s.height+"]: invalid pos= ["+pos.x+"]["+pos.y+"]");
			return buff[pos.x+pos.y*s.width];
		}

		//----------------------------------------------------------------------
		inline	const T &read(const point<> &pos) const
		{
			ASSERT(pos.x>=s.width || pos.x<0 || pos.y>=s.height || pos.y<0,
				"array[ "+s.width+"]["+s.height+"]: invalid pos= ["+pos.x+"]["+pos.y+"]");
			return buff[pos.x+pos.y*s.width];
		}

		//----------------------------------------------------------------------
		inline	T &operator[](const point<> &pos)
		{
			return modify(pos);
		}

		//----------------------------------------------------------------------
		inline	const size<> &count() const
		{
			return s;
		}

		//----------------------------------------------------------------------
		inline	void fill(const T &f)
		{
			T	*p=buff;
			for (int i=s.area(); --i>=0;)
				*(p++)=f;
		}

		//----------------------------------------------------------------------
		// простой перебор всех элементов - направление не важно
		// цикл быстрее 2-х вложенных for и лучше читается. при возможности лучше использовать его
		inline T *each_ptr(point<> *_cursor=NULL)
		{
			if (p)
			{
				if (--p<buff)
					p=NULL;
				else if (--cursor->x<0)
				{
					cursor->x+=s.width;
					cursor->y--;
				}
			}
			else
			{
				cursor=_cursor ? _cursor : &static_cursor;
				cursor->x=s.width-1;
				cursor->y=s.height-1;
				p=buff+s.area()-1;
			}
			
			return p;
		}

		//----------------------------------------------------------------------
		// перебор всех элем-в сначала по y потом по x c заданием направления
		inline T *each_ptr_yx(int dy=1, int dx=1, point<> *_cursor=NULL)
		{
			ASSERT(dx<-1 || dx>1 || dy<-1 || dy>1, "invalid step! dx="+dx+" dy="+dy);
			
			if (p)
			{
				p+=dx;
				cursor->x+=dx;
				if (coursor->x<0)
				{
					cursor->x+=s.width;
					if (dy>0)
						p+=s.width*2;
					cursor->y+=dy;
					if (cursor->y<0 || cursor->y>=s.height)
						p=NULL;
				}
				else if (cursor->x>=s.width)
				{
					cursor->x=0;
					if (dy<0)
						p-=s.width*2;
					cursor->y+=dy;
					if (cursor->y<0 || cursor->y>=s.height)
						p=NULL;
				}
			}
			else
			{
				cursor=_cursor ? _cursor : &static_cursor;
				cursor->x=dx>0 ? 0 : s.width-1;
				cursor->y=dy>0 ? 0 : s.height-1;
				p=&modify(*cursor);
			}
			
			return p;
		}

		//----------------------------------------------------------------------
		// перебор сначала по x потом по y c заданием направления
		inline T *each_ptr_xy(int dx=1, int dy=1, point<> *_cursor=NULL)
		{
			ASSERT(dx<-1 || dx>1 || dy<-1 || dy>1, "invalid step! dx="+dx+" dy="+dy);
			
			if (p)
			{
				if (dy>0)
					p+=s.width;
				else
					p-=s.width;
				cursor->y+=dy;
				if (coursor->y<0)
				{
					cursor->y+=s.height;
					p+=s.area();
					p+=dx;
					cursor->x+=dx;
					if (cursor->x<0 || cursor->x>=s.width)
						p=NULL;
				}
				else if (cursor->y>=s.height)
				{
					cursor->y=0;
					p-=s.area();
					p+=dx;
					cursor->x+=dx;
					if (cursor->x<0 || cursor->x>=s.width)
						p=NULL;
				}
			}
			else
			{
				cursor=_cursor ? _cursor : &static_cursor;
				cursor->x=dx>0 ? 0 : s.width-1;
				cursor->y=dy>0 ? 0 : s.height-1;
				p=&modify(*cursor);
			}
			
			return p;
		}

		//----------------------------------------------------------------------
		// другой порядок аргументов
		inline T *each_ptr_yx(point<> *_cursor, int dy=1, int dx=1)
		{
			return each_ptr_yx(dy, dx, _cursor);
		}

		//----------------------------------------------------------------------
		inline T *each_ptr_xy(point<> *_cursor, int dx=1, int dy=1)
		{
			return each_ptr_xy(dx, dy, _cursor);
		}

		//----------------------------------------------------------------------
		inline void break_each()
		{
			p=NULL;
		}
	};
}

#endif