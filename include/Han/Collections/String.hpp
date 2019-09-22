#pragma once

#include "Han/Allocator.hpp"
#include "Han/MallocAllocator.hpp"
#include "Han/Collections/StringView.hpp"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static constexpr const size_t kStringInitialCapacity = 8;
static constexpr const float kStringGrowthFactor = 1.5f;

struct String
{
    Allocator* allocator;
    size_t cap;
    size_t len;
    char* data;

    String()
        : String(MallocAllocator::Instance())
    {}

    explicit String(Allocator* allocator)
        : allocator(allocator)
        , cap(0)
        , len(0)
        , data(nullptr)
    {}

    String(Allocator* allocator, const char* contents)
        : allocator(allocator)
        , cap(0)
        , len(0)
        , data(nullptr)
    {
        Append(contents);
    }

    explicit String(const char* contents)
		: String(MallocAllocator::Instance(), contents)
    {
    }
    
    String(Allocator* allocator, const StringView& contents)
        : allocator(allocator)
        , cap(0)
        , len(0)
        , data(nullptr)
    {
        Append(contents);
    }

    String(const StringView& contents)
        : String(MallocAllocator::Instance(), contents)
    {
    }

    String(const String& str)
        : String()
    {
        assert(str.allocator);
        *this = str;
    }

    String(String&& str)
        : String()
    {
        *this = std::move(str);
    }

    // Copy assignment
    String& operator=(const String& str)
    {
        assert(this != &str);
        assert(str.allocator);
        if (data) {
            allocator->Deallocate(data);
        } 

        ShallowCopyFields(str);
        // copy the contents as well
        data = (char*)allocator->Allocate(str.len + 1);
        assert(data);
        memcpy(data, str.data, str.len);
        data[len] = '\0';
        return *this;
    }

    // Move assignment
    String& operator=(String&& str)
    {
        assert(this != &str);
        this->~String();
        ShallowCopyFields(str);
        str.allocator = nullptr;
        str.data = nullptr;
        str.len = 0;
        str.cap = 0;
        return *this;
    }

    ~String()
    {
        if (data) {
            allocator->Deallocate(data);
        }
        cap = 0;
        len = 0;
        data = nullptr;
        allocator = nullptr;
    }

	void Reserve(int capacity)
	{
		Resize(capacity);
	}

    String& Append(char character)
    {
        assert(data);
        size_t new_len = len + 1;
        while (new_len > (cap - 1)) {
			size_t new_cap = (size_t)(cap * kStringGrowthFactor);
            Resize(new_cap);
        }
        assert(cap > len);
        data[len] = character;
        data[len+1] = 0;
        len = new_len;
		return *this;
    }

    String& Append(const String& str) { return Append(str.data, str.len); }

    String& Append(const char* str) { return Append(str, strlen(str)); }

    String& Append(const StringView& str) { return Append(str.data, str.len); }

    String& Append(const StringView& str, size_t index_offset)
    {
        assert(index_offset < str.len);
        return Append(str.data + index_offset, str.len - index_offset);
    }

    String& Append(const char* begin, const char* end)
    {
        assert((size_t)begin < (size_t)end);
        return Append(begin, (size_t)(end - begin));
    }

    String& Append(const uint8_t* begin, const uint8_t* end)
    {
        assert((size_t)begin < (size_t)end);
        return Append((const char*)begin, (size_t)(end - begin));
    }

    String& Append(const char* str, size_t str_len)
    {
        assert(str);
        if (data) {
            size_t new_len = len + str_len;
            while (new_len > (cap - 1)) {
				size_t new_cap = (size_t)(cap * kStringGrowthFactor);
                Resize(new_cap);
            }
            memcpy(data + len, str, str_len);
            len = new_len;
        } else {
            data = (char*)allocator->Allocate(str_len + 1);
            assert(data);
            memcpy(data, str, str_len);
            len = str_len;
            cap = str_len;
        }
        data[len] = 0; // add null terminator
		return *this;
    }

    char Back() const { return data[len - 1]; }

    char Front() const { return data[0]; }

    char& operator[](size_t index) { return data[index]; }

    char operator[](size_t index) const { return data[index]; }

    bool operator==(const String& str)
    {
        if (len != str.len) {
            return false;
        }
        return memcmp(data, str.data, len) == 0;
    }

    bool operator!=(const String& str)
    {
        return !operator==(str);
    }
    
    bool IsEmpty() const { return len == 0; }

    StringView View() const { return StringView(data, len); }

private:
    void Resize(int new_cap)
    {
		if (new_cap <= cap) {
			return;
		}

        // Need to allocate more memory
        char* new_data = (char*)allocator->Allocate(new_cap);
        assert(new_data);
        memcpy(new_data, data, len);
        new_data[len] = 0;
        allocator->Deallocate(data);
        data = new_data;
        cap = new_cap;
    }

    void ShallowCopyFields(const String& str)
    {
        allocator = str.allocator;
        cap = str.cap;
        len = str.len;
        data = str.data;
    }
};

inline bool
operator==(const String& lhs, const StringView& rhs)
{
    if (lhs.len != rhs.len) {
        return false;
    }
    return memcmp(lhs.data, rhs.data, rhs.len) == 0;
}

inline bool
operator==(const StringView& lhs, const String& rhs)
{
    return rhs == lhs;
}

inline bool
operator!=(const String& lhs, const StringView& rhs)
{
    return !(lhs == rhs);
}

inline bool
operator!=(const StringView& lhs, const String& rhs)
{
    return !(lhs == rhs);
}

namespace std
{
    template<> struct hash<String>
    {
        size_t operator()(const String& s) const noexcept
        {
            // http://www.cse.yorku.ca/~oz/hash.html
            const char* c_str = s.data;
            size_t h = 5381;
            int c;
            while ((c = *c_str++))
                h = ((h << 5) + h) + c;
            return h;
        }
    };
}
