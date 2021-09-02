#pragma once

#include "milo/graphics/textures/Texture.h"

namespace milo {

	class TextureManager {
		friend class AssetManager;
	private:
		Ref<Texture2D> m_WhiteTexture;
		Ref<Texture2D> m_BlackTexture;
		Ref<Cubemap> m_WhiteCubemap;
		Ref<Cubemap> m_BlackCubemap;
	private:
		TextureManager();
		~TextureManager();
	public:
		Ref<Texture2D> whiteTexture() const;
		Ref<Texture2D> blackTexture() const;
		Ref<Cubemap> whiteCubemap() const;
		Ref<Cubemap> blackCubemap() const;
		Ref<Texture2D> createTexture2D();
		Ref<Cubemap> createCubemap();
	private:
		static Texture2D* createWhiteTexture();
		static Texture2D* createBlackTexture();
		static Cubemap* createWhiteCubemap();
		static Cubemap* createBlackCubemap();
	};

}