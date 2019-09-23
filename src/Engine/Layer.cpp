#include "Han/Layer.hpp"

LayerStack::LayerStack(Allocator* allocator)
	: _alloc(allocator)
	, _layers(allocator)
	, _layers_insert(_layers.begin())
{}

LayerStack::~LayerStack()
{
	Clear();
}

void
LayerStack::Clear()
{
	for (auto layer : _layers) {
		layer->OnDetach();
		_alloc->Delete(layer);
	}
	_layers.Clear();
}

void
LayerStack::SetAllocator(Allocator* allocator)
{
	_alloc = allocator;
	_layers.allocator = allocator;
}

void
LayerStack::PushLayer(Layer* layer)
{
	ASSERT(layer, "layer should exist");
	_layers.Insert(_layers_insert, layer);
	layer->OnAttach();
}

void
LayerStack::PopLayer(Layer* layer)
{
	ASSERT(layer, "layer should exist");
	_layers.Remove(layer);
	layer->OnDetach();
}

void
LayerStack::PushOverlay(Layer* layer)
{
	ASSERT(layer, "layer should exist");
	_layers.PushBack(layer);
	layer->OnAttach();
}

void
LayerStack::PopOverlay(Layer* layer)
{
	ASSERT(layer, "layer should exist");
	_layers.Remove(layer);
	layer->OnDetach();
}
