#pragma once

#include "Han/Allocator.hpp"
#include "Han/Collections/RobinHashMap.hpp"
#include "Han/Collections/String.hpp"
#include "Han/Collections/Array.hpp"
#include "Han/Sid.hpp"
#include "Path.hpp"

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

    ~ResourceFile()
    {
        assert(_allocator == nullptr);
        assert(_scratch_allocator == nullptr);
    }

    void Create(const Sid& sid);
    void Destroy();

	inline bool Has(const String& key) const
	{
        return _entries.Find(key) != nullptr;
	}

	inline bool Has(const StringView& key) const
	{
        String wrapper(_scratch_allocator, key.data);
        return _entries.Find(std::move(wrapper)) != nullptr;
	}

	template<typename T> const T* Get(const String& key) const
	{
        const auto* it = _entries.Find(key);
        if (it) {
            // TODO: usign dynamic_cast here means that we cannot compile the 
            // code without rtti. Consider in the future a alternative implementation without
            // rtti.
            auto ptr = dynamic_cast<const T*>(*it);
            assert(ptr != nullptr);
            return ptr;
        } else {
            return nullptr;
        }
	}

	template<typename T> const T* Get(const StringView& key) const
	{
        String wrapper(_scratch_allocator, key.data);
		return Get<T>(std::move(wrapper));
	}

    RobinHashMap<String, Val*>& GetEntries() { return _entries; }
    const RobinHashMap<String, Val*>& GetEntries() const { return _entries; }

	void Parse();
	Array<Token> Tokenize();

private:
    Allocator* _allocator;
    Allocator* _scratch_allocator;
    RobinHashMap<String, Val*> _entries;

public:
    Path filepath;
    bool is_file_correct = true;
};
