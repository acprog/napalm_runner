/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "file.h"
#include "platform.h"
#include "mem.h"

namespace mpp
{

//=============================================================
//	файловый ввод/вывод
//=============================================================
int file::read_int()
{
	int buff;
	read(&buff);
	return buff;
}

//=============================================================
string file::read_string()
{
	char	buff[1024];
	char	*p=buff;
			
	do
	{
		read(p);
	}
	while (*(p++));
	return buff;
}



//=============================================================
//	файл в памяти
//=============================================================
mem_file::mem_file(file *f)
	:buff		((uint8*)platform::get().large_allocate(f->size()))
	,cur		(buff)
	,buff_size	(f->size())
{
	int		rd;
	uint8	*tmp=new uint8[BUFFER_SIZE];
	for (uint8 *p=buff; (rd=f->read_buffer(tmp, BUFFER_SIZE)) >0; p+=rd )
		large_mem::write(buff, p, tmp, rd);
	
	delete tmp;
	delete f;	// файл закрывается - теперь он весь в памяти
}



//=============================================================
mem_file::mem_file(int sz)
	:buff		((uint8*)platform::get().large_allocate(sz))
	,cur		(buff)
	,buff_size	(sz)
{
}


//=============================================================
mem_file::~mem_file()
{
	large_mem::free(buff);
}


//=============================================================
int	 mem_file::size()
{
	return buff_size;
}


//=============================================================
void mem_file::pos_set(int pos)
{
	cur=buff+pos;
}


//=============================================================
void mem_file::pos_shift(int pos)
{
	cur+=pos;
}



//=============================================================
int mem_file::read_buffer(void *buffer, int size)
{
	if (size>buff+buff_size-cur)
		size=buff+buff_size-cur;
	mem_cpy((uint8*)buffer, cur, size);
	cur+=size;
	return size;
}


//=============================================================
void mem_file::write_buffer(const void *buffer, int size)
{
	large_mem::write(buff, cur, (uint8*)buffer, size);
	cur+=size;
}


//=============================================================
int mem_file::tell()
{
	return cur-buff;
}


//=============================================================
const uint8 *mem_file::bits()
{
	return buff;
}

}/*mpp*/
