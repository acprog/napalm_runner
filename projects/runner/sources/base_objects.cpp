/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "base_objects.h"
#include "maze.h"
#include "game.h"
#include "events.h"


//====================================================================
//	Состояние объекта, его анимация. объект не движется
//====================================================================
void object_state::start()
{
	if (state_sound)
		obj.play(state_sound);
}


//====================================================================
//	Состояние объекта, его анимация. объект не движется
//====================================================================
animated_state::animated_state(object *_obj, sound *ss, int _n_anims, animated_object *ext, REAL _period, int _n_repeats)
	:timer	(&game::get(), ext, _period, _n_repeats)
	,object_state(_obj, ss, _n_anims)
{
	stop();	// по умолчанию любое состояние неактивно
}


//====================================================================
animated_state::animated_state(object *_obj, sound *ss, const animation *_anims, animated_object *ext, REAL _period, int _n_repeats)
	:timer			(&game::get(), ext, _period, _n_repeats)
	,object_state	(_obj, ss, _anims)
{
	stop();	// по умолчанию любое состояние неактивно
}


//====================================================================
animated_state::animated_state(object *_obj, sound *ss, const animation *_anims, int _n_anims, animated_object *ext, REAL _period, int _n_repeats)
	:timer			(&game::get(), ext, _period, _n_repeats)
	,object_state	(_obj, ss, _anims, _n_anims)
{
	stop();	// по умолчанию любое состояние неактивно
}




//====================================================================
//	Игровой объект
//====================================================================
sound	*object::no_sound=NULL;
const	int object::first_frame=0;
const	int object::last_frame=1;

//====================================================================
object::object(const point<> &_pos, const animation *_ani, const size<> &s)
	:rect<>					(_pos*CAGE_SIZE, s)
	,ani					(_ani)
	,pos					(_pos)
	,state					(NULL)
	,turn_to_right			(false)
{
}


//====================================================================
bool object::pass_event(draw_objects *p)
{
	return !p->bursts_only && p->window.inside(pos_on_screen());
}


//====================================================================
void object::event(draw_objects *p, void*)
{
	state->draw(p->screen, *ani, pos_on_screen()+maze::get().screen_offset(), turn_to_right);
}



//====================================================================
void object::play(sound *s)
{
	// получаем вектор на звук
	point<>	sv=pos_on_screen()+maze::get().screen_offset();
	if (game::get().inside(sv))
		s->play();
}





//====================================================================
//	Анимированный объект
//====================================================================
animated_object::animated_object(const point<> &pos,  const animation *ani, const size<> &s)
	:object	(pos, ani, s)
	,mstate	(NULL)
{
}

//====================================================================
void animated_object::set_state(object_state &s)
{
	if (mstate)
	{
		mstate->stop();
		mstate=NULL;
	}
	state=&s;
}


//====================================================================
void animated_object::set_state(animated_state &ms)
{
	if (mstate)
		mstate->stop();
	if (mstate!=&ms)
		ms.start();
	else
		ms.unpause();
	state=mstate=&ms;
}











//====================================================================
//	Подвижный/анимированный объект
//====================================================================
motion_object::motion_object(const point<> &pos,  const animation *ani, const size<> &_bound, const size<> &s)
	:animated_object(pos, ani, s)
	,dynamic		(this, this, point<>(1, 1), point<>(0, 0))
	,mtype			(move_normal)
	,bound			(_bound/2)
	,friction		(false)
	,speed			(1)
{
	// координаты текущего объекта находятся пореди клетки
	dynamic.position()=pos_on_screen()+CAGE_SIZE/2;
}



//====================================================================
void motion_object::event(dynamic_point *p, void*)
{
	if (dynamic.speed().out().y>CAGE_SIZE)
	{
		dynamic.speed()=point<REAL>(0, CAGE_SIZE);
		dynamic.accelerate()=point<REAL>(0, 0);
	}

	if (friction)
		if (abs(dynamic.speed().out().x)<1)
		{
			friction=false;
			dynamic.stop();
		}

	point<>	new_pos=p->position().out();
	new_pos.y++;	// из-за приведения на 1 меньшее значение по y


	//---------------------------------------------------------------------
	// проверяем возможность перемещения
	point<>	speed=dynamic.speed().out(),
			cage_pos=pos_on_map()*CAGE_SIZE;
	if (speed.x>0)
	{
		if (cage_pos.x+CAGE_SIZE<new_pos.x+bound.width)
			if (!check_dpos(point<>(1, 0)))
				return;
	}
	else	if (speed.x<0)
	{
		if (cage_pos.x>new_pos.x-bound.width)
			if (!check_dpos(point<>(-1, 0)))
				return;
	}

	if (speed.y>0)
	{
		if (cage_pos.y+CAGE_SIZE<new_pos.y+bound.height)
			if (!check_dpos(point<>(0, 1)))
				return;
	}
	else	if (speed.y<0)
	{
		if (cage_pos.y>new_pos.y-bound.height)
			if (!check_dpos(point<>(0, -1)))
				return;
	}

	//---------------------------------------------------------------------
	// перемещаемся
	new_pos/=CAGE_SIZE;
	if (pos_on_map()!=new_pos)	
		move_to(new_pos, false);
	pos_on_screen()=p->position().out();
	pos_on_screen()-=CAGE_SIZE/2;
}



