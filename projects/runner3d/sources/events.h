#ifndef _EVENTS_H_
#define _EVENTS_H_

#include "base_objects.h"

//====================================================================
// передвижение объекта
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
// отрисовать план
//====================================================================
class	draw_objects
{
public:
	bool	bursts_only;
	image	*screen;
	rect<>	window;		// окно для отрисовки в координатах maze
};


//====================================================================
// возможность использования кирки
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
// проверка передвижения для всех объектов
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
	box				*box;	// если путь преграждает коробка
};


//====================================================================
// проверка передвижения для созданий
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
// проверка передвижения для коробки
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
//	раздавить объекты в клетке
//====================================================================
class crash_on_cage
{
public:
	broken_cage	*p;
};


//====================================================================
// подобрать предмет
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
// дверь активна
//====================================================================
class door_active
{
public:
	point<>	pos;
};


//====================================================================
// отрисовка объектов на карте
//====================================================================
class draw_on_minimap
{
public:
	minimap	*p;
	image	*screen;	
};


#endif // _EVENTS_H_