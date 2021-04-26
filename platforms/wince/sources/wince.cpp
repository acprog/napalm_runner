/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
// test_ppc.cpp : Defines the entry point for the application.
//


#include "wince.h"
#include <commctrl.h>
#include <aygshell.h>
#include <sipapi.h>
#include <math.h>
#include <gx.h>
#include <uniqueid.h>

#include "../resources/resource.h"


static	wchar_t	wchars_buffer[WBUFFER_SIZE];
static	string	work_dir="/runner/";


//==========================================================================================
wchar_t *to_unicode(const string &str)
{
	MultiByteToWideChar(
		CP_ACP, 
		0, 
		str, 
		-1,
		wchars_buffer,
		WBUFFER_SIZE);
	return wchars_buffer;
}


//==========================================================================================
char *from_unicode(const wchar_t *str)
{
	char *chars_buffer=(char*)wchars_buffer;
	WideCharToMultiByte(
		CP_ACP, 
		0, 
		str, 
		-1,
		chars_buffer,
		WBUFFER_SIZE*2,
		NULL,
		NULL);
	return chars_buffer;
}


//==========================================================================================
const string &mpp::log_message(const char *module, int line, const string &str)
{
	MessageBox(((wince&)platform::modify()).get_hwnd(), to_unicode(string("file: ")+module+"\tline: "+line+"\n\t"+(const char*)str), L"Message", MB_OK);
	return str;
}


//========================================================================
void mpp::error_message(const char *module, int line, const string &s)
{
	log_message(module, line, string("Fatal: ")+s);
	application::erase();
	platform::erase();
	exit(1);
}



//==========================================================================================
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	wchar_t	path[1024];
	GetModuleFileName(NULL, path, 1024);
	work_dir=from_unicode(path);
	work_dir.replace("winmobile.exe", "");
	work_dir.replace("\\", "/");
	work_dir+="/";
  	
	return platform::launch(new wince(hInstance, nCmdShow));
}



//==================================================================
void wince::redraw()
{
	COLOR	*bits=platform::modify().redraw(false);

	#define GETRAWFRAMEBUFFER   0x00020001

	#define FORMAT_565 1
	#define FORMAT_555 2
	#define FORMAT_OTHER 3

	typedef struct _RawFrameBufferInfo
	{
	   WORD wFormat;
	   WORD wBPP;
	   VOID *pFramePointer;
	   int  cxStride;
	   int  cyStride;
	   int  cxPixels;
	   int  cyPixels;
	} RawFrameBufferInfo;

	RawFrameBufferInfo rfbi;
	HDC hdc = GetDC(NULL);
	ExtEscape(hdc, GETRAWFRAMEBUFFER, 0, NULL, sizeof(RawFrameBufferInfo), (char *) &rfbi);

	ReleaseDC(NULL, hdc);

	mem_cpy((uint32*)rfbi.pFramePointer, (uint32*)bits, scr_size.area()*sizeof(COLOR)/4);
}



//==================================================================
bool wince::installation()
{
	string		lnk_name="/Windows/Start Menu/Programs/Games/"+application::read_only().name+".lnk";
	wchar_t		path[1024];

	if (SHGetShortcutTarget(to_unicode(lnk_name), path, 1024))
	{
		wchar_t		curr_path[1024];
		GetModuleFileName(NULL, curr_path, 1024);
		path[wcslen(path)-1]=0;
		if (wcscmp(path+1, curr_path)==0)
			return false;
	}
	
	if (application::prefs("wm_install", "yes", true)=="yes")
	{
		switch (
			MessageBox(
				hWnd, 
				to_unicode("Install "+application::read_only().name+" "+application::read_only().get_version()+" into current folder ("+work_dir+")?"), 
				L"Installation", 
				MB_YESNOCANCEL|MB_ICONQUESTION))
		{
		case IDYES:
			CreateDirectory(L"/Windows/Start Menu/Programs/Games", NULL);
			DeleteFile(to_unicode(lnk_name));
			GetModuleFileName(NULL, path+1, 1022);
			path[0]='\"';
			path[wcslen(path)+1]=0;
			path[wcslen(path)]='\"';
			SHCreateShortcut(to_unicode(lnk_name), path);
			break;

		case IDNO:
			// никогда больше не спрашивать
			application::prefs("wm_install")="no";
			break;

		case IDCANCEL:
			return true;
		};
	}
	return false;
}


