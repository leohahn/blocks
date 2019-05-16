#pragma once
#include <assert.h>
#include <stdint.h>
#include "Allocator.hpp"

// Currently this hash map does not support rehashing
template<typename Key, typename Value>
struct RobinHashMap
{
    // The maximum load allowed on the table before we need to rehash it.
    static constexpr float kMaxLoadFactor = 0.9f;

	struct Element
	{
		Key key;
		Value val;
		uint32_t hash;

		Element(uint32_t hash, Key&& key, Value&& val)
			: hash(hash)
			, key(std::move(key))
			, val(std::move(val))
		{}
	};

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

	void Create()
	{
		// TODO: allocate the data
		assert(!buckets);
		assert(allocator);
		assert(num_buckets > 0);
		elements = (Element*)allocator->Allocate(cap * sizeof(Element));
        assert(elements);
        for (size_t i = 0; i < cap; ++i) {
            elements[i].hash = 0; // All elements are free
        }
	}

	void Destroy()
	{
		// TODO: implement this piece of code
        for (size_t i = 0; i < num_elements; ++i) {
            elements[i]->key->~Key();
            elements[i]->val->~Value();
        }
        allocator->Deallocate(elements);
        elements = nullptr;
        allocator = nullptr;
	}

    void Add(Key&& key, Value&& value)
    {
        // TODO: eventually implement rehashing
        assert(num_elements < max_num_elements_allowed);
        uint32_t hash = HashKey(key);
        size_t pos = GetDesiredPosition(hash);
        size_t probe_distance = 0;
        size_t mask = GetMask();

        for (;;) {
            if (elements[pos].hash == 0) {
                // Position is not occupied, therefore we place the element here.
                EmplaceElement(pos, hash, std::move(key), std::move(value));
                return;
            }

            // the current postition is occupied, therefore we check if the current pos
            // was probed less than us, and if it is so, we change positions.
            size_t current_position_probe_dist = GetProbeDistance(elements[pos].hash, pos);
            if (current_position_probe_dist < probe_distance) {
                if (IsDeleted(elements[pos].hash)) {
                    EmplaceElement(pos, hash, std::move(key), std::move(value));
                    return;
                }

                probe_distance = current_position_probe_dist;
                std::swap(key, elements[pos].key);
                std::swap(value, elements[pos].val);
                std::swap(hash, elements[pos].hash);
            }

            pos = (pos + 1) % mask;
            ++probe_distance;
        }
    }

    const Value* Find(const Key& key) const
    {
        Value* val;
        if (FindHelper(key, val)) {
            return const_cast<const Value*>(val);
        } else {
            return nullptr;
        }
    }

    Value* Find(const Key& key)
    {
        Value* val;
        if (FindHelper(key, val)) {
            return val;
        } else {
            return nullptr;
        }
    }

private:
    bool FindHelper(const Key& key, Value* out_val) const
    {
        const size_t mask = GetMask();
        const uint32_t hash = HashKey(key);
        size_t pos = GetDesiredPosition(hash);
        int probe_distance = 0;

        for (;;)
        {
            if (elements[pos].hash == 0) {
                return nullptr;
            } else if (probe_distance > GetProbeDistance(elements[pos].hash, pos)) {
                return nullptr;
            } else if (elements[pos].hash == hash && elements[pos].key == key) {
                return &elements[pos].val;
            }

            pos = (pos + 1) % mask;
            ++probe_distance;
        }
    }
    
    void EmplaceElement(size_t desired_pos, uint32_t hash, Key&& key, Value&& value)
    {
        new (elements + desired_pos) Element(hash, std::move(key), std::move(value));
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
        uint32_t h = (uint32_t)hasher(key);

        h &= 0x7fffffff; // clear MSB (use for deleted marking)
        h |= (h == 0); // hash 0 is used for unused element

        return h;
    }
};

