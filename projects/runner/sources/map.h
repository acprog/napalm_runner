/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MAP_H_
#define _MAP_H_

#include "prefs.h"
#include "cage.h"

//==============================================================================
//	карта отвечает за хранение клеток, генерацию, загрузку, сохранение и пр
//==============================================================================
class map : public array_xy<cage>
{
private:
	// создание путей на карте
	void create_paths(int counter, int ladders_size, int soft, int hard, int ice, int mud, int rope);
	// дополнение путей клетками
	bool solid_paths(int counter);
	// создание предмета
	bool create_items(item_type i, int count, bool to_bottom=false, bool vertical=false, bool on_soft_only=false);
	void create_columns();
	string	map_name,
			land_name;
	static	int		level_size,
					level_difficult,
					level_players;

public:
	map(const string &land_name, const array_size &s);	// создание чистой карты
	map(const map &m, const rect<> &r);	// вырезание куска карты
	map(xml &x, const array_size &s);	// загрузка

	void save(xml &x);

	// генерация карты по параметрам
	static map *generate(const string &land_name, const array_size &s, int density, int space, int ladders_size, 
		int soft, int hard, int ice, int mud, int rope,		
		int cashs, int monsters, int items, int bombs, int boxes);
	// примерная генерация карты
	static map *generate(const string &land_name, int size, int difficult, int n_players);
	map *next_level();	// создает следующий уровень и возвращает его
	rect<>	body();	// возвращяет смысловую часть лабиринта
	void load_land();
	void operator=(const map &m);
	int difficult() const;
};


#endif