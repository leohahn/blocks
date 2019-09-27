#pragma once

#include "Han/Core.hpp"
#include "Han/Collections/Array.hpp"

// We declare the class Allocator here. We do not include the header,
// since there would then be a circular dependency between MemoryProfiler and Allocator.
struct Allocator;

class AllocatorFactory
{
public:
	enum class NodeType
	{
		Root,
		Child,
	};

	struct Node
	{
		NodeType type;
		Allocator* allocator;
		// We assume that the indices of the children do not change,
		// that is, the order of the elements in the nodes array does not change.
		Array<size_t> children_indices;
		bool owns;

		explicit Node(NodeType type, Allocator* alloc, bool owns)
			: type(type)
			, allocator(alloc)
			, owns(owns)
		{}

		void AddChild(size_t index);
	};

	~AllocatorFactory();

	// NOTE, TODO(leo): We statically create an instance of the MemoryProfiler. This will need to be changed
	// if the order of the initialization starts to matter compared to other singletons.
	void Initialize(Allocator* allocator);
	static AllocatorFactory& Instance()
	{
		static AllocatorFactory instance;
		return instance;
	}

	template<typename T, typename... Args>
	T* CreateFromParent(Allocator* parent, Args&&... args)
	{
		ASSERT(parent, "Parent should not be null");
		T* child_allocator = _allocator->New<T>(std::forward<Args>(args)...);
		AddAllocator(parent, static_cast<Allocator*>(child_allocator));
		return child_allocator;
	}

	template<typename T, typename... Args>
	T* Create(Args&&... args)
	{
		T* child_allocator = _allocator->New<T>(std::forward<Args>(args)...);
		AddAllocator(nullptr, static_cast<Allocator*>(child_allocator));
		return child_allocator;
	}

	const Array<Node>& GetNodes() const { return _nodes; }

private:
	AllocatorFactory() = default;
	void AddAllocator(Allocator* parent_allocator, Allocator* child_allocator);

private:
	Allocator* _allocator;
	Array<Node> _nodes;
};
