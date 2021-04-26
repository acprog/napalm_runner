/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
// Win32.cpp : Defines the entry point for the application.
//

#include "Win32.h"
#include "../resources/resource.h"
#include "io.h"
#include <math.h>

LPDIRECTSOUND	win32::sound::ds_object=NULL;

//==========================================================================================
const string &mpp::log_message(const char *module, int line, const string &str)
{
	fstream	f(LOG_NAME, ios::out|ios::app);

	static	char	tbuffer[32];
	_strtime( tbuffer );

	f << tbuffer << "\t\t\t\tfile: " << module << "\tline: " << line << "\n\t" << (const char*)str << "\n";
	return str;
}


//========================================================================
void mpp::error_message(const char *module, int line, const string &s)
{
	log_message(module, line, string("Fatal: ")+s);
	throw string("Fatal exception in:\n\tmodule ")+module+"\n\tline "+string(line, 10)+"\n\n\t"+s;
}





//==========================================================================================
int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
#ifndef _DEBUG
	try
	{
#endif
		remove(LOG_NAME);
		return platform::launch(new win32(hInstance, nCmdShow));
#ifndef _DEBUG
	}
	catch(const string &str)
	{
		MessageBox(NULL, (const char*)str, "Error", MB_OK|MB_ICONERROR|MB_TASKMODAL);
	}
#endif

	return -1;
}


//==================================================================
int win32::loop()
{
	//---------------------------------------------------------
	// обмен через bluetooth
	

	//---------------------------------------------------------

	SetWindowText(hWnd, application::get().name);

	MSG		msg;
	int		cycle_period=CLOCKS_PER_SEC/application::get().cycle_frequency;

	clock_t	timer=clock();
	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		while (clock()-timer>=cycle_period)
		{
			timer+=cycle_period;
			cycle	c;
			send(&c);
		}
		RECT	r;
		GetClientRect(hWnd, &r);
		InvalidateRect(hWnd, &r, false);
	}
	return (int) msg.wParam;
}



//=========================================================================================
win32::win32(HINSTANCE hInstance, int nCmdShow)
	:hWnd		(NULL)
	,scr		(NULL)
	,hInst		(hInstance)
	,prefs		("win32_prefs", file_open("win32.xml"))
	,no_sound	(prefs("sound", "yes", true)!="yes")
	,scr_size	(prefs("window/width", "640", true).atoi(), prefs("window/height", "480", true).atoi())
{
	scr_size.width&=0xfffe0;
	scr_size.height&=0xfffe0;

	register_class();
	init_instance(nCmdShow);

	InitializeCriticalSection(&msg_sync);

	if (!no_sound)
	{
		try
		{
			sound::init(hWnd);
		}
		catch(...)
		{
			message("Can't set CooperativeLevel in Direct Sound.\nContinue witchout sound.");
			no_sound=true;
		}
	}
}



//=========================================================================================
image *win32::create_screen()
{
	scr=CreateCompatibleBitmap(GetDC(hWnd), scr_size.width, scr_size.height);
	BITMAP	bmp;
	GetObject(scr, sizeof(BITMAP), &bmp);
	if (bmp.bmBitsPixel!=16 && bmp.bmBitsPixel!=32)
		ERROR(string("Incorrect screen depth (")+string(bmp.bmBitsPixel, 10)+"). Need 16 or 32 bps");
	
	return new image(scr_size);
}



//=========================================================================================
win32::~win32()
{
	DeleteObject(scr);
	DeleteCriticalSection(&msg_sync);
	
	string	s;
	prefs.write(s);
	mpp::file *f=file_open("win32.xml", false);
	f->write((const char*)s, s.length());
	delete f;
}


//=========================================================================================
ATOM win32::register_class()
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= LoadIcon(hInst, (LPCTSTR)IDI_AC);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= WINDOW_CLASS;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_AC);

	return RegisterClassEx(&wcex);
}




