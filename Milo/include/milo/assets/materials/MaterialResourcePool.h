#pragma once

#include "Material.h"

namespace milo {

	class MaterialResourcePool {
	public:
		virtual ~MaterialResourcePool() = default;
		virtual void allocateMaterialResources(Material* material) = 0;
		virtual void updateMaterial(Material* material) = 0;
		virtual void freeMaterialResources(Material* material) = 0;
	public:
		static MaterialResourcePool* create();
	};

}