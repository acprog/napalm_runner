/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "maze.h"
#include "game.h"
#include "runner.h"



//====================================================================
//	—оздание - передвигаетс€ по карте
//====================================================================
creature::creature(const point<> &pos, const animation *ani)
	:motion_object	(pos, ani, size<>(CAGE_SIZE/2, CAGE_SIZE))
	,birth_pos		(pos)
	,item			(no_item)
	
	,unfroze		(this, this, 10)
	,prev_state		(NULL)

	,wait			(this, no_sound,					row_wait,			this, 2)
	,birth			(this, no_sound,					row_birth,			this, 1)
	,move			(this, no_sound,					row_move,			this, 1)
	,raise			(this, theme::get().raise,			row_raise,			this, 0.6)
	,drop			(this, no_sound,					row_drop,			this, 1)
	,move_on_rope	(this, no_sound,					row_move_on_rope,	this, 1)
	,move_on_mud	(this, no_sound,					row_move_on_mud,	this, 2)
	,move_up		(this, no_sound,					row_move_up,		this, 0.5)
	,move_down		(this, no_sound,					row_move_up,		this, 0.5)
	,trapped		(this, theme::get().trap_catch,		row_trapped,		this, 2)
	,frozen			(this, theme::get().froze,			row_frozen,			this, 1)
	,sink			(this, land::get().sink,			row_sink,			this, 2)
	,press			(this, theme::get().press_creature,	row_press,			this, 0.8)
	,piled			(this, theme::get().crash,			row_pile,			this, 0.5)
	,eat			(this, no_sound,					row_eat,			this, 1.5)
	,invisible		(false)
{
//	LOG("creature created!");
	set_state(birth);
	unfroze.stop();
}




//====================================================================
void creature::event(timer *t)
{
	if (t==&move)
		play(land::get().move);
	else if (t==&move_on_rope)
		play(land::get().move_on_rope);
	else if (t==&move_on_mud)
		play(land::get().move_on_mud);
	else if (t==&move_up || t==&move_down)
		play(land::get().move_on_ladder);
	else if (t==&birth)
	{
		invisible=false;
		set_state(wait);
		check_move	cm;
		cm.to=cm.from=pos_on_map();
		cm.to.y++;
		cm.pass=true;
		cm.box=NULL;
		send(&cm, this);
		if (cm.pass)
            need_drop();
	}
	else if (t==&raise)
		set_state(wait);
	else if (t==&sink)
		kill();
	else if (t==&press || t==&frozen || t==&piled)
	{
		if (item!=no_item)
			new dropped(pos_on_map(), item);
		item=no_item;
		kill();
	}
	else if (t==&unfroze)
	{
		unfroze.stop();
		if (state==&frozen)
			set_state(*prev_state);
	}
}



//====================================================================
bool creature::check_dpos(const point<> &dpos)
{
	if (!motion_object::check_dpos(dpos))
		return false;

	check_creature_move	pm;
	pm.from=pm.to=pos_on_map();
	pm.to+=dpos;	
	pm.pass=true;
	send(&pm, this);
	if (!pm.pass)
	{
		// откат - перемещение невозможно
		pos_on_screen().y=pos_on_map().y*CAGE_SIZE;	// выраниваем y по высоте
		if (mstate)
			mstate->stop();
		clash(false, NULL);
		dynamic.stop();
		dynamic.position()=pos_on_screen()+CAGE_SIZE/2;
		return false;
	}
	return true;
}


//====================================================================
void creature::move_on(const point<> &dpos, REAL speed)
{
	if (state==&drop 
	||	state==&raise
	||	state==&birth
	||	state==&sink
	||	state==&press
	||	state==&frozen
	||	state==&trapped
	||	state==&piled
	||	state==&eat)
		return;

	motion_object::move_on(dpos, speed);
}


//====================================================================
void creature::clash(bool after_drop, box *box)
{
	if (state==&frozen)
		frozen.start();
	else if (after_drop && state!=&eat)
		set_state(raise);
}


//====================================================================
void creature::kill()
{
	move_to(birth_pos, true);
	stop();
	set_state(birth);
}


//====================================================================
void creature::event(pile *p)
{
	if (p->pos_on_map()==pos_on_map().y_offset(-1))	// надомной
	{
		stop();
		set_state(piled);
	}
}



//====================================================================
void creature::event(box *p)
{
	if (p->pos_on_map()==pos_on_map().y_offset(-1))	// надомной
	{
		stop();
		set_state(piled);
	}
}


