/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _EDITOR_H_
#define _EDITOR_H_

#include "prefs.h"
#include "maze.h"
#include "resources.h"
#include "bars.h"

class runner;

//====================================================================
//	Редактор карт
//====================================================================
class editor
	:public static_modal< editor >
	,public base_maze
	,public pen_handler
{
private:
	cage	brush;
	string	name;

public:
	editor(map *map, const string &name);
	virtual ~editor();

	void close();

	//----------------------------------------------------------
	void event(redraw *screen, void*);

	void pen_down(const point<> &pos);
	void pen_move(const point<> &pos);

	void show_minimap() const;
	void set_brush(const cage c);
	void undo_all();
};




//====================================================================
//	Миникарта в редакторе
//====================================================================
class editor_minimap
	:public minimap
{
public:
	editor_minimap(const size<> &mm_size, const map &cages, const point<> &linkview)
		:minimap(mm_size, cages, linkview)
	{
	}

	void pen_down(const point<> &pos)
	{
		editor::get().center_to((pos-*this-point<>(5, 5))*CAGE_SIZE/2, editor::get());
	}

	void pen_move(const point<> &pos)
	{
		pen_down(pos);
	}
};


#endif