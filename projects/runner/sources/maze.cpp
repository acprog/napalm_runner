/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "prefs.h"
#include "maze.h"
#include "objects.h"
#include "game.h"
#include "resources.h"



//===============================================================
void	cage::normalize(cage &next, bool in_editor)
{
	if (*this==no_item)
		join().bottom=false;
	if (next==no_item)
		next.join().top=false;

	if (*this==water || *this==rope)
	{
		join().bottom=true;
		join().top=false;
	}

	if ((*this!=no_item || next!=no_item) && *this!=rope && *this !=water)
		return;

	if (next!=decor_far && next!=decor_near && next==no_item)
	{
		if (!in_editor)
			next.idt.subtype=math::random(3);
		if (next.subtype()==subtype())
			++next.idt.subtype;
	}
	
	
	//---------------------------------------------------------------
	// сопряжения вверх
	switch (type())
	{
	case ladder:
		join().bottom	=(next.idc.type==ladder);
		if (next==no_item)
			next.join().top	=(next.idc.type==ladder);
		break;

	case rope:
		join().bottom=true;
		join().top=false;
		if (next==no_item)
			next.join().top=false;
		break;

	case water:
		if (next==hard || next==soft || next==slow || next==fast)
			next.join().top=true;
		join().top=false;
		join().bottom=true;
		break;

	case decor_far:
		if (in_editor)
			idt.subtype=2;
		else
			idt.subtype=math::random(3, 2);
		join().bottom=(next==decor_far);
		if (next==no_item)
			next.join().top=join().bottom;
		break;

	case decor_near:
		if (in_editor)
			idt.subtype=2;
		else
			idt.subtype=math::random(3, 2);
		join().bottom=next.join().top=(next==decor_near);
		break;

	case empty:
		if ((next==soft || next==hard) && !in_editor && math::random(1))
		{
			if (next==hard)
				*this=(math::random(1) ? decor_near : decor_far);
			else
				*this=decor_far;
			idt.subtype=math::random(1);
			join().bottom=true;
		}
		break;

	default:
		join().bottom	=(next!=empty && next!=water && next!=rope);
		if (next==no_item)
			next.join().top	=(*this!=empty && next!=rope);
	}
	
	//---------------------------------------------------------------
	switch (next.type())
	{
	case water:
		next.join().top=false;
		next.join().bottom=true;
		next.idt.subtype=(int)&next;
		break;

	case rope:
		next.idt.subtype=(int)&next;
		break;

	case decor_far:
		next.join().top=(*this==decor_far);
		break;

	case decor_near:
		next.join().top=(*this==decor_near);
		break;
	}
}



//===============================================================
//	лабиринт
//===============================================================
base_maze::base_maze(map *_map)
	:on_screen		(_map->size()*CAGE_SIZE)
	,cages			(*_map)
	,n_bursts		(0)
{
	cages.load_land();

	xml	prefs("land_prefs", land::get().file_open("prefs.xml"));
	door_animate	=new timer(this, this, prefs("animation_periods/door", "1", true).atof(), timer::infinite);
	lo_cash_animate	=new timer(this, this, prefs("animation_periods/lo_cash", "1", true).atof(), timer::infinite);
	med_cash_animate=new timer(this, this, prefs("animation_periods/med_cash", "1", true).atof(), timer::infinite);
	hi_cash_animate	=new timer(this, this, prefs("animation_periods/hi_cash", "1", true).atof(), timer::infinite);
	water_animate	=new timer(this, this, prefs("animation_periods/water", "1", true).atof(), timer::infinite);

	mem_set<const image*>(cage_groups, 16, NULL);

	set_lands();
	cage_to_draw=new image(cage_groups[cage::hard], rect<>(0, 0, CAGE_SIZE, CAGE_SIZE/2));

	//----------------------------------------------------------
	// таблицы смещения для отрисовки клеток
	for (uint8 i=0; i<16; i++)
	{
		cage	c(i);
		cage_top_offset[i]=c.subtype()*CAGE_SIZE+(c.join().top ? CAGE_SIZE*CAGE_SIZE*4/2 : 0);
		cage_bottom_offset[i]=c.subtype()*CAGE_SIZE+(c.join().bottom ? CAGE_SIZE*CAGE_SIZE*4/2 : CAGE_SIZE*CAGE_SIZE*2*4/2);
	}
}