//====================================================================
void creature::set_move_state(move_type type)
{
	if (state==&eat)
	{
		eat.unpause();
		return;
	}

	if (state==&frozen || state==&trapped)
		return;

	animated_state	*move_states[]=
	{
		&move,			//	move_impossible,
		&move,			//	move_normal,
		&move_on_rope,	//	move_rope,
		&move_on_mud,	//	move_mud,
		&move,			//	move_fast,
		&drop,			//	move_drop,
		&move_up,		//	move_up,
		&move_down,		//	move_down,
		&sink			//	move_sink
	};

	set_state(*move_states[type]);
}





//====================================================================
void creature::event(burst *b)
{
	if (b->burst_point(pos_on_map()) && !invisible)
	{
		point<>	pos=pos_on_map();
		play(theme::get().explode_creature);
		if (item!=no_item)
			new dropped(pos_on_map(), item);
		item=no_item;
		kill();
		for (int n=BURST_PIECES; --n>=0;)
			new piece(pos, b->pos_on_map(), theme::get().flesh_punch, *ani, row_explode);
	}
}



//====================================================================
void creature::crash()
{
	center_cage();
	set_state(press);
}


//====================================================================
void creature::event(crash_on_cage *c)
{
	if (c->p->pos_on_map()==pos_on_map())
		crash();
}



//====================================================================
void creature::move_to(const point<> &new_pos, bool teleport)
{
	motion_object::move_to(new_pos, teleport);
	send(this, this);
}


//====================================================================
void creature::force_trapped()
{
	if (item!=no_item)
		new dropped(pos_on_map(), item);
	item=no_item;
	center_cage();
	set_state(trapped);
}





//====================================================================
void creature::froze(const point<> &pos)
{
	if (pos_on_map().y==pos.y
	&&	pos_on_map().x>=pos.x-1
	&&	pos_on_map().x<=pos.x+1	)
	{
		if (state==&trapped)
			return;	// чтоб небыло нареканий - и так пойман, хватит

		if (state!=&frozen)
			prev_state=mstate;
		if (state!=&drop)
			stop();
		if (item!=no_item)
			new dropped(pos_on_map(), item);
		item=no_item;
		set_state(frozen);
		frozen.stop();
		unfroze.start();
	}
}



//====================================================================
void creature::event(ice_smoke *f)
{
	froze(f->pos_on_map());
}



//====================================================================
void creature::event(check_box_move *m)
{
	if (pos_on_map()==m->to)
		m->pass=false;
}










//====================================================================
//	»грок
//====================================================================
player::player(const point<> &pos, player_info pi)
	:creature		(pos, &theme::get().player)
	,break_cage		(this, land::get().player_break_cage,	row_break,		this, 0.8)
	,exit			(this, theme::get().player_exit,		row_exit,		this, 2)
	,punch_box		(this, no_sound,						row_punch_box,	this, 0.5)
	,unpunch_box	(this, no_sound,						row_punch_box,	this, 0.5)
	,punsh_up		(this, land::get().player_punch,		row_punsh,		this, 1)
	,punsh_down		(this, no_sound,						row_punsh,		this, 1)
	,make_froze		(this, no_sound,						row_make_froze,		this, 0.5)
	,unmake_froze	(this, no_sound,						row_make_froze,		this, 0.5)
	,put_trap		(this, theme::get().player_put_trap,	row_put_trap,		this, 1)
	,wait_for_birth	(this, theme::get().player_birth_wait,	row_wait_for_birth,	this, 0.5)
	,continue_move	(false)
	,door			(-1, -1)	// по умолчанию вне карты
	,box_for_punch	(NULL)
	,startup_info	(pi)
	,player_info	(pi)
{
	set_state(wait_for_birth);
	invisible=true;
}



//====================================================================
void player::set_info_ptrs() const
{
	game_bar::get().set(&lives, &bombs, &cash, &item);
}





