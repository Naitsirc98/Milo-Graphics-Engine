#pragma once
#include "milo/events/Event.h"

namespace milo
{

	class ApplicationExitEvent : public Event
	{
	public:
		ApplicationExitEvent() = default;
		~ApplicationExitEvent() = default;
	};

}