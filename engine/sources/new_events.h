/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_NEW_EVENTS_H_
#define _MPP_NEW_EVENTS_H_

/*
�� ������ ��������:
����� ������ �����������. �� �������������. �.�. ������ �������� ���������� �������, ��� ���� ����� ��� ����������
������� ��������� � ����, � ��� ��� ������� ��� ���������� � ��������� �� ����.
*/

namespace	new_events
{
	class	handler;
	class	listener;

	//�������� �������
	class	message
	{
		bool send()	// false ���� ����� �� ���������
		{
			bool	result=false;
			while (listener *l=listeners.each())
			{
				if (l->event(this))
					result=true;
				l->event(NULL);
			}	
			return result;
		}
        
    private:
    	list<listener*>	listeners;
	};
	
	// ��������� ����� ������ ������ ����������� ��� ���������� ���� � ������������ ������� ��� ������ ������
	// ������ ��������� �� �������� �������? � ��������, ��� ������� ��������� ������ listener ����� �����������
	// ������ ��������� ��������� �� handler, � � message ����� ��� ����������� ������ ����. �� �� � ������
	// ��������� ���� ���� "�� ����" �������� ������ �������, �� �������� �� ����� �������. �.�. �������� ���������,
	// � ���� ����� ������ ����� ��������������
	class listener
	{
		bool event(message *_s);	// ��� ���� �� ��������� ����������� �������, message ������� ������ ��� event, � ����� NULL
		{
			s=_s;
			if (s)
			{
				while (filter *f=filters.each())
					if (filters.find(!f->filter_event(s)))
						return false;
				return h->event(this);
			}
			return false;
		}

		void push_filter(filter *f);
		void remove_filter(filter *f);

	private:
		handler	*h; // ���������� �������
		message	*s;	// �������� �������. ��� ������������� ������� ��������� �� message ���������� �� ����
					// ����������. � ���, ��������� ���������� ���� ����� �������� ������ ���������� � hander
					// ��� ���� handler ����� ��������� �� ����� �� ����� ����� ������ � ������� ��������,
					// ��� ��� ����� ����������� � ������� message � listener
		list<filter*>	filters;
	};

	// ����� ��� �������� ����� ������������ wraper
	// ������� ������������� ���������� �� message. �.�. ��.������ � �������
	template<class T>
	class event : public listener
	{
		T *operator->() { return (T*)message; }
		operator *T()  { return (T*)message; }
	};


	// ����� ��� ��������� �������� ��������� ������ - �.�. ��������� message �������� ��������� ��������� listener
	// ��� ������� listener, ����� �� �������� �� ������ message? ����� ������� ������� �������� ��������� destroy
	// � ����� ���������� �� ����� ������ ��������� ����� �������� listener �� ���������� ����������. ������ ��� �������� �
	// �� ��. �������

	class handler
	{
		bool event(listener *ev); //���� false, ��������� �� ��������� ��������� ����������
	};

	// ��� ����� ������ ����������� �����
	// �� ��� ����� ���-�� ������� �� ������, ���� �� ������ ��� �������� ����������, �� ���� ������� �������� ���������� �����
	class reciver : public handler : public listener
	{
		bool handle(listener *ev)
		{
			return h->handle(ev);
		}
	};

	// � ��� ����� �������� ��� �����-������ ��������� (����� ��� �/� �������� ����� listener � handler.
	// �.�. �������� � listener ������ ����� ��� ����, ��� ������ �������� � ���� ������ ����� ���������� false,
	// �� �� ���������� ��������� �� ��������� handler) ��� ���� ������ ������������� ��������� ��������.
	// �.�. ���� ����������� ������ �������� ���� ��������� ��� � ���� listener, �� ����� �������������� ��������
	// ����������� ��� � ��������������� message-��. � ������ - ��� ������������� ��������� ���������� �������.
    // ��������� ��� ����� ������ - ������-�� message � listener -� ���� ��������� ������� �������?
	// �� OO� ��� ���, �� ������� ��� ���������� �� ���, ������� ������ �� ����� ���������. ��� ����
	// ����� �� ��� �/� ������ ��������� �� �����. ��� ������� ����
	class filter
	{
		bool filter_event(listener *ev)
		{
		}
	};
	/*
	!! ��� ����� ����� �������, ���� ��������� �����: 1message<->1listener. ����� ���� �����������
	������������. � ���� ����������� �������� �������� ����������+����������, �� ������� ����� �����
	dispetcher ������� ����� ������������� �� message � listener, ��������� ������ ����� + ������ ��������.
	����� ������� �������� ���� ����� ������, � �� ������� ������ ����. � ������ �������� ����� � ����������.
	��� �������� message ��� listener ��� filter ����� ������ �� ���� �������� ���������� � �� �� ������.
	���� ����� ����������� ������� �������� ���������, ����� ������� ��������� ������� ������ :)
	*/
	class dispatcher : public message : public listener
	{
		list<message>
		list<filter>
		list<listener>

	}

