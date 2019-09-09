#pragma once

#include "Han/Allocator.hpp"
#include "Han/Core.hpp"
#include "Han/Collections/StringView.hpp"
#include <assert.h>

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
        assert(allocator);
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
        
        assert(len < cap);
        data[len] = 0;
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
