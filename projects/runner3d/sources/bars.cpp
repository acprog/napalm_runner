#include "bars.h"
#include "runner.h"
#include "game.h"
#include "map.h"
#include "editor.h"
#include "resources.h"
#include "events.h"



//========================================================================
//	main menu
//========================================================================
main_menu::main_menu()
	:form			("main_menu")
	,new_game		(this, "single")
	,options		(this, "options")
	,editor			(this, "editor")
	,registration	(this, "registration")
	,hi_scores		(this, "hi_scores")
	,exit			(this, "exit")
	,napalm_games	(this, "napalm_games")
	,ver			(this, "version")
	,logo			(this, "logo")
{
	napalm_games=string("(c) Napalm Games.  ")+__DATE__;
	ver="v"+runner::get().get_version();
}
		
//========================================================================
void main_menu::event(button *b, void*)
{
	if (b==&new_game)
		new single;
	else if (b==&options)
		new class options;
	else if (b==&editor)
		new maps_manager;
	else if (b==&registration)
		new class registration;
	else if (b==&hi_scores)
		new top10;
	else if (b==&exit)
		runner::get().close();
}





//========================================================================
//	Top10
//========================================================================
top10::top10(int score)
	:form			("top10")
	,back			(this, "back")
	,start			(this, "start")
	,scores			(this, "scores")
	,new_score		(score)
{
	for (int i=0; i<10; i++)
		scores.push_sortdown(runner::prefs("top10/n"+string(i, 10), (string("00000")+string(i, 10)+"   -").crypt('s'), true).uncrypt('s'));
	if (score!=0)
	{
		if (score>=runner::prefs("top10/n9").atoi())
		{
			scores.remove(runner::prefs("top10/n9"));
			new controls::keyboard(runner::prefs("top10/prev_name", common::str("top10/player"), true), this, "top10");
		}
		else
			push_string(common::str("top10/your_result"));
	}
}




//========================================================================
top10::~top10()
{
	int i=0;
	while (string *s=scores.each_ptr())
		runner::prefs("top10/n"+string(i++, 10))=s->crypt('s');
}




//========================================================================
void top10::event(button *b, void*)
{
	if (b==&back)
		new main_menu;
	else if (b==&start)
		new single;
}


//========================================================================
void top10::event(controls::keyboard *k, void*)
{
	runner::prefs("top10/prev_name")=*k;
	push_string(*k);
}


//========================================================================
void top10::push_string(const string &name)
{
	string	s(new_score, 10);
	while (s.length()<6)
		s="0"+s;
	s+="   "+name;
	scores.push_sortdown(s);
	scores.select(s);
}



//========================================================================
//	Try again
//========================================================================
try_again::try_again(player_info _pi)
	:form		("try_again")
	,restart	(this, this, 1)
	,pi			(_pi)
	,message	(this, "message")
{
}


//========================================================================
void try_again::event(timer *t, void*)
{
	if (t==&restart)
		runner::get().restart_level(pi);
}



//========================================================================
void try_again::pen_down(const point<> &p)
{
	restart.stop();
}


//========================================================================
void try_again::pen_up(const point<> &p)
{
	runner::get().restart_level(pi);
}





//========================================================================
//	Level complete
//========================================================================
level_complete::level_complete(player_info _pi)
	:form	("level_complete")
	,banner	(this, "level_complete", new image("level_complete", &theme::get()))
	,info	(this, "info")
	,pi		(_pi)
{
	new loop(theme::get(), "loops/level complete");
}


//========================================================================
void level_complete::event(key_down *k, void*)
{
	runner::get().next_level(pi);
}


//========================================================================
void level_complete::pen_up(const point<> &p)
{
	runner::get().next_level(pi);
}





//========================================================================
//	game over
//========================================================================
game_over::game_over(int _score)
	:form		("game_over")
	,banner		(this, "game_over", new image("game_over", &theme::get()))
    ,info		(this, "info")
	,score		(_score)
{
	new loop(theme::get(), "loops/game over");
}


