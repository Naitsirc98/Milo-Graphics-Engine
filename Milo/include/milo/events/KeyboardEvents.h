#pragma once
#include "Event.h"
#include "milo/input/Keyboard.h"

namespace milo {

	class KeyEvent : public Event {
	protected:
		KeyEvent(EventType type, KeyAction action, Key key, int32 scancode, KeyModifiersBitMask keyModifiers);
	public:
		Key key() const;
		int32 scancode() const;
		KeyModifiersBitMask keyModifiers() const;
	};

	class KeyReleaseEvent : public KeyEvent {
	public:
		KeyReleaseEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers);
	};

	class KeyPressEvent : public KeyEvent {
	public:
		KeyPressEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers);
	};

	class KeyRepeatEvent : public KeyEvent {
	public:
		KeyRepeatEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers);
	};

	class KeyTypeEvent : public KeyEvent {
	public:
		KeyTypeEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers);
	};

}
