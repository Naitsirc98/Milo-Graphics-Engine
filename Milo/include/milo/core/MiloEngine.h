#pragma once

#include "milo/common/Common.h"
#include "MiloSubSystemManager.h"
#include "Application.h"
#include "milo/logging/Log.h"

#define MILO_SUCCESS 0
#define MILO_FAILURE 1
#define MILO_ENGINE_ALREADY_LAUNCHED 2

namespace milo {

	struct MiloExitResult {
		int32 exitCode;
		String message;

		inline bool success() noexcept {return exitCode == MILO_SUCCESS;}
	};

	class MiloEngine {
	private:
		static MiloEngine* s_Instance;
	public:
		static MiloExitResult launch(Application& application);
	private:
		Application& m_Application;
	private:
		MiloEngine(Application& application);
		~MiloEngine() = default;
	private:
		void start();
		void run();
		void update();
		void render();
		void renderUI();
		void init();
		void shutdown();
	};
}