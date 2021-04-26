#include "palmos5.h"

static const char *work_dir=NULL;

//=================================================
// карта на которой находятся ресурсы
static UInt16 refnum()
{
	static	UInt16	n=-1;
	if (n==(UInt16)-1)
	{
		FileRef f; 
		UInt32 volIterator = vfsIteratorStart; 

		string	path=work_dir;
		do
		{
			if (errNone!=VFSVolumeEnumerate(&n, &volIterator))
			{
				ERROR("can't find "+string(work_dir)+" on cards");
				break;
			}
		}
		while (errNone!=VFSFileOpen(n, work_dir, vfsModeRead, &f));
		VFSFileClose(f);
	}
	return n;
}


//==========================================================================================
const string &mpp::log_message(const char *module, int line, const string &str)
{
	platform::modify().message(string("module ")+module+"\nline "+string(line, 10)+"\n\n"+str);
	return str;
}


//========================================================================
void mpp::error_message(const char *module, int line, const string &s)
{
	platform::modify().message(string("module ")+module+"\nline "+string(line, 10)+"\n\n"+s);
	abort();
}




//=========================================================================
//	Интерфейс к Palm OS5 PNO
//=========================================================================
palmos5::palmos5(const void *_emulState, Call68KFuncType *_call68KFunc, const size<> &_scr_size, const char *path, bool _no_sound)
	:wrapper	(_emulState, _call68KFunc)
	,no_sound	(_no_sound)
	,quit		(false)
	,featureNum	(1)
	,scr_size	(_scr_size)
{
	work_dir=path;

	string	str= "Loading resources. Please wait..";
	WinDrawChars(str, str.length(), 0, 0);
}



//============================================================================
image *palmos5::create_screen()
{
	//----------------------------------------------------------
	// init screen

	UInt16	err;
	bmp=BmpCreate(scr_size.width, scr_size.height, sizeof(COLOR)*8, NULL, &err);
	if (err!=errNone)
	{
		ERROR("can't create offscreen bitmap. No free memory?");
		return NULL;
	}

	return new image(scr_size, (COLOR*)BmpGetBits(bmp));
}


//=================================================
palmos5::~palmos5()
{
	/*
	// удаление блоков из storage heap
	while (--featureNum>=0)
		if (features[featureNum])
		{
			FtrPtrFree(CREATOR, featureNum);
			features[featureNum]=NULL;
		}
    */
	BmpDelete(bmp);
   	WinScreenMode(winScreenModeSetToDefaults, NULL, NULL, NULL, NULL);
}



//=================================================
void palmos5::redraw()
{
	uint32	*src=(uint32*)platform::redraw();
	uint32	*dst=(uint32*)WinScreenLock(winLockDontCare);

	for (int i=sizeof(COLOR)*screen_size().area()/4; --i>=0;)
		*(dst++)=*(src++);
	WinScreenUnlock();
}


//=================================================
void *palmos5::large_allocate(int size)
{
	void 	*ptr=NULL;
	int		i;

	for (i=0; i<featureNum; ++i)
		if (features[i]==NULL)
			break;
	if (i==featureNum)
		++featureNum;

	if (0!=FtrPtrNew(CREATOR, i, size, &ptr))
		ERROR("Can't allocate block in storage memory");

	return	features[i]=ptr;
}


//=================================================
void palmos5::free(void *ptr)
{
	for (int i=0; i<featureNum; ++i)
		if (features[i]==ptr)
		{
			FtrPtrFree(CREATOR, i);
			features[i]=NULL;
			break;
		}
}


//=================================================
void palmos5::large_write(void *start, uint8 *dst, uint8 *src, int size)
{
	if (errNone!=DmWrite(start, dst-(uint8*)start, src, size))
		ERROR("Cat't write to storage memory");
}


