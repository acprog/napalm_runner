/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_MATH_H_
#define _MPP_MATH_H_

#include "platform.h"

//=============================================================
// math funcs
//=============================================================
namespace mpp{	namespace math{

	//---------------------------------------------------------------
	// от [max до min]
	inline int random(int max=0x7fff, int min=0)
	{
		return min+platform::get().rand(max-min+1);
	}

	//---------------------------------------------------------------
	inline REAL random(REAL max=1, REAL min=0)
	{
		return min+((REAL)platform::get().rand((max-min)*0x7ff))/(REAL)0x7ff;
	}
	
}}

#endif // _MPP_MATH_H_