//===============================================================
base_maze::~base_maze()
{
	delete cage_to_draw;
	delete &cages;

	delete door_animate;
	delete lo_cash_animate;
	delete med_cash_animate;
	delete hi_cash_animate;
	delete water_animate;
}



//===============================================================
void base_maze::center_to(const point<> &pos, const size<> &window)
{
	on_screen.x=window.width/2-pos.x;
	on_screen.y=window.height/2-pos.y;

	if (n_bursts)
	{
		on_screen.x+=math::random(15, -15);
		on_screen.y+=math::random(15, -15);
	}

	if (on_screen.width<=window.width)
		on_screen.x=(window.width-on_screen.width)/2;
	else if (on_screen.x>0)
		on_screen.x=0;
	else if (on_screen.right()<window.width)
		on_screen.x=window.width-on_screen.width;
	
	if (on_screen.height<=window.height)
		on_screen.y=window.height-on_screen.height;
	else if (on_screen.y>0)
		on_screen.y=0;
	else if (on_screen.bottom()<window.height)
		on_screen.y=window.height-on_screen.height;
}





//===============================================================
void base_maze::set_lands()
{
	set_land(cage::decor_far,	land::get().decor_far);
	set_land(cage::ladder,		land::get().ladder);
	set_land(cage::broken,		land::get().broken);
	set_land(cage::decor_near,	land::get().decor_near);
	set_land(cage::mirage,		land::get().soft);
	set_land(cage::water,		land::get().water);
	set_land(cage::rope,		land::get().rope);

	set_land(cage::hard,		land::get().hard);
	set_land(cage::slow,		land::get().slow);
	set_land(cage::fast,		land::get().fast);
	set_land(cage::soft,		land::get().soft);
}


//===============================================================
const image	*base_maze::item_image(item_type i) const
{
	timer *t=NULL;
	switch (i)
	{
	case closed_door:
		return &land::get().door_close;

	case inactive_door:
		return &land::get().door_open;

	case active_door:
		return &land::get().door_active.get_frame(door_animate->progress());

	case lo_cash:
		t=lo_cash_animate;
		break;

	case med_cash:
		t=med_cash_animate;
		break;

	case hi_cash:
		t=hi_cash_animate;
		break;

	case iplayer:
		return &theme::get().player.get_frame(water_animate->progress());

	case imonster:
		return &land::get().monster.get_frame(water_animate->progress());
	
	case ibox:
		return &land::get().box.get_frame(0);
	
	default:
		return &theme::get().objects.get_frame(0, i);
	}
	return &land::get().objects.get_frame(t->progress(), i-lo_cash);
}



