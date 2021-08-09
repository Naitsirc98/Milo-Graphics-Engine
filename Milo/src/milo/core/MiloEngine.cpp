#include "milo/core/MiloEngine.h"

namespace milo {

	static Mutex g_LaunchMutex;

	MiloEngine* MiloEngine::s_Instance = nullptr;

	MiloExitResult MiloEngine::launch(Application &application) {

		MiloExitResult exitResult;

		g_LaunchMutex.lock();
		{
			if(MiloEngine::s_Instance == nullptr) return {MILO_ENGINE_ALREADY_LAUNCHED, "MILO_ENGINE_ALREADY_LAUNCHED"};

			try {
				MiloEngine::s_Instance = new MiloEngine(application);
				MiloEngine::s_Instance->start();

				exitResult.exitCode = MILO_SUCCESS;
				exitResult.message = "";

			} catch(ANY_EXCEPTION) {
				try {
					std::rethrow_exception(std::current_exception());
				} catch(const Exception& e) {
					LOG_ERROR(String("A fatal exception forced the application to exit: ").append(e.what()));
					exitResult.exitCode = MILO_FAILURE;
					exitResult.message = e.what();
					INVOKE_SAFELY(MiloEngine::s_Instance->shutdown());
				}
			}

			DELETE_PTR(MiloEngine::s_Instance);

		}
		g_LaunchMutex.unlock();

		return exitResult;
	}

	MiloEngine::MiloEngine(Application &application) : m_Application(application) {

	}

	void MiloEngine::start() {
		m_Application.onInit();
		init();
		m_Application.onStart();
		run();
	}

	void MiloEngine::run() {

	}

	void MiloEngine::update() {

	}

	void MiloEngine::render() {

	}

	void MiloEngine::renderUI() {

	}

	void MiloEngine::init() {
		MiloSubSystemManager::init();
	}

	void MiloEngine::shutdown() {
		MiloSubSystemManager::shutdown();
	}
}