	// �.�. ���� ����������� ������, �� �� ��������� ������ ������ ����� ������� ���� message=NULL, �� ������� 
	// �������������� �����������-message; � ����� �����, ��������� ����� �����-�������� event-a
}


// handler ������������� �������������� �������� ���������, �� ��� ��������� ����� ������������ �� �������.
// �� ���������� ��������� ������� ���������� ��������� � ����������� ������ �� �������
class some : public new_events::hander
{
public:	// events
	event<mouse>		mouse;
	event<key_up, key_down, key_char>	keyboard;
    event<collision>	collision;
    event<cycle>		cycle;
    event<timer>		timer;

    event<creature_state>	move,
    						drop;

	bool event(new_events::listener *ev)
	{
		if (ev==&mouse)
		{
			mouse->point();
		}
		else if (ev==&keyboard)
		{			
			keyboard->key()
			keyboard->pressed(key)
		}
		else if (ev==&collision)
		{
			collision->force()
			collision->objects()
			collision->vector()
		}

	}
};


// � ���������� message �/� �����:
// ���� - ��� �������� ������ �� ����� � ��� ��� ��� ���������� � ����. �� ������ �������� ������ � ������ ��
// �� �������. ���. :)
class mouse : protected new_events::message
{
public:
	bool send(const point &p)
	{
		pos=p;
		return send();
	}

	const point &point() const { return pos; }

private:
	point pos;
};


