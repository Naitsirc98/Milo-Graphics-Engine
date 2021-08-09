#include "milo/core/MiloEngine.h"
#include <milo/events/EventSystem.h>

namespace milo {

	const float TARGET_UPS = 60.0f;
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

		float lastFrame = Time::now();
		float debugTime = Time::now();

		while(application.running()) {

			const float now = Time::now();
			Time::s_DeltaTime = now - lastFrame;
			lastFrame = now;

			update();
			render();

			++Time::s_Frame;

			if(Time::now() - debugTime >= DEBUG_MIN_TIME) {
				Log::info("Frame: {}, Ups: {}, Fps: {}, DeltaTime: {}, FrameTime: {} ms",
						  Time::frame(), Time::ups(), Time::fps(), Time::deltaTime(), Time::deltaTime() * 1000.0f);
				Time::s_Ups = Time::s_Fps = 0;
				debugTime = Time::now();
			}
		}

		application.m_Running = false;
	}

	void MiloEngine::update() {

		Time::s_UpdateDelay += Time::s_DeltaTime;
		bool wasUpdated = false;

		while(Time::s_UpdateDelay >= TARGET_UPDATE_DELAY) {

			EventSystem::update();
			// TODO...
			// debug purposes
			if(Random::nextInt() % 5 == 0) {
				int32 count = Random::nextInt(1500, 8192);
				Log::warn("Publishing {} events...", count);
				for(size_t i = 0; i < count; ++i) {
					EventSystem::publishEvent<KeyPressEvent>(Key::Key_X, 123, 0x2);
				}
			}

			++Time::s_Ups;
			Time::s_UpdateDelay -= TARGET_UPDATE_DELAY;
			wasUpdated = true;
		}

		if(wasUpdated) {
			// TODO...
		}
	}

	void MiloEngine::render() {
		// TODO: render scene
		// debug purposes
		sleep_for(std::chrono::milliseconds(Random::nextInt(1, 10)));
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
