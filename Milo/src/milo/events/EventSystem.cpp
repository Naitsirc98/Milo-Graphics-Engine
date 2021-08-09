#include "milo/events/EventSystem.h"
#include <GLFW/glfw3.h>

namespace milo {

	ArrayList<EventCallback> EventSystem::s_EventCallbacks[static_cast<size_t>(EventType::MaxEnumValue)];
	EventAllocator EventSystem::s_Allocator1;
	EventAllocator EventSystem::s_Allocator2;
	EventAllocator* EventSystem::s_FrontEventQueue;
	EventAllocator* EventSystem::s_BackEventQueue;

	void EventSystem::addEventCallback(EventType type, const EventCallback& callback) {
		ArrayList<EventCallback>& callbacks = s_EventCallbacks[static_cast<size_t>(type)];
		callbacks.push_back(callback);
	}

	void EventSystem::update() {

		pollEvents();

		EventAllocator& eventQueue = *s_FrontEventQueue;
		swapEventQueues();

		const size_t count = eventQueue.count();
		for(size_t i = 0;i < count;++i) {
			Event& event = eventQueue[i];
			const ArrayList<EventCallback>& callbacks = s_EventCallbacks[static_cast<size_t>(event.type())];
			for(const EventCallback& callback : callbacks) {
				callback(event);
			}
			event.~Event();
		}
		eventQueue.reset();
	}

	void EventSystem::pollEvents() {
		glfwPollEvents();
	}

	void EventSystem::waitEvents(float timeout) {
		glfwWaitEventsTimeout(timeout);
	}

	void EventSystem::init() {
		for(ArrayList<EventCallback>& callbackList : s_EventCallbacks) {
			callbackList.reserve(4);
		}

		const size_t maxEventCount = 8 * 1024;

		s_Allocator1.init(maxEventCount * EVENT_MAX_SIZE);
		s_Allocator2.init(maxEventCount * EVENT_MAX_SIZE);

		s_FrontEventQueue = &s_Allocator1;
		s_BackEventQueue = &s_Allocator2;
	}

	void EventSystem::shutdown() {
		for(size_t i = 0;i < s_Allocator1.count(); ++i) {
			Event& event = s_Allocator1[i];
			event.~Event();
		}

		for(size_t i = 0;i < s_Allocator2.count(); ++i) {
			Event& event = s_Allocator2[i];
			event.~Event();
		}

		s_FrontEventQueue = nullptr;
		s_BackEventQueue = nullptr;
	}

	void EventSystem::swapEventQueues() {
		EventAllocator* tmp = s_FrontEventQueue;
		s_FrontEventQueue = s_BackEventQueue;
		s_BackEventQueue = tmp;
	}
}

