/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_CONTROLS_
#define	_MPP_CONTROLS_

#include "mpp_events.h"
#include "static_class.h"
#include "string.h"
#include "xml.h"
#include "dobject.h"

namespace	mpp
{
	class platform;
	class application;
	class timer;
	class movie;


	//========================================================================
	//	Модальное состояние программы
	//========================================================================
	class modal
		:public static_source<dispatcher<redraw>,	application>
		,public static_source<dispatcher<key_down>,	application>
		,public static_source<dispatcher<key_up>,	application>
		,public static_source<dispatcher<cycle>,		application>
	{
	protected:
		modal();
	};


	//========================================================================
	//	Статическое модальное состояние
	//========================================================================
	template<class T>
	class static_modal
		:public static_class<T>
		,public modal
	{
	};


	
	//========================================================================
	namespace	controls
	{
		class form;
		class button;
		class input;
		class digital;
		class list;
		class keyboard;
		class keys_binding;

		//-------------------------------------------------------------------
		struct move
		{
			point<>	dpos;
		};

		//-------------------------------------------------------------------
		enum	text_align
		{
			to_left,
			to_right,
			to_center
		};


		//=======================================================================================
		//	вычисляет информацию rect из xml и габаритов внешнего pen_handler
		//=======================================================================================
		class xml_rect : public pen_handler
		{
		private:
			bool read(const string &str, const string &param, REAL mult, int &dst);	// читаем значение

		protected:
			xml	&layout;

		public:
			xml_rect(pen_handler *ext, xml &xm);
		};


		//=======================================================================================
		//	Элемент управления на форме
		//=======================================================================================
		class base
			:public	xml_rect
			,public dispatcher<redraw>
			,public dispatcher<controls::move>
		{
		private:
			bool	hidden;
		public:

			//---------------------------------------------------------
			base(form *parent, const string &name);
			
			bool pass_event(pen *p);
			bool pass_event(redraw *p);

			void event(controls::move *m, void *source);
			virtual	void show(bool visible);
		};



		//=======================================================================================
		//	Форма интерфейса
		//=======================================================================================
		class	form
			:public xml	// описание структуры формы
			,public	xml_rect
			,public static_source<dispatcher<redraw>,	application>
			,public dispatcher<move>
			,public handler<button>
			,public handler<input>
			,public handler<digital>
			,public handler<list>
			,public dobject
		{
		private:
			xml			*tmp_prefs;			// эта идиотская конструкция от того, что wce не понимает возвращенных ссылок
		public:
			xml			&prefs;				// сохраненные параметры формы
			
	
		protected:
			enum	styles
			{
				clear=0,
				with_place=1,
				with_border=2,
			};
			string		title;				// заголовок

		private:
			point<>		prev_pen_pos;
			rect<>		movement_region;	// окно для перемещениия
			image		*ground;
			image		*border;

		public:

			//---------------------------------------------------------
			form(const string &name, string force_ground="");
			~form();

			void event(button *b, void*){}
			void event(input *b, void*){}
			void event(digital *d, void*){}
			void event(list *d, void*){}
			void event(redraw *p, void*);
			void event(timer *t, void*);
			
			void pen_down(const point<> &p);
			void pen_move(const point<> &p);

			void set_size(const size<> &s);

			inline xml &get_prefs(const string &name)
			{
				return prefs.folder(name);
			}

			static void draw_border(image *screen, const image *src, const rect<> &r);
		};


		//=======================================================================================
		//	Строка текста
		//=======================================================================================
		class text_string
			:public base
		{
		protected:
			string		text;
		private:
			text_align	align;
			point<>		text_pos;

		public:
			//---------------------------------------------------------
			text_string(form *_parent, const string &name);
			void event(redraw *screen, void*);
			bool pass_event(pen *p);
			operator const string &() const;
			const string &operator=(const string &new_str);
		};


		//=======================================================================================
		//	Картинка на форме
		//=======================================================================================
		class picture : public base
		{
		protected:
			image	*img;

		public:
			//---------------------------------------------------------
			picture(form *parent, const string &name, image *_img=NULL);
			~picture();

			void event(redraw *screen, void*);
			
			bool pass_event(pen *p)
			{
				return false;
			}
		};



		//=======================================================================================
		//	Кнопка
		//=======================================================================================
		class button : public text_string
		{
		protected:
			bool	pushed;
			
			const	animation	*anims;
			int					n_anims;
			bool				free_on_exit;
			handler<button>		*button_handler;

		public:

