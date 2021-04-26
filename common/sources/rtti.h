#ifndef _MPP_RTTI_
#define _MPP_RTTI_

#include "string.h"
//=========================================================================
//  �������� RTTI ����������� ������ ��������� ���������� ������ ������� 
//  �� ����������
//=========================================================================

/*
	������������� RTTI. �����, ���������� � ������� ����� ����� 
	������ ���� ����������� �� rtti::base � ���������������
	rtti::register_class<some_class>("some_class", "base_class");
	���������, ��� rtti ������ ��������� ������������� ������������ ����� �����

	����� ����:
    	����� ������ ��� �������
    	����� ������� ������ �� ����� ������ � creation_options
	    ������ �/� ����������� (����� clone() - ���������� ������ ����� ������� - �/� ���������� ����������� �����)
		������������ ������������ ���������� ����
*/

namespace mpp{  namespace rtti
{
    enum creation_options
    {
        CREATE_EMPTY,
      	CREATE_BY_NAME,
	    CLONE_EXISTING,
		CREATE_FROM_FILE,
    };


	typedef void *vtable_ptr;
  
	//================================================================
	//  ������� ����� ��� rtti-������
	//================================================================
	class base
  	{
	public:
    	virtual bool create(creation_options options, void *data) = 0;  // ����� �������� RTTI-������
    	virtual ~base(){}

    	string get_class_name();
      	base *clone();
    	bool register_rtti_class(const string &name, const string &base_class, int size);
    	bool cast_rtti_class(vtable_ptr src);
    	vtable_ptr  get_rtti_vtable();
	};


	//===============================================================
	//  ����������� rtti-�������. ������������� ��������� � �����
	//===============================================================
	template<class T>
	class register_class
	{
	public:
		inline register_class(const string &name, const string &base_class="")
		{
			T   new_class;
			new_class.register_rtti_class(name, base_class, sizeof(T));
		}
	};

	//===============================================================
	//  ������ dynamic_cast. rtti::cast<dst_type>(src_type)
	//===============================================================
	template <class dst>
	dst cast(base *src)
	{
		dst	*info_object=new dst;
        if (info_object->cast_rtti_class(src->get_rtti_vtable()))
        	return (dst*)src;
		return NULL;
	}


	//================================================================
	// ��������� �������� �������
	base *create(const string &name, creation_options options=CREATE_BY_NAME, void *data=NULL);
}}  // rtti, mpp

#endif //_MPP_RTTI_
