#include "milo/graphics/Window.h"
#include "milo/graphics/Graphics.h"

namespace milo {

	Window::Window(const WindowInfo& info) {
		m_Handle = createWindow(info);
	}

	Window::~Window() {
		glfwDestroyWindow(m_Handle);
		m_Handle = nullptr;
	}

	GLFWwindow* Window::handle() const {
		return m_Handle;
	}

	Size Window::size() const {
		Size size;
		glfwGetWindowSize(m_Handle, &size.width, &size.height);
		return size;
	}

	Window& Window::size(const Size& size) {
		return this->size(size.width, size.height);
	}

	Window& Window::size(size_t width, size_t height) {
		glfwSetWindowSize(m_Handle, width, height);
		return *this;
	}

	Vector2i Window::position() const {
		Vector2i pos;
		glfwGetWindowPos(m_Handle, &pos.x, &pos.y);
		return pos;
	}

	Window& Window::position(const Vector2i& position) {
		return this->position(position.x, position.y);
	}

	Window& Window::position(size_t x, size_t y) {
		glfwSetWindowPos(m_Handle, x, y);
		return *this;
	}

	WindowState Window::state() const {
		return m_State;
	}

	Window& Window::state(WindowState state) {
		// TODO
		m_State = state;
		return *this;
	}

	CursorMode Window::cursorMode() const {
		return m_CursorMode;
	}

	Window& Window::cursorMode(CursorMode cursorMode) {
		m_CursorMode = cursorMode;
		return *this;
	}

	Window& Window::getMainWindow() {
		return s_MainWindow;
	}

	GLFWwindow* Window::createWindow(const WindowInfo& info) {

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		Size size = {info.width, info.height};

		if(info.state != WindowState::Windowed) {
			const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
			size.width = vidmode->width;
			size.height = vidmode->height;
		}

		if(info.state != WindowState::Fullscreen) {
			monitor = nullptr;
		}

		return glfwCreateWindow(size.width, size.height, info.title.c_str(), monitor, nullptr);
	}
}