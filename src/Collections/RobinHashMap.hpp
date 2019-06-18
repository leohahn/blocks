#pragma once
#include <assert.h>
#include <stdint.h>
#include <algorithm>
#include "Allocator.hpp"

// Currently this hash map does not support rehashing
template<typename Key, typename Value>
class RobinHashMap
{
public:
    // The maximum load allowed on the table before we need to rehash it.
    static constexpr float kMaxLoadFactor = 0.9f;

	struct Element
	{
        friend class RobinHashMap;
        friend class iterator;
		Key key;
		Value val;

		Element(uint32_t hash, Key&& key, Value&& val)
			: key(std::move(key))
			, val(std::move(val))
			, _hash(hash)
		{}

    private:
		uint32_t _hash;
	};

    class iterator : public std::iterator<
        std::random_access_iterator_tag,   // iterator_category
        Element,                   // value_type
        std::ptrdiff_t,            // difference_type
        Element*,               // pointer
        Element&                       // reference
        >
    {
    public:
        explicit iterator(Element* element, Element* end_element)
            : _element(element)
            , _end_element(end_element)
        {}

        iterator& operator++()
        {
            for (;;) {
                ++_element;
                if (_element == _end_element) {
                    break;
                }

                if (_element->_hash == 0 ||
                    (_element->_hash >> 31) != 0) {
                    continue;
                } else {
                    break;
                }
            }
            return *this;
        }
        iterator operator++(int) { iterator ret = *this; ++(*this); return ret; }

        bool operator==(iterator other) const { return _element == other._element; }
        bool operator!=(iterator other) const { return !(*this == other); }
        reference operator*() const { return *_element; }

    private:
        Element* _element;
        Element* _end_element;
    };

    using const_iterator = const iterator;

    iterator begin()
    {
        for (size_t i = 0; i < cap; ++i) {
            if (IsDeleted(elements[i]._hash) || elements[i]._hash == 0) {
                continue;
            } else {
                return iterator(&elements[i], elements + cap);
            }
        }
        return end();
    }

    iterator end() { return iterator(elements + cap, elements + cap); }

    const_iterator cbegin() const 
    {
        for (size_t i = 0; i < cap; ++i) {
            if (IsDeleted(elements[i]._hash) || elements[i]._hash == 0) {
                continue;
            } else {
                return iterator(&elements[i], elements + cap);
            }
        }
        return cend();
    }
    const_iterator cend() const { return iterator{elements + cap, elements + cap}; }

public:
	Allocator* allocator;
    Element* elements;
    size_t cap;
	size_t num_elements;
    size_t max_num_elements_allowed;

public:
	RobinHashMap(Allocator* allocator, size_t cap)
		: allocator(allocator)
		, elements(nullptr)
        , cap(cap)
		, num_elements(0)
        , max_num_elements_allowed((size_t)(kMaxLoadFactor * cap))
	{}

    ~RobinHashMap()
    {
        assert(allocator == nullptr);
        assert(elements == nullptr);
        assert(num_elements == 0);
        assert(cap == 0);
    }

	void Create()
	{
		// TODO: allocate the data
		assert(!elements);
		assert(allocator);
		assert(num_elements == 0);
		elements = (Element*)allocator->Allocate(cap * sizeof(Element));
        assert(elements);
        for (size_t i = 0; i < cap; ++i) {
            elements[i]._hash = 0; // All elements are free
        }
	}

	void Destroy()
	{
		// TODO: implement this piece of code
        if (elements) {
            for (size_t i = 0; i < cap; ++i) {
                if (elements[i]._hash != 0 && !IsDeleted(elements[i]._hash)) {
                    elements[i].key.~Key();
                    elements[i].val.~Value();
                }
            }
        }
        allocator->Deallocate(elements);
        elements = nullptr;
        allocator = nullptr;
        cap = 0;
        num_elements = 0;
	}

    void Add(Key key, Value value)
    {
        // TODO: eventually implement rehashing
        assert(num_elements < max_num_elements_allowed);
        uint32_t hash = HashKey(key);
        size_t pos = GetDesiredPosition(hash);
        size_t probe_distance = 0;
        size_t mask = GetMask();

        assert(pos < cap);

        for (;;) {
            if (elements[pos]._hash == 0) {
                // Position is not occupied, therefore we place the element here.
                EmplaceElement(pos, hash, std::move(key), std::move(value));
                return;
            }

            // the current postition is occupied, therefore we check if the current pos
            // was probed less than us, and if it is so, we change positions.
            size_t current_position_probe_dist = GetProbeDistance(elements[pos]._hash, pos);
            if (current_position_probe_dist < probe_distance) {
                if (IsDeleted(elements[pos]._hash)) {
                    EmplaceElement(pos, hash, std::move(key), std::move(value));
                    return;
                }

                probe_distance = current_position_probe_dist;
                std::swap(key, elements[pos].key);
                std::swap(value, elements[pos].val);
                std::swap(hash, elements[pos]._hash);
            }

            pos = (pos + 1) % mask;
            ++probe_distance;
        }
    }

    const Value* Find(const Key& key) const
    {
        Value* val;
        if (FindHelper(key, &val)) {
            return const_cast<const Value*>(val);
        } else {
            return nullptr;
        }
    }

    Value* Find(const Key& key)
    {
        Value* val;
        if (FindHelper(key, &val)) {
            return val;
        } else {
            return nullptr;
        }
    }

private:
    bool FindHelper(const Key& key, Value** out_val) const
    {
        const size_t mask = GetMask();
        const uint32_t hash = HashKey(key);
        size_t pos = GetDesiredPosition(hash);
        int probe_distance = 0;

        for (;;)
        {
            if (elements[pos]._hash == 0) {
                *out_val = nullptr;
                return false;
            } else if (probe_distance > GetProbeDistance(elements[pos]._hash, pos)) {
                return false;
            } else if (elements[pos]._hash == hash && elements[pos].key == key) {
                *out_val = &elements[pos].val;
                return true;
            }

            pos = (pos + 1) % mask;
            ++probe_distance;
        }
    }
    
    void EmplaceElement(size_t desired_pos, uint32_t hash, Key&& key, Value&& value)
    {
        new (elements + desired_pos) Element(hash, std::move(key), std::move(value));
        ++num_elements;
    }

    size_t GetMask() const
    {
        return cap - 1;
    }

    size_t GetDesiredPosition(uint32_t hash) const
    {
        return hash % (cap - 1);
    }

    size_t GetProbeDistance(uint32_t hash, size_t pos) const
    {
        return (pos + cap - GetDesiredPosition(hash)) & GetMask();
    }

    static bool IsDeleted(uint32_t hash)
    {
        return (hash >> 31) != 0;
    }

    static uint32_t HashKey(const Key& key)
    {
        std::hash<Key> hasher;
        size_t untruncated_hash = hasher(key);
        uint32_t h = static_cast<uint32_t>(untruncated_hash);

        h &= 0x7fffffff; // clear MSB (use for deleted marking)
        h |= (h == 0); // hash 0 is used for unused element

        return h;
    }
};

