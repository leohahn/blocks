#include "Han/DebugGuiLayer.hpp"
#include "Han/Logger.hpp"
#include "Han/Application.hpp"
#include "Han/AllocatorFactory.hpp"

#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_opengl3.h"

static bool IsKeyRelevant(KeyCode kc)
{
	switch (kc) {
		case KeyCode_Tab: case KeyCode_Left: case KeyCode_Right: case KeyCode_Up: case KeyCode_Down:
		case KeyCode_PageUp: case KeyCode_PageDown: case KeyCode_Home: case KeyCode_End: case KeyCode_Insert:
		case KeyCode_Delete: case KeyCode_Backspace: case KeyCode_Space: case KeyCode_Enter: case KeyCode_Escape:
		case KeyCode_KpEnter: case KeyCode_A: case KeyCode_C: case KeyCode_V: case KeyCode_X:
		case KeyCode_Y: case KeyCode_Z:
			return true;
		default:
			return false;
	}
}

void
DebugGuiLayer::OnAttach()
{
	LOG_INFO("Initializing debug GUI");
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // We can honor GetMouseCursor() values (optional)
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos requests (optional, rarely used)

	io.DisplaySize = ImVec2(Application::Instance()->GetScreenWidth(), Application::Instance()->GetScreenHeight());

	io.KeyMap[ImGuiKey_Tab] = KeyCode_Tab;
	io.KeyMap[ImGuiKey_LeftArrow] = KeyCode_Left;
	io.KeyMap[ImGuiKey_RightArrow] = KeyCode_Right;
	io.KeyMap[ImGuiKey_UpArrow] = KeyCode_Up;
	io.KeyMap[ImGuiKey_DownArrow] = KeyCode_Down;
	io.KeyMap[ImGuiKey_PageUp] = KeyCode_PageUp;
	io.KeyMap[ImGuiKey_PageDown] = KeyCode_PageDown;
	io.KeyMap[ImGuiKey_Home] = KeyCode_Home;
	io.KeyMap[ImGuiKey_End] = KeyCode_End;
	io.KeyMap[ImGuiKey_Insert] = KeyCode_Insert;
	io.KeyMap[ImGuiKey_Delete] = KeyCode_Delete;
	io.KeyMap[ImGuiKey_Backspace] = KeyCode_Backspace;
	io.KeyMap[ImGuiKey_Space] = KeyCode_Space;
	io.KeyMap[ImGuiKey_Enter] = KeyCode_Enter;
	io.KeyMap[ImGuiKey_Escape] = KeyCode_Escape;
	io.KeyMap[ImGuiKey_KeyPadEnter] = KeyCode_KpEnter;
	io.KeyMap[ImGuiKey_A] = KeyCode_A;
	io.KeyMap[ImGuiKey_C] = KeyCode_C;
	io.KeyMap[ImGuiKey_V] = KeyCode_V;
	io.KeyMap[ImGuiKey_X] = KeyCode_X;
	io.KeyMap[ImGuiKey_Y] = KeyCode_Y;
	io.KeyMap[ImGuiKey_Z] = KeyCode_Z;
}

void
DebugGuiLayer::OnDetach()
{
	LOG_INFO("Shutting down debug GUI");
	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
}

static void
ShowAllocator(const AllocatorFactory::Node& node, const Array<AllocatorFactory::Node>& nodes, int* tree_node_id)
{
	auto name = node.allocator->GetName();
	auto pretty_used_size = Utils::GetPrettySize(node.allocator->GetAllocatedBytes());
	auto pretty_total_size = Utils::GetPrettySize(node.allocator->GetSize());

	bool open = false;

	void* id = (void*)*tree_node_id;
	*tree_node_id += 1;

	int flags = node.children_indices.GetLen() == 0 ? ImGuiTreeNodeFlags_Leaf : ImGuiTreeNodeFlags_None;

	switch (node.allocator->GetType()) {
		case AllocatorType::Linear: open = ImGui::TreeNodeEx(id, flags, "[LinearAllocator] %s: %s of %s", name, pretty_used_size.data, pretty_total_size.data); break;
		case AllocatorType::Malloc: open = ImGui::TreeNodeEx(id, flags, "[MallocAllocator] %s: Used = %s", name, pretty_used_size.data); break;
		default: UNREACHABLE; break;
	}

	// Iterate over each child
	if (open) {
		for (const auto child_index : node.children_indices) {
			ASSERT(child_index >= 0 && child_index < nodes.GetLen(), "Index should be valid based on the size of the nodes array");
			const auto& child_node = nodes[child_index];
			ShowAllocator(child_node, nodes, tree_node_id);
		}
	    ImGui::TreePop();
	}
}