//=========================================================================================
void win32::init_instance(int nCmdShow)
{
	RECT	r={0, 0, scr_size.width, scr_size.height};
	AdjustWindowRectEx(&r, WS_CAPTION|WS_SYSMENU, false, NULL);

	r.right+=prefs("window/x", "0", true).atoi()-r.left;
	r.bottom+=prefs("window/y", "0", true).atoi()-r.top;
	r.left=prefs("window/x").atoi();
	r.top=prefs("window/y").atoi();

	hWnd = CreateWindow(WINDOW_CLASS, "MPP", WS_CAPTION | WS_SYSMENU, 
		r.left, r.top, r.right-r.left, r.bottom-r.top, NULL, NULL, hInst, NULL);

	if (!hWnd)
		ERROR("Can't Create window");

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
}




//=========================================================================================
LRESULT CALLBACK win32::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (platform::exist())
        return	((win32*)&platform::get())->wnd_proc(hWnd, message, wParam, lParam);
	else
		return DefWindowProc(hWnd, message, wParam, lParam);
}



//=========================================================================================
LRESULT win32::wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT			ps;
	HDC					hdc;

	static	key_down	kd;
	static	key_up		ku;
	static	key_char	kc;

	static	int	prev_key=0;

	if (!application::exist())
		return DefWindowProc(hWnd, message, wParam, lParam);

#ifndef _DEBUG
	try
	{
#endif
		EnterCriticalSection(&msg_sync);
		switch (message) 
		{
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			paint(hdc);
			EndPaint(hWnd, &ps);
			break;

		case WM_TIMER:
			/*
			if (wParam==GAME_TIMER_ID)
			{
				current_app->timer();
				GetClientRect(hWnd, &r);
				InvalidateRect(hWnd, &r, false);
			}
			*/
			break;

		case WM_CLOSE:
//			KillTimer(hWnd, GAME_TIMER_ID);
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;

		case WM_DISPLAYCHANGE:
			break;

		case WM_DESTROY:
			save_window_rect();
			PostQuitMessage(0);
			break;

		case WM_CHAR:
			kc.ch=wParam;
			send(&kc);
			break;

		case WM_KEYDOWN:
			if (wParam!=prev_key)
			{
				prev_key=wParam;
				kd.key_id=wParam;
				send(&kd);
			}
			break;

		case WM_KEYUP:
			prev_key=0;
			ku.key_id=wParam;
			send(&ku);
			break;

		case WM_LBUTTONDOWN:
			pen_event(point<>(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)), pen_down_event);
			break;

		case WM_LBUTTONUP:
			pen_event(point<>(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)), pen_up_event);
			break;

		case WM_MOUSEMOVE:
			if (wParam & MK_LBUTTON)
				pen_event(point<>(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)), pen_move_event);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		LeaveCriticalSection(&msg_sync);
#ifndef _DEBUG
	}//try
	catch(const string &str)
	{
//		KillTimer(hWnd, GAME_TIMER_ID);
		MessageBox(NULL, (const char*)str, "Error", MB_OK|MB_ICONERROR|MB_TASKMODAL);
		SendMessage(hWnd, WM_DESTROY, 0, 0);
	}
#endif

	return 0;
}


//================================================================================
void	win32::save_window_rect()
{
	RECT	r;
	GetWindowRect(hWnd, &r);
	prefs("window/x")=string(r.left, 10);
	prefs("window/y")=string(r.top, 10);
	GetClientRect(hWnd, &r);
	prefs("window/width")=string(r.right, 10);
	prefs("window/height")=string(r.bottom, 10);
}



