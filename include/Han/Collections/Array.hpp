#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <initializer_list>
#include "Han/MallocAllocator.hpp"

#define ARRAY_INITIAL_SIZE 2
#define ARRAY_INVALID_POS ((size_t)-1)

template<typename T>
struct Array
{
    using Iterator = T*;
    using ConstIterator = const T*;
public:
    Array()
        : Array(MallocAllocator::Instance())
    {}
    
    explicit Array(Allocator* allocator)
        : allocator(allocator)
        , len(0)
        , cap(0)
        , data(nullptr)
    {}

    Array(Allocator* allocator, const std::initializer_list<T>& init_list)
        : allocator(allocator)
        , len(0)
        , cap(0)
        , data(nullptr)
    {
        for (const auto& el : init_list) {
            PushBack(el);
        }
    }

    // Copy semantics
    Array(const Array& arr)
        : allocator(nullptr)
        , len(0)
        , cap(0)
        , data(nullptr)
    {
        *this = arr;
    }

    Array& operator=(const Array& arr)
    {
        if (data) {
            allocator->Deallocate(data);
        }
        allocator = arr.allocator;
        len = arr.len;
        cap = arr.cap;
        data = (T*)allocator->Allocate(arr.cap * sizeof(T));

        assert(data && "copy should not fail");
        memcpy(data, arr.data, sizeof(T) * arr.len);
        return *this;
    }

    // Move semantics
    Array(Array&& arr)
        : allocator(arr.allocator)
        , len(arr.len)
        , cap(arr.cap)
        , data(arr.data)
    {
        arr.allocator = nullptr;
        arr.data = nullptr;
        arr.len = 0;
        arr.cap = 0;
    }

    Array& operator=(Array&& arr)
    {
        if (data) {
            allocator->Deallocate(data);
        }
        allocator = arr.allocator;
        data = arr.data;
        len = arr.len;
        cap = arr.cap;
        arr.allocator = nullptr;
        arr.data = nullptr;
        arr.len = 0;
        arr.cap = 0;
        return *this;
    }

    ~Array() { Clear(); }

	void Clear()
	{
        if (data) {
			for (int64_t i = 0; i < len; ++i) {
				data[i].~T();
			}
            allocator->Deallocate(data);
        }
	}

    void PushBack(T el)
    {
        if (!data) {
            cap = ARRAY_INITIAL_SIZE;
            data = (T*)allocator->Allocate(sizeof(T) * ARRAY_INITIAL_SIZE);
            len = 0;
            assert(data);
        }

        if (len == cap) {
            // Resize array
            size_t new_cap = (size_t)(cap * 1.5f);
			Resize(new_cap);
        }

        new (data + len) T(std::move(el)); // create default instance of T
        ++len;
    }

	ConstIterator Insert(ConstIterator it, T el)
	{
		ASSERT(it >= data, "Iterator should be the same or come after data");

		if (!data) {
            cap = ARRAY_INITIAL_SIZE;
            data = (T*)allocator->Allocate(sizeof(T) * ARRAY_INITIAL_SIZE);
            len = 0;
			ASSERT(data, "Allocation should not fail");
		}

        if (len == cap) {
            // Resize array
            size_t new_cap = (size_t)(cap * 1.5f);
			Resize(new_cap);
        }

		if (it == nullptr) {
			// If the iterator is null, we just insert in the beginning of the array
			ASSERT(len == 0, "Length should be zero");
			new (data) T(std::move(el));
			++len;
			return &data[0];
		}

		// We shift now all elements after it by one position
		// in order to free up space for el.
		int64_t index_to_insert = ((it - data) / sizeof(T)) + 1;

		for (size_t i = len - 1; i >= index_to_insert; --i) {
			new (data + i + 1) T(std::move(data[i]));
		}

		new (data + index_to_insert) T(std::move(el));
		++len;
		return &data[index_to_insert];
	}

	void Remove(const T& el)
	{
		int64_t index = IndexOf(el);
		if (index >= 0) {
			auto it = &data[index];
			Erase(it);
		}
	}

	ConstIterator Erase(ConstIterator it)
	{
		ASSERT(it != nullptr, "Iterator should not be null");
		ASSERT(it >= data, "Iterator should be the same or come after data");
		ASSERT(data, "Should have elements");

		int64_t index_to_remove = (it - data) / sizeof(T);

		ASSERT(index_to_remove >= 0 && index_to_remove < len, "Index to remove should be valid");

		ConstIterator return_it;

		if (index_to_remove == len - 1) {
			// If we are removing the last element, we just call the destructor on it.
			data[index_to_remove].~T();
			return_it = end();
		} else {
			// Move every element that comes after the element to be deleted back one position.
			for (size_t i = index_to_remove + 1; i < len; ++i) {
				data[i - 1] = std::move(data[i]);
			}
			return_it = &data[index_to_remove];
		}

		--len;
		return return_it;
	}

    int64_t IndexOf(const T& el)
    {
        for (int64_t i = 0; i < (int64_t)len; ++i) {
            if (el == data[i]) {
                return i;
            }
        }
        return -1;
    }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

    void Reset() { len = 0; }

    size_t GetLen() const { return len; }
    T* GetData() const { return data; }

    Iterator begin() const
    {
        if (data) {
            return data;
        } else {
            return end();
        }
    }

    Iterator end() const
	{
		return data + len;
	}

    ConstIterator cbegin() const 
    {
        if (data) {
            return data;
        } else {
            return cend();
        }
    }
    ConstIterator cend() const { return data + len; }

private:
	void Resize(size_t new_cap)
	{
		size_t size = new_cap * sizeof(T);
		T* new_data = (T*)allocator->Allocate(size);
		assert(new_data);
		memcpy(new_data, data, len * sizeof(T));
		allocator->Deallocate(data);
		data = new_data;
		cap = new_cap;
	}

public:
    Allocator* allocator;
    size_t len;
    size_t cap;
    T* data;
};
