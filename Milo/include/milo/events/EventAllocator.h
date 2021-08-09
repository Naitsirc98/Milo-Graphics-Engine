#pragma once

#include "Event.h"

namespace milo {

	class EventAllocator {
	public:
		inline static const size_t HEADER_SIZE = sizeof(uint8);
	private:
		int8* m_Memory = nullptr;
		size_t m_Capacity = 0;
		size_t m_Pointer = 0;

	public:
		EventAllocator() = default;
		~EventAllocator();

		void init(size_t capacity);

		[[nodiscard]] size_t count() const;
		[[nodiscard]] size_t capacity() const;

		template<typename E, typename... Args>
		E* create(Args&&... args) {
			return new (allocate(sizeof(E))) E(std::forward<Args>(args)...);
		}

		void* allocate(size_t allocSize);
		void reset();

		Event& operator[](size_t index);
	};
}