#include "milo/events/Event.h"

namespace milo {

	Event::Event(EventType type) {
		*reinterpret_cast<EventType*>(m_Data) = type;
	}

	EventType Event::type() const {
		return *reinterpret_cast<EventType*>(m_Data);
	}
}