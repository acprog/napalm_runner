#ifndef _OBJECTS_H_
#define _OBJECTS_H_

#include "base_objects.h"

class player;
class monster;
class burst;
class broken_cage;
class ice_smoke;
class pile;
class minimap;

//===================================================================
typedef creature		creature_move;
typedef	player			player_move;
typedef	monster			monster_move;


class crash_on_cage;
class grab_item;
class door_active;
class draw_on_minimap;
class check_box_move;



//====================================================================
//	Создание - общая анимация и алгоримы для всех созданий
//====================================================================
class creature
	:public motion_object
	,public static_source< handler<burst>, game >
	,public static_source< handler<ice_smoke>, game >
	,public static_source< handler<pile>, game >
	,public static_source< handler<box>, game >
	,public static_source< handler<crash_on_cage>, game >
	,public static_source< handler<check_box_move>, game >
{
protected:
	point<>		birth_pos;
	bool	invisible;

	enum	animation_rows
	{
		row_wait,
		row_raise,
		row_move,
		row_move_on_rope,
		row_drop,
		row_move_on_mud,
		row_move_up,
		row_birth,
		row_trapped,
		row_frozen,
		row_sink,
		row_explode,
		row_press,
		row_pile,
		row_eat
	};

	item_type		item;

	animated_state	*prev_state;	// состояние перед заморозкой
	timer			unfroze;		// разморозить

	reversed_state	move_down;		// спуск по лестнице
	animated_state	birth,			// падение/подъем
					wait,			// без движения
					move,			// передвижение влево
					drop,			// падение
					raise,			// подъем после падения
					move_on_rope,	// влево по веревке
					move_on_mud,	// влево по грязи
					move_up,		// подъем по лестнице
					trapped,		// пойман в ловушку
					frozen,			// заморожен, разбился
					sink,			// тонет
					press,			// раздавливается землей
					piled,			// завален
					eat;			// монстр съедает игрока
	
	void set_move_state(move_type type);

public:
	creature(const point<> &pos, const	animation *anims);
	

	void event(timer *t, void*);
	void event(burst *b, void*);
	void event(crash_on_cage *c, void*);
	void event(ice_smoke *f, void*);
	void event(pile *p, void*);
	void event(box *b, void*);
	void event(check_box_move *m, void*);

	bool check_dpos(const point<> &dpos);
	void clash(bool after_drop, box *box);
	void move_on(const point<> &dpos, REAL speed);	//фильтр на комманды для созданий
	virtual	void kill();
	void crash();
	void move_to(const point<> &new_pos, bool teleport=false);
	virtual	void grab_object(item_type &item)=0;
	virtual void force_trapped();
	void froze(const point<> &p);
};


//====================================================================
struct player_info
{
	int lives,
		bombs,
		cash,
		score;
};


//====================================================================
//	Игрок
//====================================================================
class player
	:public player_info
	,public creature
	,public static_source< handler<key_down>, game >
	,public static_source< handler<key_up>, game >
	,public static_source< handler<door_active>, game >
{
private:
	player_info	startup_info;
	bool	continue_move,		// продолжить движение
			on_rope;			// на веревке
	box		*box_for_punch;

	enum	animation_rows
	{
		row_first=row_eat,
		row_break,
		row_exit,
		row_punsh,
		row_make_froze,
		row_put_trap,
		row_wait_for_birth,
		row_punch_box
	};

	animated_state	break_cage,			// ломает стенку слева
					exit,				// выход с уровня
					punsh_up,			// удар киркой
					make_froze,			// прыскает заморозку
					put_trap,			// устанавливает ловушку
					wait_for_birth,		// ожидание рождения (нажатие любой кнопки)
					punch_box;			// удар по ящику
	reversed_state	unmake_froze,		// возврат после замораживания
					punsh_down,			// возврат после удара киркой
					unpunch_box;		// возврат после удара по ящику
	point<>			door;

	void set_move_state(move_type type);
	void use_item();
	void move_to(const point<> &new_pos, bool teleport);
	void kill();
	void clash(bool after_drop, box *box);

public:
	//---------------------------------------------------------
	player(const point<> &pos, player_info i);

	void set_info_ptrs() const;

	void event(key_down *p, void*);
	void event(key_up *p, void*);
	void event(timer *t, void*);
	void event(door_active *d, void*);
	
	bool pass_event(draw_objects *p);

	void grab_object(item_type &item);
	
	bool visible()
	{
		return !invisible;
	}

	bool eat_me();
	void restart();

	void push_pick(bool _turn_to_right);
};