//====================================================================
void player::event(key_down *p)
{
	if (state==&wait_for_birth)
	{
	    play(theme::get().player_birth);
		invisible=false;
		set_state(birth);
		return;
	}

	if (	state==&drop 
		||	state==&birth
		||	state==&put_trap
		||	state==&exit
		||	state==&make_froze
		||	state==&frozen
		||	state==&trapped
		||	state==&eat
		||	state==&unmake_froze)
	{
		if (p->key_id==runner::key().left || p->key_id==runner::key().right)
			if (state==&drop || state==&raise)
			{
				turn_to_right=p->key_id==runner::key().right;
				continue_move=true;
			}

		if (p->key_id==runner::key().item && item==no_item)
			use_item();
		return;
	}	

	//---------------------------------------------------------------
	if (p->key_id==runner::key().left)
	{
		move_on(point<>(-1, 0), PLAYER_SPEED);
		if (!turn_to_right)
			continue_move=true;
	}

	//---------------------------------------------------------------
	else if (p->key_id==runner::key().right)
	{
		move_on(point<>(1, 0), PLAYER_SPEED);
		if (turn_to_right)
			continue_move=true;
	}

	//---------------------------------------------------------------
	else if (p->key_id==runner::key().up)
		move_on(point<>(0, -1), PLAYER_SPEED);

	//---------------------------------------------------------------
	else if (p->key_id==runner::key().down)
		move_on(point<>(0, 1), PLAYER_SPEED);

	//---------------------------------------------------------------
	else if (p->key_id==runner::key().break_left)
	{
		if (maze::get().break_cage(pos_on_map()+point<>(-1, 1)))
		{
			on_rope=(state==&move_on_rope);
			stop();
			turn_to_right=false;
			set_state(break_cage);
		}
	}
	//---------------------------------------------------------------
	else if (p->key_id==runner::key().break_right)
	{
		if (maze::get().break_cage(pos_on_map()+point<>(1, 1)))
		{
			on_rope=(state==&move_on_rope);
			stop();
			turn_to_right=true;
			set_state(break_cage);
		}
	}
	//---------------------------------------------------------------
	else if (p->key_id==runner::key().item)
		use_item();

	//---------------------------------------------------------------
	else if (p->key_id==runner::key().put_bomb)
	{
		if (bombs && state!=&move_up && state!=&move_down && state!=&move_on_rope)
		{
			bombs--;
			new class bomb(pos_on_map());
		}
	}
}




//====================================================================
void player::use_item()
{
	if (invisible)
	{
		invisible=false;
		return;
	}

	grab_item	g;
	g.item=&item;
	g.pos=pos_on_map();
	g.grabbed=false;
	send(&g, this);
	
	if (g.grabbed)
		play(theme::get().player_grab_item);
	else if (!invisible)
		if (state!=&move_up && state!=&move_down && state!=&move_on_rope)
			switch (item)
			{
			case key:
				if (maze::get().open_door(pos_on_map()))
					item=no_item;
				break;

			case itrap:
				item=no_item;
				stop();
				set_state(put_trap);
				break;

			case pick:
				stop();
				set_state(punsh_up);
				break;

			case freeze:
				stop();
				set_state(make_froze);
				break;

			case glass:
				invisible=true;
				item=no_item;
				break;
			}
}


//====================================================================
void player::event(door_active *d)
{
	if (d->pos==pos_on_map())
	{
		center_cage();
		set_state(exit);
	}
	else
		door=d->pos;
}


//====================================================================
void player::event(key_up *p)
{
	if (p->key_id==runner::key().left || p->key_id==runner::key().right)
		continue_move=false;

	if (state==&move || state==&move_on_mud)
	{
		stop();
		set_state(wait);
	}
	else if(
		state==&move_on_rope || 
		state==&move_up || 
		state==&move_down
		)
		stop();
}



//====================================================================
void player::restart()
{
	//штраф -нефиг сбрасывать игру заново
	startup_info.cash-=LIVE_PRICE/2;
	if (startup_info.cash<0)
		startup_info.cash=0;
	new try_again(startup_info);
}


//====================================================================
void player::move_to(const point<> &new_pos, bool teleport)
{
	creature::move_to(new_pos, teleport);
	send(this, this);
	if (new_pos==door)
	{
		center_cage();
		set_state(exit);
	}
}



//====================================================================
void player::clash(bool after_drop, box *box)
{
	creature::clash(after_drop, box);

	if (box)
	{
		set_state(punch_box);
		box_for_punch=box;
		continue_move=true;
	}
/*
	// геймплай покажет
	if (after_drop && continue_move && state==&raise)
	{
		set_state(wait);
		move_on(point<>(turn_to_right ? 1 : -1, 0), PLAYER_SPEED);
	}
*/
}


//====================================================================
void player::set_move_state(move_type type)
{
	if (type==move_drop && 	state==&wait_for_birth)
	{
		stop();
		wait_for_birth.unpause();
	}
	else
		creature::set_move_state(type);
}


