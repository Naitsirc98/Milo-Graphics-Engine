#include "milo/events/EventSystem.h"
#include <GLFW/glfw3.h>

#define MAX_EVENT_COUNT (16 * 1024)

namespace milo {

	ArrayList<EventCallback> EventSystem::s_EventCallbacks[static_cast<size_t>(EventType::MaxEnumValue)];
	EventQueue EventSystem::s_EventQueue1;
	EventQueue EventSystem::s_EventQueue2;
	EventQueue* EventSystem::s_FrontEventQueue;
	EventQueue* EventSystem::s_BackEventQueue;

	void EventSystem::addEventCallback(EventType type, const EventCallback& callback) {
		ArrayList<EventCallback>& callbacks = s_EventCallbacks[static_cast<size_t>(type)];
		callbacks.push_back(callback);
	}

	void EventSystem::update() {

		pollEvents();

		EventQueue& eventQueue = *s_FrontEventQueue;
		swapEventQueues();

		const size_t count = eventQueue.size();
		for(size_t i = 0;i < count;++i) {
			const Event& event = eventQueue[i];
			const ArrayList<EventCallback>& callbacks = s_EventCallbacks[static_cast<size_t>(event.type)];
			for(const EventCallback& callback : callbacks) {
				callback(event);
			}
		}
		eventQueue.clear();
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

		s_EventQueue1.reserve(MAX_EVENT_COUNT);
		s_EventQueue2.reserve(MAX_EVENT_COUNT);

		s_FrontEventQueue = &s_EventQueue1;
		s_BackEventQueue = &s_EventQueue2;
	}

	void EventSystem::shutdown() {
		for(size_t i = 0; i < s_EventQueue1.size(); ++i) {
			Event& event = s_EventQueue1[i];
			event.~Event();
		}

		for(size_t i = 0; i < s_EventQueue2.size(); ++i) {
			Event& event = s_EventQueue2[i];
			event.~Event();
		}

		s_FrontEventQueue = nullptr;
		s_BackEventQueue = nullptr;
	}

	void EventSystem::swapEventQueues() {
		EventQueue* tmp = s_FrontEventQueue;
		s_FrontEventQueue = s_BackEventQueue;
		s_BackEventQueue = tmp;
	}
}