//========================================================================
void game_over::event(key_down *k, void*)
{
	new loop(theme::get(), "loops/menu");
	if (maps::exist())
		new adventure();
	else if (score)
		new top10(score);		
	else
		new single;
}


//========================================================================
void game_over::pen_up(const point<> &p)
{
	new loop(theme::get(), "loops/menu");
	if (maps::exist())
		new adventure();
	else if (score)
		new top10(score);		
	else
		new single;
}





//========================================================================
//	Game options
//========================================================================
options::options()
	:form			("options")
	,music_volume	(this, "music", 0, 100)
	,sound_volume	(this, "sound", 0, 100)
	,keys			(this, "keys")
	,exit			(this, "back")
	,interfaces		(this, "interfaces")
	,thems			(this, "thems")
	,screen_angle	(this, "rotate_screen")
	,sound_quality	(this, "sound_quality")
	,music_disabled	(loop::get().volume<=5)
{
	sound_quality.push_back(common::str("sound/low"));
	sound_quality.push_back(common::str("sound/hi"));
	
	if (application::get().prefs("volume/sound_quality")=="hi")
		sound_quality.select(common::str("sound/hi"));

	screen_angle.push_back(common::str("rotate/no"));
	screen_angle.push_back(common::str("rotate/left"));
	screen_angle.push_back(common::str("rotate/up_to_down"));
	screen_angle.push_back(common::str("rotate/right"));

	string	*s=NULL;
	int		a=application::get().prefs("screen/rotate").atoi();
	for (int angle=0; s=screen_angle.each_ptr(); angle+=90)
		if (angle==a)
		{
            screen_angle.break_each();
			break;
		}
	if (s)
		screen_angle.select(*s);

	collection::get_names("theme", thems);
	collection::get_names("interface", interfaces);

	thems.select(application::get().prefs("theme"));
	interfaces.select(application::get().prefs("interface"));

	music_volume=loop::get().volume;
	sound_volume=sound::volume;
}


//========================================================================
options::~options()
{
	application::get().prefs("theme")=thems.get_selected();
	application::get().prefs("interface")=interfaces.get_selected();

	if (music_disabled && loop::get().volume>5)
		new loop(theme::get(), "loops/menu");
}


//========================================================================
void options::event(button *b, void*)
{
	if (b==&exit)
	{
		string	&sq=application::get().prefs("volume/sound_quality");
		bool	hi=sound_quality.get_selected()==common::str("sound/hi");
		if (hi && sq!="hi")
		{
			land::erase();
			theme::erase();
		}
		sq=hi ? "hi" : "low";
		
		if (string(thems.get_selected()).lo_case()!=string(application::get().prefs("theme")).lo_case())
			theme::erase();

		if (!theme::exist())
		{
			new theme(thems.get_selected());
			music_disabled=true;
		}

		// поворачиваем экран
		string s=screen_angle.get_selected();
		string *sa=NULL;
		for (int angle=0; sa=screen_angle.each_ptr(); angle+=90)
			if (s==*sa)
			{
				platform::get().rotate_screen(angle);
				application::get().prefs("screen/rotate")=string(angle, 10);
			}
		
		if (interfaces.get_selected()!=application::get().prefs("interface"))
		{
			string	iname=interfaces.get_selected();
			application::get().free_active();
			new common(iname, "interface");
			new main_menu;
		}
		else
		{
			new main_menu;
			application::get().free_unused();
		}
	}
	else if (b==&keys)
		new keys_binding;
}



//========================================================================
void options::event(controls::digital *d, void*)
{
	if (d==&sound_volume)
	{
		sound::empty->change_volume(sound_volume);
		loop::change_volume(music_volume);	// для wince
	}
	else if (d==&music_volume)
		loop::change_volume(music_volume);
}



//========================================================================
bool options::pass_event(redraw *screen)
{
	return !keys_binding::exist();
}




