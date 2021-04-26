#include "rtti.h"

#include "list.h"

namespace mpp{  namespace rtti
{
    typedef bool (*creation_method)(base *object, creation_options options, void *data);	

	//================================================================
	//  информация о классе
	//================================================================
	struct class_info
	{
		string  	name;
    	int   		size;
        vtable_ptr  vtable;
        class_info	*base_class;
	};

	static list<class_info>	classes;



	//================================================================
	//  базовый класс для rtti-класса
	//================================================================
   	vtable_ptr  base::get_rtti_vtable()
   	{
   		return *((vtable_ptr*)this);
   	}

	//================================================================
   	string base::get_class_name()
   	{
   		while (class_info *info=classes.each_ptr())
   			if (info->vtable==get_rtti_vtable())
   				return info->name;
   		return "";
   	}


	//================================================================
	bool base::register_rtti_class(const string &name, const string &base_class, int size)
	{
		if (name!="" && get_class_name()=="")
		{
			class_info  new_info;
			new_info.name=name;
			new_info.size=size;
			new_info.vtable=get_rtti_vtable();
            new_info.base_class=NULL;

   			// found base
			if (base_class!="")
			{
		   		while (class_info *info=classes.each_ptr())
   					if (info->name==base_class)				
   					{
   						new_info.base_class=info;
   						classes.break_each();
   						break;
					}
				if (!new_info.base_class)
					return false;
			}
			classes.push_back(new_info);
			return true;
		}
		return false;
	}


	//================================================================
  	base *base::clone()
  	{
		// не используем конструктор копии т.к. он может быть приватным
		return mpp::rtti::create(get_class_name(), CLONE_EXISTING, this);
		// нужно переписать на поиск по vtable -так много быстрее
  	}


	//================================================================
	// интерфейс создания классов
   	base *create(const string &name, creation_options options, void *data)
   	{
   		while (class_info *info=classes.each_ptr())
   			if (info->name==name)
   			{
   				char	*buffer=new char[info->size];
				*((vtable_ptr*)buffer)=info->vtable;
				base 	*object=(base*)buffer;
				creation_method	constructor=*(creation_method*)info->vtable;
				if (!constructor(object, options, data))
				{
					delete buffer;
					return NULL;
				}
				return object;
   			}   		
   		return NULL;
   	}


	//================================================================
   	bool base::cast_rtti_class(vtable_ptr src)
   	{
   	/*
   		while (class_info *info=classes.each_ptr())
   			if (info->vtable==get_vtable())
   				return info->cast(src);
    */
    	return false;
   	}
}}
