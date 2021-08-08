#include "milo/core/Application.h"

namespace milo {

	Application::Application(const AppConfiguration& config) : m_Configuration(config) {

	}

	Application::~Application() {
	}

	const AppConfiguration& Application::configuration() const {
		return m_Configuration;
	}

	void Application::exit() {
		s_Instance->m_Running = false;
	}
}