//====================================================================
void player::event(timer *t)
{
	if (t==&break_cage)
	{
		if (on_rope)
		{
			set_state(move_on_rope);
			move_on_rope.stop();
		}
		else
			set_state(wait);
		return;
	}
	//---------------------------------------------------------------
	else if (t==&raise)
	{
		if (continue_move)
		{
			set_state(wait);
			move_on(point<>(turn_to_right ? 1 : -1, 0), PLAYER_SPEED);
			return;
		}
	}
	//---------------------------------------------------------------
	else if (t==&put_trap)
	{
		new trap(pos_on_map());
		set_state(wait);
	}
	//---------------------------------------------------------------
	else if (t==&exit)
	{
		land::get().complete_level->play();
		++lives;
		new level_complete(*this);
		return;
	}
	//---------------------------------------------------------------
	else if (t==&make_froze)
	{
		if (state==&frozen)
			prev_state=&wait;
		else
		{
			play(theme::get().player_make_freeze);
			new ice_smoke(pos_on_map().x_offset(turn_to_right ? 2 : -2), turn_to_right);
			set_state(unmake_froze);
		}
		return;
	}
	//---------------------------------------------------------------
	else if (t==&unmake_froze)
	{
		if (state==&frozen)
			prev_state=&wait;
		else
			set_state(wait);
		return;
	}
	//---------------------------------------------------------------
	else if (t==&punsh_up)
	{
		check_pile	cp;
		cp.pos=pos_on_map().x_offset(turn_to_right ? 1 : -1);
		cp.possible=true;
		send_up<check_pile>::event(&cp);
		if (cp.possible)
			new class pile(pos_on_map().x_offset(turn_to_right ? 1 : -1));
		set_state(punsh_down);
	}
	//---------------------------------------------------------------
	else if (t==&punsh_down)
		set_state(wait);
	//---------------------------------------------------------------
	else if (t==&punch_box)
	{
		box_for_punch->punch(turn_to_right);
		set_state(unpunch_box);
	}
	//---------------------------------------------------------------
	else if (t==&unpunch_box)
	{
		set_state(wait);
		if (continue_move)
			move_on(point<>(turn_to_right ? 1 : -1, 0), PLAYER_SPEED);
	}
	//---------------------------------------------------------------
	else if (t==&eat)
	{
		if (item!=no_item)
			new dropped(pos_on_map(), item);
		item=no_item;
		kill();
	}
	creature::event(t);
}


//====================================================================
void player::grab_object(item_type &i)
{
	if (i>=lo_cash && i<=hi_cash)
	{
		int	pre_cash=cash;
		static	int	price[]={5, 20, 100};
		cash+=price[i-lo_cash];
		i=no_item;
		maze::get().get_one_cash();
		play(land::get().player_grab_cash);
		if (cash/LIVE_PRICE!=pre_cash/LIVE_PRICE)
		{
			lives+=(cash/LIVE_PRICE-pre_cash/LIVE_PRICE);
			play(theme::get().player_buy_live);
		}
	}
	else if (i==ibomb)
	{
		bombs++;
		i=no_item;
		play(theme::get().player_grab_bomb);
	}
}




//====================================================================
void player::kill()
{
	if (--lives<=0)
		new game_over(score);
	else
	{
		creature::kill();
		
		set_state(wait_for_birth);
		invisible=true;
	}
}


//====================================================================
bool player::pass_event(draw_objects *p)
{
	if (invisible && state!=&wait_for_birth)
		return false;
	return object::pass_event(p);
}



//====================================================================
bool player::eat_me()
{
	if (state==&exit || invisible)
		return false;
	if (state!=&eat)
	{
		stop();
		set_state(eat);
		play(theme::get().monster_eat);
	}
	return true;
}



//====================================================================
void player::push_pick(bool _turn_to_right)
{
	if (item==pick)
	{
		turn_to_right=_turn_to_right;
		key_down	kd;
		kd.key_id=runner::key().item;
		event(&kd);
	}
}








//====================================================================
//	ћонстр
//====================================================================
monster::monster(const point<> &pos, player *_target)
	:creature		(pos, &land::get().monster)
	,target			(_target)
{
}


