#ifndef _MPP_APPLICATION_
#define _MPP_APPLICATION_

#include "static_class.h"
#include "list.h"
//#include "event_handler.h"
#include "mpp_events.h"
#include "xml.h"
#include "controls.h"

namespace mpp
{
	class platform;
	class timer;
	class application;


	//========================================================================
	//	Интерфейс для различных приложений
	//========================================================================
	class application
		:public static_class<application>
		,public pen_handler
		,public dispatcher<redraw>
		,public dispatcher<key_down>
		,public dispatcher<key_up>
		,public dispatcher<key_char>
		,public dispatcher<cycle>
	{
	private:
		modal	*active,
				*unused;
		int		pen_as_key;
		xml		preference;
		uint32		version;			// версия
		
	public:
		// общая информация о приложении:
		string		name;				// название
		int			cycle_frequency;	// частота цикла
	
	public:
		//------------------------------------------------------------------
		application(const string &_name, int _cycle_frequency, uint32 _version);
		~application();
		void close();	// для закрытия программы изнутри.
		void goto_modal(modal *m);
		void free_unused();
		void free_active();

		void event(cycle*, void *source);

		static	application *create();	
		static string	&prefs(const string &name, const string &default_value="", bool atribute=false);
		static xml		&prefs_folder(const string &name);

		void penkey_down(int id);
		void penkey_up();
		string	get_version() const;
	};
}//mpp
#endif /*_APPLICATION_H_*/
