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

	static const int32_t WINDOW_DEFAULT_WIDTH = 1280;
	static const int32_t WINDOW_DEFAULT_HEIGHT = 720;
	static const Size WINDOW_DEFAULT_SIZE = {WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT};

	struct WindowInfo {
		String title;
		int32_t width = WINDOW_DEFAULT_WIDTH;
		int32_t height = WINDOW_DEFAULT_HEIGHT;
		WindowState state = WindowState::Windowed;
		CursorMode cursorMode = CursorMode::Normal;
		GraphicsAPI graphicsAPI = GraphicsAPI::Default;
	};

	class Window {
		friend class Graphics;
	private:
		GLFWwindow* m_Handle = nullptr;
		WindowState m_State = WindowState::Windowed;
		String m_Title;
	private:
		explicit Window(const WindowInfo& info);
		~Window();
	public:
		explicit Window(const Window& other) = delete;
		Window& operator=(const Window& other) = delete;
		[[nodiscard]] GLFWwindow* handle() const;
		[[nodiscard]] const String& title() const;
		Window& title(const String& title);
		Window& title(String&& title);
		[[nodiscard]] Size framebufferSize() const;
		[[nodiscard]] Size size() const;
		Window& size(const Size& size);
		Window& size(int32_t width, int32_t height);
		[[nodiscard]] float aspectRatio() const;
		[[nodiscard]] Vector2i position() const;
		Window& position(const Vector2i& position);
		Window& position(int32_t x, int32_t y);
		[[nodiscard]] WindowState state() const;
		Window& state(WindowState state);
		[[nodiscard]] CursorMode cursorMode() const;
		Window& cursorMode(CursorMode cursorMode);
		bool shouldClose() const;
		void close();
		void hide();
		void restore();
		void show();
		void focus();
	private:
		void setEventCallbacks();
	private:
		static Window* s_MainWindow;
	public:
		static Window& getMainWindow();
		static Window& get();
	private:
		static GLFWwindow* createWindow(const WindowInfo& info);
		static void setWindowHints(const WindowInfo& info);
	};
}