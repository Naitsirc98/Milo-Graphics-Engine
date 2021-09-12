#include "milo/core/MiloEngine.h"
#include "milo/events/EventSystem.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/Window.h"
#include "milo/graphics/Graphics.h"
#include "milo/input/Input.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/editor/MiloEditor.h"

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

		setSimulationState(SimulationState::Editor);

		application.onInit();
		init();
		Log::info("Starting Milo Application...");
		Window::get()->show();
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

			showDebugInfo(debugTime);
		}

		application.m_Running = false;
	}

	inline void MiloEngine::update(float& updateDelay, float& lastUpdate) {

		updateDelay += Time::s_RawDeltaTime;
		bool wasUpdated = false;

		while(updateDelay >= TARGET_UPDATE_DELAY) {

			const float now = Time::now();
			Time::s_DeltaTime = now - lastUpdate;
			lastUpdate = now;

			EventSystem::update();

			Input::update();

			MiloEditor::update();

			SceneManager::update();

			++Time::s_Ups;
			updateDelay -= TARGET_UPDATE_DELAY;
			wasUpdated = true;
		}

		if(wasUpdated) {
			SceneManager::lateUpdate();
			WorldRenderer::update();
		}
	}

	inline void MiloEngine::render() {

		GraphicsPresenter* graphicsPresenter = GraphicsPresenter::get();

		if(graphicsPresenter->begin()) {

			WorldRenderer::render();

			MiloEditor::render();

			graphicsPresenter->end();
		}

		++Time::s_Fps;
	}

	void MiloEngine::init() {
		MiloSubSystemManager::init();
		EventSystem::addEventCallback(EventType::WindowClose, [&](const Event& event) {Application::exit();});
	}

	void MiloEngine::shutdown() {
		MiloSubSystemManager::shutdown();
	}

	inline void MiloEngine::showDebugInfo(float& debugTime) {
		if(Time::now() - debugTime >= DEBUG_MIN_TIME) {
#ifdef _DEBUG
			String message = fmt::format("Ups: {}, Fps: {}, Dt:{}, Ft: {} ms, Mem: {}",Time::ups(), Time::fps(), Time::deltaTime(), Time::rawDeltaTime() * 1000.0f, MemoryTracker::totalAllocationSizeStr());
#else
			String message = fmt::format("Ups: {}, Fps: {}, Dt:{}, Ft: {} ms", Time::ups(), Time::fps(), Time::deltaTime(), Time::rawDeltaTime() * 1000.0f);
#endif
			Log::info(message);
			Window::get()->title("Milo Engine  " + std::move(message));

			Time::s_Ups = Time::s_Fps = 0;
			debugTime = Time::now();
		}
	}
}
