#pragma once
#include "Event.h"
#include "milo/input/Keyboard.h"

namespace milo {

	class KeyEvent : public Event
	{
	private:
		Key m_Key;
		int32 m_Scancode;
		KeyModifiersBitMask m_KeyModifiers;
	public:
		explicit KeyEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers)
			: m_Key(key), m_Scancode(scancode), m_KeyModifiers(keyModifiers) {}
		virtual ~KeyEvent() = default;
		Key key() const {return m_Key;}
		int32 scancode() const {return m_Scancode;}
		KeyModifiersBitMask keyModifiers() const {return m_KeyModifiers;}
		virtual KeyAction action() const = 0;
	};

	class KeyReleaseEvent : public KeyEvent
	{
	public:
		explicit KeyReleaseEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers)
			: KeyEvent(key, scancode, keyModifiers) {}
		~KeyReleaseEvent() override = default;
		KeyAction action() const override {return KeyAction::Release;}
		EventType type() const override {return EventType::KeyRelease;}
	};

	class KeyPressEvent : public KeyEvent
	{
	public:
		explicit KeyPressEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers)
			: KeyEvent(key, scancode, keyModifiers) {}
		~KeyPressEvent() override = default;
		KeyAction action() const override {return KeyAction::Press;}
		EventType type() const override {return EventType::KeyPress;}
	};

	class KeyRepeatEvent : public KeyEvent
	{
	public:
		explicit KeyRepeatEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers)
			: KeyEvent(key, scancode, keyModifiers) {}
		~KeyRepeatEvent() override = default;
		KeyAction action() const override {return KeyAction::Repeat;}
		EventType type() const override {return EventType::KeyRepeat;}
	};

	class KeyTypeEvent : public KeyEvent
	{
	public:
		explicit KeyTypeEvent(Key key, int32 scancode, KeyModifiersBitMask keyModifiers)
			: KeyEvent(key, scancode, keyModifiers) {}
		~KeyTypeEvent() override = default;
		KeyAction action() const override {return KeyAction::Type;}
		EventType type() const override {return EventType::KeyType;}
	};

}
