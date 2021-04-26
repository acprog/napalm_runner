/* naPalm Runner
  Copyright (C) 2006
  Author: Alexander Semenov <acmain@gmail.com>
*/
#ifndef _MPP_VPL_H_
#define _MPP_VPL_H_

namespace mpp{ namespace VPL{
	
	class link;
	class block;
	class connector;

	// типы соединений
	enum types
	{
		// простые типы
		BOOL,
		REAL,
		STRING, // у строк имеет смысл завести параметр hash (uint_32) по которому их между собой сравнивать
        VECTOR, // x, y, z
        MATRIX, // 3x4

        // табличные манипуляторы
        TABLE_SELECTOR,
        TABLE_CREATOR,

        // фильтр евентов форм
        FORM_FILTER,
	};

	//=============================================
	// связь между блоками
	//=============================================
	class link
	{
	public:
		link(connector *source, connector *target);
		virtual ~link();
		
   	private:
		connector *source, *target;
	};


	//=============================================
	// коннектор (различает разные типы связей)
	//=============================================
	class connector
	{
	public:
		connector(block *parent, types type, bool input);

		void add_link(link *new_link);
		bool remove_link(link *new_link);
		
		block *get_parent() { return parent; };
		string get_name();
		bool is_type(int t) { return type==t; }

	private:
		block 		*parent;
		types   	type;
		bool		input;
		list<link*>	links;
	};

	//=============================================
	// абстрактный блок
	//=============================================
	class block
	{
	public:
		
	private:
		
	};
}}

#endif // _MPP_VPL_H_
