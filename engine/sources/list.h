/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_LIST_H_
#define _MPP_LIST_H_

#include "tools.h"
#include "dobject.h"

namespace mpp
{
	//=====================================================================
	// представляется указателем на текущий элемент своего списка
	// предназначен для использования в циклах и динамического удаления своих элементов
	//=====================================================================
	template <class T>
	class list
	{
	private:
		//одна запись
		struct	one
		{
			T	value;
			one	*next;
		};
		one		begin,		// элемент перед первым
				*cursor,	// текущий элемент списка
				*end;		// последний эл-т списка

	public:
		//--------------------------------------------------------------
		inline list()
		{
			cursor=end=&begin;
		}

		//--------------------------------------------------------------
		virtual ~list()
		{
			clear();
		}

		//--------------------------------------------------------------
		void clear()
		{
			one	*curr=&begin,
				*next=begin.next;

			while (curr!=end)				
			{
				curr=next;
				next=curr->next;
				delete curr;
			}
			cursor=end=&begin;
		}

		//--------------------------------------------------------------
		// перебор всех эт-ов от начала до конца в цикле. возвращает текущий элемент
		T each()
		{
			if (cursor==end)
			{
				cursor=&begin;
				return NULL;
			}
			cursor=cursor->next;
			return cursor->value;
		}

		//--------------------------------------------------------------
		// перебор всех эт-ов от начала до конца в цикле. возвращает указатель на тек. элемент
		T *each_ptr()
		{
			if (cursor==end)
			{
				cursor=&begin;
				return NULL;
			}
			cursor=cursor->next;
			return &cursor->value;
		}


		//--------------------------------------------------------------
		// остановить перебор
		inline void break_each()
		{
			cursor=&begin;
		}

		//--------------------------------------------------------------
		// самая тяжелая ф-ия - удаление ей нельзя злоупотреблять
		inline	bool remove(const T &v)
		{
			one	*curr,
				*prev=&begin;

			while (prev!=end)
			{
				curr=prev->next;
				if (curr->value==v)
				{
					if (end==curr)
						end=prev;

					if (cursor==curr)
						cursor=prev;
					
					prev->next=curr->next;
					delete curr;
					return true;
				}
				prev=curr;
			}
			return false;
		}

		//--------------------------------------------------------------
		inline void push_back(const T &v)
		{
			end=end->next=new one;
			end->value=v;
		}

		//--------------------------------------------------------------
		inline void push(const T &v)
		{
			return push_back(v);
		}

		//--------------------------------------------------------------
		inline void push_start(const T &v)
		{
			one	*p=new one;
			p->next=begin.next;
			p->value=v;
			begin.next=p;
			if (end==&begin)
				end=p;
		}

		//--------------------------------------------------------------
		inline void push_sortup(const T &v)
		{
			one	*curr,
				*prev=&begin;

			while (prev!=end)
			{
				curr=prev->next;
				if (curr->value>=v)
				{
					prev->next=new one;
					prev->next->next=curr;
					prev->next->value=v;
					return;
				}
				prev=curr;
			}
			push_back(v);
		}

		//--------------------------------------------------------------
		inline void push_sortdown(const T &v)
		{
			one	*curr,
				*prev=&begin;

			while (prev!=end)
			{
				curr=prev->next;
				if (curr->value<=v)
				{
					prev->next=new one;
					prev->next->next=curr;
					prev->next->value=v;
					return;
				}
				prev=curr;
			}
			push_back(v);
		}

		//--------------------------------------------------------------
		// кол-во элементов
		inline int count()
		{
			one	*p=&begin;
			int i;
			for (i=0; p!=end; ++i)
				p=p->next;
			return i;
		}

		//--------------------------------------------------------------
		inline bool is_empty() const
		{
			return end==&begin;
		}


		//--------------------------------------------------------------
		// first elem of list
		T &get_first()
		{
			if (isEmpty())
				ERROR("empty list");
			return begin.next->value;
		}

		//--------------------------------------------------------------
		// last elem of list
		T &get_last()
		{
			if (isEmpty())
				ERROR("empty list");
			return end->value;
		}

		//--------------------------------------------------------------
		template <class T>
		int count() const
		{
			const one *p=&begin;
			int i;
			for (i=0; p!=end; ++i)
				p=p->next;
			return i;
		}

		//--------------------------------------------------------------
		T &operator[](int pos)
		{
			if (pos<0)
				ERROR("out of range : %d"+pos);
			one *p=&begin;
			while (pos-->=0)
			if (p==end)
				ERROR("out of range : "+pos);
			else
				p=p->next;
			return p->value;
		}


		//--------------------------------------------------------------
		int find(const T &v)
		{
			for (int index=0; T p=each(); index++)
				if (find(p==v))
				{
					break_each();
					return index;
				}

			return -1;
		}

		//--------------------------------------------------------------
		list<T> &operator=(list<T> &src)
		{
			clear();
			while (T *it=src.each())
				pushBack(it);
			return *this;
		}
	};




	//=================================================================
	//  Smart pointers list
	//=================================================================
	template <class T>
	class ptr_list : public list<pointer<T> >
	{
	protected:
		typedef pointer<T> PtrItem;

	public:
		//===============================================================
		T *each()
		{
			if (cursor==end)
			{
				cursor=&begin;
				return NULL;
			}
			cursor=cursor->next;
			return cursor->value;
		}

		//===============================================================
		ptr_list<T> &operator=(ptr_list<T> &src)
		{
			clear();
			while (T *it=src.each())
				push_back(it);
			return *this;
		}


		//=================================================================
		pointer<T> &push_sortup(const PtrItem &v)
		{
			one *curr,
				*prev=&begin;

			while (prev!=end)
			{
				curr=prev->next;
				if (*curr->value>*v)
				{
					prev->next=new(mem) one;
					prev->next->next=curr;
					return prev->next->value=v;
				}
				prev=curr;
			}
			return push_back(v);
		}


		//=================================================================
		pointer<T> &push_sortdown(const PtrItem &v)
		{
			one *curr,
				*prev=&begin;

			while (prev!=end)
			{
				curr=prev->next;
				if (!(*curr->value>*v))
				{
					prev->next=new(mem) one;
					prev->next->next=curr;
					return prev->next->value=v;
				}
				prev=curr;
			}
			return push_back(v);
		}
	};
}

#endif