#include "Json.hpp"
#include "Han/Collections/StringView.hpp"
#include "Han/FileSystem.hpp"
#include "Han/Logger.hpp"
#include "Utils.hpp"
#include <cctype>
#include <array>
#include <inttypes.h>

#define JSON_TOKENS \
        JT(TokenType_Invalid = 0, "Invalid"),             \
        JT(TokenType_String, "String"),           \
        JT(TokenType_Integer, "Integer"),                 \
        JT(TokenType_Real, "Real"),                 \
        JT(TokenType_OpenCurlyBraces, "Open Curly Braces ({)"),    \
        JT(TokenType_CloseCurlyBraces, "Close Curly Braces (})"),  \
        JT(TokenType_OpenBrackets, "Open Brackets ([)"),    \
        JT(TokenType_CloseBrackets, "Close Brackets (])"),  \
        JT(TokenType_Comma, "Comma (,)"), \
        JT(TokenType_Colon, "Colon (:)"), \
        JT(TokenType_Boolean, "Boolean (true or false)"), \
        JT(TokenType_Null, "Null"),

static const char* TokenNames[] =
{
#define JT(e, s) s
	JSON_TOKENS
#undef JT
};

enum TokenType
{
#define JT(e, s) e
    JSON_TOKENS
#undef JT
};

struct Token
{
    TokenType type;
    StringView str;

    Token(TokenType type, const uint8_t* str, size_t len)
        : type(type)
        , str((char*)str, len)
    {}

    Token(TokenType type)
        : type(type)
        , str()
    {}
};

static Array<Token> Tokenize(Allocator* allocator, const char* str, size_t str_size, const char** err_str);
static const char* ParseArray(Allocator* allocator,
                              const Array<Token>& tokens,
                              size_t* start,
                              Array<Json::Val>* array);
static const char* ParseObject(Allocator* allocator,
                               const Array<Token>& tokens,
                               size_t* start,
                               RobinHashMap<String, Json::Val>* obj);

void
Json::Document::Parse(const char* json_str)
{
    assert(json_str);
    Parse((uint8_t*)json_str, strlen(json_str));
}

static size_t
GetTokensLeft(const Array<Token>& tokens, size_t current)
{
    return tokens.len - current;
}

static const char*
ParseString(Allocator* allocator, const Array<Token>& tokens, size_t* start, String* str)
{
    assert(str);
    assert(start);
    assert(start > (size_t*)0);
    assert(allocator);

    const size_t num_key_tokens = 1;
    if (GetTokensLeft(tokens, *start) < num_key_tokens) {
        return "Was expecting a string";
    }

    if (tokens[*start].type == TokenType_String)
    {
        *str = String(allocator, tokens[*start].str);
        ++(*start);
        return nullptr;
    }
    else
    {
        return "Was expecting a json string";
    }
}

