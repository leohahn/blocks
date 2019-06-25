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
        JT(TokenType_Number, "Number"),                 \
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

    Token(TokenType type, uint8_t* str, size_t len)
        : type(type)
        , str((char*)str, len)
    {}

    Token(TokenType type)
        : type(type)
        , str()
    {}
};

static Array<Token> Tokenize(const char* str, size_t str_size, const char** err_str);

void
Json::Document::Parse(const char* json_str)
{
    assert(allocator != nullptr);
    assert(json_str != nullptr);

    const char* err_str = nullptr;
    Array<Token> tokens = Tokenize(allocator, json_str, strlen(json_str), &err_str);

    if (err_str) {
        LOG_ERROR("Error parsing json: %s", err_str);
        return;
    }

    LOG_DEBUG("Tokens in json string:");
    for (size_t i = 0; i < tokens.len; ++i) {
        LOG_DEBUG("-- %s", TokenNames[tokens[i].type]);
        if (tokens[i].type == TokenType_Identifier) {
            LOG_DEBUG("    val: %s", tokens[i].str.data);
        }
    }
}

static Array<Token>
Tokenize(Allocator* allocator, const char* str, size_t str_size, const char** err_str)
{
    assert(err_str);
    assert(str);
    assert(str_size > 0);
    Array<Token> tokens(allocator);

    uint8_t *it = (uint8_t*)str;
    const uint8_t *end_it = (uint8_t*)str + str_size - 1;

    while (it <= end_it) {
        it = Utils::EatWhitespaces(it, end_it);

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
        } else if (std::isdigit(*it)) {
            // parse until the last number
            uint8_t *last_it = Utils::EatWhile(static_cast<int(*)(int)>(std::isdigit), it, end_it);

            if (*last_it == '.') {
                // TODO: parse float.
                assert(false);
            } else {
                Token tk(TokenType_Number, it, (size_t)(last_it - it));
                tokens.PushBack(std::move(tk));
                it = last_it;
            }
        } else {
            // this else here should be a string
            std::array<char, 6> vals{ ',', '"' };
            uint8_t *last_it = Utils::EatUntil(vals, it, end_it);

            if (last_it > end_it) {
                *err_str = "string was not finished by another double quote";
                return tokens;
            }

            Token tk(TokenType_Identifier, it, (size_t)(last_it - it));
            tokens.PushBack(std::move(tk));

            it = last_it;
        }
    }

    assert(it == end_it + 1);
    return tokens;
}
