#include "milo/core/MiloEngine.h"
#include "milo/events/EventSystem.h"
#include "milo/scenes/SceneManager.h"

namespace milo {

	const size_t TARGET_UPS = 60.0f;
	const float TARGET_UPDATE_DELAY = 1.0f / TARGET_UPS;
	const float DEBUG_MIN_TIME = 1.0f;

	static Mutex g_LaunchMutex;

	AtomicBool MiloEngine::s_AlreadyLaunched = false;

	MiloExitResult MiloEngine::launch(Application &application) {
		if(s_AlreadyLaunched) return {MILO_ENGINE_ALREADY_LAUNCHED, "MILO_ENGINE_ALREADY_LAUNCHED"};

		MiloExitResult exitResult;

		g_LaunchMutex.lock();
		{
			Application::s_Instance = &application;
			try {
				s_AlreadyLaunched = true;

				MiloEngine::run();

				exitResult.exitCode = MILO_SUCCESS;
				exitResult.message = "";

			} catch(ANY_EXCEPTION) {
				try {
					std::rethrow_exception(std::current_exception());
				} catch(const Exception& e) {
					LOG_ERROR(String("A fatal exception forced the application to exit: ").append(e.what()));
					exitResult.exitCode = MILO_FAILURE;
					exitResult.message = e.what();
				}
			}

			INVOKE_SAFELY(MiloEngine::shutdown());

			Application::s_Instance = nullptr;
		}
		g_LaunchMutex.unlock();

		s_AlreadyLaunched = false;

		return exitResult;
	}

	void MiloEngine::run() {
		Application& application = Application::get();
		application.m_Running = true;

		application.onInit();
		init();
		application.onStart();

		float updateDelay = 0;

		float lastFrame = Time::now();
		float lastUpdate = Time::now();
		float debugTime = Time::now();

		while(application.running()) {

			const float now = Time::now();
			Time::s_RawDeltaTime = now - lastFrame;
			lastFrame = now;

			update(updateDelay, lastUpdate);

			render();

			++Time::s_Frame;

			if(Time::now() - debugTime >= DEBUG_MIN_TIME) {
#ifdef _DEBUG
				Log::info("Ups: {}, Fps: {}, Dt:{}, Ft: {} ms, Mem: {}",
						  Time::ups(), Time::fps(), Time::deltaTime(), Time::rawDeltaTime() * 1000.0f,
						  MemoryTracker::totalAllocationSizeStr());
#else
				Log::info("Ups: {}, Fps: {}, Dt:{}, Ft: {} ms", Time::ups(), Time::fps(), Time::deltaTime(), Time::rawDeltaTime() * 1000.0f);
#endif
				Time::s_Ups = Time::s_Fps = 0;
				debugTime = Time::now();
			}
		}

		application.m_Running = false;
	}

	void MiloEngine::update(float& updateDelay, float& lastUpdate) {

		updateDelay += Time::s_RawDeltaTime;
		bool wasUpdated = false;

		while(updateDelay >= TARGET_UPDATE_DELAY) {

			const float now = Time::now();
			Time::s_DeltaTime = now - lastUpdate;
			lastUpdate = now;

			EventSystem::update();

			SceneManager::update();

			++Time::s_Ups;
			updateDelay -= TARGET_UPDATE_DELAY;
			wasUpdated = true;
		}

		if(wasUpdated) {

			SceneManager::lateUpdate();

			//if(Time::s_Ups >= TARGET_UPS) updateDelay = 0;
		}
	}

	void MiloEngine::render() {
		// TODO: quit this
		sleep_for(std::chrono::nanoseconds (100));

		SceneManager::render();

		renderUI();

		++Time::s_Fps;
	}

	void MiloEngine::renderUI() {
		// TODO
	}

	void MiloEngine::init() {
		MiloSubSystemManager::init();
	}

	void MiloEngine::shutdown() {
		MiloSubSystemManager::shutdown();
	}
}
