#include "milo/input/Input.h"

namespace milo {

	Input* Input::s_Instance;

	void Input::init() {
		s_Instance = new Input();
		s_Instance->setEventCallbacks();
	}

	void Input::shutdown() {
		delete s_Instance;
	}

	KeyAction Input::getKey(Key key) {
		return s_Instance->m_Keyboard.keys[static_cast<int32_t>(key)];
	}

	bool Input::isKeyReleased(Key key) {
		return getKey(key) == KeyAction::Release;
	}

	bool Input::isKeyPressed(Key key) {
		return getKey(key) == KeyAction::Press;
	}

	bool Input::isKeyRepeated(Key key) {
		return getKey(key) == KeyAction::Repeat;
	}

	bool Input::isKeyTyped(Key key) {
		return getKey(key) == KeyAction::Type;
	}

	bool Input::isKeyActive(Key key) {
		return isKeyPressed(key) || isKeyRepeated(key);
	}

	MouseButtonAction Input::getMouseButton(MouseButton button) {
		return s_Instance->m_Mouse.buttons[static_cast<int32_t>(button)];
	}

	bool Input::isMouseButtonReleased(MouseButton button) {
		return getMouseButton(button) == MouseButtonAction::Release;
	}

	bool Input::isMouseButtonPressed(MouseButton button) {
		return getMouseButton(button) == MouseButtonAction::Press;
	}

	bool Input::isMouseButtonRepeated(MouseButton button) {
		return getMouseButton(button) == MouseButtonAction::Repeat;
	}

	bool Input::isMouseButtonClicked(MouseButton button) {
		return getMouseButton(button) == MouseButtonAction::Click;
	}


	const Vector2& Input::getMousePosition() {
		return s_Instance->m_Mouse.position;
	}

	const Vector2& Input::getMouseScroll() {
		return s_Instance->m_Mouse.scrollOffset;
	}


	void Input::setEventCallbacks() {

		EventSystem::addEventCallback(EventType::KeyRelease, [&](const Event& e) {

			auto event = (const KeyReleaseEvent&) e;

			if(isKeyPressed(event.key)) {
				KeyTypeEvent keyTypeEvent = {};
				keyTypeEvent.key = event.key;
				keyTypeEvent.modifiers = event.modifiers;
				keyTypeEvent.scancode = event.scancode;
				EventSystem::publishEvent(keyTypeEvent);
			} else {
				m_Keyboard.keys[static_cast<int32_t>(event.key)] = KeyAction::Release;
			}

			m_Keyboard.activeModifiers = event.modifiers;
		});

		EventSystem::addEventCallback(EventType::KeyPress, [&](const Event& e) {
			auto event = (const KeyPressEvent&) e;
			m_Keyboard.keys[static_cast<int32_t>(event.key)] = KeyAction::Press;
			m_Keyboard.activeModifiers = event.modifiers;
		});

		EventSystem::addEventCallback(EventType::KeyRepeat, [&](const Event& e) {
			auto event = (const KeyRepeatEvent&) e;
			m_Keyboard.keys[static_cast<int32_t>(event.key)] = KeyAction::Repeat;
			m_Keyboard.activeModifiers = event.modifiers;
		});

		EventSystem::addEventCallback(EventType::KeyType, [&](const Event& e) {
			auto event = (const KeyTypeEvent&) e;
			m_Keyboard.keys[static_cast<int32_t>(event.key)] = KeyAction::Type;
			m_Keyboard.activeModifiers = event.modifiers;

			KeyReleaseEvent keyReleaseEvent = {};
			keyReleaseEvent.key = event.key;
			keyReleaseEvent.modifiers = event.modifiers;
			keyReleaseEvent.scancode = event.scancode;

			EventSystem::publishEvent(keyReleaseEvent);
		});


		EventSystem::addEventCallback(EventType::MouseButtonRelease, [&](const Event& e) {
			auto event = (const MouseButtonReleaseEvent&) e;
			if(isMouseButtonPressed(event.button)) {
				MouseButtonClickEvent mouseButtonClickEvent = {};
				mouseButtonClickEvent.button = event.button;
				mouseButtonClickEvent.modifiers = event.modifiers;
				EventSystem::publishEvent(mouseButtonClickEvent);
			} else {
				m_Mouse.buttons[static_cast<int32_t>(event.button)] = MouseButtonAction::Release;
			}

			m_Keyboard.activeModifiers = event.modifiers;
		});

		EventSystem::addEventCallback(EventType::MouseButtonPress, [&](const Event& e) {
			auto event = (const MouseButtonPressEvent&) e;
			m_Mouse.buttons[static_cast<int32_t>(event.button)] = MouseButtonAction::Press;
			m_Keyboard.activeModifiers = event.modifiers;
		});

		EventSystem::addEventCallback(EventType::MouseButtonRepeat, [&](const Event& e) {
			auto event = (const MouseButtonRepeatEvent&) e;
			m_Mouse.buttons[static_cast<int32_t>(event.button)] = MouseButtonAction::Repeat;
			m_Keyboard.activeModifiers = event.modifiers;
		});

		EventSystem::addEventCallback(EventType::MouseButtonClick, [&](const Event& e) {
			auto event = (const MouseButtonClickEvent&) e;
			m_Mouse.buttons[static_cast<int32_t>(event.button)] = MouseButtonAction::Click;
			m_Keyboard.activeModifiers = event.modifiers;

			MouseButtonReleaseEvent mouseButtonReleaseEvent = {};
			mouseButtonReleaseEvent.button = event.button;
			mouseButtonReleaseEvent.modifiers = event.modifiers;

			EventSystem::publishEvent(mouseButtonReleaseEvent);
		});

		EventSystem::addEventCallback(EventType::MouseMove, [&](const Event& e) {
			auto event = (const MouseMoveEvent&) e;
			m_Mouse.position = event.position;
		});

		EventSystem::addEventCallback(EventType::MouseScroll, [&](const Event& e) {
			auto event = (const MouseScrollEvent&) e;
			m_Mouse.scrollOffset = event.offset;
		});
	}

	void Input::update() {
		s_Instance->m_Mouse.scrollOffset = {0, 0};
	}


	bool hasKeyModifier(const KeyModifiersBitMask& keyModifiers, KeyModifier modifier) {
		KeyModifiersBitMask modifierBit = static_cast<KeyModifiersBitMask>(modifier);
		return (keyModifiers & modifierBit) == modifierBit;
	}

	void addKeyModifier(KeyModifiersBitMask& keyModifiers, KeyModifier modifier) {
		keyModifiers |= static_cast<KeyModifiersBitMask>(modifier);
	}
}