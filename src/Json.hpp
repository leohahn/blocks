#pragma once

#include <stdint.h>
#include "Allocator.hpp"
#include "Collections/RobinHashMap.hpp"
#include "Collections/Array.hpp"
#include "Collections/String.hpp"

namespace Json {

enum class Type
{
    Integer = 0,
    Real,
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
        int64_t integer;
        double real;
        Array<Val> array;
        RobinHashMap<String, Val> object;
        bool boolean;

        TypeValues(): integer(0) {}
        ~TypeValues() {}
    } values;
    
    Val(Allocator* allocator, const char* str)
        : type(Type::String)
    {
        values.string = String(allocator, str);
    }
    
    Val()
        : type(Type::Null)
    {}
    
    Val(double real)
        : type(Type::Real)
    {
        values.real = real;
    }
    
    Val(int64_t integer)
        : type(Type::Integer)
    {
        values.integer = integer;
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
            case Type::Integer:
                values.integer = other.values.integer;
                break;
            case Type::Real:
                values.real = other.values.real;
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
            case Type::Null:
                // do nothing
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
        , root_val()
    {}

    void Parse(const char* json_str);
    void Parse(uint8_t* data, size_t size);
    bool HasParseErrors() const { return !parse_error.IsEmpty(); }
    const char* GetErrorStr() const { return parse_error.data; }
};

}

