#pragma once

#include "Han/Layer.hpp"

class DebugGuiLayer : public Layer
{
public:
	DebugGuiLayer()
		: Layer(String("DebugGui"))
	{}

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(DeltaTime delta) override;
	void OnEvent(Event& ev) override;
};