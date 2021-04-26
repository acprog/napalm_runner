/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "string.h"

namespace mpp
{


//===========================================================================
//	класс строк
//===========================================================================
string::string(int value, int radix)
	:text		(NULL)
	,size		(0)
	,max_size	(-1)
{
	static	char	buff[100];	// обратный буффер
	char			*src=buff;
	bool			negative=value<0;

	if (negative)
		value=-value;

	// запись в обратном порядке
	switch (radix)
	{
	//---------------------------------
	case 10:
		do
		{
			*(src++)=(value%10)+'0';
			value/=10;
		}
		while (value);
		break;

	//---------------------------------
	case 16:
		do
		{
			*src=(value%16)+'0';
			if (*src>'9')
				*src+='a'-'9'-1;
			src++;
			value/=16;
		}
		while (value);
		break;
	}

	// перенос в прямом порядке
	allocate(100);
	char	*dst=text;
	if (negative)
		*(dst++)='-';

	while (src>buff)
		*(dst++)=*(--src);
	*dst='\0';
	refresh_size();
}


//==========================================================================================================
// сегмент строки с..по включительно
string::string(const char *first, const char *last)
	:text		(NULL)
	,size		(0)
	,max_size	(-1)
{
	ASSERT(last<first, "sting: invalid segment");
	allocate(last-first+1);

	char	*dst=text;

	while (first<=last)
		*(dst++)=*(first++);
// еще такой вариант - если first>last, то записывать строку в обратном порядке
	*dst=0;
	refresh_size();
}


//===========================================================================
string::~string()
{
	if (text)
		delete[] text;
}



//==========================================================================================================
int	string::atoi(int radix) const
{
	unsigned int	value=0;
	for (const char *p=text; *p; p++)
	{
		unsigned char ch=*p;
		if (ch<'0' || ch>'9')
			if (radix!=16)
				break;
			else
			{
				if (ch>='a')
					ch+='0'-'a'+10;
				else if (ch>='A')
					ch+='0'-'A'+10;
			}
		
		value*=radix;
		value+=ch-'0';
	}
	return value;
}


//==========================================================================================================
REAL	string::atof() const
{
	REAL	value=0;
	for (const char *p=text; *p; p++)
	{
		if (*p=='.')
		{
			REAL	deg=1;
			while (*(++p))
			{
				if (*p<'0' || *p>'9')
					break;
				deg/=10;
				value+=deg*(*p-'0');
			}
			break;
		}
		else if (*p<'0' || *p>'9')
			break;
		value*=10;
		value+=*p-'0';
	}
	return value;
}


//==========================================================================================================
string &string::operator=(const char *s)
{
	if (!s)
	{
		if (text)
			*text=0;
		else
			allocate(0);
		size=0;
		return *this;
	}

	const char	*s1=s;
	int		s_size=0;

	while(*(s1++))
		s_size++;
	
	if (max_size<s_size)
		allocate(s_size);

	size=s_size;
	char	*dst=text;
	while (*(dst++)=*(s++));

	return *this;
}


//==========================================================================================================
string &string::operator=(const string &s)
{
	if (!s)
	{
		if (text)
			*text=0;
		else
			allocate(0);
		size=0;
		return(*this);
	}

	if (max_size<s.size)
		allocate(s.size);

	char	*dst=text,
			*src=s.text;

	size=s.size;
	while (*(dst++)=*(src++));
	return *this;
}


//==========================================================================================================
string &string::operator+=(const string &s)
{
	if (!s)
		return(*this);

	if (!(*this))
		return *this=s;

	if (max_size>=size+s.size)
	{
		char	*src=s.text, 
				*dst=text+size;
		while (*(dst++)=*(src++));
		size+=s.size;
		return *this;
	}

	char	*old_text=text;
	text=NULL;
	allocate(size+s.size);
	size=max_size;
	
	char	*src=old_text, 
			*dst=text;
	while (*(dst++)=*(src++));
	src=s.text;
	dst--;
	while (*(dst++)=*(src++));

	delete[] old_text;
	return *this;
};


//==========================================================================================================
string &string::operator+=(char sym)
{
	if (max_size<size+1)
	{
		int	sz=size;

		char	*old_text=text,
				*src=text;
		text=NULL;
		allocate(size+20);//20 - предсказание на случай последующего добавления буквы
		char	*dst=text;

		while (*(dst++)=*(src++));
		
		delete[] old_text;
		size=sz;
	}

	text[size++]=sym;// '\0' <- sym
	text[size]=0;

	return *this;
}




//==========================================================================================================
bool string::operator<(const string &s) const
{
	int	i=0;

	while(up_char((*this)[i])==up_char(s[i]))
		if (!s[i++])
			return(0);
		
	return up_char((*this)[i])<up_char(s[i]);
};


//==========================================================================================================
bool string::operator> (const string &s) const
{
	int	i=0;

	while(up_char((*this)[i])==up_char(s[i]))
		if (!(*this)[i++])
			return(0);
		
	return up_char((*this)[i])>up_char(s[i]);
};



//==========================================================================================================
string &string::replace(const string &old_str, const string &new_str)
{
	string	dst;
	char	*first=text, 
			*last=text,
			*p=old_str.text;

	while (*last)
	{
		if (*p==0)
		{
			dst+=string(first, last-old_str.length()-1);
			dst+=new_str;
			first=last;
			p=old_str.text;
		}
		else if (*last!=*p)
			p=old_str.text;
		else
			p++;
		last++;
	}

	if (*p==0)
		last-=old_str.length()+1;

	dst+=string(first, last);
	return (*this)=dst;
}


//==========================================================================================================
// поиск строки начиная с pos. возврашается первый символ найденой строки
// иначе -1
const char *string::find(const string &simple, const char *pos, bool forward) const
{
	if (forward)
	{
		if (pos==NULL)
			pos=text;

		const char	*first=pos, 
					*last=pos,
					*p=simple.text;

		while (*last)
		{
			if (*p==0)
				return first;
			else if (*last!=*p)
			{
				first=last+1;
				p=simple.text;
			}
			else
				p++;
			last++;
		}
		if (*p==0)
			return first;
	}
	else
		ERROR("Нужно писать sring::find для обратного поиска");
	return NULL;
}



//==========================================================================================================
// поиск символа начиная с pos. возврашается его позиция
// иначе -1
const char *string::find(char ch, const char *pos, bool forward) const
{
	if (forward)
	{
		if (pos==NULL)
			pos=text;
		char	*end=text+size;
		for (; pos<end; pos++)
			if (*pos==ch)
				return pos;
	}
	else
	{
		if (pos==NULL)
			pos=text+size;
		for (; --pos>=text;)
			if (*pos==ch)
				return pos;
	}
	return NULL;
}



//==========================================================================================================
// шифрование строк
string &string::crypt(char key)
{
	string	str;
	str.allocate((size+1)*2);

	// обходим все буквы и делаем xor  с  предыдущей. результат добавляем в конец строки
	// полученные символы разбиваем на 2-а по 16бит с добавлением к a и j
	uint8	*p=(uint8*)text,
			ch=key;

	// считаем сумму
	uint8	summ=0;
	while (*p)
		summ+=*(p++);
	p=(uint8*)text;

	while (*p)
	{
		ch^=*p;
		str+='a'+ch/16;
		str+='j'+ch%16;
		ch=*(p++);
	}
	
	ch^=summ;
	str+='a'+ch/16;
	str+='j'+ch%16;

	return (*this)=str;
}



//==========================================================================================================
string &string::uncrypt(char key)
{
	if (size<2)
		return *this="";

	// обратная операция
	string	str;
	str.allocate(size/2);

	// склеиваем символы
	uint8	*p=(uint8*)text,
			ch=key;

	while (*p)
	{
		ch^=(p[0]-'a')*16+p[1]-'j';
		str+=ch;
		p+=2;
	}

	str.end()[-1]=0;
	str.size--;
	// считаем сумму
	p=(uint8*)str.text;
	while (*p)
		ch-=*(p++);
	return *this=(ch==0 ? str : string(""));
}

}

