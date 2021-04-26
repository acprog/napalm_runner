/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _GAME_H_
#define _GAME_H_

#include "prefs.h"
#include "maze.h"
#include "objects.h"
#include "resources.h"
#include "bars.h"

class runner;


//====================================================================
//	Игровой экран
//====================================================================
class game 
	:public modal
	,public pen_handler
	,public static_class< game >
	,public locked_screen
	,public no_extern< send_down<burst> >
	,public no_extern< send_down<object_move> >
	,public no_extern< send_down<check_move> >
	,public no_extern< send_down<check_creature_move> >
	,public no_extern< send_down<check_box_move> >
	,public no_extern< send_down<creature_move> >
	,public no_extern< send_down<player_move> >
	,public no_extern< send_down<monster_move> >
	,public no_extern< send_down<draw_objects> >
	,public no_extern< send_down<crash_on_cage> >
	,public no_extern< send_down<broken_cage> >
	,public no_extern< send_down<grab_item> >
	,public no_extern< send_down<door_active> >
	,public no_extern< send_down<ice_smoke> >
	,public no_extern< send_down<check_pile> >
	,public no_extern< send_down<pile> >
	,public no_extern< send_down<box> >
//	,public static_extern<send_down<cycle>,		application>
{
private:
	// подсчет fps
	timer			fps_counter;
	int				drawed_frames,
					fps;
	bool			pause;
	list<object*>	for_remove;

	player			*current_player;

public:
	game(map *map, player_info pi);
	virtual ~game();

	//----------------------------------------------------------
	void event(redraw *screen); 
	void event(timer *t);
	void event(cycle *c);
	
	bool pass_event(key_down *k);
	bool pass_event(cycle *c);
	bool pass_event(redraw *screen);

	void pen_down(const point<> &pos);
	void pen_up(const point<> &pos);

	//-------------------------------------------------------------
	inline void remove_object(object *obj)
	{
		for_remove.push_back(obj);
	}

	int get_score() const;
	void restart();
};


#endif	/* _GAME_H_*/