//====================================================================
// возможность дальнейшего перемещения
bool motion_object::check_dpos(const point<> &dpos)
{
	check_move	pm(pos_on_map(), dpos);
	send(&pm);
	if (!pm.pass)
	{
		// откат - перемещение невозможно
		pos_on_screen().y=pos_on_map().y*CAGE_SIZE;	// выраниваем y по высоте
		if (mstate)
			mstate->stop();
		clash(mtype==move_drop, pm.box);
		dynamic.stop();
		dynamic.position()=pos_on_screen()+CAGE_SIZE/2;
		return false;
	}
	return true;
}



//====================================================================
void motion_object::event(object_move *p, void*)
{
	if (p->obj->pos_on_map()==pos_on_map().y_offset(-1) && p->type==move_drop)
		p->type=move_normal;	// если объект переместился в клетку надомной
	else if (p->prev_pos==pos_on_map().y_offset(1))
		if (mtype!=move_up && mtype!=move_down)
			if (check_dpos(point<>(0, 1)))
				need_drop();	// если объект переместился из клетки подомной

}
	
	

//====================================================================
// переместится в указанную клетку
void motion_object::move_to(const point<> &new_cage, bool teleport)
{
	// узнаем тип передвижения и необходимость падения
	bool	drop=(mtype==move_drop);

	object_move	om(this, move_drop);
	pos_on_map()=new_cage;
	send(&om);
	
	//---------------------------------------------------------------------
	if (teleport)
	{
		drop=false;
		center_cage();
	}

	//---------------------------------------------------------------------
	if (mtype!=om.type)
	{
		switch (mtype)
		{
		case move_fast:
			if (friction)
			{
				friction=false;
				dynamic.stop();
			}
			else
				dynamic.speed()=point<REAL>(turn_to_right ? speed : 0-speed, 0);
			break;

		case move_mud:
			dynamic.speed()=point<REAL>(turn_to_right ? speed : 0-speed, 0);
			break;
		}
		set_move_state(mtype=om.type);
	}
	
	//---------------------------------------------------------------------
	switch (mtype)
	{
	case move_drop:
		if (!drop)
			need_drop();
		break;

	case move_fast:
		break;

	case move_mud:
		dynamic.speed()=point<REAL>((turn_to_right ? speed : 0-speed)/3, 0);
		break;

	case move_sink:
		dynamic.stop();
		return;
	}
	
	//---------------------------------------------------------------------
	if (drop && mtype!=move_drop)
	{
		// нужно остановить падение
		pos_on_screen().y=pos_on_map().y*CAGE_SIZE;	// выраниваем y по высоте
		dynamic.stop();
		dynamic.position()=pos_on_screen()+CAGE_SIZE/2;
		if (mtype!=move_rope)
			clash(true, NULL);
		else
			if (mstate)
				mstate->stop();
	}

	//---------------------------------------------------------------------
	// выраниваем y по высоте
	if (dynamic.speed().out().y==0)
	{
		point<REAL>	pos=dynamic.position().out();
		pos.y=pos_on_map().y*CAGE_SIZE+CAGE_SIZE/2;
		dynamic.position()=pos;
	}
}



//====================================================================
void motion_object::move_on(const point<> &dpos, REAL _speed)
{
	if (dpos.y==0)
		turn_to_right=dpos.x>0;

	speed=_speed;

	if (mtype==move_drop && dynamic.accelerate().out().y==0)
	{
		mtype=move_normal;
		if (mstate)
			mstate->unpause();
	}
	set_move_state(mtype);

	switch (mtype)
	{
	case move_fast:
		if (dpos.y==0)
		{
			friction=false;
			dynamic.accelerate()=point<REAL>(dpos)*=speed/60;
			return;
		}
		break;

	case move_rope:
		if (dpos.y>0)
		{
			need_drop();
			return;
		}
		break;

	case move_mud:
		_speed/=3;
		break;
	}
	dynamic.speed()=point<REAL>(dpos)*=_speed;
}



//====================================================================
void motion_object::stop()
{
	if (mtype!=move_fast)
		dynamic.stop();
	else
	{
		friction=true;
		dynamic.accelerate()=point<REAL>(dynamic.speed().out().x>0 ? -0.05 : 0.05, 0);
	}
		
	if (mstate)
		mstate->stop();
}



//====================================================================
void motion_object::need_drop()
{
	dynamic.speed()=dynamic.speed().out()*=0.2;
	dynamic.accelerate()=point<REAL>(0, 1);
	set_move_state(mtype=move_drop);
}


//====================================================================
void motion_object::event(check_move *p, void*)
{
	if (p->from.y!=p->to.y && pos_on_map()==p->to)
		if (p->pass)
			p->pass=(mtype==move_up || mtype==move_down);
}


//====================================================================
void motion_object::center_cage()
{
	pos_on_screen()=pos_on_map();
	pos_on_screen()*=CAGE_SIZE;
	dynamic.stop();
	dynamic.position()=pos_on_screen()+CAGE_SIZE/2;
}


//====================================================================
void motion_object::center_x()
{
	if (pos_on_screen().x&(CAGE_SIZE-1))
	{
		pos_on_screen().x=pos_on_map().x*CAGE_SIZE;
		dynamic.position()=pos_on_screen()+CAGE_SIZE/2;	
	}
}


//====================================================================
void motion_object::event(check_pile *p, void*)
{
	if (p->pos==pos_on_map())
		p->possible=false;
}


//====================================================================
void motion_object::event(broken_cage *b, void*)
{
	if (b->pos_on_map()==pos_on_map().y_offset(1))
		if (check_dpos(point<>(0, 1)))
			need_drop();
}