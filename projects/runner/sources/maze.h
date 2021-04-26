/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MAZE_H_
#define	_MAZE_H_

#include "prefs.h"
#include "resources.h"
#include "cage.h"
#include "objects.h"
#include "map.h"

class player;
class game;



//===============================================================
//	лабиринт
//===============================================================
class base_maze 
	:public no_extern< handler<timer> >
	,public static_extern< send_down<cycle>, application >
{
protected:
	rect<>			on_screen;
	map				&cages;
	const	image	*cage_groups[16];		// изображения групп
	const	image	*cage_to_draw;
	int				cage_top_offset[16],	// смещение в группе от начала до текущей клетки
					cage_bottom_offset[16];	// для верхней и нижней половинок клетки
	timer			*door_animate,
					*lo_cash_animate,
					*med_cash_animate,
					*hi_cash_animate,
					*water_animate;
	int				n_bursts;	// используется для тряски камеры

    // установка соответсвия внутреннего указателя на кадр анимации поверхности
	inline	void set_land(int id, const	animation &anims)
	{
		cage_groups[id]=&(anims.get_frame(0, water_animate->progress()));
	}

	void	set_lands();
	void	draw_segment(image &screen, const point<> &cur, point<> p, bool near_plane);
	void	normalize_maze(bool in_editor=false);

	//------------------------------------------------------------------------------
public:

	base_maze(map *map);
	virtual ~base_maze();

	//------------------------------------------------------------
	void draw(image *screen, bool draw);
	void event(timer *t){};	// нужно не само событие, а его позиция во времени

	//------------------------------------------------------------
	void	center_to(const point<> &pos, const size<> &window);
	const image	*item_image(item_type i) const;
};	//maze




//===============================================================
//	Игровой лабиринт
//===============================================================
class maze 
	:public base_maze
	,public static_class<maze>
	,public static_extern< auto_free, game >
	,public static_extern< handler<check_move>, game >
	,public static_extern< handler<check_creature_move>, game >
	,public static_extern< handler<object_move>, game >
	,public static_extern< handler<creature_move>, game >
	,public static_extern< handler<burst>, game >
	,public static_extern< handler<grab_item>, game>
	,public static_extern< send_up<door_active>, game>
	,public static_extern< handler<check_pile>, game> 
{
private:
	point<>			door;
	int				n_cash;		// кол-во золота на уровне (чтоб определить когда выход возможен)

	void	burst_cage(const point<> &dst, const point<> &b);
	void	calc_cash();	// подсчет колл-ва золота в лабиринте
	void	make_objects();	// создание игровых объектов
	bool	activate_door();

	//------------------------------------------------------------------------------
public:

	maze(map *map);

	//------------------------------------------------------------
	void event(burst *b);
	void event(check_move *nd);
	void event(check_creature_move *nd);
	void event(object_move *nd);
	void event(creature_move *c);
	void event(grab_item *g);
	void event(check_pile *p);

	//------------------------------------------------------------
	void	copy_cage(image &dst, const point<> &pos);
	bool	break_cage(const point<> &pos);
	void	restore_cage(const point<> &pos);
	bool	open_door(const point<> &pos);
	void	get_one_cash();	// золото убавилось
	void	start_burst();
	bool	possible_punsh(const point<> &p);

	player	*create_player(player_info pi);	// если на карте нет, возвращает NULL
	monster *create_monster(player *target);
	
	//------------------------------------------------------------
	const point<>	&screen_offset() const
	{
		return on_screen;
	}
};	//maze

#endif	//_MAZE_H_
