#include "font.h"
#include "platform.h"
#include "collection.h"
#include "mem.h"

namespace mpp
{
	//==============================================================
	//	Шрифт
	//==============================================================
	font::font(const string &name, collection *coll)
		:descriptor		(new xml("font", coll->file_open("fonts/"+name+".font.xml")))
		,bits			((uint8*)coll->load_image(descriptor->record("mask"), this))
		,space_size		(height/6)
		,distance_size	(space_size/2)
		,gradient		(descriptor->record("gradient"), coll)
		,style			(0)
	{
		string	&letters=descriptor->record("chars"),
				&splitted=descriptor->record("split");

		int	i, offs;
		for (i=0; i<256; i++)	//обнуляем
			widths[i]=offset[i]=0;

		uint8	ch;
		for (i=0, offs=0; ch=letters[i]; i++)
		{
			scan_while_black(offs);
			offset[ch]=offs;
			scan_while_white(offs);
			if (splitted.find((char)ch))
			{
				scan_while_black(offs);
				scan_while_white(offs);
			}
			widths[ch]=offs-offset[ch];
		}
		delete descriptor;
	}


	//==============================================================
	font::~font()
	{
		large_mem::free(bits);
		//delete[] bits;
	}


	//==============================================================
	void	font::scan_while_black(int &offset) const
	{
		uint8	*p=bits+offset/8,
				mask=1<<((~offset) & 7),
				summ=0;
		int		y;

		for (y=height; --y>=0;)
			summ|=*(p+y*width/8);

		while (!(summ & mask))
		{
			offset++;
			mask>>=1;
			if (!mask)
			{
				p++;
				mask=1<<7;
				for (summ=0, y=height; --y>=0;)
					summ|=*(p+y*width/8);
			}
		}
	}


	//==============================================================
	void	font::scan_while_white(int &offset) const
	{
		uint8	*p=bits+offset/8,
				mask=1<<((~offset) & 7),
				summ=0;
		int		y;

		for (y=height; --y>=0;)
			summ|=*(p+y*width/8);

		while (summ & mask)
		{
			offset++;
			mask>>=1;
			if (!mask)
			{
				p++;
				mask=1<<7;
				for (summ=0, y=height; --y>=0;)
					summ|=*(p+y*width/8);
			}
		}
	}



	//==============================================================
	void font::print(image &sourface, const point<> &p, uint8 ch, int st) const
	{
		if (widths[ch]==0)
			return;

		uint8	*src;
		uint8	mask;
		COLOR	*dst=&(sourface[p]);
		const COLOR		*c=gradient.readonly_bits() + gradient.width*(st==-1 ? style : st);
		int		y, x;

		for (y=0; y<height; y++, dst+=sourface.width-widths[ch], c++)
		{
			src=bits+(offset[ch]+y*width)/8;
			mask=1<<((~offset[ch]) & 7);
			for (x=widths[ch]; --x>=0; dst++)
			{
				if (*src & mask)
					*dst=*c;
				mask>>=1;
				if (!mask)
				{
					src++;
					mask=1<<7;
				}
			}
		}
	}

	//==============================================================
	void font::print(image &sourface, point<> p, const string &str, int st) const
	{
		const char	*s=str;
		for (; *s; p.x+=widths[(uint8)*s++]+distance_size)
			if (*s==' ')
				p.x+=space_size;
			else
				print(sourface, p, (uint8)*s, st);
	}


	//==============================================================
	size<>	font::get_text_metric(const string &str) const
	{
		size<>	p(0, height);
		const char	*s=str;
		for (; *s; p.width+=widths[(uint8)*s++]+distance_size)
			if (*s==' ')
				p.width+=space_size;
		return p;
	}


	//==============================================================
	int font::set_style(int st)
	{
		int prev=style;
		if (st<gradient.height)
			style=st;
		return prev;
	}
}