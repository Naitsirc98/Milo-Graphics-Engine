#pragma once

#include <milo/graphics/rendering/descriptions/ResourceDescriptions.h>
#include "milo/graphics/textures/Texture.h"
#include "milo/graphics/rendering/passes/RenderPass.h"
#include "milo/logging/Log.h"
#include "milo/time/Time.h"

namespace milo {

	class FrameGraph {
		friend class WorldRenderer;
	protected:
		ArrayList<RenderPass*> m_RenderPasses;
		ArrayList<RenderPass*> m_RenderPassExecutionList;
		HashMap<RenderPassId, uint32_t> m_RenderPassUnusedCount;
		FrameGraphResourcePool* m_ResourcePool = nullptr;
	protected:
		FrameGraph();
		~FrameGraph();
		void init(FrameGraphResourcePool* resourcePool);
	public:
		virtual void setup(Scene* scene);
		virtual void compile(Scene* scene);
		virtual void execute(Scene* scene);
	protected:

#define RENDER_PASS_NAME(T) #T

		template<typename T>
		void push() {
			T* renderPass = get<T>();
			if(renderPass == nullptr) {
#ifdef _DEBUG
				auto name = typeid(T).name();
				Log::debug("Creating {}...", name);
				float ms = Time::millis();
#endif
				renderPass = T::create();
#ifdef _DEBUG
				ms = Time::millis() - ms;
				Log::debug("{} created after {} ms", name, ms);
#endif
				m_RenderPasses.push_back(renderPass);
			}
			m_RenderPassExecutionList.push_back(renderPass);
		}

		template<typename T>
		T* get() {
			const RenderPassId id = T::id();
			for(RenderPass* renderPass : m_RenderPasses) {
				RenderPassId otherId = renderPass->getId();
				if(otherId == id) {
					return dynamic_cast<T*>(renderPass);
				}
			}
			return nullptr;
		}

		template<typename T>
		bool exists() {
			return get<T>() != nullptr;
		}

		void deleteUnusedRenderPasses();
	};

}