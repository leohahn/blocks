#pragma once

#include "Allocator.hpp"
#include "Collections/RobinHashMap.hpp"
#include "Collections/String.hpp"
#include "Collections/Array.hpp"
#include "Path.hpp"
#include "Sid.hpp"

#define RESOURCE_TOKENS \
        RT(TokenType_Invalid = 0, "Invalid"),             \
        RT(TokenType_Identifier, "Identifier"),           \
        RT(TokenType_Integer, "Integer"),                 \
        RT(TokenType_OpenBracket, "Open Bracket ([)"),    \
        RT(TokenType_CloseBracket, "Close Bracket (])"),  \
        RT(TokenType_Comma, "Comma (,)"),                 \
        RT(TokenType_Equals, "Equals (=)"),               \
        RT(TokenType_Semicolon, "Semicolon (;)"),

static const char* ResourceFileTokenNames[] =
{
#define RT(e, s) s
	RESOURCE_TOKENS
#undef RT
};

struct ResourceFile
{
	enum TokenType
	{
#define RT(e, s) e
		RESOURCE_TOKENS
#undef RT
	};

	enum ValType
	{
		ValType_String,
		ValType_Array,
		ValType_Int,
	};

	struct Val
	{
		ValType type;
		Val(ValType type) : type(type) {}
		virtual ~Val() = default;
	};

	struct StringVal : public Val
	{
		explicit StringVal(String&& str)
			: Val(ValType_String)
			, str(std::move(str))
		{}

		String str;
	};

	struct ArrayVal : public Val
	{
		ArrayVal()
			: Val(ValType_Array)
		{}

		Array<Val*> vals;
	};

	struct IntVal : public Val
	{
		explicit IntVal(int32_t number)
			: Val(ValType_Int)
			, number(number)
		{}

		int number;
	};

	struct Token
	{
		TokenType type;
		String str;

		Token(TokenType type, String&& str)
			: type(type)
			, str(std::move(str))
		{}

		Token(TokenType type, const String& str)
			: type(type)
			, str(str)
		{}

		Token(TokenType type)
			: type(type)
			, str()
		{}
	};

	ResourceFile(Allocator* allocator, Allocator* scratch_allocator);

    void Create(const Sid& sid);
    void Destroy();

	inline bool Has(const String& key)
	{
        return _entries.Find(key) != nullptr;
	}

	template<typename T> const T* Get(const String& key)
	{
		auto ptr = dynamic_cast<T*>(_entries.Find(key));
		assert(ptr != nullptr);
		return ptr;
	}

    RobinHashMap<String, Val*>& GetEntries() { return _entries; }
    const RobinHashMap<String, Val*>& GetEntries() const { return _entries; }

public:
	Path filepath;
	bool is_file_correct = true;

private:
    Allocator* _allocator;
    Allocator* _scratch_allocator;
	RobinHashMap<String, Val*> _entries;

	void Parse();
	Array<Token> Tokenize();
};
