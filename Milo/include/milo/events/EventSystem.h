#pragma once
#include "milo/common/Common.h"
#include "milo/events/Event.h"
#include "milo/events/KeyboardEvents.h"
#include "milo/events/MouseEvents.h"
#include "milo/events/WindowEvents.h"
#include "milo/events/ApplicationEvents.h"
#include "EventAllocator.h"

namespace milo {

	class EventSystem {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		static ArrayList<EventCallback> s_EventCallbacks[static_cast<size_t>(EventType::MaxEnumValue)];
		static EventAllocator s_Allocator1;
		static EventAllocator s_Allocator2;
		static EventAllocator* s_FrontEventQueue;
		static EventAllocator* s_BackEventQueue;

	public:
		static void addEventCallback(EventType type, const EventCallback& callback);

		template<typename E, typename... Args>
		static E* publishEvent(Args&&... args) {
			E* event = s_FrontEventQueue->create<E>(std::forward<Args>(args)...);
			return event;
		}

		EventSystem() = delete;
	private:
		static void update();
		static void pollEvents();
		static void waitEvents(float timeout = 0.0f);

		static void init();
		static void shutdown();

		static void swapEventQueues();
	};
}