//========================================================================
//	Make Random map
//========================================================================
random_map::random_map(const size<>	&_s, const string &_name, const string &_land)
	:form			("generate_maze")
	,s				(_s)
	,name			(_name)
	,land			(_land)
	,maze_density	(this, "maze_density", 10, 80)
	,maze_space		(this, "maze_space", 50, 90)
	,ladders_size	(this, "ladders_size", 5, 30)
	,soft			(this, "soft", 0, 100)
	,hard			(this, "hard", 0, 100)
	,ice			(this, "fast", 0, 100)
	,mud			(this, "slow", 0, 100)
	,rope			(this, "rope", 0, 100)
	,cashs			(this, "cash", 0, 100)
	,monsters		(this, "monsters", 0, 10)
	,items			(this, "items", 0, 10)
	,bombs			(this, "bombs", 0, 10)
	,boxes			(this, "boxes", 0, 100)
	,back			(this, "back")
	,random			(this, "random")
	,start			(this, "create")
{
}


//========================================================================
void random_map::event(button *b, void*)
{
	if (b==&start)
		new editor(
			map::generate(
				land,
				s,
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
				boxes
			), name);
	else if (b==&back)
		new create_map;
	else if (b==&random)
	{
		maze_density.random();
		maze_space.random();
		ladders_size.random();
		soft.random();
		hard.random();
		ice.random();
		mud.random();
		rope.random();
		cashs.random();
		monsters.random();
		items.random();
		bombs.random();
		boxes.random();
		event((controls::digital*)&soft, this);
	}
}


//========================================================================
void random_map::event(controls::digital *d, void*)
{
	if (d==&soft
	||	d==&hard
	||	d==&ice
	||	d==&mud
	||	d==&rope
	)
	{
		// а если одно значение - ноль?
		int summ;
		while (abs(100-(summ=(int)soft+hard+ice+mud+rope))>3)
		{
			int	add=(100-summ)/4;
			if (d!=&soft)
				soft=add+soft;
			if (d!=&hard)
				hard=add+hard;
			if (d!=&ice)
				ice=add+ice;
			if (d!=&mud)
				mud=add+mud;
			if (d!=&rope)
				rope=add+rope;
		}
	}
}



//====================================================================
//	Миникарта
//====================================================================
minimap::minimap(const size<> &mm_size, const map &_cages, const point<> &_linkview)
	:form		("minimap")
	,img		(mm_size)
	,viewport	(platform::get().screen_size()/CAGE_SIZE*2)
	,cages		(_cages)
	,linkview	(_linkview)
{
	//------------------------------------------------
	// грузим цвета
	xml	desc("land_prefs", land::get().file_open("prefs.xml"));
	window		=color32(desc("minimap/colors/window").atoi(16));
	impassable	=color32(desc("minimap/colors/impassable").atoi(16));
	ladder		=color32(desc("minimap/colors/ladder").atoi(16));
	water		=color32(desc("minimap/colors/water").atoi(16));
	cash		=color32(desc("minimap/colors/cash").atoi(16));

	if (viewport.width>mm_size.width)
		viewport.width=mm_size.width;
	if (viewport.height>mm_size.height)
		viewport.height=mm_size.height;

	//--------------------------------------------------
	set_size(mm_size+10);
	refresh();
}



//====================================================================
void minimap::event(redraw *screen, void *source)
{
	form::event(screen, source);
	screen->ptr->put_transparent(img, point<>(x+5, y+5));

	viewport.x=x+5+(linkview.x>0? 0: -linkview.x)*2/CAGE_SIZE;
	viewport.y=y+5+(linkview.y>0? 0: -linkview.y)*2/CAGE_SIZE;

	draw_on_minimap	d;
	d.p=this;
	d.screen=screen->ptr;
	send(&d, this, this);

	screen->ptr->draw_rect(viewport, window);
}


//====================================================================
void minimap::draw_point(image &screen, const point<> &pos, COLOR c)
{
	point<>	p=pos*2;
	p+=5;
	p+=*this;
	screen[p]=c;
	++p.x;
	screen[p]=c;
	++p.y;
	screen[p]=c;
	--p.x;
	screen[p]=c;
}



//====================================================================
void minimap::erase_point(const point<> &pos)
{
	point<>	p=pos*2;
	img[p]=0;
	++p.x;
	img[p]=0;
	++p.y;
	img[p]=0;
	--p.x;
	img[p]=0;
}



