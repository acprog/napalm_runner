/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_SOUND_H_
#define _MPP_SOUND_H_

#include	"mpp_prefs.h"
#include	"static_class.h"
#include	"collection.h"

struct	OggVorbis_File;

namespace mpp
{
	#ifndef WORD
		#define WORD	unsigned short
		#define DWORD	unsigned long
	#endif

	//--------------------------------------------------
	typedef struct
	{
		DWORD	riff_id,
				riff_size,
				riff_format,
				fmt_id,
				fmt_size;
		//--------------------------------------------------
		struct WAVEFORMATEX
		{ 
			WORD  wFormatTag; 
			WORD  nChannels; 
			DWORD nSamplesPerSec; 
			DWORD nAvgBytesPerSec; 
			WORD  nBlockAlign; 
			WORD  wBitsPerSample; 
		}w; 
		//--------------------------------------------------
		DWORD	data_id,
				data_size;
	}riff_header;


	//=============================================================
	//	ogg - файл
	//=============================================================
	class ogg : public file
	{
	private:
		OggVorbis_File	*vf;
		bool			read_head_first;
		long			leng;
		
		riff_header		*header(riff_header *h);

	public:
		ogg(file *f);
		~ogg();
		int	 size();
		void pos_set(int pos);
		void pos_shift(int pos);

		int read_buffer(void *buffer, int size);
		void write_buffer(const void *buffer, int size){};
		int tell();
	};


	//===============================================================
	//	«вук
	//===============================================================
	class sound
	{
	protected:
		file		*pcm;

	public:
		const static	int		infinite;
		static			sound	*empty;
		static			uint8	volume;	// в %%

		sound(file *_pcm=NULL);
		virtual ~sound();
		virtual	void play(int n_repeats=0){}
		virtual	void stop(){}
		virtual void change_volume(uint8 vol){volume=vol;}
	};



	//===============================================================
	//	Loop
	//===============================================================
	class loop : public static_class<loop>
	{
	private:
		sound	*buffer;

	public:
		static	uint8	volume;	// в %%

	public:
		//----------------------------------------------------------
		loop(collection &coll, const string &name)
			:buffer	(volume>LOOP_OFF ? coll.load_sound(name, false) : sound::empty)
		{
			if (buffer!=sound::empty)
			{
				buffer->play(sound::infinite);
				buffer->change_volume(volume);
			}
		}

		//----------------------------------------------------------
		virtual ~loop()
		{
			buffer->stop();
			if (buffer!=sound::empty)
				delete buffer;
		}

		//----------------------------------------------------------
		static	void change_volume(uint8 vol)
		{
			if (volume>100)
				volume=100;
			get().buffer->change_volume(volume=vol);
		}
	};
}

#endif