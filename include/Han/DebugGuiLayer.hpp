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

private:
	bool OnMouseButtonPress(MouseButtonPressEvent& ev);
	bool OnKeyPress(KeyPressEvent& ev);
	bool OnKeyRelease(KeyReleaseEvent& ev);
	bool OnMouseWheel(MouseWheelEvent& ev);
	bool OnMouseMove(MouseMoveEvent& ev);
	bool OnMouseButtonRelease(MouseButtonReleaseEvent& ev);
	bool OnTextInputEvent(TextInputEvent& ev);
};