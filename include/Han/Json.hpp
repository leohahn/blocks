#pragma once

#include <stdint.h>
#include "Han/Allocator.hpp"
#include "Han/Collections/RobinHashMap.hpp"
#include "Han/Collections/Array.hpp"
#include "Han/Collections/String.hpp"

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
    
    Val(String str)
        : type(Type::String)
    {
        memset(&values, 0, sizeof(TypeValues));
        values.string = std::move(str);
    }
    
    Val()
        : type(Type::Null)
    {
    }
    
    Val(double real)
        : type(Type::Real)
    {
        values.real = real;
    }

    Val(bool val)
        : type(Type::Boolean)
    {
        values.boolean = val;
    }
    
    Val(int64_t integer)
        : type(Type::Integer)
    {
        values.integer = integer;
    }

    Val(RobinHashMap<String, Val> val)
        : type(Type::Object)
    {
        memset(&values, 0, sizeof(TypeValues));
        values.object = std::move(val);
    }

    Val(Array<Val> array)
        : type(Type::Array)
    {
        memset(&values, 0, sizeof(TypeValues));
        values.array = std::move(array);
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
        memset(&values, 0, sizeof(TypeValues));
        *this = std::move(other);
    }

    Val& operator=(Val&& other)
    {
        ResetUnion();
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

    bool IsString() const { return type == Type::String; }
    const String* AsString() const
    {
        return type == Type::String ? &values.string : nullptr;
    }

    bool IsObject() const { return type == Type::Object; }
    const RobinHashMap<String, Val>* AsObject() const
    {
        return type == Type::Object ? &values.object : nullptr;
    }

    bool IsArray() const { return type == Type::Array; }
    const Array<Val>* AsArray() const
    {
        return type == Type::Array ? &values.array : nullptr;
    }

    bool IsBool() const { return type == Type::Boolean; }
    const bool* AsBool() const
    {
        return type == Type::Boolean ? &values.boolean : nullptr;
    }

    bool IsReal() const { return type == Type::Real; }
    const double* AsDouble() const
    {
        return type == Type::Real ? &values.real : nullptr;
    }

    bool IsInteger() const { return type == Type::Integer; }
    const int64_t* AsInt64() const
    {
        return type == Type::Integer ? &values.integer : nullptr;
    }

    bool TryConvertNumberToDouble(double* out_val) const
    {
        if (type == Type::Integer) {
            *out_val = (double)values.integer;
            return true;
        } else if (type == Type::Real) {
            *out_val = values.real;
            return true;
        } else {
            return false;
        }
    }

    bool TryConvertNumberToFloat(float* out_val) const
    {
        if (type == Type::Integer) {
            *out_val = (float)values.integer;
            return true;
        } else if (type == Type::Real) {
            *out_val = (float)values.real;
            return true;
        } else {
            return false;
        }
    }

    String PrettyPrint(Allocator* other_allocator = nullptr) const;

private:
    void ResetUnion()
    {
        switch (type) {
            case Type::String:
                values.string.~String();
                break;
            case Type::Object:
                values.object.~RobinHashMap();
                break;
            case Type::Array:
                values.array.~Array();
                break;
            default:
                // do nothing
                break;
        }
        memset(&values, 0, sizeof(TypeValues));
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
    String PrettyPrint(Allocator* other_allocator = nullptr) const;
};

}

