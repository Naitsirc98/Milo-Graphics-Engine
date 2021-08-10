#pragma once
#include "milo/common/Common.h"
#include "milo/events/Event.h"
#include "EventQueue.h"

namespace milo {

	class EventSystem {
		friend class MiloEngine;
		friend class MiloSubSystemManager;
	private:
		static ArrayList<EventCallback> s_EventCallbacks[static_cast<size_t>(EventType::MaxEnumValue)];
		static EventQueue s_EventQueue1;
		static EventQueue s_EventQueue2;
		static EventQueue* s_FrontEventQueue;
		static EventQueue* s_BackEventQueue;

	public:
		static void addEventCallback(EventType type, const EventCallback& callback);

		template<typename E>
		static void publishEvent(const E& event) {
			s_FrontEventQueue->push<E>(event);
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
