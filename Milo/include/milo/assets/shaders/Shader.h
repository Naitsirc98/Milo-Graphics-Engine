#pragma once

#include "milo/common/Common.h"

namespace milo {

	class Shader {
		friend class ShaderManager;
	public:
		enum class Type {
			Undefined, Vertex, Fragment, Geometry, TessControl, TessEvaluation, Compute
		};
	private:
		String m_Filename;
		Type m_Type{Type::Undefined};
	protected:
		Shader(String filename, Type type) : m_Filename(std::move(filename)), m_Type(type) {}
		virtual ~Shader() = default;
		inline const String& filename() const {return m_Filename;}
		inline Type type() const {return m_Type;}
	};
}