/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "game.h"
#include "runner.h"
#include "events.h"


//====================================================================
//	Игровой экран
//====================================================================
game::game(map *map, player_info pi)
	:fps_counter		(this, this, 1, timer::infinite)
	,pen_handler		(&application::get(), 
		rect<>(0, 0, 
		platform::get().screen_size().width, 
		platform::get().screen_size().height-TOOLBAR_HEIGHT))
	,drawed_frames		(0)
	,fps				(0)
	,pause				(false)
	,current_player		(NULL)
{
	runner::get().copy_to_buffer(map);

	new maze(map);
	new minimap(map->count()*2, *map, maze::get().screen_offset());
	new game_bar();

	current_player=maze::get().create_player(pi);
	if (!current_player)
		ERROR(common::str("error/player_place"));

	player	*p;
	while (p=maze::get().create_player(pi))
		delete p;

	while (maze::get().create_monster(current_player));

	current_player->set_info_ptrs();

	if (math::random(3))
		new loop(land::get(), "loop");
	else
		new loop(theme::get(), "loops/in game");
	land::get().start_level->play();	
}


//========================================================================
game::~game()
{
	game_bar::erase();
	game_menu::erase();
	maze::erase();
	minimap::erase();
}


//========================================================================
void game::event(redraw *screen, void *source)
{
	size<>	ss=platform::get().screen_size();
	ss.height-=TOOLBAR_HEIGHT;
	image	game_win(screen->ptr, ss);

	maze::get().center_to(point<>(CAGE_SIZE/2, CAGE_SIZE/2)+*current_player, ss);

	maze::get().draw(&game_win, false);
	
	draw_objects	p;
	p.screen=&game_win;
	p.window=rect<>(point<>(-1, -1)*CAGE_SIZE-maze::get().screen_offset(), ss+CAGE_SIZE);
	p.bursts_only=false;
	send(&p, this, NULL);

	maze::get().draw(&game_win, true);

	p.window=rect<>(point<>(-1, -1)*BURST_SIZE-maze::get().screen_offset(), ss+BURST_SIZE);
	p.bursts_only=true;
	send(&p, this, NULL);

	dispatcher<redraw>::event(screen, source);
#if 0
	drawed_frames++;
	const	font	&fnt=*common_collection::read_only().font;
	fnt.print(*screen->ptr, point<>(0, height-fnt.height), string("fps=")+fps);
#endif
}


//========================================================================
bool game::pass_event(redraw *screen)
{
	return !keys_binding::exist();
}


//========================================================================
void game::pen_down(const point<> &pos)
{
	if (pause)
		return;
	
	point<>	dpos=pos;
	dpos-=maze::get().screen_offset();
	dpos-=*current_player;
		
	rect<>	use_item(0, 0, CAGE_SIZE, CAGE_SIZE),
//			put_bomb(0, CAGE_SIZE, CAGE_SIZE, CAGE_SIZE),
			pick_left(-CAGE_SIZE, -CAGE_SIZE, CAGE_SIZE, CAGE_SIZE),
			pick_right(CAGE_SIZE, -CAGE_SIZE, CAGE_SIZE, CAGE_SIZE),
			break_right(CAGE_SIZE, CAGE_SIZE, CAGE_SIZE, CAGE_SIZE),
			break_left(-CAGE_SIZE, CAGE_SIZE, CAGE_SIZE, CAGE_SIZE);

	int	id=0;
	if (use_item.inside(dpos))
		id=runner::key().item;
//	else if (put_bomb.inside(dpos))
//		id=runner::key().put_bomb;
	else if (break_left.inside(dpos))
		id=runner::key().break_left;
	else if (break_right.inside(dpos))
		id=runner::key().break_right;
	else if (pick_left.inside(dpos))
	{
		current_player->push_pick(false);
		return;
	}
	else if (pick_right.inside(dpos))
	{
		current_player->push_pick(true);
		return;
	}
	else
	{
		dpos-=CAGE_SIZE/2;	// в центр
		if (dpos.x>abs(dpos.y))
			id=runner::key().right;
		else if (-dpos.x>abs(dpos.y))
			id=runner::key().left;
		else if (dpos.y>abs(dpos.x))
			id=runner::key().down;
		else
			id=runner::key().up;
	}
	application::get().penkey_down(id);
}


//========================================================================
void game::pen_up(const point<> &pos)
{
	if (!pause)
		application::get().penkey_up();
}

//========================================================================
void game::event(timer *t, void *source)
{
	if (t==&fps_counter)
	{
		fps=drawed_frames;
		drawed_frames=0;
	}
	locked_screen::event(t, source);
}



//========================================================================
void game::event(cycle *c, void *source)
{
	dispatcher<cycle>::event(c, source);
	for_remove.clear();
}



//========================================================================
bool game::pass_event(cycle *c)
{
	return !pause;
}


//========================================================================
bool game::pass_event(key_down *k)
{
	if (k->key_id==runner::key().pause && !controls::keys_binding::exist())
	{
		pause=!pause;

		if (pause)
			new game_menu();
		else
			game_menu::erase();
	}
	return !pause;
}


//========================================================================
int game::get_score() const
{
	return current_player->score;
}


//========================================================================
void game::restart()
{
	current_player->restart();
}