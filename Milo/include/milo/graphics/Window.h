#pragma once

#include "milo/common/Common.h"
#include "milo/graphics/GraphicsAPI.h"

#include <GLFW/glfw3.h>

namespace milo {

	enum class CursorMode {
		Normal,
		Hidden,
		Captured
	};

	enum class WindowState {
		Minimized,
		Windowed,
		Maximized,
		Fullscreen
	};

	struct WindowInfo {
		String title = "";
		size_t width = 1280;
		size_t height = 720;
		WindowState state = WindowState::Windowed;
		CursorMode cursorMode = CursorMode::Normal;
		GraphicsAPI graphicsAPI = GraphicsAPI::Default;
	};

	class Window {
		friend class Graphics;
	private:
		GLFWwindow* m_Handle = nullptr;
		WindowState m_State = WindowState::Windowed;
		CursorMode m_CursorMode = CursorMode::Normal;
	private:
		explicit Window(const WindowInfo& info);
		~Window();
	public:
		explicit Window(const Window& other) = delete;
		Window& operator=(const Window& other) = delete;
		[[nodiscard]] GLFWwindow* handle() const;
		[[nodiscard]] Size size() const;
		Window& size(const Size& size);
		Window& size(size_t width, size_t height);
		[[nodiscard]] Vector2i position() const;
		Window& position(const Vector2i& position);
		Window& position(size_t x, size_t y);
		[[nodiscard]] WindowState state() const;
		Window& state(WindowState state);
		[[nodiscard]] CursorMode cursorMode() const;
		Window& cursorMode(CursorMode cursorMode);
	private:
		static Window* s_MainWindow;
	public:
		static Window& getMainWindow();
	private:
		static GLFWwindow* createWindow(const WindowInfo& info);
	};
}