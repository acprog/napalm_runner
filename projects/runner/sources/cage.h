#ifndef _CAGE_H_
#define _CAGE_H_

#include "resources.h"

//-------------------------------------------------------------
// ���� ���������
enum item_type
{
	key=0,			// ����
	itrap=1,		// �������
	pick=2,			// �����
	freeze=3,		// ���������
	glass=4,		// ��������������� �����������

	ibomb=5,		// �����

	closed_door=6,	// �������� �����
	inactive_door=7,// ���������� �����
	
	// ������� � ����, ������������� ��������
	active_door=8,	// �������� �����

	lo_cash=9,		// ���� �����
	med_cash=10,	// ������ �����
	hi_cash=11,		// ����� �����
	/*
	12-14 ������������ �� ����� �������������� �.�. '<','>' ��������� � xml �����
    */

	//(������������ ��� ���������)
	ibox=29,		// ����
	iplayer=30,		// ����� 
	imonster=31,	// ������ 

	no_item=32,		// ������������� ��� ������ ���������
};



//========================================================================
//	���� ������
//========================================================================
union cage
{
//private:
private:
	uint8		value;

	// ��� ������ �������������
	struct
	{
		uint8	cage	:4,	// ����� ������ �� �������
				type	:4;	// ������ ������ �� �������
	}idc;	// ������������ ������

	struct idt_tag
	{
		uint8	subtype		:2,	// ������������ ������������
				bottom		:1,	// ���������� ����� ���������� � ��������� ��������: ���
				top			:1,	//	����
				local_type	:2,	// ��������� ��� ������ (�� ����� ���������� ����������)
				near_plane	:1,	// ������� ��������� �� ������� �����
				inpassable	:1;	// ����� �������� ����� ���������
	}idt;	// ������������� ����

	//���� ����� ������
	enum group_types
	{
		far_plane_group		=0,
		near_plane_group	=1,
		items_group			=2,
		inpassable_group	=3
	};

	struct
	{
		uint8	item	:6,		// ����� ��������
				group	:2;		// ��� ������
	}idi;	// �������������� ��������


public:
	enum types
	{
		// ���������� �����������
		empty=7,		// �����, ������ ���
		decor_far=1,	// ������� ���������
		ladder=2,		// ��������
		broken=3,		// ����������� soft	���� ����� �����������
		decor_near=4,	// ������� ���������
		mirage=5,		// aka soft
		water=6,		// ����, �����. �������������
		rope=0,			// �������������� �������

		//������������ �����������
		hard=12,		// �� ��������
		slow=13,		// �� ��������, ��������� ��������
		fast=14,		// �� ��������, �������� ��������
		soft=15			// ��������
	};

public:
	//---------------------------------------------------------------
	inline cage()
		:value (empty<<4)
	{
	}

	//---------------------------------------------------------------
	inline cage(const cage &c)
		:value (c.value)
	{
	}

	//---------------------------------------------------------------
	inline cage(uint8 v)
		:value	(v)
	{
	}

	//---------------------------------------------------------------
	inline cage(item_type i)
	{
		if (i==no_item)
			erase();
		else
		{
			idi.group=items_group;
			idi.item=i;
		}
	}

	//---------------------------------------------------------------
	inline cage(types t)
	{
		erase();
		idc.type=t;
	}
	
	//---------------------------------------------------------------
	inline bool open_door()
	{
		if (*this==closed_door)
		{
			idi.item=inactive_door;
			return true;
		}
		return false;
	}

	//---------------------------------------------------------------
	inline uint8 subtype()
	{
		return idt.subtype;
	}

	//---------------------------------------------------------------
	inline bool hard_place() const
	{
		return idc.type!=rope && idc.type!=ladder;
	}

	//---------------------------------------------------------------
	inline void correct_item()
	{
		if (idi.item==no_item)
			value=(empty<<4);
	}

	//---------------------------------------------------------------
	inline bool activate_door()
	{
		if (*this==inactive_door)
		{
			idi.item=active_door;
			return true;
		}
		return false;
	}

	//---------------------------------------------------------------
	inline	bool check_pile()
	{
		return idi.group!=inpassable_group;
	}

	//---------------------------------------------------------------
	inline	bool check_up_pile()
	{
		return idi.group==inpassable_group;
	}

	//---------------------------------------------------------------
	inline void erase()
	{
		value=(empty<<4);
	}

	//---------------------------------------------------------------
	inline bool operator==(item_type i) const
	{
		if (idi.group!=items_group)
			return i==no_item;
		return idi.item==i;
	}

	//---------------------------------------------------------------
	inline bool operator!=(item_type i) const
	{
		return !((*this)==i);
	}

	//---------------------------------------------------------------
	inline bool operator==(types t) const
	{
		return idc.type==t;
	}

	//---------------------------------------------------------------
	inline bool operator!=(types t) const
	{
		return idc.type!=t;
	}

	//---------------------------------------------------------------
	inline operator item_type() const
	{
		if (idi.group!=items_group)
			return no_item;
		return (item_type)idi.item;
	}

	//---------------------------------------------------------------
	inline operator types() const
	{
		return type();
	}

	//---------------------------------------------------------------
	inline types type() const
	{
		return (types)idc.type;
	}

	//---------------------------------------------------------------
	// ������ ������������� �������
	inline bool stop_drop() const
	{
		int type=idc.type;
		return	type==ladder 
			//||	type==rope
			||	type==hard
			||	type==slow
			||	type==fast
			||	type==soft;
	}	

	//---------------------------------------------------------------
	inline bool no_pass() const
	{
		return idt.inpassable && idi.group==inpassable_group;
	}

	//---------------------------------------------------------------
	inline operator bool() const
	{
		return value!=(empty<<4);
	}

	//---------------------------------------------------------------
	inline operator!() const
	{
		return value==(empty<<4);
	}

	//---------------------------------------------------------------
	inline void broken_to_mirage()
	{
		if (*this==broken)
			idc.type=mirage;
	}

	//---------------------------------------------------------------
	inline idt_tag &join()
	{
		return idt;
	}

	//---------------------------------------------------------------
	inline bool draw_near() const
	{
		return idt.near_plane;
	}

	//---------------------------------------------------------------
	inline bool inpassable() const
	{
		return idi.group==inpassable_group;
	}

	//---------------------------------------------------------------
	inline uint8 id() const
	{
		return idc.cage;
	}

	//---------------------------------------------------------------
	inline void dig()
	{
		idc.type=broken;
	}

	//---------------------------------------------------------------
	inline void undig()
	{
		idc.type=soft;
	}

	//---------------------------------------------------------------
	void	normalize(cage &next, bool in_editor=false);


	//---------------------------------------------------------------
	inline bool burst()
	{
		if (*this==soft)
		{
			idc.type=broken;
			return true;
		}
		
		if ((*this==decor_far || *this==decor_near) && idt.subtype<2)
		{
			*this=empty;
			return false;
		}

		item_type i=*this;
		if (i>=itrap && i<=ibomb)
		{
			*this=empty;
			return false;
		}
		return false;
	}
};


#endif