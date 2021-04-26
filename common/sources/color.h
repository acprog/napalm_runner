#ifndef _MPP_COLOR_H_
#define _MPP_COLOR_H_

#include "tools.h"

namespace mpp
{
	class color16;

	//========================================================================================
	//	÷вет: 32 бита
	//========================================================================================
	class	color32
	{
	public:
		union
		{
			uint32	value;
			struct	//-------------------- каналы, составл€ющие цвет
			{
			uint8	blue,
					green,
					red,
					alfa;	//	Ќепрозрачный (!)
			}c;
		};

		//========================================== все ф-ции - inline!
		inline color32(uint32 v=0)
			:value	(v)
		{
		}

		//---------------------------------------
		inline color32(uint8 blue, uint8 green, uint8 red, uint8 alfa=0xff)
		{
			bgra(blue, green, red, alfa);
		}

		//---------------------------------------
		inline operator uint32(void) const
		{
			return value;
		}

		//---------------------------------------
		inline void operator=(uint32 v)
		{
			value=v;
		}

		//---------------------------------------
		inline void operator=(color32 v)
		{
			value=v.value;
		}

		//---------------------------------------
		inline void operator=(color16 v);

		//--------------------------------------- задание цвета по компонентам
		inline void rgb(uint8 r, uint8 g, uint8 b)
		{
			value=b | (g<<8) | (r<<16);
		}

		//--------------------------------------- задание цвета по компонентам
		inline void bgr(uint8 b, uint8 g, uint8 r)
		{
			value=b | (g<<8) | (r<<16);
		}

		//--------------------------------------- задание цвета по компонентам
		inline void argb(uint8 alfa, uint8 red, uint8 green, uint8 blue)
		{
			value=blue | (green<<8) | (red<<16) | (alfa<<24);
		}

		//--------------------------------------- задание цвета по компонентам
		inline void bgra(uint8 _blue, uint8 _green, uint8 _red, uint8 _alfa=0xff)
		{
			value=_blue | (_green<<8) | (_red<<16) | (_alfa<<24);
		}

		//--------------------------------------- задание цвета по компонентам
		inline void rgba(uint8 _red, uint8 _green, uint8 _blue, uint8 _alfa=0xff)
		{
			value=_blue | (_green<<8) | (_red<<16) | (_alfa<<24);
		}
	};	// color32







	//========================================================================================
	//	÷вет: 16 бит
	//========================================================================================
	class	color16
	{
	public:
		union
		{
			uint16	value;
			struct	//-------------------- каналы, составл€ющие цвет
			{
				uint16	blue:5,
						green:6,
						red:5;
			}c;
		};

		//========================================== все ф-ции - inline!
		inline color16(uint16 v=0)//black)
			:value	(v)
		{
			
		}


		//---------------------------------------
		inline color16(uint8 r, uint8 g, uint8 b)
		{
			rgb(r>>3, g>>2, b>>3);
		}

		//---------------------------------------
		inline	operator uint16() const
		{
			return value;
		}

		//---------------------------------------
		inline	const color16 &operator=(uint16 v)
		{
			value=v;
			return *this;
		}

		//---------------------------------------
		inline	const color16 &operator=(color16 v)
		{
			value=v.value;
			return *this;
		}

		//---------------------------------------
		inline	void operator=(color32 v)
		{
			rgb(v.c.red>>3, v.c.green>>2, v.c.blue>>3);
		}
		
		//--------------------------------------- задание цвета по компонентам
		inline	void rgb(uint8 r, uint8 g, uint8 b)
		{
			value=(r<<11) | (g<<5) | b;
		}

		//--------------------------------------- задание цвета по компонентам
		inline	void bgr(uint8 b, uint8 g, uint8 r)
		{
			value=(r<<11) | (g<<5) | b;
		}

		//----------------------------------------
		inline	void operator+=(color16 v)
		{
			// ћ≈ƒЋ≈ЌЌќ!!
			// потом заменить на оптимизированный по скорости код
			c.red+=v.c.red;
			if (c.red<v.c.red)
				c.red=0xff;
			
			c.green+=v.c.green;
			if (c.green<v.c.green)
				c.green=0xff;
			
			c.blue+=v.c.blue;
			if (c.blue<v.c.blue)
				c.blue=0xff;
		}

		//----------------------------------------
		inline	void operator-=(color16 v)
		{
			// ћ≈ƒЋ≈ЌЌќ!!
			// потом заменить на оптимизированный по скорости код
			if (c.red<=v.c.red)
				c.red=0;
			else
				c.red-=v.c.red;
			
			if (c.green<=v.c.green)
				c.green=0;
			else
				c.green-=v.c.green;
			
			if (c.blue<=v.c.blue)
				c.blue=0;
			else
				c.blue-=v.c.blue;
		}


		//----------------------------------------
		inline	color16 operator+(color16 v)
		{
			color16	res;
			
			res.c.red=(c.red+v.c.red)/2;
			res.c.green=(c.green+v.c.green)/2;
			res.c.blue=(c.blue+v.c.blue)/2;			
			
			return res;
		}

		//--------------------------------------------
		inline	void operator&=(color16 v)
		{
			value&=v.value;
		}
	};	// color16

	//---------------------------------------
	void color32::operator=(color16 v)
	{
		rgb(v.c.red<<3, v.c.green<<2, v.c.blue<<3);
	}
}
#endif	/* _MPP_COLOR_H_*/
