#ifndef _MPP_RTTI_
#define _MPP_RTTI_

#include "string.h"
//=========================================================================
//  механизм RTTI позвол€ющий заодно создавать экземпл€ры нужных классов 
//  по параметрам
//=========================================================================

/*
	использование RTTI. класс, информацию о котором нужно знать 
	должен быть производным от rtti::base и зарегистрирован
	rtti::register_class<some_class>("some_class", "base_class");
	считаетс€, что rtti классы исключают множественное наследование между собой

	после чего:
    	можно узнать им€ объекта
    	можно создать объект по имени класса и creation_options
	    объект м/б склонирован (метод clone() - возвращает точную копию объекта - д/б определены конструторы копии)
		использовать динамическое приведение типа
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
	//  базовый класс дл€ rtti-класса
	//================================================================
	class base
  	{
	public:
    	virtual bool create(creation_options options, void *data) = 0;  // метод создани€ RTTI-класса
    	virtual ~base(){}

    	string get_class_name();
      	base *clone();
    	bool register_rtti_class(const string &name, const string &base_class, int size);
    	bool cast_rtti_class(vtable_ptr src);
    	vtable_ptr  get_rtti_vtable();
	};


	//===============================================================
	//  регистратор rtti-классов. рекомендуетс€ создавать в стеке
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
	//  аналог dynamic_cast. rtti::cast<dst_type>(src_type)
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
	// интерфейс создани€ классов
	base *create(const string &name, creation_options options=CREATE_BY_NAME, void *data=NULL);
}}  // rtti, mpp

#endif //_MPP_RTTI_
