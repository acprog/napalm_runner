#ifndef _OS_H
#define _OS_H
/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the XIPHOPHORUS Company http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: #ifdef jail to whip a few platforms into the UNIX ideal.
 last mod: $Id: os.h,v 1.33 2003/09/02 05:11:53 xiphmont Exp $

 ********************************************************************/

#include "os_types.h"

#include "misc.h"

#    define STIN static __inline


#ifndef M_PI
#  define M_PI (3.1415926536f)
#endif

#  define rint(x)   (my_floor((x)+0.5f)) 
#  define NO_FLOAT_MATH_LIB
#  define FAST_HYPOT(a, b) sqrt((a)*(a) + (b)*(b))



#ifndef min
#  define min(x,y)  ((x)>(y)?(y):(x))
#endif

#ifndef max
#  define max(x,y)  ((x)<(y)?(y):(x))
#endif

#ifndef VORBIS_FPU_CONTROL

	typedef int vorbis_fpu_control;
	
	inline int vorbis_ftoi(double f)
	{
		return (int)(f+.5);
	}

	// We don't have special code for this compiler/arch, so do it the slow way 
	#  define vorbis_fpu_setround(vorbis_fpu_control) {}
	#  define vorbis_fpu_restore(vorbis_fpu_control) {}

#endif



#endif /* _OS_H */