//===============================================================
void minimap::refresh()
{
	COLOR *bits=img.get_bits();
	const cage	*c=&cages.read(point<>(0, 0));
	int		h, w;
	for (h=cages.count().height; --h>=0; bits+=cages.count().width*2)
		for (w=cages.count().width; --w>=0; bits+=2, c++)
			*(bits+cages.count().width*2+1)=*(bits+cages.count().width*2)=*(bits+1)=*(bits)=get_minimap_color(*c);
}


//===============================================================
COLOR minimap::get_minimap_color(cage c) const
{
	if (c==cage::hard || c==cage::soft || c==cage::mirage || c==cage::fast || c==cage::slow)
		return impassable;
	if (c==cage::water)
		return water;
	if (c==cage::ladder || c==cage::rope)
		return ladder;
	if ((item_type)c>=lo_cash && (item_type)c<=hi_cash)
		return cash;
	return COLOR(0, 0, 0);
}




//====================================================================
//	Панель управления в игре
//====================================================================
game_bar::game_bar()
	:form	("game_bar")
	,kill	(this, "lives")
	,bomb	(this, "bombs", runner::key().put_bomb,	theme::get().objects, ibomb)
	,cash	(this, "cash")
	,pause	(this, "menu")
{
	bomb.y-=7;
}


//====================================================================
void game_bar::event(button *b, void*)
{
	if (b==&kill)
		game::get().restart();
	else if (b==&pause)
		application::get().penkey_down(runner::key().pause);
}



//====================================================================
void game_bar::event(redraw *screen, void *source)
{
	form::event(screen, source);

	font	&fnt=*common::get().font;
	int		py=y+(32-fnt.get_text_metric("a").height)/2;

	int prev_style=fnt.set_style(5);
	fnt.print(*screen->ptr, point<>(kill.right(), py), string("X")+string(*n_lives, 10));
	fnt.print(*screen->ptr, point<>(bomb.right(), py), string("X")+string(*n_bombs, 10));
	fnt.print(*screen->ptr, point<>(cash.right(), py), string(*n_cash, 10)+"$");
	fnt.set_style(prev_style);
}



//====================================================================
void game_bar::set(const int *_lives, const int *_bombs, const int *_cash, const item_type *_item)
{
	n_lives=_lives;
	n_bombs=_bombs;
	n_cash=_cash;
	items.push(new item(this, _item));
}


//====================================================================
game_bar::item::item(form *parent, const item_type *_item)
	:virtual_key	(parent, "item", runner::key().item)
	,i				(_item)
{
}


//====================================================================
void game_bar::item::event(redraw *screen, void*)
{
	if (*i!=no_item)
		screen->ptr->put_transparent(theme::get().objects.get_frame(0, *i), *this);
}





//====================================================================
//	Меню в игре
//====================================================================
game_menu::game_menu()
	:form			("game_menu")
	,music_volume	(this, "music", 0, 100)
	,sound_volume	(this, "sound", 0, 100)
	,exit			(this, "exit")
	,restart		(this, "restart")
	,controls		(this, "controls")
	,resume			(this, "resume")
	,no_music		(this, "no_music")
{
	if (loop::get().volume<=LOOP_OFF)
		music_volume.show(false);
	else
		no_music.show(false);

	if (!runner::key().left || !runner::key().right || !runner::key().up || !runner::key().down)
		new keys_binding;
	music_volume=loop::get().volume;
	sound_volume=sound::volume;
}


//====================================================================
void game_menu::event(button *b, void*)
{
	if (b==&exit)
	{
		new loop(theme::get(), "loops/menu");
		if (maps::exist())
			new adventure;
		else if (game::get().get_score()==0)
			new single;
		else
			new top10(game::get().get_score());
	}
	else if (b==&restart)
	{
		game::get().restart();
		application::get().penkey_up();
	}
	else if (b==&resume)
		application::get().penkey_down(runner::key().pause);
	else if (b==&controls)
		new keys_binding;
}



