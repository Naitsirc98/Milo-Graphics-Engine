#pragma once

#include "Window.h"

namespace milo::WindowCallbacks {
	void keyCallback(GLFWwindow* window, int glfwKey, int scancode, int action, int glfwMod);
	void mouseButtonCallback(GLFWwindow* window, int glfwButton, int action, int mods);
	void mouseMoveCallback(GLFWwindow* window, double x, double y);
	void mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	void mouseEnterCallback(GLFWwindow* window, int entered);
	void windowCloseCallback(GLFWwindow* window);
	void windowFocusCallback(GLFWwindow* window, int focused);
	void windowPosCallback(GLFWwindow* window, int x, int y);
	void windowSizeCallback(GLFWwindow* window, int width, int height);
	void windowMaximizeCallback(GLFWwindow* window, int maximized);
}