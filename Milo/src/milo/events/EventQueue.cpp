#include "milo/events/EventQueue.h"
#include "milo/logging/Log.h"

namespace milo {

	EventQueue::EventQueue() = default;

	EventQueue::~EventQueue() {
		DELETE_ARRAY(m_Events);
		m_Capacity = m_Size = 0;
	}

	size_t EventQueue::size() const {
		return m_Size;
	}

	size_t EventQueue::capacity() const {
		return m_Capacity;
	}

	void EventQueue::reserve(size_t capacity) {
		if(m_Capacity == capacity) return;
		if(capacity < m_Capacity && m_Events != nullptr) DELETE_ARRAY(m_Events);
		m_Events = NEW RawEvent[capacity]{0};
		m_Capacity = capacity;
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