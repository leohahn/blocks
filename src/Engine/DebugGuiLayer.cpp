#include "Han/DebugGuiLayer.hpp"
#include "Han/Logger.hpp"
#include "Han/Application.hpp"

#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_opengl3.h"

void
DebugGuiLayer::OnAttach()
{
	LOG_INFO("Initializing debug GUI");
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(Application::Instance()->GetScreenWidth(), Application::Instance()->GetScreenHeight());
}

void
DebugGuiLayer::OnDetach()
{
	LOG_INFO("Shutting down debug GUI");
	ImGui_ImplOpenGL3_Shutdown();
}

void
DebugGuiLayer::OnUpdate(DeltaTime delta)
{
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = (float)delta;

	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	static bool show = true;
	ImGui::ShowDemoWindow(&show);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void
DebugGuiLayer::OnEvent(Event& ev)
{

}
