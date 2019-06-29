#include "Json.hpp"
#include "Collections/StringView.hpp"
#include "FileSystem.hpp"
#include "Logger.hpp"
#include "Utils.hpp"
#include <cctype>
#include <array>

#define JSON_TOKENS \
        JT(TokenType_Invalid = 0, "Invalid"),             \
        JT(TokenType_Identifier, "Identifier"),           \
        JT(TokenType_Integer, "Integer"),                 \
        JT(TokenType_Real, "Real"),                 \
        JT(TokenType_OpenCurlyBraces, "Open Curly Braces ({)"),    \
        JT(TokenType_CloseCurlyBraces, "Close Curly Braces (})"),  \
        JT(TokenType_OpenBrackets, "Open Brackets ([)"),    \
        JT(TokenType_CloseBrackets, "Close Brackets (])"),  \
        JT(TokenType_Quotes, "Quotes (\")"), \
        JT(TokenType_Comma, "Comma (,)"), \
        JT(TokenType_Colon, "Colon (:)"),

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
ParseString(Allocator* allocator, const Array<Token>& tokens, size_t start, size_t* end_it, String* str)
{
    assert(str);
    assert(start > 0);
    assert(allocator);
    
    size_t num_key_tokens = 4;
    if (GetTokensLeft(tokens, start) < num_key_tokens) {
        return "Json object key is incomplete";
    }
    
    if (tokens[start+0].type == TokenType_Quotes &&
        tokens[start+1].type == TokenType_Identifier &&
        tokens[start+2].type == TokenType_Quotes &&
        tokens[start+3].type == TokenType_Colon)
    {
        *end_it = start + 4;
        String key(allocator, tokens[start+1].str);
    }
    else
    {
        return "Was expecting a json key";
    }
    
    return nullptr;
}

static const char*
ParseObject(Allocator* allocator, const Array<Token>& tokens, size_t start, size_t* end_it, RobinHashMap<String, Json::Val>* obj)
{
    assert(tokens[start].type == TokenType_OpenCurlyBraces);
    assert(end_it);
    assert(obj);
    *end_it = start;
    
    *obj = RobinHashMap<String, Json::Val>(allocator, 32);
    size_t tokens_left = tokens.len - start;
    
    if (tokens_left < 2) {
        return "Invalid json object";
    }
    
    if (tokens[start+1].type == TokenType_CloseCurlyBraces) {
        // Empty array
        return nullptr;
    }
    
    size_t curr = start + 1; // advance one, since the current one is an open curly brace
    for (;;) {
        // Now, parse a given key of the object
        size_t num_key_tokens = 4;
        if (get_tokens_left(curr) < num_key_tokens) {
            return "Json object key is incomplete";
        }
        
        if (tokens[curr+0].type == TokenType_Quotes &&
            tokens[curr+1].type == TokenType_Identifier &&
            tokens[curr+2].type == TokenType_Quotes &&
            tokens[curr+3].type == TokenType_Colon)
        {
            // A key was found
            curr += 4;
            String key(allocator, tokens[curr+1].str);

            // Now parse the value
            if (tokens[curr+0].type == TokenType_Quotes &&
                tokens[curr+1].type == TokenType_Identifier &&
                tokens[curr+2].type == TokenType_Quotes &&
                tokens[curr+3].type == TokenType_Colon)
            {
                
            }
        }
        else
        {
            return "Was expecting key on JSON object";
        }
    }

    return nullptr;
}

static const char*
ParseArray(Allocator* allocator, const Array<Token>& tokens, size_t start, size_t* end_it, Array<Json::Val>* array)
{
    assert(tokens[start].type == TokenType_OpenBrackets);
    assert(end_it);
    assert(array);
    *end_it = start;
    
    *array = Array<Json::Val>(allocator);
    size_t tokens_left = tokens.len - start;
    
    if (tokens_left < 2) {
        return "Invalid json array";
    }
    
    if (tokens[start+1].type == TokenType_CloseBrackets) {
        // Empty array
        return nullptr;
    }
    
    if (tokens[start+1].type == TokenType_Integer) {
        int64_t n = Utils::ParseInt64((const uint8_t*)tokens[start+1].str.data, tokens[start+1].str.len);
        (*array).PushBack(Json::Val(n));
    }
    
    return nullptr;
}

void
Json::Document::Parse(uint8_t* data, size_t size)
{
    assert(allocator != nullptr);
    assert(data != nullptr);
    assert(size > 0);
    
    LOG_DEBUG("Parsing json string");

    const char* err_str = nullptr;
    Array<Token> tokens = Tokenize(allocator, (char*)data, size, &err_str);
    
    LOG_DEBUG("There are %zu tokens", tokens.len);

    if (err_str) {
        parse_error = String(allocator, err_str);
        return;
    }
    
    if (tokens.len < 2) {
        parse_error = String(allocator, "Invalid JSON string");
        return;
    }
    
//    auto loops_left = [](size_t t, const Array<Token>& tokens) -> int32_t {
//        return (int32_t)tokens.len - 1 - (int32_t)t;
//    };
    
    Val root_val;
    
    if (tokens[0].type == TokenType_OpenBrackets) {
        // Parse an array
        size_t end_it;
        Array<Val> array;
        const char* err_str = ParseArray(allocator, tokens, 0, &end_it, &array);
        if (err_str) {
            parse_error = String(allocator, err_str);
        } else {
            assert(end_it == tokens.len);
            root_val.type = Type::Array;
            root_val.values.array = std::move(array);
        }
    } else if (tokens[0].type == TokenType_OpenCurlyBraces) {
        // TODO: parse object
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
            Token tk(TokenType_Quotes);
            tokens.PushBack(tk);
            ++it;
        } else if (std::isdigit(*it) || *it == '-') {
            // parse until the last number
            if (*it == '-') {
                ++it;
            }
            const uint8_t *last_it = Utils::EatWhile(static_cast<int(*)(int)>(std::isdigit), it, end_it);

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
            // this else here should be a string
            std::array<char, 2> vals{ ',', '"' };
            const uint8_t *last_it = Utils::EatUntil(vals, it, end_it);

            if (last_it > end_it) {
                *err_str = "string was not finished by another double quote";
                return tokens;
            }

            assert(last_it - it != 0);

            Token tk(TokenType_Identifier, it, (size_t)(last_it - it));
            tokens.PushBack(std::move(tk));

            it = last_it;
        }
    }

    assert(it == end_it + 1);
    *err_str = nullptr;
    return tokens;
}
