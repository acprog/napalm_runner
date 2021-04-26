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
//	��������
//===============================================================
class base_maze 
	:public handler<timer>
	,public static_source< dispatcher<cycle>, application >
{
protected:
	rect<>			on_screen;
	map				&cages;
	const	image	*cage_groups[16];		// ����������� �����
	const	image	*cage_to_draw;
	int				cage_top_offset[16],	// �������� � ������ �� ������ �� ������� ������
					cage_bottom_offset[16];	// ��� ������� � ������ ��������� ������
	timer			*door_animate,
					*lo_cash_animate,
					*med_cash_animate,
					*hi_cash_animate,
					*water_animate;
	int				n_bursts;	// ������������ ��� ������ ������

    // ��������� ����������� ����������� ��������� �� ���� �������� �����������
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
	void event(timer *t, void*){};	// ����� �� ���� �������, � ��� ������� �� �������

	//------------------------------------------------------------
	void	center_to(const point<> &pos, const size<> &window);
	const image	*item_image(item_type i) const;
};	//maze




//===============================================================
//	������� ��������
//===============================================================
class maze 
	:public base_maze
	,public static_class<maze>
	,public static_source< handler<check_move>, game >
	,public static_source< handler<check_creature_move>, game >
	,public static_source< handler<object_move>, game >
	,public static_source< handler<creature_move>, game >
	,public static_source< handler<burst>, game >
	,public static_source< handler<grab_item>, game>
	,public static_source< handler<check_pile>, game> 
{
private:
	point<>			door;
	int				n_cash;		// ���-�� ������ �� ������ (���� ���������� ����� ����� ��������)

	void	burst_cage(const point<> &dst, const point<> &b);
	void	calc_cash();	// ������� ����-�� ������ � ���������
	void	make_objects();	// �������� ������� ��������
	bool	activate_door();

	//------------------------------------------------------------------------------
public:

	maze(map *map);

	//------------------------------------------------------------
	void event(burst *b, void*);
	void event(check_move *nd, void*);
	void event(check_creature_move *nd, void*);
	void event(object_move *nd, void*);
	void event(creature_move *c, void*);
	void event(grab_item *g, void*);
	void event(check_pile *p, void*);

	//------------------------------------------------------------
	void	copy_cage(image &dst, const point<> &pos);
	bool	break_cage(const point<> &pos);
	void	restore_cage(const point<> &pos);
	bool	open_door(const point<> &pos);
	void	get_one_cash();	// ������ ���������
	void	start_burst();
	bool	possible_punsh(const point<> &p);

	player	*create_player(player_info pi);	// ���� �� ����� ���, ���������� NULL
	monster *create_monster(player *target);
	
	//------------------------------------------------------------
	const point<>	&screen_offset() const
	{
		return on_screen;
	}
};	//maze

#endif	//_MAZE_H_