//====================================================================
//	Монстр
//====================================================================
class monster
	:public creature
	,public static_source< handler<player_move>, game >
	,public static_source< handler<draw_on_minimap>, minimap>
{
private:
	player			*target;

public:
	//---------------------------------------------------------
	monster(const point<> &pos, player *target);
	void move_to_target();

	void event(timer *t, void*);
	void event(player_move *p, void*);
	void event(pile *p, void*);
	void event(draw_on_minimap *p, void*);

	bool search_way(point<> &pos, const point<> &to);
	bool check_dpos(const point<> &pos, const point<> &dpos);
	void move_to(const point<> &new_pos, bool teleport);
	void grab_object(item_type &item);
	void drop_grabbed();
};






//====================================================================
//	Горящая бомба
//====================================================================
class bomb
	:public animated_object
	,public static_source<handler<burst>, game>
{
private:
	animated_state	fire;
	timer			timeout;
	bool			bursted;

public:
	bomb(const point<> &pos);

	void event(timer *t, void*);
	void event(burst *b, void*);
};




//====================================================================
//	Взрыв
//====================================================================
class burst
	:public animated_object
{
private:
	animated_state	explosion;

public:
	burst(const point<> &pos);

	void event(timer *t, void*);

	bool pass_event(draw_objects *p);

	bool burst_point(const point<> &p);
};





//====================================================================
//	Разрушенная клетка
//====================================================================
class broken_cage
	:public animated_object
	,public static_source<handler<burst>, game>
	,public static_source<handler<creature_move>, game>
	,public static_source<handler<monster_move>, game>
{
private:
	animated_state	make;
	reversed_state	unmake;
	timer			wait;

public:
	//--------------------------------------------------------------------
	broken_cage(const point<> &pos, bool exploded=false);

	void event(draw_objects *p, void*);
	void event(timer *t, void*);
	void event(burst *b, void*);
	void event(monster_move *mv, void*);
	void event(creature_move *c, void*);
};





//====================================================================
//	Ловушка
//====================================================================
class trap
	:public object
	,public	static_source< handler<creature_move>, game >
	,public	static_source< handler<burst>, game >
{
private:
	object_state	wait;

public:
	trap(const point<> &pos);

	//------------------------------------------------------------------
	void event(burst *b, void*);
	void event(creature_move *c, void*);
};






//====================================================================
//	брошенные вещи
//====================================================================
class dropped
	:public animated_object
	,public static_source< handler<creature_move>, game >
	,public static_source< handler<grab_item>, game>
	,public	static_source< handler<burst>, game >
{
private:
	item_type		item;

public:
	dropped(const point<> &pos, item_type _item);

	void event(creature_move *c, void*);
	void event(grab_item *g, void*);
	void event(draw_objects *p, void*);
	void event(burst *b, void*);
};






//====================================================================
//	Заморозка
//====================================================================
class ice_smoke
	:public animated_object
	,public static_source< handler<creature_move>, game>
{
private:
	animated_state	wait;

public:
	ice_smoke(const point<> &pos, bool _mirror);
	void event(timer *t, void*);
	void event(creature_move *c, void*);
};



//====================================================================
//	Завал из камней
//====================================================================
class pile
	:public motion_object
	,public static_source<handler<burst>, game>
	//,public static_source<handler<check_move>, game>
{
private:
	animated_state	make,
					wait,
					unmake;

public:
	pile(const point<> &pos);

	void event(timer *t, void*);
	void event(burst *b, void*);
	void event(check_move *m, void*);
	void event(draw_objects *p, void*);

	void set_move_state(move_type type);
	void clash(bool after_drop, box *box);
	void remove();
};



//====================================================================
//	Коробка
//====================================================================
class box
	:public motion_object
	,public handler<burst>
{
private:
	object_state	wait;
	timer			friction;

public:
	box(const point<> &pos);

	void event(burst *b, void*);
	void event(check_move *m, void*);
	void event(timer *t, void*);

	void clash(bool after_drop, box *box);
	void set_move_state(move_type type);
	void punch(bool to_right);
};





//====================================================================
//	Камни от взрыва
//====================================================================
class piece
	:public motion_object
{
private:
	timer	death;
	REAL	frame;
	int		n_anims;
	sound	*punch;
	
public:
	piece(const point<> &pos, const point<> &burst_pos, sound *_punch, const animation &_anims, int _n_anims=0);

	void event(check_move *m, void*);
	void event(object_move *m, void*);
	void event(timer *t, void*);
	void event(draw_objects *p, void*);

	void set_move_state(move_type type);
	void need_drop();
	bool check_dpos(const point<> &dpos);
};


#endif	// _OBJECTS_H_