//===============================================================
void base_maze::draw(image *screen, bool near_plane)
{
	if (!near_plane)
	{
		// рисуем фон
		point<REAL>		bg_mult;
		if (on_screen.width>screen->width)
			bg_mult.x=(REAL)(land::get().background->width-screen->width)/(on_screen.width-screen->width);
		if (bg_mult.x>0.5)
			bg_mult.x=0.5;
		if (on_screen.height>screen->height)
			bg_mult.y=(REAL)(land::get().background->height-screen->height)/(on_screen.height-screen->height);
		if (bg_mult.y>0.5)
			bg_mult.y=0.5;
		point<>	bg_pos(bg_mult.x*on_screen.x, bg_mult.y*on_screen.y);
		if (bg_pos.x & 1)
			bg_pos.x^=1;
		screen->put_safe(*land::get().background, bg_pos);
	}

	set_lands();

	// сначала выводится на экран целая часть лабиринта
	point<>	p=on_screen,	// точка для отрисовки
			start(0, 0);	// координата на карте
	if (on_screen.x<0)
	{
		start.x=-on_screen.x/CAGE_SIZE+1;
		p.x=CAGE_SIZE-(-on_screen.x%CAGE_SIZE);
	}
	if (on_screen.y<0)
	{
		start.y=-on_screen.y/CAGE_SIZE+1;
		p.y=CAGE_SIZE-(-on_screen.y%CAGE_SIZE);
	}
	point<>	end(start.x+screen->width/CAGE_SIZE-1, start.y+screen->height/CAGE_SIZE-1);
	if (end.x>=cages.size().width)
		end.x=cages.size().width;
	if (end.y>=cages.size().height)
		end.y=cages.size().height;

	// отрисовка целых
	cage	c;
	point<>	cur, def=p;
	for (cur.y=start.y; cur.y<end.y; cur.y++, p.y+=CAGE_SIZE)
		for (cur.x=start.x, p.x=def.x; cur.x<end.x; cur.x++, p.x+=CAGE_SIZE)
		{
			c=cages[cur];
			if (c!=cage::empty && c.draw_near()==near_plane)
				if (c!=no_item)
					screen->put_transparent(*item_image(c), p);
				else
				{
					cage_to_draw->set(cage_groups[c.type()], cage_top_offset[c.id()]);
					screen->put_transparent(*cage_to_draw, p);
					p.y+=CAGE_SIZE/2;
					cage_to_draw->set(cage_groups[c.type()], cage_bottom_offset[c.id()]);
					screen->put_transparent(*cage_to_draw, p);
					p.y-=CAGE_SIZE/2;
				}				
		}

	// отрисовка кусков
	// нижний край
	if (end.y<cages.size().height)
	{
		cur.x=start.x;
		p.x=def.x;
		if (on_screen.x<0)
		{
			cur.x--;
			p.x-=CAGE_SIZE;
		}
		for (; cur.x<end.x; cur.x++, p.x+=CAGE_SIZE)
			draw_segment(*screen, cur, p, near_plane);
	}

	// верхний край
	if (on_screen.y<0)
	{
		cur=start.y_offset(-1);
		p=def.y_offset(-CAGE_SIZE);
		if (on_screen.x<0)
		{
			cur.x--;
			p.x-=CAGE_SIZE;
		}
		while (cur.x<end.x)
		{
			draw_segment(*screen, cur, p, near_plane);
			cur.x++;
			p.x+=CAGE_SIZE;
		}
	}

	// левый край
	if (on_screen.x<0)
	{
		cur=start.x_offset(-1);
		p=def.x_offset(-CAGE_SIZE);
		while (cur.y<end.y)
		{

			draw_segment(*screen, cur, p, near_plane);
			cur.y++;
			p.y+=CAGE_SIZE;
		}
	}

	// правый край
	if (end.x<cages.size().width)
	{
		cur=point<>(end.x, start.y);
		p=def+point<>((end.x-start.x)*CAGE_SIZE, 0);
		if (on_screen.y<0)
		{
			cur.y--;
			p.y-=CAGE_SIZE;
		}
		if (end.y<cages.size().height)
			end.y++;
		for (;cur.y<end.y; cur.y++, p.y+=CAGE_SIZE)
			draw_segment(*screen, cur, p, near_plane);
	}
}




//===============================================================
void base_maze::draw_segment(image &screen, const point<> &cur, point<> p, bool near_plane)
{
	cage c=cages[cur];
	if (c!=cage::empty && c.draw_near()==near_plane)
		if (c!=no_item)
			screen.put_safe(*item_image(c), p, true);
		else
		{
			cage_to_draw->set(cage_groups[c.type()], cage_top_offset[c.id()]);
			screen.put_safe(*cage_to_draw, p, true);
			p.y+=CAGE_SIZE/2;
			cage_to_draw->set(cage_groups[c.type()], cage_bottom_offset[c.id()]);
			screen.put_safe(*cage_to_draw, p, true);
		}
}



//===============================================================
void	base_maze::normalize_maze(bool in_editor)
{
	cage	*curr=&cages[point<>(0, 0)];
	cage	*next=curr+cages.size().width;

	int	i;
	for (i=cages.size().width*(cages.size().height-1); --i>=0; curr++, next++)
		curr->normalize(*next, in_editor);
	for (i=cages.size().width; --i>=0; curr++, next++)
		if (*curr==no_item)
			curr->join().bottom=true;
}
















