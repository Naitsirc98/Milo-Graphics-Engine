#include <milo/input/Keyboard.h>
#include "milo/events/MouseEvents.h"

namespace milo {

	struct MouseButtonEventProperties {
		EventType type;
		MouseButton button;
		KeyModifiersBitMask modifiers;

		inline static MouseButtonEventProperties* of(void* data) {reinterpret_cast<MouseButtonEventProperties*>(data);}
	};

	MouseButtonEvent::MouseButtonEvent(EventType type, MouseButton button, KeyModifiersBitMask modifiers)
		: Event(type) {

		MouseButtonEventProperties* properties = MouseButtonEventProperties::of(m_Data);
		properties->button = button;
		properties->modifiers = modifiers;
	}

	MouseButton MouseButtonEvent::button() const {
		return MouseButtonEventProperties::of(m_Data)->button;
	}

	KeyModifiersBitMask MouseButtonEvent::modifiers() const {
		return MouseButtonEventProperties::of(m_Data)->modifiers;
	}

	MouseButtonReleaseEvent::MouseButtonReleaseEvent(MouseButton button, KeyModifiersBitMask modifiers)
		: MouseButtonEvent(EventType::MouseButtonRelease, button, modifiers) {

	}

	MouseButtonPressEvent::MouseButtonPressEvent(MouseButton button, KeyModifiersBitMask modifiers)
	: MouseButtonEvent(EventType::MouseButtonPress, button, modifiers) {

	}

	MouseButtonClickEvent::MouseButtonClickEvent(MouseButton button, KeyModifiersBitMask modifiers)
	: MouseButtonEvent(EventType::MouseButtonClick, button, modifiers) {

	}

	MouseButtonRepeatEvent::MouseButtonRepeatEvent(MouseButton button, KeyModifiersBitMask modifiers)
		: MouseButtonEvent(EventType::MouseButtonRepeat, button, modifiers) {

	}

	struct MouseMoveEventProperties {
		EventType type;
		Vector2 position;

		inline static MouseMoveEventProperties* of(void* data) {reinterpret_cast<MouseMoveEventProperties*>(data);}
	};

	MouseMoveEvent::MouseMoveEvent(const Vector2& position)
		: Event(EventType::MouseMove) {
		MouseMoveEventProperties* properties = MouseMoveEventProperties::of(m_Data);
		properties->position = position;
	}

	const Vector2& MouseMoveEvent::position() const {
		return MouseMoveEventProperties::of(m_Data)->position;
	}

	MouseEnterEvent::MouseEnterEvent() : Event(EventType::MouseEnter) {}

	MouseExitEvent::MouseExitEvent() : Event(EventType::MouseExit) {}


	struct MouseScrollEventProperties {
		EventType type;
		Vector2 offset;

		inline static MouseScrollEventProperties* of(void* data) {reinterpret_cast<MouseScrollEventProperties*>(data);}
	};

	MouseScrollEvent::MouseScrollEvent(const Vector2& offset)
		: Event(EventType::MouseScroll) {

		MouseScrollEventProperties* properties = MouseScrollEventProperties::of(m_Data);
		properties->offset = offset;
	}

	const Vector2& MouseScrollEvent::offset() const {
		return MouseScrollEventProperties::of(m_Data)->offset;
	}
}