//====================================================================
void monster::move_to_target()
{
	if (	state==&press
		||	state==&drop
		||	state==&sink
		||	state==&birth
		||	state==&trapped
		||	state==&frozen
		||	state==&piled)
		return;

	if (pos_on_map()==target->pos_on_map() && state!=&trapped && state!=&frozen)
		if (target->eat_me())
		{
			stop();
			set_state(eat);
			return;
        }

	if (!target->visible())
		if (pos_on_map()==birth_pos)
			return;

	point<>			pos=pos_on_map(),
					dpos;
	const point<>	&to=(target->visible() ? target->pos_on_map() : birth_pos),
					&from=pos_on_map();

	if (from.y==to.y)
		dpos.x=(from.x>to.x ? -1 : 1);
	else
		if (!search_way(pos, to))
		{
			if (state!=&wait)
			{
				stop();
				center_x();
				set_state(wait);
			}
			return;
		}
		else
			if (pos.x==from.x)
				dpos.y=(from.y>to.y ? -1 : 1);
			else
				dpos.x=(from.x>pos.x ? -1 : 1);
	
	move_on(dpos, MONSTER_SPEED);

	// движение по лестнице
	if (state!=&frozen && dpos.x==0 && dpos.y!=0 && state!=&drop)
		center_x();
}


//====================================================================
// выбираетс€ ближайший путь к цели (цель сверху или снизу)
bool monster::search_way(point<> &pos, const point<> &to)
{
	/*
	алгоритм нужно заменить на следующий: искать крайне-левую позицию.
	после этого двигатьс€ вправо, ища крайне-правую позицию
	при нахожднении пути сравнивать с предыдущим значением на близость к 'to'
	выбираем ту, что к to расположена ближе (чтоб монстры скорее добирались до цели)

	можно еще считать 2 варианта: лево-право без учета текущего,
	а в конце его добавл€ть при выборе решени€x	
	*/

	point<>	left(-1, 0),
			right(1, 0),
			dy(0, pos.y>to.y ? -1 : 1),
			p=pos;
	int		l, r,
			dx=to.x-pos.x;	// смещение по x на цель от текущей позиции

	// ведем перебор влево, а потом вправо, кака€ позици€ ближе, ту и возвращаем
	for (l=0; !check_dpos(p, dy); l++, p.x--)
		if (!check_dpos(p, left))
		{
			for (r=1; !check_dpos(pos, dy); r++, pos.x++)
				if (!check_dpos(pos, right))
					return false;
			return true;
		}

	if (l==0)
		return true;

	p=pos;
	++p.x;
	for (r=1; !check_dpos(p, dy); r++, p.x++)
		if (!check_dpos(p, right))
		{
			pos.x-=l;
			return true;
		}

	// выбираем позицию путь, ближайший к цели
	if (abs(dx+l)>abs(dx-r))
		pos.x+=l;
	else
		pos.x-=r;
	return true;
}



//====================================================================
bool monster::check_dpos(const point<> &pos, const point<> &dpos)
{
	check_move	pm;
	pm.from=pm.to=pos;
	pm.to+=dpos;
	pm.pass=true;
	pm.box=NULL;
	send(&pm, this);
	if (pm.pass)
	{
		check_creature_move	cm;
		cm.from=cm.to=pos;
		cm.to+=dpos;
		cm.pass=true;
		send(&cm, this);
		return cm.pass;
	}
	return false;
}



//====================================================================
void monster::event(player_move *p)
{
	move_to_target();
}


//====================================================================
void monster::event(draw_on_minimap *p)
{
	p->p->draw_point(*p->screen, pos_on_map(), COLOR(0xff, 0, 0));
}



//====================================================================
void monster::event(timer *t)
{
	if (t==&raise)
	{
		set_state(wait);
		move_to_target();
	}
	else if (t==&eat)
	{
		set_state(wait);
		move_to_target();
	}
	else
		creature::event(t);

	if (t==&unfroze)
		move_to_target();
}


//====================================================================
void monster::event(pile *p)
{
	move_to_target();
	creature::event(p);
}


//====================================================================
void monster::move_to(const point<> &new_pos, bool teleport)
{
	creature::move_to(new_pos, teleport);
	move_to_target();
	send(this, this);
}



//====================================================================
void monster::grab_object(item_type &i)
{
	if (item==no_item && i>=lo_cash && i<=hi_cash && state!=&frozen && state!=&trapped)
	{
		item=i;
		i=no_item;
	}
}



//====================================================================
void monster::drop_grabbed()
{
	if (item!=no_item)
	{
		new dropped(pos_on_map().y_offset(-1), item);
		item=no_item;
	}
}








//====================================================================
//	√ор€ща€ бомба
//====================================================================
bomb::bomb(const point<> &pos)
	:animated_object(pos, &theme::get().bomb)
	,fire			(this, no_sound, 0, this, 0.3)
	,timeout		(&game::get(), this, 2)
	,bursted		(false)
{
	play(theme::get().bomb_put);
	set_state(fire);
	timeout.start();
}


