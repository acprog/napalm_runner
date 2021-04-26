/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis SOURCE CODE IS (C) COPYRIGHT 1994-2002             *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: #ifdef jail to whip a few platforms into the UNIX ideal.
 last mod: $Id: os_types.h,v 1.14 2003/09/02 05:09:14 xiphmont Exp $

 ********************************************************************/
#ifndef _OS_TYPES_H
#define _OS_TYPES_H

/* make it easy on the folks that want to compile the libs with a
   different malloc than stdlib */
#define _ogg_malloc  my_malloc
#define _ogg_calloc  my_calloc
#define _ogg_realloc(ptr, size) my_realloc((void**)&ptr, size)
#define _ogg_free    my_free

#include "ogg_wrap.h"

#ifdef _WIN32
   /* MSVC/Borland */
   typedef __int64 ogg_int64_t;
   typedef __int32 ogg_int32_t;
   typedef unsigned __int32 ogg_uint32_t;
   typedef __int16 ogg_int16_t;
   typedef unsigned __int16 ogg_uint16_t;
#else
   typedef long long ogg_int64_t;
   typedef int ogg_int32_t;
   typedef unsigned int ogg_uint32_t;
   typedef short ogg_int16_t;
   typedef unsigned short ogg_uint16_t;
#endif

#endif  /* _OS_TYPES_H */
