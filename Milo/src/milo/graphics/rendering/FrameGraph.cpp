#include "milo/graphics/rendering/FrameGraph.h"
#include "milo/scenes/SceneManager.h"
#include "milo/graphics/rendering/passes/AllRenderPasses.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/rendering/VulkanFrameGraphResourcePool.h"

namespace milo {

	FrameGraph::FrameGraph() {
		m_RenderPasses.reserve(64);
		m_RenderPassExecutionList.reserve(m_RenderPasses.capacity());
	}

	FrameGraph::~FrameGraph() {
		for(RenderPass* renderPass : m_RenderPasses) {
			DELETE_PTR(renderPass);
		}
	}

	void FrameGraph::init(FrameGraphResourcePool* resourcePool) {
		m_ResourcePool = resourcePool;
	}

	void FrameGraph::setup(Scene* scene) {
		// TODO: add render passes by analyzing the scene composition at this frame
		// For now, lets simply add a GeometryRenderPass and a FinalRenderPass

		m_ResourcePool->clearReferences();

		push<GeometryRenderPass>();
		push<FinalRenderPass>();
	}

	void FrameGraph::compile(Scene* scene) {
		assignResources();
		deleteUnusedRenderPasses();
		freeUnusedResources();
	}

	void FrameGraph::execute(Scene* scene) {
		// TODO
		for(RenderPass* renderPass : m_RenderPassExecutionList) {
			renderPass->execute(scene);
		}
		m_RenderPassExecutionList.clear();
	}

	inline void FrameGraph::assignResources() {

		for(RenderPass* renderPass : m_RenderPassExecutionList) {

			RenderPass::InputDescription inputDesc = renderPass->inputDescription();
			RenderPass::OutputDescription outputDesc = renderPass->outputDescription();

			for(uint8_t i = 0; i < inputDesc.bufferCount; ++i) {
				renderPass->m_Input.buffers[i] = m_ResourcePool->getBuffer(inputDesc.buffers[i]);
			}
			for(uint8_t i = 0; i < inputDesc.textureCount; ++i) {
				renderPass->m_Input.textures[i] = m_ResourcePool->getTexture2D(inputDesc.textures[i]);
			}

			for(uint8_t i = 0; i < outputDesc.bufferCount; ++i) {
				renderPass->m_Output.buffers[i] = m_ResourcePool->getBuffer(outputDesc.buffers[i]);
			}
			for(uint8_t i = 0; i < outputDesc.textureCount; ++i) {
				renderPass->m_Output.textures[i] = m_ResourcePool->getTexture2D(outputDesc.textures[i]);
			}

			// TODO: only compile render pass if input/output resources have changed
			renderPass->compile(m_ResourcePool);
		}
	}

	inline void FrameGraph::deleteUnusedRenderPasses() {

		std::remove_if(m_RenderPasses.begin(), m_RenderPasses.end(), [&](RenderPass* renderPass) {

			for(RenderPass* activeRenderPass : m_RenderPassExecutionList) {
				if(activeRenderPass->getId() == renderPass->getId()) return false;
			}

			DELETE_PTR(renderPass);
			return true;
		});
	}

	inline void FrameGraph::freeUnusedResources() {
		m_ResourcePool->freeUnreferencedResources();
	}

	// ====

	FrameGraphResourcePool* FrameGraphResourcePool::create() {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			return new VulkanFrameGraphResourcePool();
		}
		throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
	}
}