/*
	��� ���� ����: ������-�� message ����� ����� ��������� � ���� ����? �� �������� �� ���?
	�������� ����� ��������� ���������, ���������� ������ ���������� � ������ ��������,
	� ������� � ���� ������ �������� ��������� ����� �����? ����� ���� ����� ����� ����� ������ ����� 
	���������, � ������� �� �������� ���������, ������� ��� �������� �� ��������. �.�. ���������� �����
	������� ��������� �������� ����������� �������. �������� � application ����� ����� ������� ����������
	����, ����� � ��.
	�.� �������� � ���������� ��������:
	1. ����� message - ��������� ����� - ���������. ����������� � ����� ���������������� 
		��� �������� ����������� �������� ���������
	2. ����� event - ���������� �����, ����������� ��������� � ��������� �������. 
		�� ����� ��������� �� ���������� event
	3. ����� handler - ������� ����� ��� ����������� �������. �������� ������ ���� �-�� event - ������ �������
	
	��� �������� ��������� ������ ����������� �������. �.�. �� ������� ������������ ������ ������ � ��� ������������
	� �������� ������ (� �������� ����� ����� ������ - ���� ������ message ������� ������������, �� ������ ����� �����
	����� ����� ����� ��������)

	������ � ��������������� ����� ����� ������� ����� �������� �� ����� � �������� ���������.
	�������� ���� ��������� �������. reciver, filter, sorter, switcher (����������, ������, ����������� � ����������)

	��� �� ����� ������ � ���������? ����������� � ���� �/� ������ ����������. ��� ���� �� ��� �/� ����������.
	� � ��������� ������ ���� ��������� �� ���� (����� ������� ���� �� ������)
	�.�. message �������� ��������� ��������, �� �� �� ����� ������ ������ ���������, �� ��� message �����
	� ���� ����� ��� ���������� ��� ����������
	
	�.�. ����� ������� ����� �������� ��������� ������ ������� ������ ������������ ������ �� message � �������� ���
	������� ������ ����������.. ������ ����� �������� �������� �����.. ��������-�� ����� ������ ��������� ����������.
	� ������ �������, ����� ���������� ��������� ������ �� ����� ������ ������ �� ���������� � ������������� ������ �������
	� ����...

	�.�. ������� 1: ����������� ���������� ���� �� ��������� ����. �� ������ ����� �������������� �����������.
	������� 2: ��������� - ��� ������ � ������ ��������� �� event<������ �����> - �������, �� ������� ���������� ������
	���������.. ������� ��� ���������� � ����� ������ �����..
	
	�.�. ������� 2 �� ��� ����, �� �� �� �������� ���������� � �������� ��������� event<����������� �����>...
	������ ���� �������� ����� listener, �� ����������� ����� ��������, �� ��������-�� handler?
	� ��� ����� ��� ���-�� ��������� ���� ������������� �� ���� �������� ��� ������, ���� ����� ���� �-��
	� ��������� void *�� ������� ����� ����� ���������� ��������..

	��� ��� �������������? ��� ��� ���� �����, ����������� �� �-� ���� handler � ��� ���� ��� ���������� �
	������� ��������� �������� ��������� �� �������... �� �������� �� ��� ��������. ���� ����� ��� � ����?
	1. ���� ��������-���������
	2. ���� �����-����������
	3. ����� ��������������. � ������������

	������� ������� � ��������������� - ������ ������������� ������� ����� ��� �����, � ������ ����������
	��������� ��������, ������� ����� �������� �� �����.

	�� ����� �������� ������-���������� ���������. �������� ��������� ������ ������ �����
	�� ������ �������� ������ �� ��������� ��� ������... ����� ������� ������� ��������� ������� � ��
    ���������� �������... ��� ��� ������ ��������? ����� -���� ��� ����� ��������� �������� ���������,
    � ����� ������������ ����� �������� ��� � ������. ������� � ��� ��� ������ up_sender/down_sender
    �� ����� ���������� ��������� ����� ������������, �.�. � ���� �� ��������� ����� ������. � ����� �������.
    ���� ������ ����� � ����� ���� - ����� ����� ������� ����� �������. ����� ������� �������� ��������� 
    ���������� ������ ��� ���������� �������. 
    ������ �� ����� ����� ��������������� � ��� ��� �� �������� ���������.. �� ��� ���� � ����������..
    �.�. � ����������� �������� ���������� ��������� �� ���������, ������ ������ ���������� ������� ���������
    �������� ����� � ���� ������ ����������� � ����������� �������� ���������.
    � ��� ����� ������� ���������-������� (������/������) ��� �������, ��� �������� ������������ ��������� ���������..
    �.�. �������� ������� ������ ��������� ����������� ������� �� �����, �� ������ ��������� ����� ��������� ����������.


    � ������ ����� �������� � ��� ��������� ��� ������� �������. �������� ��� ����� �����

    ������� ���, ������ ��������� � ���������� ������������ �������� ������������� � ��������� � �������
    ����������. �.�. � ��������� ����� ������� ���������� ���� ����������� � �� ������.
    ���� �� �������� �� ������� ������ ������� �� ����� �� �/� ���������� ����������.

    ������� ������� - � �������. ���������� �������� ��� ��������� �� ������� ��������� �� ��������, �.�.
    �� �� ���������� � ��������� ��� ������ ����� �������� ������ ������������ � ���������.
    ������ ����� ����� �����������, ��� ��������, � ������������ �������. ���� ��������
    �/� ����� �����. �������� ����� �-�� ���������� �������� ���������� ���������� ����������� �� ��������� �������
    �������� �����-�� ������� �����. �� ������ ������ ��������� (���� ����� �� ������������, �� �� ������ ����
    ������ ������ ���� ���������� ������� �����)

    ������ ��� � ��� ��� �������������� ����������:
    1. ��� ������ - ������� ������� - ������� �� �� �������.. � �� ��. ������. ���� ����� ��������� ��� ����� ��� ���������
    2. ��������� ����� - ���� �/� ��������. �� ������� ����� ��������� ����. �������, �� �������� ��� ����� ��������
    3. ���������� ���������� - ���� ��� � ����, �� ��������� �/� ��������� (�������� � �������.. ��� ���� ��� �/� ����� ���� �� ����)
    	� ��������, ��������� ���������� ������ ����������� ������ ���������. ����� ���������� � ��� ���������� �������
    4. ����������� ��������
    	��. ������ ������ ���� ����� ��������� � ��. ������ ��������� ���� � ������ �����������, ��
    	����������� ������ -���, � ���������� ������ ������� ������
    5 ����������� ����������
    	���� ��� 4, �� ���������� �/� ��������� ��� ������� 
*/

#endif // _MPP_NEW_EVENTS_H_
