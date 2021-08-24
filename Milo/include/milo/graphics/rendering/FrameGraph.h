#pragma once

#include <milo/graphics/rendering/descriptions/ResourceDescriptions.h>
#include "milo/graphics/textures/Texture.h"
#include "milo/graphics/rendering/passes/RenderPass.h"

namespace milo {

	class FrameGraph {
		friend class WorldRenderer;
	private:
		ArrayList<RenderPass*> m_RenderPasses;
		ArrayList<RenderPass*> m_RenderPassExecutionList;
		FrameGraphResourcePool* m_ResourcePool = nullptr;
	private:
		FrameGraph();
		~FrameGraph();
		void init(FrameGraphResourcePool* resourcePool);
	public:
		void setup(Scene* scene);
		void compile(Scene* scene);
		void execute(Scene* scene);
	private:
		template<typename T>
		void push() {
			T* renderPass = get<T>();
			if(renderPass == nullptr) {
				renderPass = T::create();
				m_RenderPasses.push_back(renderPass);
			}
			m_RenderPassExecutionList.push_back(renderPass);
		}

		template<typename T>
		T* get() {
			const RenderPassId id = T::id();
			for(RenderPass* renderPass : m_RenderPasses) {
				if(renderPass->getId() == id) return dynamic_cast<T*>(renderPass);
			}
			return false;
		}

		template<typename T>
		bool exists() {
			return get<T>() != nullptr;
		}

		template<typename T>
		void pop() {
			if(exists<T>()) {
				T* renderPass = dynamic_cast<T*>(m_RenderPasses[T::id()]);
				m_RenderPasses.erase(T::id());
				DELETE_PTR(renderPass);
			}
		}

		void assignResources();
		void deleteUnusedRenderPasses();
		void freeUnusedResources();
	};

}