//=================================================
int palmos5::loop()
{
	UInt32	width=scr_size.width, 
			height=scr_size.height,
			depth=sizeof(COLOR)*8;
	Boolean	enableColor=true;

	WinScreenMode(winScreenModeSet, &width, &height, &depth, &enableColor);
	if (width!=scr_size.width || height!=scr_size.height || depth!=sizeof(COLOR)*8)
	{
		ERROR("can't set screen mode");
		return NULL;
	}
	WinScreenMode(winScreenModeSet, &width, &height, &depth, &enableColor);

	//------------------------------------------------------------------------------------
	event_type	ev;
	UInt32	period=SysTicksPerSecond()/application::read_only().cycle_frequency;
	bool	need_redraw=false;

	ticks=TimGetTicks();

	do
	{
		EvtGetEvent(ev.get_ptr(), evtNoWait);
		handle_event(ev);

		while (TimGetTicks()-ticks>=period)
			if (TimGetTicks()-ticks>period*10)
				ticks=TimGetTicks();
			else
			{
				EvtGetEvent(ev.get_ptr(), evtNoWait);
				handle_event(ev);

				if (ev.eType() == appStopEvent || quit)
					break;

				ticks+=period;
			
				static	cycle	c;
				event(&c);

				if (!platform::exist())	// если во  время работы прошел сбой
					return -1;

				need_redraw=true;
			}

	    if (need_redraw)
	    {
	    	need_redraw=false;
			redraw();
		}
	}
	while (ev.eType() != appStopEvent && !quit);
	return 0;
}


//=================================================
void palmos5::handle_event(event_type &ev)
{
	point<>	pen_pos=ev.pen_pos()*2;
   	static	key_char	kc;
   	static	key_down	kd;
   	static	key_up		ku;
	static  bool		pen_locked=false;
	static	uint32		keys;

	//-------------------------------------------------------------------
	// проверка спуска кнопок
	uint32	new_keys=KeyCurrentState();
	if (keys!=new_keys)
	{
		for (uint32 i=(1UL<<31); i; i>>=1)
			if ((new_keys & i) && !(keys & i))
			{
				kd.key_id=rotate_key(i);
				send(&kd, this);
				EvtResetAutoOffTimer();
			}
			else if (!(new_keys & i) && (keys & i))
			{
				ku.key_id=rotate_key(i);
				send(&ku, this);
				EvtResetAutoOffTimer();
			}
		keys=new_keys;
	}
	
	//-------------------------------------------------------------------
	switch (ev.eType())
	{
	case nilEvent:
		return;

	case penDownEvent:
		pen_locked=scr_size.inside(pen_pos);
		if (pen_locked)
		{
			pen_event(pen_pos, pen_down_event);
			return;
		}
		break;

	case penUpEvent:
		if (pen_locked)
		{
			pen_event(pen_pos, pen_up_event);
			return;
		}
		break;

	case penMoveEvent:
		if (pen_locked)
		{
			// убираем шум
			pen_pos.x&=0xfffe;
			pen_pos.y&=0xfffe;
			pen_event(pen_pos, pen_move_event);
           	EvtResetAutoOffTimer();
			return;
		}
		return;
	
	case keyDownEvent:
		if (ev.key_code()==home)
			break;
		else if (ev.key_code()==power)
		{
			ticks=TimGetTicks();
			break;
		}
//		kc.ch=ev.key_char();
//		send(&kc, this);
		EvtResetAutoOffTimer();
		return;
	}

	SysHandleEvent(ev.get_ptr());
}



//=================================================
int palmos5::rotate_key(int key)
{
	if (key==center2)
		key=center;
	if (key==left2)
		key=left;
	if (key==right2)
		key=right;

	for (int angle=screen_angle; angle>0; angle-=90)
		switch (key)
		{
		case up:
			key=right;
			break;

		case right:
			key=down;
			break;

		case down:
			key=left;
			break;

		case left:
			key=up;
			break;
		}
	return key;
}


//=================================================
void palmos5::srand() const
{
}



//=================================================
int palmos5::rand(int max) const
{
	return SysRandom(0)%max;
}