//====================================================================
void bomb::event(timer *t)
{
#ifndef _WIN32_WCE
	if (t==&fire)
		play(theme::get().bomb_timer);
#endif
	if (t==&timeout)
	{
		bursted=true;
		new burst(pos_on_map());
			game::get().remove_object(this);
	}
}



//====================================================================
void bomb::event(burst *b)
{
	if (b->burst_point(pos_on_map()) && !bursted)
	{
		bursted=true;
		new burst(pos_on_map());
		game::get().remove_object(this);
	}
}



//====================================================================
//	¬зрыв
//====================================================================
burst::burst(const point<> &pos)
	:animated_object(pos, &theme::get().explosion, size<>(1, 1)*BURST_SIZE)
	,explosion		(this, no_sound, 0, this, 0.3)
{
	theme::get().bomb_explosion->play();
	maze::get().start_burst();
	pos_on_screen()-=CAGE_SIZE*2;	
	set_state(explosion);
}


//====================================================================
void burst::event(timer *t)
{
	if (t==&explosion)
	{
		send(this, this);
		game::get().remove_object(this);
	}
}



//====================================================================
bool burst::pass_event(draw_objects *p)
{
	return p->bursts_only && p->window.inside(pos_on_screen());
}


//====================================================================
bool burst::burst_point(const point<> &p)
{
	point<>	dp(abs(p.x-pos_on_map().x), abs(p.y-pos_on_map().y));
	
	if (dp.x>2 || dp.y>2)
		return false;
	
	if (dp.x==dp.y)
		return dp.x!=2;
	return true;
}




//====================================================================
//	–азрушенна€ клетка
//====================================================================
broken_cage::broken_cage(const point<> &pos, bool exploded)
	:animated_object(pos, NULL)
	,make	(this, no_sound, 0, this, 0.7)
	,unmake	(this, no_sound, 0, this, 0.7)
	,wait	(&game::get(), this, 10, 0)
{
	if (!exploded)
	{
		wait.stop();
		set_state(make);
	}
	send(this, this);
}



//====================================================================
void broken_cage::event(draw_objects *ptr)
{
	if (!state)
		return;

	const point<> &p=maze::get().screen_offset();
	image	&img=land::get().broken_cage;
	REAL	pos=mstate->progress();
	if (state==&unmake)
		pos=1-pos;
	maze::get().copy_cage(img, pos_on_map());
	img.and_mask(theme::get().break_mask.get_frame(pos));
	ptr->screen->put_safe(img, point<>(*this)+p, true);	
}


//====================================================================
void broken_cage::event(timer *t)
{
	if (t==&make)
	{
		make.stop();
		state=NULL;
		wait.start();
	}
	else if (t==&wait)
	{
		set_state(unmake);
		crash_on_cage	c;
		c.p=this;
		send(&c, this);
	}
	else if (t==&unmake)
	{
		maze::get().restore_cage(pos_on_map());
		game::get().remove_object(this);
	}
}


//====================================================================
void broken_cage::event(burst *b)
{
	if (b->burst_point(pos_on_map()))
	{
		if (mstate)
			mstate->stop();
		state=mstate=NULL;
		wait.stop();
		wait.start();
	}
}


//====================================================================
void broken_cage::event(monster_move *mv)
{
	if (mv->pos_on_map()==pos_on_map())
		if (state==&unmake)
			mv->crash();
		else
			mv->drop_grabbed();
}



//====================================================================
void broken_cage::event(creature_move *c)
{
	if (c->pos_on_map()==pos_on_map() && state==&unmake)
		c->crash();
}







//====================================================================
//	Ћовушка
//====================================================================
trap::trap(const point<> &pos)
	:object		(pos, &theme::get().trap)
	,wait		(this, no_sound, 0)
{
	set_state(wait);
}

	

//====================================================================
void trap::event(creature_move *c)
{
	if (c->pos_on_map()==pos_on_map())
	{
		c->force_trapped();
		delete this;
	}
}


//====================================================================
void trap::event(burst *b)
{
	if (b->burst_point(pos_on_map()))
		delete this;
}







//====================================================================
//	Ѕрошенные вещи
//====================================================================
dropped::dropped(const point<> &pos, item_type _item)
	:animated_object	(pos)
	,item				(_item)
{
}


