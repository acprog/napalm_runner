/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _RUNNER_H_
#define _RUNNER_H_

#include 	"prefs.h"
#include	"bars.h"

struct player_info;

//========================================================================
//	Napalm Runner
//  ласс приложени€ отвечает за переключение между экранами и глобальными
// параметрами приложени€
//========================================================================
class runner : public application
{
private:
	struct	key_type
	{
		int	left,
			right,
			up,
			down,
			break_left,
			break_right,
			put_bomb,
			item,
			pause;
	}keys;	// кнопки управлени€
	map		*buffer;	// дл€ востановлени€ уровн€

public:
	runner();
	~runner();

	void copy_to_buffer(map *m);
	void restart_level(player_info i);
	void next_level(player_info i);

	//----------------------------------------------------------
	// возвращает коды клавиш управлени€
	inline	static	key_type &key()
	{
		return get().keys;
	}

	//----------------------------------------------------------
	inline	static	runner &get()
	{
		return *(runner*)&application::get();
	}
};

#endif