//========================================================================
void game_menu::event(controls::digital *d, void*)
{
	if (d==&sound_volume)
	{
		sound::empty->change_volume(sound_volume);
		loop::change_volume(music_volume);
	}
	else if (d==&music_volume)
	{
		int	volume=music_volume;
		if (volume<=LOOP_OFF &&	loop::get().volume>LOOP_OFF)
			volume=LOOP_OFF+1;
		loop::change_volume(volume);
	}
}





//========================================================================
//	Maps manager
//========================================================================
maps_manager::maps_manager()
	:form			("maps_manager")
	,create_group	(this, "groups/create")
	,remove_group	(this, "groups/remove")
	,back			(this, "back")
	,create			(this, "maps/create")
	,remove			(this, "maps/remove")
	,open			(this, "maps/open")
	,groups			(this, "groups/list")
	,lmaps			(this, "maps/list")
{
	platform::get().find_files("maps", groups, true);
	collection::get_names("maps", groups);
	event(&groups, this);
}


//========================================================================
void maps_manager::event(button *b, void*)
{
	if (b==&back)
		new main_menu;
	else if (b==&remove_group)
	{
		if (groups.get_selected())
			if (platform::get().confirm(common::str("question/erase_pack")+groups.get_selected()+"\'?"))
			{
				maps::erase();
				if (platform::get().remove_file("maps/"+groups.get_selected(), true)
				||	platform::get().remove_file("maps/"+groups.get_selected()+".zip"))
					groups.remove_selected();
				event(&groups, this);
			}
	}
	else if (b==&create_group)
		new controls::keyboard("new group", this, "new_group");
	else if (b==&remove)
	{
		if (lmaps.get_selected())
			if (platform::get().confirm(common::str("question/erase_map")+lmaps.get_selected()+"\'?"))
				if (platform::get().remove_file("maps/"+groups.get_selected()+"/"+lmaps.get_selected()+".map.xml"))
					lmaps.remove_selected();
	}
	else if (b==&create)
	{
		if (groups.is_empty())
			platform::get().message(common::str("error/need_group"));
		else
			new create_map;
	}
	else if (b==&open)
	{
		platform::get().mkdir("maps/"+groups.get_selected());	// если файл в архиве
		new maps(groups.get_selected());
		new editor(maps::get().load_map(lmaps.get_selected()), lmaps.get_selected());
	}
}


//========================================================================
void maps_manager::event(controls::keyboard *k, void*)
{
	if (platform::get().mkdir("maps/"+*k))
	{
		groups.push_sortup(*k);
		event(&groups, this);
	}
}


//========================================================================
void maps_manager::event(controls::list *l, void*)
{
	if (l==&groups)
	{
		new maps(groups.get_selected());
		maps::get().find_files(".map.xml", lmaps);
	}
}





//========================================================================
//	Создание карты для редактирования
//========================================================================
create_map::create_map()
	:form		("create_map")
	,name		(this, "name")
	,maze_width	(this, "maze_width", 10, 90)
	,maze_height(this, "maze_height", 10, 90)
	,back		(this, "back")
	,random		(this, "random")
	,create		(this, "create")
	,land		(this, "lands")
{
	collection::get_names("land", land);
}


//========================================================================
void create_map::event(button *b, void*)
{
	if (b==&back)
		new maps_manager;
	else if (b==&random)
		new random_map(size<>(maze_width, maze_height), name, land.get_selected());
	else if (b==&create)
	{
		if (!string(name))
			platform::get().message(common::str("error/name"));
		else
		{
			new editor(
				new map(land.get_selected(), size<>(maze_width, maze_height)),
				name);
		}
	}
}




//========================================================================
//	Single game
//========================================================================
single::single()
	:form		("single")
	,maze_size	(this, "maze_size", 10, 50)
	,difficult	(this, "difficult", 0, 100)
	,back		(this, "back")
	,campagin	(this, "adventure")
	,start		(this, "start")
	,random		(this, "random")
	,land		(this, "lands")
{
	collection::get_names("land", land);
}


