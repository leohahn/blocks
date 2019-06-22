#include "ResourceFile.hpp"

#include "logger.hpp"
#include "FileSystem.hpp"
#include "Utils.hpp"
#include <sstream>
#include <fstream>
#include <array>
#include <algorithm>
#include <cctype>
#include <functional>

#define HASH_MAP_FIXED_SIZE 32

ResourceFile::ResourceFile(Allocator* allocator, Allocator* scratch_allocator)
    : _allocator(allocator)
    , _scratch_allocator(scratch_allocator)
    , filepath(allocator)
    , _entries(allocator, HASH_MAP_FIXED_SIZE)
{
}

void
ResourceFile::Create(const Sid& file_sid)
{
    auto resources_path = FileSystem::GetResourcesPath(_scratch_allocator);
    this->filepath.Push(resources_path);
    this->filepath.Push(file_sid.GetStr());
    _entries.Create();
    resources_path.Destroy();
    Parse();
}

void 
ResourceFile::Destroy()
{
    filepath.Destroy();
    _entries.Destroy();
    _allocator = nullptr;
    _scratch_allocator = nullptr;
}

static inline uint8_t *
EatWhitespaces(uint8_t* it, const uint8_t* end_it)
{
    uint8_t* new_it = it;
    while (std::isspace(*new_it) && new_it <= end_it) {
        new_it++;
    }
    return new_it;
}

static inline uint8_t*
EatUntil(char c, uint8_t* it, const uint8_t* end_it)
{
    uint8_t* new_it = it;
    while (*new_it != c && new_it <= end_it) {
        new_it++;
    }
    return new_it;
}

template<typename T>
static inline uint8_t *
EatUntil(const T &chars, uint8_t *it, const uint8_t *end_it)
{
    uint8_t *new_it = it;
    while (std::find(std::begin(chars), std::end(chars), *new_it) == std::end(chars) &&
        new_it <= end_it) {
        new_it++;
    }
    return new_it;
}

static inline uint8_t *
EatWhile(const std::function<bool(uint8_t)> &predicate, uint8_t *it, const uint8_t *end_it)
{
    uint8_t *new_it = it;
    while (predicate(*new_it) && new_it <= end_it) {
        new_it++;
    }
    return new_it;
}

Array<ResourceFile::Token>
ResourceFile::Tokenize()
{
    Array<Token> tokens(_scratch_allocator);

    size_t filesize;
    uint8_t* filedata = FileSystem::LoadFileToMemory(_scratch_allocator, filepath, &filesize);
    assert(filedata && filesize > 0);

    uint8_t *it = (uint8_t*)filedata;
    const uint8_t *end_it = (uint8_t*)filedata + filesize - 1;

    while (it <= end_it) {
        it = EatWhitespaces(it, end_it);

        if (*it == '#') {
            it = EatUntil('\n', it, end_it);
        } else if (std::isalpha(*it)) {
            std::array<char, 6> chars{ {' ', '=', ';', ',', ']', '\n'} };
            uint8_t *last_it = EatUntil(chars, it, end_it);

            String token_str(_scratch_allocator);
            token_str.Append(it, last_it);

            Token tk(TokenType_Identifier, std::move(token_str));
            tokens.PushBack(std::move(tk));

            it = last_it;
        } else if (std::isdigit(*it)) {
            // parse until the last number
            uint8_t *last_it = EatWhile(static_cast<int(*)(int)>(std::isdigit), it, end_it);

            if (*last_it == '.') {
                // TODO: parse float.
                assert(false);
            } else {
                String token_str(_scratch_allocator);
                token_str.Append(it, last_it);
                Token tk(TokenType_Integer, std::move(token_str));
                tokens.PushBack(std::move(tk));
                it = last_it;
            }
        } else if (*it == '=') {
            Token tk(TokenType_Equals);
            tokens.PushBack(std::move(tk));
            it++;
        } else if (*it == '[') {
            Token tk(TokenType_OpenBracket);
            tokens.PushBack(std::move(tk));
            it++;
        } else if (*it == ']') {
            Token tk(TokenType_CloseBracket);
            tokens.PushBack(std::move(tk));
            it++;
        } else if (*it == ',') {
            Token tk(TokenType_Comma);
            tokens.PushBack(std::move(tk));
            it++;
        } else if (*it == ';') {
            Token tk(TokenType_Semicolon);
            tokens.PushBack(std::move(tk));
            it++;
        }
    }

    assert(it == end_it + 1);

    _scratch_allocator->Deallocate(filedata);
    return tokens;
}

void
ResourceFile::Parse()
{
    Array<Token> tokens = Tokenize();

    if (tokens.len < 4) {
        LOG_ERROR("For file %s", filepath.data);
        LOG_ERROR("Rule must have at least 4 tokens.");
        return;
    }

    // logger.log("For file ", filepath);

    // for (auto &token : tokens)
    //     logger.log(ResourceFileTokenNames[token.type]);

    auto loops_left = [](size_t t, const Array<Token>& tokens) -> int32_t {
        return (int32_t)tokens.len - 1 - (int32_t)t;
    };

    for (size_t t = 0; t < tokens.len;) {
        if (tokens[t].type == TokenType_Identifier &&
            tokens[t + 1].type == TokenType_Equals) {
            String key = std::move(tokens[t].str);
            if (Has(key)) {
                LOG_ERROR("File already has the key %s", key.data);
                return;
            }

            if (tokens[t + 2].type == TokenType_Identifier && tokens[t + 3].type == TokenType_Semicolon) {
                String val = std::move(tokens[t + 2].str);

                // logger.log("adding entry ", key);
                // logger.log("value ", val);

                auto string_val = _allocator->New<StringVal>(std::move(val));
                _entries.Add(std::move(key), std::move(string_val));

                t += 4; // four tokens were recognized
            } else if (tokens[t + 2].type == TokenType_OpenBracket) {
                t += 3;

                auto array_val = _allocator->New<ArrayVal>();

                while (t < tokens.len) {
                    if (tokens[t].type == TokenType_Identifier) {
                        auto new_val = _allocator->New<StringVal>(std::move(tokens[t].str));
                        array_val->vals.PushBack(std::move(new_val));

                        if (loops_left(t, tokens) >= 2 &&
                            tokens[t + 1].type == TokenType_CloseBracket &&
                            tokens[t + 2].type == TokenType_Semicolon) {
                            t += 3;
                            break;
                        } else if (loops_left(t, tokens) >= 1 && tokens[t + 1].type == TokenType_Comma) {
                            t += 2;
                        } else {
                            LOG_ERROR("Invalid array.");
                            return;
                        }
                    } else {
                        LOG_ERROR("Invalid array.");
                        return;
                    }
                }

                // logger.log("adding array entry ", key);
                _entries.Add(std::move(key), std::move(array_val));
            } else if (tokens[t + 2].type == TokenType_Integer && tokens[t + 3].type == TokenType_Semicolon) {
                const String& int_str = tokens[t + 2].str;
                int32_t number;
                bool ok = Utils::ParseInt32(int_str.data, &number);
                assert(ok && "should be able to parse a number");

                auto int_val = _allocator->New<IntVal>(number);
                _entries.Add(std::move(key), std::move(int_val));

                t += 4; // four tokens were recognized
            } else {
                LOG_ERROR("Wrong syntax on value.");
                return;
            }
        } else {
            LOG_ERROR("Wrong syntax on file.");
            LOG_ERROR("Expected IDENTIFIER =");
            return;
        }
    }
}