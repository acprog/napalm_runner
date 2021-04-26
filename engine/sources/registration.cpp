/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "registration.h"
#include "application.h"
#include "platform.h"
#include "collection.h"
#include "math.h"

namespace mpp
{

//========================================================================
//	диалог ввода ключа
//========================================================================
registration::registration()
	:form	("registration")
	,tvisit	(this, "site/info")
	,tsite	(this, "site")
	,tcopy	(this, "copy/info")
	,copy	(this, "copy")
	,tkey	(this, "key/info")
	,key	(this, "key")
	,back	(this, "back")
{
	tcopy=string(tcopy)+"  "+application::get().name+" :";
	#if !defined(_WINDOWS) || defined(_DEBUG)
		copy=platform::get().get_copy();
		key=application::prefs("unlock_key", "xxxx-xxxx", true);
	#else	
		copy="Public beta";
	#endif
}


//========================================================================
void registration::event(controls::input *i, void*)
{
	if (i==&key)
	{
		string	str=key;
		for (int i=0; i<20; i++)
			str.replace(" ", "");
		if (str.length()!=9)
			platform::get().message(common::str("error/key"));
		else
		{
			#if !defined(_WINDOWS) || defined(_DEBUG)
				application::prefs("unlock_key")=str;
			#endif
		}
	}
}


//========================================================================
void registration::event(controls::button *b, void*)
{
	if (b==&back)
		delete this;
}


//========================================================================
bool registration::pass_event(redraw *screen)
{
	return !controls::keyboard::exist();
}



//========================================================================
//	баннер
//========================================================================
show_first::show_first()
	:form	(application::get().width>480 || application::get().height>480 ? "register_now2" : "register_now", "form")
	,text1	(this, "text1")
	,text2	(this, "text2")
	,text3	(this, "text3")
	,text4	(this, "text4")
	,text5	(this, "text5")
	,text6	(this, "text6")
	,text7	(this, "text7")
	,text8	(this, "text8")
	,text9	(this, "text9")
	,text10	(this, "text10")
	,text11	(this, "text11")
	,text12	(this, "text12")
	,text13	(this, "text13")
	,text14	(this, "text14")
	,text15	(this, "text15")
	,text16	(this, "text16")
	,text17	(this, "text17")
	,text18	(this, "text18")
	,text19	(this, "text19")
{
	if (application::get().width>480 || application::get().height>480)
	{	// банер сбоку
		width=(application::get().width*45)/100;
		height=application::get().height;
	}
	else
	{
		width=application::get().width;
		height=(application::get().height*40)/100;
	}
}




//========================================================================
//	экран требующий регистрации
//========================================================================
locked_screen::locked_screen()
	:check	(&application::get(), this, math::random(120.0f, 60.0f))
	,sf		(NULL)
	,key	(application::prefs("unlock_key"))
	,lock	(platform::get().get_copy())
{
	#if !defined(_WINDOWS) || defined(_DEBUG)
		if (!go(false))
			sf=new show_first;
	#endif
}


//========================================================================
locked_screen::~locked_screen()
{
	if (sf)
		delete sf;

	#if !defined(_WINDOWS) || defined(_DEBUG)
		if (key!=application::prefs("unlock_key") || lock!=platform::get().get_copy())
		{
			application::prefs("interface")="";	// чтоб доставить головную боль ломателям
			application::get().close();	// вываливаемся, если строки не соответствуют оригиналам
		}
	#endif
}


//========================================================================
void locked_screen::event(timer *t, void*)
{
	#if !defined(_WINDOWS) || defined(_DEBUG)
		if (t==&check)
			if (go(true) && !show_first::exist())
				application::get().close();	// мухлеж - закрываем приложение
		check.stop();
	#endif
}


//========================================================================
// проверяем ключ
// параметр - для передотвращения простой подмены возвращаемого значения
// означает каким не д/б возвращаемое значение в случае зарегестрированной версии
bool locked_screen::go(bool fault)
{
	#if !defined(_WINDOWS) || defined(_DEBUG)
		// алгоритм должен быть максимально простым и нелегким на угадывание
		string	str=lock;
		str[4]='7';	// подменяем '-' на 7 чтоб получить число
		long	n=str.atoi();	// теперь в n имеем 9-ти значное число
		// преобразуем число
		n^=683603956;
		n+=385763951;
		n^=(n>>16)*(n&0xffff);
		n%=1000000000;
		while (n<100000000)
			n*=10;
		// приводим обратно
		str=string(n, 10);
		str[4]='-';
		
		if (str==key)
			fault=!fault;
	#else
		fault=!fault;
	#endif

	return fault;
}

};	/* namespace mpp	*/