#pragma once

#include "milo/graphics/textures/Texture.h"

namespace milo {

	class TextureManager {
		friend class AssetManager;
		friend class Texture2D;
		friend class Cubemap;
	private:
		AtomicUInt m_TextureIdProvider{0};
		Ref<Texture2D> m_WhiteTexture;
		Ref<Texture2D> m_BlackTexture;
		Ref<Cubemap> m_WhiteCubemap;
		Ref<Cubemap> m_BlackCubemap;
	private:
		TextureManager();
		~TextureManager();
		void init();
	public:
		Ref<Texture2D> whiteTexture() const;
		Ref<Texture2D> blackTexture() const;
		Ref<Cubemap> whiteCubemap() const;
		Ref<Cubemap> blackCubemap() const;
		Ref<Texture2D> createTexture2D();
		Ref<Cubemap> createCubemap();
		Ref<Texture2D> load(const String& filename, PixelFormat format = PixelFormat::RGBA8);
	private:
		uint32_t nextTextureId();
		void registerTexture(const Texture2D& texture);
		void unregisterTexture(const Texture2D& texture);
		void registerTexture(const Cubemap& texture);
		void unregisterTexture(const Cubemap& texture);
	private:
		static Texture2D* createWhiteTexture();
		static Texture2D* createBlackTexture();
		static Cubemap* createWhiteCubemap();
		static Cubemap* createBlackCubemap();
	};

}