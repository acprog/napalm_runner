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

//-----------------------------------------------------------
// ??????? ???????
class object_move;
class draw_objects;
class check_pile;
class check_move;
class check_creature_move;

//====================================================================
// ??? ???????????? (??????? ?? ???????????)
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




//====================================================================
//	????????? ???????, ??? ????????. ?????? ?? ????????
//====================================================================
class object_state
{
private:
	object	&obj;
	sound	*state_sound;	// ???? ????????????? ? ?????????? ??????? (???????)
	const	animation	*anims;			// ?? ?????????

protected:
	int		n_anims;		// ?.?. ??????? ??? ? ???????, ?? ?????? ???? ?????????? ?? object
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
//	????????????? ????????? ???????
//====================================================================
class animated_state
	:public object_state
	,public timer
{
private:
	//--------------------------------------------------------------------
	void event(cycle *c, void*)
	{
		timer::event(c, this);
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
//	????????????? ? ???????? ??????????? ????????? ???????
//====================================================================
class reversed_state
	:public animated_state
{
private:
	//--------------------------------------------------------------------
	void event(cycle *c, void*)
	{
		timer::event(c, this);
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
//	??????????? ??????? ??????
//====================================================================
class object 
	:public rect<>
	,public dobject
	,public static_source< handler<draw_objects>, game >
{
private:
	point<>		pos;			// ?????????? ?? ????????? ?????????
	
protected:
	const	animation		*ani;	// ?????????, ???? ?/? ?????????? NULL ??? ????????? ??????? ????????
	object_state	*state;
	bool			turn_to_right;	// ?????? ???????? ??????
	
	// ????????? ??? ???????? ??????
	static	sound	*no_sound;
	const	static	first_frame;
	const	static	last_frame;

	//=======================================================
	// ???????? ?????????
	template<class T>
	inline void send(T *ev, handler<T> &dest=game::get())
	{
		dest.event(ev, this);
	}

public:
	object(const point<> &pos, const animation *ani=NULL, const size<> &s=size<>(1, 1)*CAGE_SIZE);
	virtual ~object() { LOG("remove game object"); }

	void event(timer *t, void*){};
	void event(burst *b, void*){};
	void event(draw_objects *p, void*);

	//------------------------------------------------------------------------
	bool pass_event(draw_objects *p);

	//------------------------------------------------------------------------
	// ??????? ? ???????
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
	// ??????? ? ??????
	inline point<> &pos_on_screen()
	{
		return *this;
	}

	void play(sound *s);
};





//====================================================================
//	????????????? ??????
//====================================================================
class animated_object
	:public object
	,public handler<timer>
{
protected:
	animated_state	*mstate;

public:
	animated_object(const point<> &pos,  const animation *ani=NULL, const size<> &s=size<>(1, 1)*CAGE_SIZE);
	//------------------------------------------------------------------------
	void set_state(object_state	&s);
	void set_state(animated_state	&ms);
	void event(timer *t, void*){};	// ????? ????????????? ?? ??? ??????, ? ???????? ???????
};







//====================================================================
//	????????? ??????
//====================================================================
class motion_object 
	:public	animated_object
	,public handler<dynamic_point>
	,public static_source< handler<object_move>, game >
	,public static_source< handler<check_move>, game >
	,public static_source< handler<check_pile>, game>
	,public static_source< handler<broken_cage>, game>
	,public static_source< dispatcher<cycle>, game >
{
protected:
	dynamic_point	dynamic;	// ???????? ???????

private:
	move_type		mtype;
	size<>			bound;			// ???????? ??????? ???????
	bool			friction;		// ???? ?????? (? ?????? ???? ?? ?????????? ?????? ???? ???-?? ????????)
	REAL			speed;			// ??????????? ???????? ???????
	
protected:
	virtual bool check_dpos(const point<> &dpos);
	void move_on(const point<> &dpos, REAL speed);
	void stop();	// ????????????
	virtual	void move_to(const point<> &new_pos, bool teleport=true);	// ??????????? ????
	virtual	void set_move_state(move_type type){};	// ???? ?????? ????????????, ?????????? ? ???????????? ??? ?????????
	motion_object(const point<> &pos,  const animation *ani, const size<> &_bound, const size<> &s=size<>(1, 1)*CAGE_SIZE);
	
	void reset_state()
	{
		mtype=move_normal;
	}

	void center_cage();	// ????????????? ? ????? ??????
	void center_x();	// ?????????? ?? y

public:
	void event(dynamic_point *p, void*);
	void event(object_move *p, void*);
	void event(check_move *p, void*);
	void event(check_pile *p, void*);
	void event(broken_cage *b, void*);

	//---------------------------------------------------------------------
	virtual	void clash(bool after_drop, box *box){};
	virtual void force_sink(){};
	virtual	void need_drop();	// ??????
};


#endif