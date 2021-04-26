/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "collection.h"
#include "font.h"
#include "zip/unzip.h"
#include "sound.h"
#include "mem.h"

namespace mpp{

sound	*sound::empty=NULL;
uint8	sound::volume=50;
uint8	loop::volume=50;


//===================================================================
//	файл в архиве
//===================================================================
class zip : public file
{
private:
	unzFile &uf;
	int		file_size;

public:
	zip(unzFile &_uf)
		:uf	(_uf)
	{
		unz_file_info file_info;

		if (UNZ_OK!=unzGetCurrentFileInfo(uf,&file_info, NULL, 0,NULL,0,NULL,0))
			ERROR("can't get file info");
		file_size=file_info.uncompressed_size;
	}

	//====================================================================
	virtual ~zip()
	{
		unzCloseCurrentFile(uf);
	}

	//====================================================================
	static zip *open(unzFile &uf, const string &filename)
	{
		if (UNZ_OK!=unzLocateFile(uf, filename, 0))
			return NULL;

		if (UNZ_OK!=unzOpenCurrentFilePassword(uf, NULL))
		{
			LOG("locked zip");
			return NULL;
		}

		return new zip(uf);
	}

	//====================================================================
	static void list(unzFile uf, list<string> &names)
	{
		unz_global_info gi;

		if (UNZ_OK!=unzGetGlobalInfo (uf,&gi))
			ERROR("can't get zip info");

		char filename_inzip[256];
		unz_file_info file_info;

		for (int i=gi.number_entry; --i>=0; unzGoToNextFile(uf))
		{
			if (UNZ_OK!=unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0))
				ERROR("can't get file info");

			string s=filename_inzip;
			if (s.end()[-1]!='/')	// без каталогов
				names.push_sortup(s);
		}
	}

	//====================================================================
	int read_buffer(void *buffer, int size)
	{
		return unzReadCurrentFile(uf, buffer, size);
	}

	//==================================================================
	void pos_set(int pos)
	{
		pos_shift(pos-unztell(uf));
	}

	//==================================================================
	void pos_shift(int pos)
	{
		if (pos<0)
			ERROR("zip: negative shift on file");
		char *buff=new char[pos];
		read(buff, pos);
		delete buff;
	}
	
	//==================================================================
	int	 size()
	{
		return file_size;
	}

	//==================================================================
	int tell()
	{
		return unztell(uf);
	}

	//==================================================================
	void write_buffer(const void *buffer, int size)
	{
		ERROR("You can't write to zip");
	}
};






//===============================================================
collection::collection(const string &name, const string &category)
	:folder			(category+"/"+name+"/")
	,zip_file		(unzOpen(category+"/"+name+".zip"))
{
}


//===============================================================
collection::~collection()
{
	if (!cache.is_empty())
	{
		while (cache_record *c=cache.each_ptr())
		{
			LOG("memory leak: no free resource "+c->res_name);
				delete c->ptr;
		}
		DEBUG_ONLY(ERROR("cache: memory leak"));
	}

	while (sound *s=sounds.each())
		delete s;
	unzClose(zip_file);
}

//================================================================================
string collection::get_name() const
{
	return string(folder.find('/')+1, folder.end()-2);
}


//================================================================================
file *collection::file_open(const string &name, bool for_read) const
{
	// сначала ищем в каталоге
	file	*f=platform::get().file_open(folder+name, for_read);

	// ищем в архиве
	if (!f && zip_file && for_read)
		f=zip::open((unzFile&)zip_file, name);

	return f;
}


//================================================================================
COLOR *collection::load_image(const string &_name, size<> *image_size) const
{
	string name="graphics/"+_name;

	file	*f=file_open(name+".bmp");

	if (!f)
		ERROR("bitmap "+folder+name+" no found");

	uint16    bfType;
	if (*f->read(&bfType)!='MB')
		ERROR(string("Bad bitmap ")+name);
	BITMAPFILEHEADER	head;

	f->read(&head);

	BITMAPINFOHEADER	info;
	f->read(&info);

	// correct width
	while ((info.biBitCount*info.biWidth) & ((1<<5)-1))
		info.biWidth++;

	image_size->width=info.biWidth;
	image_size->height=info.biHeight;

	COLOR *bits=NULL;
	switch (info.biBitCount)
	{
	case 1:	//for fonts
		f->pos_set(head.bfOffBits);
		bits=load_1bit_bitmap(f, info.biWidth/8, info.biHeight);
		break;

	case 8:	
		bits=load_8bit_bitmap(f, &info, (head.bfOffBits-sizeof(head)-sizeof(info))/4);
		break;

	case 24:	
		LOG(name+".bmp Warning: 24 bpp bitmap. Large size");
		bits=load_24bit_bitmap(f, &info);
		break;

	default:
		ERROR(name+" this bpp bitmap not support");
	}

	delete f;
	return bits;
}




//================================================================================
COLOR	*collection::load_24bit_bitmap(file *f, BITMAPINFOHEADER	*info) const
{
	color32	pixel;
	COLOR	*bits=large_mem::allocate<COLOR>(info->biHeight*info->biWidth),
			*p=bits+(info->biHeight-1)*info->biWidth,
			tmp;

	int	h, w;
	for (h=info->biHeight; --h>=0; p-=2*info->biWidth)
		for (w=info->biWidth; --w>=0; p++)
		{
			f->read((uint8*)&pixel, 3);
			tmp=pixel;
			large_mem::write(bits, p, &tmp);
		}

	return bits;
}