			//---------------------------------------------------------
			button(form *_parent, const string &name, handler<button> *h=NULL);
			button(form *_parent, const string &name, const animation &_anims, int _n_anims=0, handler<button> *h=NULL);
			~button();

			void event(redraw *screen, void*);
			
			bool pass_event(pen *p);

			void pen_down(const point<> &p);
			void pen_move(const point<> &p);
			void pen_up(const point<> &p);
		};



		//=======================================================================================
		//	вводимый текст
		//=======================================================================================
		class input
			:public button
			,public handler<button>
			,public handler<keyboard>
		{
		public:

			//---------------------------------------------------------
			input(form *_parent, const string &name);
			void event(button *b, void*);
			void event(keyboard *k, void*);

			const string &operator=(const string &new_str);

		private:
			handler<input>	*input_handler;
		};


		//=======================================================================================
		//	вводимое число
		//=======================================================================================
		class digital
			:public button
			,public handler<button>
		{
		private:
			string	*prefs;
			button	plus,
					minus;
			int		value,
					min,
					max;
			handler<digital>	*digital_handler;
			text_string	info;

		public:
			//---------------------------------------------------------
			digital(form *_parent, const string &name, int _min=0, int _max=100);
			~digital();
			void event(button *b, void*);

			void pen_move(const point<> &p);
			void pen_up(const point<> &p);

			operator int() const;
			int operator=(int v);
			void random();

			void show(bool visible);
		};



		//=======================================================================================
		//	Виртуальная клавиша
		//=======================================================================================
		class virtual_key : public button
		{
		private:
			int		id;

		public:
			//---------------------------------------------------------
			// для картинки
			virtual_key(form *ext, const string &name, int _id, const animation &_anims, int _n_anims=0);
			virtual_key(form *ext, const string &name, int _id);
			void pen_down(const point<> &pos);
			void pen_up(const point<> &pos);
		};


		//=======================================================================================
		//	Список
		//=======================================================================================
		class list
			:public base
			,public mpp::list<string>
			,public handler<button>
		{
		private:
			button	up,
					down;
			int		step,
					selected,
					pos,		// позиция с начала списка
					capacity;	// кол-во строк в списке
			text_string	info;
			handler<list>	*list_handler;

		public:
			//---------------------------------------------------------
			list(form *_parent, const string &name, int interval=1);

			void event(button *b, void*);
			void event(redraw *screen, void*);
			
			void pen_down(const point<> &p);
			void pen_move(const point<> &p);

			string get_selected();
			void remove_selected();
			void random();
			void select(const string &str);

			void show(bool visible);
		};

		
		//========================================================================
		//	keyboard
		//========================================================================
		class keyboard
			:public form
			,public static_class<keyboard>
			,public static_source<handler<key_char>, application>
		{
		private:
			//=======================================================================
			// шаблон для буквы
			class key
				:public button
				,public handler<button>
				,public dobject
			{
			private:
				char ch;
			public:
				inline	key(char _char, const string &name="")
					:button	(&keyboard::get(), !name ? string(_char) : name, this)
					,ch		(_char)
				{
				}

				//------------------------------------------
				void event(button *b, void*)
				{
					key_char	c;
					c.ch=ch;
					send(&c, &keyboard::get(), this);
				}
			};

			handler<keyboard>	*input_source;

			text_string			text;
			button				cancel;
			ptr_list<key>		keys;

		public:
			keyboard(const string &text, handler<keyboard> *h, const string &title_name);
			virtual ~keyboard(){}
			void event(button *b, void*);
			void event(key_char *ch, void*);
			
			operator const string&() const
			{
				return text;
			}
		};


		//=======================================================================================
		//	Линк на задание кнопок
		//=======================================================================================
		class key_binder
			:public button
			,public handler<button>
			,public handler<key_down>
			,public dobject
		{
		protected:
			bool		wait_key;	// ожидание нажатия
			int			*key;		// кнопка-цель
			text_string	info;

		public:

			//---------------------------------------------------------
			key_binder(keys_binding *ext, const string &name, int *_key);

			void event(button *b, void*);
			void event(key_down *kd, void*);
		};

		
		//========================================================================
		//	Привязка кнопок
		//========================================================================
		class keys_binding
			:public form
			,public static_class<keys_binding>
			,public dispatcher<key_down>
		{
		private:
			button	exit;
			ptr_list<key_binder>	binds;

		protected:
			void add_key(const string &name, int *link);

		public:
			keys_binding(dispatcher<key_down> *h=NULL);
			void event(button *b, void*);
		};
	};	// controls
};	//mpp

#endif	/* _CONTROLS_H_*/