//==================================================================
int wince::loop()
{
	if (installation())
		return 0;

	MSG		msg;
	int		cycle_period=CLOCKS_PER_SEC/application::read_only().cycle_frequency;
	bool	need_redraw=false;

	clock_t	timer=GetTickCount();

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		while (GetTickCount()-timer>=cycle_period)
			if (GetTickCount()-timer>cycle_period*10)
				timer=GetTickCount();
			else
			{
				timer+=cycle_period;
				cycle	c;
				event(&c);
				need_redraw=true;
			}
		
		if (need_redraw && !dont_redraw)
		{
			SetForegroundWindow(hWnd);
			SetActiveWindow(hWnd);
			SHFullScreen(hWnd, SHFS_HIDETASKBAR|SHFS_HIDESIPBUTTON|SHFS_HIDESTARTICON);

			need_redraw=false;
			redraw();
		}
	}
	return (int) msg.wParam;
}



//=========================================================================================
wince::wince(HINSTANCE hInstance, int nCmdShow)
	:hWnd		(NULL)
	,hInst		(hInstance)
	,dont_redraw(false)
{
	scr_size.width=GetSystemMetrics(SM_CXSCREEN);
	scr_size.height=GetSystemMetrics(SM_CYSCREEN);

	register_class();
	init_instance(nCmdShow);

	InitializeCriticalSection(&msg_sync);
}



//=========================================================================================
image *wince::create_screen()
{
	return new image(scr_size);
}



//=========================================================================================
wince::~wince()
{
	DeleteCriticalSection(&msg_sync);
}


//=========================================================================================
ATOM wince::register_class()
{
	WNDCLASS wcex;

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= 0;
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= WINDOW_CLASS;

	return RegisterClass(&wcex);
}




//=========================================================================================
void wince::init_instance(int nCmdShow)
{
	hWnd = CreateWindow(WINDOW_CLASS, L"MPP", WS_VISIBLE,// | WS_BORDER,
		0, 0, scr_size.width, scr_size.height, NULL, NULL, hInst, NULL);

	if (!hWnd)
	{
		ERROR("Can't Create window");
		return;
	}

	SHFullScreen(hWnd, SHFS_HIDETASKBAR|SHFS_HIDESIPBUTTON|SHFS_HIDESTARTICON);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	HDC		dc=GetDC(hWnd);
	RECT	r={0, 0, scr_size.width, scr_size.height};
	DrawText(dc, L"Loading resources. Please wait..", -1, &r, DT_CENTER|DT_VCENTER);

	GXOpenInput();
}




//=========================================================================================
LRESULT CALLBACK wince::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (platform::exist())
		return	((wince&)platform::modify()).wnd_proc(hWnd, message, wParam, lParam);
	else
		return DefWindowProc(hWnd, message, wParam, lParam);
}



//=========================================================================================
LRESULT wince::wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT			ps;
	HDC					hdc;

	static	key_down	kd;
	static	key_up		ku;
	static	key_char	kc;

	static	int	prev_key=0;

	if (!application::exist())
		return DefWindowProc(hWnd, message, wParam, lParam);

	EnterCriticalSection(&msg_sync);
	switch (message) 
	{
	case WM_CLOSE:
		dont_redraw=true;
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;

	case WM_DESTROY:
		GXCloseInput();
		PostQuitMessage(0);
		break;

	case WM_CHAR:
		kc.ch=wParam;
		send(&kc, this);
		break;

	case WM_SYSKEYDOWN:
		LOG("key="+(int)wParam+", l="+(int)lParam);
		break;

	case WM_KEYDOWN:
		if (wParam==VK_LWIN)
			break;
		
		if (wParam!=prev_key)
		{
			prev_key=wParam;
			kd.key_id=wParam;
			send(&kd, this);
		}
		break;

	case WM_KEYUP:
		if (wParam==VK_LWIN)
			break;

		prev_key=0;
		ku.key_id=wParam;
		send(&ku, this);
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

	return 0;
}




//=============================================================
//	файловый ввод/вывод
wince::file::file(HANDLE _f)
	:f			(_f)
	,tell_pos	(0)
{
}


//=============================================================
wince::file::~file()
{
	CloseHandle(f);
}


//=============================================================
int wince::file::read_buffer(void *buffer, int size)
{
	DWORD	rd;
	ReadFile(f, buffer, size, &rd, NULL);
	tell_pos+=rd;
	return rd;
}

//=============================================================
void wince::file::write_buffer(const void *buffer, int size)
{
	DWORD	wd;
	WriteFile(f, buffer, size, &wd, NULL);
}

//=============================================================
void wince::file::pos_set(int pos)
{
	tell_pos=pos;
	SetFilePointer(f, pos, NULL, FILE_BEGIN);
}

//=============================================================
void wince::file::pos_shift(int pos)
{
	tell_pos+=pos;
	SetFilePointer(f, pos, NULL, FILE_CURRENT);
}

//=============================================================
int wince::file::size()
{
	return GetFileSize(f, NULL);
}

//=============================================================
int wince::file::tell()
{
	return tell_pos;
}



//=============================================================
bool wince::file::eof()
{
	return tell_pos>=size();
}


