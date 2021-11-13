#pragma once

#include "milo/assets/AssetManager.h"

namespace milo {

	class MaterialViewerRenderer {
	public:
		virtual const Texture2D& render(Material* material) = 0;
	public:
		static MaterialViewerRenderer* create();
	public:
		virtual ~MaterialViewerRenderer() = default;
	};
}