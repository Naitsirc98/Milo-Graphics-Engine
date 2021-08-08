#pragma once

#include "milo/common/Common.h"
#include "milo/graphics/GraphicsAPI.h"

namespace milo {

	struct AppConfiguration {

		String applicationName = "Milo Application";
		// TODO
	};

	class Application {
		friend class MiloEngine;
	private:
		const AppConfiguration& m_Configuration;
		volatile bool m_Running = false;
	public:
		explicit Application(const AppConfiguration& config);
		virtual ~Application() = default;
		[[nodiscard]] const AppConfiguration& configuration() const;
		virtual void onInit() {}
		virtual void onStart() {}
		virtual void onUpdate() {}
		virtual void onLateUpdate() {}
		virtual void onRender() {}
		virtual void onShutdown() {}
		virtual void onExit() {}
	public:
		static void exit();
	private:
		static Application* s_Instance;
	};
}