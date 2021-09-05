#include "milo/assets/textures/TextureManager.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>

namespace milo {

	TextureManager::TextureManager() {
	}

	TextureManager::~TextureManager() {
	}

	void TextureManager::init() {
		m_WhiteTexture = Ref<Texture2D>(createWhiteTexture());
		m_BlackTexture = Ref<Texture2D>(createBlackTexture());
		m_WhiteCubemap = Ref<Cubemap>(createWhiteCubemap());
		m_BlackCubemap = Ref<Cubemap>(createBlackCubemap());
	}

	Ref<Texture2D> TextureManager::whiteTexture() const {
		return m_WhiteTexture;
	}

	Ref<Texture2D> TextureManager::blackTexture() const {
		return m_BlackTexture;
	}

	Ref<Cubemap> TextureManager::whiteCubemap() const {
		return m_WhiteCubemap;
	}

	Ref<Cubemap> TextureManager::blackCubemap() const {
		return m_BlackCubemap;
	}

	Ref<Texture2D> TextureManager::createTexture2D() {
		return Ref<Texture2D>(Texture2D::create());
	}

	Ref<Cubemap> TextureManager::createCubemap() {
		return Ref<Cubemap>(Cubemap::create());
	}

	Ref<Texture2D> TextureManager::load(const String& filename, PixelFormat format) {

		Image* image = Image::loadImage(filename, format);

		Texture2D* texture = Texture2D::create();

		Texture2D::AllocInfo allocInfo{};
		allocInfo.width = image->width();
		allocInfo.height = image->height();
		allocInfo.format = image->format();
		allocInfo.pixels = image->pixels();

		texture->allocate(allocInfo);

		DELETE_PTR(image);

		return Ref<Texture2D>(texture);
	}

	uint32_t TextureManager::nextTextureId() {
		return m_TextureIdProvider++;
	}

	void TextureManager::registerTexture(const Texture2D& texture) {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			if(texture.usage() & TEXTURE_USAGE_UI_BIT) {
				const auto& vkTex = dynamic_cast<const VulkanTexture2D&>(texture);
				ImGui_ImplVulkan_AddTexture(vkTex.id(), vkTex.vkSampler(), vkTex.vkImageView(), vkTex.layout());
			}
		} // TODO
	}

	void TextureManager::unregisterTexture(const Texture2D& texture) {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			if(texture.usage() & TEXTURE_USAGE_UI_BIT)
				ImGui_ImplVulkan_DeleteTexture(texture.id());
		} // TODO
	}

	void TextureManager::registerTexture(const Cubemap& texture) {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			if(texture.usage() & TEXTURE_USAGE_UI_BIT) {
				const auto& vkTex = dynamic_cast<const VulkanTexture2D&>(texture);
				ImGui_ImplVulkan_AddTexture(vkTex.id(), vkTex.vkSampler(), vkTex.vkImageView(), vkTex.layout());
			}
		} // TODO
	}

	void TextureManager::unregisterTexture(const Cubemap& texture) {
		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			if(texture.usage() & TEXTURE_USAGE_UI_BIT) ImGui_ImplVulkan_DeleteTexture(texture.id());
		} // TODO
	}

	Texture2D* TextureManager::createWhiteTexture() {

		Texture2D* texture = Texture2D::create();

		Image* image = Image::createWhite(PixelFormat::SRGBA);

		Texture2D::AllocInfo allocInfo = {};
		allocInfo.width = image->width();
		allocInfo.height = image->height();
		allocInfo.format = image->format();
		allocInfo.pixels = image->pixels();

		texture->allocate(allocInfo);

		DELETE_PTR(image);

		return texture;
	}

	Texture2D* TextureManager::createBlackTexture() {

		Texture2D* texture = Texture2D::create();

		Image* image = Image::createBlack(PixelFormat::SRGBA);

		Texture2D::AllocInfo allocInfo = {};
		allocInfo.width = image->width();
		allocInfo.height = image->height();
		allocInfo.format = image->format();
		allocInfo.pixels = image->pixels();

		texture->allocate(allocInfo);

		DELETE_PTR(image);

		return texture;
	}

	Cubemap* TextureManager::createWhiteCubemap() {

		Cubemap* texture = Cubemap::create();

		byte_t pixels[4 * 6 * 6]{1};

		Cubemap::AllocInfo allocInfo = {};
		allocInfo.width = 1;
		allocInfo.height = 1;
		allocInfo.format = PixelFormat::SRGBA;

		texture->allocate(allocInfo);

		Cubemap::UpdateInfo updateInfo{};
		updateInfo.size = 4 * 6 * 6;
		updateInfo.pixels = pixels;

		texture->update(updateInfo);

		return texture;
	}

	Cubemap* TextureManager::createBlackCubemap() {

		Cubemap* texture = Cubemap::create();

		byte_t pixels[4 * 6 * 6]{0};

		Cubemap::AllocInfo allocInfo = {};
		allocInfo.width = 1;
		allocInfo.height = 1;
		allocInfo.format = PixelFormat::SRGBA;

		texture->allocate(allocInfo);

		Cubemap::UpdateInfo updateInfo{};
		updateInfo.size = 4 * 6 * 6;
		updateInfo.pixels = pixels;

		texture->update(updateInfo);

		return texture;
	}
}