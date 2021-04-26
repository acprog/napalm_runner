#ifndef _CAGE_H_
#define _CAGE_H_

#include "resources.h"

//-------------------------------------------------------------
// типы предметов
enum item_type
{
	key=0,			// ключ
	itrap=1,		// ловушка
	pick=2,			// кирка
	freeze=3,		// заморозка
	glass=4,		// кратковременная невидимость

	ibomb=5,		// бомба

	closed_door=6,	// закрытая дверь
	inactive_door=7,// неактивная дверь
	
	// начиная с этой, анимированные предметы
	active_door=8,	// активная дверь

	lo_cash=9,		// мало денег
	med_cash=10,	// средне денег
	hi_cash=11,		// много денег
	/*
	12-14 включетельно не могут использоваться т.к. '<','>' служебные в xml файле
    */

	//(используется при генерации)
	ibox=29,		// ящик
	iplayer=30,		// игрок 
	imonster=31,	// монстр 

	no_item=32,		// используеится при сменен предметов
};



//========================================================================
//	одна клетка
//========================================================================
union cage
{
//private:
private:
	uint8		value;

	// для таблиц переадресации
	struct
	{
		uint8	cage	:4,	// номер клетки по индексу
				type	:4;	// группа клетки по индексу
	}idc;	// иденификация клетки

	struct idt_tag
	{
		uint8	subtype		:2,	// обеспечивает разнообразие
				bottom		:1,	// определяет маску сопряжения с соседними клетками: низ
				top			:1,	//	верх
				local_type	:2,	// локальный тип клетки (не несет конкретной информации)
				near_plane	:1,	// ближняя плоскость на ближнем плане
				inpassable	:1;	// через поклетку можно проходить
	}idt;	// идентификация типа

	//типы групп клеток
	enum group_types
	{
		far_plane_group		=0,
		near_plane_group	=1,
		items_group			=2,
		inpassable_group	=3
	};

	struct
	{
		uint8	item	:6,		// номер предмета
				group	:2;		// тип группы
	}idi;	// иденнтификация предмета


public:
	enum types
	{
		// проходимые поверхности
		empty=7,		// пусто, ничего нет
		decor_far=1,	// дальняя декорация
		ladder=2,		// лестница
		broken=3,		// разрушенный soft	есть смысл анимировать
		decor_near=4,	// близкая декорация
		mirage=5,		// aka soft
		water=6,		// вода, тонет. анимированная
		rope=0,			// горизонтальная веревка

		//непроходимые поверхности
		hard=12,		// не рушиться
		slow=13,		// не рушиться, замедляет движение
		fast=14,		// не рушиться, ускоряет движение
		soft=15			// рушиться
	};

public:
	//---------------------------------------------------------------
	inline cage()
		:value (empty<<4)
	{
	}

	//---------------------------------------------------------------
	inline cage(const cage &c)
		:value (c.value)
	{
	}

	//---------------------------------------------------------------
	inline cage(uint8 v)
		:value	(v)
	{
	}

	//---------------------------------------------------------------
	inline cage(item_type i)
	{
		if (i==no_item)
			erase();
		else
		{
			idi.group=items_group;
			idi.item=i;
		}
	}

	//---------------------------------------------------------------
	inline cage(types t)
	{
		erase();
		idc.type=t;
	}
	
	//---------------------------------------------------------------
	inline bool open_door()
	{
		if (*this==closed_door)
		{
			idi.item=inactive_door;
			return true;
		}
		return false;
	}

	//---------------------------------------------------------------
	inline uint8 subtype()
	{
		return idt.subtype;
	}

	//---------------------------------------------------------------
	inline bool hard_place() const
	{
		return idc.type!=rope && idc.type!=ladder;
	}

	//---------------------------------------------------------------
	inline void correct_item()
	{
		if (idi.item==no_item)
			value=(empty<<4);
	}

	//---------------------------------------------------------------
	inline bool activate_door()
	{
		if (*this==inactive_door)
		{
			idi.item=active_door;
			return true;
		}
		return false;
	}

	//---------------------------------------------------------------
	inline	bool check_pile()
	{
		return idi.group!=inpassable_group;
	}

	//---------------------------------------------------------------
	inline	bool check_up_pile()
	{
		return idi.group==inpassable_group;
	}

	//---------------------------------------------------------------
	inline void erase()
	{
		value=(empty<<4);
	}

	//---------------------------------------------------------------
	inline bool operator==(item_type i) const
	{
		if (idi.group!=items_group)
			return i==no_item;
		return idi.item==i;
	}

	//---------------------------------------------------------------
	inline bool operator!=(item_type i) const
	{
		return !((*this)==i);
	}

	//---------------------------------------------------------------
	inline bool operator==(types t) const
	{
		return idc.type==t;
	}

	//---------------------------------------------------------------
	inline bool operator!=(types t) const
	{
		return idc.type!=t;
	}

	//---------------------------------------------------------------
	inline operator item_type() const
	{
		if (idi.group!=items_group)
			return no_item;
		return (item_type)idi.item;
	}

	//---------------------------------------------------------------
	inline operator types() const
	{
		return type();
	}

	//---------------------------------------------------------------
	inline types type() const
	{
		return (types)idc.type;
	}

	//---------------------------------------------------------------
	// клетка останавливает падение
	inline bool stop_drop() const
	{
		int type=idc.type;
		return	type==ladder 
			//||	type==rope
			||	type==hard
			||	type==slow
			||	type==fast
			||	type==soft;
	}	

	//---------------------------------------------------------------
	inline bool no_pass() const
	{
		return idt.inpassable && idi.group==inpassable_group;
	}

	//---------------------------------------------------------------
	inline operator bool() const
	{
		return value!=(empty<<4);
	}

	//---------------------------------------------------------------
	inline operator!() const
	{
		return value==(empty<<4);
	}

	//---------------------------------------------------------------
	inline void broken_to_mirage()
	{
		if (*this==broken)
			idc.type=mirage;
	}

	//---------------------------------------------------------------
	inline idt_tag &join()
	{
		return idt;
	}

	//---------------------------------------------------------------
	inline bool draw_near() const
	{
		return idt.near_plane;
	}

	//---------------------------------------------------------------
	inline bool inpassable() const
	{
		return idi.group==inpassable_group;
	}

	//---------------------------------------------------------------
	inline uint8 id() const
	{
		return idc.cage;
	}

	//---------------------------------------------------------------
	inline void dig()
	{
		idc.type=broken;
	}

	//---------------------------------------------------------------
	inline void undig()
	{
		idc.type=soft;
	}

	//---------------------------------------------------------------
	void	normalize(cage &next, bool in_editor=false);


	//---------------------------------------------------------------
	inline bool burst()
	{
		if (*this==soft)
		{
			idc.type=broken;
			return true;
		}
		
		if ((*this==decor_far || *this==decor_near) && idt.subtype<2)
		{
			*this=empty;
			return false;
		}

		item_type i=*this;
		if (i>=itrap && i<=ibomb)
		{
			*this=empty;
			return false;
		}
		return false;
	}
};


#endif