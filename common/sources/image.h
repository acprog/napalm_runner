/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_IMAGE_H_
#define _MPP_IMAGE_H_

#include "mpp_prefs.h"
#include "geometric.h"
#include "tools.h"
#include "Fixed.h"
#include "color.h"
#include "string.h"

namespace	mpp
{
	class		collection;

	//==============================================================
	//	Изображение
	//==============================================================
	class image : public size<>	// размер в точках
	{
	protected:
		COLOR	*bits;
		bool	free_bits,		// удалить потом bits
				extern_bits;	// память выделена в platform. там же она и должна быть удалена
		int		offset;			// смещение от конца предыдущей строки до начала следующей
								// в точках

	public:
		//----------------------------------------------------------
		inline	image()
			:bits		(NULL)
			,offset		(0)
			,free_bits	(false)
			,extern_bits(false)
		{
		}


		//----------------------------------------------------------
		inline	image(const size<> &s, COLOR *_bits=NULL, int _offset=0)
			:size<>		(s)
			,bits		(_bits)
			,offset		(_offset)
			,free_bits	(bits==NULL)
			,extern_bits(false)
		{
			if (!bits)
				bits=new COLOR[width*height];
		}

		//----------------------------------------------------------
		inline	image(const image *parent, const rect<> &r)
			:size<>		(r)
			,bits		(parent->bits+(r.x+r.y*(parent->width+parent->offset)))
			,offset		(parent->width-r.width+parent->offset)
			,free_bits	(false)
			,extern_bits(false)
		{
		}

		//----------------------------------------------------------
		image(const string &name, collection *coll);

		//----------------------------------------------------------
		virtual ~image();

		//----------------------------------------------------------
		void	put(const image &img, const point<> &v=point<>(0, 0));

		//----------------------------------------------------------
		void	put_transparent(const image &img, const point<> &v=point<>(0, 0));

		//----------------------------------------------------------
		void	put_transparent_mirror(const image &img, const point<> &v=point<>(0, 0));

		//----------------------------------------------------------
		void	put_tiled(const image &img, const rect<> &r, bool transparent=false);

		//----------------------------------------------------------
		void	put_safe(const image &img, point<> v=point<>(0, 0), bool transparent=false, bool mirror=false);

		//----------------------------------------------------------
		// подогнать изображение в свои размеры
		void	fit(const image &img);

		//----------------------------------------------------------
		//	наложение маски по and
		void and_mask(const image &mask);

		//----------------------------------------------------------
		// 2-е ф-ции fill и clear иметь нет смысла
//		void clear(COLOR fill);

		//----------------------------------------------------------
		void fill(COLOR fill);

		//------------------------------------------------------------
		inline	COLOR &operator[](int i)
		{
			ASSERT(bits==NULL, "image: bits==NULL");
			ASSERT(i>=area(), "image: counter out of range");
			return bits[i];
		}

		//------------------------------------------------------------
		inline	COLOR &operator[](const point<> &p)
		{
			ASSERT(bits==NULL, "image: bits==NULL");
			ASSERT(p.x<0 || p.y<0 || p.x>=width || p.y>=height, "image: counter ["+p.x+"]["+p.y+"] out of range ["+width+"]["+height+"]");
			return bits[p.x + (p.y*(width+offset)) ];
		}

		//------------------------------------------------------------
		inline	COLOR* get_bits()
		{
			ASSERT(bits==NULL, "image: bits==NULL");
			return bits;
		}

		//------------------------------------------------------------
		inline	const	COLOR* readonly_bits() const
		{
			ASSERT(bits==NULL, "image: bits==NULL");
			return bits;
		}

		//----------------------------------------------------------
		// очень опасная фунция. потом после тестирования скорости от
		// нее избаваиться в первую очередь
		inline void set(const image *parent, int offset) const
		{
			const_cast<COLOR*>(bits)=const_cast<COLOR*>(parent->readonly_bits()+offset);
		}


		//---------------------------------------------------------
		rect<> draw_border(image *i, const rect<> &r);	// возвращает объем внутири контура
		void draw_rect(const rect<> &r, COLOR c);
	};







	//=========================================================================
	//	Анимация
	// кадры располагаются горизонтально
	// в одной анимации м/б несколько роликов для удобства они располагаются вертикально
	//=========================================================================
	class animation	: public image
	{
	protected:
		int			n_frames,		//колличество кадров
					n_films;		// колличество роликов
		image		frame;			// текущий кадр

	public:
		//================================================================================
		inline	animation(const string &name, collection *coll, const size<> frame_size)
			:image		(name, coll)
			,n_frames	(width/frame_size.width)
			,n_films	(height/frame_size.height)
			,frame		(this, frame_size)
		{
		}

		//================================================================================
		inline	animation(const string &name, collection *coll, int _n_frames, int _n_films)
			:image		(name, coll)
			,n_frames	(_n_frames)
			,n_films	(_n_films)
			,frame		(this, size<>(width/_n_frames, height/_n_films))
		{
		}


		//================================================================================
		// загрузка квадратной анимации (неизвестны ни размеры, ни колл-во кадров)
		inline	animation(const string &name, collection *coll)
			:image		(name, coll)
			,n_frames	(width>height ? width/height : 1)
			,n_films	(height>width ? height/width : 1)
			,frame		(this, size<>(width/n_frames, height/n_films))
		{
		}


		//================================================================================
		// по хорошему, нужно создать отдельный метод set_frame()
		virtual	const image &get_frame(int	frame_num, int film=0) const
		{
			if (frame_num>=n_frames)
				frame_num=n_frames-1;
			if (film>=n_films)
				film=n_films-1;
			frame.set(this, frame_num*frame.width+width*frame.height*film);
			return frame;
		}

		// state - [0..1), film - номер ролика
		inline	const image &get_frame(REAL	state, int film=0) const
		{
			return get_frame((int)(state*n_frames), film);
		}

		// state - [0..1), film - номер ролика
		inline	const image &get_frame(int frame_num, REAL film) const
		{
			return get_frame(frame_num, (int)(film*n_films));
		}

		// для любого колличества однотипных анимаций
		inline	const image &get_frame(REAL	state, REAL film) const
		{
			return get_frame(state, (int)(film*n_films));
		}
	};
}//mpp
#endif	/* _MPP_IMAGE_H_*/