static const char*
ParseObject(Allocator* allocator, const Array<Token>& tokens, size_t* start, RobinHashMap<String, Json::Val>* obj)
{
    assert(start);
    assert(obj);
    
    const size_t tokens_left = tokens.len - *start;
    if (tokens_left < 2) {
        return "Invalid json object";
    }

    if (tokens[*start].type != TokenType_OpenCurlyBraces) {
        return "object did not start with curly braces";
    }

    *obj = RobinHashMap<String, Json::Val>(allocator, 32);
    size_t curr = *start + 1; // advance one, since the current one is an open curly brace

    if (tokens[curr].type == TokenType_CloseCurlyBraces) {
        ++curr;
        *start = curr;
        return nullptr;
    }

    for (;;) {
        // Now, parse a given key of the object
        String key;
        const char* err_msg = ParseString(allocator, tokens, &curr, &key);
        if (err_msg) {
            return err_msg;
        }

        // Now a colon should be here
        if (tokens[curr].type != TokenType_Colon) {
            return "Expecting a colon after key in object";
        }
        ++curr;

        // Now, parse the key value
        switch (tokens[curr].type) {
            case TokenType_Boolean: {
                Json::Val boolean((tokens[curr].str == "true") ? true : false);
                (*obj).Add(std::move(key), std::move(boolean));
                ++curr;
            } break;
            case TokenType_Null: {
                (*obj).Add(std::move(key), Json::Val());
                ++curr;
            } break;
            case TokenType_Integer: {
                int64_t val = Utils::ParseInt64((const uint8_t*)tokens[curr].str.data, tokens[curr].str.len);
                (*obj).Add(std::move(key), Json::Val(val));
                ++curr;
            } break;
            case TokenType_Real: {
                double val = Utils::ParseDouble((const uint8_t*)tokens[curr].str.data, tokens[curr].str.len);
                (*obj).Add(std::move(key), Json::Val(val));
                ++curr;
            } break;
            case TokenType_OpenCurlyBraces: {
                RobinHashMap<String, Json::Val> inner_obj;
                const char* err_msg = ParseObject(allocator, tokens, &curr, &inner_obj);
                if (err_msg) {
                    return err_msg;
                }
                (*obj).Add(std::move(key), Json::Val(std::move(inner_obj)));
            } break;
            case TokenType_OpenBrackets: {
                Array<Json::Val> inner_array;
                const char* err_msg = ParseArray(allocator, tokens, &curr, &inner_array);
                if (err_msg) {
                    return err_msg;
                }
                (*obj).Add(std::move(key), Json::Val(std::move(inner_array)));
            } break;
            case TokenType_String: {
                String string_val;
                const char* err_msg = ParseString(allocator, tokens, &curr, &string_val);
                if (err_msg) {
                    return err_msg;
                }
                (*obj).Add(std::move(key), Json::Val(std::move(string_val)));
            } break;
            default: {
                LOG_ERROR("Was not expecting token %s inside object", tokens[curr].str.data);
            } break;
        }

        if (tokens[curr].type == TokenType_Comma) {
            // a comma was found, more items to parse
            ++curr;
        } else if (tokens[curr].type != TokenType_CloseCurlyBraces) {
            return "Was expecting a comma after a value inside object or a closing curly brace";
        } else {
            ++curr;
            break;
        }
    }

    *start = curr;
    return nullptr;
}

static const char*
ParseArray(Allocator* allocator, const Array<Token>& tokens, size_t* start, Array<Json::Val>* array)
{
    assert(start);
    assert(array);
    
    const size_t tokens_left = tokens.len - *start;
    if (tokens_left < 2) {
        return "Invalid json array";
    }

    if (tokens[*start].type != TokenType_OpenBrackets) {
        return "array did not start with open bracket";
    }

    *array = Array<Json::Val>(allocator);
    size_t curr = *start + 1; // advance one, since the current one is an open bracket

    if (tokens[curr].type == TokenType_CloseBrackets) {
        ++curr;
        *start = curr;
        return nullptr;
    }

    for (;;) {
        // Now, parse a value of the array
        switch (tokens[curr].type) {
            case TokenType_Boolean: {
                Json::Val boolean((tokens[curr].str == "true") ? true : false);
                (*array).PushBack(std::move(boolean));
                ++curr;
            } break;
            case TokenType_Null: {
                (*array).PushBack(Json::Val());
                ++curr;
            } break;
            case TokenType_Integer: {
                int64_t val = Utils::ParseInt64((const uint8_t*)tokens[curr].str.data, tokens[curr].str.len);
                (*array).PushBack(Json::Val(val));
                ++curr;
            } break;
            case TokenType_Real: {
                double val = Utils::ParseDouble((const uint8_t*)tokens[curr].str.data, tokens[curr].str.len);
                (*array).PushBack(Json::Val(val));
                ++curr;
            } break;
            case TokenType_OpenCurlyBraces: {
                RobinHashMap<String, Json::Val> inner_obj;
                const char* err_msg = ParseObject(allocator, tokens, &curr, &inner_obj);
                if (err_msg) {
                    return err_msg;
                }
                (*array).PushBack(Json::Val(std::move(inner_obj)));
            } break;
            case TokenType_OpenBrackets: {
                Array<Json::Val> inner_array;
                const char* err_msg = ParseArray(allocator, tokens, &curr, &inner_array);
                if (err_msg) {
                    return err_msg;
                }
                (*array).PushBack(Json::Val(std::move(inner_array)));
            } break;
            case TokenType_String: {
                String string_val;
                const char* err_msg = ParseString(allocator, tokens, &curr, &string_val);
                if (err_msg) {
                    return err_msg;
                }
                (*array).PushBack(Json::Val(std::move(string_val)));
            } break;
            default: {
                LOG_ERROR("Was not expecting token %s inside object", tokens[curr].str.data);
            } break;
        }

        if (tokens[curr].type == TokenType_Comma) {
            // a comma was found, there are more items to parse
            ++curr;
        } else if (tokens[curr].type != TokenType_CloseBrackets) {
            return "Was expecting a comma after a value inside array";
        } else {
            ++curr;
            break;
        }
    }

    *start = curr;
    return nullptr;
}