//========================================================================
void single::event(button *b, void*)
{
	if (b==&start)
	{
		maps::erase();	// без набора карт
		player_info pi;
		pi.bombs=0;
		pi.cash=0;
		pi.score=0;
		pi.lives=(100-difficult)/10+1;
		new game(
			map::generate(
				land.get_selected(),
				maze_size,
				difficult,
				1
				)
			, pi);
	}
	else if (b==&back)
		new main_menu;
	else if (b==&campagin)
		new adventure;
	else if (b==&random)
	{
		maze_size.random();
		difficult.random();
		land.random();
	}
}




//========================================================================
//	adventure
//========================================================================
adventure::adventure()
	:form		("adventure")
	,back		(this, "back")
	,open		(this, "open")
	,restart	(this, "restart")
	,groups		(this, "groups")
	,tlevel		(this, "level/info")
	,level		(this, "level")
	,tlives		(this, "lives/info")
	,lives		(this, "lives")
	,tbombs		(this, "bombs/info")
	,bombs		(this, "bombs")
	,tcashs		(this, "cashs/info")
	,cashs		(this, "cashs")
	,lmaps		(this, "maps")
{
	collection::get_names("maps", groups);
	event(&groups, this);
}


//========================================================================
void adventure::event(button *b, void*)
{
	if (b==&back)
		new single;
	else if (b==&restart)
	{
		maps::prefs("save/bombs")="0";
		maps::prefs("save/cashs")="0";
		maps::prefs("save/lives")="3";
		maps::prefs("save/level")="1";
		event(&open, this);
	}
	else if (b==&open)
	{
		string	theme_name=maps::prefs("theme");
		if (theme_name!="" && theme_name!=theme::get().get_name())
			new theme(theme_name);

		string	name=maps::prefs("level"+maps::prefs("save/level")+"/name");

		platform::get().mkdir("maps/"+groups.get_selected());	// если файл в архиве
		player_info pi;
		if (!name)
		{
			name=lmaps.get_selected();
			if (!name)
				return;
			pi.bombs=0;
			pi.cash=0;
			pi.lives=3;
			pi.score=0;
		}
		else
		{
			pi.bombs=maps::prefs("save/bombs").atoi();
			pi.cash=maps::prefs("save/cashs").atoi();
			pi.lives=maps::prefs("save/lives").atoi();
		}
		new game(maps::get().load_map(name), pi);
	}
}


//========================================================================
void adventure::event(controls::list *l, void*)
{
	if (l==&groups)
	{
		new maps(groups.get_selected());

		string	name=maps::prefs("level"+maps::prefs("save/level")+"/name");

		lmaps.show(!name);
		tlevel.show(name);
		level.show(name);
		tlives.show(name);
		lives.show(name);
		tbombs.show(name);
		bombs.show(name);
		tcashs.show(name);
		cashs.show(name);
		if (!name)
			maps::get().find_files(".map.xml", lmaps);
		else
		{
			level=name+" #"+maps::prefs("save/level", "1");
			lives=maps::prefs("save/lives");
			bombs=maps::prefs("save/bombs");
			cashs=maps::prefs("save/cashs")+"$";
		}
	}
}





//====================================================================
//	Панель управления в редакторе
//====================================================================
edit_bar::edit_bar()
	:form			("edit_bar")
	,show_minimap	(this, "show_minimap")
	,paint_erase	(this, "paint_erase")
	,paint_ground	(this, "paint_ground")
	,paint_object	(this, "paint_objects")
	,undo_all		(this, "undo_all")
	,exit			(this, "exit")
{
}



//====================================================================
void edit_bar::event(button *b, void*)
{
	if (b==&exit)
		editor::get().close();
	else if (b==&undo_all)
	{
		if (platform::get().confirm(common::str("question/undo_all")))
			editor::get().undo_all();
	}
	else if (b==&show_minimap)
	{
		if (minimap::exist())
			minimap::erase();
		else
			editor::get().show_minimap(); 
	}
	else if (b==&paint_erase)
		editor::get().set_brush(cage::empty);
	else if (b==&paint_ground)
	{
		if (terrain_bar::exist())
			terrain_bar::erase();
		else
			new terrain_bar;
	}
	else if (b==&paint_object)
	{
		if (objects_bar::exist())
			objects_bar::erase();
		else
			new objects_bar;
	}
}






