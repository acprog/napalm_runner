#ifndef _OGG_WRAP_
#define _OGG_WRAP_

#ifndef NULL
	#define NULL 0
#endif


#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

extern int errno;

#ifndef SEEK_SET
	#define SEEK_SET    0
	#define SEEK_CUR    1
	#define SEEK_END    2
#endif

extern "C"
{
	void *my_realloc(void **p, int size);
	void my_memset(void *buff, unsigned int value, int size);
	void my_memcpy(void *dst, void *src, int size);
	int my_memcmp(char *dst, char *src, int size);
	int my_strlen(const char *s);
	char *my_strcpy(char *dst, const char *src);
	char *my_strcat(char *dst, const char *src);
	void my_swap (char *a, char *b, long width );
	void my_shortsort (char *lo, char *hi, long width,int (*comp)(const void *, const void *) );
	void my_qsort (void *base, long num, long width, int (*comp)(const void *, const void *) );
	void * my_memchr (const void * buf, int chr, long cnt);
	double my_sin(double x);
	double my_log(double x);
	double my_pow(double x, double y);
}


//=============================================================================
inline void *my_malloc(int size)
{
	return new char[size];
}


//=============================================================================
inline void *my_calloc(int size, int size_el)
{
	void *p=new char[size*size_el];
	my_memset(p, 0, size*size_el);
	return p;
}



//=============================================================================
inline void my_free(void *p)
{
	delete p;
}





//=============================================================================
inline void *my_alloca(int size)
{
	return NULL;
}


//=============================================================================
inline void my_exit(int)
{
}


//========================================================
inline void my_memmove(void *dst, void *src, int size)
{
	my_memcpy(dst, src, size);
}


//========================================================
inline double my_floor(double x)
{
	return (int)x;
}


//========================================================
inline double my_ldexp(double x, int n)
{
	if (n>0)
		return x*(2<<n);
	else
		return x/=(2<<(-n));
}

//========================================================
inline double my_cos(double x)
{
	return my_sin(x+M_PI/2.0);
}



//========================================================
inline float my_fabs(float x)
{
	return x<0.0f ? 0.0f-x : x;
}


//========================================================
inline double my_abs(double x)
{
	return x<0.0 ? 0.0-x : x;
}

//========================================================
inline long my_labs(long x)
{
	return x<0 ? 0-x : x;
}


//=============================================================================
inline int my_toupper(int c)
{
	return c;// âîîáùå òóò ïåğåâîä â âåğõíé ğåãèñòğ
}




//========================================================
inline double my_atan(double x)
{
	return 0;
}


//========================================================
inline double my_sqrt(double x)
{
	return 0;
}


//========================================================
inline double my_frexp(double x, int *expptr)
{
	return 0;
}



//========================================================
inline double my_acos(double x)
{
	return 0;
}


//========================================================
inline double my_exp(double x)
{
	return 0;
}


//========================================================
inline double my_ceil(double x)
{
	return 0;
}


#endif