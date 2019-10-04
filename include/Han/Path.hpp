#pragma once

#include "Han/Allocator.hpp"
#include "Han/Core.hpp"
#include "Han/Collections/StringView.hpp"
#include <stdint.h>

#if _WIN32
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

struct Path
{
    Allocator* allocator;
    char* data;
    size_t cap;
    size_t len;
    
public:
    Path()
        : Path(nullptr)
    {}

    Path(Allocator* allocator)
        : Path(allocator, StringView())
    {}

    Path(Allocator* allocator, StringView path)
        : allocator(allocator)
        , data(nullptr)
        , cap(0)
        , len(0)
    {
        if (!path.IsEmpty()) {
            Push(path);
        }
    }
    
    Path(const Path& other_path) = delete;
    Path& operator=(const Path& other_path) = delete;
    
    Path(Path&& other_path)
        : allocator(nullptr)
        , data(nullptr)
        , cap(0)
        , len(0)
    {
        *this = std::move(other_path);
    }
    
    ~Path()
    {
        if (data) {
            allocator->Deallocate(data);
        }
    }
    
    Path& operator=(Path&& other_path)
    {
        allocator = other_path.allocator;
        cap = other_path.cap;
        len = other_path.len;
        if (data) {
            allocator->Deallocate(data);
        }
        data = other_path.data;
        other_path.allocator = nullptr;
        other_path.data = nullptr;
        other_path.len = 0;
        other_path.cap = 0;
        return *this;
    }
    
    void Push(const Path& other_path)
    {
        Push(other_path.data, other_path.len);
    }

    void Push(const StringView& str)
    {
        Push(str.data, str.len);
    }

    void Push(const char* str)
    {
        Push(str, strlen(str));
    }
    
    void Push(const char* str_data, size_t str_len)
    {
        ASSERT(allocator, "Allocator should not be null");
        if (str_len == 0) {
            return;
        }
        
        // +2 here is necessary:
        //  - +1 for the null terminated string
        //  - +1 for the extra '/' we might add
        size_t needed_cap = len + str_len + 2;
        if (needed_cap > cap) {
            Resize(needed_cap);
        }
        
        if (len == 0) {
            memcpy(data, str_data, str_len);
            len = str_len;
            data[len] = 0;
            return;
        }
        
        if (data[len-1] != PATH_SEP) {
            data[len++] = PATH_SEP;
        }
        
        if (str_data[0] == PATH_SEP) {
            memcpy(data + len, str_data + 1, str_len - 1);
            len += str_len - 1;
        } else {
            memcpy(data + len, str_data, str_len);
            len += str_len;
        }
        
        ASSERT(len < cap, "Len should be smaller than cap");
        data[len] = 0;
    }

	Path GetDir() const
	{
		int64_t last_sep_index = -1;
		for (int64_t i = (int64_t)len - 1; i >= 0; --i) {
			// TODO(leo): handle different situations for Windows and Unix
			if (data[i] == '/' || data[i] == '\\') {
				last_sep_index = i;
				break;
			}
		}

		if (last_sep_index == -1) {
			return Path(allocator, ".");
		} else {
			return Path(allocator, StringView(data, last_sep_index + 1));
		}
	}

	Path Join(const StringView& str) const
	{
		if (len == 0) return Path(allocator, str);
		if (str.len == 0) return Path(allocator, data);

		char* new_data = (char*)allocator->Allocate(len + str.len + 1);
		// Copy the first path to the new memory
		memcpy(new_data, data, len);

		size_t joined_path_start;
		if (new_data[len - 1] != '/' && new_data[len - 1] != '\\') {
			new_data[len] = '/';
			joined_path_start = len + 1;
		} else {
			joined_path_start = len;
		}

		if (str.data[0] == '/' || str.data[0] == '\\') {
			memcpy(new_data + joined_path_start, str.data + 1, str.len - 1);
			new_data[joined_path_start + str.len - 1] = 0;
		} else {
			memcpy(new_data + len, str.data, str.len);
			new_data[joined_path_start + str.len] = 0;
		}

		return Path(allocator, new_data);
	}

    void Resize(size_t desired_cap)
    {
        const float factor = 1.5f;
        size_t new_cap = (size_t)HAN_MAX(cap * factor, desired_cap);
        char* buf = (char*)allocator->Allocate(new_cap);
        assert(buf);

        // copy contents
        memcpy(buf, data, len);
        buf[len] = 0;

        allocator->Deallocate(data);
        data = buf;
        cap = new_cap;
    }

    StringView GetExtension() const
    {
        return Path::GetExtension(data);
    }

    //
    // Helper functions
    //
    static StringView GetExtension(const char* path);
};
