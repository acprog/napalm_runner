/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_REGISTRATION_H_
#define _MPP_REGISTRATION_H_

#include "controls.h"
#include "timer.h"

namespace mpp
{
	class locked_screen;

	//========================================================================
	//	диалог ввода ключа
	//========================================================================
	class registration
		:public controls::form
		,public static_class<registration>
	{
	private:
		controls::text_string
			tvisit,
			tsite,
			tcopy,
			copy,
			tkey;
		controls::input
			key;
		controls::button
			back;

	public:
		registration();
		void event(controls::button *b, void*);
		void event(controls::input *i, void*);

		bool pass_event(redraw *screen);
	};


	//========================================================================
	//	баннер
	//========================================================================
	class show_first
		:public controls::form
		,public static_class<show_first>
	{
	public:
		controls::text_string
			text1,
			text2,
			text3,
			text4,
			text5,
			text6,
			text7,
			text8,
			text9,
			text10,
			text11,
			text12,
			text13,
			text14,
			text15,
			text16,
			text17,
			text18,
			text19;
		show_first();
	};




	//========================================================================
	//	экран требующий регистрации
	//========================================================================
	class locked_screen : public handler<timer>
	{
	private:
		timer		check;
		show_first	*sf;
		string		key,
					lock;

		bool	go(bool fault);

	public:
		locked_screen();
		~locked_screen();
		void event(timer *t, void*);
	};
}

#endif // _MPP_REGISTRATION_H_
