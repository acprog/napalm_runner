/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_COLLECTION_
#define _MPP_COLLECTION_

#include "platform.h" 

namespace mpp
{
	class collection;
	class font;
	class sound;

	//========================================================================
	//	 оллекци€ ресурсов
	//========================================================================
	class	collection
	{
	private:
		//--------------------------------------------------------
		struct	BITMAPFILEHEADER
		{ 
//			uint16    bfType; 
			uint32    bfSize; 
			uint16    bfReserved1; 
			uint16    bfReserved2; 
			uint32    bfOffBits; 
		};


		//--------------------------------------------------------
		struct	BITMAPINFOHEADER
		{
			uint32   biSize; 
			uint32   biWidth; 
			uint32   biHeight; 
			uint16   biPlanes; 
			uint16   biBitCount; 
			uint32   biCompression; 
			uint32   biSizeImage; 
			uint32   biXPelsPerMeter; 
			uint32   biYPelsPerMeter; 
			uint32   biClrUsed; 
			uint32   biClrImportant; 
		};

		void	*zip_file;
		list<sound*>		sounds;	// дл€ автоматического удалени€ загруженых звуков

		COLOR		*load_24bit_bitmap(file *f, BITMAPINFOHEADER	*info) const;
		COLOR		*load_8bit_bitmap(file *f, BITMAPINFOHEADER	*info, int n_colors) const;
		COLOR		*load_1bit_bitmap(file *f, int width, int height) const;

	protected:
		string		folder;

	public:
		collection(const string &name, const string &category);
		virtual	~collection();
		COLOR	*load_image(const string &name, size<> *image_size) const;
		sound	*load_sound(const string &name, bool sfx=true);
		static void	get_names(const string &category, list<string> &names);
		void	find_files(const string &ext, list<string> &names);
		string get_name() const;
		file	*file_open(const string &name, bool for_read=true) const;

		//---------------------------------------------------------
		// кэшированние ресурсов
	private:
		//---------------------------------------------------------
		class cache_record
		{
		public:
			string	res_name;
			int		n_used;		// сколько источников используют ресурс
			void	*ptr;		// привеленный указатель (чтоб не паритс€ шаблонами)

		public:
			cache_record();
			cache_record(const cache_record &c);
			cache_record(void *_ptr, const string &name);
			bool operator==(const cache_record &c) const;
		};
		list<cache_record>	cache;

	public:
		template<class T>
		void free(const T *ptr)
		{
			while (cache_record *c=cache.each_ptr())
				if ((const T*)c->ptr==ptr)
				{
					if (--c->n_used<=0)
					{
						delete (T*)c->ptr;
						cache.remove(*c);
					}
					cache.break_each();
					return;
				}
			ERROR("corrupt resources cache");
		}

		//---------------------------------------------------------
		inline void cache_push(void *ptr, const string &name)
		{
			cache.push_back(cache_record(ptr, name));
		}

		//---------------------------------------------------------
		void	*seek_in_cache(const string &name);
	};	// collection


	//===============================================================
	// т.к. в wince нет €вных шаблонов ф-ий внутри классов
	template<class T>
	T *load(collection *c, const string &name)
	{
		T	*ptr=(T*)c->seek_in_cache(name);
		if (!ptr)
		{
			ptr=new T(name, c);
			c->cache_push(ptr, name);
		}
		return ptr;
	}


	//===============================================================
	template<class T>
	inline T *load(collection &c, const string &name)
	{
		return load<T>(&c, name);
	}


	/*
		имеет смысл сделать классы - обертки на загрузку и исползование ресурсов
		по типу dobject и pointer
	*/




	//===============================================================
	//	общий набор ресурсов, необходимый mpp дл€ работы
	//===============================================================
	class common
		:public collection
		,public static_class<common>
	{
	public:
		font		*font;
		image		button_active,
					button_passive;
		sound		*click;
	
	private:
		xml			strings;
		animation	*p_plus,	// дл€ ускорени€ диалогов их повесим в кэш на все врем€ работы
					*p_minus,
					*p_left,
					*p_right,
					*p_up,
					*p_down;

		animation	*load_always(const string &name);

	public:
		common(const string &name, const string &category);
		~common();
		static	const string &str(const string &name);
	};
}//mpp
#endif