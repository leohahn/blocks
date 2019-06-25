#pragma once

#include <cstdint>
#include "Collections/String.hpp"
#include "Collections/RobinHashMap.hpp"
#include "Logger.hpp"

#ifndef SID
#define SID(x) Sid((x), MakeStringHash((x)))
#endif

static constexpr uint64_t
MakeStringHash(const char* str)
{
    // TODO: replace this with a better hash function
    const char* c_str = str;
    uint64_t hash = 5381;
    int c = 0;
    while ((c = *c_str++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

class SidDatabase
{
public:
    static constexpr int kDatabaseSize{ 2048 };

    static void Initialize(Allocator* allocator);
    static void Terminate();

    SidDatabase(Allocator* allocator)
        : allocator(allocator)
        , _strings(allocator, kDatabaseSize)
    {}

    ~SidDatabase()
    {
        assert(allocator == nullptr);
    }

    void Create()
    {
        _strings.Create();
    }

    void Destroy()
    {
        allocator = nullptr;
    }

    void AddHash(uint64_t hash, const char* str)
    {
        assert(allocator);
        String* entry = _strings.Find(hash);
        if (entry) {
            if (*entry != str) {
                // ignore, since the string already exists
                LOG_ERROR("String %s has the same hash as already interned string %s", str, entry->data);
                assert(false);
            }
        } else {
            _strings.Add(hash, String(allocator, str));
        }
    }

    const char* FindStr(uint64_t hash) const
    {
        const String* str = _strings.Find(hash);
        if (str) {
            return str->data;
        } else {
            return nullptr;
        }
    }

public:
    Allocator* allocator;

private:
    RobinHashMap<uint64_t, String> _strings;
};

extern SidDatabase* g_debug_sid_database;

class Sid
{
public:
    Sid()
        : _hash(0)
    {}

    Sid(const char* str, uint64_t hash)
        : _hash(hash)
    {
        assert(g_debug_sid_database);
        g_debug_sid_database->AddHash(hash, str);
#ifdef _DEBUG
        _str = GetStr();
#endif
    }

    bool IsEmpty() const { return _hash == 0; }

    const char* GetStr() const
    {
        return g_debug_sid_database->FindStr(_hash);
    }

    bool operator==(const Sid& other) const
    {
        return other._hash == _hash;
    }

    bool operator!=(const Sid& other) const
    {
        return !operator==(other);
    }

    uint64_t GetHash() const { return _hash;  }

private:
    uint64_t _hash;
#ifdef _DEBUG
    const char* _str;
#endif
};

namespace std
{
    template<> struct hash<Sid>
    {
        size_t operator()(const Sid& s) const noexcept
        {
            return static_cast<size_t>(s.GetHash());
        }
    };
}

