#pragma once

#include "milo/common/Common.h"
#include "milo/graphics/textures/Texture.h"

namespace milo {

	class Asset {
		friend class AssetManager;
	protected:
		String m_Name;
		String m_Filename;
		Ref<Texture2D> m_Icon;
	public:
		Asset() = default;
		Asset(String name, String filename) : m_Name(std::move(name)), m_Filename(filename) {};
		virtual ~Asset() = default;
		inline const String& name() const {return m_Name;}
		inline const String& filename() const {return m_Filename;}
		inline const Ref<Texture2D> icon() const {return m_Icon;}
	};

}