//===============================================================
//	Игровой лабиринт
//===============================================================
maze::maze(map *_map)
	:base_maze	(_map)
	,n_cash		(0)
{
//	LOG("make object..");
	make_objects();
//	LOG("normalize..");
	normalize_maze();
//	LOG("calc cash..");
	calc_cash();

//	LOG("broken to mirajes..");
	// сломанные в миражи
	point<>	i;
	for (i.y=0; i.y<cages.size().height; i.y++)
		for (i.x=0; i.x<cages.size().width; i.x++)
			cages[i].broken_to_mirage();
//	LOG("maze constructed!");
}



//===============================================================
void maze::event(object_move *p)
{
	//------------------------------------------------------
	// смотрим текущую клетку
	switch (cages[p->obj->pos_on_map()].type())
	{
	case cage::rope:
		p->type=move_rope;
		return;

	case cage::ladder: 
		if (p->obj->pos_on_map().y<p->prev_pos.y)
            p->type=move_up;
		else
			p->type=move_down;
		return;

	case cage::water:
		p->type=move_sink;
		return;
	}

	// нижняя граница экрана
	if (p->obj->pos_on_map().y>=cages.size().height-1)
	{
		p->type=move_normal;
		return;
	}

	//------------------------------------------------------
	// смотрим клетку снизу
	p->type=move_drop;
	if  (cages[p->obj->pos_on_map()].type()!=cage::ladder)
		switch (cages[p->obj->pos_on_map().y_offset(1)].type())
		{
		case cage::fast:
			p->type=move_fast;
			return;

		case cage::hard: case cage::soft:
			p->type=move_normal;
			return;

		case cage::slow:
			p->type=move_mud;
			return;

		case cage::ladder: 
			p->type=move_normal;
		}
}



//===============================================================
void maze::event(check_move *m)
{
	if (m->to.x<0 || m->to.y<0 || m->to.x>=cages.size().width || m->to.y>=cages.size().height)
	{
		m->pass=false;
		return;
	}

	cage	from	=cages[m->from],
			to		=cages[m->to];

	if (to.inpassable())
		m->pass=false;
}



//===============================================================
void maze::event(check_creature_move *m)
{
	cage	from	=cages[m->from],
			to		=cages[m->to];

	if (m->to.y<m->from.y)
		if (m->pass && from!=cage::ladder)
			m->pass=false;
}




//===============================================================
bool	maze::break_cage(const point<> &pos)
{
	if (pos.x<0 || pos.x>=cages.size().width || pos.y<0 || pos.y>=cages.size().height)
		return false;
	
	cage	&c=cages[pos];

	if (pos.y>0)
	{
		cage	&t=cages[pos.y_offset(-1)];

		if (t.type()>=cage::hard && t==no_item)
			return false;
	}

	if (c!=cage::soft)
		return false;

	c.dig();
	new broken_cage(pos, false);
	return true;
}


//===============================================================
void	maze::restore_cage(const point<> &pos)
{
	cages[pos].undig();
}


//===============================================================
void maze::copy_cage(image &dst, const point<> &pos)
{
	cage	c=cages[pos];
	cage_to_draw->set(cage_groups[cage::soft], cage_top_offset[c.id()]);
	dst.put(*cage_to_draw);
	cage_to_draw->set(cage_groups[cage::soft], cage_bottom_offset[c.id()]);
	dst.put(*cage_to_draw, point<>(0, CAGE_SIZE/2));
}




//==========================================================================
void maze::event(creature_move *c)
{
	if (cages[c->pos_on_map()]!=no_item)
	{
		item_type	i=cages[c->pos_on_map()];
		c->grab_object(i);
		if (i==no_item)
		{
			cages[c->pos_on_map()]=cage::empty;
			minimap::get().erase_point(c->pos_on_map());
		}
		else
			cages[c->pos_on_map()]=i;
	}
}