//================================================================================
COLOR *collection::load_8bit_bitmap(file *f, BITMAPINFOHEADER	*info, int n_colors) const
{
	int		i;
	COLOR	*palette=new COLOR[256];
	color32	pal_color;

	// read palette	
	for (i=0; i<n_colors; i++)
		palette[i]=*f->read(&pal_color);

	//read bits
	i=info->biHeight*info->biWidth;
	uint8	*buff8=new uint8[info->biWidth],
			*p8;
	COLOR	*bits=large_mem::allocate<COLOR>(i);
	COLOR	*buff16=new COLOR[info->biWidth],
			*p16;
	int	h, w;

	for (h=info->biHeight; --h>=0;)
	{
		p16=buff16;
		f->read(buff8, info->biWidth);
		p8=buff8;
		for (w=info->biWidth; --w>=0; p16++, p8++)
			*p16=palette[*p8];
		large_mem::write(bits, bits+h*info->biWidth, buff16, info->biWidth);
	}
	delete[] buff8;
	delete[] buff16;
	delete[] palette;

	return bits;
}


//================================================================================
COLOR *collection::load_1bit_bitmap(file *f, int width, int height) const
{
	uint8	*bits=large_mem::allocate<uint8>(width*height),
			*tmp=new uint8[width];
	for (int i=height; --i>=0;)
	{
		f->read(tmp, width);
		large_mem::write(bits, bits+i*width, tmp, width);
	}
	delete[] tmp;

	return (COLOR*)bits;
}


//================================================================================
sound	*collection::load_sound(const string &_name, bool sfx)
{
	string	name="sounds/"+_name;
	sound	*s=NULL;

	// сначала пробуем открыть wav (такой порядок удобен для отладки)
	file	*f=file_open(name+".wav");
	if (f)
		s=platform::get().load_sound(new mem_file(f), sfx);
	else
	{
		f=file_open(name+".ogg");
		if (f)
			s=platform::get().load_sound(new mem_file(f), sfx);
	}

	if (s==NULL)
	{
		#ifdef _WINDOWS
			LOG("can't find sound "+folder+name);
		#endif
		s=sound::empty;
	}
	else if ( sfx )
		sounds.push_back(s);
	return s;
}




//===============================================================
void collection::get_names(const string &category, list<string> &names)
{
	names.clear();
	// сначала каталоги
	platform::get().find_files(category, names, true);
	
	// удаляем '..'
	while (string *s=names.each_ptr())
		if (*s=="..")
			names.remove(*s);

	// потом архивы
	list<string>	zips;
	platform::get().find_files(category, zips, false);
	string	s;
    while (string *s=zips.each_ptr())
	{
		if (string(*s).lo_case().find(".zip"))
		{
			string str=string(s->begin(), s->end()-5);
			bool	pressed=false;
			while (string *n=names.each_ptr())
				if (string(*n).lo_case()==string(str).lo_case())
					pressed=true;
			if (!pressed)
				names.push_sortup(str);
		}
	}
}



//===============================================================
void collection::find_files(const string &ext, list<string> &names)
{
	names.clear();
	// сначала в каталоге
	platform::get().find_files(folder, names, false);
	
	// потом в архиве
	if (zip_file)
	{
		list<string>	zips;
		zip::list(zip_file, zips);
		while (string *z=zips.each_ptr())
		{
			bool	pressed=false;
			while (string *s=names.each_ptr())
				if (string(*s).lo_case()==string(*z).lo_case())
					pressed=true;
			if (!pressed)
				names.push_sortup(*z);
		}
	}

	// удаляем файлы с др. расширением
	while (string *s=names.each_ptr())
		if (string(*s).lo_case().find(string(ext).lo_case()))
			*s=string(s->begin(), s->end()-ext.length()-1);
		else
			names.remove(*s);
}




//=========================================================
// кэширование
collection::cache_record::cache_record()
	:ptr		(NULL)
	,n_used		(1)
{}


//=========================================================
collection::cache_record::cache_record(void *_ptr, const string &name)
	:ptr		(_ptr)
	,n_used		(1)
	,res_name	(name)
{}


//=========================================================
collection::cache_record::cache_record(const cache_record &c)
	:ptr		(c.ptr)
	,n_used		(c.n_used)
	,res_name	(c.res_name)
{}


//-----------------------------------------------------------
bool collection::cache_record::operator==(const cache_record &c) const
{
	return ptr==c.ptr;
}


//-----------------------------------------------------------
void *collection::seek_in_cache(const string &name)
{
	while (cache_record *c=cache.each_ptr())
		if (c->res_name==name)
		{
			c->n_used++;
			void	*ptr=c->ptr;
			cache.break_each();
			return ptr;
		}
	return NULL;
}










//===============================================================
//	общий набор ресурсов, необходимый mpp для работы
//===============================================================
common::common(const string &name, const string &category)
	:collection		(name, category)
	,button_active	("active",	this)
	,button_passive	("passive",	this)
	,strings		("strings",	file_open("strings.xml"))
{
	click=load_sound("click");

	font=new class font("default", this);

	p_plus	=load_always("plus");
	p_minus	=load_always("minus");
	p_left	=load_always("left");
	p_right	=load_always("right");
	p_up	=load_always("up");
	p_down	=load_always("down");
}



//===============================================================
common::~common()
{
	delete font;

	if (p_plus)
		free(p_plus);
	if (p_minus)
		free(p_minus);
	if (p_left)
		free(p_left);
	if (p_right)
		free(p_right);
	if (p_up)
		free(p_up);
	if (p_down)
		free(p_down);
}



//===============================================================
animation	*common::load_always(const string &name)
{
	file *f=file_open(name+".bmp");
	delete f;	//определяем тока наличие файла

	if (f)
		return load<animation>(this, name);
	return NULL;
}



//===============================================================
const string &common::str(const string &name)
{
	return get().strings(name);
}

}/*mpp */