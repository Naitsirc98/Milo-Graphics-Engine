#include "milo/events/KeyboardEvents.h"

namespace milo {

	struct KeyEventProperties {
		EventType type;
		KeyAction action;
		Key key;
		int32 scancode;
		KeyModifiersBitMask keyModifiers;

		inline static KeyEventProperties* of(void* data) {reinterpret_cast<KeyEventProperties*>(data);}
	};

	KeyEvent::KeyEvent(EventType type, KeyAction action, Key key, int32 scancode, KeyModifiersBitMask keyModifiers)
		: Event(type) {
		KeyEventProperties* properties = KeyEventProperties::of(m_Data);
		properties->type = type;
		properties->action = action;
		properties->key = key;
		properties->scancode = scancode;
		properties->keyModifiers = keyModifiers;
	}

	Key KeyEvent::key() const {
		return KeyEventProperties::of(m_Data)->key;
	}

	int32 KeyEvent::scancode() const {
		return KeyEventProperties::of(m_Data)->scancode;
	}

	KeyModifiersBitMask KeyEvent::keyModifiers() const {
		return KeyEventProperties::of(m_Data)->keyModifiers;
	}

	KeyReleaseEvent::KeyReleaseEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers)
		: KeyEvent(EventType::KeyRelease, KeyAction::Release, key, scancode, keyModifiers) {

	}

	KeyPressEvent::KeyPressEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers)
		: KeyEvent(EventType::KeyPress, KeyAction::Press, key, scancode, keyModifiers) {

	}

	KeyRepeatEvent::KeyRepeatEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers)
		: KeyEvent(EventType::KeyRepeat, KeyAction::Repeat, key, scancode, keyModifiers) {

	}

	KeyTypeEvent::KeyTypeEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers)
		: KeyEvent(EventType::KeyType, KeyAction::Type, key, scancode, keyModifiers) {
	}
}