#include "milo/graphics/vulkan/rendering/passes/VulkanBoundingVolumeRenderPass.h"
#include "milo/graphics/vulkan/buffers/VulkanMeshBuffers.h"
#include "milo/graphics/rendering/WorldRenderer.h"
#include "milo/editor/MiloEditor.h"

namespace milo {

	VulkanBoundingVolumeRenderPass::VulkanBoundingVolumeRenderPass() {
		m_Device = VulkanContext::get()->device();
		createRenderPass();
		createGraphicsPipeline();
		mvk::Semaphore::create(m_SignalSemaphores.size(), m_SignalSemaphores.data());
		m_Device->graphicsCommandPool()->allocate(VK_COMMAND_BUFFER_LEVEL_PRIMARY, m_CommandBuffers.size(), m_CommandBuffers.data());
	}

	VulkanBoundingVolumeRenderPass::~VulkanBoundingVolumeRenderPass() {
		m_Device->graphicsCommandPool()->free(m_CommandBuffers.size(), m_CommandBuffers.data());
		DELETE_PTR(m_GraphicsPipeline);
		mvk::Semaphore::destroy(m_SignalSemaphores.size(), m_SignalSemaphores.data());
	}

	bool VulkanBoundingVolumeRenderPass::shouldCompile(Scene* scene) const {
		return false;
	}

	void VulkanBoundingVolumeRenderPass::compile(Scene* scene, FrameGraphResourcePool* resourcePool) {
	}

