#pragma once

#include "milo/common/Common.h"
#include "milo/input/Keyboard.h"
#include "milo/input/Mouse.h"

namespace milo {

	enum class EventType {
		KeyRelease = 0,
		KeyPress,
		KeyRepeat,
		KeyType,
		MouseEnter,
		MouseExit,
		MouseMove,
		MouseButtonRelease,
		MouseButtonPress,
		MouseButtonRepeat,
		MouseButtonClick,
		MouseScroll,
		WindowClose,
		WindowFocus,
		WindowMove,
		WindowResize,
		WindowMaximized,
		WindowMinimized,
		ApplicationExit,
		MaxEnumValue
	};

	const int32 MAX_EVENT_SIZE = 32;

	struct Event {
		EventType type = EventType::MaxEnumValue;
	};

	// === KEYBOARD EVENTS ===

	struct KeyEvent : public Event {
		KeyAction action{0};
		Key key{0};
		Scancode scancode{0};
		KeyModifiersBitMask modifiers{0};
	};

	struct KeyPressEvent : public KeyEvent {
		KeyPressEvent() : KeyEvent() {
			type = EventType::KeyPress;
			action = KeyAction::Press;
		}
	};

	struct KeyReleaseEvent : public KeyEvent {
		KeyReleaseEvent() : KeyEvent() {
			type = EventType::KeyRelease;
			action = KeyAction::Release;
		}
	};

	struct KeyRepeatEvent : public KeyEvent {
		KeyRepeatEvent() : KeyEvent() {
			type = EventType::KeyRepeat;
			action = KeyAction::Repeat;
		}
	};

	struct KeyTypeEvent : public KeyEvent {
		KeyTypeEvent() : KeyEvent() {
			type = EventType::KeyType;
			action = KeyAction::Type;
		}
	};

	// === MOUSE EVENTS ===

	struct MouseButtonEvent : public Event {
		MouseButtonAction action;
		MouseButton button;
		KeyModifiersBitMask modifiers;
	};

	struct MouseButtonPressEvent : public MouseButtonEvent {
		MouseButtonPressEvent() : MouseButtonEvent() {
			type = EventType::MouseButtonPress;
			action = MouseButtonAction::Press;
		}
	};

	struct MouseButtonReleaseEvent : public MouseButtonEvent {
		MouseButtonReleaseEvent() : MouseButtonEvent() {
			type = EventType::MouseButtonRelease;
			action = MouseButtonAction::Release;
		}
	};

	struct MouseButtonRepeatEvent : public MouseButtonEvent {
		MouseButtonRepeatEvent() : MouseButtonEvent() {
			type = EventType::MouseButtonRepeat;
			action = MouseButtonAction::Repeat;
		}
	};

	struct MouseButtonClickEvent : public MouseButtonEvent {
		MouseButtonClickEvent() : MouseButtonEvent() {
			type = EventType::MouseButtonClick;
			action = MouseButtonAction::Click;
		}
	};

	struct MouseMoveEvent : public Event {
		Vector2 position{0};

		MouseMoveEvent() : Event() {
			type = EventType::MouseMove;
		}
	};

	struct MouseEnterEvent : public Event {
		MouseEnterEvent() : Event() {
			type = EventType::MouseEnter;
		}
	};

	struct MouseExitEvent : public Event {
		MouseExitEvent() : Event() {
			type = EventType::MouseExit;
		}
	};

	struct MouseScrollEvent : public Event {
		Vector2 offset{0};
		MouseScrollEvent() : Event() {
			type = EventType::MouseScroll;
		}
	};

	// === APPLICATION EVENTS ===

	struct ApplicationExitEvent : public Event {
		ApplicationExitEvent() : Event() {
			type = EventType::ApplicationExit;
		}
	};

	// === WINDOW EVENTS ===

	struct WindowCloseEvent : public Event {
		WindowCloseEvent() : Event() {
			type = EventType::WindowClose;
		}
	};

	struct WindowFocusEvent : public Event {
		bool focused{false};

		WindowFocusEvent() : Event() {
			type = EventType::WindowFocus;
		}
	};

	struct WindowMoveEvent : public Event {
		Vector2i position{0};

		WindowMoveEvent() : Event() {
			type = EventType::WindowMove;
		}
	};

	struct WindowResizeEvent : public Event {
		Vector2i size{0};

		WindowResizeEvent() : Event() {
			type = EventType::WindowResize;
		};
	};

	struct WindowMaximizedEvent : public Event {
		bool maximized{false};

		WindowMaximizedEvent() : Event() {
			type = EventType::WindowMaximized;
		}
	};

	struct WindowMinimizedEvent : public Event {
		WindowMinimizedEvent() : Event() {
			type = EventType::WindowMinimized;
		}
	};

	// ==== ====

	using EventCallback = Function<void, const Event&>;
}
