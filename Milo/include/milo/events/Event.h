#pragma once
#include "milo/common/Common.h"

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

	class Event {
		friend class EventAllocator;
	protected:
		int8 const m_Data[MAX_EVENT_SIZE]{0};
	protected:
		explicit Event(EventType type);
	public:
		virtual ~Event() = default;
		[[nodiscard]] EventType type() const;
	};

	using EventCallback = Function<void, Event&>;
}