void
Json::Document::Parse(uint8_t* data, size_t size)
{
    assert(allocator != nullptr);
    assert(data != nullptr);
    assert(size > 0);
    
    const char* err_str = nullptr;
    Array<Token> tokens = Tokenize(allocator, (char*)data, size, &err_str);
    
    if (err_str) {
        this->parse_error = String(allocator, err_str);
        return;
    }
    
    if (tokens.len < 2) {
        this->parse_error = String(allocator, "Invalid JSON string");
        return;
    }
    
    if (tokens[0].type == TokenType_OpenBrackets) {
        // Parse an array
        size_t it = 0;
        Array<Val> array;
        const char* err_str = ParseArray(allocator, tokens, &it, &array);
        if (err_str) {
            this->parse_error = String(allocator, err_str);
        } else {
            assert(it == tokens.len);
            root_val.type = Type::Array;
            root_val.values.array = std::move(array);
        }
    } else if (tokens[0].type == TokenType_OpenCurlyBraces) {
        // Parse an object
        size_t it = 0;
        RobinHashMap<String, Val> obj;
        const char* err_str = ParseObject(allocator, tokens, &it, &obj);
        if (err_str) {
            this->parse_error = String(allocator, err_str);
        } else {
            assert(it == tokens.len);
            root_val.type = Type::Object;
            root_val.values.object = std::move(obj);
        }
    } else {
        // invalid root json value
        this->parse_error = String(allocator, "Json document did not start with an object or array");
    }
}

static Array<Token>
Tokenize(Allocator* allocator, const char* str, size_t str_size, const char** err_str)
{
    assert(err_str);
    assert(str);
    assert(str_size > 0);
    Array<Token> tokens(allocator);

    const uint8_t* it = (uint8_t*)str;
    const uint8_t* end_it = (uint8_t*)str + str_size - 1;

    while (it <= end_it) {
        it = Utils::EatWhitespaces(it, end_it);
        if (it > end_it) {
            continue;
        }

        if (*it == '{') {
            Token tk(TokenType_OpenCurlyBraces);
            tokens.PushBack(tk);
            ++it;
        } else if (*it == '}') {
            Token tk(TokenType_CloseCurlyBraces);
            tokens.PushBack(tk);
            ++it;
        } else if (*it == '[') {
            Token tk(TokenType_OpenBrackets);
            tokens.PushBack(tk);
            ++it;
        } else if (*it == ']') {
            Token tk(TokenType_CloseBrackets);
            tokens.PushBack(tk);
            ++it;
        } else if (*it == ',') {
            Token tk(TokenType_Comma);
            tokens.PushBack(tk);
            ++it;
        } else if (*it == ':') {
            Token tk(TokenType_Colon);
            tokens.PushBack(tk);
            ++it;
        } else if (*it == '"') {
            ++it;
            const uint8_t *last_it = Utils::EatUntil('"' , it, end_it);
            ++last_it;
            if (last_it > end_it) {
                *err_str = "string does not end with a double quote";
                return tokens;
            }
            Token tk(TokenType_String, it, (size_t)(last_it - 1 - it));
            tokens.PushBack(std::move(tk));
            it = last_it;
        } else if (std::isdigit(*it) || *it == '-') {
            // parse until the last number
            const uint8_t *last_it = Utils::EatWhile(static_cast<int(*)(int)>(std::isdigit), it + 1, end_it);

            bool real = false;
            if (*last_it == '.') {
                // number is a float, thus we have to parse it.
                ++last_it;
                last_it = Utils::EatWhile(static_cast<int(*)(int)>(std::isdigit), last_it, end_it);
                real = true;
            }
            
            Token tk(real ? TokenType_Real : TokenType_Integer, it, (size_t)(last_it - it));
            tokens.PushBack(std::move(tk));
            it = last_it;
        } else {
            // if this branch is executed it means either a boolean or a null
            const uint8_t *last_it = Utils::EatUntil(',', it, end_it);

            if (last_it > end_it) {
                *err_str = "value was not ended by a double quote or a comma";
                return tokens;
            }

            assert(last_it - it != 0);

            Token tk(TokenType_Null, it, (size_t)(last_it - it));

            if (tk.str == "null") {
                tk.type = TokenType_Null;
            } else if (tk.str == "true" || tk.str == "false") {
                tk.type = TokenType_Boolean;
            } else {
                LOG_ERROR("Invalid json identifier: %.*s", (int)tk.str.len, tk.str.data);
                *err_str = "Invalid identifier";
            }

            tokens.PushBack(std::move(tk));

            it = last_it;
        }
    }

    assert(it == end_it + 1);
    *err_str = nullptr;
    return tokens;
}

