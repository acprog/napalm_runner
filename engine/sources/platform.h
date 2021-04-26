/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_PLATFORM_H_
#define _MPP_PLATFORM_H_

#include "fixed.h"
#include "geometric.h"
#include "mpp_prefs.h"
#include "string.h"
#include "image.h"
#include "static_class.h"
#include "application.h"
#include "file.h"
#include "mpp_events.h"
#include "list.h"

namespace mpp
{
	class application;
	class platform;
	class sound;

	//========================================================================
	//	Интерфейс для различных платформ
	// а вообще идея такая - наследники platform не должны напрямую обращаться
	// к application
	//========================================================================
	class platform
		:public static_class<platform>
		,public pen_handler
	{
	private:
		REAL		screen_zoom;	// во ск. крат изображение приближено
		image		*screen;
		COLOR		*rotate_buffer;
		pen			pd;

	protected:
		int			screen_angle;	// угол поворота экрана (град)

		enum pen_event_type
		{
			pen_down_event,
			pen_up_event,
			pen_move_event
		};

		virtual void rotate_screen(image *screen);
		void pen_event(point<> pos, pen_event_type type);

		//=======================================================
		// передача сообщения
		template<class T>
		inline void send(T *ev)
		{
			handler<T> &dest=application::get();
			dest.event(ev, this);
		}

	public:
		platform();
		virtual ~platform();
		
		static	int	launch(platform *p);
		virtual	image *create_screen()=0;
		bool init_screen();

		inline	const size<> &screen_size() const
		{
			return *screen;
		}

		virtual void rotate_screen(int angle);	// команда на разворот экрана

		COLOR *redraw(bool erase=false, COLOR *new_buffer=NULL);

		virtual void close()=0;
		virtual	int  loop()=0;

		virtual string key_name(int key)=0;

		virtual bool find_files(const string &path, list<string> &names, bool dirs) const =0;
		virtual bool remove_file(const string &path, bool dirs=false) const =0;
		virtual bool mkdir(const string &path) const =0;

		virtual void message(const string &s) const =0;
		virtual bool confirm(const string &s) const =0;

		//===============================================================
		// случайные чиcла
		virtual void srand() const =0;
		virtual int rand(int max) const =0;
		
		//====================================================================
		// работа с большими блоками данных
		virtual	void *large_allocate(int size)
		{
			return new uint8[size];
		}

		//---------------------------------------------------------------
		virtual	void large_free(void *p)
		{
			delete[] p;
		}

		//---------------------------------------------------------------
		virtual	void large_write(void *start, uint8 *dst, uint8 *src, int size)
		{
			while (--size>=0)
				*(dst++)=*(src++);
		}



		//===================================================================
		// ф-ии необходимые для ogg decoder
		virtual	void	*realloc(void **p, int size)=0;

		//====================================================================
		// загрузка ресурсов
		virtual	sound *load_sound(mem_file *f, bool sfx) =0;
		virtual	file *file_open(const string &name, bool for_read=true, const string &path="") const =0;


		//---------------------------------------------------------------
		void release_pen_handler(pen_handler *h)
		{
			if (pd.target==h)
				pd.target=NULL;
		}


		//=======================================================================
		// регистрация
		string get_copy();	// число в формате xxxx-xxxx
	protected:
		virtual	string	get_id()=0;	// строка, уникальная для устройства
	};
}//mpp
#endif /* _MPP_PLATFORM_H_*/
