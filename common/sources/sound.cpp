/* naPalm Runner

  Copyright (C) 2006

  Author: Alexander Semenov <acmain@gmail.com>
*/
#include "sound.h"
#include "platform.h"
#include "string.h"

#include "ogg/vorbisfile.h"


namespace mpp
{
const int	sound::infinite=-1;

//===============================================================
//	Звук
//===============================================================
sound::sound(file *_pcm)
	:pcm	(_pcm)
{
}


//=============================================================================
sound::~sound()
{
	if (pcm)
		delete pcm;
}


//=============================================================================
static	int ogg_ftell(void *stream)
{
	return ((file*)stream)->tell();
}

//=============================================================================
// callbacks for ogg decoder
static	int ogg_fread(void *buff, long s, long n, void *stream)
{
	return ((file*)stream)->read_buffer(buff, s*n);
}


//=============================================================================
static	int ogg_fseek(void *stream, long offset, int origin)
{
	switch (origin)
	{
	case SEEK_CUR :
		((file*)stream)->pos_shift(offset);
		break;

	case SEEK_END :
		((file*)stream)->pos_set(((file*)stream)->size()-offset);
		break;

	case SEEK_SET :
		((file*)stream)->pos_set(offset);
		break;
	}
	return 0;
}



//=============================================================================
static	int ogg_fclose(void *stream)
{
	delete (file*)stream;
	return 0;
}







//=============================================================
//	ogg - файл
//=============================================================
ogg::ogg(file *src)
	:vf					(NULL)
	,read_head_first	(true)
	,leng				(0)
{
	if (!src)	// вообще такого быть не должно - ogg файл должен формироваться только по существующему файлу
		ERROR("OGG source unexist");

	vf=new OggVorbis_File;
	
	ov_callbacks callbacks =
	{
	    (long (*)(void *, long, long, void *))  ogg_fread,
	    (int (*)(void *, long, int))                  ogg_fseek,
	    (int (*)(void *))                             ogg_fclose,
	    (long (*)(void *))                            ogg_ftell
	};

	if (ov_open_callbacks(src, vf, NULL, 0, callbacks)<0)
	{
		ERROR("Input does not appear to be an Ogg bitstream.");
		return;
	}

	leng=size();
}

//=============================================================================
ogg::~ogg()
{
	ov_clear(vf);
	delete vf;
}


//=============================================================================
void ogg::pos_set(int pos)
{
	leng=size()-pos;
	if (pos==0)
		read_head_first=true;
	else
		pos-=sizeof(riff_header);
	
	ov_pcm_seek(vf, pos);
}


//=============================================================================
void ogg::pos_shift(int pos)
{
	ERROR("ogg: shift pos not supported");
}



//=============================================================================
int ogg::tell()
{
	ERROR("ogg: tell()");
	return 0;
}



//=============================================================================
int ogg::read_buffer(void *buffer, int size)
{
	if (leng<=0)
		return 0;

	if (size>leng)
		size=leng;

	if (read_head_first)
	{
		read_head_first=false;
		riff_header	*h=(riff_header*)buffer;
		if (sizeof(*h)!=size)
			ERROR("ogg: header size");
		header(h);
		leng-=size;
		return size;
	}

	int		current_section;	// возможно можно использовать в tell

//	int all=ov_read(vf, (char*)buffer, size, 0, 2, 1, &current_section);
	
	int		rd,
			all=0;
	char	*p=(char*)buffer;
	while( size>0 )
	{
		rd=ov_read(vf, p, size, 0, 2, 1, &current_section);
		if (rd<=0)
			break;
		p+=rd;
		all+=rd;
		size-=rd;
	}

	leng-=all;
	if (leng<0)
		all+=leng;

	return all;
}



//=============================================================================
riff_header *ogg::header(riff_header *h)
{
	vorbis_info *vi=ov_info(vf,-1);

	if (vi->bitrate_lower!=vi->bitrate_upper)
		ERROR("Ogg with VBR no support");

	h->riff_id='FFIR';
	h->riff_format='EVAW';
	h->fmt_id=' tmf';
	h->fmt_size=16;
	h->w.wFormatTag=0x0001;
	h->w.nChannels=vi->channels;
	h->w.nSamplesPerSec=vi->rate;
	h->w.wBitsPerSample=16;
	h->w.nBlockAlign=h->w.nChannels*h->w.wBitsPerSample/8;
	h->w.nAvgBytesPerSec=h->w.nSamplesPerSec*h->w.nBlockAlign;
	h->data_id='atad';
	h->data_size=(long)ov_pcm_total(vf,-1)*h->w.wBitsPerSample*vi->channels/8;
	h->data_size-=0.135*(h->w.nSamplesPerSec*h->w.wBitsPerSample*vi->channels/8);
	h->riff_size=h->data_size+sizeof(h)-8;
	return h;
}



//=============================================================================
int	ogg::size()
{
	vorbis_info *vi=ov_info(vf,-1);
	return sizeof(riff_header)+(long)(ov_pcm_total(vf,-1)-0.135*(vi->rate))*16*vi->channels/8;//-0.135*(vi->rate*16*vi->channels/8);
}


}	// namespace mpp






