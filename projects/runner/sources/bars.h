/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _BARS_H_
#define _BARS_H_

#include "prefs.h"
#include "cage.h"
#include "objects.h"


class game;
class editor;
class map;
class draw_on_minimap;

using controls::form;
using controls::button;
using controls::virtual_key;
using controls::picture;
using controls::input;
using controls::text_string;

//========================================================================
//	main menu
//========================================================================
class main_menu
	:public static_modal<main_menu>
	,public form
{
private:
	text_string
		napalm_games,
		ver;
	button
		new_game,
		options,
		editor,
		registration,
		hi_scores,
		exit;
	picture
		logo;

public:
	main_menu();
	virtual ~main_menu(){}
	void event(controls::button *b, void*);
};




//========================================================================
//	top10
//========================================================================
class top10
	:public static_modal<top10>
	,public form
	,public handler<controls::keyboard>
{
private:
	button		back,
				start;
	controls::list
				scores;
	int			new_score;

	void push_string(const string &name);

public:
	top10(int score=0);
	~top10();
	void event(button *b, void*);
	void event(controls::keyboard *k, void*);
};





	
//========================================================================
//	Game options
//========================================================================
class options
	:public static_modal<options>
	,public form
{
private:
	button
		exit,
		keys;
	controls::digital
		music_volume,
		sound_volume;
	controls::list
		interfaces,
		thems,
		screen_angle,
		sound_quality;
	bool	music_disabled;

public:
	options();
	~options();
	void event(button *b, void*);
	void event(controls::digital *d, void*);

	bool pass_event(redraw *screen);
};



//========================================================================
//	Try again
//========================================================================
class try_again
	:public static_modal<try_again>
	,public form
	,public handler<timer>
{
private:
	text_string	message;
	timer		restart;
	player_info pi;

public:
	try_again(player_info pi);
	void event(timer *t, void*);
	void pen_down(const point<> &p);
	void pen_up(const point<> &p);
};




//========================================================================
//	Level complete
//========================================================================
class level_complete
	:public static_modal<level_complete>
	,public form	
{
private:
	picture		banner;
	text_string	info;
	player_info	pi;

public:
	level_complete(player_info pi);
	void event(key_down *k, void*);
	void pen_up(const point<> &p);
};




//========================================================================
//	game over
//========================================================================
class game_over
	:public static_modal<game_over>
	,public form	
{
private:
	picture		banner;
	text_string	info;
	int			score;

public:
	game_over(int score);
	void event(key_down *k, void*);
	void pen_up(const point<> &p);
};






//========================================================================
//	Make Random map
//========================================================================
class random_map
	:public static_modal<random_map>
	,public form
{
private:
	size<>	s;
	string	name,
			land;

	button	back,
			random,
			start;
	controls::digital
		maze_density,
		maze_space,
		ladders_size,
		soft,
		hard,
		ice,
		mud,
		rope,
		cashs,
		monsters,
		items,
		bombs,
		boxes;
	
public:
	random_map(const size<>	&s, const string &name, const string &land);
	void event(button *b, void*);
	void event(controls::digital *d, void*);
};


//====================================================================
//	Миникарта
//====================================================================
class minimap
	:public form
	,public static_class<minimap>
	,public dispatcher<draw_on_minimap>
{
private:
	image			img;
	rect<>			viewport;
	const	map		&cages;
	const	point<>	&linkview;
	COLOR			window,
					impassable,
					ladder,
					water,
					cash;

	COLOR	get_minimap_color(cage c) const;

public:
	minimap(const size<> &mm_size, const map &cages, const point<> &linkview);
	void event(redraw *screen, void*);
	void set_view(const point<> &p);
	void erase_point(const point<> &p);
	void draw_point(image &screen, const point<> &p, COLOR c);
	void refresh();
};


	
	