//====================================================================
void dropped::event(creature_move *c)
{
	if (c->pos_on_map()==pos_on_map())
	{
		c->grab_object(item);
		if (item==no_item)
			delete this;
	}
}


//====================================================================
void dropped::event(draw_objects *p)
{
	p->screen->put_safe(*maze::get().item_image(item), pos_on_screen()+maze::get().screen_offset(), true);
}


//====================================================================
void dropped::event(grab_item *g)
{
	if (g->pos==pos_on_map() && !g->grabbed)
	{
		swap(*g->item, item);
		if (item==no_item)
			delete this;
		g->grabbed=true;
	}
}


//====================================================================
void dropped::event(burst *b)
{
	if (item>=itrap && item<=ibomb)
		if (b->burst_point(pos_on_map()))
			delete this;
}









//====================================================================
//	«аморозка
//====================================================================
ice_smoke::ice_smoke(const point<> &pos, bool mirror)
	:animated_object	(pos, &theme::get().ice_smoke, size<>(3, 1)*CAGE_SIZE)
	,wait				(this, no_sound, 0, this, 0.5)
{
	turn_to_right=mirror;
	set_state(wait);
	pos_on_screen().x-=CAGE_SIZE;
	send(this, this);
}

	
//====================================================================
void ice_smoke::event(timer *t)
{
	delete this;
}



//====================================================================
void ice_smoke::event(creature_move *c)
{
	c->froze(pos_on_map());
}




//====================================================================
//	«авал из камней
//====================================================================
pile::pile(const point<> &pos)
	:motion_object(pos, 0, size<>(1, 1)*CAGE_SIZE)
	,make			(this, land::get().pile_make_s,		&land::get().pile_make,	this,	1)
	,wait			(this, no_sound,					&land::get().pile,		this,	10)
	,unmake			(this, land::get().pile_unmake_s,	&land::get().pile_unmake,	this,	1)
{
	set_state(make);
	need_drop();
	send(this, this);
}

	
//====================================================================
void pile::event(timer *t)
{
	if (t==&make)
		set_state(wait);
	else if (t==&wait)
		set_state(unmake);
	else if (t==&unmake)
		remove();
}


//====================================================================
void pile::event(draw_objects *p)
{
	if (state==&make)
		pos_on_screen().y-=CAGE_SIZE/2;
	object::event(p);
	if (state==&make)
		pos_on_screen().y+=CAGE_SIZE/2;
}


//====================================================================
void pile::event(burst *b)
{
	if (b->burst_point(pos_on_map()))
		remove();
}


//====================================================================
void pile::remove()
{
	object_move	om;
	om.obj=this;
	om.prev_pos=pos_on_map();
	pos_on_map().y--;	// куданить надо переместить, чтоб вышесто€щие объекты упали
	send(&om, this);
	send(this, this);	// чтоб монстры знали что завал убран и могли двигатьс€
	delete this;
}


//====================================================================
void pile::event(check_move *m)
{
	if (m->to==pos_on_map())
		m->pass=false;
}


//====================================================================
void pile::set_move_state(move_type type)
{
	if (type==move_sink || type==move_rope)
		need_drop();
	mstate->unpause();
}



//====================================================================
void pile::clash(bool after_drop, box *box)
{
	send(this, this);
	mstate->unpause();
}




//====================================================================
//	 оробка
//====================================================================
box::box(const point<> &pos)
	:motion_object(pos, 0, size<>(1, 1)*CAGE_SIZE)
	,wait		(this, no_sound,	&land::get().box)
	,friction	(this, this, 0.22)
	,handler<burst>	(&game::get())
	,send_up<box>	(&game::get())
	,send_up<check_box_move>	(&game::get())
{
//	LOG("box constructor");
	set_state(wait);
	friction.stop();
}

	


//====================================================================
void box::event(burst *b)
{
	if (b->burst_point(pos_on_map()))
	{
		play(land::get().box_explode);
		for (int n=BURST_PIECES; --n>=0;)
			new piece(pos_on_map(), b->pos_on_map(), land::get().sliver_punch, land::get().slivers);

		object_move	om;
		om.obj=this;
		om.prev_pos=pos_on_map();
		pos_on_map().y--;	// куданить надо переместить, чтоб вышесто€щие объекты упали
		send(&om, this);
		delete this;
	}
}



//====================================================================
void box::event(check_move *m)
{
	if (m->to==pos_on_map())
	{
		m->pass=false;
		if (m->from.y==m->to.y)
			m->box=this;
	}
}

