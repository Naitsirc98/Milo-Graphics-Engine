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
		int32_t exitCode;
		String message;

		 inline bool success() const noexcept {return exitCode == MILO_SUCCESS;}
	};

	class MiloEngine {
	private:
		static AtomicBool s_AlreadyLaunched;
	public:
		static MiloExitResult launch(Application& application);
	private:
		Application& m_Application;
	public:
		MiloEngine() = delete;
	private:
		static void run();
		static void update(float& updateDelay, float& lastUpdate);
		static void render();
		static void renderUI();
		static void init();
		static void shutdown();
		static void showDebugInfo(float& debugTime);
	};
}