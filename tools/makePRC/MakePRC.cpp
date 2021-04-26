/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#include <iostream.h>
#include <fstream.h>
#include "string.h"
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>

#include	<stdio.h>
#include	<stdarg.h>
#include	<conio.h>

#include <windows.h>

// типы
#define	UInt8	unsigned char
#define	UInt16	unsigned short
#define	UInt32	unsigned long
#define	LocalID	UInt32

#define	RSRC_ENTRY_SIZE	10

/* Palm<->Unix Time conversion */
#define PALM2UNIX(a)  (a - 2082844800)
#define UNIX2PALM(a)  (a + 2082844800)


#define	SPLIT_SIZE	0x8000	// размер куска, на который режутся файлы

//--------------------------------------------------------------
inline	void error( char * fmt, ... )
{
	char	s[1024]; 
	va_list	arglist;
	va_start( arglist, fmt );
	vsprintf( s, fmt, arglist );
	va_end ( arglist );
	throw s;
}





//=======================================================================
//	Перевернутое слово
//=======================================================================
class s_word
{
private:
	char	value[2];

	//------------------------------------------------------------------
public:
	s_word(UInt16 _value=0)
	{
		operator=(_value);
	}

	void operator=(UInt16 _value)
	{
		operator=((char*)&_value);
	}

	void operator=(char *p)
	{
		value[0]=p[1];
		value[1]=p[0];
	}

	void set(UInt16 _value)
	{
		*((UInt16*)value)=_value;
	}

	void operator++() int
	{
		value[1]++;
	}
};


//=======================================================================
//	Перевернутое двойное слово
//=======================================================================
class s_dword
{
private:
	char	value[4];

	//------------------------------------------------------------------
public:
	s_dword(UInt32 _value=0)
	{
		operator=(_value);
	}

	void operator=(UInt32 _value)
	{
		operator=((char*)&_value);
	}

	void operator=(char *p)
	{
		value[0]=p[3];
		value[1]=p[2];
		value[2]=p[1];
		value[3]=p[0];
	}

	bool operator!()
	{
		return *((UInt32*)value)==0;
	}

	operator UInt32()
	{
		char	v[4];
		v[0]=value[3];
		v[1]=value[2];
		v[2]=value[1];
		v[3]=value[0];
		return *((UInt32*)v);
	}

	void save(ofstream &f)
	{
		f.write(value, 4);
	}
};


//=======================================================================
//	хранение всех записей
struct record
{
	s_dword	type;	
	s_word	number;	
	s_dword chunkID;
	char	name[1024];
	char	value[1024];
	int		n_cut;	// номер куска для разбитых файлов по 0xffff
}records[1000];

int	n_records=0;	// колл-во записей на базу
int chunkID=0;		// что-то похожее на сдвиг в байтах от начала базы


//=======================================================================
void correct_records_list()
{
	for (int i=0; i<n_records; i++)
		if (!records[i].type || records[i].value[0]=='\0')
		{
			for (int j=i; j<n_records; j++)
				records[j]=records[j+1];
			i--;
			n_records--;
		}
}


//=======================================================================
void write_buffer(ofstream &f, char *buffer, int leng)
{
	f.write(buffer, leng);
	chunkID+=leng;
}


//=======================================================================
void write_bmp(ofstream &f, char *filename)
{
	// читаем .bmp
	ifstream	src(filename, ios::binary|ios::nocreate);
	
	if (!src)
		error("Bitmap %s file is not found", filename);
	
	tagBITMAPFILEHEADER	bmp_header;
	src.read((UCHAR*)&bmp_header, sizeof(bmp_header));

	if (bmp_header.bfType!='MB')
		error("Invalid bitmap file %s", filename);

	tagBITMAPINFOHEADER	info;
	src.read((UCHAR*)&info, sizeof(info));

	if (info.biBitCount!=8)
		cout << "\nWarning: Bitmap " << filename << " witch bpp=" << info.biBitCount;

	int	row_bytes=info.biWidth*info.biBitCount;
	if ( row_bytes & 0x1f )
	{
		row_bytes|=0x1f;
		row_bytes++;
		cout << "\nIncorrect width (" << info.biWidth << ") of bitmap " << filename << " recoremed=" << row_bytes/8;
	}
	row_bytes/=8;

	// растр
	int		size=row_bytes*info.biHeight;
	char	*raster=new char[size];
	src.seekg(bmp_header.bfOffBits, ios::beg);
	src.read(raster, size);

	// пишем Tbmp
	struct BitmapType
	{
		s_word	width,
				height,
				rowBytes,
				flags;
		UInt8	pixelSize,
				version;
		s_word	nextDepthOffset;
		UInt8	transparentIndex,
				compressionType;
		s_word	reserved;  
	}tbmp_header;

	tbmp_header.width=info.biWidth;
	tbmp_header.height=info.biHeight;
	tbmp_header.rowBytes=row_bytes;
	tbmp_header.flags=0x2000;
	tbmp_header.pixelSize=info.biBitCount;
	tbmp_header.version=2;
	tbmp_header.transparentIndex=0,
	tbmp_header.compressionType=0;

	write_buffer(f, (char*)&tbmp_header, sizeof(tbmp_header));

	char	str[256];
	sprintf(str, "c:\\temp\\%s.Tbmp", filename);
	ofstream	ftemp(str, ios::binary);
	// т.к. CW зачем-то их переворачивает:
	tbmp_header.width.set(info.biWidth);
	tbmp_header.height.set(info.biHeight);
	tbmp_header.rowBytes.set(row_bytes);
	tbmp_header.flags.set(0x2000);
	
	ftemp.write((char*)&tbmp_header, sizeof(tbmp_header));

	char	*p=raster+row_bytes*(info.biHeight-1);
	for (int y=0; y<info.biHeight; y++, p-=row_bytes)
	{
		write_buffer(f, p, row_bytes);
		ftemp.write(p, row_bytes);
	}

	delete[] raster;
}






