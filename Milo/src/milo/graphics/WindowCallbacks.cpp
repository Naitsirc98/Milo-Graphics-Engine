#include "milo/graphics/WindowCallbacks.h"
#include "milo/events/EventSystem.h"

namespace milo::WindowCallbacks {

	void keyCallback(GLFWwindow* window, int glfwKey, int scancode, int action, int glfwMod) {
		KeyEvent event;
		event.key = static_cast<Key>(glfwKey);
		event.scancode = scancode;
		event.modifiers = static_cast<KeyModifiersBitMask>(glfwMod);

		switch(action) {
			case GLFW_RELEASE:
				event.type = EventType::KeyRelease;
				event.action = KeyAction::Release;
				break;
			case GLFW_PRESS:
				event.type = EventType::KeyPress;
				event.action = KeyAction::Press;
				break;
			case GLFW_REPEAT:
				event.type = EventType::KeyRepeat;
				event.action = KeyAction::Repeat;
				break;
			default:
				return;
		}

		EventSystem::publishEvent(event);
	}

	void mouseButtonCallback(GLFWwindow* window, int glfwButton, int action, int mods) {
		MouseButtonEvent event = {};
		event.button = static_cast<MouseButton>(glfwButton);
		event.modifiers = static_cast<KeyModifiersBitMask>(mods);

		switch(action) {
			case GLFW_RELEASE:
				event.type = EventType::MouseButtonRelease;
				event.action = MouseButtonAction::Release;
				break;
			case GLFW_PRESS:
				event.type = EventType::MouseButtonPress;
				event.action = MouseButtonAction::Press;
				break;
			case GLFW_REPEAT:
				event.type = EventType::MouseButtonRepeat;
				event.action = MouseButtonAction::Repeat;
				break;
			default:
				return;
		}

		EventSystem::publishEvent(event);
	}

	void mouseMoveCallback(GLFWwindow* window, double x, double y) {
		MouseMoveEvent event = {};
		event.position = {static_cast<float>(x), static_cast<float>(y)};
		EventSystem::publishEvent(event);
	}

	void mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
		MouseScrollEvent event = {};
		event.offset = {static_cast<float>(xOffset), static_cast<float>(yOffset)};
		EventSystem::publishEvent(event);
	}

	void mouseEnterCallback(GLFWwindow* window, int entered) {
		if(entered == GLFW_TRUE) EventSystem::publishEvent(MouseEnterEvent());
		else EventSystem::publishEvent(MouseExitEvent());
	}

	void windowCloseCallback(GLFWwindow* window) {
		EventSystem::publishEvent(WindowCloseEvent());
	}

	void windowFocusCallback(GLFWwindow* window, int focused) {
		WindowFocusEvent event = {};
		event.focused = focused == GLFW_TRUE;
		EventSystem::publishEvent(event);
	}

	void windowPosCallback(GLFWwindow* window, int x, int y) {
		WindowMoveEvent event = {};
		event.position = {x, y};
		EventSystem::publishEvent(event);
	}

	void windowSizeCallback(GLFWwindow* window, int width, int height) {
		WindowResizeEvent event = {};
		event.size = {width, height};
		EventSystem::publishEvent(event);
		if(width == 0 && height == 0) EventSystem::publishEvent(WindowMinimizedEvent());
	}

	void windowMaximizeCallback(GLFWwindow* window, int maximized) {
		WindowMaximizedEvent event = {};
		event.maximized = maximized == GLFW_TRUE;
		EventSystem::publishEvent(event);
	}
}