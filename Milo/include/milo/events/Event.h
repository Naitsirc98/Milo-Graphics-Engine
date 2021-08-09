#pragma once
#include "milo/common/Common.h"

namespace milo {

	const size_t EVENT_MAX_SIZE = 16;

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
		UserEvent,
		_MaxEnum
	};

	class Event {
	public:
		Event() = default;
		virtual ~Event() = default;
		[[nodiscard]] virtual EventType type() const = 0;
	};

	using EventCallback = Function<void, Event&>;
}
