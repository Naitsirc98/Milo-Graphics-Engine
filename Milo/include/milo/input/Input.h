#pragma once

#include "Keyboard.h"
#include "Mouse.h"
#include "milo/events/EventSystem.h"
#include "milo/graphics/Window.h"

namespace milo {

	class Input {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		Keyboard m_Keyboard;
		Mouse m_Mouse;
	private:
		Input() = default;
		void setEventCallbacks();
	public:
		static KeyAction getKey(Key key);
		static bool isKeyReleased(Key key);
		static bool isKeyPressed(Key key);
		static bool isKeyRepeated(Key key);
		static bool isKeyTyped(Key key);
		static bool isKeyActive(Key key);

		static MouseButtonAction getMouseButton(MouseButton button);
		static bool isMouseButtonReleased(MouseButton button);
		static bool isMouseButtonPressed(MouseButton button);
		static bool isMouseButtonRepeated(MouseButton button);
		static bool isMouseButtonClicked(MouseButton button);

		static const Vector2& getMousePosition();
		static const Vector2& getMouseScroll();

		static CursorMode cursorMode();
		static void setCursorMode(CursorMode mode);
	private:
		static Input* s_Instance;
	private:
		static void init();
		static void shutdown();
		static void update();
	};


}