//====================================================================
void box::clash(bool after_drop, box *box)
{
	// те кто нужно - придав€тьс€
	if (after_drop)
	{
		play(land::get().box_punch);
		send(this, this);
	}
}

//====================================================================
void box::set_move_state(move_type type)
{
	if (type==move_sink)
	{
		play(land::get().box_sink);
		need_drop();
	}
	else if (type==move_rope)
		need_drop();
}


//====================================================================
void box::punch(bool to_right)
{
	check_box_move	cm;
	cm.from=cm.to=pos_on_map();
	cm.to.x+=to_right ? 1 : -1;
	cm.pass=true;
	send(&cm, this);
	if (!cm.pass || !check_dpos(point<>(to_right ? 1 : -1, 0)))
		theme::get().player_cannot_punch_box->play();
	else
	{
		land::get().box_punch->play();
		reset_state();
		dynamic.speed()=point<REAL>(to_right ? 15 : -15, 0);
		dynamic.accelerate()=point<REAL>(to_right ? -2.5 : 2.5, 0);
		friction.start();
	}
}


//====================================================================
void box::event(timer *t)
{
	if (t==&friction)
	{
		friction.stop();
		point<REAL>	accelerate=dynamic.accelerate().out(),
					speed=dynamic.speed().out(),
					position=dynamic.position().out();
		accelerate.x=0;
		speed.x=0;
		pos_on_screen().x=pos_on_map().x*CAGE_SIZE;
		position.x=pos_on_screen().x+CAGE_SIZE/2;
		dynamic.accelerate()=accelerate;
		dynamic.speed()=speed;
		dynamic.position()=position;
	}
}








//====================================================================
//	 уски от взрыва
//====================================================================
piece::piece(const point<> &pos, const point<> &burst_pos, sound *_punch, const animation &_anims, int _n_anims)
	:motion_object(pos, &_anims, size<>(CAGE_SIZE/2, CAGE_SIZE/4))
	,death		(&game::get(), this, math::random(5.0f, 3.0f))
	,frame		(math::random(1.0f))
	,n_anims	(_n_anims)
	,punch		(_punch)
{
	if (burst_pos==pos)
		dynamic.speed()=point<REAL>(math::random(10.0f, -10.0f), math::random(-5.0f, -20.0f));
	else
	{
		point<REAL>	speed;
		if (burst_pos.x<pos.x)
			speed.x=math::random(30.0f);
		else if (burst_pos.x>pos.x)
			speed.x=-math::random(30.0f);
		else
			speed.x=math::random(10.0f, -10.0f);

		if (burst_pos.y<pos.y)
			speed.y=math::random(30.0f);
		else if (burst_pos.y>pos.y)
			speed.y=-math::random(30.0f);
		else
			speed.y=-math::random(10.0f, 5.0f);
		
		dynamic.speed()=speed;
	}

	dynamic.accelerate()=point<REAL>(0, 1.5);
}


//====================================================================
void piece::need_drop()
{
	if (dynamic.speed().out().y==0)
		motion_object::need_drop();
	// ничего не делаем - продолжаем движение
}

//====================================================================
void piece::event(check_move *m)
{
	// ничего не делаем - € мелкий камень - не преграда
}


//====================================================================
void piece::event(object_move *p)
{
	if (p->prev_pos==pos_on_map().y_offset(1))
		need_drop();	// если объект переместилс€ из клетки подомной
}




//====================================================================
void piece::set_move_state(move_type type)
{
}


//====================================================================
void piece::event(timer *t)
{
	if (t==&death)
		game::get().remove_object(this);
}


//====================================================================
void piece::event(draw_objects *p)
{
	p->screen->put_safe(ani->get_frame(frame, n_anims), pos_on_screen()+maze::get().screen_offset(), true);
}




//====================================================================
bool piece::check_dpos(const point<> &dpos)
{
	check_move	pm;
	pm.from=pm.to=pos_on_map();
	pm.to+=dpos;	
	pm.pass=true;
	maze::get().event(&pm);
	if (!pm.pass)
	{
		play(punch);
		// откат - перемещение невозможно
		if (dpos.y>0)
		{
			pos_on_screen().y=pos_on_map().y*CAGE_SIZE;	// выраниваем y по высоте
			dynamic.stop();
			dynamic.position()=pos_on_screen()+CAGE_SIZE/2;
		}
		else
			dynamic.speed()=dynamic.speed().out()*-0.3;
		return false;
	}
	return true;
}
