#include "milo/core/MiloEngine.h"
#include <milo/events/EventSystem.h>

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

		EventSystem::addEventCallback(EventType::KeyPress, [&](const Event& e) {
			const auto& event = static_cast<const KeyPressEvent&>(e);
		});

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
				Log::info("Frame: {}, Ups: {}, Fps: {}, DeltaTime: {}, FrameTime: {} ms",
						  Time::frame(), Time::ups(), Time::fps(), Time::deltaTime(), Time::rawDeltaTime() * 1000.0f);
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
			// TODO...
			// debug purposes
			if(Random::nextInt() % 5 == 0) {
				int32 count = Random::nextInt(1500, 8192);
				//Log::warn("Publishing {} events...", count);
				for(size_t i = 0; i < count; ++i) {
					if(i % 2 == 0) {
						KeyPressEvent event = {};
						event.key = Key::Key_X;
						event.scancode = Random::nextInt(0, 1000);
						event.modifiers = Random::nextInt(0, 80);
						EventSystem::publishEvent(event);
					} else if(i % 3 == 0) {
						MouseMoveEvent event = {};
						event.position = {i, i};
						EventSystem::publishEvent(event);
					} else if(i % 5 == 0) {
						MouseButtonRepeatEvent event = {};
						event.modifiers = i;
						event.button = MouseButton::Mouse_Button_1;
						EventSystem::publishEvent(event);
					} else {
						WindowResizeEvent event = {};
						event.size = {i, i};
						EventSystem::publishEvent(event);
					}
				}
			}

			++Time::s_Ups;
			updateDelay -= TARGET_UPDATE_DELAY;
			wasUpdated = true;
		}

		if(wasUpdated) {
			// TODO...
			if(Time::s_Ups >= TARGET_UPS) updateDelay = 0;
		}
	}

	void MiloEngine::render() {
		// TODO: render scene
		// debug purposes
		sleep_for(std::chrono::nanoseconds (1000));
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