void
DebugGuiLayer::OnUpdate(DeltaTime delta)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = (float)delta;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	static bool show = true;
	if (show) {
		ImGui::ShowDemoWindow(&show);
	}

	int tree_node_id = 1;

	// Memory Profiler window
	ImGui::Begin("MemoryProfiler");
	const auto& nodes = AllocatorFactory::Instance().GetNodes();
	for (const auto& node : nodes) {
		if (node.type == AllocatorFactory::NodeType::Root) {
			ShowAllocator(node, nodes, &tree_node_id);
		}
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void
DebugGuiLayer::OnEvent(Event& ev)
{
	EventDispatcher dispatcher(ev);
	dispatcher.Dispatch<MouseButtonPressEvent>(HAN_BIND_EV_HANDLER(DebugGuiLayer::OnMouseButtonPress));
	dispatcher.Dispatch<MouseButtonReleaseEvent>(HAN_BIND_EV_HANDLER(DebugGuiLayer::OnMouseButtonRelease));
	dispatcher.Dispatch<KeyPressEvent>(HAN_BIND_EV_HANDLER(DebugGuiLayer::OnKeyPress));
	dispatcher.Dispatch<KeyReleaseEvent>(HAN_BIND_EV_HANDLER(DebugGuiLayer::OnKeyRelease));
	dispatcher.Dispatch<MouseWheelEvent>(HAN_BIND_EV_HANDLER(DebugGuiLayer::OnMouseWheel));
	dispatcher.Dispatch<MouseMoveEvent>(HAN_BIND_EV_HANDLER(DebugGuiLayer::OnMouseMove));
	dispatcher.Dispatch<TextInputEvent>(HAN_BIND_EV_HANDLER(DebugGuiLayer::OnTextInputEvent));
}

bool
DebugGuiLayer::OnMouseMove(MouseMoveEvent& ev)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MousePos = ImVec2(ev.x, ev.y);
	return false;
}

bool
DebugGuiLayer::OnMouseButtonPress(MouseButtonPressEvent& ev)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[ev.button] = true;
	return false;
}

bool
DebugGuiLayer::OnMouseButtonRelease(MouseButtonReleaseEvent& ev)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseDown[ev.button] = false;
	return false;
}

bool
DebugGuiLayer::OnKeyPress(KeyPressEvent& ev)
{
	ImGuiIO& io = ImGui::GetIO();
	ASSERT((uint32_t)ev.key_code >= 0 && (uint32_t)ev.key_code < ARRAY_SIZE(io.KeysDown), "Array should be large enough");
	if (IsKeyRelevant(ev.key_code)) {
		io.KeysDown[ev.key_code] = true;
		io.KeyShift = ((ev.mod_flags & KeyMod_Shift) != 0);
		io.KeyCtrl = ((ev.mod_flags & KeyMod_Ctrl) != 0);
		io.KeyAlt = ((ev.mod_flags & KeyMod_Alt) != 0);
		io.KeySuper = ((ev.mod_flags & KeyMod_Super) != 0);
	}
	return false;
}

bool
DebugGuiLayer::OnKeyRelease(KeyReleaseEvent& ev)
{
	ImGuiIO& io = ImGui::GetIO();
	ASSERT((uint32_t)ev.key_code >= 0 && (uint32_t)ev.key_code < ARRAY_SIZE(io.KeysDown), "Array should be large enough");
	if (IsKeyRelevant(ev.key_code)) {
		io.KeysDown[ev.key_code] = false;
		io.KeyShift = ((ev.mod_flags & KeyMod_Shift) != 0);
		io.KeyCtrl = ((ev.mod_flags & KeyMod_Ctrl) != 0);
		io.KeyAlt = ((ev.mod_flags & KeyMod_Alt) != 0);
		io.KeySuper = ((ev.mod_flags & KeyMod_Super) != 0);
	}
	return false;
}

bool
DebugGuiLayer::OnMouseWheel(MouseWheelEvent& ev)
{
	ImGuiIO& io = ImGui::GetIO();
	io.MouseWheel += ev.y;
	io.MouseWheelH += ev.x;
	return false;
}

bool
DebugGuiLayer::OnTextInputEvent(TextInputEvent& ev)
{
	ImGuiIO& io = ImGui::GetIO();
	io.AddInputCharactersUTF8(ev.text);
	return false;
}