//------------------------------------
// файлы нужно бить на куски по 64kb
// и вставлять виртуальные ресурсы внутри них
void write_file_cut(ofstream &f, record *r)
{
	ifstream	src(r->value, ios::binary|ios::nocreate);
	if (!src)
		error("Resource (%s) file %s is not found", r->name, r->value);
	src.seekg(r->n_cut*SPLIT_SIZE, ios::beg);
	char	ch;
	int		leng=SPLIT_SIZE;
	while (--leng>=0)
	{
		src.get(ch);
		if (src.eof())
			break;
		f.put(ch);
		chunkID++;
	}
}


//=======================================================================
// файл бьется на куски по 64кб
void devide_file(record *r)
{
	struct __stat64 buf;

	if (0!=_stat64(r->value, &buf ))
		error( "Problem getting information of file: %s", r->value);

	// размер файла
	record	*ri=records+(n_records++);
	ri->type='tint';
	ri->number=r->number;
	itoa(buf.st_size, ri->value, 10);
	
	// добавляем в цепочку нужное колл-во кусков:
	r->n_cut=0;
	while (buf.st_size>0)
	{
		records[n_records++]=*r;
		r->number++;
		r->n_cut++;
		buf.st_size-=SPLIT_SIZE;
	}
	r->value[0]=0;	// удалить потом из базы
}


//=======================================================================
// найти неизвестные типы файлов и разбить на цепочки
void divide_large_files()
{
	int		nr=n_records;
	record	*r=records;
		

	for (int i=nr; --i>=0; r++)
		switch((UInt32)r->type)
		{
		case 'tSTR': case 'tver': case 'tAIS':
		case 'tint':
		case 'Tbmp':
			break;	// поддерживаемые типы
		
		default:	// внешний файл
			devide_file(r);
		}

	// 	убираем старые записи
	correct_records_list();
}





//=======================================================================
void write(ofstream &f, record *r)
{
	s_dword	tmp;

	switch ((UInt32)r->type)
	{
	case 'tSTR': case 'tver': case 'tAIS': 
		//------------------------------------
		// строка
		write_buffer(f, r->value, strlen(r->value)+1);
		break;

	case 'tint':
		//------------------------------------
		// константа
		tmp=atoi(r->value);
		tmp.save(f);
		chunkID+=4;
		break;

	case 'Tbmp':
		write_bmp(f, r->value);
		break;

	default:
		write_file_cut(f, r);
		break;
	}
}






//=======================================================================
// обновление информации о записи
void push(char *global, char *first, char *last)
{
	char	name[1024];
	strcpy(name, global);
	*last='\0';
	strcat(name, first);

	int	i;
	for (i=0; i<n_records; i++)
		if (strcmp(name, records[i].name)==0)
		{
			strcpy(records[i].value, last+1);
			return;
		}

	// если записи не было
	i=n_records++;
	strcpy(records[i].value, last+1);
	strcpy(records[i].name, name);
}




//=======================================================================
// вставка новой записи
void push_indexed(char *global, char *first, char *last)
{
	int	i=n_records++;

	memmove(&records[i].type, first, 4);
	
	first=last+1;
	while (*last!=']')
		last++;
	*(last)='\0';
	records[i].number=atoi(first);

	while (*(++last)==' ' || *last=='\t');

	first=last;
	while (*last!='=')
		last++;
	*(last)='\0';
	strcpy(records[i].name, global);
	strcat(records[i].name, first);
	strcpy(records[i].value, last+1);
}