	void VulkanBoundingVolumeRenderPass::execute(Scene* scene) {

		const uint32_t imageIndex = VulkanContext::get()->vulkanPresenter()->currentImageIndex();
		VkCommandBuffer commandBuffer = m_CommandBuffers[imageIndex];
		VulkanQueue* queue = m_Device->graphicsQueue();

		buildCommandBuffer(imageIndex, commandBuffer, scene);

		VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitSemaphores = queue->waitSemaphores().data();
		submitInfo.waitSemaphoreCount = queue->waitSemaphores().size();
		submitInfo.pSignalSemaphores = &m_SignalSemaphores[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		submitInfo.commandBufferCount = 1;
		submitInfo.pWaitDstStageMask = &waitStageMask;

		VkFence fence = queue->lastFence();
		queue->submit(submitInfo, VK_NULL_HANDLE);
		queue->setFence(fence);
	}

	void VulkanBoundingVolumeRenderPass::buildCommandBuffer(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		mvk::CommandBuffer::BeginGraphicsRenderPassInfo beginInfo{};
		beginInfo.renderPass = m_RenderPass;
		beginInfo.graphicsPipeline = m_GraphicsPipeline->vkPipeline();

		mvk::CommandBuffer::beginGraphicsRenderPass(commandBuffer, beginInfo);
		{
			renderMeshViews(imageIndex, commandBuffer, scene);
		}
		mvk::CommandBuffer::endGraphicsRenderPass(commandBuffer);
	}

	void VulkanBoundingVolumeRenderPass::renderMeshViews(uint32_t imageIndex, VkCommandBuffer commandBuffer, Scene* scene) {

		Matrix4 projView;

		if(getSimulationState() == SimulationState::Editor) {

			const EditorCamera& camera = MiloEditor::camera();
			projView = camera.projMatrix() * camera.viewMatrix();

		} else {

			Entity cameraEntity = scene->cameraEntity();
			const Camera& camera = cameraEntity.getComponent<Camera>();
			Matrix4 proj = camera.projectionMatrix();
			Matrix4 view = camera.viewMatrix(cameraEntity.getComponent<Transform>().translation);
			projView = proj * view;
		}

		Mesh* cube = Assets::meshes().getCube();
		Mesh* sphere = Assets::meshes().getSphere();
		Mesh* lastMesh = nullptr;

		VulkanMeshBuffers* cubeBuffers = (VulkanMeshBuffers*)cube->buffers();
		VulkanMeshBuffers* sphereBuffers = (VulkanMeshBuffers*)sphere->buffers();

		PushConstants pushConstants{};
		pushConstants.color = {0, 1, 0, 1};

		for(const DrawCommand& command : WorldRenderer::get().drawCommands()) {

			Vector3 s;
			Vector3 t;
			decomposeModelMatrix(command.transform, &s, nullptr, &t);
			Matrix4 modelMatrix = translate(t) * scale(s);

			const BoundingVolume& volume = command.mesh->boundingVolume();
			if(volume.type() == BoundingVolume::Type::Sphere) {

				const auto& sphereVolume = (const BoundingSphere&) volume;

				if(lastMesh != sphere) {

					VkBuffer vertexBuffers[] = {sphereBuffers->vertexBuffer()->vkBuffer()};
					VkDeviceSize offsets[] = {0};
					VK_CALLV(vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets));

					if(!sphere->indices().empty()) {
						VK_CALLV(vkCmdBindIndexBuffer(commandBuffer, sphereBuffers->indexBuffer()->vkBuffer(), 0, VK_INDEX_TYPE_UINT32));
					}

					lastMesh = sphere;
				}

				modelMatrix *= scale(Vector3(sphereVolume.radius));

			} else {

				if (lastMesh != cube) {

					VkBuffer vertexBuffers[] = {cubeBuffers->vertexBuffer()->vkBuffer()};
					VkDeviceSize offsets[] = {0};
					VK_CALLV(vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets));

					if (!cube->indices().empty()) {
						VK_CALLV(vkCmdBindIndexBuffer(commandBuffer, cubeBuffers->indexBuffer()->vkBuffer(), 0,
													  VK_INDEX_TYPE_UINT32));
					}

					lastMesh = cube;
				}

				if(volume.type() == BoundingVolume::Type::AABB) {

					const auto& aabb = (const AABB&) volume;

					modelMatrix = scale(modelMatrix, aabb.size);

				} else {

					const auto& obb = (const OBB&) volume;

					Matrix4 rotation(1.0f);
					rotation[0] = Vector4(obb.xAxis, 0);
					rotation[1] = Vector4(obb.yAxis, 0);
					rotation[2] = Vector4(obb.zAxis, 0);

					modelMatrix = translate(modelMatrix, obb.center) * scale(Matrix4(1.0f), obb.size) * rotation;
				}
			}

			pushConstants.projViewModel = projView * modelMatrix;

			VK_CALLV(vkCmdPushConstants(commandBuffer, m_GraphicsPipeline->pipelineLayout(),
										VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
										0, sizeof(PushConstants), &pushConstants));

			if(lastMesh->indices().empty()) {
				VK_CALLV(vkCmdDraw(commandBuffer, lastMesh->vertices().size(), 1, 0, 0));
			} else {
				VK_CALLV(vkCmdDrawIndexed(commandBuffer, lastMesh->indices().size(), 1, 0, 0, 0));
			}
		}
	}

	void VulkanBoundingVolumeRenderPass::createRenderPass() {

		RenderPass::Description desc;
		desc.colorAttachments.push_back({PixelFormat::RGBA32F, 1, LoadOp::Load});
		desc.depthAttachment = {PixelFormat::DEPTH, 1, LoadOp::Load};

		m_RenderPass = mvk::RenderPass::create(desc);
	}

	void VulkanBoundingVolumeRenderPass::createGraphicsPipeline() {

		VulkanGraphicsPipeline::CreateInfo pipelineInfo{};
		pipelineInfo.vkRenderPass = m_RenderPass;

		pipelineInfo.depthStencil.depthTestEnable = VK_FALSE;
		//pipelineInfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		pipelineInfo.rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
		pipelineInfo.rasterizationState.lineWidth = 2;

		pipelineInfo.shaders.push_back({"resources/shaders/bounding_volume/bounding_volume.vert", VK_SHADER_STAGE_VERTEX_BIT});
		pipelineInfo.shaders.push_back({"resources/shaders/bounding_volume/bounding_volume.frag", VK_SHADER_STAGE_FRAGMENT_BIT});

		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		pipelineInfo.dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

		VkPushConstantRange pushConstants{};
		pushConstants.offset = 0;
		pushConstants.size = sizeof(PushConstants);
		pushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		pipelineInfo.pushConstantRanges.push_back(pushConstants);

		m_GraphicsPipeline = new VulkanGraphicsPipeline("VulkanBoundingVolumeRenderPass", m_Device, pipelineInfo);
	}
}