#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

// EDITOR: 0. Include editor.h
#include "editor.h"

GLFWwindow* window;
GLint wWindow = 1440;
GLint hWindow = 900;
const std::string wTitle = "Editor Demo Application";

// EDITOR: 1. Create instance
ShaderNodeEditor editor;

void DrawGui()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// EDITOR: 5. Call DrawGui() on ImGui frame
	editor.DrawGui();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Display()
{
	// EDITOR: 4. Call Display() on frame
	editor.Display();

	DrawGui();

	// Be sure to swap buffers by the end of every frame
	glfwSwapBuffers(window);
}

void Idle()
{
	Display();
}

void Reshape(GLFWwindow* window, int w, int h)
{
	wWindow = w;
	hWindow = h;
	// EDITOR: 3. Handle window reshape
	editor.SetRenderSize(w, h);
	Display();
}

void Close(GLFWwindow* window)
{
	glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int InitializeGL(GLFWwindow*& window)
{
	if (!glfwInit())
		return -1;
	
	window = glfwCreateWindow(wWindow, hWindow, wTitle.c_str(), NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, Reshape);
	glfwSetWindowCloseCallback(window, Close);

	glfwMakeContextCurrent(window);

	glewInit();

	glEnable(GL_DEPTH_TEST);

	return 0;
}

void InitializeImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 150");

	ImGui::GetIO().IniFilename = 0;
}

void GlfwLoop()
{
	while (!glfwWindowShouldClose(window))
	{
		Idle();
		glfwPollEvents();
	}
}

int main(int argc, char** argv)
{
	int initRes = InitializeGL(window);
	if (initRes)
		return initRes;

	InitializeImGui();

	// EDITOR: 2. Initialize editor
	editor.SetRenderSize(wWindow, hWindow);
	editor.Initialize();

	GlfwLoop();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();

	return 0;
}
