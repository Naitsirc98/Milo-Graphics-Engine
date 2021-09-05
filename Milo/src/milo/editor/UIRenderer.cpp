#include "milo/editor/UIRenderer.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

namespace milo::UI {

	void image(const Texture2D& texture, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1,
			   const ImVec4& tint_col, const ImVec4& border_col) {

		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {

			const VulkanTexture2D& vkTexture = dynamic_cast<const VulkanTexture2D&>(texture);

			auto textureId = ImGui_ImplVulkan_AddTexture(vkTexture.id(), vkTexture.vkSampler(), vkTexture.vkImageView(), vkTexture.layout());

			ImGui::Image(textureId, size, uv0, uv1, tint_col, border_col);

		} else {
			throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
		}
	}

}