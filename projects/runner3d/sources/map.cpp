/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "map.h"

int		map::level_size=0,
		map::level_difficult=0,
		map::level_players=0;


//==============================================================================
//	карта отвечает за хранение клеток, генерацию, загрузку, сохранение и пр
//==============================================================================
map::map(const string &land, const size<> &s)
	:array2d<cage>	(s)
	,land_name		(land)
{
}


//========================================================================
// вырезание куска карты
map::map(const map &m, const rect<> &r)
	:array2d<cage>	(r)
	,land_name		(m.land_name)
{
	point<> src, dst;
	for (src.y=r.y, dst.y=0; dst.y<r.height; src.y++, dst.y++)
		for (src.x=r.x, dst.x=0; dst.x<r.width; src.x++, dst.x++)
			modify(dst)=m.read(src);
}


//========================================================================
map::map(xml &x, const size<> &sz)
	:array2d<cage>	(sz)
{
	land_name=x("land");

	const char 	*s=x("cages");
	cage		*c=&modify(point<>(0, 0));
	const cage	*end=c+count().area();
	for (; c!=end; c++, s++)
		if ('0'>'a')
			*c=(*s>='0' ? cage((item_type)(*s-'0')) : cage((cage::types)(*s-'a')));
		else
			*c=(*s>='a' ? cage((cage::types)(*s-'a')) : cage((item_type)(*s-'0')));
}



//========================================================================
void map::save(xml &x)
{
	x("width", true)=string(count().width, 10);
	x("height", true)=string(count().height, 10);
	x("land", true)=land_name;
	
	string	&s=x("cages");
	s.allocate(count().area());
	const cage	*c=&read(point<>(0, 0));
	const cage	*end=c+count().area();
	for (; c!=end; c++)
		s+=(*c==no_item ? 'a'+(cage::types)*c : '0'+(item_type)*c);
}





//========================================================================
void map::operator=(const map &m)
{
	point<>	i;
	for (i.y=0; i.y<count().height; ++i.y)
		for (i.x=0; i.x<count().width; ++i.x)
			modify(i)=m.read(i);
}


//========================================================================
map *map::generate(const string &land_name, int s, int difficult, int n_players)
{
	level_size=s;
	level_difficult=difficult;
	level_players=n_players;

	return generate(
		land_name, size<>(math::random(20)+s, math::random(10)+s), 
		difficult/2+20,		// density
		70,		// space
		15,		// ladders_size
		65,		// soft
		20,		// hard
		5,		// ice
		5,		// mud
		5,		// rope
		50,		// cashs
		difficult/10+level_size/20,		// monsters
		(100-difficult)/20,	// items
		(100-difficult)/30,	// bombs
		difficult/2			// boxes
		);			
}


//========================================================================
map *map::next_level()
{
	if (!maps::exist())
	{
		if (level_difficult<100)
			++level_difficult;
		else if (level_size<50)
			++level_size;

		// случайный land
        list<string> names;
		
		collection::get_names("land", names);
		int	n=math::random(names.count());
		while (string *s=names.each_ptr())
			if (n--==0)
			{
				land_name=*s;
				names.break_each();
				break;
			}

		return generate(land_name, level_size, level_difficult, level_players);
	}
	
	// следующий уровень в кампании
	int	level=maps::prefs("save/level").atoi()+1;
	string	name=maps::prefs("level"+string(level, 10)+"/name");
	if (name!="")
	{
		maps::prefs("save/level")=string(level, 10);
		return maps::get().load_map(name);		
	}

	// кампания закончена
	return NULL;
}



//========================================================================
// генерация карты по множеству параметров
map *map::generate(const string &land_name, const size<> &s, int density, int space, int ladders_size, 
		int soft, int hard, int ice, int mud, int rope, 
		int cashs, int monsters, int items, int bombs, int boxes)
{
	platform::get().srand();
	map *m=new map(land_name, s);
	int	blocks=100-space;
	blocks*=m->count().area();
	blocks/=100;
	m->create_paths((blocks*density)/100, ladders_size, soft, hard, ice, mud, rope);

	// урезаем лабиринт
	map	*cut=new map(*m, m->body());
	delete m;
	m=cut;

	m->create_columns();
	m->solid_paths((blocks*(100-density))/100);

	if (!m->create_items(iplayer, 1, true, false, true))
		m->modify(point<>(0, 0))=cage(iplayer);
		
	if (!m->create_items(closed_door, 1, true))
		m->modify(point<>(1, 0))=cage(closed_door);

	if (!m->create_items(key, 1, true))
		m->modify(point<>(2, 0))=cage(key);

	for (int it=items; --it>=0;)
		m->create_items((item_type)math::random(freeze, itrap), 1, true);

	// создаем монстров
	for (int i=1000; monsters>0;)
	{
		m->create_items(imonster,	monsters);
		monsters=0;

		if (--i<0)	
			break;

		// убираем близкостоящих к игрокам
		point<>	pl;
		for (pl.y=0; pl.y<m->count().height; ++pl.y)
			for (pl.x=0; pl.x<m->count().width; ++pl.x)
				if (m->read(pl)==iplayer)
				{	
					point<>	mn;
					for (mn.y=0; mn.y<m->count().height; ++mn.y)
						for (mn.x=0; mn.x<m->count().width; ++mn.x)
							if (m->read(mn)==imonster)
								if (abs(pl.x-mn.x)<10 && m->count().width>10)
								{
									m->modify(mn)=cage::empty;
									monsters++;
								}
				}
	}

	// ящики
	m->create_items(ibox,		(boxes*(100-space)*m->count().area())/60000, true, false, true);

	// убираем ящики из нижней строки
	for (point<> p(0, m->count().height-2); p.x<m->count().width; ++p.x)
		if (m->read(p)==ibox)
			m->modify(p)=cage::empty;

	m->create_items(ibomb,		bombs, true);

	int	ch=(cashs*space*m->count().area())/50000;
	m->create_items(lo_cash,	(ch*16)/21, false, true);
	m->create_items(med_cash,	(ch*4)/21, false, true);
	m->create_items(hi_cash,	ch/21, false, true);

	return m;
}