//=============================================================
mpp::file *wince::file_open(const string &name, bool for_read, const string &path) const
{
	HANDLE h=CreateFile(
		to_unicode(work_dir+name),
		for_read ? GENERIC_READ : GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		for_read ? OPEN_EXISTING : CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL); 
	if (h==INVALID_HANDLE_VALUE)
		return NULL;

	return new file(h);
}






//=============================================================
//	звук
wince::sound::sound(mem_file *f, bool sfx)
	:mpp::sound	(f)
	,hw			(NULL)
	,h			(NULL)
{
	if (sfx)
		h=CreateThread(
			NULL,
			1000,
			(LPTHREAD_START_ROUTINE)snd_play,
			const_cast<uint8*>(f->bits()),
			CREATE_SUSPENDED,
			NULL);
	else
	{
		riff_header	rh;
		f->read(&rh);
		MMRESULT	r=
		waveOutOpen(
			&hw, 
			WAVE_MAPPER, 
			(LPWAVEFORMATEX)&rh.w,
			NULL,
			0,
			CALLBACK_NULL);
		if (r!=MMSYSERR_NOERROR)
			ERROR("waveOutOpen");
	}
}


//=============================================================
DWORD WINAPI wince::sound::snd_play(LPCTSTR bits)
{	
	for(;;)
	{
		sndPlaySound(bits, SND_SYNC|SND_MEMORY|SND_NODEFAULT);
		SuspendThread(GetCurrentThread()); 		
	}
	return 0;
}


//=============================================================`
wince::sound::~sound()
{
	stop();
	if (h)
	{
		TerminateThread(h, 0);
		h=NULL;
	}
	else if (hw)
	{
		MMRESULT	r;
		r=waveOutBreakLoop(hw);
		if (r!=MMSYSERR_NOERROR)
			ERROR("waveOutBreakLoop");
		r=waveOutReset(hw);
		if (r!=MMSYSERR_NOERROR)
			ERROR("waveOutReset");
		r=waveOutUnprepareHeader(
			hw, 
			&whd, 
			sizeof(whd)); 
		if (r!=MMSYSERR_NOERROR)
			ERROR("waveOutunPrepareHeader");
		waveOutClose(hw);	
		if (r!=MMSYSERR_NOERROR)
			ERROR("waveOutClose");
		hw=NULL;
	}
}


//=============================================================
void wince::sound::play(int n_repeats)
{
	if (h)
	{
		static	 int prev_sfx_volume=-1;
		if (prev_sfx_volume!=volume)
		{
			waveOutSetVolume(NULL, volume*0xffff/100);
			prev_sfx_volume=volume;
		}
		ResumeThread(h);
	}
	else if (hw)
	{
		whd.lpData=(char*)(const_cast<uint8*>(((mem_file*)pcm)->bits()+sizeof(riff_header)));
		whd.dwBufferLength=pcm->size()-sizeof(riff_header);
		whd.dwFlags=WHDR_BEGINLOOP|WHDR_ENDLOOP;
		whd.dwLoops=10000;
		
		MMRESULT	r;
		r=waveOutPrepareHeader(
			hw, 
			&whd, 
			sizeof(whd)); 
		if (r!=MMSYSERR_NOERROR)
			ERROR("waveOutPrepareHeader");

		r=waveOutWrite(
			hw, 
			&whd, 
			sizeof(whd));
		if (r!=MMSYSERR_NOERROR)
			ERROR("waveOutWrite");
	}	
}

//=============================================================
void wince::sound::change_volume(uint8 s_volume)
{
	waveOutSetVolume(NULL, sound::volume*0xffff/100);
	if (hw)
	{
		MMRESULT	r;
		r=waveOutSetVolume(hw, s_volume*0xffff/100);
		if (r!=MMSYSERR_NOERROR)
			ERROR("waveOutSetVolume");
	}
}


//===============================================================
void wince::sound::stop()
{
	/*
	if (hw)
	{
		MMRESULT	r;
		r=waveOutClose(hw);
		if (r!=MMSYSERR_NOERROR)
			ERROR("waveOutClose");
		hw=NULL;
	}
	*/
}


//=============================================================
mpp::sound *wince::load_sound(mem_file *f, bool sfx)
{
	if (no_sound)
	{
		delete f;
		return NULL;
	}

	if (*(uint32*)f->bits()=='FFIR')
		return new sound(f, sfx);	// неупакованный звук

    ogg	*o=new ogg(f);

	riff_header	h;
	o->read(&h);

	mem_file	*dst=new mem_file(o->size());
    dst->write(&h);
    uint8	*tmp=new uint8[BUFFER_SIZE];
    int		rd;
	while ( (rd=o->read_buffer(tmp, BUFFER_SIZE)) >0 )
		dst->write(tmp, rd);

	delete tmp;
	delete o;
	dst->pos_set(0);
	return new sound(dst, sfx);
}