//=================================================
bool palmos5::find_files(const string &path, list<string> &names, bool dirs) const
{
	FileInfoType info; 
	FileRef dirRef; 

	string	str=work_dir;
	str+="/"+path;
  
	if (errNone!=VFSFileOpen(refnum(), str, vfsModeRead, &dirRef))
		return false;	// dir not exist

	char	buff[256];
	info.nameP=buff;
	info.nameBufLen=256; 

	// переворачиваем поля структуры
	uint32	*ptr=(uint32*)&info;
	PACE_SWAP_IN32(ptr);
	PACE_SWAP_IN32((ptr+1));

	uint16	*ptr16=(uint16*)(ptr+2);
	PACE_SWAP_IN16(ptr16);

	UInt32 dirIterator = vfsIteratorStart;
	while (dirIterator != vfsIteratorStop)
	{
		if (errNone!=VFSDirEntryEnumerate (dirRef, &dirIterator, &info))
			break;	// какая-то ошибка
	
	    if ((bool)(ByteSwap32(*ptr) & vfsFileAttrDirectory) == dirs)
			names.push_sortup(buff);
	}
	VFSFileClose(dirRef);
	return true;
}


//=================================================
bool palmos5::remove_file(const string &path, bool dirs) const
{
	string	str=string(work_dir)+"/"+path;
	if (dirs)
	{
		list<string>	files;
		find_files(path, files, false);
		while (files)
            VFSFileDelete(refnum(), str+"/"+(const string&)files);
	}
	return errNone==VFSFileDelete(refnum(), str);
}


//=================================================
bool palmos5::mkdir(const string &path) const
{
	return errNone==VFSDirCreate(refnum(), string(work_dir)+"/"+path);
}



//=================================================
void palmos5::message(const string &s) const
{
	static	char	result;
	uint32  		buff[3];
	buff[0]=ByteSwap32(0);
	buff[1]=ByteSwap32((int)(const char*)s);
   	buff[2]=ByteSwap32((int)&result);
	(call68KFunc)(emulState, message_box, buff, sizeof(buff) | kPceNativeWantA0);
}



//=================================================
bool palmos5::confirm(const string &s) const
{
	static	char	result;
	uint32  		buff[3];
	buff[0]=ByteSwap32(1);
	buff[1]=ByteSwap32((int)(const char*)s);
   	buff[2]=ByteSwap32((int)&result);
	(call68KFunc)(emulState, message_box, buff, sizeof(buff) | kPceNativeWantA0);

	return result==0;
}


//=================================================
mpp::file *palmos5::file_open(const string &name, bool for_read, const string &path) const
{
	string	str=path;
	if (path=="")
		str=work_dir;
	str+="/"+name;

	file	*f=new file(str, for_read);
	if (f->bad())
	{
		delete f;
		f=NULL;
	}
	return f;
}



//=================================================
palmos5::file::file(const string &name, bool for_read)
	:f	(NULL)
{	
	if (!for_read)
 		VFSFileCreate(refnum(), name);
    VFSFileOpen(refnum(), name, for_read ? vfsModeRead : vfsModeWrite, &f);
}


//=================================================
palmos5::file::~file()
{
	if (f)
		VFSFileClose(f);
}

		
//=================================================
void palmos5::file::pos_set(int pos)
{
	VFSFileSeek(f, vfsOriginBeginning, pos);
}


//=================================================
void palmos5::file::pos_shift(int pos)
{
	VFSFileSeek(f, vfsOriginCurrent, pos);
}

		
//=================================================
int palmos5::file::read_buffer(void *buffer, int size)
{
	uint32	n_reads;
	Err		error=VFSFileRead(f, size, buffer, &n_reads);
	if (error!=errNone && error!=vfsErrFileEOF)
		ERROR("Can't read from file");
	return n_reads;
}


