/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
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


struct crash_on_cage
{
	broken_cage	*p;
};

// ��������� �������
struct	grab_item
{
	item_type	*item;
	point<>		pos;
	bool		grabbed;
};

// ����� �������
struct door_active
{
	point<>	pos;
};


//====================================================================
// ��������� �������� �� �����
struct draw_on_minimap
{
	minimap	*p;
	image	*screen;	
};


// �������� ������������ ��� �������
struct	check_box_move
{
	point<>		from,
				to;
	bool		pass;
};



//====================================================================
//	�������� - ����� �������� � �������� ��� ���� ��������
//====================================================================
class creature
	:public motion_object
	,public static_extern< handler<burst>, game >
	,public static_extern< send_up<creature_move>, game >
	,public static_extern< send_up<check_creature_move>, game >
	,public static_extern< handler<ice_smoke>, game >
	,public static_extern< handler<pile>, game >
	,public static_extern< handler<box>, game >
	,public static_extern< handler<crash_on_cage>, game >
	,public static_extern< handler<check_box_move>, game >
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

	animated_state	*prev_state;	// ��������� ����� ����������
	timer			unfroze;		// �����������

	reversed_state	move_down;		// ����� �� ��������
	animated_state	birth,			// �������/������
					wait,			// ��� ��������
					move,			// ������������ �����
					drop,			// �������
					raise,			// ������ ����� �������
					move_on_rope,	// ����� �� �������
					move_on_mud,	// ����� �� �����
					move_up,		// ������ �� ��������
					trapped,		// ������ � �������
					frozen,			// ���������, ��������
					sink,			// �����
					press,			// �������������� ������
					piled,			// �������
					eat;			// ������ ������� ������
	
	void set_move_state(move_type type);

