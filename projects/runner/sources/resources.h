/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _RESOURCES_H_
#define _RESOURCES_H_

#include	"prefs.h"

class map;

//===============================================================
//	местность
//===============================================================
class land
	:public collection
	,public static_class<land>
{
private:
	size<>		cage,
				ground,
				short_ground;
	
public:
	image		*background,
				broken_cage,	// для отрисовки разлома клетки
				door_open;

	animation	broken,
				door_close,
				door_active,
				box,
				decor_far,
				decor_near,
                fast,
                hard,
                ladder,
                objects,
                pile,
				pile_make,
				pile_unmake,
                rope,
                slow,
                soft,
				stones,
				slivers,
				monster,
				water;

	sound		*open_door,
				*activate_door,
				*pile_make_s,
				*pile_unmake_s,
				*move,
				*move_on_rope,
				*move_on_mud,
				*move_on_ladder,
				*sink,
				*complete_level,
				*player_break_cage,
				*player_grab_cash,
				*player_punch,
				*start_level,
				*box_punch,
				*box_explode,
				*box_sink,
				*sliver_punch,
				*stone_punch;

public:
	land(const string &_name);
	~land();
};



//===============================================================
//	тема
//===============================================================
class theme
	:public collection
	,public static_class<theme>
{
private:
	size<>		cage;
	
public:
	animation	bomb,
				break_mask,
				explosion,
				ice_smoke,
				objects,
				player,
				trap;

	sound	*raise,
			*player_exit,
			*player_birth_wait,
			*player_birth,
			*player_grab_bomb,
			*player_grab_item,
			*player_put_trap,
			*player_make_freeze,
			*player_suicide,
			*player_buy_live,
			*player_cannot_punch_box,
			*bomb_put,
			*bomb_timer,
			*bomb_explosion,
			*trap_catch,
			*froze,
			*monster_grab_cash,
			*monster_drop_cash,
			*monster_eat,
			*press_creature,
			*explode_creature,
			*flesh_punch,
			*crash;
                     
public:
	theme(const string &_name);
};




//===============================================================
//	карты
//===============================================================
class maps
	:public collection
	,public static_class<maps>
{
private:
	xml	description;
	               
public:
	maps(const string &_name);
	~maps();

	void save_map(map &m, const string &name);
	map *load_map(const string &name);
	
	static	string	&prefs(const string &name, const string &deflt="", bool atribute=false);
};

#endif
