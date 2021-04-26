/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_STRING_
#define _MPP_STRING_

#include "tools.h"
#include "fixed.h"

namespace	mpp
{
	// ����� ����� ������� �������� ����� ci_string - ��� ������/������/��������� ��������-����������� �����
	// � ����� �������� ������:
	// bool prefix(const char*)
	// bool suffix(const char*)
	// string cut_to_prefix(const char*)	- ������ ������ �� ������� ������������
	// string cut_to_suffix(const char*)	- ������ ������ �� ������� ������������
	// ����������� string(float value, int count);	- ���-�� ������ ����� �������


	//===========================================================================
	//	����� �����
	//===========================================================================
	class string
	{
	private:
		char	*text;		// ��� ���������� ������ ������
		int		size,		// ������ ������ ��� ����� \0
				max_size;	// ������������ ������ ������ ��� ����� \0

	public:
		 
		//===========================================================================
		inline	string()
			:text		(NULL)
			,size		(0)
			,max_size	(-1)
		{
			allocate(0);
		}


		//==========================================================================================================
		inline	string(const string &s)
			:text		(NULL)
			,size		(0)
			,max_size	(-1)
		{
			(*this)=s;
		}

		//==========================================================================================================
		inline	string(const char *s2)
			:text		(NULL)
			,size		(0)
			,max_size	(-1)
		{
			(*this)=s2;
		}

		//==========================================================================================================
		inline	string(char ch)
			:text		(NULL)
			,size		(0)
			,max_size	(-1)
		{
			(*this)=ch;
		}

		//==========================================================================================================
		string(const char *first, const char *last);

		//==========================================================================================================
		string(int value, int radix);

		//==========================================================================================================
		int	atoi(int radix=10) const;

		//==========================================================================================================
		REAL atof() const;

		//==========================================================================================================
		inline string	&allocate(int _size)
		{
			if (text)
				delete[] text;
			max_size=_size;
			text=new char[max_size+1];
			size=0;
			*text=0;
			return *this;
		}


		//==========================================================================================================
		virtual	~string();

		//==========================================================================================================
		inline	int length() const
		{
			return size;
		}

		//==========================================================================================================
		// ��������� �� text
		inline	char *begin() const
		{
			return text;
		}


		//==========================================================================================================
		// ��������� �� \0
		inline	char *end() const
		{
			return text+size;
		}

		//==========================================================================================================
		inline	int refresh_size()
		{
			ASSERT(text==NULL, "string::refresh_size text not allocated");
			char	*p=text;
			size=0;
			while(*(p++))
				size++;

			return size;
		}


		//==========================================================================================================
		string &operator=(const char *s);

		//==========================================================================================================
		string &operator=(const string &s);

		//==========================================================================================================
		string &operator=(char ch)
		{
			allocate(1);
			text[0]=ch;
			text[1]=0;
			size=1;
			return *this;
		}


		//==========================================================================================================
		inline	operator const char*() const
		{
			return text;
		};


		//==========================================================================================================
		inline	char &operator[](int pos)
		{
			ASSERT(pos>size || text==NULL, "string["+size+"] bad index ["+pos+"]");
			return text[pos];
		};


		//==========================================================================================================
		inline	int operator!() const
		{
			if (text==NULL)
				return true;
			return text[0]==0;
		};

		//==========================================================================================================
		inline	operator bool() const
		{
			if (text==NULL)
				return false;
			return text[0]!=0;
		};

		//==========================================================================================================
		string &operator+=(const string &s);

		//==========================================================================================================
		inline	string &operator+=(const char *s2)
		{
			return *this+=string(s2);
		};

		//==========================================================================================================
		string &operator+=(char sym);

		//==========================================================================================================
		inline	string operator+(const string &s) const
		{
			return string(*this)+=s;
		};

		//==========================================================================================================
		inline	string operator+(const char *s2) const
		{
			return	string(*this)+=s2;
		};


		//==========================================================================================================
		inline	string operator+(char sym) const
		{
			return string(*this)+=sym;
		}

		//==========================================================================================================
		// ��� �������� ������������� ��������� �����
		inline	string operator+(int n) const
		{
			return string(*this)+=string(n, 10);
		}

		//----------------------------------------------------------------------------------------------------------
		inline	string operator+(sint32 n) const
		{
			return string(*this)+=string(n, 10);
		}

		//----------------------------------------------------------------------------------------------------------
		inline	string operator+(uint32 n) const
		{
			return string(*this)+=string(n, 10);
		}

		//----------------------------------------------------------------------------------------------------------
		inline	string operator+(sint16 n) const
		{
			return string(*this)+=string(n, 10);
		}

		//----------------------------------------------------------------------------------------------------------
		inline	string operator+(uint16 n) const
		{
			return string(*this)+=string(n, 10);
		}

		//==========================================================================================================
		inline	bool operator==(const string &s) const
		{
			return *(this)==s.text;
		};


		//==========================================================================================================
		inline	bool operator==(const char *s) const
		{
			char	*p=text;

			while (*p)
				if (*(p++)!=*(s++))
					return false;
			return *s=='\0';
		};


		//=========================================================================================================
		inline	bool operator!=(const string &s) const
		{
			return !(*this==s.text);
		};


		//==========================================================================================================
		inline	bool operator!=(const char *s) const
		{
			return !(*this==s);
		};


		//==========================================================================================================
		bool operator<(const string &s) const;

		//==========================================================================================================
		bool operator> (const string &s) const;

		//==========================================================================================================
		inline	bool operator<=(const string &s) const
		{
			return !((*this)>s);
		};


		//==========================================================================================================
		inline	bool operator>=(const string &s) const
		{
			return !((*this)<s);
		};

		//==========================================================================================================
		inline	static	unsigned char up_char(char sym)
		{
			return	(sym>='a' && sym<='z' ? sym-'a'+'A' : sym);
		}

		//==========================================================================================================
		inline const string &up_case()
		{
			for (char *p=text+size; --p>=text;)
				*p=up_char(*p);
			return *this;
		}

		//==========================================================================================================
		inline	static	unsigned char lo_char(char sym)
		{
			return	(sym>='A' && sym<='Z' ? sym-'A'+'a' : sym);
		}

		//==========================================================================================================
		inline const string &lo_case()
		{
			for (char *p=text+size; --p>=text;)
				*p=lo_char(*p);
			return *this;
		}

		//==========================================================================================================
		string &replace(const string &old_str, const string &new_str);
		const char *find(const string &simple, const char *pos=NULL, bool forward=true) const;
		const char *find(char ch, const char *pos=NULL, bool forward=true) const;

		/*
		//==========================================================================================================
		inline	string &insert(const string &str, int pos)
		{
			string	dst=operator()(0, pos);
			dst+=str;
			dst+=operator()(pos, size-pos);
			
			return (*this)=dst;
		}
		*/

		//==========================================================================================================
		inline	static	string fill(const string &simple, int n_repeats)
		{
			string s;

			while (--n_repeats>=0)
				s+=simple;

			return s;
		}
		
		//==========================================================================================================
		// ���������� ������
		string &crypt(char key='a');
		string &uncrypt(char key='a');
	};//string

	//==========================================================================================================
	inline	string operator+(const char *s2, const string &s)
	{
		return string(s2)+=s;
	}
}//mpp

#endif /* _STRING_H_*/