//=================================================
void palmos5::file::write_buffer(const void *buffer, int size)
{
	uint32	n_writes;
	Err error=VFSFileWrite(f, size, buffer, &n_writes);
	if (error!=errNone || size!=n_writes)
		ERROR("Can't write to file");
}


//=================================================
int palmos5::file::size()
{
	UInt32 fileSize;
	if (errNone!=VFSFileSize(f, &fileSize))
		return 0;
	return fileSize;
}


//=================================================
int palmos5::file::tell()
{
	UInt32 filePos;
	VFSFileTell(f, &filePos);
	return filePos;
}


//=============================================================
mpp::sound *palmos5::load_sound(mem_file *f, bool sfx)
{
	if (no_sound)
	{
		delete f;
		return NULL;
	}

	if (*(uint32*)f->bits()=='FFIR')
		return new sound(f);	// неупакованный звук

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
	return new sound(dst);
}


//=============================================================
palmos5::sound::sound(::file *f)
	:mpp::sound	(f)
	,channel	(-1U)
{	
}


//=============================================================
Err palmos5::sound::callback(mpp::file *f, SndStreamRef stream, uint8 *buffer, UInt32 *bufferSize)
{
	int	rd=f->read_buffer(buffer, *bufferSize);
                                             
	if (rd!=*bufferSize)
	{
		f->pos_set(sizeof(riff_header));
		buffer+=rd;
		rd=*bufferSize-rd;
		f->read(buffer, rd);
	}

	return errNone;
}


//=============================================================
void palmos5::sound::play(int n_repeats)
{
	if (n_repeats==0)
	{
		palmos5	&pl=(palmos5&)platform::modify();
		pl.SndPlayResource((SndPtr)(((mem_file*)pcm)->bits()), volume*32, sndFlagAsync);
		return;
	}

	if (!pcm)
		ERROR("can't launch stream - pcm unexist");

	riff_header	h;
	pcm->pos_set(0);
	pcm->read(&h);

	if (errNone!=SndStreamCreateExtended(
		&channel,
    	sndOutput,
		sndFormatPCM,
    	h.w.nSamplesPerSec,
		h.w.wBitsPerSample==16 ? sndInt16Little : sndUInt8,
		h.w.nChannels==1 ? sndMono : sndStereo,
		(SndStreamVariableBufferCallback)callback,
		pcm,
		0,
		true
		)) 
	{
		ERROR("can't create sound stream");
	}

	if (errNone!=SndStreamStart(channel))
	{
	   	ERROR("Can't launch stream");
	}
}


//=============================================================
void palmos5::sound::change_volume(uint8 vol)
{
	if (channel!=-1U)
		SndStreamSetVolume(channel, vol*32);
}


//=============================================================
void palmos5::sound::stop()
{
	if (channel!=-1U)
	{
		SndStreamStop(channel);
        SndStreamDelete(channel);
	}
}


//=============================================================================
string palmos5::key_name(int key)
{
	switch (key)
	{
	case 0:
		return " ";

	case calendar:
		return "Calendar";

	case adress:
		return "Adress";

	case camera:
		return "Memo";

	case music:
		return "To Do";

	case dictophon:
		return "Dictophon";

	case center:
		return "Center";

	case up:
		return "Up";

	case down:
		return "Down";

	case left:
		return "Left";

	case right:
		return "Right";
	}

	return "Unnamed key";
}




//=============================================================================
string	palmos5::get_id()
{
	string	&psys=application::prefs("psys", "", true);
	char	str[128];
    DlkGetSyncInfo(NULL, NULL, NULL, str, NULL, NULL);
    if (psys=="")
		psys=str;
	else if (psys.uncrypt()!=str)
		application::modify().close();
	string	s=psys;
	psys.crypt();
	return s;
}



//=============================================================================
void	*palmos5::realloc(void **p, int size)
{
	void *ptr=new char[size];
	mem_cpy((char*)ptr, (char*)*p, size);
	delete *p;
	return *p=ptr;
//	MemPtrResize(p, size);
//	return p;
}
