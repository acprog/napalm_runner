/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _BASE_OBJECTS_H_
#define _BASE_OBJECTS_H_

#include "prefs.h"
#include "cage.h"
#include "resources.h"


class game;
class animated_object;
class motion_object;
class burst;
class creature;
class object;
class box;
class broken_cage;

//====================================================================
// отрисовать план
struct	draw_objects
{

	bool	bursts_only;
	image	*screen;
	rect<>	window;		// окно для отрисовки в координатах maze
};


// возможность использования кирки
struct check_pile
{
	point<> pos;
	bool	possible;
};


//====================================================================
// тип передвижения (зависит от поверхности)
enum	move_type
{
	move_impossible,
	move_normal,
	move_rope,
	move_mud,
	move_fast,
	move_drop,
	move_up,
	move_down,
	move_sink
};


// объектом клетки
struct object_move
{
	motion_object	*obj;
    point<>			prev_pos;
	move_type		type;
};



// проверка передвижения для всех объектов
struct	check_move
{
	point<>		from,
				to;
	bool		pass;
	box			*box;	// если путь преграждает коробка
};


// проверка передвижения для созданий
struct	check_creature_move
{
	point<>		from,
				to;
	bool		pass;
};


//====================================================================
//	Состояние объекта, его анимация. объект не движется
//====================================================================
class object_state
{
private:
	object	&obj;
	sound	*state_sound;	// звук асоциированый с состоянием объекта (стартом)
	const	animation	*anims;			// по умолчанию

protected:
	int		n_anims;		// т.к. анимции все в группах, то группу буду передавать из object
	REAL	n_frame;
	
public:
	//--------------------------------------------------------------------
	object_state(object *_obj, sound *ss, int _n_anims=0, REAL	_frame=0)
		:n_anims	(_n_anims)
		,n_frame	(_frame)
		,state_sound(ss)
		,anims		(NULL)
		,obj		(*_obj)
	{
	}

	//--------------------------------------------------------------------
	object_state(object *_obj, sound *ss, const animation *_anims, int _n_anims=0)
		:n_anims	(_n_anims)
		,n_frame	(0)
		,anims		(_anims)
		,state_sound(ss)
		,obj		(*_obj)
	{
	}

	//--------------------------------------------------------------------
	virtual ~object_state()
	{}

	//--------------------------------------------------------------------
	inline void draw(image *screen, const animation &ani, const point<> pos, bool mirror)
	{
		if (anims)
			screen->put_safe(anims->get_frame(n_frame, n_anims), pos, true, mirror);
		else
			screen->put_safe(ani.get_frame(n_frame, n_anims), pos, true, mirror);
	}

	//--------------------------------------------------------------------
	void start();
};



//====================================================================
//	анимированное состояние объекта
//====================================================================
class animated_state
	:public object_state
	,public timer
{
private:
	//--------------------------------------------------------------------
	void event(cycle *c)
	{
		timer::event(c);
		n_frame=progress();
	}

public:
	//--------------------------------------------------------------------
	animated_state(object *_obj, sound *ss, int _n_anims, animated_object *ext, REAL _period, int _n_repeats=timer::infinite);
	animated_state(object *_obj, sound *ss, const animation *anims, animated_object *ext, REAL _period, int _n_repeats=timer::infinite);
	animated_state(object *_obj, sound *ss, const animation *anims, int _n_anims, animated_object *ext, REAL _period, int _n_repeats=timer::infinite);

	//--------------------------------------------------------------------
	inline void start()
	{
		object_state::start();
		timer::start();
	}
};




//====================================================================
//	анимированное в обратном направлении состояние объекта
//====================================================================
class reversed_state
	:public animated_state
{
private:
	//--------------------------------------------------------------------
	void event(cycle *c)
	{
		timer::event(c);
		n_frame=1-progress();
	}

public:
	//--------------------------------------------------------------------
	reversed_state(object *_obj, sound *ss, int _n_anims, animated_object *ext, REAL _period, int _n_repeats=timer::infinite)
		:animated_state(_obj, ss, _n_anims, ext, _period, _n_repeats)
	{
	}
};

















//====================================================================
//	Неподвижный игровой объект
//====================================================================
class object 
	:public rect<>
	,public static_extern< auto_free, game >
	,public static_extern< handler<draw_objects>, game >
{
private:
	point<>		pos;			// координаты на плоскости лабиринта
	
protected:
	const	animation		*ani;	// указатель, чтоб м/б передавать NULL при отсутсвии готовой анимации
	object_state	*state;
	bool			turn_to_right;	// объект повернут вправо
	
	// константы для удобства записи
	static	sound	*no_sound;
	const	static	first_frame;
	const	static	last_frame;

public:
	object(const point<> &pos, const animation *ani=NULL, const size<> &s=size<>(1, 1)*CAGE_SIZE);

	void event(timer *t){};
	void event(burst *b){};
	void event(draw_objects *p);

	//------------------------------------------------------------------------
	bool pass_event(draw_objects *p);

	//------------------------------------------------------------------------
	// позиция в клетках
	inline point<> &pos_on_map()
	{
		return pos;
	}

	//------------------------------------------------------------------------
	inline void set_state(object_state &st)
	{
		state=&st;
	}

	//------------------------------------------------------------------------
	// позиция в точках
	inline point<> &pos_on_screen()
	{
		return *this;
	}

	void play(sound *s);
};





//====================================================================
//	Анимированный объект
//====================================================================
class animated_object
	:public object
	,public no_extern< handler<timer> >
{
protected:
	animated_state	*mstate;

public:
	animated_object(const point<> &pos,  const animation *ani=NULL, const size<> &s=size<>(1, 1)*CAGE_SIZE);
	//------------------------------------------------------------------------
	void set_state(object_state	&s);
	void set_state(animated_state	&ms);
	void event(timer *t){};	// может потребоваться не сам таймер, а анимация объекта
};







//====================================================================
//	Подвижный объект
//====================================================================
class motion_object 
	:public	animated_object
	,public no_extern< handler<dynamic_point> >
	,public static_extern< send_up<object_move, and_receive>, game >
	,public static_extern< send_up<check_move, and_receive>, game >
	,public static_extern< handler<check_pile>, game>
	,public static_extern< handler<broken_cage>, game>
	,public static_extern< send_down<cycle>, game >
{
protected:
	dynamic_point	dynamic;	// динамика объекта

private:
	move_type		mtype;
	size<>			bound;			// половина размера объекта
	bool			friction;		// сила трения (а вообще силы не подвласные игроку надо как-то выделить)
	REAL			speed;			// номинальная скорость объекта
	
protected:
	virtual bool check_dpos(const point<> &dpos);
	void move_on(const point<> &dpos, REAL speed);
	void stop();	// остановиться
	virtual	void move_to(const point<> &new_pos, bool teleport=true);	// передвинуть себя
	virtual	void set_move_state(move_type type){};	// если объект перемещается, установить в соответствие его состояние
	motion_object(const point<> &pos,  const animation *ani, const size<> &_bound, const size<> &s=size<>(1, 1)*CAGE_SIZE);
	
	void reset_state()
	{
		mtype=move_normal;
	}

	void center_cage();	// перемсестится в центр клетки
	void center_x();	// центруется по y

public:
	void event(dynamic_point *p);
	void event(object_move *p);
	void event(check_move *p);
	void event(check_pile *p);
	void event(broken_cage *b);

	//---------------------------------------------------------------------
	virtual	void clash(bool after_drop, box *box){};
	virtual void force_sink(){};
	virtual	void need_drop();	// падать
};



#endif