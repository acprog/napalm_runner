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
struct draw_on_minimap;

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
	:public form
	,public modal
	,public static_class<main_menu>
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
	void event(controls::button *b);
};




//========================================================================
//	top10
//========================================================================
class top10
	:public form
	,public modal
	,public static_class<top10>
	,public no_extern<handler<controls::keyboard> >
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
	void event(button *b);
	void event(controls::keyboard *k);
};





	
//========================================================================
//	Game options
//========================================================================
class options
	:public form
	,public modal
	,public static_class<options>
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
	void event(button *b);
	void event(controls::digital *d);

	bool pass_event(redraw *screen);
};



//========================================================================
//	Try again
//========================================================================
class try_again
	:public form
	,public modal
	,public static_class<try_again>
	,public no_extern<handler <timer> >
{
private:
	text_string	message;
	timer		restart;
	player_info pi;

public:
	try_again(player_info pi);
	void event(timer *t);
	void pen_down(const point<> &p);
	void pen_up(const point<> &p);
};




//========================================================================
//	Level complete
//========================================================================
class level_complete
	:public form
	,public modal
	,public static_class<level_complete>
{
private:
	picture		banner;
	text_string	info;
	player_info	pi;

public:
	level_complete(player_info pi);
	void event(key_down *k);
	void pen_up(const point<> &p);
};




//========================================================================
//	game over
//========================================================================
class game_over
	:public form
	,public modal
	,public static_class<game_over>
{
private:
	picture		banner;
	text_string	info;
	int			score;

public:
	game_over(int score);
	void event(key_down *k);
	void pen_up(const point<> &p);
};






//========================================================================
//	Make Random map
//========================================================================
class random_map
	:public form
	,public modal
	,public static_class<random_map>
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
	void event(button *b);
	void event(controls::digital *d);
};


//====================================================================
//	Миникарта
//====================================================================
class minimap
	:public form
	,public static_class<minimap>
	,public no_extern<send_down<draw_on_minimap> >
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
	void event(redraw *screen);
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
	,public static_extern< auto_free, game >
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
		,public static_extern<auto_free, game_bar>
	{
	private:
		const	item_type	*i;

	public:
		item(form *parent, const item_type *_item);
		void event(redraw *screen);
	};


	//=====================================================================================
public:

	game_bar();
	void event(redraw *screen);
	void event(button *b);
	void set(const int *_lives, const int *_bombs, const int *_cash, const item_type *_item);
};




//====================================================================
//	Меню в игре
//====================================================================
class game_menu
	:public form
	,public static_class<game_menu>
	,public static_extern< auto_free, game >
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
	void event(button *b);
	void event(controls::digital *d);
};







//========================================================================
//	Maps manager
//========================================================================
class maps_manager
	:public form
	,public modal
	,public static_class<maps_manager>
	,public no_extern<handler<controls::keyboard> >
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
	void event(button *b);
	void event(controls::list *l);
	void event(controls::keyboard *k);
};




//========================================================================
//	Создание карты для редактирования
//========================================================================
class create_map
	:public form
	,public modal
	,public static_class<create_map>
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
	void event(button *b);
};




//========================================================================
// Single game
//========================================================================
class single
	:public form
	,public modal
	,public static_class<single>
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
	void event(button *b);
};



//========================================================================
//	Adventure
//========================================================================
class adventure
	:public form
	,public modal
	,public static_class<adventure>
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
	void event(button *b);
	void event(controls::list *l);
};



	
//====================================================================
//	Панель управления в редакторе
//====================================================================
class edit_bar
	:public form
	,public static_class<edit_bar>
	,public static_extern< auto_free, editor >
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
	void event(button *b);
};



//====================================================================
//	Панель "объекты" в редакторе
//====================================================================
class objects_bar
	:public form
	,public static_class<objects_bar>
	,public static_extern< auto_free, editor >
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
	void event(button *b);
};



//====================================================================
//	Панель "территория" в редакторе
//====================================================================
class terrain_bar
	:public form
	,public static_class<terrain_bar>
	,public static_extern< auto_free, editor >
{
private:
	//-------------------------------------------------------------
	class terrain
		:public button
	{
	public:
		terrain(form *_parent, const string &name, const	animation	&anims);
		void event(redraw *screen);
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
	void event(button *b);
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