//-----------------------------------------
// Pretty printing
//-----------------------------------------

static void PrintObject(String* str, const RobinHashMap<String, Json::Val>& obj, int indent_level);
static void PrintArray(String* str, const Array<Json::Val>& array, int indent_level);
static void PrintVal(String* str, const Json::Val& val, int indent_level);

static void
PrintIndent(String* str, int indent_level)
{
    assert(str);
    for (int i = 0; i < indent_level; ++i) {
        str->Append(' ');
    }
}

static void
PrintBoolean(String* str, bool b)
{
    assert(str);
    if (b) {
        str->Append("true");
    } else {
        str->Append("false");
    }
}

static void
PrintString(String* str, const String& str_to_print)
{
    assert(str);
    str->Append('"');
    str->Append(str_to_print);
    str->Append('"');
}

static void
PrintReal(String* str, double real)
{
    assert(str);
    char buf[32];
    sprintf(buf, "%f", real);
    str->Append(buf);
}

static void
PrintInteger(String* str, int64_t integer)
{
    assert(str);
    char buf[32];
    sprintf(buf, "%" PRId64, integer);
    str->Append(buf);
}

static void
PrintObject(String* str, const RobinHashMap<String, Json::Val>& obj, int indent_level)
{
    assert(str);
    str->Append("{\n");
    for (RobinHashMap<String, Json::Val>::iterator it = obj.begin(); it != obj.end(); it++) {
        PrintIndent(str, indent_level);
        PrintString(str, (*it).key);
        str->Append(": ");
        PrintVal(str, (*it).val, indent_level);
        if (it + 1 != obj.end()) {
            str->Append(",\n");
        } else {
            str->Append('\n');
        }
    }
    PrintIndent(str, indent_level-2);
    str->Append("}");
}

static void
PrintArray(String* str, const Array<Json::Val>& array, int indent_level)
{
    assert(str);
    str->Append("[\n");
    for (size_t i = 0; i < array.len; ++i) {
        PrintIndent(str, indent_level);
        PrintVal(str, array[i], indent_level);
        if (i != array.len-1) {
            str->Append(",\n");
        } else {
            str->Append('\n');
        }
    }
    PrintIndent(str, indent_level-2);
    str->Append("]");
}

static void
PrintVal(String* str, const Json::Val& val, int indent_level)
{
    switch (val.type) {
        case Json::Type::Array:
            PrintArray(str, val.values.array, indent_level + 2);
            break;
        case Json::Type::Boolean:
            PrintBoolean(str, val.values.boolean);
            break;
        case Json::Type::Integer:
            PrintInteger(str, val.values.integer);
            break;
        case Json::Type::Null:
            str->Append("null");
            break;
        case Json::Type::Object:
            PrintObject(str, val.values.object, indent_level + 2);
            break;
        case Json::Type::Real:
            PrintReal(str, val.values.real);
            break;
        case Json::Type::String:
            PrintString(str, val.values.string);
            break;
    }
}

String
Json::Val::PrettyPrint(Allocator* allocator) const
{
    assert(allocator);
    String str(allocator);
    PrintVal(&str, *this, 0);
    return str;
}

String
Json::Document::PrettyPrint(Allocator* other_allocator) const
{
    return root_val.PrettyPrint(other_allocator ? other_allocator : allocator);
}
