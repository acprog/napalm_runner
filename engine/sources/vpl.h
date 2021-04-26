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

	// ���� ����������
	enum types
	{
		// ������� ����
		BOOL,
		REAL,
		STRING, // � ����� ����� ����� ������� �������� hash (uint_32) �� �������� �� ����� ����� ����������
        VECTOR, // x, y, z
        MATRIX, // 3x4

        // ��������� ������������
        TABLE_SELECTOR,
        TABLE_CREATOR,

        // ������ ������� ����
        FORM_FILTER,
	};

	//=============================================
	// ����� ����� �������
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
	// ��������� (��������� ������ ���� ������)
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
	// ����������� ����
	//=============================================
	class block
	{
	public:
		
	private:
		
	};
}}

#endif // _MPP_VPL_H_
