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
	//	XML - ������
	// ����� ����� ������� ����� ��� ������� ��� XML ��� ��� �����,
	// ����� ���� �� XML ��� �������� ��������� �� ������ XML �� �
	// ������ ������� ����, ��� ���� �������� ������������ �������� XML
	//===============================================================
	class xml
	{
	private:
		
		list<xml*>	internals;
		string		name,		// �������� �����
					text,		// ����� ������ �����
					header;		// ��������� xml-�����
		bool		attribute;	// ������ � ���� ��������

		bool read(file *f);				// ������ ���������� � ����
		bool read_attributes(file *f);	// ������ ��������� � ����

		void search_clones(const string &command, list<xml*> &to_clone);	// ����� �������� �� ������������
		void clone(xml &x);	// ������������ ������
		
		inline bool empty(char ch)
		{
			return ch==' ' || ch=='\t' || ch=='\n' || ch==0xd;
		}
		
	public:
		xml(const string &_name, const string &text="", bool _attribute=false);	// �������� ������ xml-�����
		xml(const string &_name, file *f);	// ������ xml-����� �� �����
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
