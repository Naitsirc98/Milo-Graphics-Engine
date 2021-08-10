#pragma once

#include "Event.h"

namespace milo {

	struct RawEvent {
		char data[32];
	};

	class EventQueue {
	private:
		RawEvent* m_Events = nullptr;
		size_t m_Capacity = 0;
		size_t m_Size = 0;
	public:
		explicit EventQueue(size_t capacity);
		~EventQueue();
		void clear();
		[[nodiscard]] size_t size() const;
		[[nodiscard]] size_t capacity() const;

		template<typename T>
		void push(const T& event) {
#ifdef _DEBUG
			if(sizeof(T) > sizeof(RawEvent))throw MILO_RUNTIME_EXCEPTION(fmt::format("EventQueue: sizeof({})={} > MAX_EVENT_SIZE({})", typeid(T).name(), sizeof(T), MAX_EVENT_SIZE));
			if(m_Size >= m_Capacity) throw MILO_RUNTIME_EXCEPTION("EventQueue: out of memory");
#endif
			memcpy(&m_Events[m_Size++], &event, sizeof(T));
		}

		Event& operator[](size_t index);
	};
}