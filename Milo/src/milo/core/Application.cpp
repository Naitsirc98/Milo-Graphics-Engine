#include "milo/core/Application.h"

namespace milo {

	Application* Application::s_Instance = nullptr;

	Application::Application(const AppConfiguration& config) : m_Configuration(config) {

	}

	const AppConfiguration& Application::configuration() const {
		return m_Configuration;
	}

	Application &Application::get() {
		return *s_Instance;
	}

	void Application::exit() {
		s_Instance->m_Running = false;
	}
}