//===============================================================
void *my_realloc(void **p, int size)
{
	return mpp::platform::get().realloc(p, size);
}






#ifndef M_PI
#	define M_PI ((float)3.141592653589793)
#endif

#ifndef M_PI4
#	define M_PI4 (M_PI*0.25F)
#endif

#ifndef M_PI2
#	define M_PI2 (M_PI*0.5F)
#endif

/* sine and cosine within 0-PI/4. MFRAC=4 optimized for single precision */
#define MFRAC 4
void _Sico(float arg, float *sine, float *cosi) {
  int n,n2;
  float arg2,t;
  /* calculate tangent by continuous fraction */
  t=0.; arg*=0.5F; arg2=arg*arg; n=MFRAC-1; n2=(n<<1)+1;
  for(;n>=0;n--) {
    if(n>0) t=arg2/(n2-t);
    else t=arg/(1.F-t);
    n2--; n2--;
  }
  /* sine and cosine */
  arg=t*t; arg2=arg+1.F; arg2=1.F/arg2;
  *sine=t*arg2; *sine+=(*sine);
  *cosi=1.F-arg; *cosi*=arg2;
}

/* argument 0-PI/2 */
void Sico(float arg, float *sine, float *cosi) {
  if(arg<=M_PI4) _Sico(arg,sine,cosi);
  else _Sico(M_PI2-arg,cosi,sine);
}



/* first period: -PI<=arg<=PI */
void Sico1p(float arg, float *sine, float *cosi) 
{
	while (arg<-M_PI)
		arg+=2*M_PI;
	while (arg>M_PI)
		arg-=2*M_PI;



  int s=0;
  if(arg<0.F)
  {
	  arg=-arg;
	  s=1;
  }
  if(arg<=M_PI2)
	  Sico(arg,sine,cosi);
  else
  {
    Sico(arg-M_PI2,cosi,sine);
    *cosi=-(*cosi);
  }
  if(s) *sine=-(*sine);
}


//========================================================
double my_sin(double arg)
{
//	LOG("sin");
	float sine, cosi;
	while (arg<-M_PI)
		arg+=2*M_PI;
	while (arg>M_PI)
		arg-=2*M_PI;

	int s=0;
	if(arg<0.F)
	{
		arg=-arg;
		s=1;
	}
	if(arg<=M_PI2)
		Sico(arg,&sine,&cosi);
	else
		Sico(arg-M_PI2,&cosi,&sine);
	if(s)
		sine=-sine;
	return sine;
}








/* 4 iterations needed for the single precision */ 
#define ITNUM 4
//========================================================
float sqroot(float x)
{ 
//	LOG("sqroot");

  int sp=0,i,inv=0; 
  float a,b; 
  if(x<=0.F) return(0.F); 
  /* argument less than 1 : invert it */ 
  if(x<1.F) {x=1.F/x;inv=1;} 
  /* process series of division by 16 until argument is <16 */ 
  while(x>16.F) {sp++;x/=16.F;} 
  /* initial approximation */ 
  a=2.F; 
  /* Newtonian algorithm */ 
  for(i=ITNUM;i>0;i--) { 
    b=x/a; a+=b; a*=0.5F; 
  } 
  /* multiply result by 4 : as much times as divisions by 16 took place */ 
  while(sp>0) {sp--;a*=4.F;} 
  /* invert result for inverted argument */ 
  if(inv) a=1.F/a; 
  return(a); 
}



//========================================================
double my_pow(double x, double y)
{
//	LOG("pow");

	if (y>1.0)
		while (y!=1.0)
		{
			y/=2.0;
			x*=x;
		}
	else
		while (y!=1.0)
		{
			y*=2.0;
			x=sqroot(x);
		}
	
	return x;
}



//========================================================
double my_log(double x)
{
//	LOG("log");

	// ищем степень 2-ки
	int	n=1,
		i=x;
	while (i>2)
	{
		i>>=1;
		++n;
	}

	return 0.301029996*(double)n;
}
