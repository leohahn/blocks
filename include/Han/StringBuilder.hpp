#pragma once

#include "Han/MallocAllocator.hpp"
#include "Han/Collections/String.hpp"
#include "Han/Utils.hpp"

class StringBuilder
{
public:
	StringBuilder(Allocator* alloc, int capacity = 64)
		: _alloc(alloc)
	{
		_str.Reserve(capacity);
	}

	StringBuilder(int capacity = 64)
		: StringBuilder(MallocAllocator::Instance(), capacity)
	{}

	StringBuilder& operator<<(char c)
	{
		_str.Append(c);
		return *this;
	}

	StringBuilder& operator<<(const StringView& s)
	{
		_str.Append(s);
		return *this;
	}

	StringBuilder& operator<<(int i)
	{
		_str.Append(Utils::ToString(i));
		return *this;
	}

	StringBuilder& operator<<(float f)
	{
		_str.Append(Utils::ToString(f));
		return *this;
	}

	String ToString() { return std::move(_str); }

private:
	Allocator* _alloc;
	String _str;
};