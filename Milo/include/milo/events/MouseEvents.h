#pragma once
#include "Event.h"
#include "milo/input/Mouse.h"

namespace milo {

	class MouseButtonEvent : public Event {
	public:
		MouseButtonEvent(EventType type, MouseButton button, KeyModifiersBitMask modifiers);
		MouseButton button() const;
		KeyModifiersBitMask modifiers() const;
	};

	class MouseButtonReleaseEvent : public MouseButtonEvent {
	public:
		MouseButtonReleaseEvent(MouseButton button, KeyModifiersBitMask modifiers);
	};

	class MouseButtonPressEvent : public MouseButtonEvent {
	public:
		MouseButtonPressEvent(MouseButton button, KeyModifiersBitMask modifiers);
	};

	class MouseButtonRepeatEvent : public MouseButtonEvent {
	public:
		MouseButtonRepeatEvent(MouseButton button, KeyModifiersBitMask modifiers);
	};

	class MouseButtonClickEvent : public MouseButtonEvent {
	public:
		MouseButtonClickEvent(MouseButton button, KeyModifiersBitMask modifiers);
	};

	class MouseMoveEvent : public Event {
	public:
		explicit MouseMoveEvent(const Vector2& position);
		const Vector2& position() const;
	};

	class MouseEnterEvent : public Event {
	public:
		MouseEnterEvent();
	};

	class MouseExitEvent : public Event {
	public:
		MouseExitEvent();
	};

	class MouseScrollEvent : public Event {
	public:
		explicit MouseScrollEvent(const Vector2& offset);
		const Vector2& offset() const;
	};

}