/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <Mmreg.h>
#include <prerelease/dsound.h>

#include <direct.h>
#include <io.h>
#include <windowsx.h>
#include <time.h>

#include <fstream>
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::ios;

#include "..\\..\\..\\common\\sources\\all.h"
using namespace mpp;

#define		IDI_AC          129
#define		LOG_NAME		"win32.log"
#define		GAME_TIMER_ID	1000
#define		WINDOW_CLASS	"MPP for Win32"
#define		MSG(str)		MessageBox(NULL, str, "Message", MB_OK);


//========================================================================
//	Интерфейс для Win32API
//========================================================================
class win32 : public platform
{
private:
	xml			prefs;
	HINSTANCE	hInst;								// current instance
	HWND		hWnd;
	HBITMAP		scr;
	CRITICAL_SECTION	msg_sync;
	int			no_sound;
	size<>		scr_size;

	ATOM	register_class();
	void	init_instance(int);
	static	LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT wnd_proc(HWND hWnd, UINT, WPARAM, LPARAM);
	void	paint(HDC hdc);
	void	save_window_rect();
	image	*create_screen();

public:
	//---------------------------------------------------------------
	// local
	win32(HINSTANCE hInstance, int nCmdShow);
	virtual ~win32();
	int loop();
	
	//------------------------------------------------------------------
	// common interface
	
	void close()
	{
		SendMessage(hWnd, WM_CLOSE, 0, 0);
	}

	void srand() const;
	int rand(int max) const;

	string key_name(int key);

	bool find_files(const string &path, list<string> &names, bool dirs) const;
	bool remove_file(const string &path, bool dirs=0) const;
	bool mkdir(const string &path) const;

	void message(const string &s) const;
	bool confirm(const string &s) const;

	//=============================================================
	//	звук
public: class sound : public ::sound
	{
	private:
		LPDIRECTSOUNDBUFFER		buffer;
		static	LPDIRECTSOUND	ds_object;
			
	public:
		sound(riff_header *h, ::file *f);
		virtual ~sound();
		void play(int n_repeats=0);
		void stop();
		void change_volume(uint8 vol);
		static	void init(HWND hWnd);
	};
	::sound *load_sound(mem_file *f, bool sfx);

	//===================================================================
	// ф-ии необходимые для ogg decoder
	void	*realloc(void **p, int size);

	//=============================================================
	//	файловый ввод/вывод
public: class file : public mpp::file
	{
	private:
		FILE	*f;

	public:
		file(const string &name, bool for_read=true, const string &path="");
		virtual ~file();
		void pos_set(int pos);		// позиция с начала
		void pos_shift(int pos);	// позиция с тек. позиции
		int	 size();
		int  tell();
		bool eof();
	protected:
		int read_buffer(void *buffer, int size);
		void write_buffer(const void *buffer, int size);
	};
	mpp::file *file_open(const string &name, bool for_read=true, const string &path="") const;

	string	get_id();
};
