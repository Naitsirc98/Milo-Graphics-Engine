#include "milo/assets/textures/TextureManager.h"
#include "milo/graphics/Graphics.h"
#include "milo/graphics/vulkan/textures/VulkanTexture2D.h"
#include "milo/graphics/vulkan/textures/VulkanIconFactory.h"
#include "milo/graphics/vulkan/VulkanContext.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>

namespace milo {

	TextureManager::TextureManager() {
	}

	TextureManager::~TextureManager() {
		DELETE_PTR(m_IconFactory);
	}

	void TextureManager::init() {

		m_WhiteTexture = Ref<Texture2D>(createWhiteTexture());
		m_BlackTexture = Ref<Texture2D>(createBlackTexture());
		m_WhiteCubemap = Ref<Cubemap>(createWhiteCubemap());
		m_BlackCubemap = Ref<Cubemap>(createBlackCubemap());

		if(Graphics::graphicsAPI() == GraphicsAPI::Vulkan) {
			m_IconFactory = new VulkanIconFactory();
		} else {
			throw MILO_RUNTIME_EXCEPTION("Unsupported Graphics API");
		}

		createDefaultIcons();
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

	Ref<Texture2D> TextureManager::load(const String& filename, PixelFormat format, bool flipY, uint32_t mipLevels) {

		if(m_Cache.find(filename) != m_Cache.end()) return m_Cache.at(filename);

		Image* image = Image::loadImage(filename, format, flipY);

		Texture2D* texture = Texture2D::create();

		Texture2D::AllocInfo allocInfo{};
		allocInfo.width = image->width();
		allocInfo.height = image->height();
		allocInfo.format = image->format();
		allocInfo.pixels = image->pixels();
		allocInfo.mipLevels = mipLevels;

		texture->allocate(allocInfo);
		texture->generateMipmaps();

		DELETE_PTR(image);

		auto result = Ref<Texture2D>(texture);

		m_Cache[filename] = result;

		return result;
	}

	Ref<Texture2D> TextureManager::getIcon(const String &name) const {
		return m_Icons.find(name) != m_Icons.end() ? m_Icons.at(name) : nullptr;
	}

	void TextureManager::addIcon(const String &name, Ref<Texture2D> texture) {
		m_Icons[name] = texture;
	}

	void TextureManager::removeIcon(const String& name) {
		m_Icons.erase(name);
	}

	Ref<Texture2D> TextureManager::createIcon(const String &name, Mesh *mesh, Material *material) {
		Ref<Texture2D> icon = Ref<Texture2D>(m_IconFactory->createIcon(mesh, material));
		addIcon(name, icon);
		return icon;
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

	void TextureManager::createDefaultIcons() {

		addIcon("DefaultMeshIcon", load("resources/icons/mesh_default.png", PixelFormat::RGBA8, true));
		addIcon("DefaultMaterialIcon", load("resources/icons/material_default.png", PixelFormat::RGBA8, true));

		// TODO
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