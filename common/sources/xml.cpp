#include "xml.h"

namespace mpp{

	//===============================================================
	//	XML-запись
	//===============================================================
	xml::xml(const string &_name, const string &str, bool _attribute)
		:name		(_name)
		,text		(str)
		,attribute	(_attribute)
	{
	}


	//===============================================================
	// чтение xml-блока из файла. файл после прочтения удаляется
	xml::xml(const string &_name, file *f)
		:name		(_name)
		,attribute	(false)
	{
		if (!f)
			return;

		if (f->size()==0)
			return;// пустой файл

		char ch;
		string	nm;

		for(;;)
		{
			while (*f->read(&ch)!='<')	// ищем начало первого тега
			{
				if (f->eof())
				{
					DEBUG_ONLY(ERROR("Bad XML "+name));
					delete f;
					return;
				}
			}
			
			while (!empty(*f->read(&ch)) && ch!='>')	// читаем имя
				nm+=ch;

			if (nm!="?xml")
				break;

			// пока без разбора служебной информации
			header="<?xml\n";
			nm="";
			while (ch!='>')
			{
				header+=*f->read(&ch);
				if (f->eof())
				{
					DEBUG_ONLY(ERROR("Bad XML "+name));
					delete f;
					return;
				}
			}
		}

		if (name!=nm)
		{
			DEBUG_ONLY(ERROR("Icorrect XML type: "+nm+" need: "+name));
			delete f;
			return;
		}
		

		bool	bad=false;
		if (ch!='>')
			f->read(&ch);
		if (ch=='>')
			bad=!read(f);
		else if (empty(ch))
			bad=!read_attributes(f);
		
		if (bad)
		{
			DEBUG_ONLY(ERROR("Bad XML "+name));
			delete f;
			return;
			// удалить все загруженные элементы, т.к. структура нарушена
		}

		//-----------------------------------------------
		// клонируем записи
		while (xml *x=internals.each())
			if (x->name=="use_template_command")
			{
				internals.break_each();
				
				list<xml*> to_clone;	// список запросов на клонирование
				search_clones(x->text, to_clone);
				while (xml *tc=to_clone.each())
					tc->clone(folder(tc->record(x->text)));
				break;
			}

		delete f;
	}


	//===============================================================
	xml::~xml()
	{
		while (xml *x=internals.each())
			delete x;
	}


	//===============================================================
	// читаем внутреннюю структуру блока из файла
	bool xml::read(file *f)
	{
		// имя у блока уже есть, т.к. блок был создан конструктором
		string	end=("/"+name);

		for(;;)
		{
			char ch;
			while (*f->read(&ch)!='<')	// ищем начало первого тега
			{
				if (f->eof())
					return false;
				text+=ch;
			}
			
			string nm;
			while (*f->read(&ch)!='>')
			{
				if (f->eof())
					return false;
				else if (empty(ch))
					break;

				nm+=ch;
			}

			if (nm.end()[-1]!='/')	// не пустая папка
			{
				if (nm==end)
					break;

				if (!nm)	// не по правилам xml, но одиночные '< ', '<>' разрешаю
					text+=nm+"<"+ch;
				else
				{
					xml	*x=new xml(nm);
					internals.push_back(x);

					if (empty(ch))
					{
						if (!x->read_attributes(f))
							return false;
					}
					else if (!x->read(f))
						return false;
				}
			}
		}
		
		if (!internals.is_empty())
			text.allocate(0);
		return true;
	}


	//===============================================================
	// чтение атрибутов в блок
	bool xml::read_attributes(file *f)
	{
		char ch=' ';
		for(;;)
		{
			// ищем начало атрибута
			while (empty(ch))
			{
				f->read(&ch);
				if (f->eof())
					return false;
			}
			if (ch=='>')
				return read(f);
			else if (ch=='/')
				return *f->read(&ch)=='>';

			// читаем имя атрибута
			string nm;
			while (!empty(ch) && ch!='=')
			{
				nm+=ch;
				f->read(&ch);
				if (f->eof())
					return false;
			}

			// ищем значение
			while (ch!='\"')
			{
				f->read(&ch);
				if (f->eof())
					return false;
			}
	
			string	tx;
			// читаем значение
			while (*f->read(&ch)!='\"')
				tx+=ch;
			internals.push_back(new xml(nm, tx, true));
			f->read(&ch);	// читаем след. символ
		}
		return true;
	}