//==============================================================================
void map::create_paths(int counter, int ladders_size, int soft, int hard, int ice, int mud, int rope)
{
	map &map=*this;
	point<>	p;

	// сначала рисуем базовую линию внизу карты
	for (p=point<>(0, map.count().height-1); p.x<map.count().width; ++p.x)
		map[p]=cage::soft;

	while (counter>0)
	{
		do
		{
			p=point<>(	math::random(map.count().width-1), 
						math::random(map.count().height-1));
		}
		while (map[p]==cage::empty || map[p]==cage::ladder);
		
		//	можно попробовать рисовать уровень углами - отрезок по y, потом отрезок по x
		/*
		возможно стоит сначала проверить можно-ли вставить в точку этот угол, и только потом вставлять
		возвращаюсь к варианту с углами - оначала создавать углами уровень, а потом их облеплять
		углы можно сначала проверить 
		рисовать пожалуй стоит не опр. колл-во углов, а опр. колл-во клеток

		проверка углов: лестница не должна соприкасаться с другой лестницей
		поверхность не должна соприкасатьсся или пересекаться с лестницей или поверхностью
		*/

		// 1. выбираем точки
		point<>	beg=p,
				cen, end;
		
		do
			cen=map.count().force_inside(beg.y_offset(math::random(ladders_size, -ladders_size)));
		while (abs(cen.y-beg.y)<2);

		if (cen.y==0)
			++cen.y;	// неверх все-равно забраться нельзя

		do
			end=map.count().force_inside(cen.x_offset(math::random(ladders_size, -ladders_size)));
		while (abs(cen.x-end.x)<2);

		point<>	dy=point<>(0, beg.y>cen.y ? -1 : 1),
				dx=point<>(cen.x>end.x ? -1 : 1, 0);

		// 2. проверяем возможность вставить угол в указанную точку
		// пока поверхности не могут соприкасаться с другими пов-ми
		bool	pass=true;
		for (p=beg+dy; ; p+=dy)
		{
			if (map[p+dy])
				pass=false;
			if (p.x<map.count().width-1)
				if (map[p.x_offset(1)])
					pass=false;
			if (p.x>1)
				if (map[p.x_offset(-1)])
					pass=false;
			if (!pass)
				break;
			
			if (p==cen)
				break;
		}
		p-=dy;

		if (pass)
			for (p=cen+dx; p!=end; p+=dx)
			{
				if (map[p+dx])
					pass=false;
				if (p.y<map.count().height-1)
					if (map[p.y_offset(1)])
						pass=false;
				if (p.y>1)
					if (map[p.y_offset(-1)])
						pass=false;
				if (!pass)
					break;
			}

		// 3. рисуем угол
		if (pass)
		{
			// выбираем тип поверхности.
			int	terr_type=math::random(100);
			cage	c;
			if (terr_type<soft)
				c=cage::soft;
			else	if ((terr_type-=(int)soft)<hard)
				c=cage::hard;
			else	if ((terr_type-=(int)hard)<ice)
				c=cage::fast;
			else	if ((terr_type-=(int)ice)<mud)
				c=cage::slow;
			else
				c=cage::rope;

			if (dy.y<0 && map[beg]!=cage::rope)
				beg+=dy;

			for (p=beg; p!=cen; p+=dy, --counter)
				map[p]=cage::ladder;

			if (dy.y<0 || c==cage::rope)
			{
				map[p]=cage::ladder;
				p+=dx;
			}

			for (; p!=end; p+=dx, --counter)
				if (map[p]!=cage::ladder)
					map[p]=c;
		}
	}
}


//==============================================================================
bool map::solid_paths(int counter)
{
	map &map=*this;

	int	no_create=1000;	// через 1000 повторов вываливаться из цикла
	while (counter>0 && --no_create>0)
	{
		point<>	p(	math::random(map.count().width-1), 
					math::random(map.count().height-3, 1));

		if ((map[p]==cage::empty || map[p]==cage::decor_far || map[p]==cage::decor_near)
			&& (map[p.y_offset(1)]==cage::empty || map[p.y_offset(1)]==cage::decor_far || map[p.y_offset(1)]==cage::decor_near))
		{
			if (map[p.y_offset(-1)]==cage::soft)
			{
				map[p]=cage::soft;
				--counter;
				no_create=1000;
			}
			else if (map[p.y_offset(-1)]==cage::hard || map[p.y_offset(-1)]==cage::fast || map[p.y_offset(-1)]==cage::slow)
			{
				map[p]=cage::hard;
				--counter;
				no_create=1000;
			}
		}
	}
	return no_create>0;
}


