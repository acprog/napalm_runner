/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_FILE_
#define _MPP_FILE_

#include "string.h"

namespace mpp
{

	//=============================================================
	//	файловый ввод/вывод
	//=============================================================
	class file
	{
	public:
		virtual ~file(){}
		virtual int	 size()=0;				// размер файла
		virtual	void pos_set(int pos)=0;	// позиция с начала
		virtual	void pos_shift(int pos)=0;	// позиция с тек. позиции

		//---------------------------------------------------------------
		template <class T>
		inline void write(const T *data, int leng=1)
		{
			write_buffer((const void*)data, sizeof(T)*leng);
		}

		//---------------------------------------------------------------
		template <class T>
		inline T *read(T *data, int leng=1)
		{
			read_buffer((void*)data, sizeof(T)*leng);
			return data;
		}

		//---------------------------------------------------------------
		int read_int();
		string read_string();

		virtual	int read_buffer(void *buffer, int size) =0;
		virtual	void write_buffer(const void *buffer, int size) =0;
		virtual int tell()=0;
		virtual bool eof(){return false;};
	};



	//=============================================================
	//	файл в памяти
	//=============================================================
	class mem_file : public file
	{
	private:
		uint8	*buff,
				*cur;
		int		buff_size;

	public:
		mem_file(file *f);	// на чтение
		mem_file(int size);	// на запись
		~mem_file();
		int	 size();
		void pos_set(int pos);
		void pos_shift(int pos);

		int read_buffer(void *buffer, int size);
		void write_buffer(const void *buffer, int size);
		int tell();
		const uint8 *bits();
	};
}

#endif