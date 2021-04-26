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
	:public static_modal< game >
	,public pen_handler
	,public locked_screen
	,public dispatcher<burst>
	,public dispatcher<object_move>
	,public dispatcher<check_move>
	,public dispatcher<check_creature_move>
	,public dispatcher<check_box_move>
	,public dispatcher<creature_move>
	,public dispatcher<player_move>
	,public dispatcher<monster_move>
	,public dispatcher<draw_objects>
	,public dispatcher<crash_on_cage>
	,public dispatcher<broken_cage>
	,public dispatcher<grab_item>
	,public dispatcher<door_active>
	,public dispatcher<ice_smoke>
	,public dispatcher<check_pile>
	,public dispatcher<pile>
	,public dispatcher<box>
{
private:
	// подсчет fps
	timer				fps_counter;
	int					drawed_frames,
						fps;
	bool				pause;
	ptr_list<object>	for_remove;

	pointer<player>		current_player;

public:
	game(map *map, player_info pi);
	virtual ~game();

	//----------------------------------------------------------
	void event(redraw *screen, void*); 
	void event(timer *t, void*);
	void event(cycle *c, void*);
	
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
