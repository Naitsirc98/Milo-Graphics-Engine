#include "milo/assets/textures/TextureManager.h"

namespace milo {

	TextureManager::TextureManager() {
		m_WhiteTexture = Ref<Texture2D>(createWhiteTexture());
		m_BlackTexture = Ref<Texture2D>(createBlackTexture());
		m_WhiteCubemap = Ref<Cubemap>(createWhiteCubemap());
		m_BlackCubemap = Ref<Cubemap>(createBlackCubemap());
	}

	TextureManager::~TextureManager() {
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