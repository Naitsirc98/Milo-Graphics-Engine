#pragma once

#include "milo/graphics/textures/Texture.h"

namespace milo {

	static const Size DEFAULT_ICON_SIZE = {64, 64};

	class Mesh;
	class Material;

	class IconFactory {
		friend class TextureManager;
	protected:
		virtual ~IconFactory() = default;
	public:
		virtual Texture2D* createIcon(Mesh* mesh, Material* material, const Size& size = DEFAULT_ICON_SIZE) = 0;
	};

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
		Ref<Texture2D> m_BRDF;
		IconFactory* m_IconFactory{nullptr};
		HashMap<String, Ref<Texture2D>> m_Cache;
		HashMap<String, Ref<Texture2D>> m_Icons;
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
		Ref<Texture2D> load(const String& filename, PixelFormat format = PixelFormat::RGBA8, bool flipY = false, uint32_t mipLevels = AUTO_MIP_LEVELS);
		Ref<Texture2D> getIcon(const String& name) const;
		void addIcon(const String& name, Ref<Texture2D> texture);
		void removeIcon(const String& name);
		Ref<Texture2D> createIcon(const String& name, Mesh* mesh, Material* material);
	private:
		uint32_t nextTextureId();
		void registerTexture(const Texture2D& texture);
		void unregisterTexture(const Texture2D& texture);
		void registerTexture(const Cubemap& texture);
		void unregisterTexture(const Cubemap& texture);
		void createDefaultIcons();
	private:
		static Texture2D* createWhiteTexture();
		static Texture2D* createBlackTexture();
		static Cubemap* createWhiteCubemap();
		static Cubemap* createBlackCubemap();
	};

}