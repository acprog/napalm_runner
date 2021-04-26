#ifndef _MAP_H_
#define _MAP_H_

#include "prefs.h"
#include "cage.h"

//==============================================================================
//	����� �������� �� �������� ������, ���������, ��������, ���������� � ��
//==============================================================================
class map : public array2d<cage>
{
private:
	// �������� ����� �� �����
	void create_paths(int counter, int ladders_size, int soft, int hard, int ice, int mud, int rope);
	// ���������� ����� ��������
	bool solid_paths(int counter);
	// �������� ��������
	bool create_items(item_type i, int count, bool to_bottom=false, bool vertical=false, bool on_soft_only=false);
	void create_columns();
	string	map_name,
			land_name;
	static	int		level_size,
					level_difficult,
					level_players;

public:
	map(const string &land_name, const size<> &s);	// �������� ������ �����
	map(const map &m, const rect<> &r);	// ��������� ����� �����
	map(xml &x, const size<> &s);	// ��������

	void save(xml &x);

	// ��������� ����� �� ����������
	static map *generate(const string &land_name, const size<> &s, int density, int space, int ladders_size, 
		int soft, int hard, int ice, int mud, int rope,		
		int cashs, int monsters, int items, int bombs, int boxes);
	// ��������� ��������� �����
	static map *generate(const string &land_name, int size, int difficult, int n_players);
	map *next_level();	// ������� ��������� ������� � ���������� ���
	rect<>	body();	// ���������� ��������� ����� ���������
	void load_land();
	void operator=(const map &m);
	int difficult() const;
};


#endif