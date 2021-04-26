/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_FONT_H_
#define _MPP_FONT_H_

#include "collection.h"

namespace mpp
{
	//==============================================================
	//	Шрифт
	//==============================================================
	class font : public size<>
	{
	private:
		int		style;	//стиль, номер градиента
		xml		*descriptor;
		uint8	*bits;
		image	gradient;
		int		widths[256],	// ширины букв
				offset[256],	// их смещение
				space_size,		// величина пробела
				distance_size;	// величина растояния между символами

		void	scan_while_black(int &offset) const;
		void	scan_while_white(int &offset) const;

	public:
		//----------------------------------------------------------
		// letters - Используемые в шрифте символы по очереди
		// splitted - символы из 2-х вертикальных частей
		font(const string &name, collection *coll);
		virtual	~font();

		//----------------------------------------------------------
		void print(image &sourface, const point<> &p, uint8 ch, int style=-1) const;
		void print(image &sourface, point<> p, const string &str, int style=-1) const;
		size<>	get_text_metric(const string &s) const;
		int set_style(int s);
	};	// font
}	// mpp

#endif /* _MPP_FONT_*/
