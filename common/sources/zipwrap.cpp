//===================================================================
//	оболочка на zlib для использования в mpp
//===================================================================


#include "zip/unzip.h"
#include "zip/zlib.h"
#include "zip/ioapi.h"

#include "platform.h"

//==================================================================================
extern "C" void *memcpy_my(void *dst, const void *src, int size)
{
	long		*d4=(long*)dst;
	const long 	*s4=(long*)src;
	char		*d;
	const char	*s;

	for (;size>=4; size-=4)
		*(d4++)=*(s4++);

	d=(char*)d4;
	s=(char*)s4;
	
	while (--size>=0)
		*(d++)=*(s++);
	
	return dst;
}


//==================================================================================
extern "C" void *memset_my(void *dst, int n, int size)
{
	long		*d4=(long*)dst;
	char		*d;

	for (;size>=4; size-=4)
		*(d4++)=n;

	d=(char*)d4;
	
	while (--size>=0)
		*(d++)=n;
	return dst;
}



//==================================================================================
extern "C" void *malloc_my(int size)
{
	return new char[size] ;
}


//==================================================================================
extern "C" void *calloc_my(int num, int size)
{
	return new char[size*num];
}



//==================================================================================
extern "C" void free_my(void *p)
{
	delete p;
}

//==================================================================================
extern "C" int strlen_my(const char *str)
{
	int i=0;
	while (*(str++))
		i++;
	
	return i;
}


//==================================================================================
extern "C" int strcmp_my(const char *s1, const char *s2)
{
	while (*s1)
		if (*(s1++)!=*(s2++))
			return false;
	return *s2=='\0';
}


namespace mpp
{
	voidpf ZCALLBACK fopen_file_func OF((
	voidpf opaque,
	const char* filename,
	int mode));

	uLong ZCALLBACK fread_file_func OF((
	voidpf opaque,
	voidpf stream,
	void* buf,
	uLong size));

	uLong ZCALLBACK fwrite_file_func OF((
	voidpf opaque,
	voidpf stream,
	const void* buf,
	uLong size));

	long ZCALLBACK ftell_file_func OF((
	voidpf opaque,
	voidpf stream));

	long ZCALLBACK fseek_file_func OF((
	voidpf opaque,
	voidpf stream,
	uLong offset,
	int origin));

	int ZCALLBACK fclose_file_func OF((
	voidpf opaque,
	voidpf stream));

	int ZCALLBACK ferror_file_func OF((
	voidpf opaque,
	voidpf stream));


	//------------------------------------------------------------------
	voidpf ZCALLBACK fopen_file_func (
	voidpf opaque,
	const char* filename,
	int mode)
	{
		file	*f=platform::get().file_open(filename);		// пока только для чтения!
		if (f)
			return new mem_file(f);
		return NULL;
	}


	//------------------------------------------------------------------
	uLong ZCALLBACK fread_file_func (
	voidpf opaque,
	voidpf stream,
	void* buf,
	uLong size)
	{
		return ((file*)stream)->read_buffer(buf, size);
	}

	//------------------------------------------------------------------
	uLong ZCALLBACK fwrite_file_func(
	voidpf opaque,
	voidpf stream,
	const void* buf,
	uLong size)
	{
		ERROR("Can't write to zip");
		return 0;
	}

	//------------------------------------------------------------------
	long ZCALLBACK ftell_file_func (
	voidpf opaque,
	voidpf stream)
	{
		return ((file*)stream)->tell();
	}

	//------------------------------------------------------------------
	long ZCALLBACK fseek_file_func (
	voidpf opaque,
	voidpf stream,
	uLong offset,
	int origin)
	{
		switch (origin)
		{
		case ZLIB_FILEFUNC_SEEK_CUR :
			((file*)stream)->pos_shift(offset);
			break;

		case ZLIB_FILEFUNC_SEEK_END :
			((file*)stream)->pos_set(((file*)stream)->size()-offset);
			break;

		case ZLIB_FILEFUNC_SEEK_SET :
			((file*)stream)->pos_set(offset);
			break;
		}
		return 0;
	}

	//------------------------------------------------------------------
	int ZCALLBACK fclose_file_func (
	voidpf opaque,
	voidpf stream)
	{
		delete (file*)stream;
		return 0;
	}

	//------------------------------------------------------------------
	int ZCALLBACK ferror_file_func (
	voidpf opaque,
	voidpf stream)
	{
		return NULL;
	}

}	// namespace mpp


void fill_fopen_filefunc (zlib_filefunc_def* pzlib_filefunc_def)
{
	pzlib_filefunc_def->zopen_file = mpp::fopen_file_func;
	pzlib_filefunc_def->zread_file = mpp::fread_file_func;
	pzlib_filefunc_def->zwrite_file = mpp::fwrite_file_func;
	pzlib_filefunc_def->ztell_file = mpp::ftell_file_func;
	pzlib_filefunc_def->zseek_file = mpp::fseek_file_func;
	pzlib_filefunc_def->zclose_file = mpp::fclose_file_func;
	pzlib_filefunc_def->zerror_file = mpp::ferror_file_func;
    pzlib_filefunc_def->opaque = NULL;
}