//=======================================================================
//	рекурсивная разборка строк
// возвращает true - если конец блока }
bool push_string(char *global, ifstream &f)
{
	char	str[1024];

	f.getline(str, 1024);

	if (f.eof())
		return true;

	// убираем пробелы с начала
	char *first=str;
	while (*first==' ' || *first=='\t')
		first++;

	// комментарии
	if (first[0]=='/' && first[1]=='/')
		return false;

	char *last=first;
	
	while (1)
	{
		switch(*last)
		{
		case '\0':
			if (first==last)	// пустая строка
				return false;

			// это структура
			char	global_name[1024];
			strcpy(global_name, global);
			strcat(global_name, first);
			strcat(global_name, ".");
			while (!push_string(global_name, f));
			return false;
		
		case '{':
			return false;

		case '}':
			return true;

		case '[':
			// это индекс
			push_indexed(global, first, last);
			return false;			

		case '=':
			// это поле
			push(global, first, last);
			return false;
		}

		last++;
	}
}


//=======================================================================
//	поиск поля по имени
char *search(char *name)
{
	for (int i=0; i<n_records; i++)
		if (strcmp(name, records[i].name)==0)
			return records[i].value;

	error("Value %s is not found", name);
	return NULL;
}


//=======================================================================
// основная ф-ия
int main(int argc, char* argv[])
{
	try
	{
		char	info_name[1024],
				dst_name[1024];
		int	i;

		cout	<< "\nPRC Maker (c) Alexander Semenov. Napalm Games. 2003"
				<< "\n  Run: MakePRC source.info [destination.prc]";

		switch (argc)
		{
		case 1:
			cout << "\n\nEnter source filename (without extension): ";
			cin>>info_name;
			strcpy(dst_name, info_name);
			strcat(info_name, ".info");
			strcat(dst_name, ".prc");
			break;
			
		case 2:
			strcpy(info_name, argv[1]);
			strcpy(dst_name, info_name);
			strcpy(dst_name+strlen(dst_name)-4, "prc");
			break;

		case 3:
			strcpy(info_name, argv[1]);
			strcpy(dst_name, argv[2]);
		}

		ifstream	info(info_name, ios::nocreate);
	
		if (!info)
			error("Info file %s is not found", info_name);

		cout	<< "\n========================================================"
				<< "\n Make " << dst_name << " from info " << info_name;

		char	template_name[1024];
		
		char	ch=0;
		while (ch!='=')
			info.get(ch);
		info >> template_name;
		strcat(template_name, ".tml");

		ifstream	templ(template_name, ios::nocreate);
	
		if (!templ)
			error("Template file %s is not found", template_name);

		cout	<< "\n Use template " << template_name;

		while (!templ.eof())
			push_string("", templ);

		while (!info.eof())
			push_string("", info);

		//----------------------------------------------------------------
		struct	DatabaseHdrType
		{
			char	name[32];
			UInt16	attributes,
					version;
			UInt32	creationDate,
					modificationDate,
					lastBackupDate,
					modificationNumber;
			LocalID	appInfoID,
					sortInfoID;
			UInt32	type,
					creator,
					uniqueIDSeed;
			UInt32	nextRecordListID;
			s_word	numRecords;
		}header;

		// пишем заголовок
		time_t	tim=UNIX2PALM(time(0));
  
		memset(&header, 0, sizeof(header));
		
		char	*name=search("header.name");
		strcpy(header.name, name[0]=='\0' ? dst_name : name);

		header.attributes=0x100;
		header.version=atoi(search("header.version"));
		header.creationDate=tim;
		header.modificationDate=tim;
		header.type=*(UInt32*)search("header.type");
		header.creator=*(UInt32*)search("header.creator");
		
		// убрать записи не имеющие смысла
		correct_records_list();
		// разбиваем большие файлы на куски по 64кб
		divide_large_files();

		header.numRecords=n_records;
		cout << "\n Records=" << n_records;

		chunkID=sizeof(header)+n_records*RSRC_ENTRY_SIZE;

		ofstream	dst(dst_name, ios::binary);
		dst.write((char*)&header, sizeof(header));

		// презапись заголовков ресурсов
		for (i=0; i<n_records; i++)
			dst.write((char*)(records+i), RSRC_ENTRY_SIZE);

		// запись ресурсов
		for (i=0; i<n_records; i++)
		{
			records[i].chunkID=chunkID;
			write(dst, records+i);
		}

		dst.seekp(sizeof(header), ios::beg);
		// перезапись заголовков ресурсов
		for (i=0; i<n_records; i++)
			dst.write((char*)(records+i), RSRC_ENTRY_SIZE);

		cout	<< "\nSuccess!" << endl;
	}

	catch (const char *msg)
	{
		cout << "\nError: " << msg << "\n";
		return -1;
	}
	return 0;
}
