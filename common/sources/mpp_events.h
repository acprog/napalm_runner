/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_EVENTS_
#define _MPP_EVENTS_

#include "image.h"
#include "event_handler.h"
#include "static_class.h"

namespace	mpp
{
	class platform;

	//================================================================
	//	������������ �� �����
	struct redraw
	{
		image	*ptr;
	};

	//================================================================
	// ��������� ����
	struct	cycle
	{
		int	n;	// ����� �����
	};

	//================================================================
	struct	key_down
	{
		int	key_id;
	};

	//================================================================
	struct	key_up
	{
		int	key_id;
	};


	//================================================================
	struct	key_char
	{
		char	ch;
	};

	
	//================================================================
	// ���������� ������� ����
	//================================================================
	class pen_handler;

	struct pen
	{
		point<>		pos;
		pen_handler	*target;	// ����� ����������� ������� ���� - ��� ����� ������������ ������� pen_move � pen_up
	};


	//====================================================================
	class pen_handler
		:public dispatcher<pen>
		,public rect<>
	{
	public:
		//-----------------------------------------------------------------
		inline	pen_handler(pen_handler *ext, const rect<> &win)
			:dispatcher<pen>	(NULL)
			,rect<>			(win)
		{
			if (external=ext)
				ext->add_internal(this);
		}


		//-----------------------------------------------------------------
		~pen_handler()
		{
			release_pen_handler(this);
		}

		//-----------------------------------------------------------------
		virtual void release_pen_handler(pen_handler *h)
		{
			if (external)
				((pen_handler*)external)->release_pen_handler(h);
		}

		//-----------------------------------------------------------------
		bool pass_event(pen *p)
		{
			if (inside(p->pos) && (p->target==external || p->target==NULL))
			{
				p->target=this;
				return true;
			}
			return false;
		}

		//--------------------------------------------------------
		inline	void add_internal(pen_handler *h)
		{
			// ��������� ������� ���� � �������� �������
			internals.push_start(h);
		}

		//-----------------------------------------------------------------
		// ������������� ������
		virtual void pen_down(const point<> &pos){};
		virtual void pen_move(const point<> &pos){};
		virtual void pen_up(const point<> &pos){};
	};
}

#endif