	//===============================================================
	void xml::write(file *f, int tabs)
	{
		if (tabs==0)
			if (header)
			{
				f->write((const char*)header, header.length());
				f->write("\n");
			}

		int i;

		if (attribute)
			f->write("\n");

		for (i=tabs; --i>=0;)
			f->write("\t");

		if (attribute)
		{
			f->write((const char*)name, name.length());
			f->write(" = \"", 4);
			f->write((const char*)text, text.length());
			f->write("\"");
			return;
		}

		f->write("<");
		f->write((const char*)name, name.length());

		int	n_atr=0;
		while (xml *x=internals.each())
			if (x->attribute)
			{
				x->write(f, tabs+1);
				++n_atr;
			}
		
		if (n_atr==internals.count() && !text)
			f->write("/>\n", 3);
		else
		{
			f->write(">", 1);
				
			if (!internals.is_empty())
				f->write("\n", 1);

			while (xml *x=internals.each())
				if (!x->attribute)
					x->write(f, tabs+1);

			if (internals.is_empty())
			{
				f->write((const char*)text, text.length());
				f->write("</", 2);
			}
			else
			{
				for (i=tabs; --i>=0;)
					f->write("\t");
				f->write("</", 2);
			}
			f->write((const char*)name, name.length());
			f->write(">\n", 2);
		}
		if (tabs==0)
			delete f;
	}


	//===============================================================
	void xml::write(string &s, int tabs)
	{
		if (tabs==0)
			if (header)
				s+=header+"\n";

		int i;

		if (attribute)
			s+="\n";

		for (i=tabs; --i>=0;)
			s+="\t";

		if (attribute)
		{
			s+=name+" = \""+text+"\"";
			return;
		}

		s+="<"+name;

		int	n_atr=0;
		while (xml *x=internals.each())
			if (x->attribute)
			{
				x->write(s, tabs+1);
				++n_atr;
			}
		
		if (n_atr==internals.count() && !text)
			s+="/>\n";
		else
		{
			s+=">";
				
			if (!internals.is_empty())
				s+="\n";

			while (xml *x=internals.each())
				if (!x->attribute)
					x->write(s, tabs+1);

			if (internals.is_empty())
				s+=text+"</";
			else
			{
				for (i=tabs; --i>=0;)
					s+="\t";
				s+="</";
			}
			s+=name+">\n";
		}
	}




	//=======================================================================================
	string &xml::record(const string &path, const string &default_value, bool _attribute)
	{
		if (!path)
			return text;

		const char *mark=path.find('/');
		string	nm=(mark ? string(path, mark-1) : path);

		while (xml *x=internals.each())
			if (x->name==nm)
			{
				string &s=(*x)(mark ? mark+1 : "", default_value, _attribute);
				internals.break_each();
				return s;
			}

		xml	*x=(mark ? new xml(string(path, mark-1)) : new xml(path, default_value, _attribute));
		internals.push_back(x);
		return (*x)(mark ? mark+1 : "", default_value, _attribute);
	}


	//=======================================================================================
	xml &xml::folder(const string &path)
	{
		const char *mark=path.find('/');
		string	nm=(mark ? string(path, mark-1) : path);

		while (xml *x=internals.each())
			if (x->name==nm)
			{
				internals.break_each();
				return mark ? x->folder(mark+1) : *x;
			}

		xml	*x=new xml(nm);
		internals.push_back(x);
		return mark ? x->folder(mark+1) : *x;
	}


	//=======================================================================================
	// поиск запросов на клонирование
	void xml::search_clones(const string &command, list<xml*> &to_clone)
	{
		if (internals.is_empty())	// нет внутренних элементов
			return;

		if (record(command)!="")
			to_clone.push_back(this);
		while (xml *x=internals.each())
			x->search_clones(command, to_clone);
	}

	//=======================================================================================
	// клонировать на себя
	void xml::clone(xml &x)
	{
		if (!text)
		{
			text=x.text;
			attribute=x.attribute;
		}
		while (xml *i=x.internals.each())
			folder(i->name).clone(*i);
	}
}