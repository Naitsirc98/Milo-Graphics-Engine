#include <milo/events/EventSystem.h>
#include "milo/graphics/Window.h"
#include "milo/graphics/WindowCallbacks.h"
#include "milo/graphics/Graphics.h"

namespace milo {

	Window* Window::s_MainWindow = nullptr;

	Window::Window(const WindowInfo& info) : m_Title(info.title) {
		m_Handle = createWindow(info);
		setEventCallbacks();
	}

	Window::~Window() {
		glfwDestroyWindow(m_Handle);
		m_Handle = nullptr;
	}

	GLFWwindow* Window::handle() const {
		return m_Handle;
	}

	const String& Window::title() const {
		return m_Title;
	}

	Window& Window::title(const String& title) {
		m_Title = title;
		glfwSetWindowTitle(m_Handle, m_Title.c_str());
		return *this;
	}

	Window& Window::title(String&& title) {
		m_Title = std::move(title);
		glfwSetWindowTitle(m_Handle, m_Title.c_str());
		return *this;
	}

	Size Window::framebufferSize() const {
		Size size;
		glfwGetFramebufferSize(m_Handle, &size.width, &size.height);
		return size;
	}

	Size Window::size() const {
		Size size;
		glfwGetWindowSize(m_Handle, &size.width, &size.height);
		return size;
	}

	Window& Window::size(const Size& size) {
		return this->size(size.width, size.height);
	}

	Window& Window::size(int32_t width, int32_t height) {
		glfwSetWindowSize(m_Handle, width, height);
		return *this;
	}

	float Window::aspectRatio() const {
		return size().aspect();
	}

	Vector2i Window::position() const {
		Vector2i pos;
		glfwGetWindowPos(m_Handle, &pos.x, &pos.y);
		return pos;
	}

	Window& Window::position(const Vector2i& position) {
		return this->position(position.x, position.y);
	}

	Window& Window::position(int32_t x, int32_t y) {
		glfwSetWindowPos(m_Handle, x, y);
		return *this;
	}

	WindowState Window::state() const {
		return m_State;
	}

	inline Vector2i center(const Size& windowSize, const GLFWvidmode* vidmode) {
		const int32_t x = (vidmode->width - windowSize.width) / 2;
		const int32_t y = (vidmode->height - windowSize.height) / 2;
		return {x, y};
	}

	Window& Window::state(WindowState state) {
		if(m_State == state) return *this;

		m_State = state;

		glfwRestoreWindow(m_Handle);

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* vidmode = glfwGetVideoMode(monitor);
		const Size& size = state == WindowState::Windowed ? WINDOW_DEFAULT_SIZE : Size(vidmode->width, vidmode->height);
		const Vector2i position = state == WindowState::Fullscreen ? Vector2i(0, 0) : center(size, vidmode);

		glfwSetWindowMonitor(
				m_Handle,
				state == WindowState::Fullscreen ? monitor : nullptr,
				position.x, position.y,
				size.width, size.height,
				vidmode->refreshRate);

		return *this;
	}

	CursorMode Window::cursorMode() const {
		switch (glfwGetInputMode(m_Handle, GLFW_CURSOR)) {
			case GLFW_CURSOR_HIDDEN: return CursorMode::Hidden;
			case GLFW_CURSOR_DISABLED: return CursorMode::Captured;
			case GLFW_CURSOR_NORMAL: default: return CursorMode::Normal;
		}
	}

	Window& Window::cursorMode(CursorMode cursorMode) {
		int glfwMode = GLFW_CURSOR_NORMAL;
		switch (cursorMode) {
			case CursorMode::Normal:
				glfwMode = GLFW_CURSOR_NORMAL;
				break;
			case CursorMode::Hidden:
				glfwMode = GLFW_CURSOR_HIDDEN;
				break;
			case CursorMode::Captured:
				glfwMode = GLFW_CURSOR_DISABLED;
				break;
		}
		glfwSetInputMode(m_Handle, GLFW_CURSOR, glfwMode);
		return *this;
	}

	bool Window::shouldClose() const {
		return glfwWindowShouldClose(m_Handle);
	}

	void Window::close() {
		glfwSetWindowShouldClose(m_Handle, GLFW_TRUE);
	}

	void Window::hide() {
		m_State = WindowState::Minimized;
		glfwHideWindow(m_Handle);
	}

	void Window::restore() {
		m_State = WindowState::Windowed;
		glfwRestoreWindow(m_Handle);
	}

	void Window::show() {
		if(m_State == WindowState::Minimized) m_State = WindowState::Windowed;
		glfwShowWindow(m_Handle);
	}

	void Window::focus() {
		glfwFocusWindow(m_Handle);
	}


	// ======


	Window* Window::getMainWindow() {
		return s_MainWindow;
	}

	Window* Window::get() {
		return s_MainWindow;
	}

	GLFWwindow* Window::createWindow(const WindowInfo& info) {

		if(!glfwInit()) {
			String message;
			message.resize(1024, '\0');
			const char* messageBuffer = message.c_str();
			glfwGetError(&messageBuffer);
			throw MILO_RUNTIME_EXCEPTION(String("Failed to create initialize GLFW: ").append(messageBuffer));
		}

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

		setWindowHints(info);

		GLFWwindow* window = glfwCreateWindow(size.width, size.height, info.title.c_str(), monitor, nullptr);

		if(window == nullptr) {
			String message;
			message.resize(1024, '\0');
			const char* messageBuffer = message.c_str();
			glfwGetError(&messageBuffer);
			throw MILO_RUNTIME_EXCEPTION(String("Failed to create GLFW window: ").append(messageBuffer));
		}

		return window;
	}

	void Window::setEventCallbacks() {

		glfwSetKeyCallback(m_Handle, WindowCallbacks::keyCallback);
		glfwSetMouseButtonCallback(m_Handle, WindowCallbacks::mouseButtonCallback);
		glfwSetCursorPosCallback(m_Handle, WindowCallbacks::mouseMoveCallback);
		glfwSetScrollCallback(m_Handle, WindowCallbacks::mouseScrollCallback);
		glfwSetCursorEnterCallback(m_Handle, WindowCallbacks::mouseEnterCallback);
		glfwSetWindowCloseCallback(m_Handle, WindowCallbacks::windowCloseCallback);
		glfwSetWindowFocusCallback(m_Handle, WindowCallbacks::windowFocusCallback);
		glfwSetWindowPosCallback(m_Handle, WindowCallbacks::windowPosCallback);
		glfwSetWindowSizeCallback(m_Handle, WindowCallbacks::windowSizeCallback);
		glfwSetWindowMaximizeCallback(m_Handle, WindowCallbacks::windowMaximizeCallback);

		EventSystem::addEventCallback(EventType::WindowMaximized, [&](const Event& e) {
			const auto& event = static_cast<const WindowMaximizedEvent&>(e);
			if(event.maximized)
				m_State = WindowState::Maximized;
			else
				m_State = WindowState::Windowed;
		});

		EventSystem::addEventCallback(EventType::WindowMinimized, [&](const Event& e) {
			m_State = WindowState::Minimized;
		});

		EventSystem::addEventCallback(EventType::WindowResize, [&](const Event& e){
			const auto& event = static_cast<const WindowResizeEvent&>(e);
			if(m_State == WindowState::Minimized && (event.size.width > 0 || event.size.height > 0))
				m_State = WindowState::Windowed;
		});

	}

	void Window::setWindowHints(const WindowInfo& info) {

		glfwDefaultWindowHints();

		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);

		if(info.graphicsAPI == GraphicsAPI::Vulkan) {
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		} else {
			throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
		}
	}
}