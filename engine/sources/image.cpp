/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "image.h"
#include "platform.h"
#include "collection.h"
#include "timer.h"
#include "mem.h"

namespace mpp
{

//==============================================================
//	Изображение
//==============================================================
image::image(const string &name, collection *coll)
	:bits		(NULL)
	,offset		(0)
	,free_bits	(false)
	,extern_bits(true)
{
	bits=coll->load_image(name, this);
}

		
//==============================================================
image::~image()
{
	if (extern_bits)
		large_mem::free(bits);

	if (free_bits)
		delete[] bits;
	bits=NULL;
}



//==============================================================
void image::put(const image &img, const point<> &v)
{
	ASSERT(bits==NULL, "image: bits==NULL");

	int 	x, y,
			sh=sizeof(COLOR)==2 ? 1 : 0;
	uint32	*dst=(uint32*)(bits+v.x+v.y*width),
			*src=(uint32*)img.bits;

	for (y=img.height; --y>=0; dst+=(width-img.width)>>sh, src+=(img.offset)>>sh)
		for (x=img.width>>sh; --x>=0;)
			*(dst++) = *(src++);
}


//==============================================================
void image::put_transparent(const image &img, const point<> &v)
{
	ASSERT(bits==NULL, "image: bits==NULL");

	int 	x, y;
	COLOR	*dst=bits+v.x+v.y*width,
			*src=img.bits;

	for (y=img.height; --y>=0; dst+=width-img.width, src+=img.offset)
		for (x=img.width; --x>=0; dst++, src++)
			if (*src)
				*dst=*src;
}

//==============================================================
void image::put_transparent_mirror(const image &img, const point<> &v)
{
	ASSERT(bits==NULL, "image: bits==NULL");

	int 	x, y;
	COLOR	*dst=bits+v.x+v.y*width,
			*src=img.bits;

	for (y=img.height; --y>=0; dst+=width, src+=img.offset)
		for (x=img.width, dst+=img.width; --x>=0; src++)
		{
			dst--;
			if (*src)
				*dst=*src;
		}
}



//==============================================================
void image::put_tiled(const image &img, const rect<> &r, bool transparent)
{
	ASSERT(bits==NULL, "image: bits==NULL");

	int 	x, y,// Экранные к-ты
			xt=0, yt=0,	// тайловые к-ты
			sh=sizeof(COLOR)==2 ? 1 : 0;

	if (transparent)
	{
		COLOR	*dst=bits+r.x+r.y*width,
				*src=img.bits;

		for (y=r.height, yt=img.height; --y>=0; dst+=width-r.width, yt--, src+=xt)
		{
			if (yt==0)
			{
				yt=img.height;
				src=img.bits;
			}
			for (x=r.width, xt=img.width; --x>=0; xt--)
			{
				if (xt==0)
				{
					xt=img.width;
					src-=xt;
				}
				if (*src)
					*dst=*src;
				dst++;
				src++;
			}
		}
	}
	else
	{
	#ifdef _WIN32_WCE
		COLOR	*dst=bits+r.x+r.y*width,
				*src=img.bits;

		for (y=r.height, yt=img.height; --y>=0; dst+=width-r.width, yt--, src+=xt)
		{
			if (yt==0)
			{
				yt=img.height;
				src=img.bits;
			}
			for (x=r.width, xt=img.width; --x>=0; xt--)
			{
				if (xt==0)
				{
					xt=img.width;
					src-=xt;
				}
				*dst=*src;
				dst++;
				src++;
			}
		}
	#else
		uint32	*dst=(uint32*)(bits+r.x+r.y*width),
				*src=(uint32*)img.bits;

		for (y=r.height, yt=img.height; --y>=0; dst+=(width-r.width+1)>>sh, yt--, src+=xt)
		{
			if (yt==0)
			{
				yt=img.height;
				src=(uint32*)img.bits;
			}
			for (x=r.width>>sh, xt=img.width>>sh; --x>=0; xt--)
			{
				if (xt==0)
				{
					xt=img.width>>sh;
					src-=xt;
				}
				*(dst++) = *(src++);
			}
		}
	#endif
	}
}



//==============================================================
void	image::put_safe(const image &img, point<> v, bool transparent, bool mirror)
{
	ASSERT(bits==NULL, "image: bits==NULL");

	point<>	lefttop(0, 0),
			rightbottom(img.width, img.height);

	if (v.x+img.width>=width)
		rightbottom.x=width-v.x;
	if (v.y+img.height>=height)
		rightbottom.y=height-v.y;

	if (v.x<0)
	{
		lefttop.x=-v.x;
		v.x=0;
	}
	if (v.y<0)
	{
		lefttop.y=-v.y;
		v.y=0;
	}

	if (mirror)
	{
		lefttop.x=img.width-lefttop.x;
		rightbottom.x=img.width-rightbottom.x;
		swap(lefttop.x, rightbottom.x);
		image	s(&img, rect<>(lefttop, rightbottom));
		put_transparent_mirror(s, v);
	}
	else
	{
        image	s(&img, rect<>(lefttop, rightbottom));
		if (transparent)
			put_transparent(s, v);
		else 
			put(s, v);
	}
}


//==============================================================
void image::fit(const image &img)
{
	ASSERT(bits==NULL, "image: bits==NULL");

	// сейчас это работает тока на увеличение в 2-а раза
	int 	x, y;
	COLOR	*dst=bits,
			*dst2=dst+width,		// вторая строка
			*src=img.bits,
			*src2=src+img.width;	// вторая строка

	for (y=img.height-1; --y>=0; dst+=width, dst2+=width)
		for (x=img.width; --x>=0; src++, src2++)
		{
			// рисуем сразу 4-е пикселя
			*(dst++) = *src;
			*(dst++) = *src + *(src+1);
			*(dst2++) = *src + *src2;
			*(dst2++) = *src + *(src+1) + *src2 + *(src2+1);
		}
}




//==============================================================
//	наложение маски по and
void image::and_mask(const image &mask)
{
	ASSERT(bits==NULL, "image: bits==NULL");

	COLOR	*src=mask.bits,
			*dst=bits;
	
	int	h, w;

	for (w=width; --w>=0; src+=mask.offset, dst+=offset)
		for (h=height; --h>=0; )
			*(dst++)&=*(src++);
}

/*
//==============================================================
void image::clear(COLOR fill)
{
	ASSERT(bits==NULL, "image: bits==NULL");

	uint32	*p=(uint32*)bits,
			f=fill;
	if (sizeof(COLOR)==2)
	{
		f|=(f<<16);
		for (int i=(width*height)>>1; --i>=0;)
			*(p++)=f;
	}
	else
		for (int i=width*height; --i>=0;)
			*(p++)=f;
}
*/


//==============================================================
void image::fill(COLOR c)
{
	ASSERT(bits==NULL, "image: bits==NULL");

	int 	x, y;
	COLOR	*dst=bits;

	for (y=height; --y>=0; dst+=offset)
		for (x=width; --x>=0;)
			*(dst++) = c;
}




//=======================================================================================
rect<> image::draw_border(image *i, const rect<> &r)
{
	int	h=i->height/2,
		w=i->width/2;
	
	put_transparent(image(i, rect<>(0, 0, w, h)), r);
	put_transparent(image(i, rect<>(0, h, w, h)), r.y_offset(r.height-h));
	put_transparent(image(i, rect<>(w, 0, w, h)), r.x_offset(r.width-w));
	put_transparent(image(i, rect<>(w, h, w, h)), point<>(r.width-w, r.height-h)+r);
	
	
	//---------------------------------------------------
	// верхняя граница
	COLOR	*dst=bits+r.x+w+r.y*width,
			*src=i->bits+w;
	int		x, y;
	for (y=h; --y>=0; src+=i->width, dst+=width-(r.width-w*2))
		if (*src)
			for (x=r.width-w*2; --x>=0; dst++)
				*dst=*src;
		else
			dst+=r.width-w*2;

	//---------------------------------------------------
	// нижняя граница
	dst=bits+r.x+w+(r.bottom()-h)*width;
	src=i->bits+w+h*i->width;
	for (y=h; --y>=0; src+=i->width, dst+=width-(r.width-w*2))
		if (*src)
			for (x=r.width-w*2; --x>=0; dst++)
				*dst=*src;
		else
			dst+=r.width-w*2;

	//---------------------------------------------------
	// левая граница
	dst=bits+r.x+(r.y+h)*width;
	src=i->bits+h*i->width;
	for (y=r.height-h*2; --y>=0; dst+=width-w, src-=w)
		for (x=w; --x>=0; dst++, src++)
			if (*src)
				*dst=*src;

	//---------------------------------------------------
	// правая граница
	dst=bits+r.right()-w+(r.y+h)*width;
	src=i->bits+w+h*i->width;
	for (y=r.height-h*2; --y>=0; dst+=width-w, src-=w)
		for (x=w; --x>=0; dst++, src++)
			if (*src)
				*dst=*src;

	return rect<>(r.x+w, r.y+h, r.width-w*2, r.height-h*2);
}



//---------------------------------------------------------
void image::draw_rect(const rect<> &r, COLOR c)
{
	ASSERT(bits==NULL, "image: bits==NULL");

	COLOR	*p=&operator[](r);
	int		i;
	for (i=r.width; --i>=0; p++)
		*p=c;
	
	p+=width+offset-r.width;
	for (i=r.height-2; --i>=0; p+=width+offset)
		*(p+r.width-1)=*p=c;

	for (i=r.width; --i>=0; p++)
		*p=c;
}






//=======================================================================================
//	анимация
//=======================================================================================
movie::movie(const string &name, collection *coll, REAL _period)
	:animation	(name, coll)
	,timer		(&application::get(), this, _period, infinite)
{
}


//=======================================================================================
const image &movie::frame()
{
	return get_frame(0, progress());
}


} /*mpp*/