//==============================================================================
bool map::create_items(item_type item, int count, bool to_bottom, bool vertical, bool on_soft_only)
{
	map	&map=*this;
	cage	c(item);

	int	no_create=10000;	// через 10000 повторов вываливаться из цикла
	while (count>0 && --no_create>0)
	{
		point<>	p(	math::random(map.count().width-1), 
					math::random(map.count().height-1));
		point<> i;

		if (map[p]==cage::empty)
		{
			// проверка на отсутствие под предметом воды
			bool	water=false;
			for (i=p; ++i.y<map.count().height;)
				if (map[i]==cage::water)
				{
					water=true;
					break;
				}
				else if (map[i].stop_drop())
				{
					if (on_soft_only && map[i]!=cage::soft)
						water=true;
					break;
				}
				else if (to_bottom)
					p=i;

			if (!water)
			{
				// проверка на возможность упасть сверху на предмет
				bool	top_pass=false;
				if (p.y>=map.count().height-1)
					top_pass=true;
				else
					if (map[p.y_offset(1)].stop_drop())
						top_pass=true;
					else
						for (i=p; --i.y>0 && !map[i].no_pass();)
						{
							if (map[i]==cage::ladder || map[i]==cage::rope)
							{
								top_pass=true;
								break;
							}

							if (i.x<=0)
								++i.x;
							else
							{
								--i.x;
								if (map[i]==cage::ladder || map[i]==cage::rope)
								{
									top_pass=true;
									break;
								}
								i.x+=2;
							}

							if (i.x<map.count().width-1)
								if (map[i]==cage::ladder || map[i]==cage::rope)
								{
									top_pass=true;
									break;
								}
							--i.x;
						}
				
				if (top_pass)
				{
					if (vertical)
						while (p.y<map.count().height && map[p]==cage::empty && count>0)	// в вертикальный ряд
						{
							map[p]=c;
							--count;
							no_create=10000;
							++p.y;
						}
					else if (map[p]==cage::empty)
						{
							map[p]=c;
							--count;
							no_create=10000;
						}
				}
			}
		}
	}

	return no_create!=0;
}



//==============================================================================
void map::create_columns()
{
	map &map=*this;
	int	no_create=1000;	// через 1000 повторов вываливаться из цикла
	/*
	колонна ставится, если сверху и снизу не веревка и не лестница, текущая клетка пустая,
	а также пусты соседние 2-е клетки
	*/
	point<>	p, b;

	for (p.y=1; p.y<map.count().height-1; ++p.y)
		for (p.x=2; p.x<map.count().width-2; ++p.x)
			// проверяем необходимость колонны
			if (!map[p] && !map[p.x_offset(-1)] && !map[p.x_offset(-2)] && !map[p.x_offset(1)] && !map[p.x_offset(2)]
				&& map[p.y_offset(-1)] && map[p.y_offset(-1)]!=cage::ladder && map[p.y_offset(-1)]!=cage::rope)
				{
					// проверяем нижний край
					for (b=p; ++b.y<map.count().height;)
						if (map[b]==cage::ladder || map[b]==cage::rope)
							break;
						else if (map[b])
						{
							// рисуем колонну
							cage	c;
							if (map[b]==cage::soft || map[p.y_offset(-1)]==cage::soft)
								c=cage::decor_far;
							else
								c=cage::decor_near;
								//c=(platform::read_only().random(2) ? cage::decor_near : cage::decor_far);
							while (--b.y>=p.y)
								map[b]=c;
							break;
						}
				}
}


//======================================================================================
rect<>	map::body()
{
	int	left=-1, 
		right=count().width, 
		top=-1,
		i;
	bool	stop;

	// ищем левую границу
	for (stop=false; !stop && ++left<count().width;)
		for (i=count().height-1; --i>=0;)
			if (read(point<>(left, i)))
			{
				stop=true;
				break;
			}

	for (stop=false; !stop && --right>=0;)
		for (i=count().height-1; --i>=0;)
			if (read(point<>(right, i)))
			{
				stop=true;
				break;
			}
	
	for (stop=false; !stop && ++top<count().height;)
		for (i=count().width; --i>=0;)
			if (read(point<>(i, top)))
			{
				stop=true;
				break;
			}

	if (left>=right)
		return rect<>(0, 0, count().width, count().height);	// пустой лабиринт
	
	return rect<>(left, top-1, right-left+1, count().height-top+1);
}


//===========================================================
void map::load_land()
{
	if (!land::exist())
		new land(land_name);
	else
		if (land::get().get_name()!=land_name)
			new land(land_name);
}


//===========================================================
int map::difficult() const
{
	return level_difficult;
}