//====================================================================
//	Панель "объекты" в редакторе
//====================================================================
objects_bar::objects_bar()
	:form		("objects_bar")
	,key		(this, "key", theme::get().objects, 0)
	,trap		(this, "trap", theme::get().objects, 1)
	,pick		(this, "pick", theme::get().objects, 2)
	,freeze		(this, "freeze", theme::get().objects, 3)
	,box		(this, "box", land::get().box, 0)
	,bomb		(this, "bomb", theme::get().objects, 5)
	,lo_cash	(this, "lo_cash", land::get().objects, 0)
	,med_cash	(this, "med_cash", land::get().objects, 1)
	,hi_cash	(this, "hi_cash", land::get().objects, 2)
	,player		(this, "player", theme::get().player)
	,monster	(this, "monster", land::get().monster)
	,closed_door(this, "door", land::get().door_close)
{
}



//====================================================================
void objects_bar::event(button *b, void*)
{
	item_type	i;
	if (b==&key)
		i=::key;
	else if (b==&trap)
		i=::itrap;
	else if(b==&pick)
		i=::pick;
	else if (b==&freeze)
		i=::freeze;
	else if (b==&box)
		i=::ibox;
	else if (b==&bomb)
		i=::ibomb;
	else if (b==&closed_door)
		i=::closed_door;
	else if (b==&lo_cash)
		i=::lo_cash;
	else if (b==&med_cash)
		i=::med_cash;
	else if (b==&hi_cash)
		i=::hi_cash;
	else if (b==&player)
		i=::iplayer;
	else if (b==&monster)
		i=::imonster;
 
	editor::get().set_brush(i);
}





//====================================================================
//	Панель "территория" в редакторе
//====================================================================
terrain_bar::terrain_bar()
	:form		("terrain_bar")
	,decor_far	(this, "decor_far", land::get().decor_far)
	,ladder		(this, "ladder", land::get().ladder)
	,broken		(this, "mirage", land::get().broken)
	,decor_near	(this, "decor_near", land::get().decor_near)
	,water		(this, "water", land::get().water)
	,rope		(this, "rope", land::get().rope)
	,hard		(this, "hard", land::get().hard)
	,slow		(this, "slow", land::get().slow)
	,fast		(this, "fast", land::get().fast)
	,soft		(this, "soft", land::get().soft)
{
}



//====================================================================
void terrain_bar::event(button *b, void*)
{
	cage	c;

	if (b==&decor_far)
		c=cage::decor_far;
	else if (b==&ladder)
		c=cage::ladder;
	else if (b==&broken)
		c=cage::broken;
	else if (b==&decor_near)
		c=cage::decor_near;
	else if (b==&water)
		c=cage::water;
	else if (b==&rope)
		c=cage::rope;
	else if (b==&hard)
		c=cage::hard;
	else if (b==&slow)
		c=cage::slow;
	else if (b==&fast)
		c=cage::fast;
	else if (b==&soft)
		c=cage::soft;

	editor::get().set_brush(c);
}


//====================================================================
terrain_bar::terrain::terrain(form *ext, const string &name, const	animation	&_anims)
	:button(ext, name, _anims)
{
}


//====================================================================
void terrain_bar::terrain::event(redraw *screen, void*)
{
	image	img(anims, rect<>(CAGE_SIZE*2, 0, CAGE_SIZE, CAGE_SIZE));
	screen->ptr->put_transparent(img, *this);
}








//========================================================================
//	Привязка кнопок
//========================================================================
keys_binding::keys_binding()
{
	add_key("left",			&runner::key().left);
	add_key("right",		&runner::key().right);
	add_key("up",			&runner::key().up);
	add_key("down",			&runner::key().down);
	add_key("dig_left",		&runner::key().break_left);
	add_key("dig_right",	&runner::key().break_right);
	add_key("put_bomb",		&runner::key().put_bomb);
	add_key("item",			&runner::key().item);
	add_key("pause",		&runner::key().pause);
}