public:
	creature(const point<> &pos, const	animation *anims);
	

	void event(timer *t);
	void event(burst *b);
	void event(crash_on_cage *c);
	void event(ice_smoke *f);
	void event(pile *p);
	void event(box *b);
	void event(check_box_move *m);

	bool check_dpos(const point<> &dpos);
	void clash(bool after_drop, box *box);
	void move_on(const point<> &dpos, REAL speed);	//������ �� �������� ��� ��������
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
//	�����
//====================================================================
class player
	:public player_info
	,public creature
	,public static_extern< handler<key_down>, game >
	,public static_extern< handler<key_up>, game >
	,public static_extern< send_up<player_move>, game >
	,public static_extern< send_up<grab_item>, game >
	,public static_extern< handler<door_active>, game >
	,public static_extern< send_up<check_pile>, game>
{
private:
	player_info	startup_info;
	bool	continue_move,		// ���������� ��������
			on_rope;			// �� �������
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

	animated_state	break_cage,			// ������ ������ �����
					exit,				// ����� � ������
					punsh_up,			// ���� ������
					make_froze,			// �������� ���������
					put_trap,			// ������������� �������
					wait_for_birth,		// �������� �������� (������� ����� ������)
					punch_box;			// ���� �� �����
	reversed_state	unmake_froze,		// ������� ����� �������������
					punsh_down,			// ������� ����� ����� ������
					unpunch_box;		// ������� ����� ����� �� �����
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

	void event(key_down *p);
	void event(key_up *p);
	void event(timer *t);
	void event(door_active *d);
	
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
//	������
//====================================================================
class monster
	:public creature
	,public static_extern< handler<player_move>, game >
	,public static_extern< send_up<monster_move>, game >
	,public static_extern< handler<draw_on_minimap>, minimap>
{
private:
	player			*target;

public:
	//---------------------------------------------------------
	monster(const point<> &pos, player *target);
	void move_to_target();

	void event(timer *t);
	void event(player_move *p);
	void event(pile *p);
	void event(draw_on_minimap *p);

	bool search_way(point<> &pos, const point<> &to);
	bool check_dpos(const point<> &pos, const point<> &dpos);
	void move_to(const point<> &new_pos, bool teleport);
	void grab_object(item_type &item);
	void drop_grabbed();
};






//====================================================================
//	������� �����
//====================================================================
class bomb
	:public animated_object
	,public static_extern<handler<burst>, game>
{
private:
	animated_state	fire;
	timer			timeout;
	bool			bursted;

public:
	bomb(const point<> &pos);

	void event(timer *t);
	void event(burst *b);
};




//====================================================================
//	�����
//====================================================================
class burst
	:public animated_object
	,public static_extern<send_up<burst>, game>
{
private:
	animated_state	explosion;

public:
	burst(const point<> &pos);

	void event(timer *t);

	bool pass_event(draw_objects *p);

	bool burst_point(const point<> &p);
};





//====================================================================
//	����������� ������
//====================================================================
class broken_cage
	:public animated_object
	,public static_extern<handler<burst>, game>
	,public static_extern<handler<creature_move>, game>
	,public static_extern<handler<monster_move>, game>
	,public static_extern<send_up<broken_cage>, game>
	,public static_extern<send_up<crash_on_cage>, game>
{
private:
	animated_state	make;
	reversed_state	unmake;
	timer			wait;

public:
	//--------------------------------------------------------------------
	broken_cage(const point<> &pos, bool exploded=false);

	void event(draw_objects *p);
	void event(timer *t);
	void event(burst *b);
	void event(monster_move *mv);
	void event(creature_move *c);
};





//====================================================================
//	�������
//====================================================================
class trap
	:public object
	,public	static_extern< handler<creature_move>, game >
	,public	static_extern< handler<burst>, game >
{
private:
	object_state	wait;

public:
	trap(const point<> &pos);

	//------------------------------------------------------------------
	void event(burst *b);
	void event(creature_move *c);
};






//====================================================================
//	��������� ����
//====================================================================
class dropped
	:public animated_object
	,public static_extern< handler<creature_move>, game >
	,public static_extern< handler<grab_item>, game>
	,public	static_extern< handler<burst>, game >
{
private:
	item_type		item;

public:
	dropped(const point<> &pos, item_type _item);

	void event(creature_move *c);
	void event(grab_item *g);
	void event(draw_objects *p);
	void event(burst *b);
};






//====================================================================
//	���������
//====================================================================
class ice_smoke
	:public animated_object
	,public static_extern< handler<creature_move>, game>
	,public static_extern< send_up<ice_smoke>, game>
{
private:
	animated_state	wait;

public:
	ice_smoke(const point<> &pos, bool _mirror);
	void event(timer *t);
	void event(creature_move *c);
};



//====================================================================
//	����� �� ������
//====================================================================
class pile
	:public motion_object
	,public static_extern<handler<burst>, game>
	,public static_extern<handler<check_move>, game>
	,public static_extern<send_up<pile>, game>
{
private:
	animated_state	make,
					wait,
					unmake;

public:
	pile(const point<> &pos);

	void event(timer *t);
	void event(burst *b);
	void event(check_move *m);
	void event(draw_objects *p);

	void set_move_state(move_type type);
	void clash(bool after_drop, box *box);
	void remove();
};



//====================================================================
//	�������
//====================================================================
class box
	:public motion_object
	,public handler<burst>
	,public send_up<box>
	,public send_up<check_box_move>
{
private:
	object_state	wait;
	timer			friction;

public:
	box(const point<> &pos);

	void event(burst *b);
	void event(check_move *m);
	void event(timer *t);

	void clash(bool after_drop, box *box);
	void set_move_state(move_type type);
	void punch(bool to_right);
};





//====================================================================
//	����� �� ������
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

	void event(check_move *m);
	void event(object_move *m);
	void event(timer *t);
	void event(draw_objects *p);

	void set_move_state(move_type type);
	void need_drop();
	bool check_dpos(const point<> &dpos);
};


#endif	// _OBJECTS_H_
