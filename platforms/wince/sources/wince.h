/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _WINCE_H_
#define _WINCE_H_

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <Mmreg.h>

#include <windowsx.h>
#include <time.h>


#include "../../../engine/sources/all.h"
using namespace mpp;

#define		IDI_AC          129
#define		WINDOW_CLASS	L"MPP for PocketPC"
#define		WBUFFER_SIZE	1024

//========================================================================
//	Интерфейс для WinCE
//========================================================================
class wince : public platform
{
private:
	HINSTANCE	hInst;								// current instance
	HWND		hWnd;
	CRITICAL_SECTION	msg_sync;
	int			no_sound;
	size<>		scr_size;
	bool		dont_redraw;

	ATOM	register_class();
	void	init_instance(int);
	static	LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
	LRESULT wnd_proc(HWND hWnd, UINT, WPARAM, LPARAM);
	void	save_window_rect();
	image	*create_screen();
	void	redraw();
	BOOL	GetDeviceID( GUID* pGuid);
	bool	installation();

public:
	//---------------------------------------------------------------
	// local
	wince(HINSTANCE hInstance, int nCmdShow);
	virtual ~wince();
	int		loop();

	HWND	get_hwnd()
	{
		return hWnd;
	}
	
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
public: class sound : public mpp::sound
	{
	private:
		HANDLE			h;
		HWAVEOUT		hw;
		WAVEHDR			whd;
		
		static DWORD WINAPI snd_play(LPCTSTR bits);

	public:
		sound(mem_file *f, bool _sfx);
		virtual ~sound();
		void play(int n_repeats=0);
		void stop();
		void change_volume(uint8 vol);
	};
	mpp::sound *load_sound(mem_file *f, bool sfx);

	//===================================================================
	// ф-ии необходимые для ogg decoder
	void	*realloc(void **p, int size);

	//=============================================================
	//	файловый ввод/вывод
public: class file : public mpp::file
	{
	private:
		HANDLE	f;
		uint32	tell_pos;

	public:
		file(HANDLE _f);
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


#endif // _WINCE_H_
