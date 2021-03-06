#include "milo/scenes/Scene.h"
#include "milo/scenes/SceneManager.h"
#include "milo/scenes/Entity.h"
#include "milo/graphics/Window.h"

namespace milo {

	Scene::Scene(const String& name) : m_Name(name) {
		Size size = Window::get()->size();
		m_Viewport = {0, 0, (float)size.width, (float)size.height};
	}

	Scene::Scene(String&& name) : m_Name(std::move(name)) {
		Size size = Window::get()->size();
		m_Viewport = {0, 0, (float)size.width, (float)size.height};
	}

	Scene::~Scene() = default;

	const String &Scene::name() const noexcept {
		return m_Name;
	}

	bool Scene::isActiveScene() const {
		return SceneManager::activeScene() == this;
	}

	Entity Scene::createEntity(const String& name) {
		const EntityId newId = m_Registry.create();
		Entity entity = {newId, this};
		entity.setName(name);
		return entity;
	}

	bool Scene::exists(EntityId entityId) const {
		return m_Registry.valid(entityId);
	}

	Entity Scene::find(EntityId id) const {
		if(!exists(id)) throw MILO_RUNTIME_EXCEPTION(str("Entity ") + str((uint64_t)id) + " does not exists");
		return Entity(id, const_cast<Scene*>(this));
	}

	void Scene::destroyEntity(EntityId entityId) noexcept {
		if(!exists(entityId)) return;
		Entity entity = {entityId, this};
		if(entity.hasParent()) entity.parent().removeChild(entityId);

		for(EntityId childId : entity.children()) {
			destroyEntity(childId);
		}

		m_Registry.destroy(entityId);
	}

	Entity Scene::cameraEntity() noexcept {
		return Entity(m_MainCameraEntity, this);
	}

	Camera* Scene::camera() noexcept {
		Entity entity = cameraEntity();
		if(entity.valid()) return &entity.getComponent<Camera>();
		return nullptr;
	}

	void Scene::setMainCamera(EntityId id) noexcept {
		m_MainCameraEntity = id;
	}

	ECSRegistry& Scene::registry() noexcept {
		return m_Registry;
	}

	SkyboxView* Scene::skyboxView() const {
		if(m_SkyEntity == NULL_ENTITY) return nullptr;
		Entity entity(m_SkyEntity, const_cast<Scene*>(this));
		return &entity.getComponent<SkyboxView>();
	}

	void Scene::setSkyEntity(EntityId id) {
		m_SkyEntity = id;
	}

	const Viewport& Scene::viewport() const noexcept {
		return m_Viewport;
	}

	Size Scene::viewportSize() const noexcept {
		return {(int32_t)fabs(m_Viewport.width), (int32_t)fabs(m_Viewport.height)};
	}

	void Scene::update() {

		Size size = Window::get()->size();
		m_Viewport = {0, 0, (float)size.width, (float)size.height};

		if(m_SkyEntity != NULL_ENTITY) {
			SkyboxView* skyboxView = this->skyboxView();
			if(skyboxView->type == SkyType::Dynamic) {
				PreethamSky* sky = (PreethamSky*)skyboxView->skybox;
				if(sky->dirty()) {
					sky->update();
				}
			}
		}

		if(getSimulationState() == SimulationState::Editor) return;

		auto nativeScripts = m_Registry.view<NativeScriptView>();
		for(EntityId entity : nativeScripts) {
			auto& nativeScriptView = m_Registry.get<NativeScriptView>(entity);
			nativeScriptView.createIfNotExists(entity);
			nativeScriptView.script->onUpdate(entity);
		}
	}

	void Scene::lateUpdate() {

		if(getSimulationState() == SimulationState::Editor) return;

		const ECSComponentView<NativeScriptView> nativeScripts = m_Registry.view<NativeScriptView>();
		for(EntityId entity : nativeScripts) {
			auto& nativeScriptView = m_Registry.get<NativeScriptView>(entity);
			nativeScriptView.createIfNotExists(entity);
			nativeScriptView.script->onLateUpdate(entity);
		}
	}

	bool Scene::focused() const {
		return m_Focused;
	}

	void Scene::setFocused(bool focused) {
		m_Focused = focused;
	}
}