//==========================================================================
void maze::event(burst *b)
{
	n_bursts--;
	point<>	i,
			from=b->pos_on_map()-point<>(2, 2),
			to=b->pos_on_map()+point<>(2, 2);

	for (i.y=from.y; i.y<=to.y; i.y++)
		for (i.x=from.x; i.x<=to.x; i.x++)
			if (cages.size().inside(i))
				if (b->burst_point(i))
					if (cages[i].burst())
					{
						new broken_cage(i, true);	
						for (int n=BURST_PIECES; --n>=0;)
							new piece(i, b->pos_on_map(), land::get().stone_punch, land::get().stones);
					}
}


//==========================================================================
void maze::start_burst()
{
	n_bursts++;
}



//==========================================================================
void maze::event(check_pile *p)
{
	if (p->pos.y<1)
		p->possible=false;
	else if (!cages[p->pos].check_pile() || !cages[p->pos.y_offset(-1)].check_up_pile())
		p->possible=false;
}



//==========================================================================
void maze::event(grab_item *g)
{
	cage			&c=cages[g->pos];

	if (c!=no_item && (item_type)c<ibomb)
	{
		item_type	item=c;
		swap(item, *g->item);
		if (item!=no_item)
			c=item;
		else
			c=cage::empty;
		g->grabbed=true;
	}
}




//==========================================================================
bool maze::open_door(const point<> &pos)
{
	// на случай если несоколько дверей
	if (!cages[pos].open_door())
		return false;

	door=pos;
	land::get().open_door->play();
	if (n_cash==0)
	{
		cages[pos].activate_door();
		door_active	d;
		d.pos=door;
		send(&d, this);
	}
	return true;
}


//==========================================================================
void maze::get_one_cash()
{
	if (--n_cash==0)
	{
		land::get().activate_door->play();
		if (cages[door].activate_door())
		{
			door_active	d;
			d.pos=door;
			send(&d, this);
		}
	}
}



//==========================================================================
void maze::calc_cash()
{
	cage	*m=&cages[point<>(0, 0)];
	for (int i=cages.size().area(); --i>=0; m++)
		if (*m!=no_item)
			if ((item_type)*m>=lo_cash && (item_type)*m<=hi_cash)
				n_cash++;
}


//==========================================================================
player	*maze::create_player(player_info pi)
{
	point<>	i;
	for (i.y=0; i.y<cages.size().height; ++i.y)
		for (i.x=0; i.x<cages.size().width; ++i.x)
			if (cages[i]==iplayer)
			{
				cages[i].erase();
				return new player(i, pi);
			}
	return NULL;
}


//==========================================================================
monster *maze::create_monster(player *target)
{
	point<>	i;
	for (i.y=0; i.y<cages.size().height; i.y++)
		for (i.x=0; i.x<cages.size().width; i.x++)
			if (cages[i]==imonster)
			{
				cages[i].erase();
				return new monster(i, target);
			}
	return NULL;
}



//==========================================================================
void maze::make_objects()
{
//	LOG("make_objects()");
	point<>	i;
	for (i.y=0; i.y<cages.size().height; i.y++)
		for (i.x=0; i.x<cages.size().width; i.x++)
		{
			if ((item_type)cages[i]<=ibomb)
			{
				// это имеет смысл только при траве
			//	new dropped(i, cages[i]);
			//	cages[i]=cage::empty;
			}
			else if (cages[i]==ibox)
			{
//				LOG("create box");
				cages[i].erase();
				new box(i);
//				LOG("box created!");
			}
			else if (cages[i]==closed_door)
				door=i;
		}
//	LOG("objects maked!");
}


//==========================================================================
bool maze::activate_door()
{
	if (cages[door]==inactive_door)
	{
		land::get().activate_door->play();
		cages[door]=active_door;
		return true;
	}
	return false;
}


//==========================================================================
bool	maze::possible_punsh(const point<> &p)
{
	if (p.x<0 || p.y<1 || p.x>=cages.size().width || p.y>=cages.size().height)
		return false;
	
	cage	c=cages[p],
			t=cages[p.y_offset(-1)];

	if (c.inpassable())
		return false;

	return t.type()>=cage::hard && t==no_item;
}







