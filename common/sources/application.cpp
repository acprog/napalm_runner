/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "application.h"
#include "platform.h"
#include "timer.h"
#include "sound.h"
#include "controls.h"

namespace mpp
{

//========================================================================
//	Интерфейс для различных приложений
//========================================================================
application::application(const string &_name, int _cycle_frequency, uint32 _version)
	:pen_handler		(&platform::get(), platform::get().screen_size())
	,unused				(NULL)
	,active				(NULL)
	,pen_as_key			(0)
	,preference			("mpp_prefs", platform::get().file_open("mpp_prefs.xml"))
	,name				(_name)
	,cycle_frequency	(_cycle_frequency)
	,version			(_version)
{
	platform::get().rotate_screen(prefs("screen/rotate", "0", true).atoi());

	sound::empty=new sound;
	
	sound::empty->change_volume(prefs("volume/sound", "50", true).atoi());
	loop::volume=prefs("volume/music", "0", true).atoi();
}


//========================================================================
application::~application()
{
	controls::keyboard::erase();
	controls::keys_binding::erase();

	delete active;

	loop::erase();
	delete sound::empty;

	prefs("volume/sound")=string(sound::volume, 10);
	prefs("volume/music")=string(loop::volume, 10);

	string	s;
	preference.write(s);
	file *f=platform::get().file_open("mpp_prefs.xml", false);
	f->write((const char*)s, s.length());
	delete f;

	common::erase();
}


//========================================================================
void application::close()
{
	platform::get().close();
}


//========================================================================
void application::goto_modal(modal *m)
{
	if (unused)
		delete unused;
	unused=active;
	active=m;
}


//========================================================================
void application::free_unused()
{
	if (unused)
	{
		delete unused;
		unused=NULL;
	}
}


//========================================================================
void application::free_active()
{
	if (active)
	{
		delete active;
		active=NULL;
	}
}

//========================================================================
void application::penkey_down(int id)
{
	key_down	kd;
	kd.key_id=pen_as_key=id;
	send(&kd, this, this);
}


//========================================================================
void application::penkey_up()
{
	key_up	ku;
	ku.key_id=pen_as_key;
	send(&ku, this, this);
	pen_as_key=0;
}


//========================================================================
string	&application::prefs(const string &name, const string &default_value, bool atribute)
{
	return get().preference(name, default_value, atribute);
}


//========================================================================
xml	&application::prefs_folder(const string &name)
{
	return get().preference.folder(name);
}


//========================================================================
string	application::get_version() const
{
	return string("1.")+(version>9 ? "" : "0")+string(version, 10)+"."+(MPP_VERSION>9 ? "" : "0")+string(MPP_VERSION, 10);
}

//========================================================================
void application::event(cycle*, void *source)
{
	static	cycle	c={0};
	dispatcher<cycle>::event(&c, source);
	c.n++;
	free_unused();
}

}


