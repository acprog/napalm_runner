#include "runner.h"
#include "game.h"
#include "resources.h"

//========================================================================
application *application::create()
{
	return new runner;
}


extern void ogg_test();

//========================================================================
//	Napalm Runner
//========================================================================
runner::runner()
	:application("naPalm 3D Runner", 20, APP_VERSION)
	,buffer		(NULL)
{
	platform::get().srand();

	new theme(prefs("theme", "runner", true));
	new loop(theme::get(), "loops/menu");
	new common(prefs("interface", "english", true), "interface");

	keys.left		=prefs("keys/move_left", "", true).atoi();
	keys.right		=prefs("keys/move_right", "", true).atoi();
	keys.up			=prefs("keys/move_up", "", true).atoi();
	keys.down		=prefs("keys/move_down", "", true).atoi();
	keys.break_left	=prefs("keys/dig_left", "", true).atoi();
	keys.break_right=prefs("keys/dig_right", "", true).atoi();
	keys.put_bomb	=prefs("keys/put_bomb", "", true).atoi();
	keys.item		=prefs("keys/use_item", "", true).atoi();
	keys.pause		=prefs("keys/pause", "", true).atoi();

	new main_menu;
}


//========================================================================
runner::~runner()
{
	if (buffer)
		delete buffer;
	theme::erase();
	land::erase();

	prefs("keys/move_left")	=string(keys.left, 10);
	prefs("keys/move_right")=string(keys.right, 10);
	prefs("keys/move_up")	=string(keys.up, 10);
	prefs("keys/move_down")	=string(keys.down, 10);
	prefs("keys/dig_left")	=string(keys.break_left, 10);
	prefs("keys/dig_right")	=string(keys.break_right, 10);
	prefs("keys/put_bomb")	=string(keys.put_bomb, 10);
	prefs("keys/use_item")	=string(keys.item, 10);
	prefs("keys/pause")		=string(keys.pause, 10);
}



//========================================================================
void runner::copy_to_buffer(map *m)
{
	if (buffer)
		delete buffer;
	buffer=new map(*m, m->count());
}


//========================================================================
void runner::restart_level(player_info i)
{
	map	*m=buffer;
	buffer=NULL;
	new game(m, i);
	free_unused();
}


//========================================================================
void runner::next_level(player_info pi)
{
	map	*m=buffer;
	buffer=NULL;
	map *new_map=m->next_level();
	if (!new_map)
	{
		new loop(theme::get(), "loops/menu");
		new adventure();	// набор закончен
	}
	else
	{
		if (maps::exist())
		{
			maps::prefs("save/lives")=string(pi.lives, 10);
			maps::prefs("save/cashs")=string(pi.cash, 10);
			maps::prefs("save/bombs")=string(pi.bombs, 10);
		}
		else
			pi.score+=new_map->difficult();
		new game(new_map, pi);
	}		
	delete m;
	free_unused();
}