//================================================================================
void win32::paint(HDC hdc)
{
	COLOR *bits=redraw(true);

	BITMAP	bmp;
	GetObject(scr, sizeof(BITMAP), &bmp);
	if (bmp.bmBitsPixel==16)
		SetBitmapBits(scr, scr_size.area()*sizeof(COLOR), bits);
	else
	{// преобразовнаие точек 16 bit>>32
		static	uint32	*buff=new uint32[scr_size.area()];
		color32 pixel,
				*dst=(color32*)buff;
		COLOR	*src=bits;
		static COLOR j=0;
		for (int i=scr_size.area(); --i>=0; dst++, src++)
			*dst=*src;
		SetBitmapBits(scr, scr_size.area()*4, buff);
	}

	static	HDC	hCompatibleDC=CreateCompatibleDC(hdc);
	SelectObject(hCompatibleDC, scr);

	BitBlt(	hdc, 0, 0, scr_size.width, scr_size.height, 
			hCompatibleDC, 0, 0, SRCCOPY);
}


//=============================================================
//	файловый ввод/вывод
win32::file::file(const string &name, bool for_read, const string &path)
	:f	(NULL)
{
	if (for_read)
		if (access(name, 0)==-1)
			throw "Can't find file "+name;

	f=fopen(name, for_read ? "rb" : "wb");
}


//=============================================================
win32::file::~file()
{
	fclose(f);
}


//=============================================================
int win32::file::read_buffer(void *buffer, int size)
{
	return fread(buffer, 1, size, f);
    return size;
}

//=============================================================
void win32::file::write_buffer(const void *buffer, int size)
{
	fwrite(buffer, size, 1, f);
}

//=============================================================
void win32::file::pos_set(int pos)
{
	fseek(f, pos, SEEK_SET);
}

//=============================================================
void win32::file::pos_shift(int pos)
{
	fseek(f, pos, SEEK_CUR);
}

//=============================================================
int win32::file::size()
{
	return _filelength(_fileno(f));
}

//=============================================================
int win32::file::tell()
{
	return ftell(f);
}



//=============================================================
bool win32::file::eof()
{
	return feof(f);
}


//=============================================================
mpp::file *win32::file_open(const string &name, bool for_read, const string &path) const
{
	string	str=name;
	file	*f;
	try
	{
		f=new file(str, for_read, path);
	}
	catch(...)
	{
		return NULL;
	}
	return f;
}



//=============================================================
//	звук
win32::sound::sound(riff_header *h, ::file *f)
{
	DSBUFFERDESC	dsbd;
	mem_set<char>((char*)&dsbd, sizeof(DSBUFFERDESC), 0);

	dsbd.dwSize=sizeof(DSBUFFERDESC);
	dsbd.dwBufferBytes=h->data_size;
	dsbd.lpwfxFormat=(WAVEFORMATEX*)&h->w;
	dsbd.dwFlags=DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;

	if (DS_OK!=ds_object->CreateSoundBuffer(&dsbd,  &buffer, NULL))
		ERROR("DirectSound: can't create buffer");

	void			*block1;
	DWORD			bytes_block1;
	void			*block2;
	DWORD			bytes_block2;
	if (DS_OK!=buffer->Lock(0, h->data_size, &block1, &bytes_block1, &block2, &bytes_block2, 0))
		ERROR("DirectSoundBuffer: cant't lock buffer");

	f->read_buffer(block1, h->data_size);
	delete f;
	buffer->Unlock(block1, bytes_block1, block2, bytes_block2);
}



//=============================================================
win32::sound::~sound()
{
	buffer->Release();
}
 

//=============================================================
void win32::sound::play(int n_repeats)
{
	buffer->SetCurrentPosition(0);
	change_volume(volume);
	buffer->Play(0, 0, n_repeats==sound::infinite ? DSBPLAY_LOOPING : 0);
}

//=============================================================
void win32::sound::change_volume(uint8 s_volume)
{
	buffer->SetVolume(0);
	buffer->SetVolume((s_volume-80)*50);
}


//=============================================================
void win32::sound::init(HWND hWnd)
{
	if (DS_OK!=DirectSoundCreate(NULL, &ds_object, NULL))
		ERROR("Can't create DirectSound object");
	if (DS_OK!=ds_object->SetCooperativeLevel(hWnd, DSSCL_NORMAL))
		ERROR("Can't set Cooperative Level in Direct Sound");
}	


