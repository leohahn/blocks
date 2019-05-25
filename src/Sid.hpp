#pragma once

#include <cstdint>
#include "Collections/RobinHashMap.hpp"

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

class Sid
{
public:
    Sid()
        : _hash(0)
        , _str(nullptr)
    {}

    Sid(const char* str, uint64_t hash)
        : _hash(hash)
        , _str(str)
    {}

    bool operator==(const Sid& other) const
    {
#ifndef NDEBUG
        if (other._hash != _hash) {
            return false;
        } else {
            // the hashes are equal, it means that the string should be equal,
            // otherwise a hash collision has occurred.
            assert(strcmp(_str, other._str) == 0);
            return true;
        }
#else
        // In non debug mode we just ignore runtime checking for hash collisions.
        return other._hash == _hash;
#endif
    }

    bool operator!=(const Sid& other) const
    {
        return !operator==(other);
    }

    uint64_t Hash() const { return _hash;  }
    const char* Str() const { return _str;  }

private:
    uint64_t _hash;
    const char* _str;
};

namespace std
{
    template<> struct hash<Sid>
    {
        size_t operator()(const Sid& s) const noexcept
        {
            return static_cast<size_t>(s.Hash());
        }
    };
}

