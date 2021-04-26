/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "platform.h"
#include "application.h"
#include "timer.h"
#include "mem.h"

namespace mpp
{

const		int timer::infinite=-1;

//========================================================================
//	Интерфейс для различных платформ
//========================================================================
platform::platform()
	:screen_angle	(0)
	,screen_zoom	(1)
	,screen			(NULL)
	,pen_handler	(NULL, rect<>(0, 0, 0, 0))	// размер от балды, первый проверяемый размер в application
	,rotate_buffer	(NULL)
{
	pd.target=NULL;
}

//========================================================================
platform::~platform()
{
	application::erase();

	if (screen)
		delete screen;
	if (rotate_buffer)
		large_free(rotate_buffer);
}


//========================================================================
void platform::rotate_screen(int angle)
{
	if (rotate_buffer && angle==0)
	{
		large_free(rotate_buffer);
		rotate_buffer=NULL;
	}
	else if (!rotate_buffer && angle!=0)
		rotate_buffer=large_mem::allocate<COLOR>(screen->area());
		
	if (screen_angle!=angle && abs(screen_angle-angle)!=180)	// изменились пропорции
		swap(screen->width, screen->height);
	
	application::get().width=screen->width;
	application::get().height=screen->height;
	
	screen_angle=angle;
}



//========================================================================
void platform::rotate_screen(image *screen)
{
	COLOR	*src=rotate_buffer,
			*dst=screen->get_bits();
	large_mem::write((uint32*)src, (uint32*)src, (uint32*)dst, (screen->area()*sizeof(COLOR))/4);
	
	int step,
		x, y,
		i,
		w=screen->width,
		h=screen->height;

	switch (screen_angle)
	{
	case 90:
		step=w*h;
		dst+=step;
		++step;
		for (y=h; --y>=0; dst+=step)
			for (x=w; --x>=0;)
				*(dst-=h)=*(src++);
		break;

	case 180:
		dst+=w*h;
		for (i=w*h; --i>=0;)
			*(--dst)=*(src++);
		break;

	case 270:
		step=w*h+1;
		--dst;
		for (y=h; --y>=0; dst-=step)
			for (x=w; --x>=0;)
				*(dst+=h)=*(src++);
		break;
	}
}


//========================================================================
void platform::pen_event(point<> pos, pen_event_type type)
{
	//-----------------------------------------------------------
	// разворачиваем координаты
	switch (screen_angle)
	{
	case 90:
		swap(pos.x, pos.y);
		pos.x=screen->width-pos.x;
		break;

	case 180:
		pos.x=screen->width-pos.x;
		pos.y=screen->height-pos.y;
		break;

	case 270:
		swap(pos.x, pos.y);
		pos.y=screen->height-pos.y;
		break;
	}

	pos=screen->force_inside(pos);

	//--------------------------------------------------------------
	// отсылаем сообщение
	switch (type)
	{
	case pen_down_event:
		pd.pos=pos;
		pd.target=NULL;	// минимум целью будет aplication
		send(&pd);
		pd.target->pen_down(pos);
		break;

	case pen_up_event:
		if (pd.target)
			pd.target->pen_up(pos);
		break;

	case pen_move_event:
		if (pd.target)
			pd.target->pen_move(pos);
		break;
	}
}



//========================================================================
COLOR *platform::redraw(bool erase, COLOR *new_buffer)
{
	if (new_buffer)
	{
		image	*new_screen=new image(*screen, new_buffer);
		delete screen;
		screen=new_screen;
	}

	if (erase)
		screen->fill(0);

	struct	redraw	rd;
	rd.ptr=screen;
	send(&rd);
	
	if (screen_angle)
		rotate_screen(screen);

	return screen->get_bits();
}


//========================================================================
int	platform::launch(platform *p)
{
	if (platform::get().exist()==false)
		return -1;

	if (!platform::get().init_screen())
	{
		platform::erase();
		return -1;
	}

	application::create();

	if (application::get().exist()==false)
		return -1;

	int	result=p->loop();

	application::erase();
	platform::erase();

//	LOG("normal quit");

	return result;
}


//=============================================================================
bool platform::init_screen()
{
	image	*img=create_screen();
	if (img)
	{
		screen=img;
		return true;
	}
	return false;
}


//=============================================================================
string platform::get_copy()
{
	// формируем номер исходя из текстовой информации 2-х строк
	string			pl=get_id(),
					ap=application::get().name;
	unsigned int	summ=0,
					mult=0;
	int				i;
	const char		*ptr;

	for (ptr=pl; *ptr; ptr++, summ+=mult)
		for (mult=1, i=ap.length(); --i>=0;)
			mult*=(uint32)*ptr;

	for (ptr=ap; *ptr; ptr++, summ+=mult)
		for (mult=1, i=pl.length(); --i>=0;)
			mult*=(uint32)*ptr;

	summ%=1000000000;
	string	s(summ, 10);
	while (s.length()<9)
		s+='3';
	s[4]='-';
	return s;
}

}/*namespace mpp*/