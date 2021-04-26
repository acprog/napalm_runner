/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include	"resources.h"
#include	"map.h"


//===============================================================
//	местность
//===============================================================
land::land(const string &_name)
	:collection		(_name, "land")
	,cage			(CAGE_SIZE, CAGE_SIZE)
	,ground			(CAGE_SIZE*4, CAGE_SIZE/2*3)
	,short_ground	(CAGE_SIZE*4, CAGE_SIZE)
	,broken_cage(cage)
	,background	(new image("background", 	this))
	,box		("box",			this,	cage)
	,broken		("broken", 		this,	ground)
	,decor_far	("decor_far",	this,	ground)
	,decor_near ("decor_near",	this,	ground)
	,fast		("fast",		this,	ground)
	,hard		("hard",		this,	ground)
	,ladder		("ladder",		this,	ground)
	,objects	("objects",		this,	cage)
	,pile		("pile",		this,	cage)
	,pile_make	("pile_make",	this,	size<>(CAGE_SIZE, CAGE_SIZE*3/2))
	,pile_unmake("pile_unmake",	this,	cage)
	,rope		("rope",		this,	short_ground)
	,slow		("slow",		this,	ground)
	,soft		("soft",		this,	ground)
	,water		("water",		this,	short_ground)
	,stones		("stones",		this,	cage)
	,slivers	("slivers",		this,	cage)
	,door_close	("door/close",	this,	cage)
	,door_open	("door/open",	this)
	,door_active("door/active",	this,	cage)
	,monster	("monster",		this,	cage)
{
	if (application::get().prefs("volume/sound_quality")=="hi")
	{
		open_door			=load_sound("door/open");
		pile_make_s			=load_sound("pile/make");
		pile_unmake_s		=load_sound("pile/unmake");
		start_level			=load_sound("level/start");
		move				=load_sound("creature/move");
		move_on_rope		=load_sound("creature/move_on_rope");
		move_on_mud			=load_sound("creature/move_on_mud");
		move_on_ladder		=load_sound("creature/move_on_ladder");
		sink				=load_sound("creature/sink");
		player_punch		=load_sound("player/push");
		box_explode			=load_sound("box/explode");
		box_sink			=load_sound("box/sink");
		sliver_punch		=load_sound("sliver_punch");
		stone_punch			=load_sound("stone_punch");
	}
	else
		open_door			=
		pile_make_s			=
		pile_unmake_s		=
		start_level			=
		move				=
		move_on_rope		=
		move_on_mud			=
		move_on_ladder		=
		sink				=
		player_punch		=
		box_explode			=
		box_sink			=
		sliver_punch		=
		stone_punch			=
			sound::empty;
	
	activate_door		=load_sound("door/activate");
	complete_level		=load_sound("level/complete");
	player_break_cage	=load_sound("player/dig");
	player_grab_cash	=load_sound("player/grab_cash");
	box_punch			=load_sound("box/move");

	//--------------------------------------------------------
	// масштабируем background если нужно
	if (	background->width<application::get().width
		||	background->height<application::get().height)
	{
		image	*tmp=background;
		background=new image(size<>(*tmp)*2);
		background->fit(*tmp);
		delete tmp;
	}
}




//===============================================================
land::~land()
{
	delete background;
}





//===============================================================
//	тема
//===============================================================
theme::theme(const string &_name)
	:collection	(_name, "theme")
	,cage		(CAGE_SIZE, CAGE_SIZE)
	,bomb		("bomb",		this,	cage)
	,break_mask	("dig_mask",	this,	cage)
	,explosion	("explosion",	this,	cage*5)
	,ice_smoke	("ice_smoke",	this,	size<>(CAGE_SIZE*3, CAGE_SIZE))
	,objects	("objects",		this,	cage)
	,player		("player",		this,	cage)
	,trap		("trap",		this,	cage)
	,player_birth		(NULL)
{
	if (application::get().prefs("volume/sound_quality")=="hi")
	{
		player_exit			=load_sound("player/exit");
		player_birth_wait	=load_sound("player/birth_wait");
		player_grab_bomb	=load_sound("player/grab_bomb");
		player_put_trap		=load_sound("player/put_trap");
		player_make_freeze	=load_sound("player/make_freeze");
		player_suicide		=load_sound("player/suicide");
		player_buy_live		=load_sound("player/buy_live");
		bomb_put			=load_sound("bomb/put");
		bomb_timer			=load_sound("bomb/timer");
		trap_catch			=load_sound("trap_catch");
		froze				=load_sound("froze");
		monster_grab_cash	=load_sound("monster/grab_cash");
		monster_drop_cash	=load_sound("monster/drop_cash");
		press_creature		=load_sound("creature/press");
		explode_creature	=load_sound("creature/explode");
		crash				=load_sound("creature/crash");
		player_cannot_punch_box	=load_sound("player/cannot_push_box");
		flesh_punch			=load_sound("creature/flesh_punch");
	}
	else
		player_exit			=
		player_birth_wait	=
		player_grab_bomb	=
		player_put_trap		=
		player_make_freeze	=
		player_suicide		=
		player_buy_live		=
		bomb_put			=
		bomb_timer			=
		trap_catch			=
		froze				=
		monster_grab_cash	=
		monster_drop_cash	=
		press_creature		=
		explode_creature	=
		crash				=
		player_cannot_punch_box	=
		flesh_punch			=
			sound::empty;
		
	raise				=load_sound("creature/raise");
	player_birth		=load_sound("player/birth");
	player_grab_item	=load_sound("player/grab_item");
	bomb_explosion		=load_sound("bomb/explosion");
	monster_eat			=load_sound("monster/eat");
}





//===============================================================
//	Наборы карт
//===============================================================
maps::maps(const string &_name)
	:collection	(_name, "maps")
	,description("adventure", file_open("adventure.xml"))
{
    prefs("save/lives", "", true).uncrypt('l');
	prefs("save/cashs", "", true).uncrypt('c');
	prefs("save/level", "", true).uncrypt('v');
	prefs("save/bombs", "", true).uncrypt('b');
	if (prefs("save/lives")=="")
		prefs("save/lives")="1";
	if (prefs("save/level")=="")
		prefs("save/level")="1";
	if (prefs("save/cashs")=="")
		prefs("save/cashs")="0";
}


//===============================================================
maps::~maps()
{
	prefs("save/lives").crypt('l');
	prefs("save/cashs").crypt('c');
	prefs("save/level").crypt('v');
	prefs("save/bombs").crypt('b');

	string	s;
	description.write(s);
	file *f=file_open("adventure.xml", false);
	f->write((const char*)s, s.length());
	delete f;
}


//===============================================================
string	&maps::prefs(const string &name, const string &deflt, bool atribute)
{
	return get().description(name, deflt, atribute);
}


//===============================================================
void maps::save_map(map &m, const string &name)
{
	file *x=file_open(name+".map.xml", false);
	if (!x)
		ERROR(common::str("error/save_map"));
	else
	{
		xml	xm("napalm_runner_map");
		m.save(xm);
		xm.write(x);
	}
}


//===============================================================
map *maps::load_map(const string &name)
{
	file *f=file_open(name+".map.xml");
	if (!f)
		return NULL;
	xml	xm("napalm_runner_map", f);
	size<>	s;
	s.width=xm("width").atoi();
	s.height=xm("height").atoi();
	map *m=new map(xm, s);
	return m;
}
