#include "milo/editor/UIRenderer.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>

#ifdef _WINDOWS

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "milo/graphics/Window.h"

#include <Windows.h>

#endif

namespace milo::UI {

	void image(const Texture2D& texture, const Size& size, const ImVec2& uv0, const ImVec2& uv1,
			   const ImVec4& tint_col, const ImVec4& border_col) {

		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {

			const auto& vkTexture = dynamic_cast<const VulkanTexture2D&>(texture);

			auto textureId = ImGui_ImplVulkan_AddTexture(vkTexture.id(), vkTexture.vkSampler(), vkTexture.vkImageView(), vkTexture.layout());

			ImVec2 imageSize;
			if(size.isZero()) {
				imageSize = {(float)texture.size().width, (float)texture.size().height};
			} else {
				imageSize = {(float)size.width, (float)size.height};
			}

			ImVec2 uv1_ = uv1;
			uv1_.y *= -1;

			ImGui::Image(textureId, imageSize, uv0, uv1_, tint_col, border_col);

		} else {
			throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
		}
	}

	namespace FileDialog {

		Optional<String> open(const char* filter) {
#ifdef _WINDOWS

			OPENFILENAMEA ofn;
			CHAR szFile[260]{0};

			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = glfwGetWin32Window(Window::get()->handle());
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

			if (GetOpenFileNameA(&ofn) == TRUE) {
				return ofn.lpstrFile;
			}
#endif
			return {};
		}

		Optional<String> save(const char* filter) {
#ifdef _WINDOWS

			OPENFILENAMEA ofn;
			CHAR szFile[260]{0};

			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = glfwGetWin32Window(Window::get()->handle());
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = filter;
			ofn.nFilterIndex = 1;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

			if (GetSaveFileNameA(&ofn) == TRUE) {
				return ofn.lpstrFile;
			}
#endif
			return {};
		}

	}
}