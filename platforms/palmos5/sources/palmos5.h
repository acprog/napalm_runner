/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _PALM_OS5_
#define _PALM_OS5_

#include "pnomain.h"
#include "wrapper.h"


#define	CREATOR	'ACln'


//=========================================================================
//	Интерфейс к Palm OS5 PNO
//=========================================================================
class palmos5
	:public wrapper
	,public platform
{
private:
	bool 				quit;
	bool				no_sound;
	BitmapType			*bmp;
	size<>				scr_size;
   	UInt32				ticks;	// время в цикле

	// для выделение памяти из storage heap
	int					featureNum;
	void				*features[1024];	// указатели на выделенные блоки памяти

	void 		handle_event(event_type &event);
	void		redraw();

	enum key_bits
	{
		calendar=0x8,
		adress=0x10,
		camera=0x40,
		music=0x20,
		dictophon=0x2000,
		center=0x4000000,
		center2=0x100000,
		up=0x2,
		down=0x4,
		left=0x1000000,
		left2=0x40000,
		right=0x2000000,
		right2=0x80000
	};
	enum key_codes
	{
		clock=1,
		calculator=11,
		find=10,
		menu=8,
		light=2,
		home=8,
		power=30,
	};

protected:
	void *large_allocate(int size);
	void free(void *p);
	void large_write(void *start, uint8 *dst, uint8 *src, int size);
	image	*create_screen();

public:
	//---------------------------------------------------------------
	// local
	palmos5(const void *_emulState, Call68KFuncType *_call68KFunc, const size<> &screen_size, const char *work_dir, bool no_sound);
	int loop();
	~palmos5();
	
	//------------------------------------------------------------------
	// common interface

	void close()
	{
		quit=true;
	}

	void srand() const;
	int rand(int max) const;

	int rotate_key(int key);
	string key_name(int key);
	string	get_id();

	bool find_files(const string &path, list<string> &names, bool dirs) const;
	bool remove_file(const string &path, bool dirs=0) const;
	bool mkdir(const string &path) const;

	void message(const string &s) const;
	bool confirm(const string &s) const;

	//===================================================================
	// ф-ии необходимые для ogg decoder
	void	*realloc(void **p, int size);


	//=============================================================
	//	файловый ввод/вывод
public: class file : public mpp::file
	{
	private:
		FileRef	f;

	public:
		file(const string &name, bool for_read=true);
		virtual ~file();
		void pos_set(int pos);		// позиция с начала
		void pos_shift(int pos);	// позиция с тек. позиции
		int	 size();
		int  tell();

		//----------------------------------------------------------------
		bool bad()
		{
			return f==NULL;
		}	

	protected:
		int read_buffer(void *buffer, int size);
		void write_buffer(const void *buffer, int size);
	};
	mpp::file *file_open(const string &name, bool for_read=true, const string &path="") const;

	//=============================================================
	//	звук
public: class sound : public ::sound
	{
	private:
		SndStreamRef	channel;
        static  Err callback(mpp::file *f, SndStreamRef stream, uint8 *buffer, UInt32 *bufferSize);
			
	public:
		sound(::file *f);
		void play(int n_repeats=0);
		void stop();
		void change_volume(uint8 vol);
	};
	::sound *load_sound(mem_file *f, bool sfx);
};

#endif