//===============================================================
void wince::srand() const
{
	int	v=GetTickCount();
	for (int i=0; i<100; i++)
	{
		::srand(v);
		v=::rand();
	}
}



//===============================================================
int wince::rand(int max) const
{
	return ::rand()%max;
}



//=========================================================================================
bool wince::find_files(const string &_path, list<string> &names, bool dirs) const
{
	string	path=work_dir+_path+"/*.*";
	WIN32_FIND_DATA fileinfo;
	HANDLE	i=FindFirstFile(to_unicode(path), &fileinfo);
	
	if (i==INVALID_HANDLE_VALUE)
		return false;
	

	do
		if (dirs==(bool)(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			names.push_sortup(from_unicode(fileinfo.cFileName));
	while (FindNextFile(i, &fileinfo));

	FindClose(i);
	return true;
}


//=========================================================================================
bool wince::remove_file(const string &path, bool dir) const
{
	if (dir)
	{
		list<string>	files;
		find_files(path, files, false);
		while (files)
			DeleteFile(to_unicode(work_dir+path+"/"+files));
		return RemoveDirectory(to_unicode(work_dir+path));
	}
	return DeleteFile(to_unicode(work_dir+path));
}


//=========================================================================================
bool wince::mkdir(const string &path) const
{
	return CreateDirectory(to_unicode(work_dir+path), NULL);
}


//=========================================================================================
void wince::message(const string &s) const
{
	MessageBox(hWnd, to_unicode(s), L"", MB_OK|MB_ICONINFORMATION);
}

//=========================================================================================
bool wince::confirm(const string &s) const
{
	return MessageBox(hWnd, to_unicode(s), L"", MB_YESNO|MB_ICONQUESTION)==IDYES;
}



//=============================================================================
string wince::key_name(int key)
{
	if (key==0)
		return " ";

	const	char	*unnamed="unnamed key";
	
	if (key>=0xc0)
		return	"Button #"+string(key-0xc0, 10);

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
		"Center",
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
BOOL wince::GetDeviceID( GUID* pGuid)
{
	BOOL fRes;
    DWORD dwBytesReturned =0;
	DEVICE_ID* pDevID;
	int wSize;

	if (NULL == pGuid)
		return FALSE;

	memset(pGuid, 0, sizeof(GUID));

	pDevID = (DEVICE_ID*)malloc(sizeof(DEVICE_ID));
	memset(pDevID, 0, sizeof(DEVICE_ID));
	pDevID->dwSize = sizeof(DEVICE_ID);

   fRes = KernelIoControl( IOCTL_HAL_GET_DEVICEID, NULL, 0,
            pDevID, sizeof( DEVICE_ID ), &dwBytesReturned );

	wSize = pDevID->dwSize;
	free(pDevID);
	if( (FALSE != fRes) || (ERROR_INSUFFICIENT_BUFFER != GetLastError()))
		return FALSE;

	pDevID = (DEVICE_ID*)malloc(wSize);
	memset(pDevID, 0, sizeof(wSize));
	pDevID->dwSize = wSize;
    fRes = KernelIoControl( IOCTL_HAL_GET_DEVICEID, NULL, 0,
            pDevID, wSize, &dwBytesReturned );

	if((FALSE == fRes) || (ERROR_INSUFFICIENT_BUFFER == GetLastError()) )
		return FALSE;

	BYTE* pDat = (BYTE*)&pGuid->Data1;
	BYTE* pSrc = (BYTE*)(pDevID) + pDevID->dwPresetIDOffset;
	memcpy(pDat, pSrc, pDevID->dwPresetIDBytes);
	pDat +=  pDevID->dwPresetIDBytes;
	pSrc =  (BYTE*)(pDevID) + pDevID->dwPlatformIDOffset;
	memcpy(pDat, pSrc, pDevID->dwPlatformIDBytes);

	return true;
}



//=============================================================================
string	wince::get_id()
{
	/*
	GUID	guid;
	if (!GetDeviceID(&guid))
		return "wince";

	string	res=string((int)guid.Data1, 16)+string((int)guid.Data2, 16)+string((int)guid.Data3, 16)+string((int)guid.Data4, 16);
	/**/
	int		guid[4]; 
	DWORD	bytesReturned; 

	KernelIoControl(IOCTL_HAL_GET_DEVICEID, NULL, 0, guid, 16, &bytesReturned);
	string	res=string(guid[0], 16)+string(guid[1], 16)+string(guid[2], 16)+string(guid[3], 16);
	
	return res;

	/*
	p/n s26391-k151-v125
	s/n yehd019176
	m/n 10600405478
	vendor: th447de00709
	
	3138820b40dd81062a0efab4
	*/
}



//=============================================================================
void	*wince::realloc(void **p, int size)
{
	return ::realloc(*p, size);
}
