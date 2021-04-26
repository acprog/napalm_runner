/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef	_MPP_PREFS_
#define _MPP_PREFS_

#define MPP_VERSION	1

// общие параметры mpp, если будут меняться, то сразу для всех проектов
// иначе или медленно, или много шаблонов :((
#define	COLOR		color16

//#define	FIXED_REAL
#define FLOAT_REAL
//#define DOUBLE_REAL

#define LOOP_OFF	5	// <=5 музыки нет

#define BUFFER_SIZE	8000	// размер временного буффера
//#define OGG_SKIP	1000

//====================================================================
#ifdef	FIXED_REAL
	#define	REAL		Fixed<16>
	#define	BIG_REAL	Fixed<8>
	#define	TINY_REAL	Fixed<24>
#endif

#ifdef FLOAT_REAL
	#define	REAL		float
	#define	BIG_REAL	float
	#define	TINY_REAL	float
#endif

#ifdef DOUBLE_REAL
	#define	REAL		double
	#define	BIG_REAL	double
	#define	TINY_REAL	double
#endif

//====================================================================
// поганая конструкция
#ifdef __dest_os
	#if (__dest_os == __palm_os || __dest_os == __arm_bare )
		#define	__PALMOS__
	#endif
#endif


//====================================================================
#ifdef _DEBUG
	#define DEBUG_ONLY(exp)	exp
#else
	#define DEBUG_ONLY(exp)
#endif

#endif