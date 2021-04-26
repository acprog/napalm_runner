/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _PREFS_H_
#define _PREFS_H_

#include "..\\..\\..\\common\\sources\\all.h"
using namespace mpp;

#define	APP_VERSION				6
#define	CAGE_SIZE				32				// ������ ������
#define BURST_SIZE				(CAGE_SIZE*5)	// ������ ������
#define BOMB_TIMER_REPEAT		10				// ������ ��������

#define PLAYER_SPEED			4.5
#define	MONSTER_SPEED			3

#define TOOLBAR_HEIGHT			32

#define	LIVE_PRICE				1000	// ���� ����� (� �.�. :)))

#define	BURST_PIECES			10		// ����������� ������ �� ������

typedef	physics::state_vector<3, point< BIG_REAL > >	dynamic_point;

#endif