#ifndef _EVENTS_H_
#define _EVENTS_H_

#include "base_objects.h"

//====================================================================
// ������������ �������
//====================================================================
class object_move
{
public:
	inline object_move(motion_object *_obj, move_type _type=move_normal)
		:obj		(_obj)
		,prev_pos	(_obj->pos_on_map())
		,type		(_type)
	{
	}

	motion_object	*obj;
    const point<>	prev_pos;
	move_type		type;
};


//====================================================================
// ���������� ����
//====================================================================
class	draw_objects
{
public:
	bool	bursts_only;
	image	*screen;
	rect<>	window;		// ���� ��� ��������� � ����������� maze
};


//====================================================================
// ����������� ������������� �����
//====================================================================
class check_pile
{
public:
	inline check_pile(const point<> &_pos)
		:pos		(_pos)
		,possible	(true)
   	{
   	}

	const point<> 	pos;
	bool			possible;
};



//====================================================================
// �������� ������������ ��� ���� ��������
//====================================================================
class	check_move
{
public:
	inline check_move(const point<> &_from, const point<> &dpos)
		:from	(_from)
		,to		(_from+dpos)
		,pass	(true)
		,box	(NULL)
	{
	}

	const point<>	from,
					to;
	bool			pass;
	box				*box;	// ���� ���� ����������� �������
};


//====================================================================
// �������� ������������ ��� ��������
//====================================================================
class	check_creature_move
{
public:
	inline check_creature_move(const point<> &_from, const point<> &dpos)
		:from	(_from)
		,to		(_from+dpos)
		,pass	(true)
	{
	}
	
	const point<>	from,
					to;
	bool			pass;
};



//====================================================================
// �������� ������������ ��� �������
//====================================================================
class	check_box_move
{
public:
	inline check_box_move(const point<> &_from, const point<> &dpos)
		:from	(_from)
		,to		(_from+dpos)
		,pass	(true)
	{
	}

	const point<>	from,
					to;
	bool			pass;
};



//====================================================================
//	��������� ������� � ������
//====================================================================
class crash_on_cage
{
public:
	broken_cage	*p;
};


//====================================================================
// ��������� �������
//====================================================================
class	grab_item
{
public:
	grab_item(item_type *_item, const point<> &_pos)
		:item	(_item)
		,pos	(_pos)
		,grabbed(false)
	{
	}	

	item_type		*item;
	const	point<>	pos;
	bool			grabbed;
};

//====================================================================
// ����� �������
//====================================================================
class door_active
{
public:
	point<>	pos;
};


//====================================================================
// ��������� �������� �� �����
//====================================================================
class draw_on_minimap
{
public:
	minimap	*p;
	image	*screen;	
};


#endif // _EVENTS_H_