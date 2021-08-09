#include "milo/events/EventAllocator.h"

namespace milo {

	EventAllocator::~EventAllocator() {
		DELETE_ARRAY(m_Memory);
	}

	size_t EventAllocator::count() const {
		return m_Pointer / EVENT_MAX_SIZE;
	}

	size_t EventAllocator::capacity() const {
		return m_Capacity;
	}

	void* EventAllocator::allocate(size_t allocSize) {
#ifdef _DEBUG
		if(allocSize > EVENT_MAX_SIZE) throw MILO_RUNTIME_EXCEPTION("EventAllocator invalid allocSize: size > MAX_EVENT_SIZE");
		if(m_Pointer + allocSize > m_Capacity)
			throw MILO_RUNTIME_EXCEPTION("EventAllocator out of memory");
#endif
		void* allocatedMemory = &m_Memory[m_Pointer];
		memset(allocatedMemory, 0, allocSize);
		m_Pointer += allocSize;
		return allocatedMemory;
	}

	void EventAllocator::reset() {
		m_Pointer = 0;
	}

	Event& EventAllocator::operator[](size_t index) {
#ifdef _DEBUG
		if(index >= m_Pointer) throw MILO_RUNTIME_EXCEPTION("EventAllocator index out of bounds");
#endif
		return *reinterpret_cast<Event*>(&m_Memory[index * EVENT_MAX_SIZE]);
	}

	void EventAllocator::init(size_t capacity) {
		m_Memory = new int8[capacity]{0};
		m_Capacity = capacity;
	}
}