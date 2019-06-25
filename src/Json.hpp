#pragma once

#include <stdint.h>
#include "Allocator.hpp"
#include "Collections/RobinHashMap.hpp"
#include "Collections/Array.hpp"
#include "Collections/String.hpp"

namespace Json {

enum class Type
{
    Number,
    String,
    Object,
    Boolean,
    Array,
    Null,
};

struct Val
{
    Type type;
    union TypeValues
    {
        String string;
        int64_t number;
        Array<Val> array;
        RobinHashMap<String, Val> object;
        bool boolean;

        TypeValues(): number(0) {}
        ~TypeValues() {}
    } values;

    Val(Allocator* allocator, const char* str)
        : Val(allocator, Type::String)
    {
        values.string.Append(str);
    }

    Val(Allocator* allocator, Type type)
        : type(type)
    {
        switch (type) {
            case Type::Number:
                values.number = 0;
                break;
            case Type::String:
                values.string = String(allocator, "");
                break;
            case Type::Object:
                values.object = RobinHashMap<String, Val>(allocator, 4);
                break;
            case Type::Boolean:
                values.boolean = false;
                break;
            case Type::Array:
                values.array = Array<Val>(allocator);
                break;
            default:
                // ignore
                break;
        }
    }

    ~Val()
    {
        switch (type) {
            case Type::Array:
                values.array.~Array();
                break;
            case Type::Object:
                values.object.~RobinHashMap();
                break;
            case Type::String:
                values.string.~String();
                break;
            default:
                // ignore
                break;
        }
    }

    // move operators
    Val(Val&& other)
    {
        *this = std::move(other);
    }

    Val& operator=(Val&& other)
    {
        type = other.type;

        switch (other.type) {
            case Type::Number:
                values.number = other.values.number;
                break;
            case Type::String:
                values.string = std::move(other.values.string);
                break;
            case Type::Object:
                values.object = std::move(other.values.object);
                break;
            case Type::Boolean:
                values.boolean = other.values.boolean;
                break;
            case Type::Array:
                values.array = std::move(other.values.array);
                break;
        }

        other.type = Type::Null;
        return *this;
    }
};

struct Document
{
    Allocator* allocator;
    Val root_val;
    String parse_error;

    Document(Allocator* allocator)
        : allocator(allocator)
        , root_val(allocator, Type::Null)
    {}

    void Parse(const char* json_str);
    bool HasParseErrors() const { return !parse_error.IsEmpty(); }
    const char* GetErrorStr() const { return parse_error.data; }
};

}

