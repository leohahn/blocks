#include "Han/AllocatorFactory.hpp"
#include "Han/Core.hpp"
#include "Han/Allocator.hpp"
#include "Han/Utils.hpp"
#include <algorithm>
#include <functional>

AllocatorFactory::~AllocatorFactory()
{
	for (auto& node : _nodes) {
		if (node.owns) {
			_allocator->Delete(node.allocator);
		}
	}
}

void
AllocatorFactory::Initialize(Allocator* allocator)
{
	_allocator = allocator;
	_nodes.PushBack(Node(NodeType::Root, allocator, false));
}

void
AllocatorFactory::AddAllocator(Allocator* parent_allocator, Allocator* child_allocator)
{
	auto node_type = parent_allocator == nullptr ? NodeType::Root : NodeType::Child;
	_nodes.PushBack(Node(node_type, child_allocator, true));
	size_t added_index = _nodes.GetLen() - 1;

	if (parent_allocator) {
		LOG_DEBUG(
			"Adding allocator %s (%s) child of %s",
			child_allocator->GetName(),
			Utils::GetPrettySize(child_allocator->GetSize()).data, parent_allocator->GetName());
		auto parent_it = std::find_if(_nodes.begin(), _nodes.end(), [parent_allocator](const Node& node) -> bool {
			return node.allocator->GetName() == parent_allocator->GetName();
		});
		ASSERT(parent_it != _nodes.end(), "Parent allocator should exist");
		parent_it->AddChild(added_index);
	} else {
		LOG_DEBUG(
			"Adding root allocator %s of size %s",
			child_allocator->GetName(),
			Utils::GetPrettySize(child_allocator->GetSize()).data);
	}
}

void
AllocatorFactory::Node::AddChild(size_t index)
{
	children_indices.PushBack(index);
}
