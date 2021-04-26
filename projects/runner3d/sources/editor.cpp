/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "editor.h"


//====================================================================
//	Редактор
//====================================================================
editor::editor(map *map, const string &_name)
	:pen_handler		(&application::get(), 
		rect<>(0, 0, 
		platform::get().screen_size().width, 
		platform::get().screen_size().height-TOOLBAR_HEIGHT))
	,base_maze	(map)
	,name		(_name)
{
	show_minimap();
	new edit_bar;
	normalize_maze(true);
	maps::get().save_map(cages, name);	// если карта была сгенирированна, сохранить
}


//========================================================================
editor::~editor()
{
	edit_bar::erase();
	objects_bar::erase();
	terrain_bar::erase();
	minimap::erase();
}


//========================================================================
void editor::close()
{
	maps::get().save_map(cages, name);
	new maps_manager;
}


//========================================================================
void editor::undo_all()
{
	map	*m=maps::get().load_map(name);
	cages=*m;
	delete m;
	normalize_maze(true);
	minimap::get().refresh();
}

//========================================================================
void editor::show_minimap() const
{
	new editor_minimap(cages.count()*2, cages, on_screen);
}


//========================================================================
void editor::set_brush(const cage c)
{
	brush=c;
}



//========================================================================
void editor::event(redraw *screen, void *source)
{
	size<>	ss=platform::get().screen_size();
	ss.height-=TOOLBAR_HEIGHT;
	image	game_win(screen->ptr, ss);

	draw(&game_win, false);
	draw(&game_win, true);
	
	dispatcher<redraw>::event(screen, source);
}


//========================================================================
void editor::pen_down(const point<> &pos)
{
	point<> p=cages.count().force_inside((pos-on_screen)/CAGE_SIZE);
	cage	*c=&cages[p];
	*c=brush;
	
	c[-cages.count().width+((p.y>0 ? 0 : cages.count().area()))].normalize(*c, true);
	c->normalize(c[cages.count().width-((p.y<cages.count().height-1 ? 0 : cages.count().area()))], true);
	if (minimap::exist())
		minimap::get().refresh();
}


//========================================================================
void editor::pen_move(const point<> &pos)
{
	pen_down(pos);
}


