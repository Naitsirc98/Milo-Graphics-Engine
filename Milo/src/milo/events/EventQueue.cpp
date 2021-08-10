#include "milo/events/EventQueue.h"
#include "milo/logging/Log.h"

namespace milo {

	EventQueue::EventQueue(size_t capacity) : m_Capacity(capacity) {
		m_Events = new RawEvent[capacity]{0};
	}

	EventQueue::~EventQueue() {
		DELETE_ARRAY(m_Events);
		m_Capacity = m_Size = 0;
	}

	size_t EventQueue::size() const {
		return m_Size / MAX_EVENT_SIZE;
	}

	size_t EventQueue::capacity() const {
		return m_Capacity;
	}

	void EventQueue::clear() {
#ifdef _DEBUG
		memset(m_Events, 0, m_Size * sizeof(RawEvent));
#endif
		m_Size = 0;
	}

	Event& EventQueue::operator[](size_t index) {
		return *reinterpret_cast<Event*>(&m_Events[index]);
	}
}