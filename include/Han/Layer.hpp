#pragma once

#include "Han/Core.hpp"
#include "Han/Events.hpp"
#include "Han/Collections/String.hpp"
#include "Han/Collections/Array.hpp"

class Layer
{
public:
	Layer(String name)
		: _name(name)
	{}

	virtual ~Layer() = default;
	virtual void OnAttach() {}
	virtual void OnDetach() {}
	virtual void OnUpdate(DeltaTime delta) {}
	virtual void OnEvent(Event& ev) {}

	String GetDebugName() const { return _name; }

protected:
	// NOTE: Consider adding ifdef for debug builds only
	String _name;
};

class LayerStack
{
public:
	LayerStack()
		: LayerStack(nullptr)
	{}
	LayerStack(Allocator* allocator);
	~LayerStack();

	void Clear();

	void PushLayer(Layer* layer);
	void PopLayer(Layer* layer);

	void PushOverlay(Layer* layer);
	void PopOverlay(Layer* layer);

	Allocator* GetAllocator() { return _alloc; }
	void SetAllocator(Allocator* allocator);

	Array<Layer*>::Iterator begin() { return _layers.begin(); }
	Array<Layer*>::Iterator end() { return _layers.end(); }

private:
	Allocator* _alloc;
	Array<Layer*> _layers;
	Array<Layer*>::ConstIterator _layers_insert;
};
