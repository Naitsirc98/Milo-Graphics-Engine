#pragma once

#include "milo/common/Common.h"
#include "milo/graphics/textures/Texture.h"

namespace milo {

	class Material {
		friend class MaterialManager;
	private:
		String m_Filename;
		Color m_BaseColor = {1, 1, 1, 1};
		Texture2D* m_BaseColorTexture = nullptr;
		// TODO
	private:
		explicit Material(String filename);
		~Material();
	public:
		const String& filename() const;
		const Color& baseColor() const;
		Texture2D* baseColorTexture() const;
	};
}