//====================================================================
//	Панель управления в игре
//====================================================================
class game_bar
	:public form
	,public static_class<game_bar>
{
private:
	const	int		*n_lives,
					*n_bombs,
					*n_cash;
	
	virtual_key		bomb;
	button			kill,
					pause;
	picture			cash;				
	
	//=======================================================================================
	class item
		:public virtual_key
		,public dobject
	{
	private:
		const	item_type	*i;

	public:
		item(form *parent, const item_type *_item);
		void event(redraw *screen, void*);
	};

	ptr_list<item>	items;

	//=====================================================================================
public:

	game_bar();
	void event(redraw *screen, void*);
	void event(button *b, void*);
	void set(const int *_lives, const int *_bombs, const int *_cash, const item_type *_item);
};




//====================================================================
//	Меню в игре
//====================================================================
class game_menu
	:public form
	,public static_class<game_menu>
{
private:
	button	exit,
			restart,
			controls,
			resume;
	controls::digital
			music_volume,
			sound_volume;
	text_string
			no_music;

public:
	game_menu();
	void event(button *b, void*);
	void event(controls::digital *d, void*);
};







//========================================================================
//	Maps manager
//========================================================================
class maps_manager
	:public static_modal<maps_manager>
	,public form
	,public handler<controls::keyboard>
{
private:
	button
		create_group,
		remove_group,
		back,
		create,
		remove,
		open;
	controls::list
		groups,
		lmaps;

public:
	maps_manager();
	void event(button *b, void*);
	void event(controls::list *l, void*);
	void event(controls::keyboard *k, void*);
};




//========================================================================
//	Создание карты для редактирования
//========================================================================
class create_map
	:public static_modal<create_map>
	,public form
{
private:
	button
		back,
		random,
		create;
	controls::input
		name;
	controls::digital
		maze_width,
		maze_height;
	controls::list
		land;

public:
	create_map();
	void event(button *b, void*);
};




//========================================================================
// Single game
//========================================================================
class single
	:public static_modal<single>
	,public form
{
private:
	button
		back,
		campagin,
		start,
		random;
	controls::digital
		maze_size,
		difficult;
	controls::list
		land;

public:
	single();
	void event(button *b, void*);
};



//========================================================================
//	Adventure
//========================================================================
class adventure
	:public static_modal<adventure>
    ,public form
{
private:
	button
		back,
		restart,
		open;
	text_string
		tlevel,
		level,
		tlives,
		lives,	
		tbombs,
		bombs,
		tcashs,
		cashs;
	controls::list
		groups,
		lmaps;

public:
	adventure();
	void event(button *b, void*);
	void event(controls::list *l, void*);
};



	
//====================================================================
//	Панель управления в редакторе
//====================================================================
class edit_bar
	:public form
	,public static_class<edit_bar>
{
private:
	button
		show_minimap,
		paint_erase,
		paint_ground,
		paint_object,
		undo_all;

	button
		exit;

public:
	edit_bar();
	void event(button *b, void*);
};



//====================================================================
//	Панель "объекты" в редакторе
//====================================================================
class objects_bar
	:public form
	,public static_class<objects_bar>
{
private:
	button
		key,
		closed_door,
		player,
		monster,

		trap,
		pick,
		freeze,

		box,

		lo_cash,
		med_cash,
		hi_cash,
		bomb;

public:
	objects_bar();
	void event(button *b, void*);
};



//====================================================================
//	Панель "территория" в редакторе
//====================================================================
class terrain_bar
	:public form
	,public static_class<terrain_bar>
{
private:
	//-------------------------------------------------------------
	class terrain
		:public button
	{
	public:
		terrain(form *_parent, const string &name, const	animation	&anims);
		void event(redraw *screen, void*);
	};

	//-------------------------------------------------------------
	terrain
		decor_far,
		ladder,
		broken,
		decor_near,
		water,
		rope,
		hard,
		slow,
		fast,
		soft;

public:
	terrain_bar();
	void event(button *b, void*);
};



//========================================================================
//	Привязка кнопок
//========================================================================
class keys_binding
	:public controls::keys_binding
{
public:
	keys_binding();
};


#endif