//=============================================================
::sound *win32::load_sound(mem_file *f, bool sfx)
{
	if (no_sound)
	{
		delete f;
		return NULL;
	}

	riff_header	h;
	if (*(uint32*)f->bits()=='FFIR')	
	{	// неупакованный звук
		f->read(&h);
        if (h.w.wFormatTag!=WAVE_FORMAT_PCM)
		{
			LOG("invalid wave format. Need PCM");
			delete f;
			return NULL;
		}
		return new sound(&h, f);
	}

    ogg	*o=new ogg(f);
	o->read(&h);
	return new sound(&h, o);
}


//===============================================================
void win32::sound::stop()
{
	buffer->Stop();
}


//===============================================================
void win32::srand() const
{
//	::srand(clock());
	int	v=time(NULL);
	for (int i=0; i<100; i++)
	{
		::srand(v);
		v=::rand();
	}
}



//===============================================================
int win32::rand(int max) const
{
	return ::rand()%max;
}



//=========================================================================================
bool win32::find_files(const string &_path, list<string> &names, bool dirs) const
{
	string	path=_path+"/*";
	_finddata_t	fileinfo;
	intptr_t	i=_findfirst(path, &fileinfo); 
	
	if (i==-1)
		return false;
	
	while (_findnext(i, &fileinfo)==0)
		if (dirs==(bool)(fileinfo.attrib & _A_SUBDIR))
			names.push_sortup(fileinfo.name);

	_findclose(i);
	return true;
}


//=========================================================================================
bool win32::remove_file(const string &path, bool dir) const
{
	if (dir)
	{
		list<string>	files;
		find_files(path, files, false);
		while (string *s=files.each_ptr())
            remove(path+"/"+*s);
		return 0==rmdir(path);
	}
	return 0==remove(path);
}


//=========================================================================================
bool win32::mkdir(const string &path) const
{
	return 0==_mkdir(path);
}


//=========================================================================================
void win32::message(const string &s) const
{
	MessageBox(hWnd, s, "", MB_OK|MB_ICONINFORMATION);
}

//=========================================================================================
bool win32::confirm(const string &s) const
{
	return MessageBox(hWnd, s, "", MB_YESNO|MB_ICONQUESTION)==IDYES;
}



//=============================================================================
string win32::key_name(int key)
{
	if (key==0)
		return " ";

	const	char	*unnamed="unnamed key";
	
	if (key<0x8 || key>0x91)
		return unnamed;

	static	const char *keys[]=
	{
		"BACKSPACE",
		"TAB",
		unnamed,
		unnamed,
		unnamed,
		"ENTER",
		unnamed,
		unnamed,
		"SHIFT",
        "CTRL",
        "ALT",
		"PAUSE",
		"CAPS LOCK",
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		"ESC",
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		"SPACEBAR",
		"PAGE UP",
		"PAGE DOWN",
		"END",
		"HOME",
		"LEFT ARROW",
		"UP ARROW",
		"RIGHT ARROW",
		"DOWN ARROW",
		"SELECT",
		"PRINT",
		"EXECUTE",
		"PRINT SCREEN",
		"INS",
		"DEL",
		"HELP",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
		"Left Windows",
		"Right Windows",
		"Applications",
		unnamed,
		unnamed,
		"Numeric keypad 0",
		"Numeric keypad 1",
		"Numeric keypad 2",
		"Numeric keypad 3",
		"Numeric keypad 4",
		"Numeric keypad 5",
		"Numeric keypad 6",
		"Numeric keypad 7",
		"Numeric keypad 8",
		"Numeric keypad 9",
		"Multiply",
		"Add",
		"Separator",
		"Subtract",
		"Decimal",
		"Divide",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		unnamed,
		"NUM LOCK",
		"SCROLL LOCK",
	};
	return keys[key-0x8];
}


//=============================================================================
string	win32::get_id()
{
	return "win32";
}



//=============================================================================
void	*win32::realloc(void **p, int size)
{
	return ::realloc(*p, size);
}