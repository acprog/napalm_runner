/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_XML_
#define _MPP_XML_

#include "file.h"
#include "list.h"

namespace mpp
{
	//===============================================================
	//	XML - запись
	// имеет смысл сделать класс или базовый для XML или еще какой,
	// чтобы умел по XML при загрузке создавать не только XML но и
	// объект нужного типа, при этом испоьзуя приемущества парсинга XML
	//===============================================================
	class xml
	{
	private:
		
		list<xml*>	internals;
		string		name,		// название блока
					text,		// текст внутри блока
					header;		// заголовок xml-файла
		bool		attribute;	// запись в виде атрибута

		bool read(file *f);				// чтение информации в блок
		bool read_attributes(file *f);	// чтение атрибутов в блок

		void search_clones(const string &command, list<xml*> &to_clone);	// поиск запросов на клонирование
		void clone(xml &x);	// клонирование записи
		
		inline bool empty(char ch)
		{
			return ch==' ' || ch=='\t' || ch=='\n' || ch==0xd;
		}
		
	public:
		xml(const string &_name, const string &text="", bool _attribute=false);	// создание нового xml-блока
		xml(const string &_name, file *f);	// чтение xml-блока из файла
		virtual ~xml();

		void write(string &s, int tabs=0);
		void write(file *f, int tabs=0);

		xml &folder(const string &_name);
		string &record(const string &path, const string &default_value="", bool _attribute=false);

		inline string &operator()(const string &path, const string &default_value="", bool _attribute=false)
		{
			return record(path, default_value, _attribute);
		}

		inline string &record(const string &path, bool _attribute)
		{
			return record(path, "", _attribute);
		}

		inline string &operator()(const string &path, bool _attribute)
		{
			return record(path, "", _attribute);
		}
	};
}

#endif
