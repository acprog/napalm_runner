/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "controls.h"
#include "platform.h"
#include "collection.h"
#include "timer.h"
#include "font.h"
#include "tools.h"
#include "sound.h"
#include "math.h"


namespace mpp{



//========================================================================
//	Переключатель окон
//========================================================================
modal::modal()
{
	application::get().goto_modal(this);
}


namespace controls{



//=======================================================================================
//	вычисляет информацию rect из xml и габаритов внешнего pen_handler
//=======================================================================================
xml_rect::xml_rect(pen_handler *ext, xml &xm)
	:pen_handler	(ext, rect<>())
	,layout			(xm)
{
	point<REAL>		mult(ext->width, ext->height);// для процентного перевода
	mult/=100;

	string str;

	//------------------------------------------------------------
	if (str=xm("left"))
		if (read(str, "center+", mult.x, x))
			x+=ext->width/2;
		else if (read(str, "center-", mult.x, x))
			x=ext->width/2-x;
		else if (read(str, "-", mult.x, x))
			x=ext->width-x;
		else
			read(str, "+", mult.x, x);

	//------------------------------------------------------------
	if (str=xm("top"))
		if (read(str, "center+", mult.y, y))
			y+=ext->height/2;
		else if (read(str, "center-", mult.y, y))
			y=ext->height/2-y;
		else if (read(str, "-", mult.y, y))
			y=ext->height-y;
		else
			read(str, "+", mult.y, y);

	//------------------------------------------------------------
	if (str=xm("width"))
		read(str, "", mult.x, width);
	else
		if (str=xm("right"))
		{
			if (read(str, "center+", mult.x, width))
				width+=ext->width/2;
			else if (read(str, "center-", mult.x, width))
				width=ext->width/2-width;
			else if (read(str, "-", mult.x, width))
				width=ext->width-width;
			else
				read(str, "+", mult.x, width);
			width-=x;
		}

	//------------------------------------------------------------
	if (str=xm("height"))
		read(str, "", mult.y, height);
	else
		if (str=xm("bottom"))
		{
			if (read(str, "center+", mult.y, height))
				height+=ext->height/2;
			else if (read(str, "center-", mult.y, height))
				height=ext->height/2-height;
			else if (read(str, "-", mult.y, height))
				height=ext->height-height;
			else
				read(str, "+", mult.y, height);
			height-=y;
		}

	//------------------------------------------------------------
	// чтоб контролы не вылезли за пределы формы
	if (width<0)
		width=0;
	else if (right()>ext->width)
		width=ext->width-x;
	if (height<0)
		height=0;
	else if (bottom()>ext->height)
		height=ext->height-y;
	if (x<0)
		x=0;
	if (y<0)
		y=0;

	// сдвиг по внешнему краю
	x+=ext->x;
	y+=ext->y;
}





//=======================================================================================
bool xml_rect::read(const string &str, const string &param, REAL mult, int &dst)
{
	if (param)
		if (!str.find(param))
			return false;

	dst=string(((const char*)str)+param.length()).atoi();
	if (str.end()[-1]=='%')
		dst=mult*dst;

	return true;
}






//=======================================================================================
//	Элемент управления
//=======================================================================================
base::base(form *f, const string &name)
	:xml_rect			(f, f->folder(name))
	,dispatcher<redraw>	(f)
	,dispatcher<move>	(f)
	,hidden				(false)
{
}


//=======================================================================================
void base::event(move *m, void*)
{
	x+=m->dpos.x;
	y+=m->dpos.y;
}


//=======================================================================================
void base::show(bool visible)
{
	hidden=!visible;
}


//=======================================================================================
bool base::pass_event(pen *p)
{
	if (hidden)
		return false;
	return pen_handler::pass_event(p);
}


//=======================================================================================
bool base::pass_event(redraw *p)
{
	return !hidden;
}






//=======================================================================================
//	Форма интерфейса
//=======================================================================================
form::form(const string &name, string force_ground)
	:xml			(name, common::get().file_open("forms/"+name+".form.xml"))
	,xml_rect		(&application::get(), *this)
	,tmp_prefs		(&application::prefs_folder("forms").folder(name))
	,prefs			(*tmp_prefs)
	,title			(record("title"))
	,ground			(NULL)
	,border			(NULL)
{
	if (record("ground")!="")
		force_ground=record("ground");
	if (force_ground!="")
		ground=load<image>(common::get(), force_ground);
	if (record("border")!="")
        border=load<image>(common::get(), record("border"));///new image(record("border"),	&common::modify());

	//-----------------------------------------------------------------------
	// область перемещения
	xml_rect	r(&application::get(), folder("movement_region"));
	if (r.width==0)
		r.x=x;
	else
	{
		r.width-=width;
		if (prefs("x", "", true))
			x=prefs("x").atoi();
	}

	if (r.height==0)
		r.y=y;
	else
	{
		r.height-=height;
		if (prefs("y", "", true))
			y=prefs("y").atoi();
	}
	movement_region=r;

	if (x>movement_region.right())
		x=movement_region.right();
	if (y>movement_region.bottom())
		y=movement_region.bottom();
}



//--------------------------------------------------------------------------------
form::~form()
{
	LOG("free form "+title);

	if (movement_region.width)
		prefs("x")=string(x, 10);
	if (movement_region.height)
		prefs("y")=string(y, 10);

	if (ground)
		common::get().free(ground);
	if (border)
		common::get().free(border);
}



//--------------------------------------------------------------------------------
void form::set_size(const size<> &s)
{
	if (movement_region.width)
		movement_region.width+=width-s.width;
	if (movement_region.height)
		movement_region.height+=height-s.height;

	width=s.width;
	height=s.height;

	if (x>movement_region.right())
		x=movement_region.right();
	if (y>movement_region.bottom())
		y=movement_region.bottom();
}


//--------------------------------------------------------------------------------
void form::event(redraw *screen, void *source)
{
	if (ground)
		screen->ptr->put_tiled(*ground, *this);
	if (border)
	{
		screen->ptr->draw_border(border, *this);

		if (title)
		{
			font *font=common::get().font;
			int prev_style=font->set_style(4);
			font->print(*screen->ptr, point<>(border->width/2, 5)+*this, title);
			font->set_style(prev_style);
		}
	}

	dispatcher<redraw>::event(screen, source);	// вообще говоря отрисовывать надо в обратном порядке
}





//=======================================================================================
void form::pen_down(const point<> &p)
{
	prev_pen_pos=p;
}



//=======================================================================================
void form::pen_move(const point<> &p)
{
	point<> old_pos=*this;
	shift(p-prev_pen_pos);
			
	if (x>movement_region.right())
		x=movement_region.right();
	if (x<movement_region.left())
		x=movement_region.left();
			
	if (y>movement_region.bottom())
		y=movement_region.bottom();
	if (y<movement_region.top())
		y=movement_region.top();

	move	m;
	m.dpos=*this;
	m.dpos-=old_pos;
	prev_pen_pos=p;

	send(&m, this, this);
}









//=======================================================================================
//	Строка текста
//=======================================================================================
text_string::text_string(form *f, const string &name)
	:base		(f, name)
{
	if (layout("align")=="left")
		align=to_left;
	else if (layout("align")=="right")
		align=to_right;
	else
		align=to_center;
	(*this)=layout("text");
}


//=======================================================================================
void text_string::event(redraw *screen, void*)
{
	const	font	&f=*common::get().font;
	f.print(*screen->ptr, text_pos+*this, text);
}


//=======================================================================================
bool text_string::pass_event(pen *p)
{
	// элемент полностью пассивен
	return false;
}



//=======================================================================================
text_string::operator const string &() const
{
	return text;
}


//=======================================================================================
const string &text_string::operator=(const string &new_str)
{
	text=new_str;

	size<>	s=common::get().font->get_text_metric(text);

	text_pos.y=(height-s.height)/2;
	switch (align)
	{
	case to_center:
		text_pos.x=(width-s.width)/2;
		break;

	case to_right:
		text_pos.x=width-s.width;
		break;

	case to_left:
		text_pos.x=0;
	}

	return text;
}




//=======================================================================================
//	Кнопка
//=======================================================================================
button::button(form *f, const string &name, handler<button> *h)
	:text_string	(f, name)
	,button_handler	(h!=NULL ? h : f)
	,pushed			(false)
	,anims			(NULL)
	,n_anims		(0)
	,free_on_exit	(true)
{
	string	&s=f->folder(name).record("picture", "", true);
	if (s)
	{
		anims=load<animation>(common::get(), s);//new animation(s, &common::modify(), 2, 1);
		width=anims->width/2;
		height=anims->height;
	}
}


//=======================================================================================
button::button(form *f, const string &name, const animation &_anims, int _n_anims, handler<button> *h)
	:text_string	(f, name)
	,button_handler	(h!=NULL ? h : f)
	,pushed			(false)
	,anims			(&_anims)
	,n_anims		(_n_anims)
	,free_on_exit	(false)
{
}


//=======================================================================================
button::~button()
{
	if (anims && free_on_exit)
		common::get().free(anims);
}

//=======================================================================================
void button::event(redraw *screen, void *source)
{
	if (anims)
	{
		screen->ptr->put_transparent(anims->get_frame(pushed ? 1 : 0, n_anims), *this);
		return;
	}

	if (text=="")	// если текста нет, лучше не отрисовывать вообще - лучший способ скрыть не нужные кнопки
		return;

	image	*i=(pushed ? &common::get().button_active : &common::get().button_passive);
	rect<>	intr=screen->ptr->draw_border(i, *this);
	image(screen->ptr, intr).fill((*i)[point<>(i->width/2, i->height/2)]);

	int prev_style=common::get().font->set_style(pushed ? 3 : 2);
	text_string::event(screen, source);
	common::get().font->set_style(prev_style);
}


//=======================================================================================
bool button::pass_event(pen *p)
{
	return base::pass_event(p);
}


//=======================================================================================
void button::pen_down(const point<> &p)
{
	common::get().click->play();
	pushed=true;
}


//=======================================================================================
void button::pen_up(const point<> &p)
{
	if (pushed)
	{
		pushed=false;
        button_handler->event(this, this);
	}
}


//=======================================================================================
void button::pen_move(const point<> &p)
{
	pushed=inside(p);
}




//=======================================================================================
//	Виртуальная клавиша
//=======================================================================================
virtual_key::virtual_key(form *ext, const string &name, int _id, const animation &_anims, int _n_anims)
	:button	(ext, name, _anims, _n_anims)
	,id		(_id)
{
}

//=======================================================================================
virtual_key::virtual_key(form *ext, const string &name, int _id)
	:button	(ext, name)
	,id		(_id)
{
}

//=======================================================================================
void virtual_key::pen_down(const point<> &pos)
{
	application::get().penkey_down(id);
}


//=======================================================================================
void virtual_key::pen_up(const point<> &pos)
{
	application::get().penkey_up();
}






//=======================================================================================
//	вводимый текст
//=======================================================================================
input::input(form *f, const string &name)
	:button			(f, name, this)
	,input_handler	(f)
{
}


//=======================================================================================
void input::event(button *b, void*)
{
	string	str=*this;
	new keyboard(str, this, "");
}

//=======================================================================================
void input::event(keyboard *k, void*)
{
	text_string::operator=(*k);
	input_handler->event(this, this);
}



//=======================================================================================
const string &input::operator=(const string &new_str)
{
	return text_string::operator=(new_str);
}




//=======================================================================================
//	вводимое число
//=======================================================================================
digital::digital(form *f, const string &_name, int _min, int _max)
	:button	(f, _name)
	,digital_handler	(f)
	,plus	(f, _name+"/plus", this)
	,minus	(f, _name+"/minus", this)
	,info	(f, _name+"/info")
	,prefs	(&f->prefs(_name, string((_max-_min)/4+_min, 10), true))
	,value	(prefs->atoi())
	,min	(_min)
	,max	(_max)
{
	text_string::operator=(*prefs);
}


//=======================================================================================
digital::~digital()
{
	*prefs=string(value, 10);
}


//=======================================================================================
void digital::event(button *b, void*)
{
	int	prev=value;

	if (b==&plus)
	{
		if (value<max)
			value++;
	}
	else if (b==&minus)
	{
		if (value>min)
			value--;
	}

	if (prev!=value)
	{
		*this=value;
		digital_handler->event(this, this);
	}
}



//=======================================================================================
void digital::pen_move(const point<> &p)
{
	if (abs(plus.y-minus.y)<=abs(plus.x-minus.x))	// кнопки по горизонтали
		*this=(max-min)*p.x/application::get().width+min;
	else
		*this=(max-min)*p.y/application::get().height+min;
	digital_handler->event(this, this);
}


//=======================================================================================
void digital::pen_up(const point<> &p)
{
	pushed=false;
}


//=======================================================================================
digital::operator int() const
{
	return value;
}


//=======================================================================================
int digital::operator=(int v)
{
	if (v<min)
		v=min;
	else if (v>max)
		v=max;
	text_string::operator=(string(value=v, 10));
	return v;
}



//=======================================================================================
void digital::random()
{
	*(this)=math::random(max, min);
}


//=======================================================================================
void digital::show(bool visible)
{
	base::show(visible);
	plus.show(visible);
	minus.show(visible);
	info.show(visible);
}










//=======================================================================================
//	Картинка на форме
//=======================================================================================
picture::picture(form *ext, const string &name, image *_img)
	:base		(ext, name)
	,img		(_img)
{
	if (!img)
		img=new image(ext->folder(name).record("picture"), &common::get());
}


//=======================================================================================
picture::~picture()
{
	delete img;
}


//=======================================================================================
void picture::event(redraw *screen, void*)
{
	screen->ptr->put_transparent(*img, *this);
}









//=======================================================================================
//	Список
//=======================================================================================
list::list(form *ext, const string &name, int interval)
	:base			(ext, name)
	,list_handler	(ext)
	,up				(ext, name+"/up", this)
	,down			(ext, name+"/down", this)
	,step			(interval+common::get().font->get_text_metric("a").height)
	,capacity		(height/step)
	,info			(ext, name+"/info")
	,pos			(0)
	,selected		(0)
{
}


//=======================================================================================
void list::event(button *b, void*)
{
	if (b==&up)
	{
		if (pos>0)
			--pos;
	}
	else if (b==&down)
	{
		if (pos<count()-capacity)
			++pos;
	}
	
	if (capacity==1 && selected!=pos)
	{
		selected=pos;
		list_handler->event(this, this);
	}
}
			

//=======================================================================================
void list::event(redraw *screen, void*)
{
	const	font	&f=*common::get().font;
	point<>			p=*this;
	int				i, sel=selected;

	// прокручиаем элементы выше начала
	for (i=pos; --i>=0 && each_ptr(); --sel);

	string *s=NULL;
	for (i=capacity; --i>=0 && (s=each_ptr())!=NULL; p.y+=step, --sel)
		f.print(*screen->ptr, p, *s, sel==0 ? 1 : 0);
	break_each();
}
			

//=======================================================================================
void list::pen_down(const point<> &p)
{
	int	n=(p.y-y)/step+pos;
	if (n<count() && n!=selected)
	{
		selected=n;
		list_handler->event(this, this);
	}
}


//=======================================================================================
void list::pen_move(const point<> &p)
{
	pen_down(force_inside(p));
}



//=======================================================================================
string list::get_selected()
{
	int	i=selected;

	// прокручиаем элементы до выделенного
	while (string *s=each_ptr())
		if (--i<0)
		{
			break_each();
			return *s;
		}
	return "";
}


//=======================================================================================
void list::remove_selected()
{
	int	i=selected;

	// прокручиаем элементы до выделенного
	while (string *s=each_ptr())
		if (--i<0)
		{
			remove(*s);
			if (selected>=count())
				selected--;
			break_each();
			break;
		}
}



//=======================================================================================
void list::random()
{
	selected=math::random(count()-1);
	list_handler->event(this, this);
}



//=======================================================================================
void list::select(const string &str)
{
	string	*s=NULL;
	for (selected=0; s=each_ptr(); selected++)
		if (*s==str)
		{
			break_each();
			int	l=count();
			if (capacity<=l)
			{
				pos=selected;
				if (pos>=l-capacity)
					pos=l-capacity;
			}
			return;
		}
	selected=0;
}



//=======================================================================================
void list::show(bool visible)
{
	base::show(visible);
	up.show(visible);
	down.show(visible);
	info.show(visible);
}









//========================================================================
//	keyboard
//========================================================================
keyboard::keyboard(const string &str, handler<keyboard> *_h, const string &title_name)
	:form			("keyboard")
	,input_source	(_h)
	,text			(this, "input")
	,cancel			(this, "cancel")
{
	text=str;

	keys.push_back(new key(' ',  "space"));
	keys.push_back(new key('\b', "backspace"));
	keys.push_back(new key('\r', "enter"));
	keys.push_back(new key('-',  "dash"));
			
	keys.push_back(new key('1', "n1"));
	keys.push_back(new key('2', "n2"));
	keys.push_back(new key('3', "n3"));
	keys.push_back(new key('4', "n4"));
	keys.push_back(new key('5', "n5"));
	keys.push_back(new key('6', "n6"));
	keys.push_back(new key('7', "n7"));
	keys.push_back(new key('8', "n8"));
	keys.push_back(new key('9', "n9"));
	keys.push_back(new key('0', "n0"));

	keys.push_back(new key('Q'));
	keys.push_back(new key('W'));
	keys.push_back(new key('E'));
	keys.push_back(new key('R'));
	keys.push_back(new key('T'));
	keys.push_back(new key('Y'));
	keys.push_back(new key('U'));
	keys.push_back(new key('I'));
	keys.push_back(new key('O'));
	keys.push_back(new key('P'));

	keys.push_back(new key('A'));
	keys.push_back(new key('S'));
	keys.push_back(new key('D'));
	keys.push_back(new key('F'));
	keys.push_back(new key('G'));
	keys.push_back(new key('H'));
	keys.push_back(new key('J'));
	keys.push_back(new key('K'));
	keys.push_back(new key('L'));

	keys.push_back(new key('Z'));
	keys.push_back(new key('X'));
	keys.push_back(new key('C'));
	keys.push_back(new key('V'));
	keys.push_back(new key('B'));
	keys.push_back(new key('N'));
	keys.push_back(new key('M'));

	if (title_name)
		title=common::str("input/"+title_name);
}
		
//========================================================================
void keyboard::event(button *b, void*)
{
	if (b==&cancel)
		delete this;
}


//========================================================================
void keyboard::event(key_char *ch, void*)
{
	string	s=text;

	switch (ch->ch)
	{
	case '\b':
		if (s.length()>1)
			text=string(s.begin(), s.end()-2);
		else
			text="";
		break;

	case '\r':
		input_source->event(this, this);
		delete this;
		break;

	default:
		s+=(char)ch->ch;
		text=s;
	}
}




//=======================================================================================
//	Линк на задание кнопок
//=======================================================================================
key_binder::key_binder(keys_binding *ext, const string &name, int *_key)
	:button				(ext, name, this)
	,handler<key_down>	(ext)
	,info				(ext, name+"/info")
	,key				(_key)
	,wait_key			(false)
{
	text_string::operator=(platform::get().key_name(*key));
}


//=======================================================================================
void key_binder::event(button *b, void*)
{
	if (wait_key)
		text_string::operator=(platform::get().key_name(*key=0));
	wait_key=!wait_key;
}


//=======================================================================================
void key_binder::event(key_down *kd, void*)
{
	if (wait_key)
		*key=kd->key_id;
	else if (*key==kd->key_id)
		*key=0;

	text_string::operator=(platform::get().key_name(*key));
	wait_key=false;
}





//========================================================================
//	Привязка кнопок
//========================================================================
keys_binding::keys_binding(dispatcher<key_down> *h)
	:form				("keys_binding")
	,dispatcher<key_down>(h ? h : &application::get())
	,exit				(this, "exit")
{
}


//=======================================================================================
void keys_binding::event(button *b, void*)
{
	if (b==&exit)
		delete this;
}


//=======================================================================================
void keys_binding::add_key(const string &name, int *link)
{
	binds.push_back(new key_binder(this, name, link));
}

}}	// mpp::controls
